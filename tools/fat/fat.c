#include <stdio.h>
#include <stdlib.h>

int main(int argc, char** argv)
{
    if (argc < 3)
    {
        printf("Syntax: %s <disk_image> <file_name>\n", argv[0]);

        return  EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}
