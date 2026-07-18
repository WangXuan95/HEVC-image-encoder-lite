#ifndef __HEADER_H__
#define __HEADER_H__

#include <cstdint>

template <bool IS_WRITE>
inline static uint16_t *rwHeader (uint16_t *p_buf, uint32_t &height, uint32_t &width, bool &is_rgb, bool &is_large, uint32_t &crc32) {
    const static uint16_t HEADER1      = ('f' | ((uint16_t)'n'<<8));  // "fn"
    const static uint16_t HEADER2_gray = ('b' | ((uint16_t)'G'<<8));  // "bG"
    const static uint16_t HEADER2_RGB  = ('b' | ((uint16_t)'C'<<8));  // "bC"
    
    if (IS_WRITE) {
        *(p_buf++) = HEADER1;
        *(p_buf++) = is_rgb ? HEADER2_RGB : HEADER2_gray;
        *(p_buf++) = (uint16_t) height;
        *(p_buf++) = (uint16_t) width;
        *(p_buf++) = (uint16_t)(crc32>>16);
        *(p_buf++) = (uint16_t) crc32;
        *(p_buf++) = (uint16_t)(1 & is_large);
        
    } else {
        height = width = 0;
        if (*(p_buf++) != HEADER1)
            return NULL;                     // failed
        switch (*(p_buf++)) {
            case HEADER2_gray : is_rgb = false;  break;
            case HEADER2_RGB  : is_rgb = true;   break;
            default           : return NULL;  // failed
        }
        height = *(p_buf++);
        width  = *(p_buf++);
        crc32  = *(p_buf++);
        crc32<<= 16;
        crc32 |= *(p_buf++);
        is_large = 1 & *(p_buf++);
    }
    return p_buf;
}

#endif // __HEADER_H__
