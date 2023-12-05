#include "arm_nnfunctions.h"
#include "arm_nnsupportfunctions.h"
#include <assert.h>

void arm_depthwise_separable_conv_2x4_q15_q7_q31_out_q15(const q15_t *Im_in,
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
                                                         q15_t *Im_out,
                                                         const uint16_t dim_im_out_x,
                                                         const uint16_t dim_im_out_y)
{
    assert(ch_im_in == ch_im_out);
    assert(padding_x == 1 && padding_y == 0);
    assert(stride_x == 2 && stride_y == 1);
    assert(dim_kernel_x == 4 && dim_im_in_y == 2);
    assert(dim_im_in_y == 2 && dim_im_out_y == 1);

    int element_per_w_ch = dim_kernel_x * dim_kernel_y;
    int element_per_im_h = dim_im_in_x * ch_im_in; // CW
    // int element_per_im_size = element_per_im_h * dim_im_in_y;

    const q15_t *im_ptr = Im_in;
    const q7_t *wt_ptr = wt;
    q15_t *out_ptr = Im_out;

    q31_t sum;

    q31_t inM1, inM2, inM3, inM4;
    q31_t inV1, inV2, inV3, inV4;

    int i, j;
    for (i = 0; i < ch_im_in; i++) {

        q31_t cur_bias = bias ? bias[i] + NN_ROUND(out_shift) : NN_ROUND(out_shift);

        const q7_t *wt_row1 = wt_ptr;
        const q7_t *wt_row2 = wt_ptr+4;

        const q15_t *input_data_row1 = im_ptr;
        const q15_t *input_data_row2 = im_ptr + element_per_im_h;

        inM1 = arm_nn_read_q7x4(wt_row1);
        inM2 = __SXTB16(__ROR(inM1, 8));
        inM1 = __SXTB16(inM1);

        inM3 = arm_nn_read_q7x4(wt_row2);
        inM4 = __SXTB16(__ROR(inM3, 8));
        inM3 = __SXTB16(inM3);

        sum = cur_bias;

        sum += input_data_row1[0] * wt_row1[2]; // 1
        input_data_row1 += 1;

        inV2 = arm_nn_read_q15x2(input_data_row1);
        input_data_row1 += 2;
        sum = __SMLAD(inM2, inV2, sum); //2

        sum += input_data_row2[0] * wt_row2[2]; // 3
        input_data_row2 += 1;

        inV4 = arm_nn_read_q15x2(input_data_row2);
        input_data_row2 +=2;
        sum = __SMLAD(inM4, inV4, sum); // 4

        *out_ptr++ = (q15_t)__SSAT((sum >> out_shift), 16);

        for (j = 0; j < dim_im_out_x - 2; j++) {

            sum = cur_bias;

            inV1 = inV2;
            sum = __SMLAD(inM1, inV1, sum);

            inV2 = arm_nn_read_q15x2(input_data_row1);
            input_data_row1 += 2;
            sum = __SMLAD(inM2, inV2, sum);

            inV3 = inV4;
            sum = __SMLAD(inM3, inV3, sum);

            inV4 = arm_nn_read_q15x2(input_data_row2);
            input_data_row2 += 2;
            sum = __SMLAD(inM4, inV4, sum);

            *out_ptr++ = (q15_t)__SSAT((sum >> out_shift), 16);
        }

        sum = cur_bias;

        inV1 = inV2;
        sum = __SMLAD(inM1, inV1, sum);

        sum += input_data_row1[0] * wt_row1[1];

        inV3 = inV4;
        sum = __SMLAD(inM3, inV3, sum);

        sum += input_data_row2[0] * wt_row2[1];

        *out_ptr++ = (q15_t)__SSAT((sum >> out_shift), 16);

        wt_ptr += element_per_w_ch;
        im_ptr += dim_im_in_x;
    }
}

void arm_depthwise_separable_conv_2x4_q15_q7_q31_out_q31(const q15_t *Im_in,
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
                                                        const uint16_t dim_im_out_y)
{
    assert(ch_im_in == ch_im_out);
    assert(padding_x == 1 && padding_y == 0);
    assert(stride_x == 2 && stride_y == 1);
    assert(dim_kernel_x == 4 && dim_im_in_y == 2);
    assert(dim_im_in_y == 2 && dim_im_out_y == 1);

    int element_per_w_ch = dim_kernel_x * dim_kernel_y;
    int element_per_im_h = dim_im_in_x * ch_im_in; // CW
    // int element_per_im_size = element_per_im_h * dim_im_in_y;

    const q15_t *im_ptr = Im_in;
    const q7_t *wt_ptr = wt;
    q31_t *out_ptr = Im_out;

    q31_t sum;

    q31_t inM1, inM2, inM3, inM4;
    q31_t inV1, inV2, inV3, inV4;

    int i, j;
    for (i = 0; i < ch_im_in; i++) {

        q31_t cur_bias = bias ? bias[i] : 0;
        const q7_t *wt_row1 = wt_ptr;
        const q7_t *wt_row2 = wt_ptr+4;

        const q15_t *input_data_row1 = im_ptr;
        const q15_t *input_data_row2 = im_ptr + element_per_im_h;

        inM1 = arm_nn_read_q7x4(wt_row1);
        inM2 = __SXTB16(__ROR(inM1, 8));
        inM1 = __SXTB16(inM1);

        inM3 = arm_nn_read_q7x4(wt_row2);
        inM4 = __SXTB16(__ROR(inM3, 8));
        inM3 = __SXTB16(inM3);

        sum = cur_bias;

        sum += input_data_row1[0] * wt_row1[2]; // 1
        input_data_row1 += 1;

        inV2 = arm_nn_read_q15x2(input_data_row1);
        input_data_row1 += 2;
        sum = __SMLAD(inM2, inV2, sum); //2

        sum += input_data_row2[0] * wt_row2[2]; // 3
        input_data_row2 += 1;

        inV4 = arm_nn_read_q15x2(input_data_row2);
        input_data_row2 +=2;
        sum = __SMLAD(inM4, inV4, sum); // 4

        *out_ptr++ = sum;

        for (j = 0; j < dim_im_out_x - 2; j++) {

            sum = cur_bias;

            inV1 = inV2;
            sum = __SMLAD(inM1, inV1, sum);

            inV2 = arm_nn_read_q15x2(input_data_row1);
            input_data_row1 += 2;
            sum = __SMLAD(inM2, inV2, sum);

            inV3 = inV4;
            sum = __SMLAD(inM3, inV3, sum);

            inV4 = arm_nn_read_q15x2(input_data_row2);
            input_data_row2 += 2;
            sum = __SMLAD(inM4, inV4, sum);

            *out_ptr++ = sum;
        }

        sum = cur_bias;

        inV1 = inV2;
        sum = __SMLAD(inM1, inV1, sum);

        sum += input_data_row1[0] * wt_row1[1];

        inV3 = inV4;
        sum = __SMLAD(inM3, inV3, sum);

        sum += input_data_row2[0] * wt_row2[1];

        *out_ptr++ = sum;

        wt_ptr += element_per_w_ch;
        im_ptr += dim_im_in_x;
    }
}