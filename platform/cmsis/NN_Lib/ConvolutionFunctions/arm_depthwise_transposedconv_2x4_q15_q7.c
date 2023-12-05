#include "arm_nnfunctions.h"
#include "arm_nnsupportfunctions.h"
#include <assert.h>

arm_status arm_depthwise_transposed_conv2x4_q15_q7(const q15_t* Im_in,
    const uint16_t dim_im_in_x,
    const uint16_t dim_im_in_y,
    const uint16_t ch_im_in,
    const q7_t* wt,
    const uint16_t ch_im_out,
    const uint16_t dim_kernel_x,
    const uint16_t dim_kernel_y,
    const uint16_t padding_x,
    const uint16_t padding_y,
    const uint16_t outpadding_x,
    const uint16_t outpadding_y,
    const uint16_t stride_x,
    const uint16_t stride_y,
    const q31_t* bias,
    const uint16_t out_shift,
    q15_t* Im_out,
    const uint16_t dim_im_out_x,
    const uint16_t dim_im_out_y)
{

    int element_per_w_ch = dim_kernel_x * dim_kernel_y;
    int element_per_im_h = dim_im_in_x * ch_im_in;
    // int element_per_out_ch = dim_im_out_x * dim_im_out_y;

    int loop_times_per_channel = dim_im_out_x >> 1;

    const q15_t* im_ptr = Im_in;
    const q7_t* wt_ptr = wt;
    q15_t* out_ptr = Im_out;

    q31_t sum, sum2;

    q31_t inM11, inM12, inM13, inM14;
    q31_t inV;


    for (int i = 0; i < ch_im_in; i++) {

        q7_t wt_row1[] = { wt_ptr[1],wt_ptr[0],wt_ptr[3],wt_ptr[2] };
        q7_t wt_row2[] = { wt_ptr[5],wt_ptr[4],wt_ptr[7],wt_ptr[6] };
        const q7_t* wt_row1_ptr = wt_row1;
        const q7_t* wt_row2_ptr = wt_row2;

        const q15_t* input_data_row1 = im_ptr;
        const q15_t* input_data_row2 = im_ptr + element_per_im_h;

        sum = wt_ptr[2] * input_data_row1[0] + wt_ptr[6] * input_data_row2[0] + \
            bias[0] + NN_ROUND(out_shift);
        out_ptr[0] = (q15_t)(__SSAT((sum >> out_shift), 16));
        ++out_ptr;

        for (int j = 0; j < loop_times_per_channel - 1; j++) {

            sum = ((q31_t)(bias[0])) + NN_ROUND(out_shift);
            sum2 = ((q31_t)(bias[0])) + NN_ROUND(out_shift);

            inV = arm_nn_read_q15x2(input_data_row1);
            inM11 = arm_nn_read_q7x4(wt_row1_ptr);
            inM12 = __SXTB16(__ROR(inM11, 8));
            inM11 = __SXTB16(inM11);
            sum = __SMLAD(inM11, inV, sum);
            sum2 = __SMLAD(inM12, inV, sum2);
            inV = arm_nn_read_q15x2(input_data_row2);
            inM13 = arm_nn_read_q7x4(wt_row2_ptr);
            inM14 = __SXTB16(__ROR(inM13, 8));
            inM13 = __SXTB16(inM13);
            sum = __SMLAD(inM13, inV, sum);
            sum2 = __SMLAD(inM14, inV, sum2);

            *out_ptr++ = (q15_t)(__SSAT((sum >> out_shift), 16));
            *out_ptr++ = (q15_t)(__SSAT((sum2 >> out_shift), 16));

            ++input_data_row1;
            ++input_data_row2;

        }

        sum = wt_ptr[1] * input_data_row1[0] + wt_ptr[5] * input_data_row2[0] + \
            bias[0] + NN_ROUND(out_shift);
        out_ptr[0] = (q15_t)(__SSAT((sum >> out_shift), 16));
        ++out_ptr;

        wt_ptr += element_per_w_ch;
        im_ptr += dim_im_in_x;
        ++bias;

    }

    return ARM_MATH_SUCCESS;


}

arm_status arm_depthwise_transposed_conv2x4_q15_q7_q31(const q15_t* Im_in,
    const uint16_t dim_im_in_x,
    const uint16_t dim_im_in_y,
    const uint16_t ch_im_in,
    const q7_t* wt,
    const uint16_t ch_im_out,
    const uint16_t dim_kernel_x,
    const uint16_t dim_kernel_y,
    const uint16_t padding_x,
    const uint16_t padding_y,
    const uint16_t outpadding_x,
    const uint16_t outpadding_y,
    const uint16_t stride_x,
    const uint16_t stride_y,
    const q31_t* bias,
    const uint16_t out_shift,
    q31_t* Im_out,
    const uint16_t dim_im_out_x,
    const uint16_t dim_im_out_y)
{

    int element_per_w_ch = dim_kernel_x * dim_kernel_y;
    int element_per_im_h = dim_im_in_x * ch_im_in;
    // int element_per_out_ch = dim_im_out_x * dim_im_out_y;

    int loop_times_per_channel = dim_im_out_x >> 1;

    const q15_t* im_ptr = Im_in;
    const q7_t* wt_ptr = wt;
    q31_t* out_ptr = Im_out;

    q31_t sum, sum2;

    q31_t inM11, inM12, inM13, inM14;
    q31_t inV;


    for (int i = 0; i < ch_im_in; i++) {

        q7_t wt_row1[] = { wt_ptr[1],wt_ptr[0],wt_ptr[3],wt_ptr[2] };
        q7_t wt_row2[] = { wt_ptr[5],wt_ptr[4],wt_ptr[7],wt_ptr[6] };
        const q7_t* wt_row1_ptr = wt_row1;
        const q7_t* wt_row2_ptr = wt_row2;

        const q15_t* input_data_row1 = im_ptr;
        const q15_t* input_data_row2 = im_ptr + element_per_im_h;

        sum = wt_ptr[2] * input_data_row1[0] + wt_ptr[6] * input_data_row2[0] + \
            NN_ROUND(out_shift);
        if (bias) {
            sum = sum + bias[0];
        }
        // out_ptr[0] = (q15_t)(__SSAT((sum >> out_shift), 16));
        out_ptr[0] = sum;
        ++out_ptr;

        for (int j = 0; j < loop_times_per_channel - 1; j++) {

            sum = NN_ROUND(out_shift);
            sum2 =NN_ROUND(out_shift);

            if (bias) {
                sum = sum + ((q31_t)(bias[0]));
                sum2 = sum2 + ((q31_t)(bias[0]));
            }

            inV = arm_nn_read_q15x2(input_data_row1);
            inM11 = arm_nn_read_q7x4(wt_row1_ptr);
            inM12 = __SXTB16(__ROR(inM11, 8));
            inM11 = __SXTB16(inM11);
            sum = __SMLAD(inM11, inV, sum);
            sum2 = __SMLAD(inM12, inV, sum2);
            inV = arm_nn_read_q15x2(input_data_row2);
            inM13 = arm_nn_read_q7x4(wt_row2_ptr);
            inM14 = __SXTB16(__ROR(inM13, 8));
            inM13 = __SXTB16(inM13);
            sum = __SMLAD(inM13, inV, sum);
            sum2 = __SMLAD(inM14, inV, sum2);

            // *out_ptr++ = (q15_t)(__SSAT((sum >> out_shift), 16));
            // *out_ptr++ = (q15_t)(__SSAT((sum2 >> out_shift), 16));

            *out_ptr++ = sum;
            *out_ptr++ = sum2;

            ++input_data_row1;
            ++input_data_row2;

        }

        sum = wt_ptr[1] * input_data_row1[0] + wt_ptr[5] * input_data_row2[0] + \
            NN_ROUND(out_shift);
        if (bias) {
            sum = sum + bias[0];
        }
        // out_ptr[0] = (q15_t)(__SSAT((sum >> out_shift), 16));
        out_ptr[0] = sum;
        ++out_ptr;

        wt_ptr += element_per_w_ch;
        im_ptr += dim_im_in_x;
        ++bias;

    }

    return ARM_MATH_SUCCESS;


}