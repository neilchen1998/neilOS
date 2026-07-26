#ifndef FAT_H
#define FAT_H

#include <stdint.h>

#pragma pack(push, 1)

typedef struct {
    uint8_t  jump[3];              // Jump instruction
    char     oemName[8];           // OEM identifier

    // BIOS Parameter Block (BPB)
    uint16_t bytesPerSector;
    uint8_t  sectorsPerCluster;
    uint16_t reservedSectors;
    uint8_t  fatCount;
    uint16_t rootEntryCount;
    uint16_t totalSectors16;
    uint8_t  mediaDescriptor;
    uint16_t sectorsPerFAT16;
    uint16_t sectorsPerTrack;
    uint16_t numHeads;
    uint32_t hiddenSectors;
    uint32_t totalSectors32;

    // Extended Boot Record (FAT12)
    uint8_t  driveNumber;
    uint8_t  reserved1;
    uint8_t  bootSignature;
    uint32_t volumeID;
    char     volumeLabel[11];
    char     fileSystemType[8];    // "FAT12   "

    // Boot code
    uint8_t  bootCode[448];

    // Boot signature
    uint16_t signature;            // Must be 0xAA55
} BootSector;

#pragma pack(pop)

#endif
