#ifndef   __IMAGE_SAMPLER_H__
#define   __IMAGE_SAMPLER_H__

#include <cstdint>
#include "RGB_YUV.h"


template <bool IS_RGB>
class ImageSampler {
private:
    inline static void load (int16_t &Y, int16_t &U, int16_t &V, uint8_t *p_img, int16_t width, int16_t i, int16_t j) {
        if (IS_RGB) {
            uint8_t *p = p_img + 3 * (width*i + j);
            U = p[0];  //R
            Y = p[1];  //G
            V = p[2];  //B
            RGB2YUV(Y, U, V);
        } else {
            uint8_t *p = p_img + (width*i + j);
            Y = p[0];
        }
    }
    
    inline static void store (int16_t &Y, int16_t &U, int16_t &V, uint8_t *p_img, int16_t width, int16_t i, int16_t j) {
        if (IS_RGB) {
            uint8_t *p = p_img + 3 * (width*i + j);
            int16_t R=U, G=Y, B=V;
            YUV2RGB(G, R, B);
            p[0] = R;
            p[1] = G;
            p[2] = B;
        } else {
            uint8_t *p = p_img + (width*i + j);
            p[0] = Y;
        }
    }
    
    
    uint8_t *p_img;
    int16_t  height, width;
    int16_t  i, j;
    
public:
    inline bool next () {
        j ++;
        if (j >= width) {
            j = 0;
            i ++;
        }
        return (i < height);  // return true if not finished
    }
    
    
    inline void clearAtStartOfLine (int16_t &valueY, int16_t &valueU, int16_t &valueV) {
        if (j == 0) {
            valueY = valueU = valueV = 0;
        }
    }
    
    
    int16_t aY, bY, cY, dY, eY, fY, gY;
    int16_t aU, bU, cU, dU, eU, fU, gU;
    int16_t aV, bV, cV, dV, eV, fV, gV;
    
    
    inline ImageSampler (uint8_t *_p_img, int16_t _height, int16_t _width) {
        p_img  = _p_img;
        height = _height;
        width  = _width;
        i = j = 0;
        
        aY = bY = cY = dY = eY = fY = gY = MID_Y;
        aU = bU = cU = dU = eU = fU = gU = MID_UV;
        aV = bV = cV = dV = eV = fV = gV = MID_UV;
    }
    
    
    inline void loadPixels (int16_t &Y, int16_t &U, int16_t &V) {
        load(Y, U, V, p_img, width, i, j);
    }
    
    
    inline void storePixels (int16_t Y, int16_t U, int16_t V) {
        store(Y, U, V, p_img, width, i, j);
    }
    
    
    inline void getTemplates (int16_t xY, int16_t xU, int16_t xV) {
        if (j == 0) {
            if (i > 0) {
                load(bY, bU, bV, p_img, width, i-1, 0);
                fY = bY;   fU = bU;   fV = bV;
                if (i > 1)
                    load(fY, fU, fV, p_img, width, i-2, 0);
            }
            eY = aY = cY = bY;    eU = aU = cU = bU;    eV = aV = cV = bV;
        } else {
            eY = aY;    eU = aU;    eV = aV;
            aY = xY;    aU = xU;    aV = xV;
            cY = bY;    cU = bU;    cV = bV;
            bY = dY;    bU = dU;    bV = dV;
            fY = gY;    fU = gU;    fV = gV;
        }
        
        bool flag = (j+1 < width);
        
        if (i < 1) {
            dY = aY;    dU = aU;    dV = aV;
        } else if (flag) {
            load(dY, dU, dV, p_img, width, i-1 , j+1);
        }
        
        if (i < 2) {
            gY = dY;    gU = dU;    gV = dV;
        } else if (flag) {
            load(gY, gU, gV, p_img, width, i-2 , j+1);
        }
    }
};


#endif // __IMAGE_SAMPLER_X16_AVX2_H__
