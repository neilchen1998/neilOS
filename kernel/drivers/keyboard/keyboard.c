#include "keyboard.h"

#include <stdbool.h>
#include <stdint.h>

#include "arch/x86/io.h"
#include "arch/x86/pic.h"
#include "drivers/video/vga.h"

#define KEYBOARD_DATA_PORT 0x60
#define KEYBOARD_STATUS_PORT 0x64

#define KEYBOARD_SCANCODE_COUNT 128

#define SCANCODE_LEFT_SHIFT 42
#define SCANCODE_CAPS_LOCK 58
#define SCANCODE_RIGHT_SHIFT 54

#define SCANCODE_EXTENDED 0xE0

#define SCANCODE_UP 0x48
#define SCANCODE_PAGE_UP 0x49
#define SCANCODE_DOWN 0x50
#define SCANCODE_PAGE_DOWN 0x51

static bool extendedScancode = false;

// Left & right shift keys
static bool leftShiftPressed = false;
static bool rightShiftPressed = false;

static bool capsLockEnabled = false;

static const char keyboardMap[KEYBOARD_SCANCODE_COUNT] = {
    0, 27,  '1', '2', '3', '4', '5', '6', '7', '8', '9', '0',  '-', '=', '\b', '\t', 'q', 'w', 'e', 'r', 't', 'y', 'u', 'i', 'o', 'p', '[', ']', '\n',
    0, 'a', 's', 'd', 'f', 'g', 'h', 'j', 'k', 'l', ';', '\'', '`', 0,   '\\', 'z',  'x', 'c', 'v', 'b', 'n', 'm', ',', '.', '/', 0,   '*', 0,   ' '};

static const char keyboardShiftMap[KEYBOARD_SCANCODE_COUNT] = {
    0, 27,  '!', '@', '#', '$', '%', '^', '&', '*', '(', ')', '_', '+', '\b', '\t', 'Q', 'W', 'E', 'R', 'T', 'Y', 'U', 'I', 'O', 'P', '{', '}', '\n',
    0, 'A', 'S', 'D', 'F', 'G', 'H', 'J', 'K', 'L', ':', '"', '~', 0,   '|',  'Z',  'X', 'C', 'V', 'B', 'N', 'M', '<', '>', '?', 0,   '*', 0,   ' '};

// @brief Waits for the PS/2 keyboard controller's output buffer to have data available.
//
// Polls the keyboard status port up to a maximum number of attempts to check
// if bit 0 (Output Buffer Status) is set, indicating that data can be read from the data port.
//
// @return TRUE if the output buffer becomes ready within the fixed number of attempts; otherwise FALSE.
static bool keyboard_wait_input_clear(void)
{
    for (uint32_t attempt = 0; attempt < 100'000u; ++attempt)
    {
        // Read the status register of the keyboard controller
        // The first bit represents the input buffer status
        if (inb(KEYBOARD_STATUS_PORT) & 0x02u)
        {
            return true;
        }
    }

    return false;
}

// @brief Sets or clears the Caps Lock LED on the keyboard.
//
// Communicates with the 8042 keyboard controller using standard
// PS/2 protocol commands to update the keyboard indicator lights.
static void keyboard_set_caps_lock_led(void)
{
    bool acknowledged = false;

    if (!keyboard_wait_input_clear())
    {
        return;
    }

    outb(KEYBOARD_DATA_PORT, 0xED);

    for (uint32_t attempt = 0; attempt < 100'000u; ++attempt)
    {
        // The zeroth bit represents the output buffer status
        if (inb(KEYBOARD_STATUS_PORT) & 0x01u)
        {
            // The fifth bit represents the auxiliary output buffer status (mouse not keyboard)
            // The sixth bit represents the timeout error status
            // The seventh represents the parity error status
            if (inb(KEYBOARD_STATUS_PORT) & 0xFAu)
            {
                return;
            }

            acknowledged = true;

            break;
        }
    }

    if (!acknowledged || !keyboard_wait_input_clear())
    {
        return;
    }

    outb(KEYBOARD_DATA_PORT, (capsLockEnabled ? 0x04u : 0x00u));
}

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
    case SCANCODE_UP:
    {
        vga_scroll_up();
        break;
    }

    case SCANCODE_PAGE_UP:
    {
        vga_page_up();
        break;
    }

    case SCANCODE_DOWN:
    {
        vga_scroll_down();
        break;
    }

    case SCANCODE_PAGE_DOWN:
    {
        vga_page_down();
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

    if (scancode == SCANCODE_CAPS_LOCK)
    {
        capsLockEnabled = !capsLockEnabled;
        keyboard_set_caps_lock_led();

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
            vga_putchar(c);
        }
    }

    pic_send_eoi(1);
}
