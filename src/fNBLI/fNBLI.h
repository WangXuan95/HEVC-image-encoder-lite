#ifndef __FNBLI_H__
#define __FNBLI_H__

#include <cstdint>

//---------------------------------------------------------------------------------------------------------
// compressing
// return:  NULL     : failed
//          non-NULL : pointer to compressed data, need to be delete later
//---------------------------------------------------------------------------------------------------------
uint8_t *fNBLIcompress (size_t &comp_size, uint8_t *p_img, bool is_rgb, uint32_t height, uint32_t width, uint32_t &crc32);

//---------------------------------------------------------------------------------------------------------
// decompressing
// return:  NULL     : failed
//          non-NULL : pointer to image pixels, need to be deleted later
//---------------------------------------------------------------------------------------------------------
uint8_t *fNBLIdecompress (uint8_t *p_buf, bool &is_rgb, uint32_t &height, uint32_t &width, uint32_t &crc32);

#endif  // __FNBLI_H__
