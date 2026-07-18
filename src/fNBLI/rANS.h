#ifndef __R_ANS_H__
#define __R_ANS_H__

#include <cstdint>
#include "INTx16_AVX2.h"


#define    NORM_BIT    14
#define    NORM_SUM    (1 << NORM_BIT)
#define    NORM_MASK   (NORM_SUM - 1)
#define    CODE_BIT    16
#define    CODE_LOW    (1 << CODE_BIT)
#define    CODE_HIGH   ((1 << (2*CODE_BIT-NORM_BIT)) - 1)



template <int VAL_BIT>
class Histogram {
private :
    union HistItem_t {
        struct {
            uint16_t hnrm;   // normalized histogram value
            uint16_t hsum;   // prefix-sum of histogram
        };
        uint32_t count;      // raw histogram (to count the input), need to be normalized later
    };                       // note: {count} share space with {hsum,value}
    
    HistItem_t hist [1<<VAL_BIT];
    
public :
    inline Histogram () {
        static_assert(2<=VAL_BIT && VAL_BIT<=10);
        static_assert(9<=NORM_BIT && NORM_BIT<=14);
        for (int i=0; i<(1<<VAL_BIT); i++)
            hist[i].count = 0;
    }
    
    inline void add (int16_t i) {
        hist[i].count ++;
    }
    
    inline void get (int16_t i, int16_t &_hnrm, int16_t &_hsum) {
        HistItem_t item = hist[i];
        _hnrm = item.hnrm;
        _hsum = item.hsum;
    }
    
    inline void normalize () {              // normalize histogram, let its sum be NORM_SUM
        int nz_count = 0, nz_index = 0;
        uint32_t sum = 0;
        
        for (int i=0; i<(1<<VAL_BIT); i++) {
            if (hist[i].count > 0) {
                sum += hist[i].count;
                nz_count ++;     // how many non-zero values ?
                nz_index = i;    // a non-zero value
            }
        }
        
        if (nz_count <= 1) {
            hist[ nz_index].count = NORM_SUM - 1;
            hist[!nz_index].count = 1;
        
        } else {
            double scale = (double)NORM_SUM / sum;  // this func is only used in encoder, so using floating-point types here will not result in data being unable to be decoded across platforms
            
            sum = 0;
            
            for (int i=0; i<(1<<VAL_BIT); i++) {
                if (hist[i].count > 0) {
                    hist[i].count = (uint32_t)(0.49 + scale * hist[i].count);
                    if (hist[i].count == 0) hist[i].count = 1;
                    sum += hist[i].count;
                }
            }
            
            for (int i=0; sum>NORM_SUM; i=(i+1)&((1<<VAL_BIT)-1)) {
                if (hist[i].count > 1) {
                    hist[i].count --;
                    sum --;
                }
            }
            
            for (int i=0; sum<NORM_SUM; i=(i+1)&((1<<VAL_BIT)-1)) {
                if (hist[i].count > 0) {
                    hist[i].count ++;
                    sum ++;
                }
            }
        }
        
        for (int i=0; i<(1<<VAL_BIT); i++) {
            hist[i].hnrm = hist[i].count;
            hist[i].hsum = 0;
        }
    }
    
    inline void calculateAccumlate () {
        hist[0].hsum = 0;
        for (int i=1; i<(1<<VAL_BIT); i++) {
            hist[i].hsum = hist[i-1].hsum + hist[i-1].hnrm;
        }
    }
    
    inline void calculateDecodeLookupTable (uint16_t dlut[NORM_SUM]) {
        for (int i=0; i<(1<<VAL_BIT)-1; i++) {
            for (int j=hist[i].hsum; j<hist[i+1].hsum; j++) {
                dlut[j] = i;
            }
        }
        for (int j=hist[(1<<VAL_BIT)-1].hsum; j<NORM_SUM; j++) {
            dlut[j] = (1<<VAL_BIT)-1;
        }
    }
    
    // encode a normalized histogram to a uint16_t stream
    // use a simple compression code:
    //         |  a 16-bit code   | explain                                                               |
    //   code1 | 00AAAAAAAAAAAAAA | where AAAAAAAAAAAAAAA is a 14-bit histogram value                     |
    //   code2 | 01BBBBBBBCCCCCCC | where BBBBBBB and CCCCCCC are two 7-bit histogram deltas              |
    //   code3 | 10DDDDDDDEEEEEEE | where DDDDDDD and EEEEEEE are two 7-bit histogram values              |
    //   code4 | 110FFFFFGGGGHHHH | where FFFFF, GGGG, and HHHH are three 5-bit or 4-bit histogram values |
    //   code5 | 1110IIIJJJKKKLLL | where III, JJJ, KKK, and LLL are four 3-bit histogram values          |
    //   code6 | 1111MMRRRRRRRRRR | repeat MM for RRRRRRRRRR+1 times                                      |
    // note: only need to decode hist[1~(1<<VAL_BIT)-1]. The decoder calculates hist[0] = NORM_SUM - sum(hist[1~(1<<VAL_BIT)-1])
    inline uint16_t* encode (uint16_t *p_buf) {
        int16_t hprev = 0;
        for (int i=1; i<(1<<VAL_BIT); ) {
            int16_t h0 = hist[i].hnrm;
            int len;
            for (len=1; i+len<(1<<VAL_BIT) && h0==hist[i+len].hnrm; len++);
            if (len > 3 && h0 < 4) {
                *(p_buf++) = (h0<<10) | (len-1) | 0xF000;
                i += len;
            } else {
                int16_t h1 = (i < (1<<VAL_BIT)-1) ? hist[i+1].hnrm : INT16_MAX;
                int16_t h2 = (i < (1<<VAL_BIT)-2) ? hist[i+2].hnrm : INT16_MAX;
                int16_t h3 = (i < (1<<VAL_BIT)-3) ? hist[i+3].hnrm : INT16_MAX;
                int16_t d0 =                    (hprev-h0) & NORM_MASK;
                int16_t d1 = (i < (1<<VAL_BIT)-1) ? ((h0 - h1) & NORM_MASK) : INT16_MAX;
                
                if               (h0<8   && h1<8   && h2<8   && h3<8) {
                    *(p_buf++) = (h0<<9) | (h1<<6) | (h2<<3) |  h3 | 0xE000;
                    i += 4;
                } else if        (h0<32  && h1<16  && h2<16) {
                    *(p_buf++) = (h0<<8) | (h1<<4) |  h2           | 0xC000;
                    i += 3;
                } else if        (h0<128 && h1<128) {
                    *(p_buf++) = (h0<<7) |  h1                     | 0x8000;
                    i += 2;
                } else if        (d0<128&& d1<128) {
                    *(p_buf++) = (d0<<7) |  d1                     | 0x4000;
                    i += 2;
                } else {
                    *(p_buf++) =  h0;
                    i += 1;
                }
            }
            hprev = hist[i-1].hnrm;
        }
        return p_buf;
    }
    
    inline uint16_t* decode (uint16_t *p_buf) {          // decode a normalized histogram from a uint16_t stream
        for (uint16_t i=0; i<(1<<VAL_BIT); i++) hist[i].hnrm = 0;
        for (uint16_t i=1; i<(1<<VAL_BIT); ) {
            uint16_t code = *(p_buf++);
            switch (code >> 12) {
                case 0:  case 1:  case 2:  case 3:              // code1
                    hist[i++].hnrm = code;
                    break;
                case 4:  case 5:  case 6:  case 7:              // code2
                    hist[i].hnrm = (hist[i-1].hnrm-((code>>7)&0x7F)) & NORM_MASK;
                    i++;
                    hist[i].hnrm = (hist[i-1].hnrm-( code    &0x7F)) & NORM_MASK;
                    i++;
                    break;
                case 8:  case 9:  case 10:  case 11:            // code3
                    hist[i++].hnrm = (code>>7) & 0x7F;
                    hist[i++].hnrm = (code   ) & 0x7F;
                    break;
                case 12: case 13:                               // code4
                    hist[i++].hnrm = (code>>8) & 0x1F;
                    hist[i++].hnrm = (code>>4) & 0x0F;
                    hist[i++].hnrm = (code   ) & 0x0F;
                    break;
                case 14:                                        // code5
                    hist[i++].hnrm = (code>>9) & 0x07;
                    hist[i++].hnrm = (code>>6) & 0x07;
                    hist[i++].hnrm = (code>>3) & 0x07;
                    hist[i++].hnrm = (code   ) & 0x07;
                    break;
                default:                                        // code6
                    uint16_t hrep = (code>>10) & 0x003;
                    int      len  = (code    ) & ((1<<VAL_BIT)-1);
                    for (len++; len>0; len--) hist[i++].hnrm = hrep;
                    break;
            }
        }
        hist[0].hnrm = NORM_SUM;
        for (int i=1; i<(1<<VAL_BIT); i++) hist[0].hnrm -= hist[i].hnrm;
        return p_buf;
    }
};



//----------------------------------------------------------------------------------------------
// stack for context-value pairs
// This stack supports first pushing a series of single context-value pairs, then pushing a series of 16*(context-value pairs).
// You cannot interleave pushing single and 16 context-value pairs.
//   example usage:
//      call push();                         for N times
//      call push_x16();                     for M times
//      while (nempty_x16()) {               this loop will run M times
//          pop_x16();
//      }
//      while (nempty()) {                   this loop will run N times
//          pop();
//      }
//----------------------------------------------------------------------------------------------
class CtxValPairStack {
private:
    uint16_t *p_top;
    uint32_t  count1;
    uint32_t  count16;
    
public:
    CtxValPairStack () {}
    
    inline void init (uint16_t *_p_top) {
        p_top   = _p_top;
        count1  = 0;
        count16 = 0;
    }
    
    inline bool nempty     () { return count1  != 0; }
    
    inline bool nempty_x16 () { return count16 != 0; }
    
    inline void push (int16_t ctx, int16_t val) {
        count1 ++;
        p_top --;
        p_top[0] = (ctx<<10) | val;
    }
    
    inline void pop (int16_t &ctx, int16_t &val) {
        count1 --;
        ctx = 0x03F & (p_top[0]>>10);
        val = 0x3FF &  p_top[0];
        p_top ++;
    }
    
    inline void push_x16_AVX2 (INT16x16_t &v_ctx, INT16x16_t &v_val) {
        count16 ++;
        p_top -= 16;
        ((v_ctx<<10) | v_val).to_array((int16_t*)p_top);
    } 
    
    inline void pop_x16_AVX2 (INT32x16_t &v_ctx, INT32x16_t &v_val) {
        count16 --;
        INT16x16_t vtmp;
        vtmp.from_array((int16_t*)p_top);
        v_val.from_INT16x16(vtmp & 0x3FF);
        v_ctx.from_INT16x16(vtmp.logical_right_shift(10));
        p_top += 16;
    }  
};



template <int N_CTX>
class rANSe {
private :
    uint32_t        ans [16] _ALIGN_AVX2_;
    uint32_t       &ans0 = ans[0];
    uint16_t       *p_buf;
    Histogram<9>   *hists;
    CtxValPairStack stack;
    
    inline static void reverseStream (uint16_t *p_start, uint16_t *p_final) {
        for (p_final--; p_start<p_final; p_final--, p_start++) {
            uint16_t tmp = p_start[0];
            p_start[0] = p_final[0];
            p_final[0] = tmp;
        }
    }
    
    inline void encode (int16_t ctx, int16_t val) {
        int16_t hnrm, hsum;
        hists[ctx].get(val, hnrm, hsum);
        uint32_t nans = ans0 / hnrm;
        if (nans > CODE_HIGH) {
            *(p_buf++) = ans0;
            ans0 >>= CODE_BIT;
            nans = ans0 / hnrm;
        }
        ans0 %= hnrm;
        ans0 += (nans << NORM_BIT);
        ans0 += hsum;
    }
    
    inline void encode_x16_AVX2 (INT32x16_t &v_ctx, INT32x16_t &v_val) {
        INT32x16_t v_adr   = (v_ctx << 9) | v_val;
        
        INT32x16_t v_hnrm  = v_adr.lookup_from((int32_t*)hists);
        INT32x16_t v_hsum  = v_hnrm.logical_right_shift(16);
                   v_hnrm &= NORM_MASK;
        
        uint32_t tmp [16] _ALIGN_AVX2_;
        v_hnrm.to_aligned_array((int32_t*)tmp);
        for (int i=0; i<16; i++) tmp[i] = ans[i] / tmp[i];
        INT32x16_t v_nans;
        v_nans.from_aligned_array((int32_t*)tmp);
        
        INT32x16_t v_flag  = (v_nans > CODE_HIGH) | (v_nans >> 31);
        
        uint32_t flag = v_flag.get_highest_bit_of_each_u16();
        for (int i=15; i>=0; i--) {
            if (flag >> 31) *(p_buf++) = ans[i];
            flag <<= 2;
        }
        
        INT32x16_t v_ans;
        v_ans.from_aligned_array((int32_t*)ans);
        
        v_ans .set_if(v_flag, v_ans.logical_right_shift(CODE_BIT));
        v_nans.set_if(v_flag, v_ans.div_u32_with_rem(v_hnrm, v_ans));    
        v_ans += (v_nans << NORM_BIT);
        v_ans += v_hsum;
        
        v_ans.to_aligned_array((int32_t*)ans);
    }
    
    inline void encode_flush () {
        for (int i=15; i>=0; i--) {
            *(p_buf++) =  ans[i];
            *(p_buf++) = (ans[i]>>CODE_BIT);
        }
    }
    
public:
    // construct -----------------
    inline rANSe (uint16_t *_p_buf, uint16_t *_p_buf_end=NULL) {
        static_assert(1<=N_CTX && N_CTX<=64);
        p_buf = _p_buf;
        hists = new Histogram<9> [N_CTX];
        stack.init(_p_buf_end);
    }
    
    inline ~rANSe () {
        delete[] hists;
    }
    
    inline void codec (int16_t ctx, int16_t val) {
        hists[ctx].add(val);
        stack.push(ctx, val);
    }
    
    inline void codec_x16_AVX2 (INT16x16_t &v_ctx, INT16x16_t &v_val) {
        stack.push_x16_AVX2(v_ctx, v_val);
        hists[v_ctx[ 0]].add(v_val[ 0]);
        hists[v_ctx[ 1]].add(v_val[ 1]);
        hists[v_ctx[ 2]].add(v_val[ 2]);
        hists[v_ctx[ 3]].add(v_val[ 3]);
        hists[v_ctx[ 4]].add(v_val[ 4]);
        hists[v_ctx[ 5]].add(v_val[ 5]);
        hists[v_ctx[ 6]].add(v_val[ 6]);
        hists[v_ctx[ 7]].add(v_val[ 7]);
        hists[v_ctx[ 8]].add(v_val[ 8]);
        hists[v_ctx[ 9]].add(v_val[ 9]);
        hists[v_ctx[10]].add(v_val[10]);
        hists[v_ctx[11]].add(v_val[11]);
        hists[v_ctx[12]].add(v_val[12]);
        hists[v_ctx[13]].add(v_val[13]);
        hists[v_ctx[14]].add(v_val[14]);
        hists[v_ctx[15]].add(v_val[15]);
    }
    
    inline uint16_t *encode_all () {
        for (int ctx=0; ctx<N_CTX; ctx++) {
            hists[ctx].normalize();
            hists[ctx].calculateAccumlate();
            p_buf = hists[ctx].encode(p_buf);
        }
        
        uint16_t *p_buf_tmp = p_buf;
        
        for (int i=0; i<16; i++) ans[i] = CODE_LOW;
        
        while (stack.nempty_x16()) {
            INT32x16_t         v_ctx, v_val;
            stack.pop_x16_AVX2(v_ctx, v_val);
            encode_x16_AVX2   (v_ctx, v_val);
        }
        
        while (stack.nempty()) {
            int16_t   ctx, val;
            stack.pop(ctx, val);
            encode   (ctx, val);
        }
        
        encode_flush();
        
        reverseStream(p_buf_tmp, p_buf);
        
        //for (int i=0; i<4; i++)
        //    *(p_buf++) = 0;
        
        return p_buf;
    }
};



template <int N_CTX>
class rANSd {
private :
    INT32x16_t      v_ans;
    uint32_t        ans [16] _ALIGN_AVX2_;
    uint32_t       &ans0 = ans[0];
    uint16_t       *p_buf;
    Histogram<9>   *hists;
    uint16_t      (*dluts) [NORM_SUM];
    
public:
    inline void codec (int16_t ctx, int16_t &val) {
        int16_t hnrm, hsum;
        int16_t lb  = ans0 & NORM_MASK;
        val = dluts[ctx][lb];
        hists[ctx].get(val, hnrm, hsum);
        ans0>>= NORM_BIT;
        ans0 *= hnrm;
        ans0 += lb;
        ans0 -= hsum;
        if (ans0 < CODE_LOW) {
            ans0 <<= CODE_BIT;
            ans0  |= *(p_buf++);
        }
    }
    
    inline void switch_mode_to_AVX2 () {
        v_ans.from_aligned_array((int32_t*)ans);
    }
    
    inline void codec_x16_AVX2 (INT16x16_t &v16_ctx, INT16x16_t &v16_val) {
        INT32x16_t v_ctx;
        v_ctx.from_INT16x16(v16_ctx);
        INT32x16_t v_lb    = v_ans & NORM_MASK;
        INT32x16_t v_adr   = (v_ctx<<NORM_BIT) | v_lb;
        INT32x16_t v_val   = v_adr.lookup_from(dluts[0]);
                   v_ans.local_logical_right_shift(NORM_BIT);
                   v_adr   = (v_ctx<<9) | v_val;
        INT32x16_t v_hnrm  = v_adr.lookup_from((int32_t*)hists);
                   v_ans  *= (v_hnrm & NORM_MASK);
                   v_hnrm.local_logical_right_shift(16);
                   v_ans  += v_lb;
                   v_ans  -= v_hnrm;
        INT32x16_t v_flag  = (v_ans > (CODE_LOW-1)) | (v_ans >> 31);
                   v_ans.set_if_not(v_flag, (v_ans<<CODE_BIT));
        
        INT16x16_t v16_buf;
        v16_buf.from_array((int16_t*)p_buf);
        
        p_buf += v16_buf.align_by_mask_eq0(v_flag.to_INT16x16());
        
        INT32x16_t v32_buf;
        v32_buf.from_UINT16x16(v16_buf);
        v_ans |= v32_buf;
        
        v16_val = v_val.to_INT16x16();
    }
    
    
    // construct -----------------
    inline rANSd (uint16_t *_p_buf) {
        static_assert(1<=N_CTX && N_CTX<=64);
        p_buf = _p_buf;
        
        hists = new Histogram<9> [N_CTX];
        
        dluts = (uint16_t(*)[NORM_SUM]) new uint16_t [N_CTX*NORM_SUM];
        
        for (int ctx=0; ctx<N_CTX; ctx++) {
            p_buf = hists[ctx].decode(p_buf);
            hists[ctx].calculateAccumlate();
            hists[ctx].calculateDecodeLookupTable(dluts[ctx]);
        }
        
        for (int i=0; i<16; i++) {
            ans[i]  = (*(p_buf++)) << CODE_BIT;
            ans[i] |= (*(p_buf++));
        }
    }
    
    inline ~rANSd () {
        delete[] hists;
        delete[] dluts;
    }
};


#endif // __R_ANS_H__
