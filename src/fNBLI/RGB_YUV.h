#ifndef __RGB_YUV_H__
#define __RGB_YUV_H__

#define    MAX_Y         255
#define    MID_Y         (MAX_Y >> 1)
#define    MAX_UV        511
#define    MID_UV        (MAX_UV >> 1)

template <typename T>
inline static void RGB2YUV (T&Y, T&U, T&V) {
    U -= Y;
    V -= Y;
    Y += (U + V + 2) >> 2;
    V -= U >> 2;
    V += MAX_Y;
    U += MAX_Y;
}

template <typename T>
inline static void YUV2RGB (T&Y, T&U, T&V) {
    U -= MAX_Y;
    V -= MAX_Y;
    V += U >> 2;
    Y -= (U + V + 2) >> 2;
    V += Y;
    U += Y;
}

#endif // __RGB_YUV_H__
