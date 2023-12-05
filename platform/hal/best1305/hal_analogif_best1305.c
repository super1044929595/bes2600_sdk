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
#ifndef NO_ISPI

#include "plat_types.h"
#include "cmsis.h"
#include "hal_analogif.h"
#include "hal_location.h"
#include "hal_spi.h"
#include "hal_trace.h"

#define PMU_REG(r)                      (((r) & 0xFFF) | 0x0000)

#define ANA_REG_CHIP_ID                 PMU_REG(0x00)

#define ANA_CHIP_ID_SHIFT               (4)
#define ANA_CHIP_ID_MASK                (0xFFF << ANA_CHIP_ID_SHIFT)
#define ANA_CHIP_ID(n)                  BITFIELD_VAL(ANA_CHIP_ID, n)
#define ANA_VAL_CHIP_ID                 0x135

// ISPI_ARBITRATOR_ENABLE should be defined when:
// 1) BT and MCU will access RF register at the same time; or
// 2) BT can access PMU/ANA, and BT will access RF register at the same time
//    when MCU is accessing PMU/ANA register

#ifdef ISPI_ARBITRATOR_ENABLE
// Min padding OSC cycles needed: BT=0 MCU=6
// When OSC=26M and SPI=6.5M, min padding SPI cycles is BT=0 MCU=2
#define PADDING_CYCLES                  2
#else
#define PADDING_CYCLES                  0
#endif

#define ANA_READ_CMD(r)                 (((1 << 24) | (((r) & 0xFF) << 16)) << PADDING_CYCLES)
#define ANA_WRITE_CMD(r, v)             (((((r) & 0xFF) << 16) | ((v) & 0xFFFF)) << PADDING_CYCLES)
#define ANA_READ_VAL(v)                 (((v) >> PADDING_CYCLES) & 0xFFFF)

#define ANA_PAGE_0                      (0xA000)
#define ANA_PAGE_1                      (0xA010)
#define ANA_PAGE_2                      (0xA020)
#define ANA_PAGE_3                      (0xA030)
#define ANA_PAGE_QTY                    4

#define ISPI_REG_CS(r)                  ((r) >> 12)
#define ISPI_REG_PAGE(r)                (((r) >> 8) & 0xF)
#define ISPI_REG_OFFSET(r)              ((r) & 0xFF)

static bool BOOT_BSS_LOC analogif_inited;
static uint8_t BOOT_BSS_LOC ana_cs;
#ifdef ISPI_CHECK_RF_ACCESS
static enum HAL_ANALOGIF_RF_ACCESS_T BOOT_BSS_LOC rf_access;
#endif

static const BOOT_RODATA_FLASH_LOC struct HAL_SPI_CFG_T spi_cfg = {
    .clk_delay_half = false,
    .clk_polarity = false,
    .slave = false,
    .dma_rx = false,
    .dma_tx = false,
    .rx_sep_line = true,
    .cs = 0,
    .rate = 6500000,
    .tx_bits = 25 + PADDING_CYCLES,
    .rx_bits = 25 + PADDING_CYCLES,
    .rx_frame_bits = 0,
};

// Only (CS=0) have multiple pages
static const BOOT_RODATA_SRAM_LOC uint16_t page_reg[] = { 0x00, 0x60, 0x80, };
// Max page number is 4
static const BOOT_RODATA_SRAM_LOC uint16_t page_val[ANA_PAGE_QTY] = { ANA_PAGE_0, ANA_PAGE_1, ANA_PAGE_2, ANA_PAGE_3, };

enum HAL_ANALOGIF_RF_ACCESS_T hal_analogif_rf_access_config(enum HAL_ANALOGIF_RF_ACCESS_T type)
{
    enum HAL_ANALOGIF_RF_ACCESS_T old_val = HAL_ANALOGIF_RF_ACCESS_ALL;

#ifdef ISPI_CHECK_RF_ACCESS
    old_val = rf_access;
    rf_access = type;
#endif

    return old_val;
}

__STATIC_FORCEINLINE void hal_analogif_check_rf_access(uint8_t cs, uint8_t page, uint8_t reg)
{
#ifdef ISPI_CHECK_RF_ACCESS
    if (cs == 0 && reg >= 0x80) {
        ASSERT(rf_access != HAL_ANALOGIF_RF_ACCESS_NONE,
            "RF-ISPI: page=%u reg=0x%X caller=%p",
            page, reg, __builtin_return_address(0));
        ASSERT(rf_access != HAL_ANALOGIF_RF_ACCESS_NO_PAGE || page == 0,
            "RF-ISPI P: page=%u reg=0x%X caller=%p",
            page, reg, __builtin_return_address(0));
    }
#endif
}

static int BOOT_TEXT_SRAM_LOC hal_analogif_rawread(unsigned short reg, unsigned short *val)
{
    int ret;
    unsigned int data;
    unsigned int cmd;

    data = 0;
    cmd = ANA_READ_CMD(reg);
    ret = hal_ispi_recv(&cmd, &data, 4);
    if (ret) {
        return ret;
    }
    *val = ANA_READ_VAL(data);
    return 0;
}

static int BOOT_TEXT_SRAM_LOC hal_analogif_rawwrite(unsigned short reg, unsigned short val)
{
    int ret;
    unsigned int cmd;

    cmd = ANA_WRITE_CMD(reg, val);
    ret = hal_ispi_send(&cmd, 4);
    if (ret) {
        return ret;
    }
    return 0;
}

static uint8_t BOOT_TEXT_SRAM_LOC get_page_reg_idx(uint16_t reg)
{
    uint8_t idx;

    if (reg <= 0x5F) {
        idx = 0;
    } else if (reg <= 0x7F) {
        idx = 1;
    } else {
        idx = 2;
    }
    return idx;
}

int BOOT_TEXT_SRAM_LOC hal_analogif_reg_read(unsigned short reg, unsigned short *val)
{
    int ret;
    uint32_t lock;
    uint8_t cs;
    uint8_t page;
    uint8_t idx = 0;

    cs = ISPI_REG_CS(reg);
    page = ISPI_REG_PAGE(reg);
    reg = ISPI_REG_OFFSET(reg);

    hal_analogif_check_rf_access(cs, page, reg);

    lock = int_lock();
    if (cs != ana_cs) {
        hal_ispi_activate_cs(cs);
        ana_cs = cs;
    }
    if (page) {
        idx = get_page_reg_idx(reg);
        hal_analogif_rawwrite(page_reg[idx], page_val[page & 3]);
    }
    ret = hal_analogif_rawread(reg, val);
    if (page) {
        hal_analogif_rawwrite(page_reg[idx], page_val[0]);
    }
    int_unlock(lock);

    return ret;
}

int BOOT_TEXT_SRAM_LOC hal_analogif_reg_write(unsigned short reg, unsigned short val)
{
    int ret;
    uint32_t lock;
    uint8_t cs;
    uint8_t page;
    uint8_t idx = 0;

    cs = ISPI_REG_CS(reg);
    page = ISPI_REG_PAGE(reg);
    reg = ISPI_REG_OFFSET(reg);

    hal_analogif_check_rf_access(cs, page, reg);

    lock = int_lock();
    if (cs != ana_cs) {
        hal_ispi_activate_cs(cs);
        ana_cs = cs;
    }
    if (page) {
        idx = get_page_reg_idx(reg);
        hal_analogif_rawwrite(page_reg[idx], page_val[page & 3]);
    }
    ret = hal_analogif_rawwrite(reg, val);
    if (page) {
        hal_analogif_rawwrite(page_reg[idx], page_val[0]);
    }
    int_unlock(lock);

    return ret;
}

int BOOT_TEXT_FLASH_LOC hal_analogif_open(void)
{
    int ret;
    unsigned short chip_id;
    const struct HAL_SPI_CFG_T *cfg_ptr;
    struct HAL_SPI_CFG_T cfg;

    if (analogif_inited) {
        // Restore the nominal rate
        cfg_ptr = &spi_cfg;
    } else {
        analogif_inited = true;
        // Crystal freq is unknown yet. Let SPI run on half of the nominal rate
        cfg = spi_cfg;
        cfg.rate /= 2;
        cfg_ptr = &cfg;
    }

    ret = hal_ispi_open(cfg_ptr);
    if (ret) {
        return ret;
    }

    ana_cs = 0;
    hal_ispi_activate_cs(ana_cs);

    ret = hal_analogif_reg_read(ANA_REG_CHIP_ID, &chip_id);
    if (ret) {
        return ret;
    }

    if (GET_BITFIELD(chip_id, ANA_CHIP_ID) != ANA_VAL_CHIP_ID) {
        return -1;
    }

    return 0;
}

#endif

