#include "arm_nnfunctions.h"
#include "arm_nnsupportfunctions.h"
#include "dsp/matrix_functions.h"
#include <stdio.h>
#include <assert.h>

arm_status arm_convolve_1x1_HCW_q15_q7_q31_out_q31(const q15_t *Im_in,
                                             const uint16_t dim_im_in_x,
                                             const uint16_t dim_im_in_y,
                                             const uint16_t ch_im_in,
                                             const q7_t *wt,
                                             const uint16_t ch_im_out,
                                             const uint16_t dim_kernel_x,
                                             const uint16_t dim_kernel_y,
                                             const uint16_t padding_x,
                                             const uint16_t padding_y,
                                             const uint16_t stride_x,
                                             const uint16_t stride_y,
                                             const q31_t *bias,
                                             const uint16_t out_shift,
                                             q31_t *Im_out,
                                             const uint16_t dim_im_out_x,
                                             const uint16_t dim_im_out_y,
                                             q15_t *bufferA)
{
    // (void*)bufferA;
    assert(dim_im_in_y == 1 && dim_im_out_y == 1);
    assert(dim_kernel_x == 1 && dim_kernel_y == 1);
    assert(padding_x == 0 && padding_y == 0);
    assert(stride_x == 1 && stride_y == 1);
    assert(ch_im_in == 1 && dim_im_out_x % 4 == 0);

    q15_t *pIn = (q15_t*)Im_in;
    q31_t *pOut = Im_out;
    q7_t in_kernel;
    q31_t out_bias;

    uint16_t row_num = 0;
    uint16_t widthCnt = dim_im_out_x >> 2;
    while (widthCnt) {
        const q7_t *cur_wt = wt;
        const q31_t* pbias = bias;
        q31_t *cur_pOut = pOut + row_num;
        for(int och = 0; och < ch_im_out; och++) {
            const q15_t *pB1 = pIn + row_num*ch_im_in;
            const q15_t *pB2 = pB1 + ch_im_in;
            const q15_t *pB3 = pB2 + ch_im_in;
            const q15_t *pB4 = pB3 + ch_im_in;

            in_kernel = *cur_wt++;
            out_bias = *pbias++ + NN_ROUND(out_shift);

            cur_pOut[0] = *pB1 * in_kernel + out_bias;
            cur_pOut[1] = *pB2 * in_kernel + out_bias;
            cur_pOut[2] = *pB3 * in_kernel + out_bias;
            cur_pOut[3] = *pB4 * in_kernel + out_bias;
            cur_pOut += dim_im_out_x;
        }
        widthCnt--;
        row_num+=4;
    }
    return ARM_MATH_SUCCESS;
}


arm_status arm_convolve_1x1_HCW_q15_q7_q31_out_q31_fast(const q15_t *Im_in,
                                                const uint16_t dim_im_in_x,
                                                const uint16_t dim_im_in_y,
                                                const uint16_t ch_im_in,
                                                const q7_t *wt,
                                                const uint16_t ch_im_out,
                                                const uint16_t dim_kernel_x,
                                                const uint16_t dim_kernel_y,
                                                const uint16_t padding_x,
                                                const uint16_t padding_y,
                                                const uint16_t stride_x,
                                                const uint16_t stride_y,
                                                const q31_t *bias,
                                                const uint16_t out_shift,
                                                q31_t *Im_out,
                                                const uint16_t dim_im_out_x,
                                                const uint16_t dim_im_out_y,
                                                q15_t *bufferA)
{
    assert(dim_im_in_y == 1 && dim_im_out_y == 1);
    assert(dim_kernel_x == 1 && dim_kernel_y == 1);
    assert(padding_x == 0 && padding_y == 0);
    assert(stride_x == 1 && stride_y == 1);
    assert(ch_im_in % 4 == 0 && dim_im_out_x % 4 == 0);

    q15_t *pIn = bufferA;
    q31_t *pOut = Im_out;

    // CW -> WC
    arm_matrix_instance_q15 pISrc;
    pISrc.numRows = ch_im_in;
    pISrc.numCols = dim_im_in_x;
    pISrc.pData = (q15_t *)Im_in;
    arm_matrix_instance_q15 pIDst;
    pIDst.numRows = dim_im_in_x;
    pIDst.numCols = ch_im_in;
    pIDst.pData = pIn;
    arm_mat_trans_q15(&pISrc, &pIDst);

    q31_t in_data1, in_data2;
    q31_t in_kernel1, in_kernel2;

    q31_t sum1, sum2, sum3, sum4;

    uint16_t row_num = 0;
    
    uint16_t widthCnt = dim_im_out_x >> 2;
    while (widthCnt) {
        const q7_t *cur_wt = wt;
        q31_t *cur_pOut = pOut + row_num;
        for(int och = 0; och < ch_im_out; och++) {
            const q15_t *pB1 = pIn + row_num * ch_im_in;
            const q15_t *pB2 = pB1 + ch_im_in;
            const q15_t *pB3 = pB2 + ch_im_in;
            const q15_t *pB4 = pB3 + ch_im_in;
            uint16_t ichCnt = ch_im_in >> 2;
            sum1 = sum2 = sum3 = sum4 = bias ? bias[och] + NN_ROUND(out_shift) : NN_ROUND(out_shift);
            while (ichCnt) {
                in_kernel1 = arm_nn_read_q7x4(cur_wt);
                in_kernel2 = __SXTB16(__ROR(in_kernel1, 8));
                in_kernel1 = __SXTB16(in_kernel1);
                cur_wt += 4;

                in_data1 = arm_nn_read_q15x2_ia(&pB1);
                sum1 = __SMLAD(in_data1, in_kernel1, sum1);
                in_data2 = arm_nn_read_q15x2_ia(&pB1);
                sum1 = __SMLAD(in_data2, in_kernel2, sum1);

                in_data1 = arm_nn_read_q15x2_ia(&pB2);
                sum2 = __SMLAD(in_data1, in_kernel1, sum2);
                in_data2 = arm_nn_read_q15x2_ia(&pB2);
                sum2 = __SMLAD(in_data2, in_kernel2, sum2);

                in_data1 = arm_nn_read_q15x2_ia(&pB3);
                sum3 = __SMLAD(in_data1, in_kernel1, sum3);
                in_data2 = arm_nn_read_q15x2_ia(&pB3);
                sum3 = __SMLAD(in_data2, in_kernel2, sum3);

                in_data1 = arm_nn_read_q15x2_ia(&pB4);
                sum4 = __SMLAD(in_data1, in_kernel1, sum4);
                in_data2 = arm_nn_read_q15x2_ia(&pB4);
                sum4 = __SMLAD(in_data2, in_kernel2, sum4);

                ichCnt--;
            }

            cur_pOut[0] = sum1;
            cur_pOut[1] = sum2;
            cur_pOut[2] = sum3;
            cur_pOut[3] = sum4;
            cur_pOut += dim_im_out_x;
        }
        widthCnt--;
        row_num+=4;
    }
    return ARM_MATH_SUCCESS;
}