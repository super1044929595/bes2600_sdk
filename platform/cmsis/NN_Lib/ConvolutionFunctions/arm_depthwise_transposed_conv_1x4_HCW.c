#include "arm_nnfunctions.h"
#include "arm_nnsupportfunctions.h"
#include <assert.h>

arm_status arm_depthwise_transposed_conv_1x4_q15_q7_q31_out_q15(const q15_t *Im_in,
                                                         const uint16_t dim_im_in_x,
                                                         const uint16_t dim_im_in_y,
                                                         const uint16_t ch_im_in,
                                                         const q7_t *wt,
                                                         const uint16_t ch_im_out,
                                                         const uint16_t dim_kernel_x,
                                                         const uint16_t dim_kernel_y,
                                                         const uint16_t padding_x,
                                                         const uint16_t padding_y,
                                                         const uint16_t outpadding_x,
                                                         const uint16_t outpadding_y,
                                                         const uint16_t stride_x,
                                                         const uint16_t stride_y,
                                                         const q31_t *bias,
                                                         const uint16_t out_shift,
                                                         q15_t *Im_out,
                                                         const uint16_t dim_im_out_x,
                                                         const uint16_t dim_im_out_y)
{
    assert(ch_im_in == ch_im_out);
    assert(padding_x == 1 && padding_y == 0);
    assert(stride_x == 2 && stride_y == 1);
    assert(dim_kernel_x == 4 && dim_im_in_y == 1);
    assert(dim_im_in_y == 1 && dim_im_out_y == 1);
    assert(dim_im_out_x % 4 == 0);

    q31_t sum1, sum2, sum3, sum4;

    q31_t inM1, inM2;
    q31_t inV1, inV2;

    const q15_t *cur_pIn = Im_in;
    const q7_t *cur_pKer = wt;
    q15_t *pOut = Im_out;

    for (int ich = 0; ich < ch_im_in; ich++) {
        int32_t o_vale = bias ? bias[ich] + NN_ROUND(out_shift) : NN_ROUND(out_shift);

        inM1 = arm_nn_read_q7x4(cur_pKer);
        inM2 = __SXTB16(__ROR(inM1, 8));
        inM1 = __SXTB16(inM1);

        sum1 = cur_pIn[0] * cur_pKer[3] + o_vale;
        *pOut++ = (q15_t)__SSAT((sum1 >> out_shift), 16);

        inV1 = arm_nn_read_q15x2(cur_pIn);
        sum2 = __SMLAD(inV1, inM1, o_vale);
        sum3 = __SMLAD(inV1, inM2, o_vale);
        write_q15x2_ia(&pOut, __PKHBT(__SSAT((sum2 >> out_shift), 16),
                                      __SSAT((sum3 >> out_shift), 16), 16));
        cur_pIn++;

        uint16_t owCnt = (dim_im_out_x-4) >> 2;
        while (owCnt) {

            inV1 = arm_nn_read_q15x2(cur_pIn);
            sum1 = __SMLAD(inV1, inM1, o_vale); // 1
            sum2 = __SMLAD(inV1, inM2, o_vale); // 2
            write_q15x2_ia(&pOut, __PKHBT(__SSAT((sum1 >> out_shift), 16),
                                          __SSAT((sum2 >> out_shift), 16), 16));
            cur_pIn++;

            inV2 = arm_nn_read_q15x2(cur_pIn);
            sum3 = __SMLAD(inV2, inM1, o_vale); // 3
            sum4 = __SMLAD(inV2, inM2, o_vale); // 4
            write_q15x2_ia(&pOut, __PKHBT(__SSAT((sum3 >> out_shift), 16),
                                          __SSAT((sum4 >> out_shift), 16), 16));
            cur_pIn++;

            owCnt--;
        }

        sum4 = cur_pIn[0] * cur_pKer[0] + o_vale;
        *pOut++ = (q15_t)__SSAT((sum4 >> out_shift), 16);

        cur_pKer += 4;
        cur_pIn++;
    }
    return ARM_MATH_SUCCESS;
}

arm_status arm_depthwise_transposed_conv_1x4_q15_q7_q31_out_q31(const q15_t *Im_in,
                                                         const uint16_t dim_im_in_x,
                                                         const uint16_t dim_im_in_y,
                                                         const uint16_t ch_im_in,
                                                         const q7_t *wt,
                                                         const uint16_t ch_im_out,
                                                         const uint16_t dim_kernel_x,
                                                         const uint16_t dim_kernel_y,
                                                         const uint16_t padding_x,
                                                         const uint16_t padding_y,
                                                         const uint16_t outpadding_x,
                                                         const uint16_t outpadding_y,
                                                         const uint16_t stride_x,
                                                         const uint16_t stride_y,
                                                         const q31_t *bias,
                                                         const uint16_t out_shift,
                                                         q31_t *Im_out,
                                                         const uint16_t dim_im_out_x,
                                                         const uint16_t dim_im_out_y)
{
    assert(ch_im_in == ch_im_out);
    assert(padding_x == 1 && padding_y == 0);
    assert(stride_x == 2 && stride_y == 1);
    assert(dim_kernel_x == 4 && dim_im_in_y == 1);
    assert(dim_im_in_y == 1 && dim_im_out_y == 1);
    assert(dim_im_out_x % 4 == 0);

    q31_t inM1, inM2;
    q31_t inV1, inV2;

    const q15_t *cur_pIn = Im_in;
    const q7_t *cur_pKer = wt;
    q31_t *pOut = Im_out;

    for (int ich = 0; ich < ch_im_in; ich++) {
        int32_t o_vale = bias ? bias[ich] : 0;

        inM1 = arm_nn_read_q7x4(cur_pKer);
        inM2 = __SXTB16(__ROR(inM1, 8));
        inM1 = __SXTB16(inM1);

        *pOut++ = cur_pIn[0] * cur_pKer[3] + o_vale;
        inV1 = arm_nn_read_q15x2(cur_pIn);
        *pOut++ = __SMLAD(inV1, inM1, o_vale); // 1
        *pOut++ = __SMLAD(inV1, inM2, o_vale); // 2
        cur_pIn++;

        uint16_t owCnt = (dim_im_out_x-4) >> 2;
        while (owCnt) {

            inV1 = arm_nn_read_q15x2(cur_pIn);
            *pOut++ = __SMLAD(inV1, inM1, o_vale); // 1
            *pOut++ = __SMLAD(inV1, inM2, o_vale); // 2
            cur_pIn++;

            inV2 = arm_nn_read_q15x2(cur_pIn);
            *pOut++ = __SMLAD(inV2, inM1, o_vale); // 1
            *pOut++ = __SMLAD(inV2, inM2, o_vale); // 2

            cur_pIn++;
            owCnt--;
        }

        *pOut++ = cur_pIn[0] * cur_pKer[0] + o_vale;
        cur_pKer += 4;
        cur_pIn++;
    }
    return ARM_MATH_SUCCESS;
}