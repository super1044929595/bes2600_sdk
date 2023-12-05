#
# Copyright (c) 2019-2021 Arm Limited. All rights reserved.
#
# SPDX-License-Identifier: Apache-2.0
#
# Licensed under the Apache License, Version 2.0 (the License); you may
# not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
# www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an AS IS BASIS, WITHOUT
# WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.
#

project(CMSISNNConvolutions)

file(GLOB SRC "./*_s8*.c")
file(GLOB SRCL arm_depthwise_separable_conv2x4_q15_q7_q31.c
               arm_convolve_1x1_HWC_q15_q7_q31.c
               arm_depthwise_transposed_conv2x4_q15_q7.c)
add_library(CMSISNNConvolutions STATIC ${SRC} ${SRCL})

### Includes
target_include_directories(CMSISNNConvolutions PUBLIC "${NN}/Include")
target_include_directories(CMSISNNConvolutions PUBLIC "${ROOT}/CMSIS/Core/Include")
target_include_directories(CMSISNNConvolutions PUBLIC "${ROOT}/CMSIS/DSP/Include")



