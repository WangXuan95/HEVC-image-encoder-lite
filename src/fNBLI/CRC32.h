#ifndef __CRC32_H__
#define __CRC32_H__

#include <cstdint>

inline static uint32_t calculateCRC32 (uint8_t *p_data, size_t size) {
    const static uint32_t TABLE_CRC32 [] = { 0x00000000, 0x1db71064, 0x3b6e20c8, 0x26d930ac, 0x76dc4190, 0x6b6b51f4, 0x4db26158, 0x5005713c, 0xedb88320, 0xf00f9344, 0xd6d6a3e8, 0xcb61b38c, 0x9b64c2b0, 0x86d3d2d4, 0xa00ae278, 0xbdbdf21c };
    uint32_t crc = 0xFFFFFFFF;
    uint8_t *p_data_end = p_data + size;
    for (; p_data<p_data_end; p_data++) {
        crc ^= *p_data;
        crc = TABLE_CRC32[crc & 0x0f] ^ (crc >> 4);
        crc = TABLE_CRC32[crc & 0x0f] ^ (crc >> 4);
    }
    return crc;
}

#endif // __CRC32_H__
