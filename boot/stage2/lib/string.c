#include "string.h"

#include <stdarg.h>
#include <stdint.h>

int memcmp(const void *ptr1, const void *ptr2, uint32_t n)
{
    const unsigned char *a = (const unsigned char *)ptr1;
    const unsigned char *b = (const unsigned char *)ptr2;

    for (uint32_t i = 0; i < n; i++)
    {
        if (a[i] != b[i])
        {
            return (a[i] > b[i]) ? 1 : -1;
        }
    }

    return 0;
}

uint32_t strlen(const char *str)
{
    uint32_t length = 0;

    while (str[length] != '\0')
    {
        ++length;
    }

    return length;
}

int strcmp(const char *s1, const char *s2)
{
    while (*s1 && *s2 && *s1 == *s2)
    {
        ++s1;
        ++s2;
    }

    return (unsigned char)*s1 - (unsigned char)*s2;
}

int strncmp(const char *s1, const char *s2, uint32_t n)
{
    uint32_t i = 0;

    while (i < n)
    {
        if (s1[i] != s2[i])
        {
            return (unsigned char)s1[i] - (unsigned char)s2[i];
        }

        if (s1[i] == '\0')
        {
            return 0;
        }

        ++i;
    }

    return 0;
}
