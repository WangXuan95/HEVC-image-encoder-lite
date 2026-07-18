#ifndef __INTx16_AVX2_H__
#define __INTx16_AVX2_H__

#include <cstdint>
#include <immintrin.h>

#define _ALIGN_AVX2_  __attribute__((aligned(32)))



//----------------------------------------------------------------------------------------------------------------------
// 16 x int16_t type, realize parallelization using AVX2
//----------------------------------------------------------------------------------------------------------------------
class INT16x16_t {
public:
    __m256i vec;
    
    #define _RET_INT16x16_t_(vec1) { \
        INT16x16_t res;              \
        res.vec = (vec1);            \
        return res;                  \
    }
    
public:
    //-------------------------------------------------------------------------------------------------
    // assignment
    //-------------------------------------------------------------------------------------------------
    inline void operator= (const INT16x16_t& other) { vec = other.vec; }
    inline void operator= (const int16_t value    ) { vec = _mm256_set1_epi16(value); }
    
    
    //-------------------------------------------------------------------------------------------------
    // construction method
    //-------------------------------------------------------------------------------------------------
    inline INT16x16_t (                   ) {                }
    inline INT16x16_t (const int16_t value) { *this = value; }
    
    inline INT16x16_t (
      const int16_t v0 , const int16_t v1 , const int16_t v2 , const int16_t v3 ,
      const int16_t v4 , const int16_t v5 , const int16_t v6 , const int16_t v7 , 
      const int16_t v8 , const int16_t v9 , const int16_t v10, const int16_t v11, 
      const int16_t v12, const int16_t v13, const int16_t v14, const int16_t v15
    ) {
        vec = _mm256_set_epi16(v15, v14, v13, v12, v11, v10, v9, v8, v7, v6, v5, v4, v3, v2, v1, v0);
    }
    
    
    //-------------------------------------------------------------------------------------------------
    // from/to memory
    //-------------------------------------------------------------------------------------------------
    inline void from_array (const int16_t array []) {
        vec = _mm256_loadu_si256((__m256i*)array);
    }
    
    inline void from_aligned_array (const int16_t array [16]) {
        //assert((((size_t)array) % 32) == 0);
        vec = _mm256_load_si256((__m256i*)array);
    }
    
    inline void to_array (int16_t array [16]) const {
        _mm256_storeu_si256((__m256i*)array, vec);
    }
    
    inline void to_aligned_array (int16_t array [16]) const {
        //assert((((size_t)array) % 32) == 0);
        _mm256_store_si256((__m256i*)array, vec);
    }
    
    
    //-------------------------------------------------------------------------------------------------
    // indexing
    //-------------------------------------------------------------------------------------------------
    inline int16_t operator[] (const int8_t index) const {   // note: index must be constant in compiling
        return _mm256_extract_epi16(vec, index);
    }
    
    //inline int16_t extract (const int8_t index) const {
    //    __m256i tmp = _mm256_srli_si256(vec, ((index&0x7)<<1));
    //    if (index < 8) return _mm256_extract_epi16(tmp, 0);
    //    else           return _mm256_extract_epi16(tmp, 8);
    //}
    
    inline int16_t extract (const int8_t index) const {
        __m128i v_tmp;
        if (index < 8) {
            v_tmp = _mm256_castsi256_si128(vec);
        } else {
            v_tmp = _mm256_extracti128_si256(vec, 1);
        }
        int16_t i16 = index & 0x07;
        i16 <<= 1;
        i16 += ((i16+1)<<8);
        return _mm_extract_epi16(_mm_shuffle_epi8(v_tmp, _mm_set1_epi16(i16)), 0);
    }
    
    inline void replace_one_value (const int8_t index, const int16_t value) {
        vec = _mm256_insert_epi16(vec, value, index);
    }
    
    
    //-------------------------------------------------------------------------------------------------
    // arithematic (+ - * / %)
    //-------------------------------------------------------------------------------------------------
    inline INT16x16_t operator- () const {
        _RET_INT16x16_t_(_mm256_sub_epi16(_mm256_setzero_si256(), vec));
    }
    
    inline INT16x16_t operator+ (const INT16x16_t& other) const {
        _RET_INT16x16_t_(_mm256_add_epi16(vec, other.vec));
    }
    
    inline INT16x16_t operator- (const INT16x16_t& other) const {
        _RET_INT16x16_t_(_mm256_sub_epi16(vec, other.vec));
    }
    
    inline INT16x16_t operator* (const INT16x16_t& other) const {
        _RET_INT16x16_t_(_mm256_mullo_epi16(vec, other.vec));
    }
    
    inline INT16x16_t operator+ (const int16_t value) const {
        _RET_INT16x16_t_(_mm256_add_epi16(vec, _mm256_set1_epi16(value)));
    }
    
    inline INT16x16_t operator- (const int16_t value) const {
        _RET_INT16x16_t_(_mm256_sub_epi16(vec, _mm256_set1_epi16(value)));
    }
    
    inline INT16x16_t operator* (const int16_t value) const {
        _RET_INT16x16_t_(_mm256_mullo_epi16(vec, _mm256_set1_epi16(value)));
    }
    
    //-------------------------------------------------------------------------------------------------
    // local arithematic (+= -= *= /=)
    //-------------------------------------------------------------------------------------------------
    inline void local_negative () {
        vec = _mm256_sub_epi16(_mm256_setzero_si256(), vec);
    }
    
    inline void operator+= (const INT16x16_t& other) {
        vec = _mm256_add_epi16(vec, other.vec);
    }
    
    inline void operator-= (const INT16x16_t& other) {
        vec = _mm256_sub_epi16(vec, other.vec);
    }
    
    inline void operator*= (const INT16x16_t& other) {
        vec = _mm256_mullo_epi16(vec, other.vec);
    }
    
    inline void operator+= (const int16_t value) {
        vec = _mm256_add_epi16(vec, _mm256_set1_epi16(value));
    }
    
    inline void operator-= (const int16_t value) {
        vec = _mm256_sub_epi16(vec, _mm256_set1_epi16(value));
    }
    
    inline void operator*= (const int16_t value) {
        vec = _mm256_mullo_epi16(vec, _mm256_set1_epi16(value));
    }
    
    //-------------------------------------------------------------------------------------------------
    // relational operation (return a conditional vector)
    //-------------------------------------------------------------------------------------------------
    inline INT16x16_t operator== (const INT16x16_t& other) const {
        _RET_INT16x16_t_(_mm256_cmpeq_epi16(vec, other.vec));
    }
    
    inline INT16x16_t operator>  (const INT16x16_t& other) const {
        _RET_INT16x16_t_(_mm256_cmpgt_epi16(vec, other.vec));
    }
    
    inline INT16x16_t operator<  (const INT16x16_t& other) const {
        _RET_INT16x16_t_(_mm256_cmpgt_epi16(other.vec, vec));
    }
    
    inline INT16x16_t operator== (const int16_t value) const {
        _RET_INT16x16_t_(_mm256_cmpeq_epi16(vec, _mm256_set1_epi16(value)));
    }
    
    inline INT16x16_t operator>  (const int16_t value) const {
        _RET_INT16x16_t_(_mm256_cmpgt_epi16(vec, _mm256_set1_epi16(value)));
    }
    
    inline INT16x16_t operator<  (const int16_t value) const {
        _RET_INT16x16_t_(_mm256_cmpgt_epi16(_mm256_set1_epi16(value), vec));
    }
    
    //-------------------------------------------------------------------------------------------------
    // select by condition, regard *this as a conditional vector
    //-------------------------------------------------------------------------------------------------
    inline INT16x16_t select (const INT16x16_t& other1, const INT16x16_t& other0) const {
        _RET_INT16x16_t_(_mm256_blendv_epi8(other0.vec, other1.vec, vec));
    }
    
    inline void select_from (const INT16x16_t& cond, const INT16x16_t& other1, const INT16x16_t& other0) {
        vec = _mm256_blendv_epi8(other0.vec, other1.vec, cond.vec);
    }
    
    inline void set_if      (const INT16x16_t& cond, const INT16x16_t& other1) {
        vec = _mm256_blendv_epi8(vec, other1.vec, cond.vec);
    }
    
    inline void set_if      (const INT16x16_t& cond, const int16_t value) {
        vec = _mm256_blendv_epi8(vec, _mm256_set1_epi16(value), cond.vec);
    }
    
    inline void set_if_not  (const INT16x16_t& cond, const INT16x16_t& other0) {
        vec = _mm256_blendv_epi8(other0.vec, vec, cond.vec);
    }
    
    inline void set_if_not  (const INT16x16_t& cond, const int16_t value) {
        vec = _mm256_blendv_epi8(_mm256_set1_epi16(value), vec, cond.vec);
    }
    
    //-------------------------------------------------------------------------------------------------
    // bit-wise logical
    //-------------------------------------------------------------------------------------------------
    inline INT16x16_t operator~ () const {
        _RET_INT16x16_t_(_mm256_xor_si256(vec, _mm256_set1_epi16(-1)));
    }
    
    inline INT16x16_t operator& (const INT16x16_t& other) const {
        _RET_INT16x16_t_(_mm256_and_si256(vec, other.vec));
    }
    
    inline INT16x16_t operator| (const INT16x16_t& other) const {
        _RET_INT16x16_t_(_mm256_or_si256(vec, other.vec));
    }
    
    inline INT16x16_t operator^ (const INT16x16_t& other) const {
        _RET_INT16x16_t_(_mm256_xor_si256(vec, other.vec));
    }
    
    inline INT16x16_t operator<< (const int8_t value) const {
        _RET_INT16x16_t_(_mm256_slli_epi16(vec, value));
    }
    
    inline INT16x16_t operator>> (const int8_t value) const {
        _RET_INT16x16_t_(_mm256_srai_epi16(vec, value));
    }
    
    inline INT16x16_t logical_right_shift (const int8_t value) const {
        _RET_INT16x16_t_(_mm256_srli_epi16(vec, value));
    }
    
    inline void local_bitwise_not () {
        vec = _mm256_xor_si256(vec, _mm256_set1_epi16(-1));
    }
    
    inline void operator&= (const INT16x16_t& other) {
        vec = _mm256_and_si256(vec, other.vec);
    }
    
    inline void operator|= (const INT16x16_t& other) {
        vec = _mm256_or_si256(vec, other.vec);
    }
    
    inline void operator^= (const INT16x16_t& other) {
        vec = _mm256_xor_si256(vec, other.vec);
    }
    
    inline void operator<<= (const int8_t value) {
        vec = _mm256_slli_epi16(vec, value);
    }
    
    inline void operator>>= (const int8_t value) {
        vec = _mm256_srai_epi16(vec, value);
    }
    
    inline void local_logical_right_shift (const int8_t value) {
        vec = _mm256_srli_epi16(vec, value);
    }
    
    //-------------------------------------------------------------------------------------------------
    // abs, min, max, clip
    //-------------------------------------------------------------------------------------------------
    inline INT16x16_t abs () const {
        _RET_INT16x16_t_(_mm256_abs_epi16(vec));
    }
    
    inline INT16x16_t min (const INT16x16_t& other) const {
        _RET_INT16x16_t_(_mm256_min_epi16(vec, other.vec));
    }
    
    inline INT16x16_t min (const int16_t value) const {
        return min(INT16x16_t(value));
    }
    
    inline INT16x16_t max (const INT16x16_t& other) const {
        _RET_INT16x16_t_(_mm256_max_epi16(vec, other.vec));
    }
    
    inline INT16x16_t max (const int16_t value) const {
        return max(INT16x16_t(value));
    }
    
    inline INT16x16_t clip (const INT16x16_t& low, const INT16x16_t& hgh) const {
        return this->max(low).min(hgh);
    }
    
    inline INT16x16_t clip (const INT16x16_t& low, const int16_t hgh) const {
        return this->max(low).min(hgh);
    }
    
    inline INT16x16_t clip (const int16_t low, const INT16x16_t& hgh) const {
        return this->max(low).min(hgh);
    }
    
    inline INT16x16_t clip (const int16_t low, const int16_t hgh) const {
        return this->max(low).min(hgh);
    }
    
    inline void local_abs () {
        vec = _mm256_abs_epi16(vec);
    }
    
    inline void local_min (const INT16x16_t& other) {
        vec = _mm256_min_epi16(vec, other.vec);
    }
    
    inline void local_min (const int16_t value) {
        local_min(INT16x16_t(value));
    }
    
    inline void local_max (const INT16x16_t& other) {
        vec = _mm256_max_epi16(vec, other.vec);
    }
    
    inline void local_max (const int16_t value) {
        local_max(INT16x16_t(value));
    }
    
    inline void local_clip (const INT16x16_t& low, const INT16x16_t& hgh) {
        local_max(low);
        local_min(hgh);
    }
    
    inline void local_clip (const INT16x16_t& low, const int16_t hgh) {
        local_max(low);
        local_min(hgh);
    }
    
    inline void local_clip (const int16_t low, const INT16x16_t& hgh) {
        local_max(low);
        local_min(hgh);
    }
    
    inline void local_clip (const int16_t low, const int16_t hgh) {
        local_max(low);
        local_min(hgh);
    }
    
    //-------------------------------------------------------------------------------------------------
    // lookup from a lookup table
    //-------------------------------------------------------------------------------------------------
    inline INT16x16_t lookup_from (const int16_t lookup_table[]) const {
        __m256i tmp1 = _mm256_cvtepi16_epi32(_mm256_castsi256_si128(vec));
        __m256i tmp2 = _mm256_cvtepi16_epi32(_mm256_extracti128_si256(vec, 1));
        
        tmp1 = _mm256_i32gather_epi32((const int32_t *)lookup_table, tmp1, 2);
        tmp2 = _mm256_i32gather_epi32((const int32_t *)lookup_table, tmp2, 2);
        
        INT16x16_t res;
        res.vec = _mm256_set1_epi32(0xFFFF);
        tmp1 = _mm256_and_si256(tmp1, res.vec);
        tmp2 = _mm256_and_si256(tmp2, res.vec);
        
        res.vec = _mm256_packus_epi32(tmp1, tmp2);
        res.vec = _mm256_permute4x64_epi64(res.vec, _MM_SHUFFLE(3, 1, 2, 0));
        
        return res;
    }
    
    inline INT16x16_t lookup_from (const uint8_t lookup_table[]) const {
        __m256i tmp1 = _mm256_cvtepi16_epi32(_mm256_castsi256_si128(vec));
        __m256i tmp2 = _mm256_cvtepi16_epi32(_mm256_extracti128_si256(vec, 1));
        
        tmp1 = _mm256_i32gather_epi32((const int32_t *)lookup_table, tmp1, 1);
        tmp2 = _mm256_i32gather_epi32((const int32_t *)lookup_table, tmp2, 1);
        
        INT16x16_t res;
        res.vec = _mm256_set1_epi32(0xFF);
        tmp1 = _mm256_and_si256(tmp1, res.vec);
        tmp2 = _mm256_and_si256(tmp2, res.vec);
        
        res.vec = _mm256_packus_epi32(tmp1, tmp2);
        res.vec = _mm256_permute4x64_epi64(res.vec, _MM_SHUFFLE(3, 1, 2, 0));
        
        return res;
    }
    
    
    //-------------------------------------------------------------------------------------------------
    // shift as a whole 256 bit value
    //-------------------------------------------------------------------------------------------------
    inline INT16x16_t ring_shift_right () const {
        INT16x16_t res;
        __m256i tmp = _mm256_slli_si256(vec, 14);
                tmp = _mm256_permute4x64_epi64(tmp, _MM_SHUFFLE(1, 0, 3, 2));
            res.vec = _mm256_srli_si256(vec, 2);
            res.vec = _mm256_or_si256(res.vec, tmp);
        return res;
    }
    
    
    //-------------------------------------------------------------------------------------------------
    // get mask bits, each mask bit is from the highest bit of each uint8 value, total 32 bits
    //-------------------------------------------------------------------------------------------------
    inline uint32_t get_highest_bits_of_each_u8 () const {
        return (uint32_t)_mm256_movemask_epi8(vec);
    }
    
    
    //-------------------------------------------------------------------------------------------------
    // equal to:
    //   int align_by_mask_eq0 ( const int16_t mask[16] ) {
    //       int16_t data [16];
    //       int j = 0;
    //       for           (int i=0 ; i<16 ; i++) {
    //           if (mask[i] == 0) {
    //               data[i] = this.vec[j];
    //               j ++;
    //           } else if (mask[i] == 0xFFFF) {
    //               data[i] = 0;
    //           } else {
    //               undefined !!!!!!! not permitted
    //           }
    //       }
    //       copy data to this.vec;
    //       return j;
    //   }
    //-------------------------------------------------------------------------------------------------
    inline int align_by_mask_eq0 (const INT16x16_t &mask) {
        __m256i m, s, v1, v2;
        
        m  = _mm256_add_epi16(mask.vec, _mm256_set1_epi16(1));
        
        s  = _mm256_permute4x64_epi64(_mm256_packus_epi16(m, m), _MM_SHUFFLE(3, 1, 2, 0));  // now there are 16 * 8bit masks in low 128 bit of v, each mask in 8bits, and can only be 1 or 0
        
        s  = _mm256_add_epi8(s, _mm256_slli_si256(s, 1));   // prefix-sum
        s  = _mm256_add_epi8(s, _mm256_slli_si256(s, 2));   // prefix-sum
        s  = _mm256_add_epi8(s, _mm256_slli_si256(s, 4));   // prefix-sum
        s  = _mm256_add_epi8(s, _mm256_slli_si256(s, 8));   // prefix-sum
        
        int next_offset = _mm256_extract_epi8(s, 15);
        
        s  = _mm256_slli_si256(s, 1);
        
        v1 = _mm256_srli_epi16(_mm256_slli_epi16(this->vec, 8), 8);
        v2 = _mm256_srli_epi16(this->vec, 8);
        
        v1 = _mm256_permute4x64_epi64(_mm256_packus_epi16(v1, v1), _MM_SHUFFLE(3, 1, 2, 0));  // only valid in low 128bits
        v2 = _mm256_permute4x64_epi64(_mm256_packus_epi16(v2, v2), _MM_SHUFFLE(3, 1, 2, 0));  // only valid in low 128bits
        
        v1 = _mm256_shuffle_epi8(v1, s);
        v2 = _mm256_shuffle_epi8(v2, s);
        
        v1 = _mm256_cvtepu8_epi16(_mm256_castsi256_si128(v1));
        v2 = _mm256_cvtepu8_epi16(_mm256_castsi256_si128(v2));
        
        this->vec = _mm256_or_si256(_mm256_slli_epi16(v2, 8), v1);
        
        this->vec = _mm256_blendv_epi8(this->vec, m, mask.vec);
        
        return next_offset;
    }
};





//----------------------------------------------------------------------------------------------------------------------
// 16 x int32_t type, realize parallelization using AVX2
//----------------------------------------------------------------------------------------------------------------------
class INT32x16_t {
private:
    __m256i vec1, vec2;
    
public:
    //-------------------------------------------------------------------------------------------------
    // assignment
    //-------------------------------------------------------------------------------------------------
    inline void operator= (const INT32x16_t& other) {
        vec1 = other.vec1;
        vec2 = other.vec2;
    }
    
    inline void operator= (const int32_t value) {
        vec1 = _mm256_set1_epi32(value);
        vec2 = _mm256_set1_epi32(value);
    }
    
    
    //-------------------------------------------------------------------------------------------------
    // construction method
    //-------------------------------------------------------------------------------------------------
    inline INT32x16_t (                   ) {                }
    inline INT32x16_t (const int32_t value) { *this = value; }
    
    inline INT32x16_t (
      const int32_t v0 , const int32_t v1 , const int32_t v2 , const int32_t v3 ,
      const int32_t v4 , const int32_t v5 , const int32_t v6 , const int32_t v7 , 
      const int32_t v8 , const int32_t v9 , const int32_t v10, const int32_t v11, 
      const int32_t v12, const int32_t v13, const int32_t v14, const int32_t v15
    ) {
        vec1 = _mm256_set_epi32( v7,  v6,  v5,  v4,  v3,  v2, v1, v0);
        vec2 = _mm256_set_epi32(v15, v14, v13, v12, v11, v10, v9, v8);
    }
    
    
    //-------------------------------------------------------------------------------------------------
    // from/to memory
    //-------------------------------------------------------------------------------------------------
    inline void from_array (const int32_t array [16]) {
        vec1 = _mm256_loadu_si256((__m256i*)&array[0]);
        vec2 = _mm256_loadu_si256((__m256i*)&array[8]);
    }
    
    inline void from_aligned_array (const int32_t array [16]) {
        //assert((((size_t)array) % 32) == 0);
        vec1 = _mm256_load_si256((__m256i*)&array[0]);
        vec2 = _mm256_load_si256((__m256i*)&array[8]);
    }
    
    inline void to_array (int32_t array [16]) const {
        _mm256_storeu_si256((__m256i*)&array[0], vec1);
        _mm256_storeu_si256((__m256i*)&array[8], vec2);
    }
    
    inline void to_aligned_array (int32_t array [16]) const {
        //assert((((size_t)array) % 32) == 0);
        _mm256_store_si256((__m256i*)&array[0], vec1);
        _mm256_store_si256((__m256i*)&array[8], vec2);
    }
    
    
    //-------------------------------------------------------------------------------------------------
    // convert from/to INT16x16_t
    //-------------------------------------------------------------------------------------------------
    inline INT16x16_t  to_INT16x16 () const {
        INT16x16_t res;
        res.vec = _mm256_packs_epi32(vec1, vec2);
        res.vec = _mm256_permute4x64_epi64(res.vec, _MM_SHUFFLE(3, 1, 2, 0));
        return res;
    }
    
    inline void from_INT16x16  (const INT16x16_t &other) {
        vec1 = _mm256_cvtepi16_epi32(_mm256_castsi256_si128  (other.vec));
        vec2 = _mm256_cvtepi16_epi32(_mm256_extracti128_si256(other.vec, 1));
    }
    
    inline void from_UINT16x16 (const INT16x16_t &other) {
        vec1 = _mm256_cvtepu16_epi32(_mm256_castsi256_si128  (other.vec));
        vec2 = _mm256_cvtepu16_epi32(_mm256_extracti128_si256(other.vec, 1));
    }
    
    
    //-------------------------------------------------------------------------------------------------
    // arithematic (+ - * / %)
    //-------------------------------------------------------------------------------------------------
    inline INT32x16_t operator- () const {
        __m256i vecv = _mm256_setzero_si256();
        INT32x16_t res;
        res.vec1 = _mm256_sub_epi32(vecv, vec1);
        res.vec2 = _mm256_sub_epi32(vecv, vec2);
        return res;
    }
    
    inline INT32x16_t operator+ (const INT32x16_t& other) const {
        INT32x16_t res;
        res.vec1 = _mm256_add_epi32(vec1, other.vec1);
        res.vec2 = _mm256_add_epi32(vec2, other.vec2);
        return res;
    }
    
    inline INT32x16_t operator- (const INT32x16_t& other) const {
        INT32x16_t res;
        res.vec1 = _mm256_sub_epi32(vec1, other.vec1);
        res.vec2 = _mm256_sub_epi32(vec2, other.vec2);
        return res;
    }
    
    inline INT32x16_t operator* (const INT32x16_t& other) const {
        INT32x16_t res;
        res.vec1 = _mm256_mullo_epi32(vec1, other.vec1);
        res.vec2 = _mm256_mullo_epi32(vec2, other.vec2);
        return res;
    }
    
    inline INT32x16_t operator+ (const int32_t value) const {
        __m256i vecv = _mm256_set1_epi32(value);
        INT32x16_t res;
        res.vec1 = _mm256_add_epi32(vec1, vecv);
        res.vec2 = _mm256_add_epi32(vec2, vecv);
        return res;
    }
    
    inline INT32x16_t operator- (const int32_t value) const {
        __m256i vecv = _mm256_set1_epi32(value);
        INT32x16_t res;
        res.vec1 = _mm256_sub_epi32(vec1, vecv);
        res.vec2 = _mm256_sub_epi32(vec2, vecv);
        return res;
    }
    
    inline INT32x16_t operator* (const int32_t value) const {
        __m256i vecv = _mm256_set1_epi32(value);
        INT32x16_t res;
        res.vec1 = _mm256_mullo_epi32(vec1, vecv);
        res.vec2 = _mm256_mullo_epi32(vec2, vecv);
        return res;
    }
    
    
    //-------------------------------------------------------------------------------------------------
    // local arithematic (+= -= *= /=)
    //-------------------------------------------------------------------------------------------------
    inline void local_negative () {
        __m256i vecv = _mm256_setzero_si256();
        vec1 = _mm256_sub_epi32(vecv, vec1);
        vec2 = _mm256_sub_epi32(vecv, vec2);
    }
    
    inline void operator+= (const INT32x16_t& other) {
        vec1 = _mm256_add_epi32(vec1, other.vec1);
        vec2 = _mm256_add_epi32(vec2, other.vec2);
    }
    
    inline void operator-= (const INT32x16_t& other) {
        vec1 = _mm256_sub_epi32(vec1, other.vec1);
        vec2 = _mm256_sub_epi32(vec2, other.vec2);
    }
    
    inline void operator*= (const INT32x16_t& other) {
        vec1 = _mm256_mullo_epi32(vec1, other.vec1);
        vec2 = _mm256_mullo_epi32(vec2, other.vec2);
    }
    
    inline void operator+= (const int32_t value) {
        __m256i vecv = _mm256_set1_epi32(value);
        vec1 = _mm256_add_epi32(vec1, vecv);
        vec2 = _mm256_add_epi32(vec2, vecv);
    }
    
    inline void operator-= (const int32_t value) {
        __m256i vecv = _mm256_set1_epi32(value);
        vec1 = _mm256_sub_epi32(vec1, vecv);
        vec2 = _mm256_sub_epi32(vec2, vecv);
    }
    
    inline void operator*= (const int32_t value) {
        __m256i vecv = _mm256_set1_epi32(value);
        vec1 = _mm256_mullo_epi32(vec1, vecv);
        vec2 = _mm256_mullo_epi32(vec2, vecv);
    }
    
    
    //-------------------------------------------------------------------------------------------------
    // division, cannot use AVX2 to speedup, since there's no division operation in AVX2
    //-------------------------------------------------------------------------------------------------
    inline INT32x16_t div_u32_with_rem (const INT32x16_t &other, INT32x16_t &remainder) const {
        uint32_t array1 [16] _ALIGN_AVX2_;
        uint32_t array2 [16] _ALIGN_AVX2_;
        
        array1[ 0] = (uint32_t)_mm256_extract_epi32(this->vec1, 0) / (uint32_t)_mm256_extract_epi32(other.vec1, 0);
        array2[ 0] = (uint32_t)_mm256_extract_epi32(this->vec1, 0) % (uint32_t)_mm256_extract_epi32(other.vec1, 0);
        array1[ 1] = (uint32_t)_mm256_extract_epi32(this->vec1, 1) / (uint32_t)_mm256_extract_epi32(other.vec1, 1);
        array2[ 1] = (uint32_t)_mm256_extract_epi32(this->vec1, 1) % (uint32_t)_mm256_extract_epi32(other.vec1, 1);
        array1[ 2] = (uint32_t)_mm256_extract_epi32(this->vec1, 2) / (uint32_t)_mm256_extract_epi32(other.vec1, 2);
        array2[ 2] = (uint32_t)_mm256_extract_epi32(this->vec1, 2) % (uint32_t)_mm256_extract_epi32(other.vec1, 2);
        array1[ 3] = (uint32_t)_mm256_extract_epi32(this->vec1, 3) / (uint32_t)_mm256_extract_epi32(other.vec1, 3);
        array2[ 3] = (uint32_t)_mm256_extract_epi32(this->vec1, 3) % (uint32_t)_mm256_extract_epi32(other.vec1, 3);
        array1[ 4] = (uint32_t)_mm256_extract_epi32(this->vec1, 4) / (uint32_t)_mm256_extract_epi32(other.vec1, 4);
        array2[ 4] = (uint32_t)_mm256_extract_epi32(this->vec1, 4) % (uint32_t)_mm256_extract_epi32(other.vec1, 4);
        array1[ 5] = (uint32_t)_mm256_extract_epi32(this->vec1, 5) / (uint32_t)_mm256_extract_epi32(other.vec1, 5);
        array2[ 5] = (uint32_t)_mm256_extract_epi32(this->vec1, 5) % (uint32_t)_mm256_extract_epi32(other.vec1, 5);
        array1[ 6] = (uint32_t)_mm256_extract_epi32(this->vec1, 6) / (uint32_t)_mm256_extract_epi32(other.vec1, 6);
        array2[ 6] = (uint32_t)_mm256_extract_epi32(this->vec1, 6) % (uint32_t)_mm256_extract_epi32(other.vec1, 6);
        array1[ 7] = (uint32_t)_mm256_extract_epi32(this->vec1, 7) / (uint32_t)_mm256_extract_epi32(other.vec1, 7);
        array2[ 7] = (uint32_t)_mm256_extract_epi32(this->vec1, 7) % (uint32_t)_mm256_extract_epi32(other.vec1, 7);
        array1[ 8] = (uint32_t)_mm256_extract_epi32(this->vec2, 0) / (uint32_t)_mm256_extract_epi32(other.vec2, 0);
        array2[ 8] = (uint32_t)_mm256_extract_epi32(this->vec2, 0) % (uint32_t)_mm256_extract_epi32(other.vec2, 0);
        array1[ 9] = (uint32_t)_mm256_extract_epi32(this->vec2, 1) / (uint32_t)_mm256_extract_epi32(other.vec2, 1);
        array2[ 9] = (uint32_t)_mm256_extract_epi32(this->vec2, 1) % (uint32_t)_mm256_extract_epi32(other.vec2, 1);
        array1[10] = (uint32_t)_mm256_extract_epi32(this->vec2, 2) / (uint32_t)_mm256_extract_epi32(other.vec2, 2);
        array2[10] = (uint32_t)_mm256_extract_epi32(this->vec2, 2) % (uint32_t)_mm256_extract_epi32(other.vec2, 2);
        array1[11] = (uint32_t)_mm256_extract_epi32(this->vec2, 3) / (uint32_t)_mm256_extract_epi32(other.vec2, 3);
        array2[11] = (uint32_t)_mm256_extract_epi32(this->vec2, 3) % (uint32_t)_mm256_extract_epi32(other.vec2, 3);
        array1[12] = (uint32_t)_mm256_extract_epi32(this->vec2, 4) / (uint32_t)_mm256_extract_epi32(other.vec2, 4);
        array2[12] = (uint32_t)_mm256_extract_epi32(this->vec2, 4) % (uint32_t)_mm256_extract_epi32(other.vec2, 4);
        array1[13] = (uint32_t)_mm256_extract_epi32(this->vec2, 5) / (uint32_t)_mm256_extract_epi32(other.vec2, 5);
        array2[13] = (uint32_t)_mm256_extract_epi32(this->vec2, 5) % (uint32_t)_mm256_extract_epi32(other.vec2, 5);
        array1[14] = (uint32_t)_mm256_extract_epi32(this->vec2, 6) / (uint32_t)_mm256_extract_epi32(other.vec2, 6);
        array2[14] = (uint32_t)_mm256_extract_epi32(this->vec2, 6) % (uint32_t)_mm256_extract_epi32(other.vec2, 6);
        array1[15] = (uint32_t)_mm256_extract_epi32(this->vec2, 7) / (uint32_t)_mm256_extract_epi32(other.vec2, 7);
        array2[15] = (uint32_t)_mm256_extract_epi32(this->vec2, 7) % (uint32_t)_mm256_extract_epi32(other.vec2, 7);
        
        remainder.from_aligned_array((int32_t*)array2);
        INT32x16_t res;
        res.from_aligned_array((int32_t*)array1);
        return res;
    }
    
    
    //-------------------------------------------------------------------------------------------------
    // relational operation (return a conditional vector)
    //-------------------------------------------------------------------------------------------------
    inline INT32x16_t operator== (const INT32x16_t& other) const {
        INT32x16_t res;
        res.vec1 = _mm256_cmpeq_epi32(vec1, other.vec1);
        res.vec2 = _mm256_cmpeq_epi32(vec2, other.vec2);
        return res;
    }
    
    inline INT32x16_t operator>  (const INT32x16_t& other) const {
        INT32x16_t res;
        res.vec1 = _mm256_cmpgt_epi32(vec1, other.vec1);
        res.vec2 = _mm256_cmpgt_epi32(vec2, other.vec2);
        return res;
    }
    
    inline INT32x16_t operator<  (const INT32x16_t& other) const {
        INT32x16_t res;
        res.vec1 = _mm256_cmpgt_epi32(other.vec1, vec1);
        res.vec2 = _mm256_cmpgt_epi32(other.vec2, vec2);
        return res;
    }
    
    inline INT32x16_t operator== (const int32_t value) const {
        __m256i vecv = _mm256_set1_epi32(value);
        INT32x16_t res;
        res.vec1 = _mm256_cmpeq_epi32(vec1, vecv);
        res.vec2 = _mm256_cmpeq_epi32(vec2, vecv);
        return res;
    }
    
    inline INT32x16_t operator>  (const int32_t value) const {
        __m256i vecv = _mm256_set1_epi32(value);
        INT32x16_t res;
        res.vec1 = _mm256_cmpgt_epi32(vec1, vecv);
        res.vec2 = _mm256_cmpgt_epi32(vec2, vecv);
        return res;
    }
    
    inline INT32x16_t operator<  (const int32_t value) const {
        __m256i vecv = _mm256_set1_epi32(value);
        INT32x16_t res;
        res.vec1 = _mm256_cmpgt_epi32(vecv, vec1);
        res.vec2 = _mm256_cmpgt_epi32(vecv, vec2);
        return res;
    }
    
    //-------------------------------------------------------------------------------------------------
    // select by condition, regard *this as a conditional vector
    //-------------------------------------------------------------------------------------------------
    inline INT32x16_t select (const INT32x16_t& other1, const INT32x16_t& other0) const {
        INT32x16_t res;
        res.vec1 = _mm256_blendv_epi8(other0.vec1, other1.vec1, vec1);
        res.vec2 = _mm256_blendv_epi8(other0.vec2, other1.vec2, vec2);
        return res;
    }
    
    inline void select_from (const INT32x16_t& cond, const INT32x16_t& other1, const INT32x16_t& other0) {
        vec1 = _mm256_blendv_epi8(other0.vec1, other1.vec1, cond.vec1);
        vec2 = _mm256_blendv_epi8(other0.vec2, other1.vec2, cond.vec2);
    }
    
    inline void set_if      (const INT32x16_t& cond, const INT32x16_t& other1) {
        vec1 = _mm256_blendv_epi8(vec1, other1.vec1, cond.vec1);
        vec2 = _mm256_blendv_epi8(vec2, other1.vec2, cond.vec2);
    }
    
    inline void set_if      (const INT32x16_t& cond, const int32_t value) {
        __m256i vecv = _mm256_set1_epi32(value);
        vec1 = _mm256_blendv_epi8(vec1, vecv, cond.vec1);
        vec2 = _mm256_blendv_epi8(vec2, vecv, cond.vec2);
    }
    
    inline void set_if_not  (const INT32x16_t& cond, const INT32x16_t& other0) {
        vec1 = _mm256_blendv_epi8(other0.vec1, vec1, cond.vec1);
        vec2 = _mm256_blendv_epi8(other0.vec2, vec2, cond.vec2);
    }
    
    inline void set_if_not  (const INT32x16_t& cond, const int32_t value) {
        __m256i vecv = _mm256_set1_epi32(value);
        vec1 = _mm256_blendv_epi8(vecv, vec1, cond.vec1);
        vec2 = _mm256_blendv_epi8(vecv, vec2, cond.vec2);
    }
    
    //-------------------------------------------------------------------------------------------------
    // bit-wise logical
    //-------------------------------------------------------------------------------------------------
    inline INT32x16_t operator~ () const {
        INT32x16_t res;
        res.vec1 = _mm256_xor_si256(vec1, _mm256_set1_epi16(-1));
        res.vec2 = _mm256_xor_si256(vec2, _mm256_set1_epi16(-1));
        return res;
    }
    
    inline INT32x16_t operator& (const INT32x16_t& other) const {
        INT32x16_t res;
        res.vec1 = _mm256_and_si256(vec1, other.vec1);
        res.vec2 = _mm256_and_si256(vec2, other.vec2);
        return res;
    }
    
    inline INT32x16_t operator| (const INT32x16_t& other) const {
        INT32x16_t res;
        res.vec1 = _mm256_or_si256(vec1, other.vec1);
        res.vec2 = _mm256_or_si256(vec2, other.vec2);
        return res;
    }
    
    inline INT32x16_t operator^ (const INT32x16_t& other) const {
        INT32x16_t res;
        res.vec1 = _mm256_xor_si256(vec1, other.vec1);
        res.vec2 = _mm256_xor_si256(vec2, other.vec2);
        return res;
    }
    
    inline INT32x16_t operator<< (const int8_t value) const {
        INT32x16_t res;
        res.vec1 = _mm256_slli_epi32(vec1, value);
        res.vec2 = _mm256_slli_epi32(vec2, value);
        return res;
    }
    
    inline INT32x16_t operator>> (const int8_t value) const {
        INT32x16_t res;
        res.vec1 = _mm256_srai_epi32(vec1, value);
        res.vec2 = _mm256_srai_epi32(vec2, value);
        return res;
    }
    
    inline INT32x16_t logical_right_shift (const int8_t value) const {
        INT32x16_t res;
        res.vec1 = _mm256_srli_epi32(vec1, value);
        res.vec2 = _mm256_srli_epi32(vec2, value);
        return res;
    }
    
    inline void local_bitwise_not () {
        vec1 = _mm256_xor_si256(vec1, _mm256_set1_epi16(-1));
        vec2 = _mm256_xor_si256(vec2, _mm256_set1_epi16(-1));
    }
    
    inline void operator&= (const INT32x16_t& other) {
        vec1 = _mm256_and_si256(vec1, other.vec1);
        vec2 = _mm256_and_si256(vec2, other.vec2);
    }
    
    inline void operator|= (const INT32x16_t& other) {
        vec1 = _mm256_or_si256(vec1, other.vec1);
        vec2 = _mm256_or_si256(vec2, other.vec2);
    }
    
    inline void operator^= (const INT32x16_t& other) {
        vec1 = _mm256_xor_si256(vec1, other.vec1);
        vec2 = _mm256_xor_si256(vec2, other.vec2);
    }
    
    inline void operator<<= (const int8_t value) {
        vec1 = _mm256_slli_epi32(vec1, value);
        vec2 = _mm256_slli_epi32(vec2, value);
    }
    
    inline void operator>>= (const int8_t value) {
        vec1 = _mm256_srai_epi32(vec1, value);
        vec2 = _mm256_srai_epi32(vec2, value);
    }
    
    inline void local_logical_right_shift (const int8_t value) {
        vec1 = _mm256_srli_epi32(vec1, value);
        vec2 = _mm256_srli_epi32(vec2, value);
    }
    
    //-------------------------------------------------------------------------------------------------
    // lookup from a lookup table
    //-------------------------------------------------------------------------------------------------
    inline INT32x16_t mask_lookup_from (const uint8_t lookup_table[], const INT32x16_t &mask, const INT32x16_t &default_value) const {
        INT32x16_t res;
        
        res.vec1 = _mm256_mask_i32gather_epi32(default_value.vec1, (const int32_t *)lookup_table, vec1, mask.vec1, 1);
        res.vec2 = _mm256_mask_i32gather_epi32(default_value.vec2, (const int32_t *)lookup_table, vec2, mask.vec2, 1);
        
        __m256i vFF = _mm256_set1_epi32(0xFF);
        res.vec1 = _mm256_and_si256(res.vec1, vFF);
        res.vec2 = _mm256_and_si256(res.vec2, vFF);
        
        return res;
    }
    
    inline INT32x16_t mask_lookup_u32_from (const uint8_t lookup_table[], const INT32x16_t &mask, const INT32x16_t &default_value) const {
        INT32x16_t res;
        res.vec1 = _mm256_mask_i32gather_epi32(default_value.vec1, (const int32_t *)lookup_table, vec1, mask.vec1, 1);
        res.vec2 = _mm256_mask_i32gather_epi32(default_value.vec2, (const int32_t *)lookup_table, vec2, mask.vec2, 1);
        return res;
    }
    
    inline INT32x16_t lookup_from (const uint16_t lookup_table[]) const {
        INT32x16_t res;
        
        res.vec1 = _mm256_i32gather_epi32((const int32_t *)lookup_table, vec1, 2);
        res.vec2 = _mm256_i32gather_epi32((const int32_t *)lookup_table, vec2, 2);
        
        __m256i vFFFF = _mm256_set1_epi32(0xFFFF);
        
        res.vec1 = _mm256_and_si256(res.vec1, vFFFF);
        res.vec2 = _mm256_and_si256(res.vec2, vFFFF);
        
        return res;
    }
    
    inline INT32x16_t lookup_from (const int32_t lookup_table[]) const {
        INT32x16_t res;
        res.vec1 = _mm256_i32gather_epi32(lookup_table, vec1, 4);
        res.vec2 = _mm256_i32gather_epi32(lookup_table, vec2, 4);
        return res;
    }
    
    
    //-------------------------------------------------------------------------------------------------
    // get mask bits, each mask bit is from the highest bit of each uint16 value, total 32 bits
    //-------------------------------------------------------------------------------------------------
    inline uint32_t get_highest_bit_of_each_u16 () const {
        __m256i v;
        v = _mm256_packs_epi32(vec1, vec2);
        v = _mm256_permute4x64_epi64(v, _MM_SHUFFLE(3, 1, 2, 0));
        return (uint32_t)_mm256_movemask_epi8(v);
    }
};


#endif // __INTx16_AVX2_H__
