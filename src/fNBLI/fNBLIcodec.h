#ifndef   __FNBLI_CODEC_H__
#define   __FNBLI_CODEC_H__

#include "ImageSampler.h"
#include "ImageSampler_x16_AVX.h"
#include "ModelingFunctions.h"
#include "PxCorrector.h"
#include "rANS.h"


#define    N_QD      12


template <bool IS_RGB, bool IS_ENC, typename CODEC_T>
static void fNBLIcodec (
    CODEC_T             &codec,
    PxCorrector<MAX_Y>  &pcY,
    PxCorrector<MAX_UV> &pcU,
    PxCorrector<MAX_UV> &pcV,
    uint8_t *p_img, int16_t height, int16_t width
) {
    ImageSampler<IS_RGB> sp(p_img, height, width);
    
    int16_t errY=0, xY=MID_Y ;
    int16_t errU=0, xU=MID_UV;
    int16_t errV=0, xV=MID_UV;
    
    do {
        sp.getTemplates(xY, xU, xV);
        sp.clearAtStartOfLine(errY, errU, errV);
        
        if (IS_ENC) sp.loadPixels(xY, xU, xV);
        
        if (IS_RGB) {
            int16_t dvdh;
            int16_t px  = predict<MAX_UV>(dvdh, sp.aU, sp.bU, sp.cU, sp.dU, sp.eU, sp.fU, sp.gU);
            int16_t qd  = getQD          (dvdh, errU);
            int16_t adr = getContextAddr (sp.aU, sp.bU, sp.cU, sp.dU, sp.eU, sp.fU, px, qd);
            qd += N_QD;
            int16_t w;
            if (IS_ENC) {
                errU = pcU.act<IS_ENC>(adr, px, xU, w);
                codec.codec(qd, w);
            } else {
                codec.codec(qd, w);
                errU = pcU.act<IS_ENC>(adr, px, xU, w);
            }
            errV = (std::abs(errV) + std::abs(errU) + 1) >> 1;
        }
        
        if (IS_RGB) {
            int16_t dvdh;
            int16_t px  = predict<MAX_UV>(dvdh, sp.aV, sp.bV, sp.cV, sp.dV, sp.eV, sp.fV, sp.gV);
            int16_t qd  = getQD          (dvdh, errV);
            int16_t adr = getContextAddr (sp.aV, sp.bV, sp.cV, sp.dV, sp.eV, sp.fV, px, qd);
            adr = (adr<<1) | (errU>0);
            qd += N_QD;
            int16_t w;
            if (IS_ENC) {
                errV = pcV.act<IS_ENC>(adr, px, xV, w);
                codec.codec(qd, w);
            } else {
                codec.codec(qd, w);
                errV = pcV.act<IS_ENC>(adr, px, xV, w);
            }
            errY = (std::abs(errY) + std::abs(errU) + std::abs(errV) + 1) >> 1;
        }
        
        {
            int16_t dvdh;
            int16_t px  = predict<MAX_Y>(dvdh, sp.aY, sp.bY, sp.cY, sp.dY, sp.eY, sp.fY, sp.gY);
            int16_t qd  = getQD         (dvdh, errY);
            int16_t adr = getContextAddr(sp.aY, sp.bY, sp.cY, sp.dY, sp.eY, sp.fY, px, qd);
            if (IS_RGB) adr = (adr<<2) | (errU>0) | ((errV>0)<<1);
            int16_t w;
            if (IS_ENC) {
                errY = pcY.act<IS_ENC>(adr, px, xY, w);
                codec.codec(qd, w);
            } else {
                codec.codec(qd, w);
                errY = pcY.act<IS_ENC>(adr, px, xY, w);
            }
        }
        
        if (!IS_ENC) sp.storePixels(xY, xU, xV);
        
    } while (sp.next());
}



template <bool IS_RGB, bool IS_ENC, typename CODEC_T>
static void fNBLIcodec_x16_AVX2 (
    CODEC_T             &codec,
    PxCorrector<MAX_Y>  &pcY,
    PxCorrector<MAX_UV> &pcU,
    PxCorrector<MAX_UV> &pcV,
    uint8_t *p_img, int16_t height, int16_t width
) {
    INT16x16_t errY(0), xY(MID_Y) ;
    INT16x16_t errU(0), xU(MID_UV);
    INT16x16_t errV(0), xV(MID_UV);
    
    ImageSampler_x16_AVX<IS_RGB> sp(p_img, height, width);
    
    do {
        sp.getTemplates(xY, xU, xV);
        sp.clearAtStartOfLine(errY, errU, errV);
        
        if (IS_ENC) sp.loadPixels(xY, xU, xV);
        
        if (IS_RGB) {
            INT16x16_t dvdh;
            INT16x16_t px  = predict_x16_AVX2<MAX_UV>(dvdh, sp.aU, sp.bU, sp.cU, sp.dU, sp.eU, sp.fU, sp.gU);
            INT16x16_t qd  = getQD_x16_AVX2          (dvdh, errU);
            INT16x16_t adr = getContextAddr_x16_AVX2 (sp.aU, sp.bU, sp.cU, sp.dU, sp.eU, sp.fU, px, qd);
            qd += N_QD;
            INT16x16_t w;
            if (IS_ENC) {
                errU = pcU.act_x16_AVX2<IS_ENC>(adr, px, xU, w);
                codec.codec_x16_AVX2(qd, w);
            } else {
                codec.codec_x16_AVX2(qd, w);
                errU = pcU.act_x16_AVX2<IS_ENC>(adr, px, xU, w);
            }
            errV = (errV.abs() + errU.abs() + 1) >> 1;
        }
        
        if (IS_RGB) {
            INT16x16_t dvdh;
            INT16x16_t px  = predict_x16_AVX2<MAX_UV>(dvdh, sp.aV, sp.bV, sp.cV, sp.dV, sp.eV, sp.fV, sp.gV);
            INT16x16_t qd  = getQD_x16_AVX2          (dvdh, errV);
            INT16x16_t adr = getContextAddr_x16_AVX2 (sp.aV, sp.bV, sp.cV, sp.dV, sp.eV, sp.fV, px, qd);
            adr = (adr<<1) | (errU>0).logical_right_shift(15);
            qd += N_QD;
            INT16x16_t w;
            if (IS_ENC) {
                errV = pcV.act_x16_AVX2<IS_ENC>(adr, px, xV, w);
                codec.codec_x16_AVX2(qd, w);
            } else {
                codec.codec_x16_AVX2(qd, w);
                errV = pcV.act_x16_AVX2<IS_ENC>(adr, px, xV, w);
            }
            errY = (errY.abs() + errU.abs() + errV.abs() + 1) >> 1;
        }
        
        {
            INT16x16_t dvdh;
            INT16x16_t px  = predict_x16_AVX2<MAX_Y>(dvdh, sp.aY, sp.bY, sp.cY, sp.dY, sp.eY, sp.fY, sp.gY);
            INT16x16_t qd  = getQD_x16_AVX2         (dvdh, errY);
            INT16x16_t adr = getContextAddr_x16_AVX2(sp.aY, sp.bY, sp.cY, sp.dY, sp.eY, sp.fY, px, qd);
            if (IS_RGB)adr = (adr<<2) | (errU>0).logical_right_shift(15) | ((errV>0).logical_right_shift(15)<<1);
            INT16x16_t w;
            if (IS_ENC) {
                errY = pcY.act_x16_AVX2<IS_ENC>(adr, px, xY, w);
                codec.codec_x16_AVX2(qd, w);
            } else {
                codec.codec_x16_AVX2(qd, w);
                errY = pcY.act_x16_AVX2<IS_ENC>(adr, px, xY, w);
            }
        }
        
        if (!IS_ENC) sp.storePixels(xY, xU, xV);
        
    } while (sp.next());
}


#endif // __FNBLI_CODEC_H__
