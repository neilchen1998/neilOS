#ifndef BOOT_STAGE2_STRING_H
#define BOOT_STAGE2_STRING_H

#include <stdarg.h>
#include <stdint.h>

int memcmp(const void *ptr1, const void *ptr2, uint32_t n);

uint32_t strlen(const char *str);

int strcmp(const char *s1, const char *s2);

int strncmp(const char *s1, const char *s2, uint32_t n);

#endif
