/***************************************************************************
 *
 * Copyright 2015-2020 BES.
 * All rights reserved. All unpublished rights reserved.
 *
 * No part of this work may be used or reproduced in any form or by any
 * means, or stored in a database or retrieval system, without prior written
 * permission of BES.
 *
 * Use of this work is governed by a license granted by BES.
 * This work contains confidential and proprietary information of
 * BES. which is protected by copyright, trade secret,
 * trademark and other intellectual property rights.
 *
 ****************************************************************************/
#ifndef __REG_CODEC_BEST1305_H__
#define __REG_CODEC_BEST1305_H__

#include "plat_types.h"

struct CODEC_T {
    __IO uint32_t REG_000;
    __IO uint32_t REG_004;
    __IO uint32_t REG_008;
    __IO uint32_t REG_00C;
    __IO uint32_t REG_010;
    __IO uint32_t REG_014;
    __IO uint32_t REG_018;
    __IO uint32_t REG_01C;
    __IO uint32_t REG_020;
    __IO uint32_t REG_024;
    __IO uint32_t REG_028;
    __IO uint32_t REG_02C;
    __IO uint32_t REG_030;
    __IO uint32_t REG_034;
    __IO uint32_t REG_038;
    __IO uint32_t REG_03C;
    __IO uint32_t REG_040;
    __IO uint32_t REG_044;
    __IO uint32_t REG_048;
    __IO uint32_t REG_04C;
    __IO uint32_t REG_050;
    __IO uint32_t REG_054;
    __IO uint32_t REG_058;
    __IO uint32_t REG_05C;
    __IO uint32_t REG_060;
    __IO uint32_t REG_064;
    __IO uint32_t REG_068;
    __IO uint32_t REG_06C;
    __IO uint32_t REG_070;
    __IO uint32_t REG_074;
    __IO uint32_t REG_078;
    __IO uint32_t REG_07C;
    __IO uint32_t REG_080;
    __IO uint32_t REG_084;
    __IO uint32_t REG_088;
    __IO uint32_t REG_08C;
    __IO uint32_t REG_090;
    __IO uint32_t REG_094;
    __IO uint32_t REG_098;
    __IO uint32_t REG_09C;
    __IO uint32_t REG_0A0;
    __IO uint32_t REG_0A4;
    __IO uint32_t REG_0A8;
    __IO uint32_t REG_0AC;
    __IO uint32_t REG_0B0;
    __IO uint32_t REG_0B4;
    __IO uint32_t REG_0B8;
    __IO uint32_t REG_0BC;
    __IO uint32_t REG_0C0;
    __IO uint32_t REG_0C4;
    __IO uint32_t REG_0C8;
    __IO uint32_t REG_0CC;
    __IO uint32_t REG_0D0;
    __IO uint32_t REG_0D4;
    __IO uint32_t REG_0D8;
    __IO uint32_t REG_0DC;
    __IO uint32_t REG_0E0;
    __IO uint32_t REG_0E4;
    __IO uint32_t REG_0E8;
    __IO uint32_t REG_0EC;
    __IO uint32_t REG_0F0;
    __IO uint32_t REG_0F4;
    __IO uint32_t REG_0F8;
    __IO uint32_t REG_0FC;
    __IO uint32_t REG_100;
    __IO uint32_t REG_104;
    __IO uint32_t REG_108;
    __IO uint32_t REG_10C;
    __IO uint32_t REG_110;
    __IO uint32_t REG_114;
    __IO uint32_t REG_118;
    __IO uint32_t REG_11C;
    __IO uint32_t REG_120;
    __IO uint32_t REG_124;
    __IO uint32_t REG_128;
    __IO uint32_t REG_12C;
    __IO uint32_t REG_130;
    __IO uint32_t REG_134;
    __IO uint32_t REG_138;
    __IO uint32_t REG_13C;
    __IO uint32_t REG_140;
    __IO uint32_t REG_144;
    __IO uint32_t REG_148;
    __IO uint32_t REG_14C;
    __IO uint32_t REG_150;
    __IO uint32_t REG_154;
    __IO uint32_t REG_158;
    __IO uint32_t REG_15C;
    __IO uint32_t REG_160;
    __IO uint32_t REG_164;
    __IO uint32_t REG_168;
    __IO uint32_t REG_16C;
    __IO uint32_t REG_170;
    __IO uint32_t REG_174;
    __IO uint32_t REG_178;
    __IO uint32_t REG_17C;
    __IO uint32_t REG_180;
    __IO uint32_t REG_184;
    __IO uint32_t REG_188;
    __IO uint32_t REG_18C;
    __IO uint32_t REG_190;
    __IO uint32_t REG_194;
    __IO uint32_t REG_198;
    __IO uint32_t REG_19C;
    __IO uint32_t REG_1A0;
    __IO uint32_t REG_1A4;
    __IO uint32_t REG_1A8;
    __IO uint32_t REG_1AC;
    __IO uint32_t REG_1B0;
    __IO uint32_t REG_1B4;
    __IO uint32_t REG_1B8;
    __IO uint32_t REG_1BC;
    __IO uint32_t REG_1C0;
    __IO uint32_t REG_1C4;
    __IO uint32_t REG_1C8;
    __IO uint32_t REG_1CC;
    __IO uint32_t REG_1D0;
    __IO uint32_t REG_1D4;
    __IO uint32_t REG_1D8;
    __IO uint32_t REG_1DC;
    __IO uint32_t REG_1E0;
    __IO uint32_t REG_1E4;
    __IO uint32_t REG_1E8;
    __IO uint32_t REG_1EC;
    __IO uint32_t REG_1F0;
    __IO uint32_t REG_1F4;
    __IO uint32_t REG_1F8;
    __IO uint32_t REG_1FC;
    __IO uint32_t REG_200;
    __IO uint32_t REG_204;
    __IO uint32_t REG_208;
    __IO uint32_t REG_20C;
    __IO uint32_t REG_210;
    __IO uint32_t REG_214;
    __IO uint32_t REG_218;
    __IO uint32_t REG_21C;
    __IO uint32_t REG_220;
    __IO uint32_t REG_224;
    __IO uint32_t REG_228;
    __IO uint32_t REG_22C;
    __IO uint32_t REG_230;
    __IO uint32_t REG_234;
    __IO uint32_t REG_238;
    __IO uint32_t REG_23C;
    __IO uint32_t REG_240;
    __IO uint32_t REG_244;
    __IO uint32_t REG_248;
    __IO uint32_t REG_24C;
    __IO uint32_t REG_250;
    __IO uint32_t REG_254;
    __IO uint32_t REG_258;
    __IO uint32_t REG_25C;
    __IO uint32_t REG_260;
    __IO uint32_t REG_264;
    __IO uint32_t REG_268;
    __IO uint32_t REG_26C;
    __IO uint32_t REG_270;
    __IO uint32_t REG_274;
    __IO uint32_t REG_278;
    __IO uint32_t REG_27C;
    __IO uint32_t REG_280;
    __IO uint32_t REG_284;
    __IO uint32_t REG_288;
    __IO uint32_t REG_28C;
    __IO uint32_t REG_290;
    __IO uint32_t REG_294;
    __IO uint32_t REG_298;
    __IO uint32_t REG_29C;
    __IO uint32_t REG_2A0;
    __IO uint32_t REG_2A4;
    __IO uint32_t REG_2A8;
    __IO uint32_t REG_2AC;
    __IO uint32_t REG_2B0;
    __IO uint32_t REG_2B4;
    __IO uint32_t REG_2B8;
    __IO uint32_t REG_2BC;
    __IO uint32_t REG_2C0;
    __IO uint32_t REG_2C4;
    __IO uint32_t REG_2C8;
    __IO uint32_t REG_2CC;
    __IO uint32_t REG_2D0;
    __IO uint32_t REG_2D4;
    __IO uint32_t REG_2D8;
    __IO uint32_t REG_2DC;
    __IO uint32_t REG_2E0;
    __IO uint32_t REG_2E4;
    __IO uint32_t REG_2E8;
    __IO uint32_t REG_2EC;
    __IO uint32_t REG_2F0;
    __IO uint32_t REG_2F4;
    __IO uint32_t REG_2F8;
    __IO uint32_t REG_2FC;
    __IO uint32_t REG_300;
    __IO uint32_t REG_304;
    __IO uint32_t REG_308;
    __IO uint32_t REG_30C;
    __IO uint32_t REG_310;
    __IO uint32_t REG_314;
    __IO uint32_t REG_318;
    __IO uint32_t REG_31C;
    __IO uint32_t REG_320;
    __IO uint32_t REG_324;
    __IO uint32_t REG_328;
    __IO uint32_t REG_32C;
    __IO uint32_t REG_330;
    __IO uint32_t REG_334;
    __IO uint32_t REG_338;
    __IO uint32_t REG_33C;
    __IO uint32_t REG_340;
    __IO uint32_t REG_344;
    __IO uint32_t REG_348;
    __IO uint32_t REG_34C;
    __IO uint32_t REG_350;
    __IO uint32_t REG_354;
    __IO uint32_t REG_358;
    __IO uint32_t REG_35C;
    __IO uint32_t REG_360;
    __IO uint32_t REG_364;
    __IO uint32_t REG_368;
    __IO uint32_t REG_36C;
    __IO uint32_t REG_370;
    __IO uint32_t REG_374;
    __IO uint32_t REG_378;
    __IO uint32_t REG_37C;
    __IO uint32_t REG_380;
    __IO uint32_t REG_384;
    __IO uint32_t REG_388;
    __IO uint32_t REG_38C;
    __IO uint32_t REG_390;
    __IO uint32_t REG_394;
    __IO uint32_t REG_398;
    __IO uint32_t REG_39C;
    __IO uint32_t REG_3A0;
    __IO uint32_t REG_3A4;
    __IO uint32_t REG_3A8;
    __IO uint32_t REG_3AC;
    __IO uint32_t REG_3B0;
    __IO uint32_t REG_3B4;
    __IO uint32_t REG_3B8;
    __IO uint32_t REG_3BC;
    __IO uint32_t REG_3C0;
    __IO uint32_t REG_3C4;
    __IO uint32_t REG_3C8;
    __IO uint32_t REG_3CC;
    __IO uint32_t REG_3D0;
    __IO uint32_t REG_3D4;
    __IO uint32_t REG_3D8;
    __IO uint32_t REG_3DC;
    __IO uint32_t REG_3E0;
    __IO uint32_t REG_3E4;
    __IO uint32_t REG_3E8;
    __IO uint32_t REG_3EC;
    __IO uint32_t REG_3F0;
    __IO uint32_t REG_3F4;
    __IO uint32_t REG_3F8;
    __IO uint32_t REG_3FC;
    __IO uint32_t REG_400;
    __IO uint32_t REG_404;
    __IO uint32_t REG_408;
    __IO uint32_t REG_40C;
    __IO uint32_t REG_410;
    __IO uint32_t REG_414;
    __IO uint32_t REG_418;
    __IO uint32_t REG_41C;
    __IO uint32_t REG_420;
    __IO uint32_t REG_424;
    __IO uint32_t REG_428;
    __IO uint32_t REG_42C;
    __IO uint32_t REG_430;
    __IO uint32_t REG_434;
    __IO uint32_t REG_438;
    __IO uint32_t REG_43C;
    __IO uint32_t REG_440;
    __IO uint32_t REG_444;
    __IO uint32_t REG_448;
    __IO uint32_t REG_44C;
    __IO uint32_t REG_450;
    __IO uint32_t REG_454;
    __IO uint32_t REG_458;
    __IO uint32_t REG_45C;
    __IO uint32_t REG_460;
    __IO uint32_t REG_464;
    __IO uint32_t REG_468;
    __IO uint32_t REG_46C;
    __IO uint32_t REG_470;
};

// reg_000
#define CODEC_CODEC_IF_EN                                   (1 << 0)
#define CODEC_ADC_ENABLE                                    (1 << 1)
#define CODEC_ADC_ENABLE_CH0                                (1 << 2)
#define CODEC_ADC_ENABLE_CH1                                (1 << 3)
#define CODEC_ADC_ENABLE_CH2                                (1 << 4)
#define CODEC_ADC_ENABLE_CH3                                (1 << 5)
#define CODEC_DAC_ENABLE                                    (1 << 6)
#define CODEC_DMACTRL_RX                                    (1 << 7)
#define CODEC_DMACTRL_TX                                    (1 << 8)
#define CODEC_DAC_ENABLE_SND                                (1 << 9)
#define CODEC_DMACTRL_TX_SND                                (1 << 10)

// reg_004
#define CODEC_RX_FIFO_FLUSH_CH0                             (1 << 0)
#define CODEC_RX_FIFO_FLUSH_CH1                             (1 << 1)
#define CODEC_RX_FIFO_FLUSH_CH2                             (1 << 2)
#define CODEC_RX_FIFO_FLUSH_CH3                             (1 << 3)
#define CODEC_TX_FIFO_FLUSH                                 (1 << 4)
#define CODEC_TX_FIFO_FLUSH_SND                             (1 << 5)
#define CODEC_MC_FIFO_FLUSH                                 (1 << 6)

// reg_008
#define CODEC_CODEC_RX_THRESHOLD(n)                         (((n) & 0xF) << 0)
#define CODEC_CODEC_RX_THRESHOLD_MASK                       (0xF << 0)
#define CODEC_CODEC_RX_THRESHOLD_SHIFT                      (0)
#define CODEC_CODEC_TX_THRESHOLD(n)                         (((n) & 0xF) << 4)
#define CODEC_CODEC_TX_THRESHOLD_MASK                       (0xF << 4)
#define CODEC_CODEC_TX_THRESHOLD_SHIFT                      (4)
#define CODEC_CODEC_TX_THRESHOLD_SND(n)                     (((n) & 0xF) << 8)
#define CODEC_CODEC_TX_THRESHOLD_SND_MASK                   (0xF << 8)
#define CODEC_CODEC_TX_THRESHOLD_SND_SHIFT                  (8)
#define CODEC_MC_THRESHOLD(n)                               (((n) & 0xF) << 12)
#define CODEC_MC_THRESHOLD_MASK                             (0xF << 12)
#define CODEC_MC_THRESHOLD_SHIFT                            (12)

// reg_00c
#define CODEC_CODEC_RX_OVERFLOW(n)                          (((n) & 0x1F) << 0)
#define CODEC_CODEC_RX_OVERFLOW_MASK                        (0x1F << 0)
#define CODEC_CODEC_RX_OVERFLOW_SHIFT                       (0)
#define CODEC_CODEC_RX_UNDERFLOW(n)                         (((n) & 0x1F) << 5)
#define CODEC_CODEC_RX_UNDERFLOW_MASK                       (0x1F << 5)
#define CODEC_CODEC_RX_UNDERFLOW_SHIFT                      (5)
#define CODEC_CODEC_TX_OVERFLOW                             (1 << 10)
#define CODEC_CODEC_TX_UNDERFLOW                            (1 << 11)
#define CODEC_CODEC_TX_OVERFLOW_SND                         (1 << 12)
#define CODEC_CODEC_TX_UNDERFLOW_SND                        (1 << 13)
#define CODEC_MC_OVERFLOW                                   (1 << 14)
#define CODEC_MC_UNDERFLOW                                  (1 << 15)
#define CODEC_EVENT_TRIGGER                                 (1 << 16)
#define CODEC_FB_CHECK_ERROR_TRIG_CH0                       (1 << 17)
#define CODEC_BT_TRIGGER                                    (1 << 18)
#define CODEC_ADC_MAX_OVERFLOW                              (1 << 19)
#define CODEC_TIME_TRIGGER                                  (1 << 20)

// reg_010
#define CODEC_CODEC_RX_OVERFLOW_MSK(n)                      (((n) & 0x1F) << 0)
#define CODEC_CODEC_RX_OVERFLOW_MSK_MASK                    (0x1F << 0)
#define CODEC_CODEC_RX_OVERFLOW_MSK_SHIFT                   (0)
#define CODEC_CODEC_RX_UNDERFLOW_MSK(n)                     (((n) & 0x1F) << 5)
#define CODEC_CODEC_RX_UNDERFLOW_MSK_MASK                   (0x1F << 5)
#define CODEC_CODEC_RX_UNDERFLOW_MSK_SHIFT                  (5)
#define CODEC_CODEC_TX_OVERFLOW_MSK                         (1 << 10)
#define CODEC_CODEC_TX_UNDERFLOW_MSK                        (1 << 11)
#define CODEC_CODEC_TX_OVERFLOW_SND_MSK                     (1 << 12)
#define CODEC_CODEC_TX_UNDERFLOW_SND_MSK                    (1 << 13)
#define CODEC_MC_OVERFLOW_MSK                               (1 << 14)
#define CODEC_MC_UNDERFLOW_MSK                              (1 << 15)
#define CODEC_EVENT_TRIGGER_MSK                             (1 << 16)
#define CODEC_FB_CHECK_ERROR_TRIG_CH0_MSK                   (1 << 17)
#define CODEC_BT_TRIGGER_MSK                                (1 << 18)
#define CODEC_ADC_MAX_OVERFLOW_MSK                          (1 << 19)
#define CODEC_TIME_TRIGGER_MSK                              (1 << 20)

// reg_014
#define CODEC_FIFO_COUNT_CH0(n)                             (((n) & 0xF) << 0)
#define CODEC_FIFO_COUNT_CH0_MASK                           (0xF << 0)
#define CODEC_FIFO_COUNT_CH0_SHIFT                          (0)
#define CODEC_FIFO_COUNT_CH1(n)                             (((n) & 0xF) << 4)
#define CODEC_FIFO_COUNT_CH1_MASK                           (0xF << 4)
#define CODEC_FIFO_COUNT_CH1_SHIFT                          (4)
#define CODEC_FIFO_COUNT_CH2(n)                             (((n) & 0xF) << 8)
#define CODEC_FIFO_COUNT_CH2_MASK                           (0xF << 8)
#define CODEC_FIFO_COUNT_CH2_SHIFT                          (8)
#define CODEC_FIFO_COUNT_CH3(n)                             (((n) & 0xF) << 12)
#define CODEC_FIFO_COUNT_CH3_MASK                           (0xF << 12)
#define CODEC_FIFO_COUNT_CH3_SHIFT                          (12)

// reg_018
#define CODEC_FIFO_COUNT_TX(n)                              (((n) & 0xF) << 0)
#define CODEC_FIFO_COUNT_TX_MASK                            (0xF << 0)
#define CODEC_FIFO_COUNT_TX_SHIFT                           (0)
#define CODEC_FIFO_COUNT_TX_SND(n)                          (((n) & 0xF) << 4)
#define CODEC_FIFO_COUNT_TX_SND_MASK                        (0xF << 4)
#define CODEC_FIFO_COUNT_TX_SND_SHIFT                       (4)
#define CODEC_STATE_RX_CH(n)                                (((n) & 0x1FF) << 8)
#define CODEC_STATE_RX_CH_MASK                              (0x1FF << 8)
#define CODEC_STATE_RX_CH_SHIFT                             (8)
#define CODEC_MC_FIFO_COUNT(n)                              (((n) & 0xF) << 17)
#define CODEC_MC_FIFO_COUNT_MASK                            (0xF << 17)
#define CODEC_MC_FIFO_COUNT_SHIFT                           (17)

// reg_01c
#define CODEC_RX_FIFO_DATA(n)                               (((n) & 0xFFFFFFFF) << 0)
#define CODEC_RX_FIFO_DATA_MASK                             (0xFFFFFFFF << 0)
#define CODEC_RX_FIFO_DATA_SHIFT                            (0)

// reg_020
#define CODEC_RX_FIFO_DATA(n)                               (((n) & 0xFFFFFFFF) << 0)
#define CODEC_RX_FIFO_DATA_MASK                             (0xFFFFFFFF << 0)
#define CODEC_RX_FIFO_DATA_SHIFT                            (0)

// reg_024
#define CODEC_RX_FIFO_DATA(n)                               (((n) & 0xFFFFFFFF) << 0)
#define CODEC_RX_FIFO_DATA_MASK                             (0xFFFFFFFF << 0)
#define CODEC_RX_FIFO_DATA_SHIFT                            (0)

// reg_028
#define CODEC_RX_FIFO_DATA(n)                               (((n) & 0xFFFFFFFF) << 0)
#define CODEC_RX_FIFO_DATA_MASK                             (0xFFFFFFFF << 0)
#define CODEC_RX_FIFO_DATA_SHIFT                            (0)

// reg_02c
#define CODEC_RX_FIFO_DATA(n)                               (((n) & 0xFFFFFFFF) << 0)
#define CODEC_RX_FIFO_DATA_MASK                             (0xFFFFFFFF << 0)
#define CODEC_RX_FIFO_DATA_SHIFT                            (0)

// reg_038
#define CODEC_MC_FIFO_DATA(n)                               (((n) & 0xFFFFFFFF) << 0)
#define CODEC_MC_FIFO_DATA_MASK                             (0xFFFFFFFF << 0)
#define CODEC_MC_FIFO_DATA_SHIFT                            (0)

// reg_03c
#define CODEC_TX_FIFO_DATA_SND(n)                           (((n) & 0xFFFFFFFF) << 0)
#define CODEC_TX_FIFO_DATA_SND_MASK                         (0xFFFFFFFF << 0)
#define CODEC_TX_FIFO_DATA_SND_SHIFT                        (0)

// reg_040
#define CODEC_MODE_16BIT_ADC                                (1 << 0)
#define CODEC_MODE_24BIT_ADC                                (1 << 1)
#define CODEC_MODE_32BIT_ADC                                (1 << 2)

// reg_044
#define CODEC_DUAL_CHANNEL_DAC                              (1 << 0)
#define CODEC_DAC_EXCHANGE_L_R                              (1 << 1)
#define CODEC_MODE_16BIT_DAC                                (1 << 2)
#define CODEC_MODE_32BIT_DAC                                (1 << 3)
#define CODEC_DUAL_CHANNEL_DAC_SND                          (1 << 4)
#define CODEC_DAC_EXCHANGE_L_R_SND                          (1 << 5)
#define CODEC_MODE_16BIT_DAC_SND                            (1 << 6)
#define CODEC_MODE_32BIT_DAC_SND                            (1 << 7)

// reg_04c
#define CODEC_MC_ENABLE                                     (1 << 0)
#define CODEC_DUAL_CHANNEL_MC                               (1 << 1)
#define CODEC_MODE_16BIT_MC                                 (1 << 2)
#define CODEC_DMACTRL_MC                                    (1 << 3)
#define CODEC_MC_DELAY(n)                                   (((n) & 0xFF) << 4)
#define CODEC_MC_DELAY_MASK                                 (0xFF << 4)
#define CODEC_MC_DELAY_SHIFT                                (4)
#define CODEC_MC_RATE_SEL                                   (1 << 12)
#define CODEC_MODE_32BIT_MC                                 (1 << 13)
#define CODEC_MC_EN_SEL                                     (1 << 14)
#define CODEC_MC_RATE_SRC_SEL                               (1 << 15)
#define CODEC_MC_CH_SEL                                     (1 << 16)
#define CODEC_CODEC_DAC_ENABLE_SND_SEL(n)                   (((n) & 0x3) << 17)
#define CODEC_CODEC_DAC_ENABLE_SND_SEL_MASK                 (0x3 << 17)
#define CODEC_CODEC_DAC_ENABLE_SND_SEL_SHIFT                (17)

// reg_050
#define CODEC_CODEC_COUNT_KEEP(n)                           (((n) & 0xFFFFFFFF) << 0)
#define CODEC_CODEC_COUNT_KEEP_MASK                         (0xFFFFFFFF << 0)
#define CODEC_CODEC_COUNT_KEEP_SHIFT                        (0)

// reg_054
#define CODEC_DAC_ENABLE_SEL(n)                             (((n) & 0x3) << 0)
#define CODEC_DAC_ENABLE_SEL_MASK                           (0x3 << 0)
#define CODEC_DAC_ENABLE_SEL_SHIFT                          (0)
#define CODEC_ADC_ENABLE_SEL(n)                             (((n) & 0x3) << 2)
#define CODEC_ADC_ENABLE_SEL_MASK                           (0x3 << 2)
#define CODEC_ADC_ENABLE_SEL_SHIFT                          (2)
#define CODEC_DAC_ENABLE_SEL_SND(n)                         (((n) & 0x3) << 4)
#define CODEC_DAC_ENABLE_SEL_SND_MASK                       (0x3 << 4)
#define CODEC_DAC_ENABLE_SEL_SND_SHIFT                      (4)
#define CODEC_CODEC_DAC_ENABLE_SEL(n)                       (((n) & 0x3) << 6)
#define CODEC_CODEC_DAC_ENABLE_SEL_MASK                     (0x3 << 6)
#define CODEC_CODEC_DAC_ENABLE_SEL_SHIFT                    (6)
#define CODEC_CODEC_ADC_ENABLE_SEL(n)                       (((n) & 0x3) << 8)
#define CODEC_CODEC_ADC_ENABLE_SEL_MASK                     (0x3 << 8)
#define CODEC_CODEC_ADC_ENABLE_SEL_SHIFT                    (8)
#define CODEC_CODEC_ADC_UH_ENABLE_SEL(n)                    (((n) & 0x3) << 10)
#define CODEC_CODEC_ADC_UH_ENABLE_SEL_MASK                  (0x3 << 10)
#define CODEC_CODEC_ADC_UH_ENABLE_SEL_SHIFT                 (10)
#define CODEC_CODEC_ADC_LH_ENABLE_SEL(n)                    (((n) & 0x3) << 12)
#define CODEC_CODEC_ADC_LH_ENABLE_SEL_MASK                  (0x3 << 12)
#define CODEC_CODEC_ADC_LH_ENABLE_SEL_SHIFT                 (12)
#define CODEC_CODEC_DAC_UH_ENABLE_SEL(n)                    (((n) & 0x3) << 14)
#define CODEC_CODEC_DAC_UH_ENABLE_SEL_MASK                  (0x3 << 14)
#define CODEC_CODEC_DAC_UH_ENABLE_SEL_SHIFT                 (14)
#define CODEC_CODEC_DAC_LH_ENABLE_SEL(n)                    (((n) & 0x3) << 16)
#define CODEC_CODEC_DAC_LH_ENABLE_SEL_MASK                  (0x3 << 16)
#define CODEC_CODEC_DAC_LH_ENABLE_SEL_SHIFT                 (16)
#define CODEC_GPIO_TRIGGER_DB_ENABLE                        (1 << 18)
#define CODEC_STAMP_CLR_USED                                (1 << 19)
#define CODEC_EVENT_SEL(n)                                  (((n) & 0x3) << 20)
#define CODEC_EVENT_SEL_MASK                                (0x3 << 20)
#define CODEC_EVENT_SEL_SHIFT                               (20)
#define CODEC_EVENT_FOR_CAPTURE                             (1 << 22)
#define CODEC_TEST_PORT_SEL(n)                              (((n) & 0x7) << 23)
#define CODEC_TEST_PORT_SEL_MASK                            (0x7 << 23)
#define CODEC_TEST_PORT_SEL_SHIFT                           (23)
#define CODEC_PLL_OSC_TRIGGER_SEL(n)                        (((n) & 0x3) << 26)
#define CODEC_PLL_OSC_TRIGGER_SEL_MASK                      (0x3 << 26)
#define CODEC_PLL_OSC_TRIGGER_SEL_SHIFT                     (26)
#define CODEC_FAULT_MUTE_DAC_ENABLE                         (1 << 28)
#define CODEC_FAULT_MUTE_DAC_ENABLE_SND                     (1 << 29)
#define CODEC_MODE_HCLK_ACCESS_REG                          (1 << 30)

// reg_060
#define CODEC_EN_CLK_ADC_ANA(n)                             (((n) & 0x7) << 0)
#define CODEC_EN_CLK_ADC_ANA_MASK                           (0x7 << 0)
#define CODEC_EN_CLK_ADC_ANA_SHIFT                          (0)
#define CODEC_EN_CLK_ADC(n)                                 (((n) & 0x1F) << 3)
#define CODEC_EN_CLK_ADC_MASK                               (0x1F << 3)
#define CODEC_EN_CLK_ADC_SHIFT                              (3)
#define CODEC_EN_CLK_DAC                                    (1 << 8)
#define CODEC_EN_CLK_IIR(n)                                 (((n) & 0x3) << 9)
#define CODEC_EN_CLK_IIR_MASK                               (0x3 << 9)
#define CODEC_EN_CLK_IIR_SHIFT                              (9)
#define CODEC_EN_CLK_RS(n)                                  (((n) & 0x3) << 11)
#define CODEC_EN_CLK_RS_MASK                                (0x3 << 11)
#define CODEC_EN_CLK_RS_SHIFT                               (11)
#define CODEC_POL_ADC_ANA(n)                                (((n) & 0x7) << 13)
#define CODEC_POL_ADC_ANA_MASK                              (0x7 << 13)
#define CODEC_POL_ADC_ANA_SHIFT                             (13)
#define CODEC_POL_DAC_OUT                                   (1 << 16)
#define CODEC_CFG_CLK_OUT(n)                                (((n) & 0xF) << 17)
#define CODEC_CFG_CLK_OUT_MASK                              (0xF << 17)
#define CODEC_CFG_CLK_OUT_SHIFT                             (17)

// reg_064
#define CODEC_SOFT_RSTN_ADC_ANA(n)                          (((n) & 0x7) << 0)
#define CODEC_SOFT_RSTN_ADC_ANA_MASK                        (0x7 << 0)
#define CODEC_SOFT_RSTN_ADC_ANA_SHIFT                       (0)
#define CODEC_SOFT_RSTN_ADC(n)                              (((n) & 0x1F) << 3)
#define CODEC_SOFT_RSTN_ADC_MASK                            (0x1F << 3)
#define CODEC_SOFT_RSTN_ADC_SHIFT                           (3)
#define CODEC_SOFT_RSTN_DAC                                 (1 << 8)
#define CODEC_SOFT_RSTN_RS(n)                               (((n) & 0x3) << 9)
#define CODEC_SOFT_RSTN_RS_MASK                             (0x3 << 9)
#define CODEC_SOFT_RSTN_RS_SHIFT                            (9)
#define CODEC_SOFT_RSTN_RS1                                 (1 << 11)
#define CODEC_SOFT_RSTN_IIR(n)                              (((n) & 0x3) << 12)
#define CODEC_SOFT_RSTN_IIR_MASK                            (0x3 << 12)
#define CODEC_SOFT_RSTN_IIR_SHIFT                           (12)
#define CODEC_SOFT_RSTN_IIR1                                (1 << 14)
#define CODEC_SOFT_RSTN_32K                                 (1 << 15)

// reg_068
#define CODEC_RET1N_RF                                      (1 << 0)
#define CODEC_RET2N_RF                                      (1 << 1)
#define CODEC_PGEN_RF                                       (1 << 2)
#define CODEC_MS_RF(n)                                      (((n) & 0xF) << 3)
#define CODEC_MS_RF_MASK                                    (0xF << 3)
#define CODEC_MS_RF_SHIFT                                   (3)
#define CODEC_MSE_RF                                        (1 << 7)
#define CODEC_WMS_RF(n)                                     (((n) & 0xF) << 8)
#define CODEC_WMS_RF_MASK                                   (0xF << 8)
#define CODEC_WMS_RF_SHIFT                                  (8)
#define CODEC_WMSE_RF                                       (1 << 12)
#define CODEC_RET1N_SRAM                                    (1 << 13)
#define CODEC_RET2N_SRAM                                    (1 << 14)
#define CODEC_PGEN_SRAM                                     (1 << 15)
#define CODEC_EMA_SRAM(n)                                   (((n) & 0x7) << 16)
#define CODEC_EMA_SRAM_MASK                                 (0x7 << 16)
#define CODEC_EMA_SRAM_SHIFT                                (16)
#define CODEC_EMAW_SRAM(n)                                  (((n) & 0x3) << 19)
#define CODEC_EMAW_SRAM_MASK                                (0x3 << 19)
#define CODEC_EMAW_SRAM_SHIFT                               (19)
#define CODEC_EMAS_SRAM                                     (1 << 21)
#define CODEC_WABL_SRAM                                     (1 << 22)
#define CODEC_WABLM_SRAM(n)                                 (((n) & 0x3) << 23)
#define CODEC_WABLM_SRAM_MASK                               (0x3 << 23)
#define CODEC_WABLM_SRAM_SHIFT                              (23)
#define CODEC_MS_ROM(n)                                     (((n) & 0xF) << 25)
#define CODEC_MS_ROM_MASK                                   (0xF << 25)
#define CODEC_MS_ROM_SHIFT                                  (25)
#define CODEC_MSE_ROM                                       (1 << 29)
#define CODEC_PGEN_ROM_0                                    (1 << 30)
#define CODEC_PGEN_ROM_1                                    (1 << 31)

// reg_06c
#define CODEC_TRIG_TIME(n)                                  (((n) & 0x3FFFFF) << 0)
#define CODEC_TRIG_TIME_MASK                                (0x3FFFFF << 0)
#define CODEC_TRIG_TIME_SHIFT                               (0)
#define CODEC_TRIG_TIME_ENABLE                              (1 << 22)
#define CODEC_TRIG_MODE                                     (1 << 23)
#define CODEC_GET_CNT_TRIG                                  (1 << 24)

// reg_078
#define CODEC_RESERVED_REG0(n)                              (((n) & 0xFFFFFFFF) << 0)
#define CODEC_RESERVED_REG0_MASK                            (0xFFFFFFFF << 0)
#define CODEC_RESERVED_REG0_SHIFT                           (0)

// reg_07c
#define CODEC_RESERVED_REG1(n)                              (((n) & 0xFFFFFFFF) << 0)
#define CODEC_RESERVED_REG1_MASK                            (0xFFFFFFFF << 0)
#define CODEC_RESERVED_REG1_SHIFT                           (0)

// reg_080
#define CODEC_CODEC_ADC_EN                                  (1 << 0)
#define CODEC_CODEC_ADC_EN_CH0                              (1 << 1)
#define CODEC_CODEC_ADC_EN_CH1                              (1 << 2)
#define CODEC_CODEC_ADC_EN_CH2                              (1 << 3)
#define CODEC_CODEC_ADC_EN_CH3                              (1 << 4)
#define CODEC_CODEC_SIDE_TONE_GAIN(n)                       (((n) & 0x1F) << 5)
#define CODEC_CODEC_SIDE_TONE_GAIN_MASK                     (0x1F << 5)
#define CODEC_CODEC_SIDE_TONE_GAIN_SHIFT                    (5)
#define CODEC_CODEC_SIDE_TONE_MIC_SEL(n)                    (((n) & 0x7) << 10)
#define CODEC_CODEC_SIDE_TONE_MIC_SEL_MASK                  (0x7 << 10)
#define CODEC_CODEC_SIDE_TONE_MIC_SEL_SHIFT                 (10)
#define CODEC_CODEC_SIDE_TONE_IIR_ENABLE                    (1 << 13)
#define CODEC_CODEC_ADC_LOOP                                (1 << 14)
#define CODEC_CODEC_LOOP_SEL_L(n)                           (((n) & 0x7) << 15)
#define CODEC_CODEC_LOOP_SEL_L_MASK                         (0x7 << 15)
#define CODEC_CODEC_LOOP_SEL_L_SHIFT                        (15)
#define CODEC_CODEC_CLKX2_EN                                (1 << 18)
#define CODEC_CODEC_ADC_UH_EN                               (1 << 19)
#define CODEC_CODEC_ADC_LH_EN                               (1 << 20)
#define CODEC_CODEC_TEST_PORT_SEL(n)                        (((n) & 0x1F) << 21)
#define CODEC_CODEC_TEST_PORT_SEL_MASK                      (0x1F << 21)
#define CODEC_CODEC_TEST_PORT_SEL_SHIFT                     (21)

// reg_084
#define CODEC_CODEC_ADC_SIGNED_CH0                          (1 << 0)
#define CODEC_CODEC_ADC_IN_SEL_CH0(n)                       (((n) & 0x7) << 1)
#define CODEC_CODEC_ADC_IN_SEL_CH0_MASK                     (0x7 << 1)
#define CODEC_CODEC_ADC_IN_SEL_CH0_SHIFT                    (1)
#define CODEC_CODEC_ADC_DOWN_SEL_CH0(n)                     (((n) & 0x3) << 4)
#define CODEC_CODEC_ADC_DOWN_SEL_CH0_MASK                   (0x3 << 4)
#define CODEC_CODEC_ADC_DOWN_SEL_CH0_SHIFT                  (4)
#define CODEC_CODEC_ADC_HBF3_BYPASS_CH0                     (1 << 6)
#define CODEC_CODEC_ADC_HBF2_BYPASS_CH0                     (1 << 7)
#define CODEC_CODEC_ADC_HBF1_BYPASS_CH0                     (1 << 8)
#define CODEC_CODEC_ADC_GAIN_SEL_CH0                        (1 << 9)
#define CODEC_CODEC_ADC_GAIN_CH0(n)                         (((n) & 0xFFFFF) << 10)
#define CODEC_CODEC_ADC_GAIN_CH0_MASK                       (0xFFFFF << 10)
#define CODEC_CODEC_ADC_GAIN_CH0_SHIFT                      (10)
#define CODEC_CODEC_ADC_HBF3_SEL_CH0(n)                     (((n) & 0x3) << 30)
#define CODEC_CODEC_ADC_HBF3_SEL_CH0_MASK                   (0x3 << 30)
#define CODEC_CODEC_ADC_HBF3_SEL_CH0_SHIFT                  (30)

// reg_088
#define CODEC_CODEC_ADC_SIGNED_CH1                          (1 << 0)
#define CODEC_CODEC_ADC_IN_SEL_CH1(n)                       (((n) & 0x7) << 1)
#define CODEC_CODEC_ADC_IN_SEL_CH1_MASK                     (0x7 << 1)
#define CODEC_CODEC_ADC_IN_SEL_CH1_SHIFT                    (1)
#define CODEC_CODEC_ADC_DOWN_SEL_CH1(n)                     (((n) & 0x3) << 4)
#define CODEC_CODEC_ADC_DOWN_SEL_CH1_MASK                   (0x3 << 4)
#define CODEC_CODEC_ADC_DOWN_SEL_CH1_SHIFT                  (4)
#define CODEC_CODEC_ADC_HBF3_BYPASS_CH1                     (1 << 6)
#define CODEC_CODEC_ADC_HBF2_BYPASS_CH1                     (1 << 7)
#define CODEC_CODEC_ADC_HBF1_BYPASS_CH1                     (1 << 8)
#define CODEC_CODEC_ADC_GAIN_SEL_CH1                        (1 << 9)
#define CODEC_CODEC_ADC_GAIN_CH1(n)                         (((n) & 0xFFFFF) << 10)
#define CODEC_CODEC_ADC_GAIN_CH1_MASK                       (0xFFFFF << 10)
#define CODEC_CODEC_ADC_GAIN_CH1_SHIFT                      (10)
#define CODEC_CODEC_ADC_HBF3_SEL_CH1(n)                     (((n) & 0x3) << 30)
#define CODEC_CODEC_ADC_HBF3_SEL_CH1_MASK                   (0x3 << 30)
#define CODEC_CODEC_ADC_HBF3_SEL_CH1_SHIFT                  (30)

// reg_08c
#define CODEC_CODEC_ADC_SIGNED_CH2                          (1 << 0)
#define CODEC_CODEC_ADC_IN_SEL_CH2(n)                       (((n) & 0x7) << 1)
#define CODEC_CODEC_ADC_IN_SEL_CH2_MASK                     (0x7 << 1)
#define CODEC_CODEC_ADC_IN_SEL_CH2_SHIFT                    (1)
#define CODEC_CODEC_ADC_DOWN_SEL_CH2(n)                     (((n) & 0x3) << 4)
#define CODEC_CODEC_ADC_DOWN_SEL_CH2_MASK                   (0x3 << 4)
#define CODEC_CODEC_ADC_DOWN_SEL_CH2_SHIFT                  (4)
#define CODEC_CODEC_ADC_HBF3_BYPASS_CH2                     (1 << 6)
#define CODEC_CODEC_ADC_HBF2_BYPASS_CH2                     (1 << 7)
#define CODEC_CODEC_ADC_HBF1_BYPASS_CH2                     (1 << 8)
#define CODEC_CODEC_ADC_GAIN_SEL_CH2                        (1 << 9)
#define CODEC_CODEC_ADC_GAIN_CH2(n)                         (((n) & 0xFFFFF) << 10)
#define CODEC_CODEC_ADC_GAIN_CH2_MASK                       (0xFFFFF << 10)
#define CODEC_CODEC_ADC_GAIN_CH2_SHIFT                      (10)
#define CODEC_CODEC_ADC_HBF3_SEL_CH2(n)                     (((n) & 0x3) << 30)
#define CODEC_CODEC_ADC_HBF3_SEL_CH2_MASK                   (0x3 << 30)
#define CODEC_CODEC_ADC_HBF3_SEL_CH2_SHIFT                  (30)

// reg_094
#define CODEC_CODEC_ECHO_ENABLE_CH0                         (1 << 0)
#define CODEC_CODEC_ADC_ECHO_DOWN_SEL_CH0(n)                (((n) & 0x3) << 1)
#define CODEC_CODEC_ADC_ECHO_DOWN_SEL_CH0_MASK              (0x3 << 1)
#define CODEC_CODEC_ADC_ECHO_DOWN_SEL_CH0_SHIFT             (1)
#define CODEC_CODEC_ADC_ECHO_HBF3_BYPASS_CH0                (1 << 3)
#define CODEC_CODEC_ADC_ECHO_HBF2_BYPASS_CH0                (1 << 4)
#define CODEC_CODEC_ADC_ECHO_HBF1_BYPASS_CH0                (1 << 5)
#define CODEC_CODEC_ADC_ECHO_GAIN_SEL_CH0                   (1 << 6)
#define CODEC_CODEC_ADC_ECHO_GAIN_CH0(n)                    (((n) & 0xFFFFF) << 7)
#define CODEC_CODEC_ADC_ECHO_GAIN_CH0_MASK                  (0xFFFFF << 7)
#define CODEC_CODEC_ADC_ECHO_GAIN_CH0_SHIFT                 (7)

// reg_098
#define CODEC_CODEC_DAC_EN                                  (1 << 0)
#define CODEC_CODEC_DAC_EN_CH0                              (1 << 1)
#define CODEC_CODEC_DAC_DITHER_GAIN(n)                      (((n) & 0x1F) << 2)
#define CODEC_CODEC_DAC_DITHER_GAIN_MASK                    (0x1F << 2)
#define CODEC_CODEC_DAC_DITHER_GAIN_SHIFT                   (2)
#define CODEC_CODEC_DAC_SDM_GAIN(n)                         (((n) & 0x7) << 7)
#define CODEC_CODEC_DAC_SDM_GAIN_MASK                       (0x7 << 7)
#define CODEC_CODEC_DAC_SDM_GAIN_SHIFT                      (7)
#define CODEC_CODEC_DITHER_BYPASS                           (1 << 10)
#define CODEC_CODEC_DAC_HBF3_BYPASS                         (1 << 11)
#define CODEC_CODEC_DAC_HBF2_BYPASS                         (1 << 12)
#define CODEC_CODEC_DAC_HBF1_BYPASS                         (1 << 13)
#define CODEC_CODEC_DAC_UP_SEL(n)                           (((n) & 0x7) << 14)
#define CODEC_CODEC_DAC_UP_SEL_MASK                         (0x7 << 14)
#define CODEC_CODEC_DAC_UP_SEL_SHIFT                        (14)
#define CODEC_CODEC_DAC_TONE_TEST                           (1 << 17)
#define CODEC_CODEC_DAC_SIN1K_STEP(n)                       (((n) & 0xF) << 18)
#define CODEC_CODEC_DAC_SIN1K_STEP_MASK                     (0xF << 18)
#define CODEC_CODEC_DAC_SIN1K_STEP_SHIFT                    (18)
#define CODEC_CODEC_DAC_OSR_SEL(n)                          (((n) & 0x3) << 22)
#define CODEC_CODEC_DAC_OSR_SEL_MASK                        (0x3 << 22)
#define CODEC_CODEC_DAC_OSR_SEL_SHIFT                       (22)
#define CODEC_CODEC_DAC_LR_SWAP                             (1 << 24)
#define CODEC_CODEC_DAC_SDM_H4_6M_CH0                       (1 << 25)
#define CODEC_CODEC_DAC_L_FIR_UPSAMPLE                      (1 << 26)
#define CODEC_CODEC_DAC_USE_HBF4                            (1 << 27)
#define CODEC_CODEC_DAC_USE_HBF5                            (1 << 28)

// reg_09c
#define CODEC_CODEC_DAC_GAIN_CH0(n)                         (((n) & 0xFFFFF) << 0)
#define CODEC_CODEC_DAC_GAIN_CH0_MASK                       (0xFFFFF << 0)
#define CODEC_CODEC_DAC_GAIN_CH0_SHIFT                      (0)
#define CODEC_CODEC_DAC_GAIN_SEL_CH0                        (1 << 20)
#define CODEC_CODEC_DAC_GAIN_UPDATE                         (1 << 21)
#define CODEC_CODEC_ADC_GAIN_UPDATE_CH0                     (1 << 22)
#define CODEC_CODEC_ADC_GAIN_UPDATE_CH1                     (1 << 23)
#define CODEC_CODEC_ADC_GAIN_UPDATE_CH2                     (1 << 24)
#define CODEC_CODEC_ADC_GAIN_UPDATE_CH3                     (1 << 25)
#define CODEC_CODEC_ADC_ECHO_GAIN_UPDATE_CH0                (1 << 26)
#define CODEC_CODEC_DAC_GAIN_TRIGGER_SEL(n)                 (((n) & 0x3) << 27)
#define CODEC_CODEC_DAC_GAIN_TRIGGER_SEL_MASK               (0x3 << 27)
#define CODEC_CODEC_DAC_GAIN_TRIGGER_SEL_SHIFT              (27)

// reg_0a0
#define CODEC_CODEC_DAC_OUT_SWAP                            (1 << 0)
#define CODEC_CODEC_DAC_H4_DELAY_CH0(n)                     (((n) & 0x3) << 1)
#define CODEC_CODEC_DAC_H4_DELAY_CH0_MASK                   (0x3 << 1)
#define CODEC_CODEC_DAC_H4_DELAY_CH0_SHIFT                  (1)
#define CODEC_CODEC_DAC_L4_DELAY_CH0(n)                     (((n) & 0x3) << 3)
#define CODEC_CODEC_DAC_L4_DELAY_CH0_MASK                   (0x3 << 3)
#define CODEC_CODEC_DAC_L4_DELAY_CH0_SHIFT                  (3)
#define CODEC_CODEC_DAC_HBF4_DELAY_SEL                      (1 << 5)
#define CODEC_CODEC_DAC_MIX_MODE1                           (1 << 6)
#define CODEC_CODEC_DAC_MIX_MODE2                           (1 << 7)
#define CODEC_CODEC_DAC_ECHO_DATA_SEL(n)                    (((n) & 0x3) << 8)
#define CODEC_CODEC_DAC_ECHO_DATA_SEL_MASK                  (0x3 << 8)
#define CODEC_CODEC_DAC_ECHO_DATA_SEL_SHIFT                 (8)

// reg_0a4
#define CODEC_CODEC_PDM_ENABLE                              (1 << 0)
#define CODEC_CODEC_PDM_DATA_INV                            (1 << 1)
#define CODEC_CODEC_PDM_RATE_SEL(n)                         (((n) & 0x3) << 2)
#define CODEC_CODEC_PDM_RATE_SEL_MASK                       (0x3 << 2)
#define CODEC_CODEC_PDM_RATE_SEL_SHIFT                      (2)
#define CODEC_CODEC_PDM_ADC_SEL_CH0                         (1 << 4)
#define CODEC_CODEC_PDM_ADC_SEL_CH1                         (1 << 5)
#define CODEC_CODEC_PDM_ADC_SEL_CH2                         (1 << 6)
#define CODEC_CODEC_ADC_DCF_BYPASS_CH0                      (1 << 7)
#define CODEC_CODEC_ADC_DCF_BYPASS_CH1                      (1 << 8)
#define CODEC_CODEC_ADC_DCF_BYPASS_CH2                      (1 << 9)
#define CODEC_CODEC_DAC_SDM_CLOSE                           (1 << 10)
#define CODEC_CODEC_DITHERF_BYPASS                          (1 << 11)
#define CODEC_CODEC_DAC_DITHERF_GAIN(n)                     (((n) & 0x3) << 12)
#define CODEC_CODEC_DAC_DITHERF_GAIN_MASK                   (0x3 << 12)
#define CODEC_CODEC_DAC_DITHERF_GAIN_SHIFT                  (12)
#define CODEC_CODEC_DAC_UH_EN                               (1 << 14)
#define CODEC_CODEC_DAC_LH_EN                               (1 << 15)
#define CODEC_CODEC_DAC_SDM_3RD_EN_CH0                      (1 << 16)
#define CODEC_EN_48KX64_MODE(n)                             (((n) & 0x3) << 17)
#define CODEC_EN_48KX64_MODE_MASK                           (0x3 << 17)
#define CODEC_EN_48KX64_MODE_SHIFT                          (17)

// reg_0a8
#define CODEC_CODEC_PDM_MUX_CH0(n)                          (((n) & 0x7) << 0)
#define CODEC_CODEC_PDM_MUX_CH0_MASK                        (0x7 << 0)
#define CODEC_CODEC_PDM_MUX_CH0_SHIFT                       (0)
#define CODEC_CODEC_PDM_MUX_CH1(n)                          (((n) & 0x7) << 3)
#define CODEC_CODEC_PDM_MUX_CH1_MASK                        (0x7 << 3)
#define CODEC_CODEC_PDM_MUX_CH1_SHIFT                       (3)
#define CODEC_CODEC_PDM_MUX_CH2(n)                          (((n) & 0x7) << 6)
#define CODEC_CODEC_PDM_MUX_CH2_MASK                        (0x7 << 6)
#define CODEC_CODEC_PDM_MUX_CH2_SHIFT                       (6)
#define CODEC_CODEC_PDM_CAP_PHASE_CH0(n)                    (((n) & 0x3) << 9)
#define CODEC_CODEC_PDM_CAP_PHASE_CH0_MASK                  (0x3 << 9)
#define CODEC_CODEC_PDM_CAP_PHASE_CH0_SHIFT                 (9)
#define CODEC_CODEC_PDM_CAP_PHASE_CH1(n)                    (((n) & 0x3) << 11)
#define CODEC_CODEC_PDM_CAP_PHASE_CH1_MASK                  (0x3 << 11)
#define CODEC_CODEC_PDM_CAP_PHASE_CH1_SHIFT                 (11)
#define CODEC_CODEC_PDM_CAP_PHASE_CH2(n)                    (((n) & 0x3) << 13)
#define CODEC_CODEC_PDM_CAP_PHASE_CH2_MASK                  (0x3 << 13)
#define CODEC_CODEC_PDM_CAP_PHASE_CH2_SHIFT                 (13)

// reg_0b0
#define CODEC_CODEC_CLASSG_EN                               (1 << 0)
#define CODEC_CODEC_CLASSG_QUICK_DOWN                       (1 << 1)
#define CODEC_CODEC_CLASSG_STEP_3_4N                        (1 << 2)
#define CODEC_CODEC_CLASSG_LR                               (1 << 3)
#define CODEC_CODEC_CLASSG_WINDOW(n)                        (((n) & 0xFFF) << 4)
#define CODEC_CODEC_CLASSG_WINDOW_MASK                      (0xFFF << 4)
#define CODEC_CODEC_CLASSG_WINDOW_SHIFT                     (4)

// reg_0b4
#define CODEC_CODEC_CLASSG_THD0(n)                          (((n) & 0xFF) << 0)
#define CODEC_CODEC_CLASSG_THD0_MASK                        (0xFF << 0)
#define CODEC_CODEC_CLASSG_THD0_SHIFT                       (0)
#define CODEC_CODEC_CLASSG_THD1(n)                          (((n) & 0xFF) << 8)
#define CODEC_CODEC_CLASSG_THD1_MASK                        (0xFF << 8)
#define CODEC_CODEC_CLASSG_THD1_SHIFT                       (8)
#define CODEC_CODEC_CLASSG_THD2(n)                          (((n) & 0xFF) << 16)
#define CODEC_CODEC_CLASSG_THD2_MASK                        (0xFF << 16)
#define CODEC_CODEC_CLASSG_THD2_SHIFT                       (16)

// reg_0bc
#define CODEC_CODEC_ADC_MC_EN_CH0                           (1 << 0)
#define CODEC_CODEC_FEEDBACK_MC_EN_CH0                      (1 << 1)
#define CODEC_CODEC_DAC_L_IIR_ENABLE                        (1 << 2)
#define CODEC_CODEC_ADC_CH0_IIR_ENABLE                      (1 << 3)
#define CODEC_CODEC_ADC_CH1_IIR_ENABLE                      (1 << 4)
#define CODEC_CODEC_ADC_CH2_IIR_ENABLE                      (1 << 5)
#define CODEC_CODEC_ADC_CH3_IIR_ENABLE                      (1 << 6)
#define CODEC_CODEC_FB_CHECK_UDC_CH0(n)                     (((n) & 0xF) << 7)
#define CODEC_CODEC_FB_CHECK_UDC_CH0_MASK                   (0xF << 7)
#define CODEC_CODEC_FB_CHECK_UDC_CH0_SHIFT                  (7)

// reg_0c0
#define CODEC_CODEC_DRE_ENABLE_CH0                          (1 << 0)
#define CODEC_CODEC_DRE_STEP_MODE_CH0(n)                    (((n) & 0x7) << 1)
#define CODEC_CODEC_DRE_STEP_MODE_CH0_MASK                  (0x7 << 1)
#define CODEC_CODEC_DRE_STEP_MODE_CH0_SHIFT                 (1)
#define CODEC_CODEC_DRE_INI_ANA_GAIN_CH0(n)                 (((n) & 0xF) << 4)
#define CODEC_CODEC_DRE_INI_ANA_GAIN_CH0_MASK               (0xF << 4)
#define CODEC_CODEC_DRE_INI_ANA_GAIN_CH0_SHIFT              (4)
#define CODEC_CODEC_DRE_DELAY_CH0(n)                        (((n) & 0xFF) << 8)
#define CODEC_CODEC_DRE_DELAY_CH0_MASK                      (0xFF << 8)
#define CODEC_CODEC_DRE_DELAY_CH0_SHIFT                     (8)
#define CODEC_CODEC_DRE_AMP_HIGH_CH0(n)                     (((n) & 0xFFFF) << 16)
#define CODEC_CODEC_DRE_AMP_HIGH_CH0_MASK                   (0xFFFF << 16)
#define CODEC_CODEC_DRE_AMP_HIGH_CH0_SHIFT                  (16)

// reg_0c4
#define CODEC_CODEC_DRE_WINDOW_CH0(n)                       (((n) & 0x1FFFFF) << 0)
#define CODEC_CODEC_DRE_WINDOW_CH0_MASK                     (0x1FFFFF << 0)
#define CODEC_CODEC_DRE_WINDOW_CH0_SHIFT                    (0)
#define CODEC_CODEC_DRE_THD_DB_OFFSET_CH0(n)                (((n) & 0xF) << 21)
#define CODEC_CODEC_DRE_THD_DB_OFFSET_CH0_MASK              (0xF << 21)
#define CODEC_CODEC_DRE_THD_DB_OFFSET_CH0_SHIFT             (21)
#define CODEC_CODEC_DRE_THD_DB_OFFSET_SIGN_CH0              (1 << 25)
#define CODEC_CODEC_DRE_GAIN_OFFSET_CH0(n)                  (((n) & 0x1F) << 26)
#define CODEC_CODEC_DRE_GAIN_OFFSET_CH0_MASK                (0x1F << 26)
#define CODEC_CODEC_DRE_GAIN_OFFSET_CH0_SHIFT               (26)

// reg_0d0
#define CODEC_CODEC_ANC_ENABLE_CH0                          (1 << 0)
#define CODEC_CODEC_DUAL_ANC_CH0                            (1 << 1)
#define CODEC_CODEC_ANC_MUTE_CH0                            (1 << 2)
#define CODEC_CODEC_ANC_RATE_SEL                            (1 << 3)
#define CODEC_CODEC_ANC_FF_SR_SEL(n)                        (((n) & 0x3) << 4)
#define CODEC_CODEC_ANC_FF_SR_SEL_MASK                      (0x3 << 4)
#define CODEC_CODEC_ANC_FF_SR_SEL_SHIFT                     (4)
#define CODEC_CODEC_ANC_FF_IN_PHASE_SEL(n)                  (((n) & 0x7) << 6)
#define CODEC_CODEC_ANC_FF_IN_PHASE_SEL_MASK                (0x7 << 6)
#define CODEC_CODEC_ANC_FF_IN_PHASE_SEL_SHIFT               (6)
#define CODEC_CODEC_ANC_FB_SR_SEL(n)                        (((n) & 0x3) << 9)
#define CODEC_CODEC_ANC_FB_SR_SEL_MASK                      (0x3 << 9)
#define CODEC_CODEC_ANC_FB_SR_SEL_SHIFT                     (9)
#define CODEC_CODEC_ANC_FB_IN_PHASE_SEL(n)                  (((n) & 0x7) << 11)
#define CODEC_CODEC_ANC_FB_IN_PHASE_SEL_MASK                (0x7 << 11)
#define CODEC_CODEC_ANC_FB_IN_PHASE_SEL_SHIFT               (11)
#define CODEC_CODEC_FEEDBACK_CH0                            (1 << 14)
#define CODEC_CODEC_ANC_MUSIC_CH_SEL                        (1 << 15)

// reg_0d4
#define CODEC_CODEC_ANC_MUTE_GAIN_FF_CH0(n)                 (((n) & 0xFFF) << 0)
#define CODEC_CODEC_ANC_MUTE_GAIN_FF_CH0_MASK               (0xFFF << 0)
#define CODEC_CODEC_ANC_MUTE_GAIN_FF_CH0_SHIFT              (0)
#define CODEC_CODEC_ANC_MUTE_GAIN_PASS0_FF_CH0              (1 << 12)
#define CODEC_CODEC_ANC_MUTE_GAIN_UPDATE_FF_CH0             (1 << 13)

// reg_0d8
#define CODEC_CODEC_ANC_MUTE_GAIN_FB_CH0(n)                 (((n) & 0xFFF) << 0)
#define CODEC_CODEC_ANC_MUTE_GAIN_FB_CH0_MASK               (0xFFF << 0)
#define CODEC_CODEC_ANC_MUTE_GAIN_FB_CH0_SHIFT              (0)
#define CODEC_CODEC_ANC_MUTE_GAIN_PASS0_FB_CH0              (1 << 12)
#define CODEC_CODEC_ANC_MUTE_GAIN_UPDATE_FB_CH0             (1 << 13)

// reg_0dc
#define CODEC_CODEC_ADC_IIR_CH0_SEL(n)                      (((n) & 0xF) << 0)
#define CODEC_CODEC_ADC_IIR_CH0_SEL_MASK                    (0xF << 0)
#define CODEC_CODEC_ADC_IIR_CH0_SEL_SHIFT                   (0)
#define CODEC_CODEC_ADC_IIR_CH1_SEL(n)                      (((n) & 0xF) << 4)
#define CODEC_CODEC_ADC_IIR_CH1_SEL_MASK                    (0xF << 4)
#define CODEC_CODEC_ADC_IIR_CH1_SEL_SHIFT                   (4)
#define CODEC_CODEC_ADC_IIR_CH2_SEL(n)                      (((n) & 0xF) << 8)
#define CODEC_CODEC_ADC_IIR_CH2_SEL_MASK                    (0xF << 8)
#define CODEC_CODEC_ADC_IIR_CH2_SEL_SHIFT                   (8)
#define CODEC_CODEC_ADC_IIR_CH3_SEL(n)                      (((n) & 0xF) << 12)
#define CODEC_CODEC_ADC_IIR_CH3_SEL_MASK                    (0xF << 12)
#define CODEC_CODEC_ADC_IIR_CH3_SEL_SHIFT                   (12)

// reg_0e0
#define CODEC_CODEC_DAC_DC_CH0(n)                           (((n) & 0x7FFFF) << 0)
#define CODEC_CODEC_DAC_DC_CH0_MASK                         (0x7FFFF << 0)
#define CODEC_CODEC_DAC_DC_CH0_SHIFT                        (0)
#define CODEC_CODEC_DAC_DC_UPDATE_CH0                       (1 << 19)
#define CODEC_CODEC_DAC_ANA_GAIN_UPDATE_DELAY_CH0(n)        (((n) & 0xFF) << 20)
#define CODEC_CODEC_DAC_ANA_GAIN_UPDATE_DELAY_CH0_MASK      (0xFF << 20)
#define CODEC_CODEC_DAC_ANA_GAIN_UPDATE_DELAY_CH0_SHIFT     (20)
#define CODEC_CODEC_DAC_DC_UPDATE_PASS0_CH0                 (1 << 28)
#define CODEC_CODEC_DAC_DC_UPDATE_STATUS_CH0                (1 << 29)

// reg_0e4
#define CODEC_CODEC_RESAMPLE_DAC_ENABLE                     (1 << 0)
#define CODEC_CODEC_RESAMPLE_DAC_L_ENABLE                   (1 << 1)
#define CODEC_CODEC_RESAMPLE_DAC_FIFO_ENABLE                (1 << 2)
#define CODEC_CODEC_RESAMPLE_ADC_ENABLE                     (1 << 3)
#define CODEC_CODEC_RESAMPLE_ADC_CH_CNT(n)                  (((n) & 0x7) << 4)
#define CODEC_CODEC_RESAMPLE_ADC_CH_CNT_MASK                (0x7 << 4)
#define CODEC_CODEC_RESAMPLE_ADC_CH_CNT_SHIFT               (4)
#define CODEC_CODEC_RESAMPLE_DAC_PHASE_UPDATE               (1 << 7)
#define CODEC_CODEC_RESAMPLE_DAC_UPDATE_TRIGGER_SEL(n)      (((n) & 0x3) << 8)
#define CODEC_CODEC_RESAMPLE_DAC_UPDATE_TRIGGER_SEL_MASK    (0x3 << 8)
#define CODEC_CODEC_RESAMPLE_DAC_UPDATE_TRIGGER_SEL_SHIFT   (8)
#define CODEC_CODEC_RESAMPLE_ADC_PHASE_UPDATE               (1 << 10)
#define CODEC_CODEC_RESAMPLE_ADC_UPDATE_TRIGGER_SEL(n)      (((n) & 0x3) << 11)
#define CODEC_CODEC_RESAMPLE_ADC_UPDATE_TRIGGER_SEL_MASK    (0x3 << 11)
#define CODEC_CODEC_RESAMPLE_ADC_UPDATE_TRIGGER_SEL_SHIFT   (11)
#define CODEC_CODEC_RESAMPLE_DAC_ENABLE_TRIGGER_SEL(n)      (((n) & 0x3) << 13)
#define CODEC_CODEC_RESAMPLE_DAC_ENABLE_TRIGGER_SEL_MASK    (0x3 << 13)
#define CODEC_CODEC_RESAMPLE_DAC_ENABLE_TRIGGER_SEL_SHIFT   (13)
#define CODEC_CODEC_RESAMPLE_ADC_ENABLE_TRIGGER_SEL(n)      (((n) & 0x3) << 15)
#define CODEC_CODEC_RESAMPLE_ADC_ENABLE_TRIGGER_SEL_MASK    (0x3 << 15)
#define CODEC_CODEC_RESAMPLE_ADC_ENABLE_TRIGGER_SEL_SHIFT   (15)
#define CODEC_CODEC_ADC_REMAP_ENABLE                        (1 << 17)
#define CODEC_CODEC_RESAMPLE_DAC_ENABLE_SND                 (1 << 18)
#define CODEC_CODEC_RESAMPLE_DAC_L_ENABLE_SND               (1 << 19)
#define CODEC_CODEC_RESAMPLE_DAC_FIFO_ENABLE_SND            (1 << 20)
#define CODEC_CODEC_RESAMPLE_DAC_PHASE_UPDATE_SND           (1 << 21)
#define CODEC_CODEC_RESAMPLE_DAC_UPDATE_TRIGGER_SEL_SND(n)  (((n) & 0x3) << 22)
#define CODEC_CODEC_RESAMPLE_DAC_UPDATE_TRIGGER_SEL_SND_MASK (0x3 << 22)
#define CODEC_CODEC_RESAMPLE_DAC_UPDATE_TRIGGER_SEL_SND_SHIFT (22)
#define CODEC_CODEC_RESAMPLE_DAC_ENABLE_TRIGGER_SEL_SND(n)  (((n) & 0x3) << 24)
#define CODEC_CODEC_RESAMPLE_DAC_ENABLE_TRIGGER_SEL_SND_MASK (0x3 << 24)
#define CODEC_CODEC_RESAMPLE_DAC_ENABLE_TRIGGER_SEL_SND_SHIFT (24)

// reg_0ec
#define CODEC_CODEC_RAMP_STEP_CH0(n)                        (((n) & 0xFFF) << 0)
#define CODEC_CODEC_RAMP_STEP_CH0_MASK                      (0xFFF << 0)
#define CODEC_CODEC_RAMP_STEP_CH0_SHIFT                     (0)
#define CODEC_CODEC_RAMP_EN_CH0                             (1 << 12)
#define CODEC_CODEC_RAMP_INTERVAL_CH0(n)                    (((n) & 0x7) << 13)
#define CODEC_CODEC_RAMP_INTERVAL_CH0_MASK                  (0x7 << 13)
#define CODEC_CODEC_RAMP_INTERVAL_CH0_SHIFT                 (13)

// reg_0f0
#define CODEC_CODEC_RESAMPLE_DAC_PHASE_INC_SND(n)           (((n) & 0xFFFFFFFF) << 0)
#define CODEC_CODEC_RESAMPLE_DAC_PHASE_INC_SND_MASK         (0xFFFFFFFF << 0)
#define CODEC_CODEC_RESAMPLE_DAC_PHASE_INC_SND_SHIFT        (0)

// reg_0f4
#define CODEC_CODEC_RESAMPLE_DAC_PHASE_INC(n)               (((n) & 0xFFFFFFFF) << 0)
#define CODEC_CODEC_RESAMPLE_DAC_PHASE_INC_MASK             (0xFFFFFFFF << 0)
#define CODEC_CODEC_RESAMPLE_DAC_PHASE_INC_SHIFT            (0)

// reg_0f8
#define CODEC_CODEC_RESAMPLE_ADC_PHASE_INC(n)               (((n) & 0xFFFFFFFF) << 0)
#define CODEC_CODEC_RESAMPLE_ADC_PHASE_INC_MASK             (0xFFFFFFFF << 0)
#define CODEC_CODEC_RESAMPLE_ADC_PHASE_INC_SHIFT            (0)

// reg_0fc
#define CODEC_CODEC_RESAMPLE_ADC_CH0_SEL(n)                 (((n) & 0x3) << 0)
#define CODEC_CODEC_RESAMPLE_ADC_CH0_SEL_MASK               (0x3 << 0)
#define CODEC_CODEC_RESAMPLE_ADC_CH0_SEL_SHIFT              (0)
#define CODEC_CODEC_RESAMPLE_ADC_CH1_SEL(n)                 (((n) & 0x3) << 2)
#define CODEC_CODEC_RESAMPLE_ADC_CH1_SEL_MASK               (0x3 << 2)
#define CODEC_CODEC_RESAMPLE_ADC_CH1_SEL_SHIFT              (2)
#define CODEC_CODEC_RESAMPLE_ADC_CH2_SEL(n)                 (((n) & 0x3) << 4)
#define CODEC_CODEC_RESAMPLE_ADC_CH2_SEL_MASK               (0x3 << 4)
#define CODEC_CODEC_RESAMPLE_ADC_CH2_SEL_SHIFT              (4)
#define CODEC_CODEC_RESAMPLE_ADC_CH3_SEL(n)                 (((n) & 0x3) << 6)
#define CODEC_CODEC_RESAMPLE_ADC_CH3_SEL_MASK               (0x3 << 6)
#define CODEC_CODEC_RESAMPLE_ADC_CH3_SEL_SHIFT              (6)
#define CODEC_CODEC_RESAMPLE_ADC_CH0_ECHO_SEL               (1 << 8)
#define CODEC_CODEC_RESAMPLE_ADC_CH1_ECHO_SEL               (1 << 9)
#define CODEC_CODEC_RESAMPLE_ADC_CH2_ECHO_SEL               (1 << 10)
#define CODEC_CODEC_RESAMPLE_ADC_CH3_ECHO_SEL               (1 << 11)
#define CODEC_CODEC_RESAMPLE_ADC_CH0_ECHO                   (1 << 12)
#define CODEC_CODEC_RESAMPLE_ADC_CH1_ECHO                   (1 << 13)
#define CODEC_CODEC_RESAMPLE_ADC_CH2_ECHO                   (1 << 14)
#define CODEC_CODEC_RESAMPLE_ADC_CH3_ECHO                   (1 << 15)

// reg_130
#define CODEC_CODEC_FB_CHECK_ENABLE_CH0                     (1 << 0)
#define CODEC_CODEC_FB_CHECK_ACC_SAMPLE_RATE_CH0(n)         (((n) & 0x3) << 1)
#define CODEC_CODEC_FB_CHECK_ACC_SAMPLE_RATE_CH0_MASK       (0x3 << 1)
#define CODEC_CODEC_FB_CHECK_ACC_SAMPLE_RATE_CH0_SHIFT      (1)
#define CODEC_CODEC_FB_CHECK_SRC_SEL_CH0(n)                 (((n) & 0x3) << 3)
#define CODEC_CODEC_FB_CHECK_SRC_SEL_CH0_MASK               (0x3 << 3)
#define CODEC_CODEC_FB_CHECK_SRC_SEL_CH0_SHIFT              (3)
#define CODEC_CODEC_FB_CHECK_KEEP_SEL_CH0                   (1 << 5)
#define CODEC_CODEC_FB_CHECK_ACC_WINDOW_CH0(n)              (((n) & 0xFFF) << 6)
#define CODEC_CODEC_FB_CHECK_ACC_WINDOW_CH0_MASK            (0xFFF << 6)
#define CODEC_CODEC_FB_CHECK_ACC_WINDOW_CH0_SHIFT           (6)
#define CODEC_CODEC_FB_CHECK_TRIG_WINDOW_CH0(n)             (((n) & 0x3FF) << 18)
#define CODEC_CODEC_FB_CHECK_TRIG_WINDOW_CH0_MASK           (0x3FF << 18)
#define CODEC_CODEC_FB_CHECK_TRIG_WINDOW_CH0_SHIFT          (18)
#define CODEC_CODEC_FB_CHECK_KEEP_CH0                       (1 << 28)
#define CODEC_CODEC_FB_CHECK_DCF_BYPASS_CH0                 (1 << 29)

// reg_138
#define CODEC_CODEC_FB_CHECK_THRESHOLD_CH0(n)               (((n) & 0xFFFFFFFF) << 0)
#define CODEC_CODEC_FB_CHECK_THRESHOLD_CH0_MASK             (0xFFFFFFFF << 0)
#define CODEC_CODEC_FB_CHECK_THRESHOLD_CH0_SHIFT            (0)

// reg_140
#define CODEC_CODEC_FB_CHECK_DATA_AVG_KEEP_CH0(n)           (((n) & 0xFFFFFFFF) << 0)
#define CODEC_CODEC_FB_CHECK_DATA_AVG_KEEP_CH0_MASK         (0xFFFFFFFF << 0)
#define CODEC_CODEC_FB_CHECK_DATA_AVG_KEEP_CH0_SHIFT        (0)

// reg_174
#define CODEC_CODEC_ADC_DC_DOUT_CH0_SYNC(n)                 (((n) & 0x1FFFFF) << 0)
#define CODEC_CODEC_ADC_DC_DOUT_CH0_SYNC_MASK               (0x1FFFFF << 0)
#define CODEC_CODEC_ADC_DC_DOUT_CH0_SYNC_SHIFT              (0)

// reg_178
#define CODEC_CODEC_ADC_DC_DOUT_CH1_SYNC(n)                 (((n) & 0x1FFFFF) << 0)
#define CODEC_CODEC_ADC_DC_DOUT_CH1_SYNC_MASK               (0x1FFFFF << 0)
#define CODEC_CODEC_ADC_DC_DOUT_CH1_SYNC_SHIFT              (0)

// reg_17c
#define CODEC_CODEC_ADC_DC_DOUT_CH2_SYNC(n)                 (((n) & 0x1FFFFF) << 0)
#define CODEC_CODEC_ADC_DC_DOUT_CH2_SYNC_MASK               (0x1FFFFF << 0)
#define CODEC_CODEC_ADC_DC_DOUT_CH2_SYNC_SHIFT              (0)

// reg_184
#define CODEC_CODEC_ADC_DC_DIN_CH0(n)                       (((n) & 0x7FFF) << 0)
#define CODEC_CODEC_ADC_DC_DIN_CH0_MASK                     (0x7FFF << 0)
#define CODEC_CODEC_ADC_DC_DIN_CH0_SHIFT                    (0)
#define CODEC_CODEC_ADC_DC_UPDATE_CH0                       (1 << 15)
#define CODEC_CODEC_ADC_DC_SEL                              (1 << 16)

// reg_188
#define CODEC_CODEC_ADC_DC_DIN_CH1(n)                       (((n) & 0x7FFF) << 0)
#define CODEC_CODEC_ADC_DC_DIN_CH1_MASK                     (0x7FFF << 0)
#define CODEC_CODEC_ADC_DC_DIN_CH1_SHIFT                    (0)
#define CODEC_CODEC_ADC_DC_UPDATE_CH1                       (1 << 15)

// reg_18c
#define CODEC_CODEC_ADC_DC_DIN_CH2(n)                       (((n) & 0x7FFF) << 0)
#define CODEC_CODEC_ADC_DC_DIN_CH2_MASK                     (0x7FFF << 0)
#define CODEC_CODEC_ADC_DC_DIN_CH2_SHIFT                    (0)
#define CODEC_CODEC_ADC_DC_UPDATE_CH2                       (1 << 15)

// reg_198
#define CODEC_CODEC_ADC_UDC_CH0(n)                          (((n) & 0xF) << 0)
#define CODEC_CODEC_ADC_UDC_CH0_MASK                        (0xF << 0)
#define CODEC_CODEC_ADC_UDC_CH0_SHIFT                       (0)
#define CODEC_CODEC_ADC_UDC_CH1(n)                          (((n) & 0xF) << 4)
#define CODEC_CODEC_ADC_UDC_CH1_MASK                        (0xF << 4)
#define CODEC_CODEC_ADC_UDC_CH1_SHIFT                       (4)
#define CODEC_CODEC_ADC_UDC_CH2(n)                          (((n) & 0xF) << 8)
#define CODEC_CODEC_ADC_UDC_CH2_MASK                        (0xF << 8)
#define CODEC_CODEC_ADC_UDC_CH2_SHIFT                       (8)

// reg_19c
#define CODEC_CODEC_DRE_DB_HIGH_CH0(n)                      (((n) & 0x3F) << 0)
#define CODEC_CODEC_DRE_DB_HIGH_CH0_MASK                    (0x3F << 0)
#define CODEC_CODEC_DRE_DB_HIGH_CH0_SHIFT                   (0)
#define CODEC_CODEC_DRE_DB_LOW_CH0(n)                       (((n) & 0x3F) << 6)
#define CODEC_CODEC_DRE_DB_LOW_CH0_MASK                     (0x3F << 6)
#define CODEC_CODEC_DRE_DB_LOW_CH0_SHIFT                    (6)
#define CODEC_CODEC_DRE_GAIN_TOP_CH0(n)                     (((n) & 0xF) << 12)
#define CODEC_CODEC_DRE_GAIN_TOP_CH0_MASK                   (0xF << 12)
#define CODEC_CODEC_DRE_GAIN_TOP_CH0_SHIFT                  (12)
#define CODEC_CODEC_DRE_DELAY_DC_CH0(n)                     (((n) & 0xFF) << 16)
#define CODEC_CODEC_DRE_DELAY_DC_CH0_MASK                   (0xFF << 16)
#define CODEC_CODEC_DRE_DELAY_DC_CH0_SHIFT                  (16)

// reg_1a0
#define CODEC_CODEC_DAC_DRE_GAIN_STEP0_CH0(n)               (((n) & 0x3FFF) << 0)
#define CODEC_CODEC_DAC_DRE_GAIN_STEP0_CH0_MASK             (0x3FFF << 0)
#define CODEC_CODEC_DAC_DRE_GAIN_STEP0_CH0_SHIFT            (0)
#define CODEC_CODEC_DAC_DRE_GAIN_STEP1_CH0(n)               (((n) & 0x3FFF) << 14)
#define CODEC_CODEC_DAC_DRE_GAIN_STEP1_CH0_MASK             (0x3FFF << 14)
#define CODEC_CODEC_DAC_DRE_GAIN_STEP1_CH0_SHIFT            (14)

// reg_1a4
#define CODEC_CODEC_DAC_DRE_GAIN_STEP2_CH0(n)               (((n) & 0x3FFF) << 0)
#define CODEC_CODEC_DAC_DRE_GAIN_STEP2_CH0_MASK             (0x3FFF << 0)
#define CODEC_CODEC_DAC_DRE_GAIN_STEP2_CH0_SHIFT            (0)
#define CODEC_CODEC_DAC_DRE_GAIN_STEP3_CH0(n)               (((n) & 0x3FFF) << 14)
#define CODEC_CODEC_DAC_DRE_GAIN_STEP3_CH0_MASK             (0x3FFF << 14)
#define CODEC_CODEC_DAC_DRE_GAIN_STEP3_CH0_SHIFT            (14)

// reg_1a8
#define CODEC_CODEC_DAC_DRE_GAIN_STEP4_CH0(n)               (((n) & 0x3FFF) << 0)
#define CODEC_CODEC_DAC_DRE_GAIN_STEP4_CH0_MASK             (0x3FFF << 0)
#define CODEC_CODEC_DAC_DRE_GAIN_STEP4_CH0_SHIFT            (0)
#define CODEC_CODEC_DAC_DRE_GAIN_STEP5_CH0(n)               (((n) & 0x3FFF) << 14)
#define CODEC_CODEC_DAC_DRE_GAIN_STEP5_CH0_MASK             (0x3FFF << 14)
#define CODEC_CODEC_DAC_DRE_GAIN_STEP5_CH0_SHIFT            (14)

// reg_1ac
#define CODEC_CODEC_DAC_DRE_GAIN_STEP6_CH0(n)               (((n) & 0x3FFF) << 0)
#define CODEC_CODEC_DAC_DRE_GAIN_STEP6_CH0_MASK             (0x3FFF << 0)
#define CODEC_CODEC_DAC_DRE_GAIN_STEP6_CH0_SHIFT            (0)
#define CODEC_CODEC_DAC_DRE_GAIN_STEP7_CH0(n)               (((n) & 0x3FFF) << 14)
#define CODEC_CODEC_DAC_DRE_GAIN_STEP7_CH0_MASK             (0x3FFF << 14)
#define CODEC_CODEC_DAC_DRE_GAIN_STEP7_CH0_SHIFT            (14)

// reg_1b0
#define CODEC_CODEC_DAC_DRE_GAIN_STEP8_CH0(n)               (((n) & 0x3FFF) << 0)
#define CODEC_CODEC_DAC_DRE_GAIN_STEP8_CH0_MASK             (0x3FFF << 0)
#define CODEC_CODEC_DAC_DRE_GAIN_STEP8_CH0_SHIFT            (0)
#define CODEC_CODEC_DAC_DRE_GAIN_STEP9_CH0(n)               (((n) & 0x3FFF) << 14)
#define CODEC_CODEC_DAC_DRE_GAIN_STEP9_CH0_MASK             (0x3FFF << 14)
#define CODEC_CODEC_DAC_DRE_GAIN_STEP9_CH0_SHIFT            (14)

// reg_1b4
#define CODEC_CODEC_DAC_DRE_GAIN_STEP10_CH0(n)              (((n) & 0x3FFF) << 0)
#define CODEC_CODEC_DAC_DRE_GAIN_STEP10_CH0_MASK            (0x3FFF << 0)
#define CODEC_CODEC_DAC_DRE_GAIN_STEP10_CH0_SHIFT           (0)
#define CODEC_CODEC_DAC_DRE_GAIN_STEP11_CH0(n)              (((n) & 0x3FFF) << 14)
#define CODEC_CODEC_DAC_DRE_GAIN_STEP11_CH0_MASK            (0x3FFF << 14)
#define CODEC_CODEC_DAC_DRE_GAIN_STEP11_CH0_SHIFT           (14)

// reg_1b8
#define CODEC_CODEC_DAC_DRE_GAIN_STEP12_CH0(n)              (((n) & 0x3FFF) << 0)
#define CODEC_CODEC_DAC_DRE_GAIN_STEP12_CH0_MASK            (0x3FFF << 0)
#define CODEC_CODEC_DAC_DRE_GAIN_STEP12_CH0_SHIFT           (0)
#define CODEC_CODEC_DAC_DRE_GAIN_STEP13_CH0(n)              (((n) & 0x3FFF) << 14)
#define CODEC_CODEC_DAC_DRE_GAIN_STEP13_CH0_MASK            (0x3FFF << 14)
#define CODEC_CODEC_DAC_DRE_GAIN_STEP13_CH0_SHIFT           (14)

// reg_1bc
#define CODEC_CODEC_DAC_DRE_GAIN_STEP14_CH0(n)              (((n) & 0x3FFF) << 0)
#define CODEC_CODEC_DAC_DRE_GAIN_STEP14_CH0_MASK            (0x3FFF << 0)
#define CODEC_CODEC_DAC_DRE_GAIN_STEP14_CH0_SHIFT           (0)
#define CODEC_CODEC_DAC_DRE_GAIN_STEP15_CH0(n)              (((n) & 0x3FFF) << 14)
#define CODEC_CODEC_DAC_DRE_GAIN_STEP15_CH0_MASK            (0x3FFF << 14)
#define CODEC_CODEC_DAC_DRE_GAIN_STEP15_CH0_SHIFT           (14)

// reg_1c0
#define CODEC_CODEC_ADC_DRE_ENABLE_CH0                      (1 << 0)
#define CODEC_CODEC_ADC_DRE_STEP_MODE_CH0(n)                (((n) & 0x3) << 1)
#define CODEC_CODEC_ADC_DRE_STEP_MODE_CH0_MASK              (0x3 << 1)
#define CODEC_CODEC_ADC_DRE_STEP_MODE_CH0_SHIFT             (1)
#define CODEC_CODEC_ADC_DRE_THD_DB_OFFSET_CH0(n)            (((n) & 0xF) << 3)
#define CODEC_CODEC_ADC_DRE_THD_DB_OFFSET_CH0_MASK          (0xF << 3)
#define CODEC_CODEC_ADC_DRE_THD_DB_OFFSET_CH0_SHIFT         (3)
#define CODEC_CODEC_ADC_DRE_INI_ANA_GAIN_CH0(n)             (((n) & 0xF) << 7)
#define CODEC_CODEC_ADC_DRE_INI_ANA_GAIN_CH0_MASK           (0xF << 7)
#define CODEC_CODEC_ADC_DRE_INI_ANA_GAIN_CH0_SHIFT          (7)
#define CODEC_CODEC_ADC_DRE_DELAY_DIG_CH0(n)                (((n) & 0x7) << 11)
#define CODEC_CODEC_ADC_DRE_DELAY_DIG_CH0_MASK              (0x7 << 11)
#define CODEC_CODEC_ADC_DRE_DELAY_DIG_CH0_SHIFT             (11)
#define CODEC_CODEC_ADC_DRE_DELAY_ANA_CH0(n)                (((n) & 0x1F) << 14)
#define CODEC_CODEC_ADC_DRE_DELAY_ANA_CH0_MASK              (0x1F << 14)
#define CODEC_CODEC_ADC_DRE_DELAY_ANA_CH0_SHIFT             (14)
#define CODEC_CODEC_ADC_DRE_THD_DB_OFFSET_SIGN_CH0          (1 << 19)
#define CODEC_CODEC_ADC_DRE_BIT_SEL_CH0(n)                  (((n) & 0x3) << 20)
#define CODEC_CODEC_ADC_DRE_BIT_SEL_CH0_MASK                (0x3 << 20)
#define CODEC_CODEC_ADC_DRE_BIT_SEL_CH0_SHIFT               (20)
#define CODEC_CODEC_ADC_DRE_OVERFLOW_MUTE_EN_CH0            (1 << 22)
#define CODEC_CODEC_ADC_DRE_MUTE_MODE_CH0                   (1 << 23)
#define CODEC_CODEC_ADC_DRE_MUTE_RANGE_SEL_CH0(n)           (((n) & 0x3) << 24)
#define CODEC_CODEC_ADC_DRE_MUTE_RANGE_SEL_CH0_MASK         (0x3 << 24)
#define CODEC_CODEC_ADC_DRE_MUTE_RANGE_SEL_CH0_SHIFT        (24)
#define CODEC_CODEC_ADC_DRE_MUTE_STATUS_CH0                 (1 << 26)

// reg_1c4
#define CODEC_CODEC_ADC_DRE_AMP_HIGH_CH0(n)                 (((n) & 0x7FF) << 0)
#define CODEC_CODEC_ADC_DRE_AMP_HIGH_CH0_MASK               (0x7FF << 0)
#define CODEC_CODEC_ADC_DRE_AMP_HIGH_CH0_SHIFT              (0)
#define CODEC_CODEC_ADC_DRE_WINDOW_CH0(n)                   (((n) & 0xFFFFF) << 11)
#define CODEC_CODEC_ADC_DRE_WINDOW_CH0_MASK                 (0xFFFFF << 11)
#define CODEC_CODEC_ADC_DRE_WINDOW_CH0_SHIFT                (11)

// reg_1c8
#define CODEC_CODEC_ADC_DRE_ENABLE_CH1                      (1 << 0)
#define CODEC_CODEC_ADC_DRE_STEP_MODE_CH1(n)                (((n) & 0x3) << 1)
#define CODEC_CODEC_ADC_DRE_STEP_MODE_CH1_MASK              (0x3 << 1)
#define CODEC_CODEC_ADC_DRE_STEP_MODE_CH1_SHIFT             (1)
#define CODEC_CODEC_ADC_DRE_THD_DB_OFFSET_CH1(n)            (((n) & 0xF) << 3)
#define CODEC_CODEC_ADC_DRE_THD_DB_OFFSET_CH1_MASK          (0xF << 3)
#define CODEC_CODEC_ADC_DRE_THD_DB_OFFSET_CH1_SHIFT         (3)
#define CODEC_CODEC_ADC_DRE_INI_ANA_GAIN_CH1(n)             (((n) & 0xF) << 7)
#define CODEC_CODEC_ADC_DRE_INI_ANA_GAIN_CH1_MASK           (0xF << 7)
#define CODEC_CODEC_ADC_DRE_INI_ANA_GAIN_CH1_SHIFT          (7)
#define CODEC_CODEC_ADC_DRE_DELAY_DIG_CH1(n)                (((n) & 0x7) << 11)
#define CODEC_CODEC_ADC_DRE_DELAY_DIG_CH1_MASK              (0x7 << 11)
#define CODEC_CODEC_ADC_DRE_DELAY_DIG_CH1_SHIFT             (11)
#define CODEC_CODEC_ADC_DRE_DELAY_ANA_CH1(n)                (((n) & 0x1F) << 14)
#define CODEC_CODEC_ADC_DRE_DELAY_ANA_CH1_MASK              (0x1F << 14)
#define CODEC_CODEC_ADC_DRE_DELAY_ANA_CH1_SHIFT             (14)
#define CODEC_CODEC_ADC_DRE_THD_DB_OFFSET_SIGN_CH1          (1 << 19)
#define CODEC_CODEC_ADC_DRE_BIT_SEL_CH1(n)                  (((n) & 0x3) << 20)
#define CODEC_CODEC_ADC_DRE_BIT_SEL_CH1_MASK                (0x3 << 20)
#define CODEC_CODEC_ADC_DRE_BIT_SEL_CH1_SHIFT               (20)
#define CODEC_CODEC_ADC_DRE_OVERFLOW_MUTE_EN_CH1            (1 << 22)
#define CODEC_CODEC_ADC_DRE_MUTE_MODE_CH1                   (1 << 23)
#define CODEC_CODEC_ADC_DRE_MUTE_RANGE_SEL_CH1(n)           (((n) & 0x3) << 24)
#define CODEC_CODEC_ADC_DRE_MUTE_RANGE_SEL_CH1_MASK         (0x3 << 24)
#define CODEC_CODEC_ADC_DRE_MUTE_RANGE_SEL_CH1_SHIFT        (24)
#define CODEC_CODEC_ADC_DRE_MUTE_STATUS_CH1                 (1 << 26)

// reg_1cc
#define CODEC_CODEC_ADC_DRE_AMP_HIGH_CH1(n)                 (((n) & 0x7FF) << 0)
#define CODEC_CODEC_ADC_DRE_AMP_HIGH_CH1_MASK               (0x7FF << 0)
#define CODEC_CODEC_ADC_DRE_AMP_HIGH_CH1_SHIFT              (0)
#define CODEC_CODEC_ADC_DRE_WINDOW_CH1(n)                   (((n) & 0xFFFFF) << 11)
#define CODEC_CODEC_ADC_DRE_WINDOW_CH1_MASK                 (0xFFFFF << 11)
#define CODEC_CODEC_ADC_DRE_WINDOW_CH1_SHIFT                (11)

// reg_1d0
#define CODEC_CODEC_ADC_DRE_ENABLE_CH2                      (1 << 0)
#define CODEC_CODEC_ADC_DRE_STEP_MODE_CH2(n)                (((n) & 0x3) << 1)
#define CODEC_CODEC_ADC_DRE_STEP_MODE_CH2_MASK              (0x3 << 1)
#define CODEC_CODEC_ADC_DRE_STEP_MODE_CH2_SHIFT             (1)
#define CODEC_CODEC_ADC_DRE_THD_DB_OFFSET_CH2(n)            (((n) & 0xF) << 3)
#define CODEC_CODEC_ADC_DRE_THD_DB_OFFSET_CH2_MASK          (0xF << 3)
#define CODEC_CODEC_ADC_DRE_THD_DB_OFFSET_CH2_SHIFT         (3)
#define CODEC_CODEC_ADC_DRE_INI_ANA_GAIN_CH2(n)             (((n) & 0xF) << 7)
#define CODEC_CODEC_ADC_DRE_INI_ANA_GAIN_CH2_MASK           (0xF << 7)
#define CODEC_CODEC_ADC_DRE_INI_ANA_GAIN_CH2_SHIFT          (7)
#define CODEC_CODEC_ADC_DRE_DELAY_DIG_CH2(n)                (((n) & 0x7) << 11)
#define CODEC_CODEC_ADC_DRE_DELAY_DIG_CH2_MASK              (0x7 << 11)
#define CODEC_CODEC_ADC_DRE_DELAY_DIG_CH2_SHIFT             (11)
#define CODEC_CODEC_ADC_DRE_DELAY_ANA_CH2(n)                (((n) & 0x1F) << 14)
#define CODEC_CODEC_ADC_DRE_DELAY_ANA_CH2_MASK              (0x1F << 14)
#define CODEC_CODEC_ADC_DRE_DELAY_ANA_CH2_SHIFT             (14)
#define CODEC_CODEC_ADC_DRE_THD_DB_OFFSET_SIGN_CH2          (1 << 19)
#define CODEC_CODEC_ADC_DRE_BIT_SEL_CH2(n)                  (((n) & 0x3) << 20)
#define CODEC_CODEC_ADC_DRE_BIT_SEL_CH2_MASK                (0x3 << 20)
#define CODEC_CODEC_ADC_DRE_BIT_SEL_CH2_SHIFT               (20)
#define CODEC_CODEC_ADC_DRE_OVERFLOW_MUTE_EN_CH2            (1 << 22)
#define CODEC_CODEC_ADC_DRE_MUTE_MODE_CH2                   (1 << 23)
#define CODEC_CODEC_ADC_DRE_MUTE_RANGE_SEL_CH2(n)           (((n) & 0x3) << 24)
#define CODEC_CODEC_ADC_DRE_MUTE_RANGE_SEL_CH2_MASK         (0x3 << 24)
#define CODEC_CODEC_ADC_DRE_MUTE_RANGE_SEL_CH2_SHIFT        (24)
#define CODEC_CODEC_ADC_DRE_MUTE_STATUS_CH2                 (1 << 26)

// reg_1d4
#define CODEC_CODEC_ADC_DRE_AMP_HIGH_CH2(n)                 (((n) & 0x7FF) << 0)
#define CODEC_CODEC_ADC_DRE_AMP_HIGH_CH2_MASK               (0x7FF << 0)
#define CODEC_CODEC_ADC_DRE_AMP_HIGH_CH2_SHIFT              (0)
#define CODEC_CODEC_ADC_DRE_WINDOW_CH2(n)                   (((n) & 0xFFFFF) << 11)
#define CODEC_CODEC_ADC_DRE_WINDOW_CH2_MASK                 (0xFFFFF << 11)
#define CODEC_CODEC_ADC_DRE_WINDOW_CH2_SHIFT                (11)

// reg_1e0
#define CODEC_CODEC_ADC_DRE_GAIN_STEP0_CH0(n)               (((n) & 0x3FFF) << 0)
#define CODEC_CODEC_ADC_DRE_GAIN_STEP0_CH0_MASK             (0x3FFF << 0)
#define CODEC_CODEC_ADC_DRE_GAIN_STEP0_CH0_SHIFT            (0)
#define CODEC_CODEC_ADC_DRE_GAIN_STEP1_CH0(n)               (((n) & 0x3FFF) << 14)
#define CODEC_CODEC_ADC_DRE_GAIN_STEP1_CH0_MASK             (0x3FFF << 14)
#define CODEC_CODEC_ADC_DRE_GAIN_STEP1_CH0_SHIFT            (14)

// reg_1e4
#define CODEC_CODEC_ADC_DRE_GAIN_STEP2_CH0(n)               (((n) & 0x3FFF) << 0)
#define CODEC_CODEC_ADC_DRE_GAIN_STEP2_CH0_MASK             (0x3FFF << 0)
#define CODEC_CODEC_ADC_DRE_GAIN_STEP2_CH0_SHIFT            (0)
#define CODEC_CODEC_ADC_DRE_GAIN_STEP3_CH0(n)               (((n) & 0x3FFF) << 14)
#define CODEC_CODEC_ADC_DRE_GAIN_STEP3_CH0_MASK             (0x3FFF << 14)
#define CODEC_CODEC_ADC_DRE_GAIN_STEP3_CH0_SHIFT            (14)

// reg_1e8
#define CODEC_CODEC_ADC_DRE_GAIN_STEP4_CH0(n)               (((n) & 0x3FFF) << 0)
#define CODEC_CODEC_ADC_DRE_GAIN_STEP4_CH0_MASK             (0x3FFF << 0)
#define CODEC_CODEC_ADC_DRE_GAIN_STEP4_CH0_SHIFT            (0)
#define CODEC_CODEC_ADC_DRE_GAIN_STEP5_CH0(n)               (((n) & 0x3FFF) << 14)
#define CODEC_CODEC_ADC_DRE_GAIN_STEP5_CH0_MASK             (0x3FFF << 14)
#define CODEC_CODEC_ADC_DRE_GAIN_STEP5_CH0_SHIFT            (14)

// reg_1ec
#define CODEC_CODEC_ADC_DRE_GAIN_STEP6_CH0(n)               (((n) & 0x3FFF) << 0)
#define CODEC_CODEC_ADC_DRE_GAIN_STEP6_CH0_MASK             (0x3FFF << 0)
#define CODEC_CODEC_ADC_DRE_GAIN_STEP6_CH0_SHIFT            (0)
#define CODEC_CODEC_ADC_DRE_GAIN_STEP7_CH0(n)               (((n) & 0x3FFF) << 14)
#define CODEC_CODEC_ADC_DRE_GAIN_STEP7_CH0_MASK             (0x3FFF << 14)
#define CODEC_CODEC_ADC_DRE_GAIN_STEP7_CH0_SHIFT            (14)

// reg_1f0
#define CODEC_CODEC_ADC_DRE_GAIN_STEP8_CH0(n)               (((n) & 0x3FFF) << 0)
#define CODEC_CODEC_ADC_DRE_GAIN_STEP8_CH0_MASK             (0x3FFF << 0)
#define CODEC_CODEC_ADC_DRE_GAIN_STEP8_CH0_SHIFT            (0)
#define CODEC_CODEC_ADC_DRE_GAIN_STEP9_CH0(n)               (((n) & 0x3FFF) << 14)
#define CODEC_CODEC_ADC_DRE_GAIN_STEP9_CH0_MASK             (0x3FFF << 14)
#define CODEC_CODEC_ADC_DRE_GAIN_STEP9_CH0_SHIFT            (14)

// reg_1f4
#define CODEC_CODEC_ADC_DRE_GAIN_STEP10_CH0(n)              (((n) & 0x3FFF) << 0)
#define CODEC_CODEC_ADC_DRE_GAIN_STEP10_CH0_MASK            (0x3FFF << 0)
#define CODEC_CODEC_ADC_DRE_GAIN_STEP10_CH0_SHIFT           (0)
#define CODEC_CODEC_ADC_DRE_GAIN_STEP11_CH0(n)              (((n) & 0x3FFF) << 14)
#define CODEC_CODEC_ADC_DRE_GAIN_STEP11_CH0_MASK            (0x3FFF << 14)
#define CODEC_CODEC_ADC_DRE_GAIN_STEP11_CH0_SHIFT           (14)

// reg_1f8
#define CODEC_CODEC_ADC_DRE_GAIN_STEP12_CH0(n)              (((n) & 0x3FFF) << 0)
#define CODEC_CODEC_ADC_DRE_GAIN_STEP12_CH0_MASK            (0x3FFF << 0)
#define CODEC_CODEC_ADC_DRE_GAIN_STEP12_CH0_SHIFT           (0)
#define CODEC_CODEC_ADC_DRE_GAIN_STEP13_CH0(n)              (((n) & 0x3FFF) << 14)
#define CODEC_CODEC_ADC_DRE_GAIN_STEP13_CH0_MASK            (0x3FFF << 14)
#define CODEC_CODEC_ADC_DRE_GAIN_STEP13_CH0_SHIFT           (14)

// reg_1fc
#define CODEC_CODEC_ADC_DRE_GAIN_STEP14_CH0(n)              (((n) & 0x3FFF) << 0)
#define CODEC_CODEC_ADC_DRE_GAIN_STEP14_CH0_MASK            (0x3FFF << 0)
#define CODEC_CODEC_ADC_DRE_GAIN_STEP14_CH0_SHIFT           (0)
#define CODEC_CODEC_ADC_DRE_GAIN_STEP15_CH0(n)              (((n) & 0x3FFF) << 14)
#define CODEC_CODEC_ADC_DRE_GAIN_STEP15_CH0_MASK            (0x3FFF << 14)
#define CODEC_CODEC_ADC_DRE_GAIN_STEP15_CH0_SHIFT           (14)

// reg_200
#define CODEC_CODEC_ADC_DRE_GAIN_STEP0_CH1(n)               (((n) & 0x3FFF) << 0)
#define CODEC_CODEC_ADC_DRE_GAIN_STEP0_CH1_MASK             (0x3FFF << 0)
#define CODEC_CODEC_ADC_DRE_GAIN_STEP0_CH1_SHIFT            (0)
#define CODEC_CODEC_ADC_DRE_GAIN_STEP1_CH1(n)               (((n) & 0x3FFF) << 14)
#define CODEC_CODEC_ADC_DRE_GAIN_STEP1_CH1_MASK             (0x3FFF << 14)
#define CODEC_CODEC_ADC_DRE_GAIN_STEP1_CH1_SHIFT            (14)

// reg_204
#define CODEC_CODEC_ADC_DRE_GAIN_STEP2_CH1(n)               (((n) & 0x3FFF) << 0)
#define CODEC_CODEC_ADC_DRE_GAIN_STEP2_CH1_MASK             (0x3FFF << 0)
#define CODEC_CODEC_ADC_DRE_GAIN_STEP2_CH1_SHIFT            (0)
#define CODEC_CODEC_ADC_DRE_GAIN_STEP3_CH1(n)               (((n) & 0x3FFF) << 14)
#define CODEC_CODEC_ADC_DRE_GAIN_STEP3_CH1_MASK             (0x3FFF << 14)
#define CODEC_CODEC_ADC_DRE_GAIN_STEP3_CH1_SHIFT            (14)

// reg_208
#define CODEC_CODEC_ADC_DRE_GAIN_STEP4_CH1(n)               (((n) & 0x3FFF) << 0)
#define CODEC_CODEC_ADC_DRE_GAIN_STEP4_CH1_MASK             (0x3FFF << 0)
#define CODEC_CODEC_ADC_DRE_GAIN_STEP4_CH1_SHIFT            (0)
#define CODEC_CODEC_ADC_DRE_GAIN_STEP5_CH1(n)               (((n) & 0x3FFF) << 14)
#define CODEC_CODEC_ADC_DRE_GAIN_STEP5_CH1_MASK             (0x3FFF << 14)
#define CODEC_CODEC_ADC_DRE_GAIN_STEP5_CH1_SHIFT            (14)

// reg_20c
#define CODEC_CODEC_ADC_DRE_GAIN_STEP6_CH1(n)               (((n) & 0x3FFF) << 0)
#define CODEC_CODEC_ADC_DRE_GAIN_STEP6_CH1_MASK             (0x3FFF << 0)
#define CODEC_CODEC_ADC_DRE_GAIN_STEP6_CH1_SHIFT            (0)
#define CODEC_CODEC_ADC_DRE_GAIN_STEP7_CH1(n)               (((n) & 0x3FFF) << 14)
#define CODEC_CODEC_ADC_DRE_GAIN_STEP7_CH1_MASK             (0x3FFF << 14)
#define CODEC_CODEC_ADC_DRE_GAIN_STEP7_CH1_SHIFT            (14)

// reg_210
#define CODEC_CODEC_ADC_DRE_GAIN_STEP8_CH1(n)               (((n) & 0x3FFF) << 0)
#define CODEC_CODEC_ADC_DRE_GAIN_STEP8_CH1_MASK             (0x3FFF << 0)
#define CODEC_CODEC_ADC_DRE_GAIN_STEP8_CH1_SHIFT            (0)
#define CODEC_CODEC_ADC_DRE_GAIN_STEP9_CH1(n)               (((n) & 0x3FFF) << 14)
#define CODEC_CODEC_ADC_DRE_GAIN_STEP9_CH1_MASK             (0x3FFF << 14)
#define CODEC_CODEC_ADC_DRE_GAIN_STEP9_CH1_SHIFT            (14)

// reg_214
#define CODEC_CODEC_ADC_DRE_GAIN_STEP10_CH1(n)              (((n) & 0x3FFF) << 0)
#define CODEC_CODEC_ADC_DRE_GAIN_STEP10_CH1_MASK            (0x3FFF << 0)
#define CODEC_CODEC_ADC_DRE_GAIN_STEP10_CH1_SHIFT           (0)
#define CODEC_CODEC_ADC_DRE_GAIN_STEP11_CH1(n)              (((n) & 0x3FFF) << 14)
#define CODEC_CODEC_ADC_DRE_GAIN_STEP11_CH1_MASK            (0x3FFF << 14)
#define CODEC_CODEC_ADC_DRE_GAIN_STEP11_CH1_SHIFT           (14)

// reg_218
#define CODEC_CODEC_ADC_DRE_GAIN_STEP12_CH1(n)              (((n) & 0x3FFF) << 0)
#define CODEC_CODEC_ADC_DRE_GAIN_STEP12_CH1_MASK            (0x3FFF << 0)
#define CODEC_CODEC_ADC_DRE_GAIN_STEP12_CH1_SHIFT           (0)
#define CODEC_CODEC_ADC_DRE_GAIN_STEP13_CH1(n)              (((n) & 0x3FFF) << 14)
#define CODEC_CODEC_ADC_DRE_GAIN_STEP13_CH1_MASK            (0x3FFF << 14)
#define CODEC_CODEC_ADC_DRE_GAIN_STEP13_CH1_SHIFT           (14)

// reg_21c
#define CODEC_CODEC_ADC_DRE_GAIN_STEP14_CH1(n)              (((n) & 0x3FFF) << 0)
#define CODEC_CODEC_ADC_DRE_GAIN_STEP14_CH1_MASK            (0x3FFF << 0)
#define CODEC_CODEC_ADC_DRE_GAIN_STEP14_CH1_SHIFT           (0)
#define CODEC_CODEC_ADC_DRE_GAIN_STEP15_CH1(n)              (((n) & 0x3FFF) << 14)
#define CODEC_CODEC_ADC_DRE_GAIN_STEP15_CH1_MASK            (0x3FFF << 14)
#define CODEC_CODEC_ADC_DRE_GAIN_STEP15_CH1_SHIFT           (14)

// reg_22c
#define CODEC_CODEC_TT_ENABLE_CH0                           (1 << 0)
#define CODEC_CODEC_TT_ADC_SEL_CH0(n)                       (((n) & 0x7) << 1)
#define CODEC_CODEC_TT_ADC_SEL_CH0_MASK                     (0x7 << 1)
#define CODEC_CODEC_TT_ADC_SEL_CH0_SHIFT                    (1)
#define CODEC_CODEC_MM_ENABLE_CH0                           (1 << 4)
#define CODEC_CODEC_MM_FIFO_EN_CH0                          (1 << 5)
#define CODEC_CODEC_MM_FIFO_BYPASS_CH0                      (1 << 6)
#define CODEC_CODEC_MM_DELAY_CH0(n)                         (((n) & 0x1F) << 7)
#define CODEC_CODEC_MM_DELAY_CH0_MASK                       (0x1F << 7)
#define CODEC_CODEC_MM_DELAY_CH0_SHIFT                      (7)

// reg_230
#define CODEC_CODEC_MUTE_GAIN_COEF_TT_CH0(n)                (((n) & 0xFFF) << 0)
#define CODEC_CODEC_MUTE_GAIN_COEF_TT_CH0_MASK              (0xFFF << 0)
#define CODEC_CODEC_MUTE_GAIN_COEF_TT_CH0_SHIFT             (0)
#define CODEC_CODEC_MUTE_GAIN_PASS0_TT_CH0                  (1 << 12)
#define CODEC_CODEC_MUTE_GAIN_UPDATE_TT_CH0                 (1 << 13)

// reg_234
#define CODEC_CODEC_MUTE_GAIN_COEF_MM_CH0(n)                (((n) & 0xFFF) << 0)
#define CODEC_CODEC_MUTE_GAIN_COEF_MM_CH0_MASK              (0xFFF << 0)
#define CODEC_CODEC_MUTE_GAIN_COEF_MM_CH0_SHIFT             (0)
#define CODEC_CODEC_MUTE_GAIN_PASS0_MM_CH0                  (1 << 12)
#define CODEC_CODEC_MUTE_GAIN_UPDATE_MM_CH0                 (1 << 13)

// reg_238
#define CODEC_CODEC_ANC_CALIB_GAIN_COEF_FF_CH0(n)           (((n) & 0xFFFF) << 0)
#define CODEC_CODEC_ANC_CALIB_GAIN_COEF_FF_CH0_MASK         (0xFFFF << 0)
#define CODEC_CODEC_ANC_CALIB_GAIN_COEF_FF_CH0_SHIFT        (0)

// reg_23c
#define CODEC_CODEC_ANC_CALIB_GAIN_COEF_FB_CH0(n)           (((n) & 0xFFFF) << 0)
#define CODEC_CODEC_ANC_CALIB_GAIN_COEF_FB_CH0_MASK         (0xFFFF << 0)
#define CODEC_CODEC_ANC_CALIB_GAIN_COEF_FB_CH0_SHIFT        (0)

// reg_240
#define CODEC_CODEC_ANC_CALIB_GAIN_PASS0_FF_CH0             (1 << 0)
#define CODEC_CODEC_ANC_CALIB_GAIN_UPDATE_FF_CH0            (1 << 1)
#define CODEC_CODEC_ANC_CALIB_GAIN_PASS0_FB_CH0             (1 << 2)
#define CODEC_CODEC_ANC_CALIB_GAIN_UPDATE_FB_CH0            (1 << 3)
#define CODEC_CODEC_CALIB_GAIN_PASS0_TT_CH0                 (1 << 4)
#define CODEC_CODEC_CALIB_GAIN_UPDATE_TT_CH0                (1 << 5)

// reg_244
#define CODEC_CODEC_IIR0_ENABLE                             (1 << 0)
#define CODEC_CODEC_IIR0_IIRA_ENABLE                        (1 << 1)
#define CODEC_CODEC_IIR0_IIRB_ENABLE                        (1 << 2)
#define CODEC_CODEC_IIR0_CH0_BYPASS                         (1 << 3)
#define CODEC_CODEC_IIR0_CH1_BYPASS                         (1 << 4)
#define CODEC_CODEC_IIR0_GAINCAL_EXT_CH0_BYPASS             (1 << 5)
#define CODEC_CODEC_IIR0_GAINCAL_EXT_CH1_BYPASS             (1 << 6)
#define CODEC_CODEC_IIR0_GAINUSE_EXT_CH0_BYPASS             (1 << 7)
#define CODEC_CODEC_IIR0_GAINUSE_EXT_CH1_BYPASS             (1 << 8)
#define CODEC_CODEC_IIR0_LMT_CH0_BYPASS                     (1 << 9)
#define CODEC_CODEC_IIR0_LMT_CH1_BYPASS                     (1 << 10)
#define CODEC_CODEC_IIR0_COUNT_CH0(n)                       (((n) & 0x1F) << 11)
#define CODEC_CODEC_IIR0_COUNT_CH0_MASK                     (0x1F << 11)
#define CODEC_CODEC_IIR0_COUNT_CH0_SHIFT                    (11)
#define CODEC_CODEC_IIR0_COUNT_CH1(n)                       (((n) & 0x1F) << 16)
#define CODEC_CODEC_IIR0_COUNT_CH1_MASK                     (0x1F << 16)
#define CODEC_CODEC_IIR0_COUNT_CH1_SHIFT                    (16)
#define CODEC_CODEC_IIR0_COEF_SWAP                          (1 << 21)
#define CODEC_CODEC_IIR0_AUTO_STOP                          (1 << 22)
#define CODEC_CODEC_IIR0_COEF_SWAP_STATUS                   (1 << 23)
#define CODEC_CODEC_IIR0_IIRA_STOP_STATUS                   (1 << 24)
#define CODEC_CODEC_IIR0_IIRB_STOP_STATUS                   (1 << 25)

// reg_24c
#define CODEC_CODEC_IIR2_ENABLE                             (1 << 0)
#define CODEC_CODEC_IIR2_IIRA_ENABLE                        (1 << 1)
#define CODEC_CODEC_IIR2_IIRB_ENABLE                        (1 << 2)
#define CODEC_CODEC_IIR2_CH0_BYPASS                         (1 << 3)
#define CODEC_CODEC_IIR2_CH1_BYPASS                         (1 << 4)
#define CODEC_CODEC_IIR2_GAINCAL_EXT_CH0_BYPASS             (1 << 5)
#define CODEC_CODEC_IIR2_GAINCAL_EXT_CH1_BYPASS             (1 << 6)
#define CODEC_CODEC_IIR2_GAINUSE_EXT_CH0_BYPASS             (1 << 7)
#define CODEC_CODEC_IIR2_GAINUSE_EXT_CH1_BYPASS             (1 << 8)
#define CODEC_CODEC_IIR2_LMT_CH0_BYPASS                     (1 << 9)
#define CODEC_CODEC_IIR2_LMT_CH1_BYPASS                     (1 << 10)
#define CODEC_CODEC_IIR2_CH01_MIX                           (1 << 11)
#define CODEC_CODEC_IIR2_COUNT_CH0(n)                       (((n) & 0x1F) << 12)
#define CODEC_CODEC_IIR2_COUNT_CH0_MASK                     (0x1F << 12)
#define CODEC_CODEC_IIR2_COUNT_CH0_SHIFT                    (12)
#define CODEC_CODEC_IIR2_COUNT_CH1(n)                       (((n) & 0x1F) << 17)
#define CODEC_CODEC_IIR2_COUNT_CH1_MASK                     (0x1F << 17)
#define CODEC_CODEC_IIR2_COUNT_CH1_SHIFT                    (17)
#define CODEC_CODEC_IIR2_COEF_SWAP                          (1 << 22)
#define CODEC_CODEC_IIR2_AUTO_STOP                          (1 << 23)
#define CODEC_CODEC_IIR2_COEF_SWAP_STATUS                   (1 << 24)
#define CODEC_CODEC_IIR2_IIRA_STOP_STATUS                   (1 << 25)
#define CODEC_CODEC_IIR2_IIRB_STOP_STATUS                   (1 << 26)

// reg_258
#define CODEC_CODEC_DEQ_IIR_ENABLE                          (1 << 0)
#define CODEC_CODEC_DEQ_IIR_IIRA_ENABLE                     (1 << 1)
#define CODEC_CODEC_DEQ_IIR_IIRB_ENABLE                     (1 << 2)
#define CODEC_CODEC_DEQ_IIR_CH0_BYPASS                      (1 << 3)
#define CODEC_CODEC_DEQ_IIR_GAINCAL_EXT_CH0_BYPASS          (1 << 4)
#define CODEC_CODEC_DEQ_IIR_GAINUSE_EXT_CH0_BYPASS          (1 << 5)
#define CODEC_CODEC_DEQ_IIR_COUNT_CH0(n)                    (((n) & 0x3F) << 6)
#define CODEC_CODEC_DEQ_IIR_COUNT_CH0_MASK                  (0x3F << 6)
#define CODEC_CODEC_DEQ_IIR_COUNT_CH0_SHIFT                 (6)
#define CODEC_CODEC_DEQ_IIR_COEF_SWAP                       (1 << 12)
#define CODEC_CODEC_DEQ_IIR_AUTO_STOP                       (1 << 13)
#define CODEC_CODEC_DEQ_IIR_COEF_SWAP_STATUS                (1 << 14)
#define CODEC_CODEC_DEQ_IIR_IIRA_STOP_STATUS                (1 << 15)
#define CODEC_CODEC_DEQ_IIR_IIRB_STOP_STATUS                (1 << 16)

// reg_25c
#define CODEC_CODEC_IIR0_GAIN_EXT_UPDATE_CH0                (1 << 0)
#define CODEC_CODEC_IIR0_GAIN_EXT_UPDATE_CH1                (1 << 1)
#define CODEC_CODEC_IIR0_GAIN_EXT_SEL_CH0                   (1 << 2)
#define CODEC_CODEC_IIR0_GAIN_EXT_SEL_CH1                   (1 << 3)
#define CODEC_CODEC_IIR2_GAIN_EXT_UPDATE_CH0                (1 << 4)
#define CODEC_CODEC_IIR2_GAIN_EXT_UPDATE_CH1                (1 << 5)
#define CODEC_CODEC_IIR2_GAIN_EXT_SEL_CH0                   (1 << 6)
#define CODEC_CODEC_IIR2_GAIN_EXT_SEL_CH1                   (1 << 7)
#define CODEC_CODEC_DEQ_IIR_GAIN_EXT_UPDATE_CH0             (1 << 8)
#define CODEC_CODEC_DEQ_IIR_GAIN_EXT_SEL_CH0                (1 << 9)
#define CODEC_CODEC_IIR0_LMT_TH_UPDATE_CH0                  (1 << 10)
#define CODEC_CODEC_IIR0_LMT_TH_UPDATE_CH1                  (1 << 11)
#define CODEC_CODEC_IIR2_LMT_TH_UPDATE_CH0                  (1 << 12)
#define CODEC_CODEC_IIR2_LMT_TH_UPDATE_CH1                  (1 << 13)

// reg_260
#define CODEC_CODEC_IIR0_GAINA_EXT_CH0(n)                   (((n) & 0xFFFFFFFF) << 0)
#define CODEC_CODEC_IIR0_GAINA_EXT_CH0_MASK                 (0xFFFFFFFF << 0)
#define CODEC_CODEC_IIR0_GAINA_EXT_CH0_SHIFT                (0)

// reg_264
#define CODEC_CODEC_IIR0_GAINA_EXT_CH1(n)                   (((n) & 0xFFFFFFFF) << 0)
#define CODEC_CODEC_IIR0_GAINA_EXT_CH1_MASK                 (0xFFFFFFFF << 0)
#define CODEC_CODEC_IIR0_GAINA_EXT_CH1_SHIFT                (0)

// reg_268
#define CODEC_CODEC_IIR0_GAINB_EXT_CH0(n)                   (((n) & 0xFFFFFFFF) << 0)
#define CODEC_CODEC_IIR0_GAINB_EXT_CH0_MASK                 (0xFFFFFFFF << 0)
#define CODEC_CODEC_IIR0_GAINB_EXT_CH0_SHIFT                (0)

// reg_26c
#define CODEC_CODEC_IIR0_GAINB_EXT_CH1(n)                   (((n) & 0xFFFFFFFF) << 0)
#define CODEC_CODEC_IIR0_GAINB_EXT_CH1_MASK                 (0xFFFFFFFF << 0)
#define CODEC_CODEC_IIR0_GAINB_EXT_CH1_SHIFT                (0)

// reg_280
#define CODEC_CODEC_IIR2_GAINA_EXT_CH0(n)                   (((n) & 0xFFFFFFFF) << 0)
#define CODEC_CODEC_IIR2_GAINA_EXT_CH0_MASK                 (0xFFFFFFFF << 0)
#define CODEC_CODEC_IIR2_GAINA_EXT_CH0_SHIFT                (0)

// reg_284
#define CODEC_CODEC_IIR2_GAINA_EXT_CH1(n)                   (((n) & 0xFFFFFFFF) << 0)
#define CODEC_CODEC_IIR2_GAINA_EXT_CH1_MASK                 (0xFFFFFFFF << 0)
#define CODEC_CODEC_IIR2_GAINA_EXT_CH1_SHIFT                (0)

// reg_288
#define CODEC_CODEC_IIR2_GAINB_EXT_CH0(n)                   (((n) & 0xFFFFFFFF) << 0)
#define CODEC_CODEC_IIR2_GAINB_EXT_CH0_MASK                 (0xFFFFFFFF << 0)
#define CODEC_CODEC_IIR2_GAINB_EXT_CH0_SHIFT                (0)

// reg_28c
#define CODEC_CODEC_IIR2_GAINB_EXT_CH1(n)                   (((n) & 0xFFFFFFFF) << 0)
#define CODEC_CODEC_IIR2_GAINB_EXT_CH1_MASK                 (0xFFFFFFFF << 0)
#define CODEC_CODEC_IIR2_GAINB_EXT_CH1_SHIFT                (0)

// reg_2a0
#define CODEC_CODEC_IIR0_GAINA_EXT_OUT_CH0_SYNC(n)          (((n) & 0xFFFFFFFF) << 0)
#define CODEC_CODEC_IIR0_GAINA_EXT_OUT_CH0_SYNC_MASK        (0xFFFFFFFF << 0)
#define CODEC_CODEC_IIR0_GAINA_EXT_OUT_CH0_SYNC_SHIFT       (0)

// reg_2a4
#define CODEC_CODEC_IIR0_GAINA_EXT_OUT_CH1_SYNC(n)          (((n) & 0xFFFFFFFF) << 0)
#define CODEC_CODEC_IIR0_GAINA_EXT_OUT_CH1_SYNC_MASK        (0xFFFFFFFF << 0)
#define CODEC_CODEC_IIR0_GAINA_EXT_OUT_CH1_SYNC_SHIFT       (0)

// reg_2a8
#define CODEC_CODEC_IIR0_GAINB_EXT_OUT_CH0_SYNC(n)          (((n) & 0xFFFFFFFF) << 0)
#define CODEC_CODEC_IIR0_GAINB_EXT_OUT_CH0_SYNC_MASK        (0xFFFFFFFF << 0)
#define CODEC_CODEC_IIR0_GAINB_EXT_OUT_CH0_SYNC_SHIFT       (0)

// reg_2ac
#define CODEC_CODEC_IIR0_GAINB_EXT_OUT_CH1_SYNC(n)          (((n) & 0xFFFFFFFF) << 0)
#define CODEC_CODEC_IIR0_GAINB_EXT_OUT_CH1_SYNC_MASK        (0xFFFFFFFF << 0)
#define CODEC_CODEC_IIR0_GAINB_EXT_OUT_CH1_SYNC_SHIFT       (0)

// reg_2c0
#define CODEC_CODEC_IIR2_GAINA_EXT_OUT_CH0_SYNC(n)          (((n) & 0xFFFFFFFF) << 0)
#define CODEC_CODEC_IIR2_GAINA_EXT_OUT_CH0_SYNC_MASK        (0xFFFFFFFF << 0)
#define CODEC_CODEC_IIR2_GAINA_EXT_OUT_CH0_SYNC_SHIFT       (0)

// reg_2c4
#define CODEC_CODEC_IIR2_GAINA_EXT_OUT_CH1_SYNC(n)          (((n) & 0xFFFFFFFF) << 0)
#define CODEC_CODEC_IIR2_GAINA_EXT_OUT_CH1_SYNC_MASK        (0xFFFFFFFF << 0)
#define CODEC_CODEC_IIR2_GAINA_EXT_OUT_CH1_SYNC_SHIFT       (0)

// reg_2c8
#define CODEC_CODEC_IIR2_GAINB_EXT_OUT_CH0_SYNC(n)          (((n) & 0xFFFFFFFF) << 0)
#define CODEC_CODEC_IIR2_GAINB_EXT_OUT_CH0_SYNC_MASK        (0xFFFFFFFF << 0)
#define CODEC_CODEC_IIR2_GAINB_EXT_OUT_CH0_SYNC_SHIFT       (0)

// reg_2cc
#define CODEC_CODEC_IIR2_GAINB_EXT_OUT_CH1_SYNC(n)          (((n) & 0xFFFFFFFF) << 0)
#define CODEC_CODEC_IIR2_GAINB_EXT_OUT_CH1_SYNC_MASK        (0xFFFFFFFF << 0)
#define CODEC_CODEC_IIR2_GAINB_EXT_OUT_CH1_SYNC_SHIFT       (0)

// reg_2e0
#define CODEC_CODEC_DEQ_IIR_GAINA_EXT_CH0(n)                (((n) & 0xFFFFFFFF) << 0)
#define CODEC_CODEC_DEQ_IIR_GAINA_EXT_CH0_MASK              (0xFFFFFFFF << 0)
#define CODEC_CODEC_DEQ_IIR_GAINA_EXT_CH0_SHIFT             (0)

// reg_2e8
#define CODEC_CODEC_DEQ_IIR_GAINB_EXT_CH0(n)                (((n) & 0xFFFFFFFF) << 0)
#define CODEC_CODEC_DEQ_IIR_GAINB_EXT_CH0_MASK              (0xFFFFFFFF << 0)
#define CODEC_CODEC_DEQ_IIR_GAINB_EXT_CH0_SHIFT             (0)

// reg_2f0
#define CODEC_CODEC_DEQ_IIR_GAINA_EXT_OUT_CH0_SYNC(n)       (((n) & 0xFFFFFFFF) << 0)
#define CODEC_CODEC_DEQ_IIR_GAINA_EXT_OUT_CH0_SYNC_MASK     (0xFFFFFFFF << 0)
#define CODEC_CODEC_DEQ_IIR_GAINA_EXT_OUT_CH0_SYNC_SHIFT    (0)

// reg_2f8
#define CODEC_CODEC_DEQ_IIR_GAINB_EXT_OUT_CH0_SYNC(n)       (((n) & 0xFFFFFFFF) << 0)
#define CODEC_CODEC_DEQ_IIR_GAINB_EXT_OUT_CH0_SYNC_MASK     (0xFFFFFFFF << 0)
#define CODEC_CODEC_DEQ_IIR_GAINB_EXT_OUT_CH0_SYNC_SHIFT    (0)

// reg_300
#define CODEC_CODEC_IIR0_LMT_DELAY_CH0(n)                   (((n) & 0x7F) << 0)
#define CODEC_CODEC_IIR0_LMT_DELAY_CH0_MASK                 (0x7F << 0)
#define CODEC_CODEC_IIR0_LMT_DELAY_CH0_SHIFT                (0)
#define CODEC_CODEC_IIR2_LMT_DELAY_CH0(n)                   (((n) & 0x7F) << 7)
#define CODEC_CODEC_IIR2_LMT_DELAY_CH0_MASK                 (0x7F << 7)
#define CODEC_CODEC_IIR2_LMT_DELAY_CH0_SHIFT                (7)

// reg_304
#define CODEC_CODEC_IIR0_LMT_DELAY_CH1(n)                   (((n) & 0x7F) << 0)
#define CODEC_CODEC_IIR0_LMT_DELAY_CH1_MASK                 (0x7F << 0)
#define CODEC_CODEC_IIR0_LMT_DELAY_CH1_SHIFT                (0)
#define CODEC_CODEC_IIR2_LMT_DELAY_CH1(n)                   (((n) & 0x7F) << 7)
#define CODEC_CODEC_IIR2_LMT_DELAY_CH1_MASK                 (0x7F << 7)
#define CODEC_CODEC_IIR2_LMT_DELAY_CH1_SHIFT                (7)

// reg_340
#define CODEC_CODEC_IIR0_GAIN_EXT_TH(n)                     (((n) & 0xFFFFFFFF) << 0)
#define CODEC_CODEC_IIR0_GAIN_EXT_TH_MASK                   (0xFFFFFFFF << 0)
#define CODEC_CODEC_IIR0_GAIN_EXT_TH_SHIFT                  (0)

// reg_348
#define CODEC_CODEC_IIR2_GAIN_EXT_TH(n)                     (((n) & 0xFFFFFFFF) << 0)
#define CODEC_CODEC_IIR2_GAIN_EXT_TH_MASK                   (0xFFFFFFFF << 0)
#define CODEC_CODEC_IIR2_GAIN_EXT_TH_SHIFT                  (0)

// reg_350
#define CODEC_CODEC_DEQ_IIR_GAIN_EXT_TH(n)                  (((n) & 0xFFFFFFFF) << 0)
#define CODEC_CODEC_DEQ_IIR_GAIN_EXT_TH_MASK                (0xFFFFFFFF << 0)
#define CODEC_CODEC_DEQ_IIR_GAIN_EXT_TH_SHIFT               (0)

// reg_354
#define CODEC_CODEC_IIR0_LMT_TH_CH0(n)                      (((n) & 0x7FFFFF) << 0)
#define CODEC_CODEC_IIR0_LMT_TH_CH0_MASK                    (0x7FFFFF << 0)
#define CODEC_CODEC_IIR0_LMT_TH_CH0_SHIFT                   (0)

// reg_358
#define CODEC_CODEC_IIR0_LMT_TH_CH1(n)                      (((n) & 0x7FFFFF) << 0)
#define CODEC_CODEC_IIR0_LMT_TH_CH1_MASK                    (0x7FFFFF << 0)
#define CODEC_CODEC_IIR0_LMT_TH_CH1_SHIFT                   (0)

// reg_364
#define CODEC_CODEC_IIR2_LMT_TH_CH0(n)                      (((n) & 0x7FFFFF) << 0)
#define CODEC_CODEC_IIR2_LMT_TH_CH0_MASK                    (0x7FFFFF << 0)
#define CODEC_CODEC_IIR2_LMT_TH_CH0_SHIFT                   (0)

// reg_368
#define CODEC_CODEC_IIR2_LMT_TH_CH1(n)                      (((n) & 0x7FFFFF) << 0)
#define CODEC_CODEC_IIR2_LMT_TH_CH1_MASK                    (0x7FFFFF << 0)
#define CODEC_CODEC_IIR2_LMT_TH_CH1_SHIFT                   (0)

// reg_374
#define CODEC_CODEC_MUSIC_MIX_OFF_CH0                       (1 << 0)
#define CODEC_CODEC_PDU_OFF_CH0                             (1 << 1)
#define CODEC_CODEC_PDU_MIX_EN_CH0                          (1 << 2)
#define CODEC_CODEC_TT_OUT_OFF_CH0                          (1 << 3)
#define CODEC_CODEC_TT_PDU_OFF_CH0                          (1 << 4)

// reg_378
#define CODEC_CODEC_CALIB_GAIN_COEF_TT_CH0(n)               (((n) & 0xFFFF) << 0)
#define CODEC_CODEC_CALIB_GAIN_COEF_TT_CH0_MASK             (0xFFFF << 0)
#define CODEC_CODEC_CALIB_GAIN_COEF_TT_CH0_SHIFT            (0)

// reg_380
#define CODEC_CODEC_DAC_DRE_DC0_CH0(n)                      (((n) & 0xFFFF) << 0)
#define CODEC_CODEC_DAC_DRE_DC0_CH0_MASK                    (0xFFFF << 0)
#define CODEC_CODEC_DAC_DRE_DC0_CH0_SHIFT                   (0)
#define CODEC_CODEC_DAC_DRE_DC1_CH0(n)                      (((n) & 0xFFFF) << 16)
#define CODEC_CODEC_DAC_DRE_DC1_CH0_MASK                    (0xFFFF << 16)
#define CODEC_CODEC_DAC_DRE_DC1_CH0_SHIFT                   (16)

// reg_384
#define CODEC_CODEC_DAC_DRE_DC2_CH0(n)                      (((n) & 0xFFFF) << 0)
#define CODEC_CODEC_DAC_DRE_DC2_CH0_MASK                    (0xFFFF << 0)
#define CODEC_CODEC_DAC_DRE_DC2_CH0_SHIFT                   (0)
#define CODEC_CODEC_DAC_DRE_DC3_CH0(n)                      (((n) & 0xFFFF) << 16)
#define CODEC_CODEC_DAC_DRE_DC3_CH0_MASK                    (0xFFFF << 16)
#define CODEC_CODEC_DAC_DRE_DC3_CH0_SHIFT                   (16)

// reg_388
#define CODEC_CODEC_DAC_DRE_DC4_CH0(n)                      (((n) & 0xFFFF) << 0)
#define CODEC_CODEC_DAC_DRE_DC4_CH0_MASK                    (0xFFFF << 0)
#define CODEC_CODEC_DAC_DRE_DC4_CH0_SHIFT                   (0)
#define CODEC_CODEC_DAC_DRE_DC5_CH0(n)                      (((n) & 0xFFFF) << 16)
#define CODEC_CODEC_DAC_DRE_DC5_CH0_MASK                    (0xFFFF << 16)
#define CODEC_CODEC_DAC_DRE_DC5_CH0_SHIFT                   (16)

// reg_38c
#define CODEC_CODEC_DAC_DRE_DC6_CH0(n)                      (((n) & 0xFFFF) << 0)
#define CODEC_CODEC_DAC_DRE_DC6_CH0_MASK                    (0xFFFF << 0)
#define CODEC_CODEC_DAC_DRE_DC6_CH0_SHIFT                   (0)
#define CODEC_CODEC_DAC_DRE_DC7_CH0(n)                      (((n) & 0xFFFF) << 16)
#define CODEC_CODEC_DAC_DRE_DC7_CH0_MASK                    (0xFFFF << 16)
#define CODEC_CODEC_DAC_DRE_DC7_CH0_SHIFT                   (16)

// reg_390
#define CODEC_CODEC_DAC_DRE_DC8_CH0(n)                      (((n) & 0xFFFF) << 0)
#define CODEC_CODEC_DAC_DRE_DC8_CH0_MASK                    (0xFFFF << 0)
#define CODEC_CODEC_DAC_DRE_DC8_CH0_SHIFT                   (0)
#define CODEC_CODEC_DAC_DRE_DC9_CH0(n)                      (((n) & 0xFFFF) << 16)
#define CODEC_CODEC_DAC_DRE_DC9_CH0_MASK                    (0xFFFF << 16)
#define CODEC_CODEC_DAC_DRE_DC9_CH0_SHIFT                   (16)

// reg_394
#define CODEC_CODEC_DAC_DRE_DC10_CH0(n)                     (((n) & 0xFFFF) << 0)
#define CODEC_CODEC_DAC_DRE_DC10_CH0_MASK                   (0xFFFF << 0)
#define CODEC_CODEC_DAC_DRE_DC10_CH0_SHIFT                  (0)
#define CODEC_CODEC_DAC_DRE_DC11_CH0(n)                     (((n) & 0xFFFF) << 16)
#define CODEC_CODEC_DAC_DRE_DC11_CH0_MASK                   (0xFFFF << 16)
#define CODEC_CODEC_DAC_DRE_DC11_CH0_SHIFT                  (16)

// reg_398
#define CODEC_CODEC_DAC_DRE_DC12_CH0(n)                     (((n) & 0xFFFF) << 0)
#define CODEC_CODEC_DAC_DRE_DC12_CH0_MASK                   (0xFFFF << 0)
#define CODEC_CODEC_DAC_DRE_DC12_CH0_SHIFT                  (0)
#define CODEC_CODEC_DAC_DRE_DC13_CH0(n)                     (((n) & 0xFFFF) << 16)
#define CODEC_CODEC_DAC_DRE_DC13_CH0_MASK                   (0xFFFF << 16)
#define CODEC_CODEC_DAC_DRE_DC13_CH0_SHIFT                  (16)

// reg_39c
#define CODEC_CODEC_DAC_DRE_DC14_CH0(n)                     (((n) & 0xFFFF) << 0)
#define CODEC_CODEC_DAC_DRE_DC14_CH0_MASK                   (0xFFFF << 0)
#define CODEC_CODEC_DAC_DRE_DC14_CH0_SHIFT                  (0)
#define CODEC_CODEC_DAC_DRE_DC15_CH0(n)                     (((n) & 0xFFFF) << 16)
#define CODEC_CODEC_DAC_DRE_DC15_CH0_MASK                   (0xFFFF << 16)
#define CODEC_CODEC_DAC_DRE_DC15_CH0_SHIFT                  (16)

// reg_3a0
#define CODEC_CODEC_DAC_DRE_ANA_GAIN_CH0(n)                 (((n) & 0x1F) << 0)
#define CODEC_CODEC_DAC_DRE_ANA_GAIN_CH0_MASK               (0x1F << 0)
#define CODEC_CODEC_DAC_DRE_ANA_GAIN_CH0_SHIFT              (0)
#define CODEC_CODEC_DAC_DRE_COUNT_CH0(n)                    (((n) & 0x1FFFFF) << 5)
#define CODEC_CODEC_DAC_DRE_COUNT_CH0_MASK                  (0x1FFFFF << 5)
#define CODEC_CODEC_DAC_DRE_COUNT_CH0_SHIFT                 (5)

// reg_468
#define CODEC_CODEC_DAC_EN_SND                              (1 << 0)
#define CODEC_CODEC_DAC_EN_SND_CH0                          (1 << 1)
#define CODEC_CODEC_DAC_HBF3_BYPASS_SND                     (1 << 2)
#define CODEC_CODEC_DAC_HBF2_BYPASS_SND                     (1 << 3)
#define CODEC_CODEC_DAC_HBF1_BYPASS_SND                     (1 << 4)
#define CODEC_CODEC_DAC_UP_SEL_SND(n)                       (((n) & 0x7) << 5)
#define CODEC_CODEC_DAC_UP_SEL_SND_MASK                     (0x7 << 5)
#define CODEC_CODEC_DAC_UP_SEL_SND_SHIFT                    (5)
#define CODEC_CODEC_DAC_TONE_TEST_SND                       (1 << 8)
#define CODEC_CODEC_DAC_SIN1K_SEL                           (1 << 9)
#define CODEC_CODEC_DAC_LR_SWAP_SND                         (1 << 10)

// reg_46c
#define CODEC_CODEC_DAC_GAIN_SND_CH0(n)                     (((n) & 0xFFFFF) << 0)
#define CODEC_CODEC_DAC_GAIN_SND_CH0_MASK                   (0xFFFFF << 0)
#define CODEC_CODEC_DAC_GAIN_SND_CH0_SHIFT                  (0)
#define CODEC_CODEC_DAC_GAIN_SEL_SND_CH0                    (1 << 20)
#define CODEC_CODEC_DAC_GAIN_UPDATE_SND                     (1 << 21)
#define CODEC_CODEC_DAC_GAIN_TRIGGER_SEL_SND(n)             (((n) & 0x3) << 22)
#define CODEC_CODEC_DAC_GAIN_TRIGGER_SEL_SND_MASK           (0x3 << 22)
#define CODEC_CODEC_DAC_GAIN_TRIGGER_SEL_SND_SHIFT          (22)

// reg_470
#define CODEC_CODEC_RAMP_STEP_SND_CH0(n)                    (((n) & 0xFFF) << 0)
#define CODEC_CODEC_RAMP_STEP_SND_CH0_MASK                  (0xFFF << 0)
#define CODEC_CODEC_RAMP_STEP_SND_CH0_SHIFT                 (0)
#define CODEC_CODEC_RAMP_DIRECT_SND_CH0                     (1 << 12)
#define CODEC_CODEC_RAMP_EN_SND_CH0                         (1 << 13)
#define CODEC_CODEC_RAMP_INTERVAL_SND_CH0(n)                (((n) & 0x7) << 14)
#define CODEC_CODEC_RAMP_INTERVAL_SND_CH0_MASK              (0x7 << 14)
#define CODEC_CODEC_RAMP_INTERVAL_SND_CH0_SHIFT             (14)

#endif
