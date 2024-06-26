/***************************************************************************
 *
 * Copyright 2015-2021 BES.
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
#include "plat_addr_map.h"

#ifdef SDIO_DEVICE_BASE

#include "reg_sdio_device.h"
#include "hal_sdio_device.h"
#include "cmsis_nvic.h"
#include "hal_trace.h"
#include "hal_timer.h"
#include <string.h>

#ifdef SDIO_DEVICE_DEBUG
    #define HAL_SDIO_DEVICE_TRACE(n, s, ...)    TRACE(n, "[%u]" s, TICKS_TO_MS(hal_sys_timer_get()), ##__VA_ARGS__)
#else
    #define HAL_SDIO_DEVICE_TRACE(n, s, ...)    TRACE_DUMMY(n, s, ##__VA_ARGS__)
#endif

static struct SDIO_DEVICE_T *const sdio_device = {
    (struct SDIO_DEVICE_T *)SDIO_DEVICE_BASE,
};

struct SDIO_DEVICE_ADMA_DESC_T {
    uint32_t adma_desc_ctrl;
    uint32_t adma_desc_addr;
};

enum SDIO_DEVICE_ADMA_ATT_T {
    SDIO_DEVICE_ADMA_ATT_VALID  = 0x01,
    SDIO_DEVICE_ADMA_ATT_END    = 0x02,
    SDIO_DEVICE_ADMA_ATT_INT    = 0x04,
    SDIO_DEVICE_ADMA_ATT_NOP    = 0x00,
    SDIO_DEVICE_ADMA_ATT_RSV    = 0x10,
    SDIO_DEVICE_ADMA_ATT_TRAN   = 0x20,
    SDIO_DEVICE_ADMA_ATT_LINK   = 0x30,
};

struct SDIO_DEVICE_RXTX_CTRL_T {
    uint8_t *rxaddr;//device perspective, device is rx, host is tx
    uint32_t rxlen;
    uint8_t *txaddr;//device perspective, device is tx, host is rx
    uint32_t txlen;
    uint8_t status;
    uint8_t rxbuf_cnt;
    struct HAL_SDIO_DEVICE_CB_T *sdio_callback;
};

static struct SDIO_DEVICE_RXTX_CTRL_T sdio_rxtx_ctrl;

#ifndef SDIO_DEVICE_ENABLE_DMA
#define SDIO_DEVICE_ADMA_TXRX_LEN       65536
#define SDIO_DEVICE_ADMA_DESC_NUM       (65+1)  //Each link has a maximum of 1k and a maximum of 65 links,add an nop link
struct SDIO_DEVICE_ADMA_DESC_T __attribute__((aligned(4))) sdio_device_adma_desc[SDIO_DEVICE_ADMA_DESC_NUM];
#endif

#define DMA_BOUNDARY_SIZE   4096
#define ADMA_BOUNDARY_SIZE  1024

#ifdef SDIO_DEVICE_DEBUG
#define SDIO_STATUS_FORMAT(x)  {"sdio device irq:SDIO_"#x}
struct SDIO_INT_STATUS_NAME_T {
    char *name;
};
const static struct SDIO_INT_STATUS_NAME_T sdio_int_status[] = {
    SDIO_STATUS_FORMAT(TRX_COMPLETE_INT),
    SDIO_STATUS_FORMAT(DMA1_INT),
    SDIO_STATUS_FORMAT(SLEEP_AWAKE_INT),
    SDIO_STATUS_FORMAT(WR_START_INT),
    SDIO_STATUS_FORMAT(RD_START_INT),
    SDIO_STATUS_FORMAT(PASSWORD_SET_INT),
    SDIO_STATUS_FORMAT(PASSWORD_RESET_INT),
    SDIO_STATUS_FORMAT(LOCK_CARD_INT),
    SDIO_STATUS_FORMAT(UNLOCK_CARD_INT),
    SDIO_STATUS_FORMAT(FORCE_ERASE_INT),
    SDIO_STATUS_FORMAT(ERASE_INT),
    SDIO_STATUS_FORMAT(CMD11_INT),
    SDIO_STATUS_FORMAT(CMD0_CMD52_SOFTRST_INT),
    SDIO_STATUS_FORMAT(CMD6_CHECK_DONE_INT),
    SDIO_STATUS_FORMAT(CMD6_SWITCH_DONE_INT),
    SDIO_STATUS_FORMAT(PROGRAM_CSD_INT),
    SDIO_STATUS_FORMAT(ACMD23_INT),
    SDIO_STATUS_FORMAT(CMD20_INT),
    SDIO_STATUS_FORMAT(HOST_INT),
    SDIO_STATUS_FORMAT(CMD4_INT),
    SDIO_STATUS_FORMAT(BOOT_START_INT),
    SDIO_STATUS_FORMAT(FUNC1_RESET_INT),
    SDIO_STATUS_FORMAT(FUNC2_RESET_INT),
    SDIO_STATUS_FORMAT(CMD11_CLK_STOP_INT),
    SDIO_STATUS_FORMAT(CMD11_CLK_START_INT),
    SDIO_STATUS_FORMAT(PROGRAM_START_INT),
    SDIO_STATUS_FORMAT(CMD40_INT),
    SDIO_STATUS_FORMAT(CMDR1B_INT),
    SDIO_STATUS_FORMAT(FUNCX_CRC_END_ERROR_INT),
    SDIO_STATUS_FORMAT(FUNCX_ABORT_INT),
    SDIO_STATUS_FORMAT(LRST_INT),
    SDIO_STATUS_FORMAT(BOOT_COMPLETE_INT),
};
const static struct SDIO_INT_STATUS_NAME_T sdio_int_status2[] = {
    SDIO_STATUS_FORMAT(FUNC3_RESET_INT),
    SDIO_STATUS_FORMAT(FUNC4_RESET_INT),
    SDIO_STATUS_FORMAT(FUNC5_RESET_INT),
    SDIO_STATUS_FORMAT(RESERVED_BIT3),
    SDIO_STATUS_FORMAT(RESERVED_BIT4),
    SDIO_STATUS_FORMAT(PACKED_INT),
    SDIO_STATUS_FORMAT(ADMA_ERROR_INT),
};
#endif

static void hal_sdio_device_read_cccr(uint32_t *cccr)
{
    *cccr = sdio_device->SDIO_CCCR_CONTROL;
}

static void hal_sdio_device_write_cccr(uint32_t cccr)
{
    sdio_device->SDIO_CCCR_CONTROL = cccr;
}

POSSIBLY_UNUSED static void hal_sdio_device_read_card_addr(uint32_t *addr)
{
    //cpu cannot read,2021-05-17
    return;
}

static void hal_sdio_device_write_card_addr(uint32_t addr)
{
    sdio_device->CARD_ADDRESS = addr;
}

static void hal_sdio_device_interrupt_status_enable(void)
{
    sdio_device->INTERRUPT_STATUS_ENABLE = 0xFFFFFFFF;
    sdio_device->INTERRUPT_STATUS2_ENABLE = SDIO_FUNC3_RESET_EN    \
                                          | SDIO_FUNC4_RESET_EN    \
                                          | SDIO_FUNC5_RESET_EN    \
                                          | SDIO_PACKED_EN         \
                                          | SDIO_ADMA_ERROR_EN;
}

POSSIBLY_UNUSED static void hal_sdio_device_interrupt_status_disable(uint32_t disable_bit)
{
    if (disable_bit) {
        sdio_device->INTERRUPT_STATUS_ENABLE &= ~disable_bit;
    }
}

POSSIBLY_UNUSED static void hal_sdio_device_interrupt_status2_disable(uint32_t disable_bit)
{
    if (disable_bit) {
        sdio_device->INTERRUPT_STATUS2_ENABLE &= ~disable_bit;
    }
}

POSSIBLY_UNUSED static void hal_sdio_device_interrupt_signal_enable(void)
{
    sdio_device->INTERRUPT_SIGNAL_ENABLE = 0xFFFFFFFF;
    sdio_device->INTERRUPT_SIGNAL2_ENABLE = SDIO_FUNC3_RESET_EN    \
                                          | SDIO_FUNC4_RESET_EN    \
                                          | SDIO_FUNC5_RESET_EN    \
                                          | SDIO_PACKED_EN         \
                                          | SDIO_ADMA_ERROR_EN;
}

POSSIBLY_UNUSED static void hal_sdio_device_interrupt_signal_disable(void)
{
    sdio_device->INTERRUPT_SIGNAL_ENABLE = 0;
    sdio_device->INTERRUPT_SIGNAL2_ENABLE = 0;
}

static void hal_sdio_device_io_ready(void)
{
    sdio_device->IO_READY = SDIO_FUNC1_READY | SDIO_FUNC2_READY;
}

POSSIBLY_UNUSED static void hal_sdio_device_read_ocr(uint32_t *ocr)
{
    *ocr = sdio_device->CARD_OCR;
}

static void hal_sdio_device_write_ocr(void)
{
    sdio_device->CARD_OCR = SDIO_CARD_OCR(0xFF8000) \
                          | SDIO_SWITCH_TO_1V8      \
                          | SDIO_NUM_FUNCTIONS(2);
}

static void hal_sdio_device_set_card_ready(void)
{
    sdio_device->CONTROL |= SDIO_CARD_INIT_DONE;
}

POSSIBLY_UNUSED static void hal_sdio_device_enable_adma(void)
{
    sdio_device->CONTROL2 |= SDIO_ADMA_ENABLE;
}

POSSIBLY_UNUSED static void hal_sdio_device_disable_adma(void)
{
    sdio_device->CONTROL2 &= (~SDIO_ADMA_ENABLE);
}

POSSIBLY_UNUSED static void hal_sdio_device_gen_int_to_host(uint16_t signal_value)
{
    sdio_device->FUNCTION1_CONTROL = signal_value;
}

static void hal_sdio_device_clr_host_int(void)
{
    sdio_device->HOST_INT_CLR |= SDIO_HOST_INT_CLR;
}

static void hal_sdio_device_read_tplmanid(uint32_t *tplmanid)
{
    *tplmanid = sdio_device->TPLMANID;
}

static void hal_sdio_device_write_tplmanid(uint32_t tplmanid)
{
    sdio_device->TPLMANID = tplmanid;
}

void hal_sdio_device_change_tplmanid_manf(uint16_t new_manf_id)
{
    uint32_t val = 0;

    val = sdio_device->TPLMANID;
    val &= 0xFFFF0000;
    val |= new_manf_id;
    sdio_device->TPLMANID = val;//Write for the first time: generate a falling edge
    sdio_device->TPLMANID = val;//The second real write: a rising edge
}

void hal_sdio_device_change_tplmanid_card(uint16_t new_card_id)
{
    uint32_t val = 0;

    val = sdio_device->TPLMANID;
    val &= 0x0000FFFF;
    val |= (new_card_id << SDIO_TPLMID_CARD_LOW_SHIFT);
    sdio_device->TPLMANID = val;//Write for the first time: generate a falling edge
    sdio_device->TPLMANID = val;//The second real write: a rising edge
}

void hal_sdio_device_download_ready(void)
{
    sdio_device->SDIO_LOAD_CFG |= SDIO_DEVICE_LOAD_RDY;
}

static void hal_sdio_device_download_not_ready(void)
{
    sdio_device->SDIO_LOAD_CFG &= (~SDIO_DEVICE_LOAD_RDY);
}

void hal_sdio_device_rx_success(void)
{
    sdio_device->SDIO_LOAD_CFG |= SDIO_DEVICE_RX_SUCCESS;
}

void hal_sdio_device_rx_error(void)
{
    sdio_device->SDIO_LOAD_CFG &= (~SDIO_DEVICE_RX_SUCCESS);
}

//set firmware status:run
void hal_sdio_device_fws_already_run(void)
{
    sdio_device->SDIO_LOAD_CFG = SET_BITFIELD(sdio_device->SDIO_LOAD_CFG, SDIO_FW_STATUS1, 0x02);//low byte
    sdio_device->SDIO_LOAD_CFG = SET_BITFIELD(sdio_device->SDIO_LOAD_CFG, SDIO_FW_STATUS2, 0x20);
    sdio_device->SDIO_LOAD_CFG = SET_BITFIELD(sdio_device->SDIO_LOAD_CFG, SDIO_FW_STATUS3, 0xBE);//high byte
}

//set firmware status:stop
void hal_sdio_device_fws_stop_run(void)
{
    sdio_device->SDIO_LOAD_CFG = SET_BITFIELD(sdio_device->SDIO_LOAD_CFG, SDIO_FW_STATUS1, 0x00);//low byte
    sdio_device->SDIO_LOAD_CFG = SET_BITFIELD(sdio_device->SDIO_LOAD_CFG, SDIO_FW_STATUS2, 0x00);
    sdio_device->SDIO_LOAD_CFG = SET_BITFIELD(sdio_device->SDIO_LOAD_CFG, SDIO_FW_STATUS3, 0x00);//high byte
}

static void hal_sdio_device_rxbuf_cfg(uint32_t buf_len, uint8_t buf_cnt)
{
    sdio_device->RX_BUF_CFG = (sdio_device->RX_BUF_CFG & ~(SDIO_RX_BUF_CNT_MASK | SDIO_RX_BUF_LEN_MASK)) |
                              SDIO_RX_BUF_CNT(buf_cnt) | SDIO_RX_BUF_LEN(buf_len);
}

static void hal_sdio_device_rxbuf_cnt_cfg(uint8_t buf_cnt)
{
    sdio_device->RX_BUF_CFG = SET_BITFIELD(sdio_device->RX_BUF_CFG, SDIO_RX_BUF_CNT, buf_cnt);
}

static void hal_sdio_device_irq_handler(void)
{
    uint32_t i;
    uint32_t branch;
    uint32_t status;
    uint32_t status2;
    static uint16_t real_rx_len = 0;

    status = sdio_device->INTERRUPT_STATUS;
    status2 = sdio_device->INTERRUPT_STATUS2;
    hal_sdio_device_interrupt_status_disable(status);
    hal_sdio_device_interrupt_status2_disable(status2);
    hal_sdio_device_interrupt_signal_disable();
    sdio_device->INTERRUPT_STATUS = 0xFFFFFFFF; //clear all int flag
    sdio_device->INTERRUPT_STATUS2 = 0xFFFFFFFF;//clear all int flag

    HAL_SDIO_DEVICE_TRACE(0, "   ");
    HAL_SDIO_DEVICE_TRACE(1, "---sdio device irq status=0x%X", status);
    HAL_SDIO_DEVICE_TRACE(1, "---sdio device irq status2=0x%X", status2);
#ifdef SDIO_DEVICE_DEBUG
    //print interrupt status
    for (i = 0; i < 32; i++) {
        if (status & (1 << i))
            HAL_SDIO_DEVICE_TRACE(1, "%s", sdio_int_status[i].name);
    }
    for (i = 0; i < 7; i++) {
        if (status2 & (1 << i))
            HAL_SDIO_DEVICE_TRACE(1, "%s", sdio_int_status2[i].name);
    }
#endif

    //-----------------High priority states are processed first-----------------
    if (status2 & SDIO_ADMA_ERROR_INT) {
        status2 &= ~SDIO_ADMA_ERROR_INT;
        // TODO:2021-05-21
        //adma error process
    }
    if (status & SDIO_FUNCX_ABORT_INT) {
        status &= ~SDIO_FUNCX_ABORT_INT;
        // TODO:2021-05-21
    }
    if (status & SDIO_CMD0_CMD52_SOFTRST_INT) {
        status &= ~SDIO_CMD0_CMD52_SOFTRST_INT;
        // TODO:2021-05-21
    }
    if (status & SDIO_FUNC1_RESET_INT) {
        status &= ~SDIO_FUNC1_RESET_INT;
        // TODO:2021-05-21
    }
    if (status & SDIO_FUNC2_RESET_INT) {
        status &= ~SDIO_FUNC2_RESET_INT;
        // TODO:2021-05-21
    }
    if (status & SDIO_CMDR1B_INT) {
        status &= ~SDIO_CMDR1B_INT;
        // TODO:2021-05-21
    }
    if (status & SDIO_FUNCX_CRC_END_ERROR_INT) {
        status &= ~SDIO_FUNCX_CRC_END_ERROR_INT;
        // TODO:2021-05-21
    }
    if (status & SDIO_HOST_INT) {
        status &= ~SDIO_HOST_INT;
        hal_sdio_device_clr_host_int();
        if (sdio_rxtx_ctrl.sdio_callback->hal_sdio_device_int_from_host) {
            sdio_rxtx_ctrl.sdio_callback->hal_sdio_device_int_from_host();
        }
    }

    //--------------------Low priority state post processing--------------------
    while (status) {
        i = get_lsb_pos(status);
        branch = status & (1 << i);
        status &= ~(1 << i);
        switch (branch) {
            case SDIO_PROGRAM_START_INT: {
                sdio_rxtx_ctrl.status = HAL_SDIO_DEVICE_IDLE;
                if (sdio_rxtx_ctrl.sdio_callback->hal_sdio_device_enum_done) {
                    sdio_rxtx_ctrl.sdio_callback->hal_sdio_device_enum_done(HAL_SDIO_DEVICE_RDY);
                }
                break;
            }
            case SDIO_WR_START_INT:
            case SDIO_RD_START_INT: {
                uint32_t cmd = 0;
                uint32_t arg;
                uint16_t block_count;
                uint16_t block_size;
                uint32_t address;
                uint32_t length;
                uint32_t register_address;

                cmd = sdio_device->COMMAND;
                arg = sdio_device->ARGUMENT;
                block_size = (cmd >> 1) & 0xFFF;
                block_count = arg & 0x1FF;
                register_address = (arg >> 9) & 0x1FFFF;

                if (branch == SDIO_WR_START_INT) {
                    ASSERT(sdio_rxtx_ctrl.rxbuf_cnt != 0, "%s:%d. no rx buf", __func__, __LINE__);
                    ASSERT(sdio_rxtx_ctrl.rxaddr != NULL, "%s:%d. rx buf is NULL", __func__, __LINE__);

                    sdio_rxtx_ctrl.status = HAL_SDIO_DEVICE_RX;
                    address = (uint32_t)sdio_rxtx_ctrl.rxaddr;
                    length = register_address;
                    real_rx_len = length;

                    ASSERT(length <= sdio_rxtx_ctrl.rxlen, "%s:%d,required max len is %d,recv len is %d", __func__, __LINE__, sdio_rxtx_ctrl.rxlen, length);
                    hal_sdio_device_rx_error();
                    if (sdio_rxtx_ctrl.sdio_callback->hal_sdio_device_rxtx_start) {
                        sdio_rxtx_ctrl.sdio_callback->hal_sdio_device_rxtx_start(HAL_SDIO_DEVICE_RX);
                    }
                } else {
                    sdio_rxtx_ctrl.status = HAL_SDIO_DEVICE_TX;
                    address = (uint32_t)sdio_rxtx_ctrl.txaddr;
                    if (sdio_rxtx_ctrl.txlen % block_size) {
                        length = (sdio_rxtx_ctrl.txlen / block_size + 1) * block_size;
                    } else {
                        length = sdio_rxtx_ctrl.txlen;
                    }
                    if (sdio_rxtx_ctrl.sdio_callback->hal_sdio_device_rxtx_start) {
                        sdio_rxtx_ctrl.sdio_callback->hal_sdio_device_rxtx_start(HAL_SDIO_DEVICE_TX);
                    }
                }

#ifdef SDIO_DEVICE_ENABLE_DMA
                sdio_device->DMA1_ADDRESS = address;//device view
                sdio_device->DMA1_CONTROL = SDIO_DMA1_ADDR_VALID | SDIO_DMA1_BUF_SIZE(length / DMA_BOUNDARY_SIZE - 1);
#else
                //config adma desc
                uint32_t temp_addr;
                temp_addr = address & 0x3FF;//Get the offset address within the 1K address
                if (temp_addr) {
                    uint8_t cfg_done;

                    temp_addr = ADMA_BOUNDARY_SIZE - temp_addr;
                    if (length <= temp_addr) {  //Only 1 link
                        sdio_device_adma_desc[0].adma_desc_ctrl = (uint32_t)(length << 16);
                        sdio_device_adma_desc[0].adma_desc_ctrl |= SDIO_DEVICE_ADMA_ATT_TRAN | SDIO_DEVICE_ADMA_ATT_VALID;
                        sdio_device_adma_desc[0].adma_desc_addr = address;
                        cfg_done = 1;

                        HAL_SDIO_DEVICE_TRACE(3, "***adma_desc[%d].addr ctrl=0x%08X 0x%08X@0x%08X", 0, sdio_device_adma_desc[0].adma_desc_addr,
                                              sdio_device_adma_desc[0].adma_desc_ctrl, (uint32_t)&sdio_device_adma_desc[0].adma_desc_ctrl);

                        //add an nop link
                        sdio_device_adma_desc[1].adma_desc_ctrl = SDIO_DEVICE_ADMA_ATT_NOP | SDIO_DEVICE_ADMA_ATT_VALID | SDIO_DEVICE_ADMA_ATT_END;
                        sdio_device_adma_desc[1].adma_desc_addr = 0;
                        HAL_SDIO_DEVICE_TRACE(3, "***adma_desc[%d].addr ctrl=0x%08X 0x%08X@0x%08X", 1, sdio_device_adma_desc[1].adma_desc_addr,
                                              sdio_device_adma_desc[1].adma_desc_ctrl, (uint32_t)&sdio_device_adma_desc[1].adma_desc_ctrl);

                    } else {                    //More than 1 link
                        sdio_device_adma_desc[0].adma_desc_ctrl = (uint32_t)(temp_addr << 16);
                        sdio_device_adma_desc[0].adma_desc_ctrl |= SDIO_DEVICE_ADMA_ATT_TRAN | SDIO_DEVICE_ADMA_ATT_VALID;
                        sdio_device_adma_desc[0].adma_desc_addr = address;
                        cfg_done = 0;

                        HAL_SDIO_DEVICE_TRACE(3, "***adma_desc[%d].addr ctrl=0x%08X 0x%08X@0x%08X", 0, sdio_device_adma_desc[0].adma_desc_addr,
                                              sdio_device_adma_desc[0].adma_desc_ctrl, (uint32_t)&sdio_device_adma_desc[0].adma_desc_ctrl);
                    }

                    if (!cfg_done) {
                        uint32_t integer;
                        uint32_t remainder;

                        integer = (length - temp_addr) / ADMA_BOUNDARY_SIZE;
                        remainder = (length - temp_addr) % ADMA_BOUNDARY_SIZE;
                        if ((integer) || (remainder)) {
                            if (!remainder) {
                                for (i = 0; i < integer; i++) {
                                    sdio_device_adma_desc[1 + i].adma_desc_ctrl = (uint32_t)(ADMA_BOUNDARY_SIZE << 16);
                                    sdio_device_adma_desc[1 + i].adma_desc_ctrl |= SDIO_DEVICE_ADMA_ATT_TRAN | SDIO_DEVICE_ADMA_ATT_VALID;
                                    sdio_device_adma_desc[1 + i].adma_desc_addr = address + temp_addr + i * ADMA_BOUNDARY_SIZE;

                                    HAL_SDIO_DEVICE_TRACE(3, "***adma_desc[%d].addr ctrl=0x%08X 0x%08X@0x%08X", 1 + i, sdio_device_adma_desc[1 + i].adma_desc_addr,
                                                          sdio_device_adma_desc[1 + i].adma_desc_ctrl, (uint32_t)&sdio_device_adma_desc[1 + i].adma_desc_ctrl);
                                }

                                //add an nop link
                                sdio_device_adma_desc[1 + i].adma_desc_ctrl = SDIO_DEVICE_ADMA_ATT_NOP | SDIO_DEVICE_ADMA_ATT_VALID | SDIO_DEVICE_ADMA_ATT_END;
                                sdio_device_adma_desc[1 + i].adma_desc_addr = 0;
                                HAL_SDIO_DEVICE_TRACE(3, "***adma_desc[%d].addr ctrl=0x%08X 0x%08X@0x%08X", 1 + i, sdio_device_adma_desc[1 + i].adma_desc_addr,
                                                      sdio_device_adma_desc[1 + i].adma_desc_ctrl, (uint32_t)&sdio_device_adma_desc[1 + i].adma_desc_ctrl);
                            } else {
                                //process integer
                                for (i = 0; i < integer; i++) {
                                    sdio_device_adma_desc[1 + i].adma_desc_ctrl = (uint32_t)(ADMA_BOUNDARY_SIZE << 16);
                                    sdio_device_adma_desc[1 + i].adma_desc_ctrl |= SDIO_DEVICE_ADMA_ATT_TRAN | SDIO_DEVICE_ADMA_ATT_VALID;
                                    sdio_device_adma_desc[1 + i].adma_desc_addr = address + temp_addr + i * ADMA_BOUNDARY_SIZE;

                                    HAL_SDIO_DEVICE_TRACE(3, "***adma_desc[%d].addr ctrl=0x%08X 0x%08X@0x%08X", 1 + i, sdio_device_adma_desc[1 + i].adma_desc_addr,
                                                          sdio_device_adma_desc[1 + i].adma_desc_ctrl, (uint32_t)&sdio_device_adma_desc[1 + i].adma_desc_ctrl);
                                }

                                //process remainder
                                sdio_device_adma_desc[1 + i].adma_desc_ctrl = (uint32_t)((length - temp_addr - i * ADMA_BOUNDARY_SIZE) << 16);
                                sdio_device_adma_desc[1 + i].adma_desc_ctrl |= SDIO_DEVICE_ADMA_ATT_TRAN | SDIO_DEVICE_ADMA_ATT_VALID;
                                sdio_device_adma_desc[1 + i].adma_desc_addr = address + temp_addr + i * ADMA_BOUNDARY_SIZE;

                                HAL_SDIO_DEVICE_TRACE(3, "***adma_desc[%d].addr ctrl=0x%08X 0x%08X@0x%08X", 1 + i, sdio_device_adma_desc[1 + i].adma_desc_addr,
                                                      sdio_device_adma_desc[1 + i].adma_desc_ctrl, (uint32_t)&sdio_device_adma_desc[1 + i].adma_desc_ctrl);

                                //add an nop link
                                sdio_device_adma_desc[2 + i].adma_desc_ctrl = SDIO_DEVICE_ADMA_ATT_NOP | SDIO_DEVICE_ADMA_ATT_VALID | SDIO_DEVICE_ADMA_ATT_END;
                                sdio_device_adma_desc[2 + i].adma_desc_addr = 0;
                                HAL_SDIO_DEVICE_TRACE(3, "***adma_desc[%d].addr ctrl=0x%08X 0x%08X@0x%08X", 2 + i, sdio_device_adma_desc[2 + i].adma_desc_addr,
                                                      sdio_device_adma_desc[2 + i].adma_desc_ctrl, (uint32_t)&sdio_device_adma_desc[2 + i].adma_desc_ctrl);
                            }
                        }
                    }
                } else {
                    if (length == SDIO_DEVICE_ADMA_TXRX_LEN)
                        sdio_device_adma_desc[0].adma_desc_ctrl = 0;
                    else
                        sdio_device_adma_desc[0].adma_desc_ctrl = (uint32_t)(length << 16);
                    sdio_device_adma_desc[0].adma_desc_ctrl |= SDIO_DEVICE_ADMA_ATT_TRAN | SDIO_DEVICE_ADMA_ATT_VALID;
                    sdio_device_adma_desc[0].adma_desc_addr = address;
                    HAL_SDIO_DEVICE_TRACE(3, "***adma_desc[%d].addr ctrl=0x%08X 0x%08X@0x%08X", 0, sdio_device_adma_desc[0].adma_desc_addr,
                                          sdio_device_adma_desc[0].adma_desc_ctrl, (uint32_t)&sdio_device_adma_desc[0].adma_desc_ctrl);

                    //add an nop link
                    sdio_device_adma_desc[1].adma_desc_ctrl = SDIO_DEVICE_ADMA_ATT_NOP | SDIO_DEVICE_ADMA_ATT_VALID | SDIO_DEVICE_ADMA_ATT_END;
                    sdio_device_adma_desc[1].adma_desc_addr = 0;
                    HAL_SDIO_DEVICE_TRACE(3, "***adma_desc[%d].addr ctrl=0x%08X 0x%08X@0x%08X", 1, sdio_device_adma_desc[1].adma_desc_addr,
                                          sdio_device_adma_desc[1].adma_desc_ctrl, (uint32_t)&sdio_device_adma_desc[1].adma_desc_ctrl);
                }

                sdio_device->DMA1_ADDRESS = (uint32_t)&sdio_device_adma_desc[0].adma_desc_ctrl;
                HAL_SDIO_DEVICE_TRACE(1, "***sdio_device->DMA1_ADDRESS=0x%X", sdio_device->DMA1_ADDRESS);
                sdio_device->DMA1_CONTROL = SDIO_DEVICE_ADMA_ATT_VALID;
#endif
                HAL_SDIO_DEVICE_TRACE(1, "cmd=0x%X received", cmd);
                HAL_SDIO_DEVICE_TRACE(1, "   Application cmd :%s", (cmd & 0x1) ? "Application cmd" : "Not an Application cmd");
                HAL_SDIO_DEVICE_TRACE(1, "   block/byte size :%d", block_size);
                HAL_SDIO_DEVICE_TRACE(1, "   cmd             :cmd%d", (cmd >> 13) & 0x3F);
                if (((cmd >> 19) & 0x3) == 0)
                    HAL_SDIO_DEVICE_TRACE(1, "   bus width       :1 bit");
                else if (((cmd >> 19) & 0x3) == 1)
                    HAL_SDIO_DEVICE_TRACE(1, "   bus width       :4 bit");
                else if (((cmd >> 19) & 0x3) == 2)
                    HAL_SDIO_DEVICE_TRACE(1, "   bus width       :5 bit");
                else
                    HAL_SDIO_DEVICE_TRACE(1, "   bus width       :Reserved");
                HAL_SDIO_DEVICE_TRACE(1, "   current speed   :0x%X", (cmd >> 21) & 0x7);
                HAL_SDIO_DEVICE_TRACE(1, "   card state      :0x%X", (cmd >> 24) & 0xF);
                HAL_SDIO_DEVICE_TRACE(1, "arg=0x%X received", arg);
                HAL_SDIO_DEVICE_TRACE(1, "   byte/block count:%d", block_count);
                HAL_SDIO_DEVICE_TRACE(1, "   register addr   :0x%X", register_address);
                HAL_SDIO_DEVICE_TRACE(1, "   OP code         :%s", ((arg >> 26) & 0x1) ? "incrementing address" : "fixed address");
                HAL_SDIO_DEVICE_TRACE(1, "   block mode      :%s", ((arg >> 27) & 0x1) ? "block mode" : "byte mode");
                HAL_SDIO_DEVICE_TRACE(1, "   function number :%d", (arg >> 28) & 0x7);
                HAL_SDIO_DEVICE_TRACE(1, "   r/w flag        :%s", ((arg >> 31) & 0x1) ? "write mode" : "read mode");
                HAL_SDIO_DEVICE_TRACE(1, "sdio_device->DMA1_ADDRESS=0x%X@0x%X",
                                      (uint32_t)sdio_device->DMA1_ADDRESS,
                                      (uint32_t)&sdio_device->DMA1_ADDRESS);
                HAL_SDIO_DEVICE_TRACE(1, "sdio_device->DMA1_CONTROL=0x%X@0x%X",
                                      (uint32_t)sdio_device->DMA1_CONTROL,
                                      (uint32_t)&sdio_device->DMA1_CONTROL);
                break;
            }
            case SDIO_TRX_COMPLETE_INT: {
                if (sdio_rxtx_ctrl.status == HAL_SDIO_DEVICE_TX) { //send to host
                    sdio_rxtx_ctrl.status = HAL_SDIO_DEVICE_IDLE;
                    if (sdio_rxtx_ctrl.sdio_callback->hal_sdio_device_send_done) {
                        sdio_rxtx_ctrl.sdio_callback->hal_sdio_device_send_done(sdio_rxtx_ctrl.txaddr, sdio_rxtx_ctrl.txlen);
                    }
                    HAL_SDIO_DEVICE_TRACE(0, "------device tx ok");
                    HAL_SDIO_DEVICE_TRACE(TR_ATTR_NO_TS | TR_ATTR_NO_ID, "  ");
                } else if (sdio_rxtx_ctrl.status == HAL_SDIO_DEVICE_RX) { //read from host
                    uint8_t *rxaddr = sdio_rxtx_ctrl.rxaddr;

                    sdio_rxtx_ctrl.rxaddr = NULL;
                    sdio_rxtx_ctrl.rxbuf_cnt --;
                    hal_sdio_device_rxbuf_cnt_cfg(sdio_rxtx_ctrl.rxbuf_cnt);

                    sdio_rxtx_ctrl.status = HAL_SDIO_DEVICE_IDLE;
                    if (sdio_rxtx_ctrl.sdio_callback->hal_sdio_device_recv_done) {
                        sdio_rxtx_ctrl.sdio_callback->hal_sdio_device_recv_done(rxaddr, real_rx_len);
                    }
                    if ((sdio_rxtx_ctrl.rxbuf_cnt) && (sdio_rxtx_ctrl.rxaddr == NULL)) {
                        ASSERT(0, "%s:%d, rx error, invalid rxaddr", __func__, __LINE__);
                        break;
                    }
                    HAL_SDIO_DEVICE_TRACE(0, "------device rx ok");
                    HAL_SDIO_DEVICE_TRACE(TR_ATTR_NO_TS | TR_ATTR_NO_ID, "  ");

                    //enable next block transfer
                    sdio_device->CONTROL |= SDIO_PROGRAM_DONE;
                }
                break;
            }
            case SDIO_DMA1_INT: {
                HAL_SDIO_DEVICE_TRACE(1, "------DMA1 int ,After enabling adma, the interrupt is no longer used");
                break;
            }
            default: {
                break;
            }
        }
    }

    if (!status2)
        goto out;

    switch (status2) {
        default:
            HAL_SDIO_DEVICE_TRACE(2, "%s:%d,error:Unknown status2=0x%X", __func__, __LINE__, status2);
            break;
    }

out:
    hal_sdio_device_interrupt_status_enable();
    hal_sdio_device_interrupt_signal_enable();
}

static void hal_sdio_device_irq_enable(void)
{
    NVIC_SetVector(SDIO_DEVICE_IRQn, (uint32_t)hal_sdio_device_irq_handler);
    NVIC_SetPriority(SDIO_DEVICE_IRQn, IRQ_PRIORITY_NORMAL);
    NVIC_ClearPendingIRQ(SDIO_DEVICE_IRQn);
    NVIC_EnableIRQ(SDIO_DEVICE_IRQn);
}

static void hal_sdio_device_irq_disable(void)
{
    NVIC_DisableIRQ(SDIO_DEVICE_IRQn);
}

void hal_sdio_device_open(void)
{
    uint32_t val = 0;

    HAL_SDIO_DEVICE_TRACE(TR_ATTR_NO_TS | TR_ATTR_NO_ID, "  ");
    HAL_SDIO_DEVICE_TRACE(2, "%s:%d, start", __func__, __LINE__);

    hal_cmu_clock_enable(HAL_CMU_MOD_H_SDIO_SLAVE);
    hal_cmu_reset_clear(HAL_CMU_MOD_H_SDIO_SLAVE);

    memset(&sdio_rxtx_ctrl, 0, sizeof(struct SDIO_DEVICE_RXTX_CTRL_T));
    sdio_rxtx_ctrl.status = HAL_SDIO_DEVICE_NOT_RDY;
    hal_sdio_device_read_cccr(&val);       //Cross clock domain, so read and write again
    hal_sdio_device_write_cccr(val);
    hal_sdio_device_read_tplmanid(&val);   //Cross clock domain, so read and write again
    hal_sdio_device_write_tplmanid(val);
    hal_sdio_device_download_not_ready();
    hal_sdio_device_rx_error();
    hal_sdio_device_fws_stop_run();
    hal_sdio_device_write_card_addr(0);
    hal_sdio_device_interrupt_status_enable();
    hal_sdio_device_io_ready();
    hal_sdio_device_write_ocr();
#ifndef SDIO_DEVICE_ENABLE_DMA
    hal_sdio_device_enable_adma();
#endif
    hal_sdio_device_set_card_ready();
    hal_sdio_device_irq_enable();
    sdio_rxtx_ctrl.status = HAL_SDIO_DEVICE_RDY;

    HAL_SDIO_DEVICE_TRACE(2, "sdio_device->SDIO_CCCR_CONTROL=0x%X@0x%X",
                          (uint32_t)sdio_device->SDIO_CCCR_CONTROL,
                          (uint32_t)&sdio_device->SDIO_CCCR_CONTROL);
    HAL_SDIO_DEVICE_TRACE(2, "sdio_device->CARD_ADDRESS=0x%X@0x%X",
                          (uint32_t)sdio_device->CARD_ADDRESS,
                          (uint32_t)&sdio_device->CARD_ADDRESS);
    HAL_SDIO_DEVICE_TRACE(2, "sdio_device->INTERRUPT_STATUS_ENABLE=0x%X@0x%X",
                          (uint32_t)sdio_device->INTERRUPT_STATUS_ENABLE,
                          (uint32_t)&sdio_device->INTERRUPT_STATUS_ENABLE);
    HAL_SDIO_DEVICE_TRACE(2, "sdio_device->INTERRUPT_STATUS2_ENABLE=0x%X@0x%X",
                          (uint32_t)sdio_device->INTERRUPT_STATUS2_ENABLE,
                          (uint32_t)&sdio_device->INTERRUPT_STATUS2_ENABLE);
    HAL_SDIO_DEVICE_TRACE(2, "sdio_device->IO_READY=0x%X@0x%X",
                          (uint32_t)sdio_device->IO_READY,
                          (uint32_t)&sdio_device->IO_READY);
    HAL_SDIO_DEVICE_TRACE(2, "sdio_device->CARD_OCR=0x%X@0x%X",
                          (uint32_t)sdio_device->CARD_OCR,
                          (uint32_t)&sdio_device->CARD_OCR);
    HAL_SDIO_DEVICE_TRACE(2, "sdio_device->CONTROL=0x%X@0x%X",
                          (uint32_t)sdio_device->CONTROL,
                          (uint32_t)&sdio_device->CONTROL);
    HAL_SDIO_DEVICE_TRACE(2, "sdio_device->CONTROL2=0x%X@0x%X",
                          (uint32_t)sdio_device->CONTROL2,
                          (uint32_t)&sdio_device->CONTROL2);
    HAL_SDIO_DEVICE_TRACE(2, "%s:%d,end", __func__, __LINE__);
    HAL_SDIO_DEVICE_TRACE(TR_ATTR_NO_TS | TR_ATTR_NO_ID, "  ");
}

void hal_sdio_device_close(void)
{
    sdio_rxtx_ctrl.status = HAL_SDIO_DEVICE_NOT_RDY;
    sdio_device->CONTROL &= ~SDIO_CARD_INIT_DONE;
    hal_sdio_device_disable_adma();
    hal_sdio_device_irq_disable();
}

enum HAL_SDIO_DEVICE_STATUS hal_sdio_device_send(const uint8_t *buf, uint32_t buf_len)
{
    //TODO : To be optimized?,2021-06-08
    if (sdio_rxtx_ctrl.status != HAL_SDIO_DEVICE_IDLE) {
        HAL_SDIO_DEVICE_TRACE(1, "***sdio device status:%d,device is not idle", sdio_rxtx_ctrl.status);
        return HAL_SDIO_DEVICE_BUSY;
    }

    sdio_rxtx_ctrl.txaddr = (uint8_t *)buf;
    sdio_rxtx_ctrl.txlen = buf_len;
    ASSERT(buf_len <= SDIO_DEVICE_ADMA_TXRX_LEN, "%s:%d,maximum length is %d,len=%d", __func__, __LINE__, SDIO_DEVICE_ADMA_TXRX_LEN, buf_len);
    hal_sdio_device_gen_int_to_host(buf_len);//Notify the host to read data from the device

    HAL_SDIO_DEVICE_TRACE(1, "---Notify the host to read data:0x%X", buf_len);
    HAL_SDIO_DEVICE_TRACE(1, "sdio_rxtx_ctrl.txaddr=0x%X",
                          (uint32_t)sdio_rxtx_ctrl.txaddr);
    HAL_SDIO_DEVICE_TRACE(1, "sdio_rxtx_ctrl.txlen=%d",
                          (uint32_t)sdio_rxtx_ctrl.txlen);

    return HAL_SDIO_DEVICE_IDLE;
}

void hal_sdio_device_recv(uint8_t *buf, uint32_t buf_len, uint8_t buf_cnt)
{
    if (buf == NULL || buf_len == 0) {
        TR_ERROR(0, "rx buf is NULL. buf:%p, len:%d", buf, buf_len);
        return;
    }
    if (buf_len > SDIO_DEVICE_ADMA_TXRX_LEN) {
        TR_ERROR(0, "rx buf len %d > %d", buf_len, SDIO_DEVICE_ADMA_TXRX_LEN);
        return;
    }

    //rxbuf_cnt should be equal or greater than the old
    ASSERT(buf_cnt >= sdio_rxtx_ctrl.rxbuf_cnt, "%s, rxbuf_cnt mismatch:%d, %d",
           __func__, buf_cnt, sdio_rxtx_ctrl.rxbuf_cnt);

    sdio_rxtx_ctrl.rxaddr = buf;
    sdio_rxtx_ctrl.rxlen = buf_len;

    sdio_rxtx_ctrl.rxbuf_cnt = buf_cnt;
    hal_sdio_device_rxbuf_cfg(buf_len, buf_cnt);
}

void hal_sdio_device_register_callback(struct HAL_SDIO_DEVICE_CB_T *callback)
{
    sdio_rxtx_ctrl.sdio_callback = callback;
}

#endif

