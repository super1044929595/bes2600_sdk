#include "arm_nnfunctions.h"
#include "arm_nnsupportfunctions.h"
#include "dsp/matrix_functions.h"
#include "hal_trace.h"


arm_status arm_convolve_1x1_HWC_q15_q7_q31_out_q31(const q15_t *Im_in,
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

    q31_t *pOut = Im_out;

    // CW -> WC
    arm_matrix_instance_q15 pISrc;
    pISrc.numRows = ch_im_in;
    pISrc.numCols = dim_im_in_x;
    pISrc.pData = (q15_t *)Im_in;
    arm_matrix_instance_q15 pIDst;
    pIDst.numRows = dim_im_in_x;
    pIDst.numCols = ch_im_in;
    pIDst.pData = bufferA;
    arm_mat_trans_q15(&pISrc, &pIDst);
    // memcpy(Im_in, buffer, dim_im_in_x * ch_im_in * sizeof(q15_t));

    uint16_t rowCnt = dim_im_out_x >> 2;
    uint16_t row_num = 0;
    while (rowCnt) {
        const q7_t *pA = wt;
        pOut = Im_out + row_num;

        for (int j = 0; j < ch_im_out; j++) {

            q31_t sum = (q31_t)bias[j];
            q31_t sum1 = sum;
            q31_t sum2 = sum;
            q31_t sum3 = sum;
            const q15_t *pB1 = bufferA + row_num * ch_im_in;
            const q15_t *pB2 = pB1 + ch_im_in;
            const q15_t *pB3 = pB2 + ch_im_in;
            const q15_t *pB4 = pB3 + ch_im_in;
            uint16_t colCnt = ch_im_in >> 2;

            while (colCnt) {
                q31_t inA1, inA2;
                q31_t inB1, inB2;

                pA = read_and_pad(pA, &inA1, &inA2);

                inB1 = arm_nn_read_q15x2_ia(&pB1);
                sum = __SMLAD(inA1, inB1, sum);
                inB2 = arm_nn_read_q15x2_ia(&pB1);
                sum = __SMLAD(inA2, inB2, sum);

                inB1 = arm_nn_read_q15x2_ia(&pB2);
                sum1 = __SMLAD(inA1, inB1, sum1);
                inB2 = arm_nn_read_q15x2_ia(&pB2);
                sum1 = __SMLAD(inA2, inB2, sum1);

                inB1 = arm_nn_read_q15x2_ia(&pB3);
                sum2 = __SMLAD(inA1, inB1, sum2);
                inB2 = arm_nn_read_q15x2_ia(&pB3);
                sum2 = __SMLAD(inA2, inB2, sum2);

                inB1 = arm_nn_read_q15x2_ia(&pB4);
                sum3 = __SMLAD(inA1, inB1, sum3);
                inB2 = arm_nn_read_q15x2_ia(&pB4);
                sum3 = __SMLAD(inA2, inB2, sum3);

                colCnt--;
            }

            colCnt = ch_im_in & 0x3;
            while (colCnt) {
                q7_t inA1 = *pA++;
                q15_t inB1 = *pB1++;
                sum += inA1 * inB1;

                inB1 = *pB2++;
                sum1 += inA1 * inB1;

                inB1 = *pB3++;
                sum2 += inA1 * inB1;

                inB1 = *pB4++;
                sum3 += inA1 * inB1;
                colCnt--;
            }

            // pOut[i*ch_im_out + j] = (q15_t)__SSAT((sum >> out_shift), 16);
            // pOut[(i+1)*ch_im_out + j] = (q15_t)__SSAT((sum1 >> out_shift), 16);
            // pOut[(i+2)*ch_im_out + j] = (q15_t)__SSAT((sum2 >> out_shift), 16);
            // pOut[(i+3)*ch_im_out + j] = (q15_t)__SSAT((sum3 >> out_shift), 16);
            // pOut[0] = (q15_t)__SSAT((sum >> out_shift), 16);
            // pOut[1] = (q15_t)__SSAT((sum1 >> out_shift), 16);
            // pOut[2] = (q15_t)__SSAT((sum2 >> out_shift), 16);
            // pOut[3] = (q15_t)__SSAT((sum3 >> out_shift), 16);
            pOut[0] = sum;
            pOut[1] = sum1;
            pOut[2] = sum2;
            pOut[3] = sum3;
            pOut += dim_im_out_x;
        }

        rowCnt--;
        row_num += 4;
    }

    rowCnt = dim_im_out_x & 0x3;
    while (rowCnt) {
        const q7_t *pA = wt;
        pOut = Im_out + row_num;

        for (int j = 0; j < ch_im_out; j++) {

            q31_t sum = (q31_t)bias[j];
            const q15_t *pB1 = bufferA + row_num * ch_im_in;
            uint16_t colCnt = ch_im_in >> 2;

            while (colCnt) {
                q31_t inA1, inA2;
                q31_t inB1, inB2;

                pA = read_and_pad(pA, &inA1, &inA2);

                inB1 = arm_nn_read_q15x2_ia(&pB1);
                sum = __SMLAD(inA1, inB1, sum);
                inB2 = arm_nn_read_q15x2_ia(&pB1);
                sum = __SMLAD(inA2, inB2, sum);

                colCnt--;
            }

            colCnt = ch_im_in & 0x3;
            while (colCnt) {
                q7_t inA1 = *pA++;
                q15_t inB1 = *pB1++;
                sum += inA1 * inB1;

                colCnt--;
            }

            // pOut[i*ch_im_out + j] = (q15_t)__SSAT((sum >> out_shift), 16);
            // pOut[0] = (q15_t)__SSAT((sum >> out_shift), 16);
            pOut[0] = sum;
            pOut += dim_im_out_x;
        }

        rowCnt--;
        row_num++;
    }
    return ARM_MATH_SUCCESS;
}
