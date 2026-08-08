#include <stdbool.h>
#include <stdint.h>

#include "debug.h"
#include "endian.h"
#include "print.h"
#include "string.h"

#define BOOT_SECTOR_SEG ((uint8_t*)0x2000u) // 512 bytes
#define FAT_SEG         ((uint8_t*)0x2200u) // up to 9 * 512 = 4608 bytes
#define ROOT_DIR_SEG    ((uint8_t*)0x3400u) // 512 bytes
#define KERNEL_SEG      ((uint8_t*)0x4000u) // kernel load area

uint8_t  g_drive;
uint16_t g_sectors_per_track;
uint16_t g_num_heads;

static uint8_t  g_num_fats;
static uint8_t  g_sectors_per_cluster;
static uint16_t g_bytes_per_sector;
static uint16_t g_reserved_sectors;
static uint16_t g_root_entry_count;
static uint32_t g_total_sectors;
static uint32_t g_sectors_per_fat;
static uint32_t g_root_dir_sectors;
static uint32_t g_first_data_sector;
static uint32_t g_first_fat_sector;
static uint32_t g_first_root_dir_sector;
static uint32_t g_data_sectors;
static uint32_t g_cluster_count;
static uint32_t g_root_dir_first_sector;

#pragma pack(push, 1)
typedef struct DirEntry
{
    char     name[8];
    char     ext[3];
    uint8_t  attribute;
    uint8_t  reserved[10];
    uint16_t modTime;
    uint16_t modDate;
    uint16_t firstCluster;
    uint32_t fileSize;
} DirEntry;

typedef struct
{
    uint16_t bytes_per_sector;
    uint8_t  sectors_per_cluster;
    uint16_t reserved_sectors;
    uint8_t  fat_count;
    uint16_t root_entries;
    uint16_t sectors_per_fat;
} BPB;
#pragma pack(pop)

_Static_assert(sizeof(DirEntry) == 32, "DirEntry must be 32 bytes");

extern int disk_get_params(void);
extern int disk_read_sector(uint32_t lba, uint16_t segment, uint16_t offset);
extern void jump_to_kernel(void);

static uint16_t current_ds(void)
{
    uint16_t seg;

    __asm__ volatile ("mov %%ds, %0" : "=r"(seg));

    return seg;
}

static int fat_init(uint8_t drive)
{
    g_drive = drive;
    uint16_t ds = current_ds();

    if (disk_get_params() != 0)
    {
        return -1;
    }

    if (disk_read_sector(0, ds, ((uint16_t)(uintptr_t)BOOT_SECTOR_SEG)) != 0)
    {
        return -1;
    }

    // Parse FAT12 BPB (BIOS Parameter Block) fields from the boot sector
    g_bytes_per_sector = read_le16(BOOT_SECTOR_SEG + 11);

    g_sectors_per_cluster = BOOT_SECTOR_SEG[13];

    g_reserved_sectors = read_le16(BOOT_SECTOR_SEG + 14);

    g_num_fats = BOOT_SECTOR_SEG[16];

    g_root_entry_count = read_le16(BOOT_SECTOR_SEG + 17);

    g_total_sectors = read_le16(BOOT_SECTOR_SEG + 19);

    if (g_total_sectors == 0)
    {
        printf("Falls back to 32-bit...\n");
        g_total_sectors = read_le32(BOOT_SECTOR_SEG + 32);
    }

    g_sectors_per_fat = read_le16(BOOT_SECTOR_SEG + 22);

    if (g_bytes_per_sector == 0 ||
        g_sectors_per_cluster == 0 ||
        g_reserved_sectors == 0 ||
        g_num_fats == 0 ||
        g_sectors_per_fat == 0 ||
        g_total_sectors == 0)
    {
        return -1;
    }

    // RootDirSectors = RootDirEntries × DirectoryEntrySize / BytesPerSector (DirectoryEntrySize is always 32)
    // and we need to perform a ceiling division here
    g_root_dir_sectors = ((g_root_entry_count * 32) + (g_bytes_per_sector - 1)) / g_bytes_per_sector;

    g_first_fat_sector = g_reserved_sectors;

    g_first_root_dir_sector = g_reserved_sectors + (g_num_fats * g_sectors_per_fat);

    g_root_dir_first_sector = g_first_root_dir_sector;

    g_first_data_sector = (g_first_root_dir_sector + g_root_dir_sectors);

    g_data_sectors = (g_total_sectors - g_first_data_sector);

    g_cluster_count = g_data_sectors / g_sectors_per_cluster;

    if (g_cluster_count >= 4085)
    {
        // Not FAT12
        return -1;
    }

    return 0;
}

static int fat_find_file(const char* name83, uint16_t* firstCluster, uint32_t* fileSize)
{
    uint16_t ds = current_ds();
    uint32_t root_lba = g_first_root_dir_sector;
    uint32_t root_sectors = g_root_dir_sectors;

    for (uint32_t i = 0; i < root_sectors; ++i)
    {
        if (disk_read_sector(root_lba + i, ds, ((uint16_t)(uintptr_t)ROOT_DIR_SEG)) != 0)
        {
            return -1;
        }

        uint16_t entries = g_bytes_per_sector / (uint16_t)sizeof(DirEntry);
        const DirEntry* e = (const DirEntry*)ROOT_DIR_SEG;

        for (uint16_t j = 0; j < entries; ++j, ++e)
        {
            // Check if it is the end of the directory
            if ((uint8_t)e->name[0] == 0x00u)
            {
                return -1;
            }

            // Check if it is a deleted entry
            if ((uint8_t)e->name[0] == 0xE5u)
            {
                continue;
            }

            // Check if it is an LFN entry
            if (e->attribute == 0x0Fu)
            {
                continue;
            }

            // Check if it is a directory or a volume label
            if ((e->attribute & 0x10u) || (e->attribute & 0x08u))
            {
                continue;
            }

            // Compare the data from the name field (8 bytes)
            // and the extension field (3 bytes)
            if (memcmp(e->name, name83, 8) == 0 && memcmp(e->ext, name83 + 8, 3) == 0)
            {
               *firstCluster = e->firstCluster;
               *fileSize = e->fileSize;
               return 0;
            }
        }
    }

    return -1;
}

uint16_t fat12_next_cluster(const uint8_t* fat, uint16_t cluster)
{
    uint16_t offset = cluster + (cluster / 2);  // offset = 3 * cluster / 2
    uint16_t value;

    // Check if the offset is an odd number
    if (cluster & 1u)
    {
        // Use the high 12 bits
        value = ((fat[offset] >> 4) | (fat[offset + 1] << 4)) & 0x0FFF;
    }
    else
    {
        // Use the low 12 bits
        value = (fat[offset] | ((fat[offset + 1] & 0x0F) << 8)) & 0x0FFF;
    }

    return value;
}

static int fat_read_table(void)
{
    uint16_t ds = current_ds();

    for (uint32_t i = 0; i < g_sectors_per_fat; ++i)
    {
        uint16_t dstOff = (uint16_t)((uintptr_t)FAT_SEG + (i * g_bytes_per_sector));

        if (disk_read_sector(g_first_fat_sector + i, ds, dstOff) != 0)
        {
            return -1;
        }
    }

    return 0;
}

static int fat_load_file(uint16_t firstCluster, uint32_t fileSize, void* destination)
{
    uint16_t ds = current_ds();
    uint8_t* dest = destination;
    uint16_t cluster = firstCluster;
    uint32_t remaining = fileSize;

    if (cluster < 2u)
    {
        return -1;
    }

    if (fat_read_table() != 0)
    {
        return -1;
    }

    while (remaining)
    {
        uint32_t lba = g_first_data_sector + ((uint32_t)cluster - 2u) * g_sectors_per_cluster;

        uint8_t i = 0;
        while ((i < g_sectors_per_cluster) && (remaining > 0))
        {
            uint16_t dstOff = (uint16_t)(uintptr_t)dest;
            uint32_t chunk = g_bytes_per_sector;

            if (disk_read_sector(lba + i, ds, dstOff) != 0)
            {
                return -1;
            }

            if (chunk > remaining)
            {
                chunk = remaining;
            }

            dest += chunk;
            remaining -= chunk;
            ++i;
        }

        if (remaining == 0)
        {
            return 0;
        }

        cluster = fat12_next_cluster(FAT_SEG, cluster);

        // Checks if the cluster is bad
        if (cluster == 0xFF7u)
        {
            return -1;
        }

        // Check if the chain ends too early or invalid
        if (cluster < 2u || cluster >= 0xFF8u)
        {
            return -1;
        }
    }

    return 0;
}

void debug_print_name83(const DirEntry* e)
{
    for (uint16_t i = 0; i < 8; ++i)
    {
        char c = e->name[i];
        bios_print_char(c == ' ' ? '_' : c);
    }
}

void debug_dump_root_dir(void)
{
    uint16_t ds = current_ds();
    uint32_t root_lba = g_first_root_dir_sector;
    uint32_t root_sector = g_root_dir_sectors;
    uint16_t printed = 0;

    bios_print("Root entries seen:\n");

    for (uint32_t i = 0; i < root_sector; ++i)
    {
        if (disk_read_sector(root_lba + i, ds, (uint16_t)(uintptr_t)ROOT_DIR_SEG) != 0)
        {
            bios_print("ERROR: cannot read!\n");
            return;
        }

        uint16_t entries = g_bytes_per_sector / (uint16_t)sizeof(DirEntry);
        const DirEntry* e = (const DirEntry*)ROOT_DIR_SEG;

        for (uint16_t j = 0; j < entries && printed < 12; ++j, ++e)
        {
            if ((uint8_t)e->name[0] == 0x00u)
            {
                return;
            }

            if ((uint8_t)e->name[0] == 0xE5u || e->attribute == 0x0Fu)
            {
                continue;
            }

            if (e->attribute & 0x10u || (e->attribute & 0x08u))
            {
                continue;
            }

            bios_print("   ");
            debug_print_name83(e);
            bios_print("\n");
            ++printed;
        }
    }
}

void stage2_main(void)
{
    bios_print("stage2_main reached!\n");

    uint16_t cluster;
    uint32_t fileSize;

    if (fat_init(g_drive) != 0)
    {
        bios_print("ERROR: FAT init failed!\n");
        for (;;)
        {
            // empty
        }
    }

    bios_print("Looking for kernel...\n");

    if (fat_find_file("KERNEL     ", &cluster, &fileSize) != 0 && fat_find_file("KERNEL  BIN", &cluster, &fileSize) != 0)
    {
        debug_dump_root_dir();
        bios_print("ERROR: Kernel not found!\n");
        for (;;)
        {
            // empty
        }
    }

    if (fat_load_file(cluster, fileSize, (void*)KERNEL_SEG) != 0)
    {
        bios_print("ERROR: Load failed!\n");
        for (;;)
        {
            // empty
        }
    }

    bios_print("Starting kernel...\n");

#ifdef DEBUG
    {
        uint8_t* k = (uint8_t*)KERNEL_SEG;
        uint16_t jump_seg = (uint16_t)(current_ds() + (((uint16_t)(uintptr_t)KERNEL_SEG) >> 4));

        printf("jump %x:0000\n", jump_seg);
        printf("k[0..7] %x %x %x %x %x %x %x %x\n", k[0], k[1], k[2], k[3], k[4], k[5], k[6], k[7]);
    }
#endif

    jump_to_kernel();

    for (;;)
    {
        // empty
    }
}
