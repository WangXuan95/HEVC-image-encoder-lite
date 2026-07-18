#ifndef   __PX_CORRECTOR_H__
#define   __PX_CORRECTOR_H__

#include <cstdint>
#include <algorithm>
#include "INTx16_AVX2.h"


template <int16_t MAXVAL>
class PxCorrector {
private:
    const static int CTX_COEF  = 6;
    const static int CTX_SCALE = 7;
    
    inline static int16_t getSign (int16_t ctx) {
        return ((ctx) >> (CTX_SCALE-1)) & 1;
    }
    
    inline static INT16x16_t getSign_x16_AVX2 (INT16x16_t &ctx) {
        return (ctx >> (CTX_SCALE-1)) & 1;
    }
    
    inline static int16_t clip (int16_t x, int16_t a, int16_t b) {
        return ((x)<(a)) ? (a) : (((x)>(b)) ? (b) : (x));          // clip x between a~b
    }
    
    inline static int16_t correctPx (int16_t ctx, int16_t px, int16_t sign) {
        return clip((px + sign + (ctx >> CTX_SCALE)), 0, MAXVAL);
    }
    
    inline static INT16x16_t correctPx_x16_AVX2 (INT16x16_t &ctx, INT16x16_t &px, INT16x16_t &sign) {
        return (px + sign + (ctx >> CTX_SCALE)).clip(0, MAXVAL);
    }
    
    inline static int16_t updateContext (int16_t ctx, int16_t err) {
        int32_t ctx32 = ctx;
        int32_t err32 = err;
        ctx32  = ((ctx32<<CTX_COEF) - ctx32);
        ctx32 += (err32 << CTX_SCALE);
        ctx32 += ((1 << (CTX_COEF-1)) - 1);
        ctx32>>= CTX_COEF;
        return (int16_t)ctx32;
    }
    
    inline static void updateContext_x16_AVX2 (INT16x16_t &ctx, INT16x16_t &err) {
        INT32x16_t ctx32, err32;
        ctx32.from_INT16x16(ctx);
        err32.from_INT16x16(err);
        ctx32  = ((ctx32<<CTX_COEF) - ctx32);
        ctx32 += (err32 << CTX_SCALE);
        ctx32 += ((1 << (CTX_COEF-1)) - 1);
        ctx32>>= CTX_COEF;
        ctx = ctx32.to_INT16x16();
    }
    
    inline static void writeContext_x16_AVX2 (int16_t *array_ctx, INT16x16_t &adr, INT16x16_t &ctx) {
        array_ctx[ adr[ 0] ] = ctx[ 0];
        array_ctx[ adr[ 1] ] = ctx[ 1];
        array_ctx[ adr[ 2] ] = ctx[ 2];
        array_ctx[ adr[ 3] ] = ctx[ 3];
        array_ctx[ adr[ 4] ] = ctx[ 4];
        array_ctx[ adr[ 5] ] = ctx[ 5];
        array_ctx[ adr[ 6] ] = ctx[ 6];
        array_ctx[ adr[ 7] ] = ctx[ 7];
        array_ctx[ adr[ 8] ] = ctx[ 8];
        array_ctx[ adr[ 9] ] = ctx[ 9];
        array_ctx[ adr[10] ] = ctx[10];
        array_ctx[ adr[11] ] = ctx[11];
        array_ctx[ adr[12] ] = ctx[12];
        array_ctx[ adr[13] ] = ctx[13];
        array_ctx[ adr[14] ] = ctx[14];
        array_ctx[ adr[15] ] = ctx[15];
    }
    
    inline static int16_t mapXtoW (int16_t x, int16_t px, int16_t sign) {
        return (sign ? (x-px) : (px-x)) & MAXVAL;
    }
    
    inline static INT16x16_t mapXtoW_x16_AVX2 (INT16x16_t &x, INT16x16_t &px, INT16x16_t &sign) {
        return ((sign==0).select(px-x, x-px)) & MAXVAL;
    }
    
    inline static int16_t mapWtoX (int16_t w, int16_t px, int16_t sign) {
        return (sign ? (px+w) : (px-w)) & MAXVAL;
    }
    
    inline static INT16x16_t mapWtoX_x16_AVX2 (INT16x16_t &w, INT16x16_t &px, INT16x16_t &sign) {
        return ((sign==0).select(px-w, px+w)) & MAXVAL;
    }
    
    
    int16_t *array_ctx;
    
public:
    inline PxCorrector (int n_context) {
        array_ctx = new int16_t [n_context];
        for (int i=0; i<n_context; i++)
            array_ctx[i] = 0;
    }
    
    inline ~PxCorrector () {
        delete[] array_ctx;
    }
    
    template <bool IS_ENC>
    inline int16_t act (int16_t adr, int16_t px, int16_t &x, int16_t &w) {
        int16_t ctx = array_ctx[adr];
        int16_t sign= getSign(ctx);
        int16_t pxc = correctPx(ctx, px, sign);
        if (IS_ENC) {
            w       = mapXtoW(x, pxc, sign);
        } else {
            x       = mapWtoX(w, pxc, sign);
        }
        int16_t err = clip((x-px), -(MAXVAL/2), (MAXVAL/2));
        array_ctx[adr] = updateContext(ctx, err);
        return err;
    }
    
    template <bool IS_ENC>
    inline INT16x16_t act_x16_AVX2 (INT16x16_t &adr, INT16x16_t &px, INT16x16_t &x, INT16x16_t &w) {
        INT16x16_t ctx  = adr.lookup_from(array_ctx);
        INT16x16_t sign = getSign_x16_AVX2(ctx);
        INT16x16_t pxc  = correctPx_x16_AVX2(ctx, px, sign);
        if (IS_ENC) {
            w           = mapXtoW_x16_AVX2(x, pxc, sign);
        } else {
            x           = mapWtoX_x16_AVX2(w, pxc, sign);
        }
        INT16x16_t err  = (x - px).clip(-(MAXVAL/2), (MAXVAL/2));
        updateContext_x16_AVX2(ctx, err);
        writeContext_x16_AVX2(array_ctx, adr, ctx);
        return err;
    }
};

#endif // __PX_CORRECTOR_H__
