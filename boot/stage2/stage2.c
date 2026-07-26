void print_char(char c)
{
    volatile char *video = (char *)0xb8000;

    video[0] = c;
    video[1] = 0x07;
}

void stage2_main(void)
{
    print_char('S');
}
