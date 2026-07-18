#include <cstddef>
#include "CRC32.h"
#include "Header.h"
#include "SupportAVX2.h"
#include "fNBLIcodec.h"


#define    N_CTX     (N_QD * 256)


// Note: the image is divided into 2 parts: banner and body.
//       The height of banner = height - body, and at least 2. (process in serial)
//       The height of body is a multiple of 16.               (process in parallel)
#define    DIVIDE_BANNER(height,height_banner,height_body) { \
    (height_banner) = (height) & 0xF;                        \
    if ((height_banner) < 2)                                 \
        (height_banner) += 16;                               \
    (height_body) = (height) - (height_banner);              \
}


inline bool sizeInvalid (uint32_t height, uint32_t width) {
    return !(( 0< (height)) && ((height)<=32000) && (0 < (width)) && ((width)<=32000));
}

inline bool sizeLarge (uint32_t height, uint32_t width) {
    return ((18<=(height)) && (32<=(width)) && ((64*64)<=(height)*(width)));
}



//---------------------------------------------------------------------------------------------------------
// compressing
// return:  NULL     : failed
//          non-NULL : pointer to compressed data, need to be delete later
//---------------------------------------------------------------------------------------------------------
uint8_t *fNBLIcompress (size_t &comp_size, uint8_t *p_img, bool is_rgb, uint32_t height, uint32_t width, uint32_t &crc32) {
    if (sizeInvalid(height, width))
        return NULL;
    
    bool is_large = sizeLarge(height, width) && supportAVX2();
    
    size_t img_size = ((size_t)height * width * (is_rgb?3:1));
    size_t buf_size = img_size + (img_size>>4) + (1<<21);
    
    uint16_t *p_buf_base = new uint16_t [buf_size];
    uint16_t *p_buf      = p_buf_base;
    uint16_t *p_buf_end  = p_buf_base +  buf_size;
    
    if (crc32) crc32 = calculateCRC32(p_img, img_size);
    
    p_buf = rwHeader<true>(p_buf, height, width, is_rgb, is_large, crc32);
    
    PxCorrector<MAX_UV> pcU(N_CTX), pcV(N_CTX*2);
    PxCorrector<MAX_Y>  pcY(N_CTX*4);
    
    rANSe<N_QD*2> encoder (p_buf, p_buf_end);
    
    if (!is_large) {                           // small image, process in serial
        if (is_rgb)
            fNBLIcodec<true, true, rANSe<N_QD*2>> (encoder, pcY, pcU, pcV, p_img, height, width);
        else
            fNBLIcodec<false,true, rANSe<N_QD*2>> (encoder, pcY, pcU, pcV, p_img, height, width);
        
    } else {                                   // large image, process in parallel
        uint32_t height_banner, height_body;
        DIVIDE_BANNER(height, height_banner, height_body);
        
        uint8_t *p_img_body = p_img + ((size_t)height_banner * width * (is_rgb?3:1));
        
        if (is_rgb) {
            fNBLIcodec         <true, true, rANSe<N_QD*2>> (encoder, pcY, pcU, pcV, p_img,    height_banner, width);
            fNBLIcodec_x16_AVX2<true, true, rANSe<N_QD*2>> (encoder, pcY, pcU, pcV, p_img_body, height_body, width);
            
        } else {
            fNBLIcodec         <false,true, rANSe<N_QD*2>> (encoder, pcY, pcU, pcV, p_img   , height_banner, width);
            fNBLIcodec_x16_AVX2<false,true, rANSe<N_QD*2>> (encoder, pcY, pcU, pcV, p_img_body, height_body, width);
        }
    }
    
    p_buf = encoder.encode_all();
    
    comp_size = 2 * (p_buf-p_buf_base);  // length in bytes = length in words * 2
    
    return (uint8_t*)p_buf_base;
}



//---------------------------------------------------------------------------------------------------------
// decompressing
// return:  NULL     : failed
//          non-NULL : pointer to image pixels, need to be deleted later
//---------------------------------------------------------------------------------------------------------
uint8_t *fNBLIdecompress (uint8_t *p_buf, bool &is_rgb, uint32_t &height, uint32_t &width, uint32_t &crc32) {
    if (1 & (size_t)p_buf)       // buffer must align to 2bytes
        return NULL;
    
    uint16_t *p_u16 = (uint16_t*)p_buf;
    
    bool is_large;
    
    p_u16 = rwHeader<false>(p_u16, height, width, is_rgb, is_large, crc32);
    
    if (p_u16 == NULL)
        return NULL;
    
    if (sizeInvalid(height, width))
        return NULL;
    
    if (is_large && !supportAVX2())   // CPU do not support AVX2, but image need to be decompressed by AVX2
        return NULL;
    
    size_t img_size = ((size_t)height * width * (is_rgb?3:1));
    uint8_t  *p_img = new uint8_t [img_size];
    
    PxCorrector<MAX_UV> pcU(N_CTX), pcV(N_CTX*2);
    PxCorrector<MAX_Y>  pcY(N_CTX*4);
    
    rANSd<N_QD*2> decoder (p_u16);
    
    if (!is_large) {                           // small image, process in serial
        if (is_rgb)
            fNBLIcodec<true, false, rANSd<N_QD*2>> (decoder, pcY, pcU, pcV, p_img, height, width);
        else
            fNBLIcodec<false,false, rANSd<N_QD*2>> (decoder, pcY, pcU, pcV, p_img, height, width);
        
    } else {                                   // large image, process in parallel
        uint32_t height_banner, height_body;
        DIVIDE_BANNER(height, height_banner, height_body);
        
        uint8_t *p_img_body = p_img + ((size_t)height_banner * width * (is_rgb?3:1));
        
        if (is_rgb) {
            fNBLIcodec         <true, false, rANSd<N_QD*2>> (decoder, pcY, pcU, pcV, p_img,    height_banner, width);
            decoder.switch_mode_to_AVX2();
            fNBLIcodec_x16_AVX2<true, false, rANSd<N_QD*2>> (decoder, pcY, pcU, pcV, p_img_body, height_body, width);
            
        } else {
            fNBLIcodec         <false,false, rANSd<N_QD*2>> (decoder, pcY, pcU, pcV, p_img   , height_banner, width);
            decoder.switch_mode_to_AVX2();
            fNBLIcodec_x16_AVX2<false,false, rANSd<N_QD*2>> (decoder, pcY, pcU, pcV, p_img_body, height_body, width);
        }
    }
    
    if (crc32) {
        if (crc32 != calculateCRC32(p_img, img_size)) {
            delete[] p_img;
            return NULL;
        }
    }
    
    return p_img;
}

