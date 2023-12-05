#ifndef BES_NNFUNCTIONS_H
#define BES_NNFUNCTIONS_H

#include "arm_math.h"
#include "arm_nnfunctions.h"

arm_status bes_fully_connected_mat_q7_vec_f32(const float32_t *pV,
                                              const q7_t *pM,
                                              const uint16_t dim_vec,
                                              const uint16_t num_of_rows,
                                              float bias_scale,
                                              float weight_scale,
                                              const q7_t *bias,
                                              float32_t *pOut,
                                              float32_t *vec_buffer);

int32_t bes_fully_connected_mat_q7_vec_f32_get_buffer_size(const uint16_t dim_vec,
                                                           const uint16_t num_of_rows);

void bes_nn_activations_direct_f32(float32_t *data, uint16_t size, arm_nn_activation_type type);

#endif
