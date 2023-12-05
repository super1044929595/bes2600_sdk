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
#ifndef __HAL_DMACFG_BEST1305_H__
#define __HAL_DMACFG_BEST1305_H__

#define AUDMA_PERIPH_NUM                        16
#define GPDMA_PERIPH_NUM                        16

#define AUDMA_CHAN_NUM                          8
#define GPDMA_CHAN_NUM                          8

#define AUDMA_CHAN_START                        (0)
#define GPDMA_CHAN_START                        (0)

static const uint32_t audma_fifo_addr[AUDMA_PERIPH_NUM] = {
    CODEC_BASE + 0x01C,     // CODEC RX
    CODEC_BASE + 0x01C,     // CODEC TX
    BTPCM_BASE + 0x1C0,     // BTPCM RX
    BTPCM_BASE + 0x1C8,     // BTPCM TX
    I2S0_BASE + 0x200,      // I2S0 RX
    I2S0_BASE + 0x240,      // I2S0 TX
    0,                      // NULL
    0,                      // NULL
    0,                      // NULL
    0,                      // NULL
    0,                      // NULL
    CODEC_BASE + 0x03C,     // CODEC TX2
    BTDUMP_BASE + 0x34,     // BTDUMP
    CODEC_BASE + 0x038,     // MC RX
    0,                      // NULL
    0,                      // NULL
};

static const enum HAL_DMA_PERIPH_T audma_fifo_periph[AUDMA_PERIPH_NUM] = {
    HAL_AUDMA_CODEC_RX,
    HAL_AUDMA_CODEC_TX,
    HAL_AUDMA_BTPCM_RX,
    HAL_AUDMA_BTPCM_TX,
    HAL_AUDMA_I2S0_RX,
    HAL_AUDMA_I2S0_TX,
    HAL_DMA_PERIPH_NULL,
    HAL_DMA_PERIPH_NULL,
    HAL_DMA_PERIPH_NULL,
    HAL_DMA_PERIPH_NULL,
    HAL_DMA_PERIPH_NULL,
    HAL_AUDMA_CODEC_TX2,
    HAL_AUDMA_BTDUMP,
    HAL_AUDMA_MC_RX,
    HAL_DMA_PERIPH_NULL,
    HAL_DMA_PERIPH_NULL,
};

static const uint32_t gpdma_fifo_addr[GPDMA_PERIPH_NUM] = {
    0,                      // NULL
    0,                      // NULL
    I2C0_BASE + 0x010,      // I2C0 RX
    I2C0_BASE + 0x010,      // I2C0 TX
    SPI_BASE + 0x008,       // SPI RX
    SPI_BASE + 0x008,       // SPI TX
    0,                      // NULL
    0,                      // NULL
    UART0_BASE + 0x000,     // UART0 RX
    UART0_BASE + 0x000,     // UART0 TX
    UART1_BASE + 0x000,     // UART1 RX
    UART1_BASE + 0x000,     // UART1 TX
    I2C1_BASE + 0x010,      // I2C1 RX
    I2C1_BASE + 0x010,      // I2C1 TX
    I2C2_BASE + 0x010,      // I2C2 RX
    I2C2_BASE + 0x010,      // I2C2 TX
};

static const enum HAL_DMA_PERIPH_T gpdma_fifo_periph[GPDMA_PERIPH_NUM] = {
    HAL_DMA_PERIPH_NULL,
    HAL_DMA_PERIPH_NULL,
    HAL_GPDMA_I2C0_RX,
    HAL_GPDMA_I2C0_TX,
    HAL_GPDMA_SPI_RX,
    HAL_GPDMA_SPI_TX,
    HAL_GPDMA_SPILCD_RX,
    HAL_GPDMA_SPILCD_TX,
    HAL_GPDMA_UART0_RX,
    HAL_GPDMA_UART0_TX,
    HAL_GPDMA_UART1_RX,
    HAL_GPDMA_UART1_TX,
    HAL_GPDMA_I2C1_RX,
    HAL_GPDMA_I2C1_TX,
    HAL_GPDMA_I2C2_RX,
    HAL_GPDMA_I2C2_TX,
};

#endif

