#include <stdint.h>

// @brief Prints a single character using BIOS teletype.
//
// Uses BIOS interrupt 0x10 with function 0x0E to display
// the input character on the active text-mode screen.
//
// @param c The character to print.
static void bios_print_char(char c)
{
    // AH = 0x0E selects teletype output
    // AL = c is the character to print
    uint16_t ax = 0x0E00u | (unsigned char)c;

    // BH = 0x00 displays page 0
    // BL = 0x07 sets the text attribute to light gray
    uint16_t bx = 0x0007u;

    // Inline assembly
    __asm__ volatile (
        "int $0x10"
        :
        : "a"(ax), "b"(bx)  // loads ax into AX and bx into BX
    );
}

// @brief Prints a string using BIOS teletype.
//
// Uses BIOS interrupt 0x10 with function 0x0E to display
// the input string on the active text-mode screen.
//
// @param str The string to print.
static void bios_print(const char* str)
{
    while (*str != '\0')
    {
        bios_print_char(*str++);
    }
}

void stage2_main(void)
{
    bios_print("stage2_main reached!\n");
}
