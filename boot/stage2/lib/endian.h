#ifndef BOOT_STAGE2_ENDIAN_H
#define BOOT_STAGE2_ENDIAN_H

#include <stdint.h>

uint16_t read_le16(const uint8_t *p);

uint32_t read_le32(const uint8_t *p);

#endif
