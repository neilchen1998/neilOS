#define VGA_MEMORY ((volatile unsigned short *)0xB8000)

static unsigned int cursor = 0;

void puts(const char *str)
{
    while (*str)
    {
        VGA_MEMORY[cursor++] = (unsigned short)*str | 0x0F00;
        str++;
    }
}

void kmain(void)
{
    VGA_MEMORY[0] = 'X' | 0x0F00;

    puts("Hello from neilOS!");

    for (;;)
    {
        __asm__ volatile ("hlt");
    }
}
