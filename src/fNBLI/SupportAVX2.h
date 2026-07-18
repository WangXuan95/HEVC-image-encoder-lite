#ifndef __SUPPORT_AVX2_H__
#define __SUPPORT_AVX2_H__

#ifdef   __linux__
    inline static bool supportAVX2 () {
        return __builtin_cpu_supports("avx2");
    }
#else
#ifdef   _WIN32
    #include <intrin.h>
    inline static bool supportAVX2 () {
        int cpuInfo[4];
        __cpuid(cpuInfo, 1);
        __cpuidex(cpuInfo, 7, 0);   // AVX2 is bit 5 of the EBX register when EAX = 7 and ECX = 0
        return (cpuInfo[1] & (1 << 5));
    }
#else
    static_assert(0, "platform must be linux or windows");
#endif
#endif

#endif // __SUPPORT_AVX2_H__
