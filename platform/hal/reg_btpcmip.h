/***************************************************************************
 *
 * Copyright 2015-2019 BES.
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
#ifndef __REG_BTPCMIP_H_
#define __REG_BTPCMIP_H_

#include "plat_types.h"

#define BTPCMIP_FIFO_DEPTH 8

/* btpcmip register */
/* enable register */
#define BTPCMIP_ENABLE_REG_REG_OFFSET 0x0
#define BTPCMIP_ENABLE_REG_BTPCM_ENABLE_SHIFT (0)
#define BTPCMIP_ENABLE_REG_BTPCM_ENABLE_MASK ((0x1)<<BTPCMIP_ENABLE_REG_BTPCM_ENABLE_SHIFT)

/* recv fifo flush register */
#define BTPCMIP_RX_FIFO_FLUSH_REG_OFFSET 0x4
#define BTPCMIP_RX_FIFO_FLUSH_SHIFT (0)
#define BTPCMIP_RX_FIFO_FLUSH_MASK ((0x1)<<BTPCMIP_RX_FIFO_FLUSH_SHIFT)

/* send fifo flush register */
#define BTPCMIP_TX_FIFO_FLUSH_REG_OFFSET 0x8
#define BTPCMIP_TX_FIFO_FLUSH_SHIFT (0)
#define BTPCMIP_TX_FIFO_FLUSH_MASK ((0x1)<<BTPCMIP_TX_FIFO_FLUSH_SHIFT)

/* send buffer register */
#define BTPCMIP_TX_BUFF_REG_OFFSET 0xc

/* recv buffer register */
#define BTPCMIP_RX_BUFF_REG_OFFSET 0xc

/* config register */
#define BTPCMIP_CR_REG_OFFSET 0x10
#define BTPCMIP_CR_MASK2_SHIFT (21)
#define BTPCMIP_CR_MASK2_MASK ((0x1)<<BTPCMIP_CR_MASK2_SHIFT)
#define BTPCMIP_CR_SYNCSHORT_SHIFT (20)
#define BTPCMIP_CR_SYNCSHORT_MASK ((0x1)<<BTPCMIP_CR_SYNCSHORT_SHIFT)
#define BTPCMIP_CR_MASK1_SHIFT (19)
#define BTPCMIP_CR_MASK1_MASK ((0x1)<<BTPCMIP_CR_MASK1_SHIFT)
#define BTPCMIP_CR_LENTH_SHIFT (16)
#define BTPCMIP_CR_LENTH_MASK ((0x7)<<BTPCMIP_CR_LENTH_SHIFT)
// Since 1302
#define BTPCMIP_CR_PCM_CLK_OPEN_EN_SHIFT (15)
#define BTPCMIP_CR_PCM_CLK_OPEN_EN_MASK ((0x1)<<BTPCMIP_CR_PCM_CLK_OPEN_EN_SHIFT)
// -- End of since 1302
#define BTPCMIP_CR_SLOTSEL_SHIFT (0)
#define BTPCMIP_CR_SLOTSEL_MASK ((0x7)<<BTPCMIP_CR_SLOTSEL_SHIFT)

/* rx config register */
#define BTPCMIP_RCR0_REG_OFFSET 0x14
#define BTPCMIP_RCR0_SIGNEXTIN_SHIFT (4)
#define BTPCMIP_RCR0_SIGNEXTIN_MASK ((0x1)<<BTPCMIP_RCR0_SIGNEXTIN_SHIFT)
#define BTPCMIP_RCR0_MSBIN_SHIFT (3)
#define BTPCMIP_RCR0_MSBIN_MASK ((0x1)<<BTPCMIP_RCR0_MSBIN_SHIFT)
#define BTPCMIP_RCR0_SIGNIN_SHIFT (2)
#define BTPCMIP_RCR0_SIGNIN_MASK ((0x1)<<BTPCMIP_RCR0_SIGNIN_SHIFT)
#define BTPCMIP_RCR0_2SIN_SHIFT (1)
#define BTPCMIP_RCR0_2SIN_MASK ((0x1)<<BTPCMIP_RCR0_2SIN_SHIFT)
#define BTPCMIP_RCR0_1SIN_SHIFT (0)
#define BTPCMIP_RCR0_1SIN_MASK ((0x1)<<BTPCMIP_RCR0_1SIN_SHIFT)

/* tx config register */
#define BTPCMIP_TCR0_REG_OFFSET 0x18
#define BTPCMIP_TCR0_SIGNEXTO_SHIFT (4)
#define BTPCMIP_TCR0_SIGNEXTO_MASK ((0x1)<<BTPCMIP_TCR0_SIGNEXTO_SHIFT)
#define BTPCMIP_TCR0_MSBO_SHIFT (3)
#define BTPCMIP_TCR0_MSBO_MASK ((0x1)<<BTPCMIP_TCR0_MSBO_SHIFT)
#define BTPCMIP_TCR0_SIGNO_SHIFT (2)
#define BTPCMIP_TCR0_SIGNO_MASK ((0x1)<<BTPCMIP_TCR0_SIGNO_SHIFT)
#define BTPCMIP_TCR0_2SO_SHIFT (1)
#define BTPCMIP_TCR0_2SO_MASK ((0x1)<<BTPCMIP_TCR0_2SO_SHIFT)
#define BTPCMIP_TCR0_1SO_SHIFT (0)
#define BTPCMIP_TCR0_1SO_MASK ((0x1)<<BTPCMIP_TCR0_1SO_SHIFT)

/* int status register */
#define BTPCMIP_INT_STATUS_REG_OFFSET 0x1c
#define BTPCMIP_INT_STATUS_TX_FIFO_OVER_SHIFT (5)
#define BTPCMIP_INT_STATUS_TX_FIFO_OVER_MASK ((0x1)<<BTPCMIP_INT_STATUS_TX_FIFO_OVER_SHIFT)
#define BTPCMIP_INT_STATUS_TX_FIFO_EMPTY_SHIFT (4)
#define BTPCMIP_INT_STATUS_TX_FIFO_EMPTY_MASK ((0x1)<<BTPCMIP_INT_STATUS_TX_FIFO_EMPTY_SHIFT)
#define BTPCMIP_INT_STATUS_RX_FIFO_OVER_SHIFT (1)
#define BTPCMIP_INT_STATUS_RX_FIFO_OVER_MASK ((0x1)<<BTPCMIP_INT_STATUS_RX_FIFO_OVER_SHIFT)
#define BTPCMIP_INT_STATUS_RX_FIFO_DA_SHIFT (0)
#define BTPCMIP_INT_STATUS_RX_FIFO_DA_MASK ((0x1)<<BTPCMIP_INT_STATUS_RX_FIFO_DA_SHIFT)

/* int mask register */
#define BTPCMIP_INT_MASK_REG_OFFSET 0x20
#define BTPCMIP_INT_MASK_TX_FIFO_OVER_SHIFT (5)
#define BTPCMIP_INT_MASK_TX_FIFO_OVER_MASK ((0x1)<<BTPCMIP_INT_MASK_TX_FIFO_OVER_SHIFT)
#define BTPCMIP_INT_MASK_TX_FIFO_EMPTY_SHIFT (4)
#define BTPCMIP_INT_MASK_TX_FIFO_EMPTY_MASK ((0x1)<<BTPCMIP_INT_MASK_TX_FIFO_EMPTY_SHIFT)
#define BTPCMIP_INT_MASK_RX_FIFO_OVER_SHIFT (1)
#define BTPCMIP_INT_MASK_RX_FIFO_OVER_MASK ((0x1)<<BTPCMIP_INT_MASK_RX_FIFO_OVER_SHIFT)
#define BTPCMIP_INT_MASK_RX_FIFO_DA_SHIFT (0)
#define BTPCMIP_INT_MASK_RX_FIFO_DA_MASK ((0x1)<<BTPCMIP_INT_MASK_RX_FIFO_DA_SHIFT)
#define BTPCMIP_INT_MASK_ALL \
    (BTPCMIP_INT_MASK_TX_FIFO_OVER_MASK|BTPCMIP_INT_MASK_TX_FIFO_EMPTY_MASK|BTPCMIP_INT_MASK_RX_FIFO_OVER_MASK|BTPCMIP_INT_MASK_RX_FIFO_DA_MASK)
#define BTPCMIP_INT_UNMASK_ALL 0

/* clr recv over flow register */
#define BTPCMIP_CLR_RX_OVER_FLOW_REG_OFFSET 0x24
#define BTPCMIP_CLR_RX_OVER_FLOW_CLR_SHIFT (0)
#define BTPCMIP_CLR_RX_OVER_FLOW_CLR_MASK ((0x1)<<BTPCMIP_CLR_RX_OVER_FLOW_CLR_SHIFT)

/* clr send over flow register */
#define BTPCMIP_CLR_TX_OVER_FLOW_REG_OFFSET 0x28
#define BTPCMIP_CLR_TX_OVER_FLOW_CLR_SHIFT (0)
#define BTPCMIP_CLR_TX_OVER_FLOW_CLR_MASK ((0x1)<<BTPCMIP_CLR_TX_OVER_FLOW_CLR_SHIFT)

/* recv fifo config register */
#define BTPCMIP_RX_FIFO_CFG_REG_OFFSET 0x2c
#define BTPCMIP_RX_FIFO_CFG_LEVEL_SHIFT (0)
#define BTPCMIP_RX_FIFO_CFG_LEVEL_MASK ((0xf)<<BTPCMIP_RX_FIFO_CFG_LEVEL_SHIFT)

/* send fifo config register */
#define BTPCMIP_TX_FIFO_CFG_REG_OFFSET 0x30
#define BTPCMIP_TX_FIFO_CFG_LEVEL_SHIFT (0)
#define BTPCMIP_TX_FIFO_CFG_LEVEL_MASK ((0xf)<<BTPCMIP_TX_FIFO_CFG_LEVEL_SHIFT)

/* dma ctrl register */
#define BTPCMIP_DMA_CTRL_REG_OFFSET 0x34
#define BTPCMIP_DMA_CTRL_TX_ENABLE_SHIFT (1)
#define BTPCMIP_DMA_CTRL_TX_ENABLE_MASK ((0x1)<<BTPCMIP_DMA_CTRL_TX_ENABLE_SHIFT)
#define BTPCMIP_DMA_CTRL_RX_ENABLE_SHIFT (0)
#define BTPCMIP_DMA_CTRL_RX_ENABLE_MASK ((0x1)<<BTPCMIP_DMA_CTRL_RX_ENABLE_SHIFT)
/* btpcmip register end */

#endif /* __REG_BTPCMIP_H_ */
