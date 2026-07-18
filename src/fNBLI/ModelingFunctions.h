#ifndef   __MODELING_FUNCTIONS_H__
#define   __MODELING_FUNCTIONS_H__

#include <cstdint>
#include <algorithm>
#include "INTx16_AVX2.h"


template <int16_t MAXVAL>
inline static int16_t predict (int16_t &dvdh, int16_t a, int16_t b, int16_t c, int16_t d, int16_t e, int16_t f, int16_t g) {
    int16_t dh = std::abs(a-e) + std::abs(b-c) + std::abs(b-d);
    int16_t dv = std::abs(a-c) + std::abs(b-f) + std::abs(d-g);
    dvdh = dv + dh;
    int16_t dvh= dv - dh;
    int16_t ab = (dvh > 0) ? a : b;
    dvh = std::abs(dvh);
    int16_t px = (a+b)*9 + (d<<1) - (c<<1) - e - f;
    px = std::min(std::max(px, (int16_t)0), (int16_t)(16*MAXVAL));
    if      (dvh > 80)
        return ab;
    else if (dvh > 32)
        return (  px + (ab<<4) + 16) >> 5;
    else if (dvh >  8)
        return (3*px + (ab<<4) + 32) >> 6;
    else
        return (  px           +  8) >> 4;
}


template <int16_t MAXVAL>
inline static INT16x16_t predict_x16_AVX2 (INT16x16_t &dvdh, INT16x16_t &a, INT16x16_t &b, INT16x16_t &c, INT16x16_t &d, INT16x16_t &e, INT16x16_t &f, INT16x16_t &g) {
    INT16x16_t dh = (a-e).abs() + (b-c).abs() + (b-d).abs();
    INT16x16_t dv = (a-c).abs() + (b-f).abs() + (d-g).abs();
    dvdh = dh + dv;
    INT16x16_t dvh= dv - dh;
    INT16x16_t ab = (dvh>0).select(a, b);
    dvh.local_abs();
    INT16x16_t px  = ((a+b)*9 + (d<<1) - (c<<1) - e - f).clip(0, 16*MAXVAL);
    INT16x16_t pxa = (px   + (ab<<4) + 16) >> 5;
    INT16x16_t pxb = (px*3 + (ab<<4) + 32) >> 6;
               px  = (px             +  8) >> 4;
    px.set_if((dvh> 8), pxb);
    px.set_if((dvh>32), pxa);
    px.set_if((dvh>80), ab);
    return px;
}


inline static int16_t getQD (int16_t dvdh, int16_t err) {
    static uint8_t lut_qd [] = {0, 1, 2, 2, 3, 3, 4, 4, 4, 5, 5, 5, 5, 5, 5, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 11};  // look up this array to convert delta to quantize-delta (QD)
    int16_t qd = dvdh + (std::abs(err)<<1);
    qd = std::min(qd, (int16_t)(sizeof(lut_qd)/sizeof(*lut_qd)-1));
    return lut_qd[qd];
}


inline static INT16x16_t getQD_x16_AVX2 (INT16x16_t &dvdh, INT16x16_t &err) {
    static uint8_t lut_qd [] = {0, 1, 2, 2, 3, 3, 4, 4, 4, 5, 5, 5, 5, 5, 5, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 11};  // look up this array to convert delta to quantize-delta (QD)
    INT16x16_t qd = dvdh + (err.abs()<<1);
    qd.local_min((int16_t)(sizeof(lut_qd)/sizeof(*lut_qd)-1));
    return qd.lookup_from(lut_qd);
}


inline static int16_t getContextAddr (int16_t a, int16_t b, int16_t c, int16_t d, int16_t e, int16_t f, int16_t px, int16_t qd) {
    int16_t adr = qd;
    adr <<= 1;       adr |= (px > a);
    adr <<= 1;       adr |= (px > b);
    adr <<= 1;       adr |= (px > c);
    adr <<= 1;       adr |= (px > d);
    adr <<= 1;       adr |= (px > e);
    adr <<= 1;       adr |= (px > f);
    adr <<= 1;       adr |= (px > (2*a-e));
    adr <<= 1;       adr |= (px > (2*b-f));
    return adr;
}


inline static INT16x16_t getContextAddr_x16_AVX2 (INT16x16_t &a, INT16x16_t &b, INT16x16_t &c, INT16x16_t &d, INT16x16_t &e, INT16x16_t &f, INT16x16_t &px, INT16x16_t &qd) {
    INT16x16_t adr = qd << 1;
                     adr |= (px > a).logical_right_shift(15);
    adr <<= 1;       adr |= (px > b).logical_right_shift(15);
    adr <<= 1;       adr |= (px > c).logical_right_shift(15);
    adr <<= 1;       adr |= (px > d).logical_right_shift(15);
    adr <<= 1;       adr |= (px > e).logical_right_shift(15);
    adr <<= 1;       adr |= (px > f).logical_right_shift(15);
    adr <<= 1;       adr |= (px > ((a<<1)-e)).logical_right_shift(15);
    adr <<= 1;       adr |= (px > ((b<<1)-f)).logical_right_shift(15);
    return adr;
}

#endif // __MODELING_FUNCTIONS_H__
