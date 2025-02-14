/**
 * \file dnn/src/arm_common/pooling/do_pooling_5x5_nchw44.cpp
 * MegEngine is Licensed under the Apache License, Version 2.0 (the "License")
 *
 * Copyright (c) 2014-2021 Megvii Inc. All rights reserved.
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT ARRANTIES OR CONDITIONS OF ANY KIND, either express or
 * implied.
 */
#include "src/arm_common/pooling/do_pooling_5x5_nchw44.h"
#include "src/arm_common/pooling/algo.h"
#include "src/arm_common/simd_macro/marm_neon.h"
#include "src/common/unroll_macro.h"

namespace megdnn {
namespace arm_common {

void do_max_pooling_5x5_stride1_int8_nchw44_NEON(
        const int8_t* src, int8_t* dst, size_t IH, size_t IW, size_t OH, size_t OW,
        size_t PH, size_t PW, const WorkspaceBundle& ws) {
    const int8_t* sptr = nullptr;
    size_t IH2, IW2;
    sptr = handle_padding(src, IH, IW, IH2, IW2, PH, PW, ws, true);
    size_t oh = 0;
    for (; oh < OH; ++oh) {
        size_t ih = oh;
        const int8_t* sptr0 = sptr + (ih + 0) * IW2 * 4;
        const int8_t* sptr1 = sptr + (ih + 1) * IW2 * 4;
        const int8_t* sptr2 = sptr + (ih + 2) * IW2 * 4;
        const int8_t* sptr3 = sptr + (ih + 3) * IW2 * 4;
        const int8_t* sptr4 = sptr + (ih + 4) * IW2 * 4;
        int8_t* __restrict dptr = dst + oh * OW * 4;
        size_t ow = 0;
        for (; ow + 3 < OW; ow += 4) {
            int8x16_t src00, src04, max_out, max_tmp0, max_tmp1, max_tmp2, max_tmp3,
                    max_tmp4;
            int32x4_t src1234, src2345, src3456;

#define CACULATE_ROW(i)                                                               \
    src00 = vld1q_s8(sptr##i);                                                        \
    src04 = vld1q_s8(sptr##i + 4 * 4);                                                \
    src1234 = vextq_s32(vreinterpretq_s32_s8(src00), vreinterpretq_s32_s8(src04), 1); \
    src2345 = vextq_s32(vreinterpretq_s32_s8(src00), vreinterpretq_s32_s8(src04), 2); \
    src3456 = vextq_s32(vreinterpretq_s32_s8(src00), vreinterpretq_s32_s8(src04), 3); \
    max_tmp##i = vmaxq_s8(src00, vreinterpretq_s8_s32(src1234));                      \
    max_tmp##i = vmaxq_s8(max_tmp##i, vreinterpretq_s8_s32(src2345));                 \
    max_tmp##i = vmaxq_s8(max_tmp##i, vreinterpretq_s8_s32(src3456));                 \
    max_tmp##i = vmaxq_s8(max_tmp##i, src04);

            UNROLL_CALL_NOWRAPPER(5, CACULATE_ROW)
            max_out = vmaxq_s8(max_tmp0, max_tmp1);
            max_out = vmaxq_s8(max_out, max_tmp2);
            max_out = vmaxq_s8(max_out, max_tmp3);
            max_out = vmaxq_s8(max_out, max_tmp4);

            vst1q_s8(dptr, max_out);

            sptr0 += 16;
            sptr1 += 16;
            sptr2 += 16;
            sptr3 += 16;
            sptr4 += 16;
            dptr += 16;
#undef CACULATE_ROW
        }
        for (; ow < OW; ++ow) {
            int8x8_t src01, src23, max_out;

#define CACULATE_ROW(i)           \
    src01 = vld1_s8(sptr##i);     \
    src23 = vld1_s8(sptr##i + 8); \
    int8x8_t max_tmp##i = vmax_s8(src01, src23);

            UNROLL_CALL_NOWRAPPER(5, CACULATE_ROW)

            max_out = vmax_s8(max_tmp0, max_tmp1);
            max_out = vmax_s8(max_out, max_tmp2);
            max_out = vmax_s8(max_out, max_tmp3);
            max_out = vmax_s8(max_out, max_tmp4);

#define COMPARE_SRC45(i)    \
    int32x2_t src##i##_45 = \
            vld1_dup_s32(reinterpret_cast<const int32_t*>(sptr##i + 4 * 4));
            UNROLL_CALL_NOWRAPPER(5, COMPARE_SRC45)
            int8x8_t max_45 =
                    vmax_s8(vreinterpret_s8_s32(src0_45), vreinterpret_s8_s32(src1_45));
            max_45 = vmax_s8(max_45, vreinterpret_s8_s32(src1_45));
            max_45 = vmax_s8(max_45, vreinterpret_s8_s32(src2_45));
            max_45 = vmax_s8(max_45, vreinterpret_s8_s32(src3_45));
            max_45 = vmax_s8(max_45, vreinterpret_s8_s32(src4_45));

#define store(i) \
    *(dptr + i) = std::max(std::max(max_out[i], max_out[i + 4]), max_45[i]);
            UNROLL_CALL_NOWRAPPER(4, store)
#undef store
#undef COMPARE_SRC45
#undef CACULATE_ROW
            sptr0 += 4;
            sptr1 += 4;
            sptr2 += 4;
            sptr3 += 4;
            sptr4 += 4;
            dptr += 4;
        }
    }
}
void do_max_pooling_5x5_stride2_int8_nchw44_NEON(
        const int8_t* src, int8_t* dst, size_t IH, size_t IW, size_t OH, size_t OW,
        size_t PH, size_t PW, const WorkspaceBundle& ws) {
    const int8_t* sptr = nullptr;
    size_t IH2, IW2;
    sptr = handle_padding(src, IH, IW, IH2, IW2, PH, PW, ws, true);
    size_t oh = 0;
    for (; oh < OH; ++oh) {
        size_t ih = oh << 1;
        const int8_t* sptr0 = sptr + (ih + 0) * IW2 * 4;
        const int8_t* sptr1 = sptr + (ih + 1) * IW2 * 4;
        const int8_t* sptr2 = sptr + (ih + 2) * IW2 * 4;
        const int8_t* sptr3 = sptr + (ih + 3) * IW2 * 4;
        const int8_t* sptr4 = sptr + (ih + 4) * IW2 * 4;
        int8_t* __restrict dptr = dst + oh * OW * 4;
        size_t ow = 0;
        for (; ow + 3 < OW; ow += 4) {
            int8x16_t src00, src04, max_tmp0, max_tmp1, max_tmp2, max_tmp3, max_tmp4;
            int32x4_t src0246, src1357, src2468, src3579, src46810, src10, src09, src08;
            int32x4x2_t src_tmp;
#define CACULATE_ROW(i)                                                             \
    src00 = vld1q_s8(sptr##i);                                                      \
    src04 = vld1q_s8(sptr##i + 4 * 4);                                              \
    src08 = vld1q_dup_s32(reinterpret_cast<const int32_t*>(sptr##i + 4 * 8));       \
    src09 = vld1q_dup_s32(reinterpret_cast<const int32_t*>(sptr##i + 4 * 9));       \
    src10 = vld1q_dup_s32(reinterpret_cast<const int32_t*>(sptr##i + 4 * 10));      \
    src_tmp = vuzpq_s32(vreinterpretq_s32_s8(src00), vreinterpretq_s32_s8(src04));  \
    src0246 = src_tmp.val[0];                                                       \
    src1357 = src_tmp.val[1];                                                       \
    src2468 = vextq_s32(src0246, src08, 1);                                         \
    src3579 = vextq_s32(src1357, src09, 1);                                         \
    src46810 = vextq_s32(src2468, src10, 1);                                        \
    max_tmp##i =                                                                    \
            vmaxq_s8(vreinterpretq_s8_s32(src0246), vreinterpretq_s8_s32(src1357)); \
    max_tmp##i = vmaxq_s8(max_tmp##i, vreinterpretq_s8_s32(src2468));               \
    max_tmp##i = vmaxq_s8(max_tmp##i, vreinterpretq_s8_s32(src3579));               \
    max_tmp##i = vmaxq_s8(max_tmp##i, vreinterpretq_s8_s32(src46810));

            UNROLL_CALL_NOWRAPPER(5, CACULATE_ROW)

            int8x16_t max_out = vmaxq_s8(max_tmp0, max_tmp1);
            max_out = vmaxq_s8(max_out, max_tmp2);
            max_out = vmaxq_s8(max_out, max_tmp3);
            max_out = vmaxq_s8(max_out, max_tmp4);

            vst1q_s8(dptr, max_out);

            sptr0 += 32;
            sptr1 += 32;
            sptr2 += 32;
            sptr3 += 32;
            sptr4 += 32;
            dptr += 16;
#undef CACULATE_ROW
        }
        for (; ow < OW; ++ow) {
            int8x8_t src01, src23, max_out;

#define CACULATE_ROW(i)           \
    src01 = vld1_s8(sptr##i);     \
    src23 = vld1_s8(sptr##i + 8); \
    int8x8_t max_tmp##i = vmax_s8(src01, src23);

            UNROLL_CALL_NOWRAPPER(5, CACULATE_ROW)

            max_out = vmax_s8(max_tmp0, max_tmp1);
            max_out = vmax_s8(max_out, max_tmp2);
            max_out = vmax_s8(max_out, max_tmp3);
            max_out = vmax_s8(max_out, max_tmp4);

#define COMPARE_SRC45(i)    \
    int32x2_t src##i##_45 = \
            vld1_dup_s32(reinterpret_cast<const int32_t*>(sptr##i + 4 * 4));
            UNROLL_CALL_NOWRAPPER(5, COMPARE_SRC45)
            int8x8_t max_45 =
                    vmax_s8(vreinterpret_s8_s32(src0_45), vreinterpret_s8_s32(src1_45));
            max_45 = vmax_s8(max_45, vreinterpret_s8_s32(src1_45));
            max_45 = vmax_s8(max_45, vreinterpret_s8_s32(src2_45));
            max_45 = vmax_s8(max_45, vreinterpret_s8_s32(src3_45));
            max_45 = vmax_s8(max_45, vreinterpret_s8_s32(src4_45));

#define store(i) \
    *(dptr + i) = std::max(std::max(max_out[i], max_out[i + 4]), max_45[i]);
            UNROLL_CALL_NOWRAPPER(4, store)
#undef store
#undef COMPARE_SRC45
#undef CACULATE_ROW
            sptr0 += 8;
            sptr1 += 8;
            sptr2 += 8;
            sptr3 += 8;
            sptr4 += 8;
            dptr += 4;
        }
    }
}

void do_avg_pooling_5x5_stride1_int8_nchw44_NEON(
        const int8_t* src, int8_t* dst, size_t IH, size_t IW, size_t OH, size_t OW,
        size_t PH, size_t PW, const WorkspaceBundle& ws) {
    int16_t filter_size = 25;
    const int8_t* sptr = nullptr;
    size_t IH2, IW2;
    sptr = handle_padding(src, IH, IW, IH2, IW2, PH, PW, ws, false);
    size_t oh = 0;
    for (; oh < OH; ++oh) {
        size_t ih = oh;
        const int8_t* __restrict sptr0 = sptr + (ih + 0) * IW2 * 4;
        const int8_t* __restrict sptr1 = sptr + (ih + 1) * IW2 * 4;
        const int8_t* __restrict sptr2 = sptr + (ih + 2) * IW2 * 4;
        const int8_t* __restrict sptr3 = sptr + (ih + 3) * IW2 * 4;
        const int8_t* __restrict sptr4 = sptr + (ih + 4) * IW2 * 4;
        int8_t* __restrict dptr = dst + oh * OW * 4;
        size_t ow = 0;
        for (; ow + 3 < OW; ow += 4) {
            int16x8_t src01, src23, src12, src34, src45, src56, src67;
            int16x8_t sum01 = vdupq_n_s16(0);
            int16x8_t sum23 = vdupq_n_s16(0);

#define CACULATE_ROW(i)                      \
    src01 = vmovl_s8(vld1_s8(sptr##i));      \
    src23 = vmovl_s8(vld1_s8(sptr##i + 8));  \
    src12 = vmovl_s8(vld1_s8(sptr##i + 4));  \
    src34 = vmovl_s8(vld1_s8(sptr##i + 12)); \
    src45 = vmovl_s8(vld1_s8(sptr##i + 16)); \
    src56 = vmovl_s8(vld1_s8(sptr##i + 20)); \
    src67 = vmovl_s8(vld1_s8(sptr##i + 24)); \
    sum01 = vaddq_s16(sum01, src01);         \
    sum01 = vaddq_s16(sum01, src12);         \
    sum01 = vaddq_s16(sum01, src23);         \
    sum01 = vaddq_s16(sum01, src34);         \
    sum01 = vaddq_s16(sum01, src45);         \
    sum23 = vaddq_s16(sum23, src23);         \
    sum23 = vaddq_s16(sum23, src34);         \
    sum23 = vaddq_s16(sum23, src45);         \
    sum23 = vaddq_s16(sum23, src56);         \
    sum23 = vaddq_s16(sum23, src67);

            UNROLL_CALL_NOWRAPPER(5, CACULATE_ROW)

#define sum_define(i) int16_t sum##i;
            UNROLL_CALL_NOWRAPPER(8, sum_define)

#define sum01_avg(i)                                                            \
    sum##i = vgetq_lane_s16(sum01, i) > 0                                       \
                   ? (vgetq_lane_s16(sum01, i) + filter_size / 2) / filter_size \
                   : (vgetq_lane_s16(sum01, i) - filter_size / 2) / filter_size;
#define sum23_avg(i)                                                            \
    sum##i = vgetq_lane_s16(sum23, i) > 0                                       \
                   ? (vgetq_lane_s16(sum23, i) + filter_size / 2) / filter_size \
                   : (vgetq_lane_s16(sum23, i) - filter_size / 2) / filter_size;
#define store_sum01(i) *(dptr + i) = static_cast<int8_t>(sum##i);
#define store_sum23(i) *(dptr + i + 8) = static_cast<int8_t>(sum##i);

            UNROLL_CALL_NOWRAPPER(8, sum01_avg)
            UNROLL_CALL_NOWRAPPER(8, store_sum01)

            UNROLL_CALL_NOWRAPPER(8, sum23_avg)
            UNROLL_CALL_NOWRAPPER(8, store_sum23)

            sptr0 += 16;
            sptr1 += 16;
            sptr2 += 16;
            sptr3 += 16;
            sptr4 += 16;
            dptr += 16;

#undef store_sum01
#undef store_sum23
#undef sum01_avg
#undef sum23_avg
#undef sum_define
#undef CACULATE_ROW
        }
        for (; ow < OW; ++ow) {
            int32x2_t src004;
            int8x8_t src001, src023;
            int16x8_t src01, src23, src04, sum01, sum02;
            sum01 = vdupq_n_s16(0);
            sum02 = vdupq_n_s16(0);

#define CACULATE_ROW(i)                                                       \
    src001 = vld1_s8(sptr##i);                                                \
    src023 = vld1_s8(sptr##i + 8);                                            \
    src004 = vld1_dup_s32(reinterpret_cast<const int32_t*>(sptr##i + 4 * 4)); \
    src01 = vmovl_s8(src001);                                                 \
    src23 = vmovl_s8(src023);                                                 \
    src04 = vmovl_s8(vreinterpret_s8_s32(src004));                            \
    sum01 = vaddq_s16(sum01, src01);                                          \
    sum01 = vaddq_s16(sum01, src23);                                          \
    sum02 = vaddq_s16(sum02, src04);

            UNROLL_CALL_NOWRAPPER(5, CACULATE_ROW)

#define do_acc(i)                                                              \
    int16_t sum##i = vgetq_lane_s16(sum01, i) + vgetq_lane_s16(sum01, i + 4) + \
                     vgetq_lane_s16(sum02, i + 4);
#define do_avg(i)                                                  \
    sum##i = sum##i > 0 ? (sum##i + filter_size / 2) / filter_size \
                        : (sum##i - filter_size / 2) / filter_size;
#define store(i) *(dptr + i) = static_cast<int8_t>(sum##i);

            UNROLL_CALL_NOWRAPPER(4, do_acc)
            UNROLL_CALL_NOWRAPPER(4, do_avg)
            UNROLL_CALL_NOWRAPPER(4, store)

#undef store
#undef do_avg
#undef do_acc
#undef CACULATE_ROW
            sptr0 += 4;
            sptr1 += 4;
            sptr2 += 4;
            sptr3 += 4;
            sptr4 += 4;
            dptr += 4;
        }
    }
}

void do_avg_pooling_5x5_stride2_int8_nchw44_NEON(
        const int8_t* src, int8_t* dst, size_t IH, size_t IW, size_t OH, size_t OW,
        size_t PH, size_t PW, const WorkspaceBundle& ws) {
    int16_t filter_size = 25;
    const int8_t* sptr = nullptr;
    size_t IH2, IW2;
    sptr = handle_padding(src, IH, IW, IH2, IW2, PH, PW, ws, false);
    size_t oh = 0;
    for (; oh < OH; ++oh) {
        size_t ih = oh << 1;
        const int8_t* __restrict sptr0 = sptr + (ih + 0) * IW2 * 4;
        const int8_t* __restrict sptr1 = sptr + (ih + 1) * IW2 * 4;
        const int8_t* __restrict sptr2 = sptr + (ih + 2) * IW2 * 4;
        const int8_t* __restrict sptr3 = sptr + (ih + 3) * IW2 * 4;
        const int8_t* __restrict sptr4 = sptr + (ih + 4) * IW2 * 4;
        int8_t* __restrict dptr = dst + oh * OW * 4;
        size_t ow = 0;
        for (; ow + 3 < OW; ow += 4) {
            int32x4x2_t src_tmp;
            int8x16_t src00, src04;
            int16x8_t src02, src13, src57, src24, src68, src35, src79, src46, src810;
            int32x4_t src08, src09, src10, src0246, src1357, src2468, src3579, src46810;
            int16x8_t sum01 = vdupq_n_s16(0);
            int16x8_t sum23 = vdupq_n_s16(0);

#define CACULATE_ROW(i)                                                            \
    src00 = vld1q_s8(sptr##i);                                                     \
    src04 = vld1q_s8(sptr##i + 4 * 4);                                             \
    src08 = vld1q_dup_s32(reinterpret_cast<const int32_t*>(sptr##i + 4 * 8));      \
    src09 = vld1q_dup_s32(reinterpret_cast<const int32_t*>(sptr##i + 4 * 9));      \
    src10 = vld1q_dup_s32(reinterpret_cast<const int32_t*>(sptr##i + 4 * 10));     \
    src_tmp = vuzpq_s32(vreinterpretq_s32_s8(src00), vreinterpretq_s32_s8(src04)); \
    src0246 = src_tmp.val[0];                                                      \
    src1357 = src_tmp.val[1];                                                      \
    src2468 = vextq_s32(src0246, src08, 1);                                        \
    src3579 = vextq_s32(src1357, src09, 1);                                        \
    src46810 = vextq_s32(src2468, src10, 1);                                       \
    src02 = vmovl_s8(vget_low_s8(vreinterpretq_s8_s32(src0246)));                  \
    src46 = vmovl_s8(vget_high_s8(vreinterpretq_s8_s32(src0246)));                 \
    src13 = vmovl_s8(vget_low_s8(vreinterpretq_s8_s32(src1357)));                  \
    src57 = vmovl_s8(vget_high_s8(vreinterpretq_s8_s32(src1357)));                 \
    src24 = vmovl_s8(vget_low_s8(vreinterpretq_s8_s32(src2468)));                  \
    src68 = vmovl_s8(vget_high_s8(vreinterpretq_s8_s32(src2468)));                 \
    src35 = vmovl_s8(vget_low_s8(vreinterpretq_s8_s32(src3579)));                  \
    src79 = vmovl_s8(vget_high_s8(vreinterpretq_s8_s32(src3579)));                 \
    src46 = vmovl_s8(vget_low_s8(vreinterpretq_s8_s32(src46810)));                 \
    src810 = vmovl_s8(vget_high_s8(vreinterpretq_s8_s32(src46810)));               \
    sum01 = vaddq_s16(sum01, src02);                                               \
    sum01 = vaddq_s16(sum01, src13);                                               \
    sum01 = vaddq_s16(sum01, src24);                                               \
    sum01 = vaddq_s16(sum01, src35);                                               \
    sum01 = vaddq_s16(sum01, src46);                                               \
    sum23 = vaddq_s16(sum23, src46);                                               \
    sum23 = vaddq_s16(sum23, src57);                                               \
    sum23 = vaddq_s16(sum23, src68);                                               \
    sum23 = vaddq_s16(sum23, src79);                                               \
    sum23 = vaddq_s16(sum23, src810);

            UNROLL_CALL_NOWRAPPER(5, CACULATE_ROW)

#define sum_define(i) int16_t sum##i;
            UNROLL_CALL_NOWRAPPER(8, sum_define)

#define sum01_avg(i)                                                            \
    sum##i = vgetq_lane_s16(sum01, i) > 0                                       \
                   ? (vgetq_lane_s16(sum01, i) + filter_size / 2) / filter_size \
                   : (vgetq_lane_s16(sum01, i) - filter_size / 2) / filter_size;
#define sum23_avg(i)                                                            \
    sum##i = vgetq_lane_s16(sum23, i) > 0                                       \
                   ? (vgetq_lane_s16(sum23, i) + filter_size / 2) / filter_size \
                   : (vgetq_lane_s16(sum23, i) - filter_size / 2) / filter_size;
#define store_sum01(i) *(dptr + i) = static_cast<int8_t>(sum##i);
#define store_sum23(i) *(dptr + i + 8) = static_cast<int8_t>(sum##i);

            UNROLL_CALL_NOWRAPPER(8, sum01_avg)
            UNROLL_CALL_NOWRAPPER(8, store_sum01)

            UNROLL_CALL_NOWRAPPER(8, sum23_avg)
            UNROLL_CALL_NOWRAPPER(8, store_sum23)

            sptr0 += 32;
            sptr1 += 32;
            sptr2 += 32;
            sptr3 += 32;
            sptr4 += 32;
            dptr += 16;

#undef store_sum01
#undef store_sum23
#undef sum01_avg
#undef sum23_avg
#undef sum_define
#undef CACULATE_ROW
        }
        for (; ow < OW; ++ow) {
            int32x2_t src004;
            int8x8_t src001, src023;
            int16x8_t src01, src23, src04, sum01, sum02;
            sum01 = vdupq_n_s16(0);
            sum02 = vdupq_n_s16(0);

#define CACULATE_ROW(i)                                                       \
    src001 = vld1_s8(sptr##i);                                                \
    src023 = vld1_s8(sptr##i + 8);                                            \
    src004 = vld1_dup_s32(reinterpret_cast<const int32_t*>(sptr##i + 4 * 4)); \
    src01 = vmovl_s8(src001);                                                 \
    src23 = vmovl_s8(src023);                                                 \
    src04 = vmovl_s8(vreinterpret_s8_s32(src004));                            \
    sum01 = vaddq_s16(sum01, src01);                                          \
    sum01 = vaddq_s16(sum01, src23);                                          \
    sum02 = vaddq_s16(sum02, src04);

            UNROLL_CALL_NOWRAPPER(5, CACULATE_ROW)

#define do_acc(i)                                                              \
    int16_t sum##i = vgetq_lane_s16(sum01, i) + vgetq_lane_s16(sum01, i + 4) + \
                     vgetq_lane_s16(sum02, i + 4);
#define do_avg(i)                                                  \
    sum##i = sum##i > 0 ? (sum##i + filter_size / 2) / filter_size \
                        : (sum##i - filter_size / 2) / filter_size;
#define store(i) *(dptr + i) = static_cast<int8_t>(sum##i);

            UNROLL_CALL_NOWRAPPER(4, do_acc)
            UNROLL_CALL_NOWRAPPER(4, do_avg)
            UNROLL_CALL_NOWRAPPER(4, store)

#undef store
#undef do_avg
#undef do_acc
#undef CACULATE_ROW
            sptr0 += 8;
            sptr1 += 8;
            sptr2 += 8;
            sptr3 += 8;
            sptr4 += 8;
            dptr += 4;
        }
    }
}

}  // namespace arm_common
}  // namespace megdnn
   // vim: syntax=cpp.doxygen
