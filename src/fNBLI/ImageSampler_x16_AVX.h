#ifndef   __IMAGE_SAMPLER_X16_AVX2_H__
#define   __IMAGE_SAMPLER_X16_AVX2_H__

#include <cstdint>
#include "RGB_YUV.h"
#include "INTx16_AVX2.h"


template <bool IS_RGB>
class ImageSampler_x16_AVX {
private:
    const static int32_t MID_RGB = (MID_Y<<16) | (MID_Y<<8) | (MID_Y);
    
    inline static void store3 (int32_t value, uint8_t *p) {
        p[0] = value;
        p[1] = value >> 8;
        p[2] = value >> 16;
    }
    
    uint8_t   *p_img;
    int16_t    height, width;
    INT16x16_t i, j;
    
    inline void load_YUV (int16_t &Y, int16_t &U, int16_t &V, int16_t i, int16_t j) {
        if (IS_RGB) {
            uint8_t *p = p_img + 3*(width*i + j);
            U = p[0];  //R
            Y = p[1];  //G
            V = p[2];  //B
            RGB2YUV(Y, U, V);
        } else {
            uint8_t *p = p_img + (width*i + j);
            Y = p[0];  //G
        }
    }
    
    inline void initIJ () {
        int16_t array_i [16] _ALIGN_AVX2_;
        int16_t array_j [16] _ALIGN_AVX2_;
        array_i[15] = -16;
        array_j[15] = width;
        for (int k=14; k>=0; k--) {
            array_i[k] = array_i[k+1] + 1;
            array_j[k] = array_j[k+1] - 2;
        }
        array_i[15] = 0;
        array_j[15] = 0;
        i.from_aligned_array(array_i);
        j.from_aligned_array(array_j);
    }
    
public:
    inline bool next () {
        j += 1;
        INT16x16_t cond = (j == width);
        j.set_if(cond, 0);
        i.set_if(cond, i+16);
        return (i[0] < height);  // return true if not finished
    }
    
    
    inline void clearAtStartOfLine (INT16x16_t &valueY, INT16x16_t &valueU, INT16x16_t &valueV) {
        INT16x16_t flag = (j==0);
        valueY.set_if(flag, 0);
        if (IS_RGB) {
            valueU.set_if(flag, 0);
            valueV.set_if(flag, 0);
        }
    }
    
    
    INT16x16_t aY, bY, cY, dY, eY, fY, gY;
    INT16x16_t aU, bU, cU, dU, eU, fU, gU;
    INT16x16_t aV, bV, cV, dV, eV, fV, gV;
    
    
    inline ImageSampler_x16_AVX (uint8_t *_p_img, int16_t _height, int16_t _width) {
        p_img  = _p_img;
        height = _height;
        width  = _width;
        initIJ();
        
        aY=MID_Y;  bY=MID_Y;  cY=MID_Y;  dY=MID_Y;  eY=MID_Y;  fY=MID_Y;  gY=MID_Y; 
        aU=MID_UV; bU=MID_UV; cU=MID_UV; dU=MID_UV; eU=MID_UV; fU=MID_UV; gU=MID_UV;
        aV=MID_UV; bV=MID_UV; cV=MID_UV; dV=MID_UV; eV=MID_UV; fV=MID_UV; gV=MID_UV;
    }
    
    
    inline void loadPixels (INT16x16_t &Y, INT16x16_t &U, INT16x16_t &V) {
        INT32x16_t addr, j32, flag;
        flag.from_INT16x16( (i>(-1)) & (i<height) );  // flags: in image region
        addr.from_INT16x16(i);
        j32 .from_INT16x16(j);
        addr *= width;
        addr += j32;                                  // addr = (i*width + j)
        if (IS_RGB) {
            addr *= 3;
            INT32x16_t RGB = addr.mask_lookup_u32_from(p_img, flag, INT32x16_t(MID_RGB)); // fetch pixels from memory
            U = ( RGB      & 0xFF).to_INT16x16();  //R
            Y = ((RGB>>8 ) & 0xFF).to_INT16x16();  //G
            V = ((RGB>>16) & 0xFF).to_INT16x16();  //B
            RGB2YUV(Y, U, V);
        } else {
            Y = addr.mask_lookup_from(p_img, flag, INT32x16_t(MID_Y)).to_INT16x16();      // fetch pixels from memory
        }
    }
    
    
    inline void storePixels (INT16x16_t &Y, INT16x16_t &U, INT16x16_t &V) {
        INT16x16_t flag = (i>(-1)) & (i<height);  // flags: in image region
        INT32x16_t addr, j32;
        addr.from_INT16x16(i);
        j32 .from_INT16x16(j);
        addr *= width;
        addr += j32;                              // addr = (i*width + j)
        if (IS_RGB)
            addr *= 3;
        int32_t array_addr [16] _ALIGN_AVX2_;
        addr.to_aligned_array(array_addr);
        
        if (IS_RGB) {
            INT32x16_t vRGB, vB32;
            INT16x16_t R=U, G=Y, B=V;
            YUV2RGB(G, R, B);
            vRGB.from_UINT16x16(R | (G<<8));
            vB32.from_UINT16x16(B);
            vRGB |= vB32 << 16;
            
            int32_t RGB [16] _ALIGN_AVX2_;
            vRGB.to_aligned_array(RGB);
            
            if (flag[ 0]) store3(RGB[ 0], &p_img[array_addr[ 0]]);
            if (flag[ 1]) store3(RGB[ 1], &p_img[array_addr[ 1]]);
            if (flag[ 2]) store3(RGB[ 2], &p_img[array_addr[ 2]]);
            if (flag[ 3]) store3(RGB[ 3], &p_img[array_addr[ 3]]);
            if (flag[ 4]) store3(RGB[ 4], &p_img[array_addr[ 4]]);
            if (flag[ 5]) store3(RGB[ 5], &p_img[array_addr[ 5]]);
            if (flag[ 6]) store3(RGB[ 6], &p_img[array_addr[ 6]]);
            if (flag[ 7]) store3(RGB[ 7], &p_img[array_addr[ 7]]);
            if (flag[ 8]) store3(RGB[ 8], &p_img[array_addr[ 8]]);
            if (flag[ 9]) store3(RGB[ 9], &p_img[array_addr[ 9]]);
            if (flag[10]) store3(RGB[10], &p_img[array_addr[10]]);
            if (flag[11]) store3(RGB[11], &p_img[array_addr[11]]);
            if (flag[12]) store3(RGB[12], &p_img[array_addr[12]]);
            if (flag[13]) store3(RGB[13], &p_img[array_addr[13]]);
            if (flag[14]) store3(RGB[14], &p_img[array_addr[14]]);
            if (flag[15]) store3(RGB[15], &p_img[array_addr[15]]);
            
        } else {
            if (flag[ 0]) p_img[array_addr[ 0]] = Y[ 0];
            if (flag[ 1]) p_img[array_addr[ 1]] = Y[ 1];
            if (flag[ 2]) p_img[array_addr[ 2]] = Y[ 2];
            if (flag[ 3]) p_img[array_addr[ 3]] = Y[ 3];
            if (flag[ 4]) p_img[array_addr[ 4]] = Y[ 4];
            if (flag[ 5]) p_img[array_addr[ 5]] = Y[ 5];
            if (flag[ 6]) p_img[array_addr[ 6]] = Y[ 6];
            if (flag[ 7]) p_img[array_addr[ 7]] = Y[ 7];
            if (flag[ 8]) p_img[array_addr[ 8]] = Y[ 8];
            if (flag[ 9]) p_img[array_addr[ 9]] = Y[ 9];
            if (flag[10]) p_img[array_addr[10]] = Y[10];
            if (flag[11]) p_img[array_addr[11]] = Y[11];
            if (flag[12]) p_img[array_addr[12]] = Y[12];
            if (flag[13]) p_img[array_addr[13]] = Y[13];
            if (flag[14]) p_img[array_addr[14]] = Y[14];
            if (flag[15]) p_img[array_addr[15]] = Y[15];
        }
    }
    
    
private:
    inline void getTemplatesStartOfLine () {
        INT16x16_t sol = (j == 0);
        
        int32_t bits = sol.get_highest_bits_of_each_u8();
        int8_t k;
        for (k=0; k<16; k++) {
            if (bits & 1)
                break;
            bits >>= 2;
        }
        
        if (k < 16) {
            int16_t ik = i.extract(k);
            
            int16_t bYk, dYk, fYk, gYk;    bYk = dYk = fYk = gYk = MID_Y;
            int16_t bUk, dUk, fUk, gUk;    bUk = dUk = fUk = gUk = MID_UV;
            int16_t bVk, dVk, fVk, gVk;    bVk = dVk = fVk = gVk = MID_UV;
            
            if (ik < height) {   // this function only call when encode/decode the body, since needn't handle the case of i[k]<=1
                load_YUV(bYk, bUk, bVk, ik-1, 0);
                load_YUV(dYk, dUk, dVk, ik-1, 1);
                load_YUV(fYk, fUk, fVk, ik-2, 0);
                load_YUV(gYk, gUk, gVk, ik-2, 1);
            }
            
            aY.set_if(sol, bYk);
            bY.set_if(sol, bYk);
            cY.set_if(sol, bYk);
            dY.set_if(sol, dYk);
            eY.set_if(sol, bYk);
            fY.set_if(sol, fYk);
            gY.set_if(sol, gYk);
            
            if (IS_RGB) {
                aU.set_if(sol, bUk);    aV.set_if(sol, bVk);
                bU.set_if(sol, bUk);    bV.set_if(sol, bVk);
                cU.set_if(sol, bUk);    cV.set_if(sol, bVk);
                dU.set_if(sol, dUk);    dV.set_if(sol, dVk);
                eU.set_if(sol, bUk);    eV.set_if(sol, bVk);
                fU.set_if(sol, fUk);    fV.set_if(sol, fVk);
                gU.set_if(sol, gUk);    gV.set_if(sol, gVk);
            }
        }
    }
    
    
public:
    inline void getTemplates (INT16x16_t &xY, INT16x16_t &xU, INT16x16_t &xV) {
        eY = aY;
        aY = xY;
        cY = bY;
        bY = dY;
        fY = gY;
        
        if (IS_RGB) {
            eU = aU;    eV = aV;
            aU = xU;    aV = xV;
            cU = bU;    cV = bV;
            bU = dU;    bV = dV;
            fU = gU;    fV = gV;
        }
        
        INT16x16_t flag(-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,0);
        flag = flag & (j<(width-1));
        
        gY.set_if(flag, cY.ring_shift_right());
        dY.set_if(flag, aY.ring_shift_right());
        
        if (IS_RGB) {
            gU.set_if(flag, cU.ring_shift_right());
            dU.set_if(flag, aU.ring_shift_right());
            gV.set_if(flag, cV.ring_shift_right());
            dV.set_if(flag, aV.ring_shift_right());
        }
        
        getTemplatesStartOfLine();
        
        if (i[15] < height && j[15] < width-1) {   // this function only call when encode/decode the body, since needn't handle the case of i[k]<=1
            int16_t Y, U, V;
            
            load_YUV(Y, U, V, i[15]-2, j[15]+1);
            gY.replace_one_value(15, Y);
            if (IS_RGB) {
                gU.replace_one_value(15, U);
                gV.replace_one_value(15, V);
            }
            
            load_YUV(Y, U, V, i[15]-1, j[15]+1);
            dY.replace_one_value(15, Y);
            if (IS_RGB) {
                dU.replace_one_value(15, U);
                dV.replace_one_value(15, V);
            }
        }
    }
};


#endif // __IMAGE_SAMPLER_X16_AVX2_H__
