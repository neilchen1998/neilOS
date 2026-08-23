#include "keyboard.h"

#include <stdbool.h>
#include <stdint.h>

#include "arch/x86/io.h"
#include "arch/x86/pic.h"
#include "drivers/video/vga.h"

#define KEYBOARD_DATA_PORT 0x60

#define KEYBOARD_SCANCODE_COUNT 128

#define SCANCODE_LEFT_SHIFT 42
#define SCANCODE_RIGHT_SHIFT 54

#define SCANCODE_EXTENDED 0xE0

#define SCANCODE_PAGE_UP 0x49
#define SCANCODE_PAGE_DOWN 0x51

static bool extendedScancode = false;

// Left & right shift keys
static bool leftShiftPressed = false;
static bool rightShiftPressed = false;

static const char keyboardMap[KEYBOARD_SCANCODE_COUNT] = {
    0, 27,  '1', '2', '3', '4', '5', '6', '7', '8', '9', '0',  '-', '=', '\b', '\t', 'q', 'w', 'e', 'r', 't', 'y', 'u', 'i', 'o', 'p', '[', ']', '\n',
    0, 'a', 's', 'd', 'f', 'g', 'h', 'j', 'k', 'l', ';', '\'', '`', 0,   '\\', 'z',  'x', 'c', 'v', 'b', 'n', 'm', ',', '.', '/', 0,   '*', 0,   ' '};

static const char keyboardShiftMap[KEYBOARD_SCANCODE_COUNT] = {
    0, 27,  '!', '@', '#', '$', '%', '^', '&', '*', '(', ')', '_', '+', '\b', '\t', 'Q', 'W', 'E', 'R', 'T', 'Y', 'U', 'I', 'O', 'P', '{', '}', '\n',
    0, 'A', 'S', 'D', 'F', 'G', 'H', 'J', 'K', 'L', ':', '"', '~', 0,   '|',  'Z',  'X', 'C', 'V', 'B', 'N', 'M', '<', '>', '?', 0,   '*', 0,   ' '};

// @brief Handles an extended keyboard scancode.
//
// Handles an extended keyboard scancode received after an E0 prefix.
//
// @param scancode The extended keyboard scancode.
static void keyboard_handler_extended(uint8_t scancode)
{
    // Check if the key is released
    if (scancode & 0x80)
    {
        return;
    }

    switch (scancode)
    {
    case SCANCODE_PAGE_UP:
    {
        terminal_page_up();
        break;
    }

    case SCANCODE_PAGE_DOWN:
    {
        terminal_page_down();
        break;
    }

    default:
        break;
    }
}

void keyboard_init(void)
{
    uint8_t mask = inb(PIC1_DATA);

    // Enable IRQ1 (keyboard)
    mask &= ~(1 << 1);

    outb(PIC1_DATA, mask);
}

void keyboard_handler(void)
{
    uint8_t scancode = inb(KEYBOARD_DATA_PORT);

    if (scancode == SCANCODE_EXTENDED)
    {
        extendedScancode = true;

        pic_send_eoi(1);

        return;
    }

    if (extendedScancode)
    {
        extendedScancode = false;

        keyboard_handler_extended(scancode);

        pic_send_eoi(1);

        return;
    }

    // Check if the key is released
    if (scancode & 0x80)
    {
        scancode &= 0x7F;

        // Check left shift or right shift is released
        if (scancode == SCANCODE_LEFT_SHIFT)
        {
            leftShiftPressed = false;
        }

        if (scancode == SCANCODE_RIGHT_SHIFT)
        {
            rightShiftPressed = false;
        }

        pic_send_eoi(1);

        return;
    }

    // Check if the left shift or right shift is pressed
    if (scancode == SCANCODE_LEFT_SHIFT)
    {
        leftShiftPressed = true;

        pic_send_eoi(1);

        return;
    }

    if (scancode == SCANCODE_RIGHT_SHIFT)
    {
        rightShiftPressed = true;

        pic_send_eoi(1);

        return;
    }

    bool shiftPressed = leftShiftPressed || rightShiftPressed;

    if (scancode < KEYBOARD_SCANCODE_COUNT)
    {
        char c = (shiftPressed) ? keyboardShiftMap[scancode] : keyboardMap[scancode];

        // Only put the character if it is valid
        if (c != 0)
        {
            terminal_putchar(c);
        }
    }

    pic_send_eoi(1);
}
