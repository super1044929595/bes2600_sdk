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
#include "plat_addr_map.h"
#include CHIP_SPECIFIC_HDR(reg_cmu)
#include CHIP_SPECIFIC_HDR(reg_aoncmu)
#include CHIP_SPECIFIC_HDR(reg_btcmu)
#include "analog.h"
#include "cmsis_nvic.h"
#include "hal_aud.h"
#include "hal_bootmode.h"
#include "hal_chipid.h"
#include "hal_cmu.h"
#include "hal_codec.h"
#include "hal_location.h"
#include "hal_psc.h"
#include "hal_sleep_core_pd.h"
#include "hal_sysfreq.h"
#include "hal_timer.h"
#include "hal_trace.h"
#include "pmu.h"
#include "system_cp.h"

#ifdef USB_HIGH_SPEED
#ifndef USB_USE_USBPLL
#define USB_USE_USBPLL
#endif
#endif

#ifndef __PIE__
//#define MANUAL_RAM_RETENTION
#endif
//#define NO_RAM_RETENTION
#define RAM_RETENTION_ONE_BY_ONE

//#define RAM_NAP

#define CODEC_CLK_FROM_ANA

#define BEST1305_13M_WORKAROUND

#define HAL_CMU_USB_PLL_CLOCK           (192 * 1000 * 1000)
#define HAL_CMU_AUD_PLL_CLOCK           (CODEC_FREQ_48K_SERIES * CODEC_CMU_DIV)

#define HAL_CMU_USB_CLOCK_60M           (60 * 1000 * 1000)
#define HAL_CMU_USB_CLOCK_48M           (48 * 1000 * 1000)

#define HAL_CMU_PLL_LOCKED_TIMEOUT      US_TO_TICKS(200)
#define HAL_CMU_26M_READY_TIMEOUT       MS_TO_TICKS(4)
#define HAL_CMU_LPU_EXTRA_TIMEOUT       MS_TO_TICKS(1)

#ifdef CORE_SLEEP_POWER_DOWN
#define TIMER1_SEL_LOC                  BOOT_TEXT_SRAM_LOC
#else
#define TIMER1_SEL_LOC                  BOOT_TEXT_FLASH_LOC
#endif

enum CMU_AUD_26M_X4_USER_T {
    CMU_AUD_26M_X4_USER_IIR,
    CMU_AUD_26M_X4_USER_IIR_EQ,
    CMU_AUD_26M_X4_USER_FIR,
    CMU_AUD_26M_X4_USER_RS,
    CMU_AUD_26M_X4_USER_RS_ADC,

    CMU_AUD_26M_X4_USER_QTY,
};

enum CMU_DEBUG_REG_SEL_T {
    CMU_DEBUG_REG_SEL_MCU_PC        = 0,
    CMU_DEBUG_REG_SEL_MCU_LR        = 1,
    CMU_DEBUG_REG_SEL_MCU_SP        = 2,
    CMU_DEBUG_REG_SEL_CP_PC         = 3,
    CMU_DEBUG_REG_SEL_CP_LR         = 4,
    CMU_DEBUG_REG_SEL_CP_SP         = 5,
    CMU_DEBUG_REG_SEL_DEBUG         = 7,
};

enum CMU_DMA_REQ_T {
    CMU_DMA_REQ_CODEC_RX            = 0,
    CMU_DMA_REQ_CODEC_TX            = 1,
    CMU_DMA_REQ_PCM_RX              = 2,
    CMU_DMA_REQ_PCM_TX              = 3,
    CMU_DMA_REQ_I2S0_RX             = 4,
    CMU_DMA_REQ_I2S0_TX             = 5,
    CMU_DMA_REQ_RESERVED06          = 6,
    CMU_DMA_REQ_RESERVED07          = 7,
    CMU_DMA_REQ_RESERVED08          = 8,
    CMU_DMA_REQ_RESERVED09          = 9,
    CMU_DMA_REQ_RESERVED10          = 10,
    CMU_DMA_REQ_RESERVED11          = 11,
    CMU_DMA_REQ_BTDUMP              = 12,
    CMU_DMA_REQ_CODEC_MC            = 13,
    CMU_DMA_REQ_RESERVED14          = 14,
    CMU_DMA_REQ_RESERVED15          = 15,
    CMU_DMA_REQ_RESERVED16          = 16,
    CMU_DMA_REQ_RESERVED17          = 17,
    CMU_DMA_REQ_RESERVED18          = 18,
    CMU_DMA_REQ_RESERVED19          = 19,
    CMU_DMA_REQ_FLS0                = 20,
    CMU_DMA_REQ_RESERVED21          = 21,
    CMU_DMA_REQ_I2C0_RX             = 22,
    CMU_DMA_REQ_I2C0_TX             = 23,
    CMU_DMA_REQ_SPILCD0_RX          = 24,
    CMU_DMA_REQ_SPILCD0_TX          = 25,
    CMU_DMA_REQ_RESERVED26          = 26,
    CMU_DMA_REQ_RESERVED27          = 27,
    CMU_DMA_REQ_UART0_RX            = 28,
    CMU_DMA_REQ_UART0_TX            = 29,
    CMU_DMA_REQ_UART1_RX            = 30,
    CMU_DMA_REQ_UART1_TX            = 31,
    CMU_DMA_REQ_I2C1_RX             = 32,
    CMU_DMA_REQ_I2C1_TX             = 33,
    CMU_DMA_REQ_RESERVED34          = 34,
    CMU_DMA_REQ_RESERVED35          = 35,
    CMU_DMA_REQ_SPI_ITN_RX          = 36,
    CMU_DMA_REQ_SPI_ITN_TX          = 37,
    CMU_DMA_REQ_RESERVED38          = 38,
    CMU_DMA_REQ_RESERVED39          = 39,
    CMU_DMA_REQ_I2C2_RX             = 40,
    CMU_DMA_REQ_I2C2_TX             = 41,
    CMU_DMA_REQ_CODEC_TX2           = 42,

    CMU_DMA_REQ_QTY,
    CMU_DMA_REQ_NULL                = CMU_DMA_REQ_QTY,
};

struct CP_STARTUP_CFG_T {
    __IO uint32_t stack;
    __IO uint32_t reset_hdlr;
};

static uint32_t cp_entry;

static struct CMU_T * const cmu = (struct CMU_T *)CMU_BASE;

static struct AONCMU_T * const aoncmu = (struct AONCMU_T *)AON_CMU_BASE;

static struct BTCMU_T * const POSSIBLY_UNUSED btcmu = (struct BTCMU_T *)BT_CMU_BASE;

#define HAL_CMU_PLL_USB_HS              HAL_CMU_PLL_QTY
#ifdef USB_USE_USBPLL
#define PLL_USER_MAP_NUM                (HAL_CMU_PLL_QTY + 1)
#else
#define PLL_USER_MAP_NUM                HAL_CMU_PLL_QTY
#endif
static uint8_t BOOT_BSS_LOC pll_user_map[PLL_USER_MAP_NUM];
STATIC_ASSERT(HAL_CMU_PLL_USER_QTY <= sizeof(pll_user_map[0]) * 8, "Too many PLL users");

static bool anc_enabled;

#ifdef __AUDIO_RESAMPLE__
static bool aud_resample_en = true;
#ifdef ANA_26M_X4_ENABLE
static uint8_t aud_26m_x4_map;
STATIC_ASSERT(CMU_AUD_26M_X4_USER_QTY <= sizeof(aud_26m_x4_map) * 8, "Too many aud_26m_x4 users");
#endif
#endif

#ifdef LOW_SYS_FREQ
static enum HAL_CMU_FREQ_T BOOT_BSS_LOC cmu_sys_freq;
#endif

void hal_cmu_audio_resample_enable(void)
{
#ifdef __AUDIO_RESAMPLE__
    aud_resample_en = true;
#endif
}

void hal_cmu_audio_resample_disable(void)
{
#ifdef __AUDIO_RESAMPLE__
    aud_resample_en = false;
#endif
}

int hal_cmu_get_audio_resample_status(void)
{
#ifdef __AUDIO_RESAMPLE__
    return aud_resample_en;
#else
    return false;
#endif
}

static inline void aocmu_reg_update_wait(void)
{
    // Make sure AOCMU (26M clock domain) write opertions finish before return
    aoncmu->CHIP_ID;
}

int hal_cmu_clock_enable(enum HAL_CMU_MOD_ID_T id)
{
    if (id >= HAL_CMU_AON_MCU) {
        return 1;
    }

    if (id < HAL_CMU_MOD_P_CMU) {
        cmu->HCLK_ENABLE = (1 << id);
    } else if (id < HAL_CMU_MOD_O_SLEEP) {
        cmu->PCLK_ENABLE = (1 << (id - HAL_CMU_MOD_P_CMU));
    } else if (id < HAL_CMU_AON_A_CMU) {
        cmu->OCLK_ENABLE = (1 << (id - HAL_CMU_MOD_O_SLEEP));
    } else {
        aoncmu->MOD_CLK_ENABLE = (1 << (id - HAL_CMU_AON_A_CMU));
        aocmu_reg_update_wait();
    }

    return 0;
}

int hal_cmu_clock_disable(enum HAL_CMU_MOD_ID_T id)
{
    if (id >= HAL_CMU_AON_MCU) {
        return 1;
    }

    if (id < HAL_CMU_MOD_P_CMU) {
        cmu->HCLK_DISABLE = (1 << id);
    } else if (id < HAL_CMU_MOD_O_SLEEP) {
        cmu->PCLK_DISABLE = (1 << (id - HAL_CMU_MOD_P_CMU));
    } else if (id < HAL_CMU_AON_A_CMU) {
        cmu->OCLK_DISABLE = (1 << (id - HAL_CMU_MOD_O_SLEEP));
    } else {
        aoncmu->MOD_CLK_DISABLE = (1 << (id - HAL_CMU_AON_A_CMU));
    }

    return 0;
}

enum HAL_CMU_CLK_STATUS_T hal_cmu_clock_get_status(enum HAL_CMU_MOD_ID_T id)
{
    uint32_t status;

    if (id >= HAL_CMU_AON_MCU) {
        return HAL_CMU_CLK_DISABLED;
    }

    if (id < HAL_CMU_MOD_P_CMU) {
        status = cmu->HCLK_ENABLE & (1 << id);
    } else if (id < HAL_CMU_MOD_O_SLEEP) {
        status = cmu->PCLK_ENABLE & (1 << (id - HAL_CMU_MOD_P_CMU));
    } else if (id < HAL_CMU_AON_A_CMU) {
        status = cmu->OCLK_ENABLE & (1 << (id - HAL_CMU_MOD_O_SLEEP));
    } else {
        status = aoncmu->MOD_CLK_ENABLE & (1 << (id - HAL_CMU_AON_A_CMU));
    }

    return status ? HAL_CMU_CLK_ENABLED : HAL_CMU_CLK_DISABLED;
}

int hal_cmu_clock_set_mode(enum HAL_CMU_MOD_ID_T id, enum HAL_CMU_CLK_MODE_T mode)
{
    __IO uint32_t *reg;
    uint32_t val;
    uint32_t lock;

    if (id >= HAL_CMU_AON_MCU) {
        return 1;
    }

    if (id < HAL_CMU_MOD_P_CMU) {
        reg = &cmu->HCLK_MODE;
        val = (1 << id);
    } else if (id < HAL_CMU_MOD_O_SLEEP) {
        reg = &cmu->PCLK_MODE;
        val = (1 << (id - HAL_CMU_MOD_P_CMU));
    } else if (id < HAL_CMU_AON_A_CMU) {
        reg = &cmu->OCLK_MODE;
        val = (1 << (id - HAL_CMU_MOD_O_SLEEP));
    } else {
        reg = &aoncmu->MOD_CLK_MODE;
        val = (1 << (id - HAL_CMU_AON_A_CMU));
    }

    lock = int_lock();
    if (mode == HAL_CMU_CLK_MANUAL) {
        *reg |= val;
    } else {
        *reg &= ~val;
    }
    int_unlock(lock);

    return 0;
}

enum HAL_CMU_CLK_MODE_T hal_cmu_clock_get_mode(enum HAL_CMU_MOD_ID_T id)
{
    uint32_t mode;

    if (id >= HAL_CMU_AON_MCU) {
        return HAL_CMU_CLK_AUTO;
    }

    if (id < HAL_CMU_MOD_P_CMU) {
        mode = cmu->HCLK_MODE & (1 << id);
    } else if (id < HAL_CMU_MOD_O_SLEEP) {
        mode = cmu->PCLK_MODE & (1 << (id - HAL_CMU_MOD_P_CMU));
    } else if (id < HAL_CMU_AON_A_CMU) {
        mode = cmu->OCLK_MODE & (1 << (id - HAL_CMU_MOD_O_SLEEP));
    } else {
        mode = aoncmu->MOD_CLK_MODE & (1 << (id - HAL_CMU_AON_A_CMU));
    }

    return mode ? HAL_CMU_CLK_MANUAL : HAL_CMU_CLK_AUTO;
}

int hal_cmu_reset_set(enum HAL_CMU_MOD_ID_T id)
{
    if (id >= HAL_CMU_MOD_QTY) {
        return 1;
    }

    if (id < HAL_CMU_MOD_P_CMU) {
        cmu->HRESET_SET = (1 << id);
    } else if (id < HAL_CMU_MOD_O_SLEEP) {
        cmu->PRESET_SET = (1 << (id - HAL_CMU_MOD_P_CMU));
    } else if (id < HAL_CMU_AON_A_CMU) {
        cmu->ORESET_SET = (1 << (id - HAL_CMU_MOD_O_SLEEP));
    } else {
        aoncmu->RESET_SET = (1 << (id - HAL_CMU_AON_A_CMU));
    }

    return 0;
}

int hal_cmu_reset_clear(enum HAL_CMU_MOD_ID_T id)
{
    if (id >= HAL_CMU_MOD_QTY) {
        return 1;
    }

    if (id < HAL_CMU_MOD_P_CMU) {
        cmu->HRESET_CLR = (1 << id);
        asm volatile("nop; nop;");
    } else if (id < HAL_CMU_MOD_O_SLEEP) {
        cmu->PRESET_CLR = (1 << (id - HAL_CMU_MOD_P_CMU));
        asm volatile("nop; nop; nop; nop;");
    } else if (id < HAL_CMU_AON_A_CMU) {
        cmu->ORESET_CLR = (1 << (id - HAL_CMU_MOD_O_SLEEP));
    } else {
        aoncmu->RESET_CLR = (1 << (id - HAL_CMU_AON_A_CMU));
        aocmu_reg_update_wait();
    }

    return 0;
}

enum HAL_CMU_RST_STATUS_T hal_cmu_reset_get_status(enum HAL_CMU_MOD_ID_T id)
{
    uint32_t status;

    if (id >= HAL_CMU_MOD_QTY) {
        return HAL_CMU_RST_SET;
    }

    if (id < HAL_CMU_MOD_P_CMU) {
        status = cmu->HRESET_SET & (1 << id);
    } else if (id < HAL_CMU_MOD_O_SLEEP) {
        status = cmu->PRESET_SET & (1 << (id - HAL_CMU_MOD_P_CMU));
    } else if (id < HAL_CMU_AON_A_CMU) {
        status = cmu->ORESET_SET & (1 << (id - HAL_CMU_MOD_O_SLEEP));
    } else {
        status = aoncmu->RESET_SET & (1 << (id - HAL_CMU_AON_A_CMU));
    }

    return status ? HAL_CMU_RST_CLR : HAL_CMU_RST_SET;
}

int hal_cmu_reset_pulse(enum HAL_CMU_MOD_ID_T id)
{
    volatile int i;

    if (id >= HAL_CMU_MOD_QTY) {
        return 1;
    }

    if (hal_cmu_reset_get_status(id) == HAL_CMU_RST_SET) {
        return hal_cmu_reset_clear(id);
    }

    if (id < HAL_CMU_MOD_P_CMU) {
        cmu->HRESET_PULSE = (1 << id);
    } else if (id < HAL_CMU_MOD_O_SLEEP) {
        cmu->PRESET_PULSE = (1 << (id - HAL_CMU_MOD_P_CMU));
    } else if (id < HAL_CMU_AON_A_CMU) {
        cmu->ORESET_PULSE = (1 << (id - HAL_CMU_MOD_O_SLEEP));
    } else {
        aoncmu->RESET_PULSE = (1 << (id - HAL_CMU_AON_A_CMU));
        // Total 3 CLK-26M cycles needed
        // AOCMU runs in 26M clock domain and its read operations consume at least 1 26M-clock cycle.
        // (Whereas its write operations will finish at 1 HCLK cycle -- finish once in async bridge fifo)
        aoncmu->CHIP_ID;
        aoncmu->CHIP_ID;
        aoncmu->CHIP_ID;
    }
    // Delay 5+ PCLK cycles (10+ HCLK cycles)
    for (i = 0; i < 3; i++);

    return 0;
}

int hal_cmu_timer_set_div(enum HAL_CMU_TIMER_ID_T id, uint32_t div)
{
    uint32_t lock;

    if (div < 1) {
        return 1;
    }

    div -= 1;
    if ((div & (CMU_CFG_DIV_TIMER00_MASK >> CMU_CFG_DIV_TIMER00_SHIFT)) != div) {
        return 1;
    }

    lock = int_lock();
    if (id == HAL_CMU_TIMER_ID_00) {
        cmu->TIMER0_CLK = SET_BITFIELD(cmu->TIMER0_CLK, CMU_CFG_DIV_TIMER00, div);
    } else if (id == HAL_CMU_TIMER_ID_01) {
        cmu->TIMER0_CLK = SET_BITFIELD(cmu->TIMER0_CLK, CMU_CFG_DIV_TIMER01, div);
    } else if (id == HAL_CMU_TIMER_ID_10) {
        cmu->TIMER1_CLK = SET_BITFIELD(cmu->TIMER1_CLK, CMU_CFG_DIV_TIMER10, div);
    } else if (id == HAL_CMU_TIMER_ID_11) {
        cmu->TIMER1_CLK = SET_BITFIELD(cmu->TIMER1_CLK, CMU_CFG_DIV_TIMER11, div);
    } else if (id == HAL_CMU_TIMER_ID_20) {
        cmu->TIMER2_CLK = SET_BITFIELD(cmu->TIMER2_CLK, CMU_CFG_DIV_TIMER20, div);
    } else if (id == HAL_CMU_TIMER_ID_21) {
        cmu->TIMER2_CLK = SET_BITFIELD(cmu->TIMER2_CLK, CMU_CFG_DIV_TIMER21, div);
    }
    int_unlock(lock);

    return 0;
}

void BOOT_TEXT_FLASH_LOC hal_cmu_timer0_select_fast(void)
{
    uint32_t lock;

    lock = int_lock();
    // 6.5M
    cmu->PERIPH_CLK |= (1 << CMU_SEL_TIMER_FAST_SHIFT);
    // AON Timer
    aoncmu->CLK_SELECT |= AON_CMU_SEL_TIMER_FAST;
    int_unlock(lock);
}

void BOOT_TEXT_FLASH_LOC hal_cmu_timer0_select_slow(void)
{
    uint32_t lock;

    lock = int_lock();
    // 16K
    cmu->PERIPH_CLK &= ~(1 << CMU_SEL_TIMER_FAST_SHIFT);
    // AON Timer
    aoncmu->CLK_SELECT &= ~AON_CMU_SEL_TIMER_FAST;
    int_unlock(lock);
}

void TIMER1_SEL_LOC hal_cmu_timer1_select_fast(void)
{
    uint32_t lock;

    lock = int_lock();
    // 6.5M
    cmu->PERIPH_CLK |= (1 << (CMU_SEL_TIMER_FAST_SHIFT + 1));
    int_unlock(lock);
}

void TIMER1_SEL_LOC hal_cmu_timer1_select_slow(void)
{
    uint32_t lock;

    lock = int_lock();
    // 16K
    cmu->PERIPH_CLK &= ~(1 << (CMU_SEL_TIMER_FAST_SHIFT + 1));
    int_unlock(lock);
}

#ifdef OSC_26M_X4_AUD2BB

// Any of 78M/104M/208M is changed to 26M x4 (104M)

#define SYS_SET_FREQ_FUNC(f, F, CLK_OV) \
int hal_cmu_ ##f## _set_freq(enum HAL_CMU_FREQ_T freq) \
{ \
    uint32_t enable; \
    uint32_t disable; \
    if (freq >= HAL_CMU_FREQ_QTY) { \
        return 1; \
    } \
    if (freq == HAL_CMU_FREQ_32K) { \
        enable = 0; \
        disable = CMU_SEL_OSC_ ##F## _DISABLE | CMU_SEL_OSCX2_ ##F## _DISABLE | \
            CMU_SEL_PLL_ ##F## _DISABLE | CMU_RSTN_DIV_ ##F## _DISABLE | CMU_BYPASS_DIV_ ##F## _DISABLE; \
    } else if (freq == HAL_CMU_FREQ_26M) { \
        enable = CMU_SEL_OSC_ ##F## _ENABLE; \
        disable = CMU_SEL_OSCX2_ ##F## _DISABLE | \
            CMU_SEL_PLL_ ##F## _DISABLE | CMU_RSTN_DIV_ ##F## _DISABLE | CMU_BYPASS_DIV_ ##F## _DISABLE; \
    } else if (freq == HAL_CMU_FREQ_52M) { \
        enable = CMU_SEL_OSCX2_ ##F## _ENABLE; \
        disable = CMU_SEL_PLL_ ##F## _DISABLE | CMU_RSTN_DIV_ ##F## _DISABLE | CMU_BYPASS_DIV_ ##F## _DISABLE; \
    } else { \
        enable = CMU_SEL_PLL_ ##F## _ENABLE | CMU_BYPASS_DIV_ ##F## _ENABLE; \
        disable = CMU_RSTN_DIV_ ##F## _DISABLE; \
    } \
    if (enable & CMU_SEL_PLL_ ##F## _ENABLE) { \
        CLK_OV; \
        cmu->SYS_CLK_ENABLE = CMU_RSTN_DIV_ ##F## _ENABLE; \
        if (enable & CMU_BYPASS_DIV_ ##F## _ENABLE) { \
            cmu->SYS_CLK_ENABLE = CMU_BYPASS_DIV_ ##F## _ENABLE; \
        } else { \
            cmu->SYS_CLK_DISABLE = CMU_BYPASS_DIV_ ##F## _DISABLE; \
        } \
    } \
    cmu->SYS_CLK_ENABLE = enable; \
    if (enable & CMU_SEL_PLL_ ##F## _ENABLE) { \
        cmu->SYS_CLK_DISABLE = disable; \
    } else { \
        cmu->SYS_CLK_DISABLE = disable & ~(CMU_RSTN_DIV_ ##F## _DISABLE | CMU_BYPASS_DIV_ ##F## _DISABLE); \
        cmu->SYS_CLK_DISABLE = CMU_BYPASS_DIV_ ##F## _DISABLE; \
        cmu->SYS_CLK_DISABLE = CMU_RSTN_DIV_ ##F## _DISABLE; \
    } \
    return 0; \
}

#else // !OSC_26M_X4_AUD2BB

#define SYS_SET_FREQ_FUNC(f, F, CLK_OV) \
int hal_cmu_ ##f## _set_freq(enum HAL_CMU_FREQ_T freq) \
{ \
    uint32_t lock; \
    uint32_t enable; \
    uint32_t disable; \
    int div = -1; \
    if (freq >= HAL_CMU_FREQ_QTY) { \
        return 1; \
    } \
    if (freq == HAL_CMU_FREQ_32K) { \
        enable = 0; \
        disable = CMU_SEL_OSC_ ##F## _DISABLE | CMU_SEL_OSCX2_ ##F## _DISABLE | \
            CMU_SEL_PLL_ ##F## _DISABLE | CMU_RSTN_DIV_ ##F## _DISABLE | CMU_BYPASS_DIV_ ##F## _DISABLE; \
    } else if (freq == HAL_CMU_FREQ_26M) { \
        enable = CMU_SEL_OSC_ ##F## _ENABLE; \
        disable = CMU_SEL_OSCX2_ ##F## _DISABLE | \
            CMU_SEL_PLL_ ##F## _DISABLE | CMU_RSTN_DIV_ ##F## _DISABLE | CMU_BYPASS_DIV_ ##F## _DISABLE; \
    } else if (freq == HAL_CMU_FREQ_52M) { \
        enable = CMU_SEL_OSCX2_ ##F## _ENABLE; \
        disable = CMU_SEL_PLL_ ##F## _DISABLE | CMU_RSTN_DIV_ ##F## _DISABLE | CMU_BYPASS_DIV_ ##F## _DISABLE; \
    } else if (freq == HAL_CMU_FREQ_78M) { \
        enable = CMU_SEL_PLL_ ##F## _ENABLE | CMU_RSTN_DIV_ ##F## _ENABLE; \
        disable = CMU_BYPASS_DIV_ ##F## _DISABLE; \
        div = 1; \
    } else if (freq == HAL_CMU_FREQ_104M) { \
        enable = CMU_SEL_PLL_ ##F## _ENABLE | CMU_RSTN_DIV_ ##F## _ENABLE; \
        disable = CMU_BYPASS_DIV_ ##F## _DISABLE; \
        div = 0; \
    } else { \
        enable = CMU_SEL_PLL_ ##F## _ENABLE | CMU_BYPASS_DIV_ ##F## _ENABLE; \
        disable = CMU_RSTN_DIV_ ##F## _DISABLE; \
    } \
    if (div >= 0) { \
        CLK_OV; \
        lock = int_lock(); \
        cmu->SYS_DIV = SET_BITFIELD(cmu->SYS_DIV, CMU_CFG_DIV_ ##F, div); \
        int_unlock(lock); \
    } \
    if (enable & CMU_SEL_PLL_ ##F## _ENABLE) { \
        cmu->SYS_CLK_ENABLE = CMU_RSTN_DIV_ ##F## _ENABLE; \
        if (enable & CMU_BYPASS_DIV_ ##F## _ENABLE) { \
            cmu->SYS_CLK_ENABLE = CMU_BYPASS_DIV_ ##F## _ENABLE; \
        } else { \
            cmu->SYS_CLK_DISABLE = CMU_BYPASS_DIV_ ##F## _DISABLE; \
        } \
    } \
    cmu->SYS_CLK_ENABLE = enable; \
    if (enable & CMU_SEL_PLL_ ##F## _ENABLE) { \
        cmu->SYS_CLK_DISABLE = disable; \
    } else { \
        cmu->SYS_CLK_DISABLE = disable & ~(CMU_RSTN_DIV_ ##F## _DISABLE | CMU_BYPASS_DIV_ ##F## _DISABLE); \
        cmu->SYS_CLK_DISABLE = CMU_BYPASS_DIV_ ##F## _DISABLE; \
        cmu->SYS_CLK_DISABLE = CMU_RSTN_DIV_ ##F## _DISABLE; \
    } \
    return 0; \
}

#endif // !OSC_26M_X4_AUD2BB

#ifdef MCU_SYS_CLOCK_400M
#define FLASH_DIV_OFFSET                2
#elif defined(MCU_SYS_CLOCK_300M)
#define FLASH_DIV_OFFSET                1
#else
#define FLASH_DIV_OFFSET                0
#endif

#ifdef OSC_26M_X4_AUD2BB
#define FLASH_FREQ_OV                   { aoncmu->CLK_OUT |= AON_CMU_SEL_X4_FLS; }
#else
#define FLASH_FREQ_OV                   { div += FLASH_DIV_OFFSET; }
#endif

BOOT_TEXT_SRAM_LOC SYS_SET_FREQ_FUNC(flash, FLS, FLASH_FREQ_OV);

#ifdef LOW_SYS_FREQ
#ifdef LOW_SYS_FREQ_6P5M
int BOOT_TEXT_SRAM_LOC hal_cmu_fast_timer_offline(void)
{
    return (cmu_sys_freq == HAL_CMU_FREQ_6P5M);
}
#endif

static int hal_cmu_freq_is_special_low(enum HAL_CMU_FREQ_T freq)
{
#ifdef BEST1305_13M_WORKAROUND
    return (freq == HAL_CMU_FREQ_6P5M);
#else
    return (freq == HAL_CMU_FREQ_6P5M ||
            freq == HAL_CMU_FREQ_13M);
#endif
}

static void hal_cmu_osc_to_dig_x4_enable(void)
{
    aoncmu->TOP_CLK_ENABLE = AON_CMU_EN_X4_ANA_ENABLE;
    aoncmu->CLK_SELECT |= AON_CMU_SEL_X4_SYS;
    aoncmu->CLK_SELECT |= AON_CMU_SEL_X4_DIG;
    aoncmu->CLK_OUT |= AON_CMU_SEL_X4_SLOW;
}

static void hal_cmu_osc_to_dig_x4_disable(void)
{
    aoncmu->CLK_SELECT &= ~AON_CMU_SEL_X4_DIG;
    aoncmu->CLK_OUT &= ~AON_CMU_SEL_X4_SLOW;
#ifndef OSC_26M_X4_AUD2BB
    aoncmu->CLK_SELECT &= ~AON_CMU_SEL_X4_SYS;
#endif
#ifndef ANA_26M_X4_ENABLE
    aoncmu->TOP_CLK_DISABLE = AON_CMU_EN_X4_ANA_DISABLE;
#endif
}
#endif

int hal_cmu_sys_set_freq(enum HAL_CMU_FREQ_T freq)
{
    uint32_t lock;
    uint32_t enable;
    uint32_t disable;
    int div = -1;

    if (freq >= HAL_CMU_FREQ_QTY) {
        return 1;
    }

    lock = int_lock();

#ifdef LOW_SYS_FREQ
    int next_low;
    int cur_low;

    next_low = hal_cmu_freq_is_special_low(freq);
    cur_low = hal_cmu_freq_is_special_low(cmu_sys_freq);
    if (next_low != cur_low) {
        // Switch to 26M first
        cmu->SYS_CLK_ENABLE = CMU_SEL_OSC_SYS_ENABLE;
        cmu->SYS_CLK_DISABLE = CMU_SEL_OSCX2_SYS_DISABLE | CMU_SEL_PLL_SYS_DISABLE;
        if (next_low) {
            hal_cmu_osc_to_dig_x4_enable();
        } else {
            hal_cmu_osc_to_dig_x4_disable();
        }
    }
    cmu_sys_freq = freq;
#endif

    switch (freq) {
    case HAL_CMU_FREQ_32K:
        enable = 0;
        disable = CMU_SEL_OSC_SYS_DISABLE | CMU_SEL_OSCX2_SYS_DISABLE |
            CMU_SEL_PLL_SYS_DISABLE | CMU_RSTN_DIV_SYS_DISABLE | CMU_BYPASS_DIV_SYS_DISABLE;
        break;
    case HAL_CMU_FREQ_6P5M:
#ifndef BEST1305_13M_WORKAROUND
#if defined(LOW_SYS_FREQ) && defined(LOW_SYS_FREQ_6P5M)
        enable = CMU_SEL_PLL_SYS_ENABLE | CMU_RSTN_DIV_SYS_ENABLE;
        disable = CMU_BYPASS_DIV_SYS_DISABLE;
        div = 0;
        break;
#endif
    case HAL_CMU_FREQ_13M:
#endif
#ifdef LOW_SYS_FREQ
        enable = CMU_SEL_PLL_SYS_ENABLE | CMU_BYPASS_DIV_SYS_ENABLE;
        disable = CMU_RSTN_DIV_SYS_DISABLE;
        break;
#endif
#ifdef BEST1305_13M_WORKAROUND
    case HAL_CMU_FREQ_13M:
#endif
    case HAL_CMU_FREQ_26M:
        enable = CMU_SEL_OSC_SYS_ENABLE;
        disable = CMU_SEL_OSCX2_SYS_DISABLE |
            CMU_SEL_PLL_SYS_DISABLE | CMU_RSTN_DIV_SYS_DISABLE | CMU_BYPASS_DIV_SYS_DISABLE;
        break;
    case HAL_CMU_FREQ_52M:
        enable = CMU_SEL_OSCX2_SYS_ENABLE;
        disable = CMU_SEL_PLL_SYS_DISABLE | CMU_RSTN_DIV_SYS_DISABLE | CMU_BYPASS_DIV_SYS_DISABLE;
        break;
#ifndef OSC_26M_X4_AUD2BB
    case HAL_CMU_FREQ_78M:
        enable = CMU_SEL_PLL_SYS_ENABLE | CMU_RSTN_DIV_SYS_ENABLE;
        disable = CMU_BYPASS_DIV_SYS_DISABLE;
        div = 1;
        break;
    case HAL_CMU_FREQ_104M:
        enable = CMU_SEL_PLL_SYS_ENABLE | CMU_RSTN_DIV_SYS_ENABLE;
        disable = CMU_BYPASS_DIV_SYS_DISABLE;
        div = 0;
        break;
#endif
    default:
        enable = CMU_SEL_PLL_SYS_ENABLE | CMU_BYPASS_DIV_SYS_ENABLE;
        disable = CMU_RSTN_DIV_SYS_DISABLE;
        break;
    }

    if (div >= 0) {
        cmu->SYS_DIV = SET_BITFIELD(cmu->SYS_DIV, CMU_CFG_DIV_SYS, div);
    }

    if (enable & CMU_SEL_PLL_SYS_ENABLE) {
        cmu->SYS_CLK_ENABLE = CMU_RSTN_DIV_SYS_ENABLE;
        if (enable & CMU_BYPASS_DIV_SYS_ENABLE) {
            cmu->SYS_CLK_ENABLE = CMU_BYPASS_DIV_SYS_ENABLE;
        } else {
            cmu->SYS_CLK_DISABLE = CMU_BYPASS_DIV_SYS_DISABLE;
        }
    }
    cmu->SYS_CLK_ENABLE = enable;
    if (enable & CMU_SEL_PLL_SYS_ENABLE) {
        cmu->SYS_CLK_DISABLE = disable;
    } else {
        cmu->SYS_CLK_DISABLE = disable & ~(CMU_RSTN_DIV_SYS_DISABLE | CMU_BYPASS_DIV_SYS_DISABLE);
        cmu->SYS_CLK_DISABLE = CMU_BYPASS_DIV_SYS_DISABLE;
        cmu->SYS_CLK_DISABLE = CMU_RSTN_DIV_SYS_DISABLE;
    }

    int_unlock(lock);

    return 0;
}

int hal_cmu_mem_set_freq(enum HAL_CMU_FREQ_T freq)
{
    return 0;
}

enum HAL_CMU_FREQ_T BOOT_TEXT_SRAM_LOC hal_cmu_sys_get_freq(void)
{
    uint32_t sys_clk;
    uint32_t div;

    sys_clk = cmu->SYS_CLK_ENABLE;

    if (sys_clk & CMU_SEL_PLL_SYS_ENABLE) {
        if (sys_clk & CMU_BYPASS_DIV_SYS_ENABLE) {
            return HAL_CMU_FREQ_208M;
        } else {
            div = GET_BITFIELD(cmu->SYS_DIV, CMU_CFG_DIV_SYS);
            if (div == 0) {
                return HAL_CMU_FREQ_104M;
            } else if (div == 1) {
                // (div == 1): 69M
                return HAL_CMU_FREQ_78M;
            } else {
                // (div == 2): 52M
                // (div == 3): 42M
                return HAL_CMU_FREQ_52M;
            }
        }
    } else if (sys_clk & CMU_SEL_OSCX2_SYS_ENABLE) {
        return HAL_CMU_FREQ_52M;
    } else if (sys_clk & CMU_SEL_OSC_SYS_ENABLE) {
        return HAL_CMU_FREQ_26M;
    } else {
        return HAL_CMU_FREQ_32K;
    }
}

int BOOT_TEXT_SRAM_LOC hal_cmu_flash_select_pll(enum HAL_CMU_PLL_T pll)
{
    return hal_cmu_sys_select_pll(pll);
}

int hal_cmu_mem_select_pll(enum HAL_CMU_PLL_T pll)
{
    return hal_cmu_sys_select_pll(pll);
}

// hal_cmu_flash_select_pll() requires in BOOT_TEXT_SRAM_LOC
int BOOT_TEXT_SRAM_LOC hal_cmu_sys_select_pll(enum HAL_CMU_PLL_T pll)
{
    uint32_t lock;
    uint32_t sel;

    if (pll >= HAL_CMU_PLL_QTY) {
        return 1;
    }

    lock = int_lock();
    // 0/1:bbpll, 2:audpll, 3:usbpll
    sel = (pll == HAL_CMU_PLL_AUD) ? 2 : 0;
    aoncmu->CLK_SELECT = SET_BITFIELD(aoncmu->CLK_SELECT, AON_CMU_SEL_PLL_SYS, sel);
    int_unlock(lock);

    return 0;
}

int hal_cmu_get_pll_status(enum HAL_CMU_PLL_T pll)
{
    return !!(aoncmu->TOP_CLK_ENABLE & ((pll == HAL_CMU_PLL_AUD) ? AON_CMU_EN_CLK_TOP_PLLAUD_ENABLE : AON_CMU_EN_CLK_TOP_PLLUSB_ENABLE));
}

int hal_cmu_pll_enable(enum HAL_CMU_PLL_T pll, enum HAL_CMU_PLL_USER_T user)
{
    uint32_t pu_val;
    uint32_t en_val;
    uint32_t check;
    uint32_t lock;
    uint32_t sel;
    uint32_t start;
    uint32_t timeout;

    if (pll >= HAL_CMU_PLL_QTY) {
        return 1;
    }
    if (user >= HAL_CMU_PLL_USER_QTY && user != HAL_CMU_PLL_USER_ALL) {
        return 2;
    }

#ifdef USB_USE_USBPLL
    if (pll == HAL_CMU_PLL_USB && user == HAL_CMU_PLL_USER_USB) {
        pll = HAL_CMU_PLL_USB_HS;
    }
#endif

    if (pll == HAL_CMU_PLL_AUD) {
        pu_val = AON_CMU_PU_PLLAUD_ENABLE;
        en_val = AON_CMU_EN_CLK_TOP_PLLAUD_ENABLE;
        check = AON_CMU_LOCK_PLLAUD;
#ifdef USB_USE_USBPLL
    } else if (pll == HAL_CMU_PLL_USB_HS) {
        pu_val = AON_CMU_PU_PLLUSB_ENABLE;
        en_val = AON_CMU_EN_CLK_TOP_PLLUSB_ENABLE;
        check = AON_CMU_LOCK_PLLUSB;
#endif
    } else {
        pu_val = AON_CMU_PU_PLLBB_ENABLE;
        en_val = AON_CMU_EN_CLK_TOP_PLLBB_ENABLE;
        check = AON_CMU_LOCK_PLLBB;
    }

    lock = int_lock();
    if (pll_user_map[pll] == 0 || user == HAL_CMU_PLL_USER_ALL) {
#ifndef ROM_BUILD
        pmu_pll_div_reset_set(pll);
#endif
        aoncmu->TOP_CLK_ENABLE = pu_val;
#ifndef ROM_BUILD
        hal_sys_timer_delay_us(20);
        pmu_pll_div_reset_clear(pll);
        // Wait at least 10us for clock ready
#endif
    } else {
        check = 0;
    }
    if (user < HAL_CMU_PLL_USER_QTY) {
        pll_user_map[pll] |= (1 << user);
    }
    if (user == HAL_CMU_PLL_USER_AUD) {
        // 0/1:audpll, 2:bbpll, 3:usbpll
        sel = (pll == HAL_CMU_PLL_AUD) ? 0 : 2;
        aoncmu->CLK_SELECT = SET_BITFIELD(aoncmu->CLK_SELECT, AON_CMU_SEL_PLL_AUD, sel);
    }
    // HAL_CMU_PLL_USER_SYS selects PLL in hal_cmu_sys_select_pll()
    int_unlock(lock);

    start = hal_sys_timer_get();
    timeout = HAL_CMU_PLL_LOCKED_TIMEOUT;
    do {
        if (check) {
            if (aoncmu->CODEC_DIV & check) {
                //break;
            }
        } else {
            if (aoncmu->TOP_CLK_ENABLE & en_val) {
                break;
            }
        }
    } while ((hal_sys_timer_get() - start) < timeout);

    aoncmu->TOP_CLK_ENABLE = en_val;

#ifndef USB_USE_USBPLL
    if (pll == HAL_CMU_PLL_USB && user == HAL_CMU_PLL_USER_USB) {
        aoncmu->TOP_CLK_ENABLE = AON_CMU_EN_CLK_TOP_PLLBB2_ENABLE;
    }
#endif

    return (aoncmu->CODEC_DIV & check) ? 0 : 2;
}

int hal_cmu_pll_disable(enum HAL_CMU_PLL_T pll, enum HAL_CMU_PLL_USER_T user)
{
    uint32_t lock;

    if (pll >= HAL_CMU_PLL_QTY) {
        return 1;
    }
    if (user >= HAL_CMU_PLL_USER_QTY && user != HAL_CMU_PLL_USER_ALL) {
        return 2;
    }

    if (pll == HAL_CMU_PLL_USB && user == HAL_CMU_PLL_USER_USB) {
#ifdef USB_USE_USBPLL
        pll = HAL_CMU_PLL_USB_HS;
#else
        aoncmu->TOP_CLK_DISABLE = AON_CMU_EN_CLK_TOP_PLLBB2_DISABLE;
#endif
    }

    lock = int_lock();
    if (user < HAL_CMU_PLL_USER_ALL) {
        pll_user_map[pll] &= ~(1 << user);
    }
    if (pll_user_map[pll] == 0 || user == HAL_CMU_PLL_USER_ALL) {
        if (pll == HAL_CMU_PLL_AUD) {
            aoncmu->TOP_CLK_DISABLE = AON_CMU_EN_CLK_TOP_PLLAUD_DISABLE;
            aoncmu->TOP_CLK_DISABLE = AON_CMU_PU_PLLAUD_DISABLE;
#ifdef USB_USE_USBPLL
        } else if (pll == HAL_CMU_PLL_USB_HS) {
            aoncmu->TOP_CLK_DISABLE = AON_CMU_EN_CLK_TOP_PLLUSB_DISABLE;
            aoncmu->TOP_CLK_DISABLE = AON_CMU_PU_PLLUSB_DISABLE;
#endif
        } else {
            aoncmu->TOP_CLK_DISABLE = AON_CMU_EN_CLK_TOP_PLLBB_DISABLE;
            aoncmu->TOP_CLK_DISABLE = AON_CMU_PU_PLLBB_DISABLE;
        }
    }
    int_unlock(lock);

    return 0;
}

void BOOT_TEXT_FLASH_LOC hal_cmu_low_freq_mode_init(void)
{
#if defined(MCU_HIGH_PERFORMANCE_MODE)
    hal_cmu_sys_select_pll(HAL_CMU_PLL_USB);
#else
    // No need to switch to USB PLL, for there is a clock gate and a clock mux
    // in front of the AUD/USB switch
    hal_cmu_sys_select_pll(HAL_CMU_PLL_AUD);
#endif

//#ifdef FLASH_LOW_SPEED
#ifdef OSC_26M_X4_AUD2BB
    aoncmu->CLK_SELECT &= ~AON_CMU_SEL_X4_SYS;
#endif
//#endif
}

void hal_cmu_low_freq_mode_enable(enum HAL_CMU_FREQ_T old_freq, enum HAL_CMU_FREQ_T new_freq)
{
    // TODO: Need to lock irq?
    enum HAL_CMU_PLL_T POSSIBLY_UNUSED pll;

#if defined(MCU_HIGH_PERFORMANCE_MODE)
    pll = HAL_CMU_PLL_USB;
#else
    pll = HAL_CMU_PLL_AUD;
#endif

#ifdef OSC_26M_X4_AUD2BB
    if (new_freq <= HAL_CMU_FREQ_52M) {
        aoncmu->CLK_SELECT &= ~AON_CMU_SEL_X4_SYS;
    }
    if (new_freq <= HAL_CMU_FREQ_104M && old_freq > HAL_CMU_FREQ_104M) {
        if (new_freq > HAL_CMU_FREQ_52M) {
            // PLL is in use now. Switch to X2 first.
            hal_cmu_sys_set_freq(HAL_CMU_FREQ_52M);
            aoncmu->CLK_SELECT |= AON_CMU_SEL_X4_SYS;
            // X4 is in use now
            hal_cmu_sys_set_freq(new_freq);
        }
        hal_cmu_pll_disable(pll, HAL_CMU_PLL_USER_SYS);
    }
#else
#ifdef FLASH_LOW_SPEED
    if (old_freq > HAL_CMU_FREQ_52M && new_freq <= HAL_CMU_FREQ_52M) {
        hal_cmu_pll_disable(pll, HAL_CMU_PLL_USER_SYS);
    }
#endif
#endif
}

void hal_cmu_low_freq_mode_disable(enum HAL_CMU_FREQ_T old_freq, enum HAL_CMU_FREQ_T new_freq)
{
    // TODO: Need to lock irq?
    enum HAL_CMU_PLL_T POSSIBLY_UNUSED pll;

#if defined(MCU_HIGH_PERFORMANCE_MODE)
    pll = HAL_CMU_PLL_USB;
#else
    pll = HAL_CMU_PLL_AUD;
#endif

#ifdef OSC_26M_X4_AUD2BB
    if (new_freq <= HAL_CMU_FREQ_52M) {
        aoncmu->CLK_SELECT &= ~AON_CMU_SEL_X4_SYS;
    } else if (new_freq <= HAL_CMU_FREQ_104M) {
        aoncmu->CLK_SELECT |= AON_CMU_SEL_X4_SYS;
    } else {
        if (old_freq <= HAL_CMU_FREQ_104M) {
            hal_cmu_pll_enable(pll, HAL_CMU_PLL_USER_SYS);
            if (old_freq > HAL_CMU_FREQ_52M) {
                // X4 is in use now. Switch to X2 before stopping X4
                hal_cmu_sys_set_freq(HAL_CMU_FREQ_52M);
            }
            aoncmu->CLK_SELECT &= ~AON_CMU_SEL_X4_SYS;
        }
    }
#else
#ifdef FLASH_LOW_SPEED
    if (old_freq <= HAL_CMU_FREQ_52M && new_freq > HAL_CMU_FREQ_52M) {
        hal_cmu_pll_enable(pll, HAL_CMU_PLL_USER_SYS);
    }
#endif
#endif
}

int hal_cmu_codec_adc_set_div(uint32_t div)
{
    uint32_t lock;

    if (div < 2) {
        return 1;
    }

    div -= 2;
    lock = int_lock();
    aoncmu->CODEC_DIV = SET_BITFIELD(aoncmu->CODEC_DIV, AON_CMU_CFG_DIV_CODEC, div);
    int_unlock(lock);

    return 0;
}

uint32_t hal_cmu_codec_adc_get_div(void)
{
    return GET_BITFIELD(aoncmu->CODEC_DIV, AON_CMU_CFG_DIV_CODEC) + 2;
}

int hal_cmu_codec_dac_set_div(uint32_t div)
{
    return hal_cmu_codec_adc_set_div(div);
}

uint32_t hal_cmu_codec_dac_get_div(void)
{
    return hal_cmu_codec_adc_get_div();;
}

#if defined(__AUDIO_RESAMPLE__) && defined(ANA_26M_X4_ENABLE)
void hal_cmu_audio_26m_x4_enable(enum CMU_AUD_26M_X4_USER_T user)
{
    uint32_t lock;

    if (user >= CMU_AUD_26M_X4_USER_QTY) {
        return;
    }

    lock = int_lock();

    if (aud_26m_x4_map == 0) {
        aoncmu->CLK_SELECT |= AON_CMU_SEL_X4_AUD;
    }
    aud_26m_x4_map |= (1 << user);

    int_unlock(lock);
}

void hal_cmu_audio_26m_x4_disable(enum CMU_AUD_26M_X4_USER_T user)
{
    uint32_t lock;

    if (user >= CMU_AUD_26M_X4_USER_QTY) {
        return;
    }

    lock = int_lock();

    if (aud_26m_x4_map & (1 << user)) {
        aud_26m_x4_map &= ~(1 << user);
        if (aud_26m_x4_map == 0) {
            aoncmu->CLK_SELECT &= ~AON_CMU_SEL_X4_AUD;
        }
    }

    int_unlock(lock);
}
#endif

void hal_cmu_codec_iir_enable(uint32_t speed)
{
    uint32_t lock;
    uint32_t mask;
    uint32_t val;
    uint32_t div;
    uint32_t cfg_speed = 0;

    mask = AON_CMU_SEL_OSC_CODECIIR | AON_CMU_SEL_OSCX2_CODECIIR | AON_CMU_BYPASS_DIV_CODECIIR;
    val = 0;

    if (speed <= 26000000) {
        val |= AON_CMU_SEL_OSC_CODECIIR | AON_CMU_SEL_OSCX2_CODECIIR;
        cfg_speed = 26000000;
    } else if (speed <= 52000000) {
        val |= AON_CMU_SEL_OSCX2_CODECIIR;
        cfg_speed = 52000000;
    } else {
#if defined(__AUDIO_RESAMPLE__) && defined(ANA_26M_X4_ENABLE)
        if (hal_cmu_get_audio_resample_status()) {
            hal_cmu_audio_26m_x4_enable(CMU_AUD_26M_X4_USER_IIR);
            val |= AON_CMU_BYPASS_DIV_CODECIIR;
            cfg_speed = 104000000;
        }
        else
#endif
        {
            // Assume audio stream is one of 48K series
            div = HAL_CMU_AUD_PLL_CLOCK / speed;
            if (div >= 2) {
                hal_cmu_codec_iir_set_div(div);
                cfg_speed = HAL_CMU_AUD_PLL_CLOCK / div;
            } else {
                val |= AON_CMU_BYPASS_DIV_CODECIIR;
                cfg_speed = HAL_CMU_AUD_PLL_CLOCK;
            }
            analog_aud_pll_open(ANA_AUD_PLL_USER_IIR);
        }
    }

    ASSERT(speed <= cfg_speed, "%s: speed %u should <= cfg_speed %u", __func__, speed, cfg_speed);

    lock = int_lock();
    aoncmu->CODEC_IIR = (aoncmu->CODEC_IIR & ~mask) | val;
    int_unlock(lock);

    aoncmu->TOP_CLK_ENABLE = AON_CMU_EN_CLK_CODEC_IIR_ENABLE;

    aocmu_reg_update_wait();
}

void hal_cmu_codec_iir_disable(void)
{
    uint32_t lock;
    uint32_t val;

    aoncmu->TOP_CLK_DISABLE = AON_CMU_EN_CLK_CODEC_IIR_DISABLE;

    val = AON_CMU_SEL_OSC_CODECIIR | AON_CMU_SEL_OSCX2_CODECIIR;

    lock = int_lock();
    aoncmu->CODEC_IIR |= val;
    int_unlock(lock);

#if defined(__AUDIO_RESAMPLE__) && defined(ANA_26M_X4_ENABLE)
    hal_cmu_audio_26m_x4_disable(CMU_AUD_26M_X4_USER_IIR);
#endif
    analog_aud_pll_close(ANA_AUD_PLL_USER_IIR);
}

int hal_cmu_codec_iir_set_div(uint32_t div)
{
    uint32_t lock;

    if (div < 2) {
        return 1;
    }

    div -= 2;
    lock = int_lock();
    aoncmu->CODEC_IIR = SET_BITFIELD(aoncmu->CODEC_IIR, AON_CMU_CFG_DIV_CODECIIR, div);
    int_unlock(lock);

    return 0;
}

void hal_cmu_codec_iir_eq_enable(uint32_t speed)
{
    uint32_t lock;
    uint32_t mask;
    uint32_t val;
    uint32_t div;
    uint32_t cfg_speed = 0;

    mask = AON_CMU_SEL_OSC_4_CODECIIR1 | AON_CMU_SEL_OSC_2_CODECIIR1 | AON_CMU_SEL_OSC_CODECIIR1 |
        AON_CMU_SEL_OSCX2_CODECIIR1 | AON_CMU_BYPASS_DIV_CODECIIR1;
    val = AON_CMU_EN_CLK_CODEC_IIR1;

    if (speed <= 13000000) {
        val |= AON_CMU_SEL_OSC_2_CODECIIR1 | AON_CMU_SEL_OSC_CODECIIR1 | AON_CMU_SEL_OSCX2_CODECIIR1;
        cfg_speed = 13000000;
    } else if (speed <= 26000000) {
        val |= AON_CMU_SEL_OSC_CODECIIR1 | AON_CMU_SEL_OSCX2_CODECIIR1;
        cfg_speed = 26000000;
    } else if (speed <= 52000000) {
        val |= AON_CMU_SEL_OSCX2_CODECIIR1;
        cfg_speed = 52000000;
    } else {
#if defined(__AUDIO_RESAMPLE__) && defined(ANA_26M_X4_ENABLE)
        if (hal_cmu_get_audio_resample_status()) {
            hal_cmu_audio_26m_x4_enable(CMU_AUD_26M_X4_USER_IIR_EQ);
            val |= AON_CMU_BYPASS_DIV_CODECIIR1;
            cfg_speed = 104000000;
        }
        else
#endif
        {
            // Assume audio stream is one of 48K series
            div = HAL_CMU_AUD_PLL_CLOCK / speed;
            if (div >= 2) {
                hal_cmu_codec_iir_set_div(div);
                cfg_speed = HAL_CMU_AUD_PLL_CLOCK / div;
            } else {
                val |= AON_CMU_BYPASS_DIV_CODECIIR1;
                cfg_speed = HAL_CMU_AUD_PLL_CLOCK;
            }
            analog_aud_pll_open(ANA_AUD_PLL_USER_IIR_EQ);
        }
    }

    ASSERT(speed <= cfg_speed, "%s: speed %u should <= cfg_speed %u", __func__, speed, cfg_speed);

    lock = int_lock();
    aoncmu->CODEC_IIR = (aoncmu->CODEC_IIR & ~mask) | val;
    int_unlock(lock);

    aocmu_reg_update_wait();
}

void hal_cmu_codec_iir_eq_disable(void)
{
    uint32_t lock;
    uint32_t val;
    uint32_t mask;

    mask = AON_CMU_EN_CLK_CODEC_IIR1;
    val = AON_CMU_SEL_OSC_4_CODECIIR1 | AON_CMU_SEL_OSC_2_CODECIIR1 | AON_CMU_SEL_OSC_CODECIIR1 | AON_CMU_SEL_OSCX2_CODECIIR1;

    lock = int_lock();
    aoncmu->CODEC_IIR = (aoncmu->CODEC_IIR & ~mask) | val;
    int_unlock(lock);

#if defined(__AUDIO_RESAMPLE__) && defined(ANA_26M_X4_ENABLE)
    hal_cmu_audio_26m_x4_disable(CMU_AUD_26M_X4_USER_IIR_EQ);
#endif
    analog_aud_pll_close(ANA_AUD_PLL_USER_IIR_EQ);
}

int hal_cmu_codec_iir_eq_set_div(uint32_t div)
{
    uint32_t lock;

    if (div < 2) {
        return 1;
    }

    div -= 2;
    lock = int_lock();
    aoncmu->CODEC_IIR = SET_BITFIELD(aoncmu->CODEC_IIR, AON_CMU_CFG_DIV_CODECIIR1, div);
    int_unlock(lock);

    return 0;
}

void hal_cmu_codec_rs_enable(uint32_t speed)
{
    uint32_t lock;
    uint32_t mask;
    uint32_t val;
    uint32_t div;
    uint32_t cfg_speed = 0;

    mask = AON_CMU_SEL_OSC_CODECRS | AON_CMU_SEL_OSCX2_CODECRS | AON_CMU_BYPASS_DIV_CODECRS;
    val = 0;

    if (speed <= 26000000) {
        val |= AON_CMU_SEL_OSC_CODECRS | AON_CMU_SEL_OSCX2_CODECRS;
        cfg_speed = 26000000;
    } else if (speed <= 52000000) {
        val |= AON_CMU_SEL_OSCX2_CODECRS;
        cfg_speed = 52000000;
    } else {
#if defined(__AUDIO_RESAMPLE__) && defined(ANA_26M_X4_ENABLE)
        if (hal_cmu_get_audio_resample_status()) {
            hal_cmu_audio_26m_x4_enable(CMU_AUD_26M_X4_USER_RS);
            val |= AON_CMU_BYPASS_DIV_CODECRS;
            cfg_speed = 104000000;
        }
        else
#endif
        {
            // Assume audio stream is one of 48K series
            div = HAL_CMU_AUD_PLL_CLOCK / speed;
            if (div >= 2) {
                hal_cmu_codec_rs_set_div(div);
                cfg_speed = HAL_CMU_AUD_PLL_CLOCK / div;
            } else {
                val |= AON_CMU_BYPASS_DIV_CODECRS;
                cfg_speed = HAL_CMU_AUD_PLL_CLOCK;
            }
            analog_aud_pll_open(ANA_AUD_PLL_USER_RS_DAC);
        }

        pmu_rs_freq_config(cfg_speed);
    }

    ASSERT(speed <= cfg_speed, "%s: speed %u should <= cfg_speed %u", __func__, speed, cfg_speed);

    lock = int_lock();
    aoncmu->CODEC_IIR = (aoncmu->CODEC_IIR & ~mask) | val;
    int_unlock(lock);

    aoncmu->TOP_CLK_ENABLE = AON_CMU_EN_CLK_CODEC_RS_ENABLE;

    aocmu_reg_update_wait();
}

void hal_cmu_codec_rs_disable(void)
{
    uint32_t lock;
    bool high_speed;

    aoncmu->TOP_CLK_DISABLE = AON_CMU_EN_CLK_CODEC_RS_DISABLE;

    high_speed = !(aoncmu->CODEC_IIR & AON_CMU_SEL_OSC_CODECRS);

    lock = int_lock();
    aoncmu->CODEC_IIR |= AON_CMU_SEL_OSC_CODECRS | AON_CMU_SEL_OSCX2_CODECRS;
    int_unlock(lock);

    if (high_speed) {
        pmu_rs_freq_config(0);
    }

#if defined(__AUDIO_RESAMPLE__) && defined(ANA_26M_X4_ENABLE)
    hal_cmu_audio_26m_x4_disable(CMU_AUD_26M_X4_USER_RS);
#endif
    analog_aud_pll_close(ANA_AUD_PLL_USER_RS_DAC);
}

int hal_cmu_codec_rs_set_div(uint32_t div)
{
    uint32_t lock;

    if (div < 2) {
        return 1;
    }

    div -= 2;
    lock = int_lock();
    aoncmu->CODEC_IIR = SET_BITFIELD(aoncmu->CODEC_IIR, AON_CMU_CFG_DIV_CODECRS, div);
    int_unlock(lock);

    return 0;
}

void hal_cmu_codec_rs_adc_enable(uint32_t speed)
{
    uint32_t lock;
    uint32_t mask;
    uint32_t val;
    uint32_t div;
    uint32_t cfg_speed = 0;

    mask = AON_CMU_SEL_OSC_CODECRS1 | AON_CMU_SEL_OSCX2_CODECRS1 | AON_CMU_BYPASS_DIV_CODECRS1;
    val = AON_CMU_EN_CLK_CODEC_RS1;

    if (speed <= 26000000) {
        val |= AON_CMU_SEL_OSC_CODECRS1 | AON_CMU_SEL_OSCX2_CODECRS1;
        cfg_speed = 26000000;
    } else if (speed <= 52000000) {
        val |= AON_CMU_SEL_OSCX2_CODECRS1;
        cfg_speed = 52000000;
    } else {
#if defined(__AUDIO_RESAMPLE__) && defined(ANA_26M_X4_ENABLE)
        if (hal_cmu_get_audio_resample_status()) {
            hal_cmu_audio_26m_x4_enable(CMU_AUD_26M_X4_USER_RS_ADC);
            val |= AON_CMU_BYPASS_DIV_CODECRS1;
            cfg_speed = 104000000;
        }
        else
#endif
        {
            // Assume audio stream is one of 48K series
            div = HAL_CMU_AUD_PLL_CLOCK / speed;
            if (div >= 2) {
                hal_cmu_codec_iir_set_div(div);
                cfg_speed = HAL_CMU_AUD_PLL_CLOCK / div;
            } else {
                val |= AON_CMU_BYPASS_DIV_CODECRS1;
                cfg_speed = HAL_CMU_AUD_PLL_CLOCK;
            }
            analog_aud_pll_open(ANA_AUD_PLL_USER_RS_ADC);
        }
    }

    ASSERT(speed <= cfg_speed, "%s: speed %u should <= cfg_speed %u", __func__, speed, cfg_speed);

    lock = int_lock();
    aoncmu->CODEC_IIR = (aoncmu->CODEC_IIR & ~mask) | val;
    int_unlock(lock);

    aocmu_reg_update_wait();
}

void hal_cmu_codec_rs_adc_disable(void)
{
    uint32_t lock;
    uint32_t val;
    uint32_t mask;

    mask = AON_CMU_EN_CLK_CODEC_RS1;
    val = AON_CMU_SEL_OSC_CODECRS1 | AON_CMU_SEL_OSCX2_CODECRS1;

    lock = int_lock();
    aoncmu->CODEC_IIR = (aoncmu->CODEC_IIR & ~mask) | val;
    int_unlock(lock);

#if defined(__AUDIO_RESAMPLE__) && defined(ANA_26M_X4_ENABLE)
    hal_cmu_audio_26m_x4_disable(CMU_AUD_26M_X4_USER_RS_ADC);
#endif
    analog_aud_pll_close(ANA_AUD_PLL_USER_RS_ADC);
}

int hal_cmu_codec_rs_adc_set_div(uint32_t div)
{
    uint32_t lock;

    if (div < 2) {
        return 1;
    }

    div -= 2;
    lock = int_lock();
    aoncmu->CODEC_IIR = SET_BITFIELD(aoncmu->CODEC_IIR, AON_CMU_CFG_DIV_CODECRS1, div);
    int_unlock(lock);

    return 0;
}

void hal_cmu_anc_enable(enum HAL_CMU_ANC_CLK_USER_T user)
{
    anc_enabled = true;
}

void hal_cmu_anc_disable(enum HAL_CMU_ANC_CLK_USER_T user)
{
    anc_enabled = false;
}

int hal_cmu_anc_get_status(enum HAL_CMU_ANC_CLK_USER_T user)
{
    return anc_enabled;
}

void hal_cmu_codec_clock_enable(void)
{
    uint32_t clk;

#ifdef CODEC_CLK_FROM_ANA
    // Always use ANA clock
    clk = AON_CMU_EN_CLK_CODEC_HCLK_ENABLE | AON_CMU_EN_CLK_CODEC_ENABLE;
#else
#ifdef __AUDIO_RESAMPLE__
    if (hal_cmu_get_audio_resample_status()) {
        uint32_t lock;
        lock = int_lock();
        aoncmu->CODEC_DIV = (aoncmu->CODEC_DIV & ~AON_CMU_SEL_OSC_CODEC) | AON_CMU_SEL_OSC_2_CODEC;
        int_unlock(lock);

#ifdef RESAMPLE_CODEC_CLK_ANA
        clk = AON_CMU_EN_CLK_CODEC_HCLK_ENABLE | AON_CMU_EN_CLK_CODEC_ENABLE;
#else
        clk = AON_CMU_EN_CLK_PLL_CODEC_ENABLE | AON_CMU_EN_CLK_CODEC_HCLK_ENABLE | AON_CMU_EN_CLK_CODEC_ENABLE;
#endif
    }
    else
#endif
    {
        clk = AON_CMU_EN_CLK_PLL_CODEC_ENABLE | AON_CMU_EN_CLK_CODEC_HCLK_ENABLE | AON_CMU_EN_CLK_CODEC_ENABLE;
    }
#endif
    aoncmu->TOP_CLK_ENABLE = clk;
    hal_cmu_clock_enable(HAL_CMU_MOD_H_CODEC);
}

void hal_cmu_codec_clock_disable(void)
{
    uint32_t clk;

    hal_cmu_clock_disable(HAL_CMU_MOD_H_CODEC);

#ifdef CODEC_CLK_FROM_ANA
    clk = AON_CMU_EN_CLK_CODEC_HCLK_DISABLE | AON_CMU_EN_CLK_CODEC_DISABLE;
#else
#ifdef __AUDIO_RESAMPLE__
    if (hal_cmu_get_audio_resample_status()) {
        uint32_t lock;
        lock = int_lock();
        aoncmu->CODEC_DIV &= ~(AON_CMU_SEL_OSC_2_CODEC | AON_CMU_SEL_OSC_CODEC);
        int_unlock(lock);

        clk = AON_CMU_EN_CLK_CODEC_HCLK_DISABLE | AON_CMU_EN_CLK_CODEC_DISABLE;
    }
    else
#endif
    {
        clk = AON_CMU_EN_CLK_PLL_CODEC_DISABLE | AON_CMU_EN_CLK_CODEC_HCLK_DISABLE | AON_CMU_EN_CLK_CODEC_DISABLE;
    }
#endif
    aoncmu->TOP_CLK_DISABLE = clk;
}

void hal_cmu_codec_high_speed_enable(void)
{
    uint32_t lock;
    uint32_t div;

    if ((aoncmu->CODEC_DIV & AON_CMU_SEL_OSC_CODEC) == 0) {
        lock = int_lock();
        aoncmu->CODEC_DIV = (aoncmu->CODEC_DIV & ~AON_CMU_SEL_OSCX2_CODEC) | AON_CMU_SEL_OSC_CODEC;
        int_unlock(lock);

        div = hal_cmu_codec_adc_get_div();
        hal_cmu_codec_adc_set_div(div / 2);
    }
}

void hal_cmu_codec_high_speed_disable(void)
{
    uint32_t lock;
    uint32_t div;

    if (aoncmu->CODEC_DIV & AON_CMU_SEL_OSC_CODEC) {
        lock = int_lock();
        aoncmu->CODEC_DIV = (aoncmu->CODEC_DIV & ~(AON_CMU_SEL_OSC_CODEC | AON_CMU_SEL_OSCX2_CODEC));
        int_unlock(lock);

        div = hal_cmu_codec_adc_get_div();
        hal_cmu_codec_adc_set_div(div * 2);
    }
}

void hal_cmu_codec_reset_set(void)
{
    aoncmu->RESET_SET = AON_CMU_SOFT_RSTN_CODEC_SET;
}

void hal_cmu_codec_reset_clear(void)
{
    aoncmu->RESET_CLR = AON_CMU_SOFT_RSTN_CODEC_CLR;
    aocmu_reg_update_wait();
}

void hal_cmu_codec_set_fault_mask(uint32_t msk)
{
    uint32_t lock;

    lock = int_lock();
    // If bit set 1, DAC will be muted when some faults occur
    cmu->PERIPH_CLK = SET_BITFIELD(cmu->PERIPH_CLK, CMU_MASK_OBS, msk);
    int_unlock(lock);
}

void hal_cmu_i2s_clock_out_enable(enum HAL_I2S_ID_T id)
{
    uint32_t lock;
    uint32_t val;

    if (id == HAL_I2S_ID_0) {
        val = CMU_EN_CLK_I2S0_OUT;
    } else {
        val = CMU_EN_CLK_I2S1_OUT;
    }

    lock = int_lock();
    cmu->I2C_CLK |= val;
    int_unlock(lock);
}

void hal_cmu_i2s_clock_out_disable(enum HAL_I2S_ID_T id)
{
    uint32_t lock;
    uint32_t val;

    if (id == HAL_I2S_ID_0) {
        val = CMU_EN_CLK_I2S0_OUT;
    } else {
        val = CMU_EN_CLK_I2S1_OUT;
    }

    lock = int_lock();
    cmu->I2C_CLK &= ~val;
    int_unlock(lock);
}

void hal_cmu_i2s_set_slave_mode(enum HAL_I2S_ID_T id)
{
    uint32_t lock;
    uint32_t val;

    if (id == HAL_I2S_ID_0) {
        val = CMU_SEL_I2S0_CLKIN;
    } else {
        val = CMU_SEL_I2S1_CLKIN;
    }

    lock = int_lock();
    cmu->I2C_CLK |= val;
    int_unlock(lock);
}

void hal_cmu_i2s_set_master_mode(enum HAL_I2S_ID_T id)
{
    uint32_t lock;
    uint32_t val;

    if (id == HAL_I2S_ID_0) {
        val = CMU_SEL_I2S0_CLKIN;
    } else {
        val = CMU_SEL_I2S1_CLKIN;
    }

    lock = int_lock();
    cmu->I2C_CLK &= ~val;
    int_unlock(lock);
}

void hal_cmu_i2s_clock_enable(enum HAL_I2S_ID_T id)
{
    uint32_t lock;
    uint32_t val;
    volatile uint32_t *reg;

    if (id == HAL_I2S_ID_0) {
        val = AON_CMU_EN_CLK_PLL_I2S0;
        reg = &aoncmu->PCM_I2S_CLK;
    } else {
        val = AON_CMU_EN_CLK_PLL_I2S1;
        reg = &aoncmu->SPDIF_CLK;
    }

    lock = int_lock();
    *reg |= val;
    int_unlock(lock);
}

void hal_cmu_i2s_clock_disable(enum HAL_I2S_ID_T id)
{
    uint32_t lock;
    uint32_t val;
    volatile uint32_t *reg;

    if (id == HAL_I2S_ID_0) {
        val = AON_CMU_EN_CLK_PLL_I2S0;
        reg = &aoncmu->PCM_I2S_CLK;
    } else {
        val = AON_CMU_EN_CLK_PLL_I2S1;
        reg = &aoncmu->SPDIF_CLK;
    }

    lock = int_lock();
    *reg &= ~val;
    int_unlock(lock);
}

int hal_cmu_i2s_set_div(enum HAL_I2S_ID_T id, uint32_t div)
{
    uint32_t lock;

    if (div < 2) {
        return 1;
    }

    div -= 2;
    if ((div & (AON_CMU_CFG_DIV_I2S0_MASK >> AON_CMU_CFG_DIV_I2S0_SHIFT)) != div) {
        return 1;
    }

    lock = int_lock();
    if (id == HAL_I2S_ID_0) {
        aoncmu->PCM_I2S_CLK = SET_BITFIELD(aoncmu->PCM_I2S_CLK, AON_CMU_CFG_DIV_I2S0, div);
    } else {
        aoncmu->SPDIF_CLK = SET_BITFIELD(aoncmu->SPDIF_CLK, AON_CMU_CFG_DIV_I2S1, div);
    }
    int_unlock(lock);

    return 0;
}

void hal_cmu_pcm_clock_out_enable(void)
{
    uint32_t lock;

    lock = int_lock();
    cmu->I2C_CLK |= CMU_EN_CLK_PCM_OUT;
    int_unlock(lock);
}

void hal_cmu_pcm_clock_out_disable(void)
{
    uint32_t lock;

    lock = int_lock();
    cmu->I2C_CLK &= ~CMU_EN_CLK_PCM_OUT;
    int_unlock(lock);
}

void hal_cmu_pcm_set_slave_mode(int clk_pol)
{
    uint32_t lock;
    uint32_t mask;
    uint32_t cfg;

    mask = CMU_SEL_PCM_CLKIN | CMU_POL_CLK_PCM_IN;

    if (clk_pol) {
        cfg = CMU_SEL_PCM_CLKIN | CMU_POL_CLK_PCM_IN;
    } else {
        cfg = CMU_SEL_PCM_CLKIN;
    }

    lock = int_lock();
    cmu->I2C_CLK = (cmu->I2C_CLK & ~mask) | cfg;
    int_unlock(lock);
}

void hal_cmu_pcm_set_master_mode(void)
{
    uint32_t lock;

    lock = int_lock();
    cmu->I2C_CLK &= ~CMU_SEL_PCM_CLKIN;
    int_unlock(lock);
}

void hal_cmu_pcm_clock_enable(void)
{
    uint32_t lock;

    lock = int_lock();
    aoncmu->PCM_I2S_CLK |= AON_CMU_EN_CLK_PLL_PCM;
    int_unlock(lock);
}

void hal_cmu_pcm_clock_disable(void)
{
    uint32_t lock;

    lock = int_lock();
    aoncmu->PCM_I2S_CLK &= ~AON_CMU_EN_CLK_PLL_PCM;
    int_unlock(lock);
}

int hal_cmu_pcm_set_div(uint32_t div)
{
    uint32_t lock;

    if (div < 2) {
        return 1;
    }

    div -= 2;
    if ((div & (AON_CMU_CFG_DIV_PCM_MASK >> AON_CMU_CFG_DIV_PCM_SHIFT)) != div) {
        return 1;
    }

    lock = int_lock();
    aoncmu->PCM_I2S_CLK = SET_BITFIELD(aoncmu->PCM_I2S_CLK, AON_CMU_CFG_DIV_PCM, div);
    int_unlock(lock);
    return 0;
}

void BOOT_TEXT_FLASH_LOC hal_cmu_apb_init_div(void)
{
    // Divider defaults to 2 (reg_val = div - 2)
    //cmu->SYS_DIV = SET_BITFIELD(cmu->SYS_DIV, CMU_CFG_DIV_PCLK, 0);
}

int hal_cmu_periph_set_div(uint32_t div)
{
    uint32_t lock;
    int ret = 0;

    if (div == 0 || div > ((AON_CMU_CFG_DIV_PER_MASK >> AON_CMU_CFG_DIV_PER_SHIFT) + 2)) {
        aoncmu->TOP_CLK_DISABLE = AON_CMU_EN_CLK_PLL_PER_DISABLE;
        if (div > ((AON_CMU_CFG_DIV_PER_MASK >> AON_CMU_CFG_DIV_PER_SHIFT) + 2)) {
            ret = 1;
        }
    } else {
        lock = int_lock();
        if (div == 1) {
            aoncmu->CLK_SELECT |= AON_CMU_BYPASS_DIV_PER;
        } else {
            div -= 2;
            aoncmu->CLK_SELECT = (aoncmu->CLK_SELECT & ~(AON_CMU_CFG_DIV_PER_MASK | AON_CMU_BYPASS_DIV_PER)) |
                AON_CMU_CFG_DIV_PER(div);
        }
        int_unlock(lock);
        aoncmu->TOP_CLK_ENABLE = AON_CMU_EN_CLK_PLL_PER_ENABLE;
    }

    return ret;
}

#define PERPH_SET_DIV_FUNC(f, F, r) \
int hal_cmu_ ##f## _set_div(uint32_t div) \
{ \
    uint32_t lock; \
    int ret = 0; \
    lock = int_lock(); \
    if (div < 2 || div > ((CMU_CFG_DIV_ ##F## _MASK >> CMU_CFG_DIV_ ##F## _SHIFT) + 2)) { \
        cmu->r &= ~(CMU_SEL_OSCX2_ ##F | CMU_SEL_PLL_ ##F | CMU_EN_PLL_ ##F); \
        ret = 1; \
    } else { \
        div -= 2; \
        cmu->r = (cmu->r & ~(CMU_CFG_DIV_ ##F## _MASK)) | CMU_SEL_OSCX2_ ##F | CMU_SEL_PLL_ ##F | \
            CMU_CFG_DIV_ ##F(div); \
        cmu->r |= CMU_EN_PLL_ ##F; \
    } \
    int_unlock(lock); \
    return ret; \
}

PERPH_SET_DIV_FUNC(uart0, UART0, UART_CLK);
PERPH_SET_DIV_FUNC(uart1, UART1, UART_CLK);
PERPH_SET_DIV_FUNC(spi, SPI1, SYS_DIV);
PERPH_SET_DIV_FUNC(i2c, I2C, I2C_CLK);

#define PERPH_SET_FREQ_FUNC(f, F, r) \
int hal_cmu_ ##f## _set_freq(enum HAL_CMU_PERIPH_FREQ_T freq) \
{ \
    uint32_t lock; \
    int ret = 0; \
    lock = int_lock(); \
    if (freq == HAL_CMU_PERIPH_FREQ_26M) { \
        cmu->r &= ~(CMU_SEL_OSCX2_ ##F | CMU_SEL_PLL_ ##F | CMU_EN_PLL_ ##F); \
    } else if (freq == HAL_CMU_PERIPH_FREQ_52M) { \
        cmu->r = (cmu->r & ~(CMU_SEL_PLL_ ##F | CMU_EN_PLL_ ##F)) | CMU_SEL_OSCX2_ ##F; \
    } else { \
        ret = 1; \
    } \
    int_unlock(lock); \
    return ret; \
}

PERPH_SET_FREQ_FUNC(uart0, UART0, UART_CLK);
PERPH_SET_FREQ_FUNC(uart1, UART1, UART_CLK);
PERPH_SET_FREQ_FUNC(spi, SPI1, SYS_DIV);
PERPH_SET_FREQ_FUNC(i2c, I2C, I2C_CLK);

int hal_cmu_ispi_set_freq(enum HAL_CMU_PERIPH_FREQ_T freq)
{
    uint32_t lock;
    int ret = 0;

    lock = int_lock();
    if (freq == HAL_CMU_PERIPH_FREQ_26M) {
        cmu->SYS_DIV &= ~CMU_SEL_OSCX2_SPI2;
    } else if (freq == HAL_CMU_PERIPH_FREQ_52M) {
        cmu->SYS_DIV |= CMU_SEL_OSCX2_SPI2;
    } else {
        ret = 1;
    }
    int_unlock(lock);

    return ret;
}

int hal_cmu_clock_out_enable(enum HAL_CMU_CLOCK_OUT_ID_T id)
{
    uint32_t lock;
    uint32_t sel;
    uint32_t cfg;

    enum CMU_CLK_OUT_SEL_T {
        CMU_CLK_OUT_SEL_AON     = 0,
        CMU_CLK_OUT_SEL_CODEC   = 1,
        CMU_CLK_OUT_SEL_BT      = 2,
        CMU_CLK_OUT_SEL_MCU     = 3,

        CMU_CLK_OUT_SEL_QTY
    };

    sel = CMU_CLK_OUT_SEL_QTY;
    cfg = 0;

    if (id <= HAL_CMU_CLOCK_OUT_AON_SYS) {
        sel = CMU_CLK_OUT_SEL_AON;
        cfg = id - HAL_CMU_CLOCK_OUT_AON_32K;
    } else if (HAL_CMU_CLOCK_OUT_MCU_32K <= id && id <= HAL_CMU_CLOCK_OUT_MCU_SPI1) {
        sel = CMU_CLK_OUT_SEL_MCU;
        lock = int_lock();
        cmu->PERIPH_CLK = SET_BITFIELD(cmu->PERIPH_CLK, CMU_CFG_CLK_OUT, id - HAL_CMU_CLOCK_OUT_MCU_32K);
        int_unlock(lock);
    } else if (HAL_CMU_CLOCK_OUT_CODEC_ADC_ANA <= id && id <= HAL_CMU_CLOCK_OUT_CODEC_HCLK) {
        sel = CMU_CLK_OUT_SEL_CODEC;
        hal_codec_select_clock_out(id - HAL_CMU_CLOCK_OUT_CODEC_ADC_ANA);
    } else if (HAL_CMU_CLOCK_OUT_BT_32K <= id && id <= HAL_CMU_CLOCK_OUT_BT_26M) {
        sel = CMU_CLK_OUT_SEL_BT;
        btcmu->CLK_OUT = SET_BITFIELD(btcmu->CLK_OUT, BT_CMU_CFG_CLK_OUT, id - HAL_CMU_CLOCK_OUT_BT_32K);
    }

    if (sel < CMU_CLK_OUT_SEL_QTY) {
        lock = int_lock();
        aoncmu->CLK_OUT = (aoncmu->CLK_OUT & ~(AON_CMU_SEL_CLK_OUT_MASK | AON_CMU_CFG_CLK_OUT_MASK)) |
            AON_CMU_SEL_CLK_OUT(sel) | AON_CMU_CFG_CLK_OUT(cfg) | AON_CMU_EN_CLK_OUT;
        int_unlock(lock);

        return 0;
    }

    return 1;
}

void hal_cmu_clock_out_disable(void)
{
    uint32_t lock;

    lock = int_lock();
    aoncmu->CLK_OUT &= ~AON_CMU_EN_CLK_OUT;
    int_unlock(lock);
}

int hal_cmu_i2s_mclk_enable(enum HAL_CMU_I2S_MCLK_ID_T id)
{
    uint32_t lock;

    lock = int_lock();
    aoncmu->PCM_I2S_CLK = SET_BITFIELD(aoncmu->PCM_I2S_CLK, AON_CMU_SEL_I2S_MCLK, id) | AON_CMU_EN_I2S_MCLK;
    int_unlock(lock);

    return 0;
}

void hal_cmu_i2s_mclk_disable(void)
{
    uint32_t lock;

    lock = int_lock();
    aoncmu->PCM_I2S_CLK &= ~AON_CMU_EN_I2S_MCLK;
    int_unlock(lock);
}

int hal_cmu_pwm_set_freq(enum HAL_PWM_ID_T id, uint32_t freq)
{
    uint32_t lock;
    int clk_32k;
    uint32_t div;

    if (id >= HAL_PWM_ID_QTY) {
        return 1;
    }

    if (freq == 0) {
        clk_32k = 1;
        div = 0;
    } else {
        clk_32k = 0;
        div = hal_cmu_get_crystal_freq() / freq;
        if (div < 2) {
            return 1;
        }

        div -= 2;
        if ((div & (AON_CMU_CFG_DIV_PWM0_MASK >> AON_CMU_CFG_DIV_PWM0_SHIFT)) != div) {
            return 1;
        }
    }

    lock = int_lock();
    if (id == HAL_PWM_ID_0) {
        aoncmu->PWM01_CLK = (aoncmu->PWM01_CLK & ~(AON_CMU_CFG_DIV_PWM0_MASK | AON_CMU_SEL_OSC_PWM0 | AON_CMU_EN_OSC_PWM0)) |
            AON_CMU_CFG_DIV_PWM0(div) | (clk_32k ? 0 : (AON_CMU_SEL_OSC_PWM0 | AON_CMU_EN_OSC_PWM0));
    } else if (id == HAL_PWM_ID_1) {
        aoncmu->PWM01_CLK = (aoncmu->PWM01_CLK & ~(AON_CMU_CFG_DIV_PWM1_MASK | AON_CMU_SEL_OSC_PWM1 | AON_CMU_EN_OSC_PWM1)) |
            AON_CMU_CFG_DIV_PWM1(div) | (clk_32k ? 0 : (AON_CMU_SEL_OSC_PWM1 | AON_CMU_EN_OSC_PWM1));
    } else if (id == HAL_PWM_ID_2) {
        aoncmu->PWM23_CLK = (aoncmu->PWM23_CLK & ~(AON_CMU_CFG_DIV_PWM2_MASK | AON_CMU_SEL_OSC_PWM2 | AON_CMU_EN_OSC_PWM2)) |
            AON_CMU_CFG_DIV_PWM2(div) | (clk_32k ? 0 : (AON_CMU_SEL_OSC_PWM2 | AON_CMU_EN_OSC_PWM2));
    } else {
        aoncmu->PWM23_CLK = (aoncmu->PWM23_CLK & ~(AON_CMU_CFG_DIV_PWM3_MASK | AON_CMU_SEL_OSC_PWM3 | AON_CMU_EN_OSC_PWM3)) |
            AON_CMU_CFG_DIV_PWM3(div) | (clk_32k ? 0 : (AON_CMU_SEL_OSC_PWM3 | AON_CMU_EN_OSC_PWM3));
    }
    int_unlock(lock);
    return 0;
}

void hal_cmu_jtag_enable(void)
{
    uint32_t lock;

    lock = int_lock();
    cmu->MCU_TIMER &= ~(CMU_SECURE_BOOT_JTAG | CMU_SECURE_BOOT_I2C);
    int_unlock(lock);
}

void hal_cmu_jtag_disable(void)
{
    uint32_t lock;

    lock = int_lock();
    cmu->MCU_TIMER |= (CMU_SECURE_BOOT_JTAG | CMU_SECURE_BOOT_I2C);
    int_unlock(lock);
}

void hal_cmu_jtag_clock_enable(void)
{
    aoncmu->TOP_CLK_ENABLE = AON_CMU_EN_CLK_TOP_JTAG_ENABLE;
}

void hal_cmu_jtag_clock_disable(void)
{
    aoncmu->TOP_CLK_DISABLE = AON_CMU_EN_CLK_TOP_JTAG_DISABLE;
}

void hal_cmu_rom_clock_init(void)
{
    aoncmu->CODEC_DIV = (aoncmu->CODEC_DIV & ~AON_CMU_SEL_CLK_OSCX2) |
        AON_CMU_BYPASS_LOCK_PLLBB | AON_CMU_BYPASS_LOCK_PLLAUD | AON_CMU_SEL_CLK_OSC;
    // Enable PMU fast clock
    aoncmu->CLK_OUT &= ~(AON_CMU_SEL_DCDC_PLL | AON_CMU_SEL_DCDC_OSCX2);
    aoncmu->CLK_OUT |= AON_CMU_BYPASS_DIV_DCDC;
    // Clear USBPHY reset
    aoncmu->CLK_OUT |= AON_CMU_USB_SOFT_RESETN_PHY;
    aoncmu->TOP_CLK_ENABLE = AON_CMU_EN_CLK_DCDC0_ENABLE;

    // Debug Select CMU REG F4
    cmu->MCU_TIMER = SET_BITFIELD(cmu->MCU_TIMER, CMU_DEBUG_REG_SEL, CMU_DEBUG_REG_SEL_DEBUG);
}

void hal_cmu_init_chip_feature(uint16_t feature)
{
    aoncmu->CHIP_FEATURE = feature | AON_CMU_EFUSE_LOCK;
}

void BOOT_TEXT_FLASH_LOC hal_cmu_osc_x2_enable(void)
{
    // Enable OSCX2 for MCU peripheral
    aoncmu->TOP_CLK_ENABLE = AON_CMU_EN_CLK_OSCX2_MCU_ENABLE;
}

void BOOT_TEXT_FLASH_LOC hal_cmu_osc_x4_enable(void)
{
#ifdef ANA_26M_X4_ENABLE
    aoncmu->TOP_CLK_ENABLE = AON_CMU_EN_X4_ANA_ENABLE;
#endif
#ifdef OSC_26M_X4_AUD2BB
    aoncmu->CLK_SELECT |= AON_CMU_SEL_X4_SYS;
    aoncmu->CLK_SELECT &= ~AON_CMU_SEL_X4_DIG;
#endif
}

void BOOT_TEXT_FLASH_LOC hal_cmu_module_init_state(void)
{
    aoncmu->CODEC_DIV = (aoncmu->CODEC_DIV & ~AON_CMU_SEL_CLK_OSCX2) |
        AON_CMU_BYPASS_LOCK_PLLBB | AON_CMU_BYPASS_LOCK_PLLAUD | AON_CMU_SEL_CLK_OSC;
    // Slow down PMU fast clock
    aoncmu->CLK_OUT = (aoncmu->CLK_OUT & ~(AON_CMU_BYPASS_DIV_DCDC | AON_CMU_CFG_DIV_DCDC_MASK)) | AON_CMU_CFG_DIV_DCDC(2);
    // Clear USBPHY reset
    aoncmu->CLK_OUT |= AON_CMU_USB_SOFT_RESETN_PHY;
    // Keep PMU OSC clock enabled
    aoncmu->TOP_CLK_ENABLE = AON_CMU_EN_CLK_60M_BT_ENABLE;

    // IIR/FIR/RS clock config
    aoncmu->CODEC_IIR = AON_CMU_SEL_OSC_CODECIIR | AON_CMU_SEL_OSCX2_CODECIIR | AON_CMU_SEL_OSC_4_CODECIIR1 |
        AON_CMU_SEL_OSC_2_CODECIIR1 | AON_CMU_SEL_OSC_CODECIIR1 | AON_CMU_SEL_OSCX2_CODECIIR1 |
        AON_CMU_SEL_OSC_CODECFIR | AON_CMU_SEL_OSCX2_CODECFIR | AON_CMU_SEL_OSC_CODECRS | AON_CMU_SEL_OSCX2_CODECRS |
        AON_CMU_SEL_OSC_CODECRS1 | AON_CMU_SEL_OSCX2_CODECRS1 | AON_CMU_SEL_OSC_CODECPSAP;

    // CACHE RAM retention config (no retention by default)
    cmu->BOOT_DVS = (cmu->BOOT_DVS & ~(CMU_RF_RET1N0 | CMU_RF_RET2N0 | CMU_RF_PGEN0 | CMU_RF_PGEN1)) |
        CMU_RF_RET1N1 | CMU_RF_RET2N1;
    // RAM retention config
#ifdef NO_RAM_RETENTION
    aoncmu->RAM_RET0 = AON_CMU_RAM_PGEN0(0) | AON_CMU_RAM_PGEN1(0);
    aoncmu->RAM_RET1 = AON_CMU_RAM_RET1N0(0xFFFF) | AON_CMU_RAM_RET2N0(0xFFFF);
    aoncmu->RAM_RET2 = AON_CMU_RAM_RET1N1(0xFFFF) | AON_CMU_RAM_RET2N1(0xFFFF);
    aoncmu->RAM_RET3 &= ~AON_CMU_PG_AUTO_EN_REG;
    aoncmu->FLASH_IOEN |= AON_CMU_RAM_PWR_RDYN_BYPASS;
#elif defined(RAM_RETENTION_ONE_BY_ONE)
    aoncmu->RAM_RET0 = AON_CMU_RAM_PGEN0(0) | AON_CMU_RAM_PGEN1(0);
    aoncmu->RAM_RET1 = AON_CMU_RAM_RET1N0(0) | AON_CMU_RAM_RET2N0(0);
    aoncmu->RAM_RET2 = AON_CMU_RAM_RET1N1(0xFFFF) | AON_CMU_RAM_RET2N1(0xFFFF);
    aoncmu->RAM_RET3 = (aoncmu->RAM_RET3 & ~(AON_CMU_POWER_MODES_0_MASK | AON_CMU_POWER_MODES_1_MASK | AON_CMU_POWER_MODES_2_MASK |
        AON_CMU_POWER_MODES_3_MASK | AON_CMU_POWER_MODES_4_MASK | AON_CMU_POWER_MODES_5_MASK | AON_CMU_POWER_MODES_6_MASK |
        AON_CMU_POWER_MODES_7_MASK)) | AON_CMU_POWER_MODES_0(2) | AON_CMU_POWER_MODES_1(2) | AON_CMU_POWER_MODES_2(2) |
        AON_CMU_POWER_MODES_3(2) | AON_CMU_POWER_MODES_4(2) | AON_CMU_POWER_MODES_5(2) | AON_CMU_POWER_MODES_6(2) |
        AON_CMU_POWER_MODES_7(2) | AON_CMU_PG_AUTO_EN_REG;
#else
    aoncmu->RAM_RET0 = AON_CMU_RAM_PGEN0(0) | AON_CMU_RAM_PGEN1(0);
    aoncmu->RAM_RET1 = AON_CMU_RAM_RET1N0(0) | AON_CMU_RAM_RET2N0(0);
    aoncmu->RAM_RET2 = AON_CMU_RAM_RET1N1(0xFFFF) | AON_CMU_RAM_RET2N1(0xFFFF);
    aoncmu->RAM_RET3 &= ~AON_CMU_PG_AUTO_EN_REG;
    aoncmu->FLASH_IOEN |= AON_CMU_RAM_PWR_RDYN_BYPASS;
#endif

    // DMA channel config
    cmu->ADMA_CH0_4_REQ =
        // codec
        CMU_ADMA_CH0_REQ_IDX(CMU_DMA_REQ_CODEC_RX) | CMU_ADMA_CH1_REQ_IDX(CMU_DMA_REQ_CODEC_TX) |
        // btpcm
        CMU_ADMA_CH2_REQ_IDX(CMU_DMA_REQ_PCM_RX) | CMU_ADMA_CH3_REQ_IDX(CMU_DMA_REQ_PCM_TX) |
        // i2s0
        CMU_ADMA_CH4_REQ_IDX(CMU_DMA_REQ_I2S0_RX);
    cmu->ADMA_CH5_9_REQ =
        // i2s0
        CMU_ADMA_CH5_REQ_IDX(CMU_DMA_REQ_I2S0_TX) |
        // null
        CMU_ADMA_CH6_REQ_IDX(CMU_DMA_REQ_NULL) | CMU_ADMA_CH7_REQ_IDX(CMU_DMA_REQ_NULL) |
        // null
        CMU_ADMA_CH8_REQ_IDX(CMU_DMA_REQ_NULL) | CMU_ADMA_CH9_REQ_IDX(CMU_DMA_REQ_NULL);
    cmu->ADMA_CH10_14_REQ =
        // null
        CMU_ADMA_CH10_REQ_IDX(CMU_DMA_REQ_NULL) | CMU_ADMA_CH11_REQ_IDX(CMU_DMA_REQ_CODEC_TX2) |
        // btdump
        CMU_ADMA_CH12_REQ_IDX(CMU_DMA_REQ_BTDUMP) |
        // mc
        CMU_ADMA_CH13_REQ_IDX(CMU_DMA_REQ_CODEC_MC) |
        // null
        CMU_ADMA_CH14_REQ_IDX(CMU_DMA_REQ_NULL);
    cmu->ADMA_CH15_REQ =
        // null
        CMU_ADMA_CH15_REQ_IDX(CMU_DMA_REQ_NULL);
    cmu->GDMA_CH0_4_REQ =
        // flash
        CMU_GDMA_CH0_REQ_IDX(CMU_DMA_REQ_FLS0) |
        // null
        CMU_GDMA_CH1_REQ_IDX(CMU_DMA_REQ_NULL) |
        // i2c0
        CMU_GDMA_CH2_REQ_IDX(CMU_DMA_REQ_I2C0_RX) | CMU_GDMA_CH3_REQ_IDX(CMU_DMA_REQ_I2C0_TX) |
        // spi
        CMU_GDMA_CH4_REQ_IDX(CMU_DMA_REQ_SPILCD0_RX);
    cmu->GDMA_CH5_9_REQ =
        // spi
        CMU_GDMA_CH5_REQ_IDX(CMU_DMA_REQ_SPILCD0_TX) |
        // null
        CMU_GDMA_CH6_REQ_IDX(CMU_DMA_REQ_NULL) | CMU_GDMA_CH7_REQ_IDX(CMU_DMA_REQ_NULL) |
        // uart0
        CMU_GDMA_CH8_REQ_IDX(CMU_DMA_REQ_UART0_RX) | CMU_GDMA_CH9_REQ_IDX(CMU_DMA_REQ_UART0_TX);
    cmu->GDMA_CH10_14_REQ =
        // uart1
        CMU_GDMA_CH10_REQ_IDX(CMU_DMA_REQ_UART1_RX) | CMU_GDMA_CH11_REQ_IDX(CMU_DMA_REQ_UART1_TX) |
        // i2c1
        CMU_GDMA_CH12_REQ_IDX(CMU_DMA_REQ_I2C1_RX) | CMU_GDMA_CH13_REQ_IDX(CMU_DMA_REQ_I2C1_TX) |
        // i2c2
        CMU_GDMA_CH14_REQ_IDX(CMU_DMA_REQ_I2C2_RX);
    cmu->GDMA_CH15_REQ =
        // i2c2
        CMU_GDMA_CH15_REQ_IDX(CMU_DMA_REQ_I2C2_TX);

#ifndef SIMU
    cmu->ORESET_SET = SYS_ORST_USB | SYS_ORST_SDMMC | SYS_ORST_WDT | SYS_ORST_TIMER2 |
        SYS_ORST_I2C0 | SYS_ORST_I2C1 | SYS_ORST_SPI | SYS_ORST_SLCD | SYS_ORST_SPI_PHY |
        SYS_ORST_UART0 | SYS_ORST_UART1 | SYS_ORST_UART2 | SYS_ORST_I2S0 | SYS_ORST_SPDIF0 | SYS_ORST_PCM |
        SYS_ORST_USB32K | SYS_ORST_I2S1 | SYS_ORST_I2C3;
    cmu->PRESET_SET = SYS_PRST_WDT | SYS_PRST_TIMER2 | SYS_PRST_I2C0 | SYS_PRST_I2C1 |
        SYS_PRST_SPI | SYS_PRST_SLCD | SYS_PRST_SPI_PHY |
        SYS_PRST_UART0 | SYS_PRST_UART1 | SYS_PRST_UART2 |
        SYS_PRST_PCM | SYS_PRST_I2S0 | SYS_PRST_SPDIF0 | SYS_PRST_I2S1 | SYS_PRST_BCM |
        SYS_PRST_TRNG | SYS_PRST_I2C3;
    cmu->HRESET_SET = SYS_HRST_SDMMC | SYS_HRST_USBC | SYS_HRST_CODEC | SYS_HRST_BT_TP_DUMP |
        SYS_HRST_USBH | SYS_HRST_SENSOR_HUB | SYS_HRST_BT_DUMP | SYS_HRST_CP | SYS_HRST_BCM | SYS_HRST_ICACHE1;

    cmu->OCLK_DISABLE = SYS_OCLK_USB | SYS_OCLK_SDMMC | SYS_OCLK_WDT | SYS_OCLK_TIMER2 |
        SYS_OCLK_I2C0 | SYS_OCLK_I2C1 | SYS_OCLK_SPI | SYS_OCLK_SLCD | SYS_OCLK_SPI_PHY |
        SYS_OCLK_UART0 | SYS_OCLK_UART1 | SYS_OCLK_UART2 | SYS_OCLK_I2S0 | SYS_OCLK_SPDIF0 | SYS_OCLK_PCM |
        SYS_OCLK_USB32K | SYS_OCLK_I2S1 | SYS_OCLK_I2C3;
    cmu->PCLK_DISABLE = SYS_PCLK_WDT | SYS_PCLK_TIMER2 | SYS_PCLK_I2C0 | SYS_PCLK_I2C1 |
        SYS_PCLK_SPI | SYS_PCLK_SLCD | SYS_PCLK_SPI_PHY |
        SYS_PCLK_UART0 | SYS_PCLK_UART1 | SYS_PCLK_UART2 |
        SYS_PCLK_PCM | SYS_PCLK_I2S0 | SYS_PCLK_SPDIF0 | SYS_PCLK_I2S1 | SYS_PCLK_BCM |
        SYS_PCLK_TRNG | SYS_PCLK_I2C3;
    cmu->HCLK_DISABLE = SYS_HCLK_SDMMC | SYS_HCLK_USBC | SYS_HCLK_CODEC | SYS_HCLK_BT_TP_DUMP |
        SYS_HCLK_USBH | SYS_HCLK_SENSOR_HUB | SYS_HCLK_BT_DUMP | SYS_HCLK_CP | SYS_HCLK_BCM | SYS_HCLK_ICACHE1;

    cmu->ORESET_SET = SYS_ORST_I2C2;
    cmu->PRESET_SET = SYS_PRST_I2C2;
    cmu->HRESET_SET = SYS_HRST_SPI_AHB | SYS_HRST_I2C_AHB;
    cmu->OCLK_DISABLE = SYS_OCLK_I2C2;
    cmu->PCLK_DISABLE = SYS_PCLK_I2C2;
    cmu->HCLK_DISABLE = SYS_HCLK_SPI_AHB | SYS_HCLK_I2C_AHB;

    aoncmu->TOP_CLK_DISABLE = AON_CMU_EN_CLK_PLL_CODEC_DISABLE | AON_CMU_EN_CLK_CODEC_HCLK_DISABLE | AON_CMU_EN_CLK_CODEC_DISABLE |
        AON_CMU_EN_CLK_CODEC_IIR_DISABLE | AON_CMU_EN_CLK_PLL_BT_DISABLE |
        AON_CMU_EN_CLK_OSCX2_BT_DISABLE | AON_CMU_EN_CLK_OSC_BT_DISABLE |
        AON_CMU_EN_CLK_32K_BT_DISABLE | AON_CMU_EN_CLK_PLL_PER_DISABLE;

    aoncmu->RESET_SET = AON_CMU_ARESETN_SET(AON_ARST_PWM) |
        AON_CMU_ORESETN_SET(AON_ORST_PWM0 | AON_ORST_PWM1 | AON_ORST_PWM2 | AON_ORST_PWM3) |
        AON_CMU_SOFT_RSTN_CODEC_SET | AON_CMU_SOFT_RSTN_BT_SET | AON_CMU_SOFT_RSTN_BTCPU_SET;

    aoncmu->MOD_CLK_DISABLE = AON_CMU_MANUAL_ACLK_DISABLE(AON_ACLK_PWM) |
        AON_CMU_MANUAL_OCLK_DISABLE(AON_OCLK_PWM0 | AON_OCLK_PWM1 | AON_OCLK_PWM2 | AON_OCLK_PWM3);

    aoncmu->MOD_CLK_MODE &= ~AON_CMU_MODE_ACLK(AON_ACLK_GPIO_INT | AON_ACLK_WDT | AON_ACLK_PWM |
        AON_ACLK_TIMER | AON_ACLK_PSC | AON_ACLK_IOMUX);
    cmu->PCLK_MODE &= ~(SYS_PCLK_CMU | SYS_PCLK_WDT | SYS_PCLK_TIMER0 | SYS_PCLK_TIMER1 | SYS_PCLK_TIMER2);

    //cmu->HCLK_MODE = 0;
    //cmu->PCLK_MODE = SYS_PCLK_UART0 | SYS_PCLK_UART1 | SYS_PCLK_UART2;
    //cmu->OCLK_MODE = 0;
#endif

    hal_psc_init();
}

void BOOT_TEXT_FLASH_LOC hal_cmu_ema_init(void)
{
#ifdef RAM_NAP
    aoncmu->RAM_CFG = AON_CMU_RAM_MSE | AON_CMU_RAM_MS(5);
    __DSB();
    aoncmu->FLASH_IOEN |= AON_CMU_MEM_NAP_CTRL_DR;
#endif
}

void hal_cmu_lpu_wait_26m_ready(void)
{
    while ((cmu->WAKEUP_CLK_CFG & (CMU_LPU_AUTO_SWITCH26 | CMU_LPU_STATUS_26M)) ==
            CMU_LPU_AUTO_SWITCH26);
}

int hal_cmu_lpu_busy(void)
{
    if ((cmu->WAKEUP_CLK_CFG & (CMU_LPU_AUTO_SWITCH26 | CMU_LPU_STATUS_26M)) ==
            CMU_LPU_AUTO_SWITCH26) {
        return 1;
    }
    if ((cmu->WAKEUP_CLK_CFG & (CMU_LPU_AUTO_SWITCHPLL | CMU_LPU_STATUS_PLL)) ==
            CMU_LPU_AUTO_SWITCHPLL) {
        return 1;
    }
    return 0;
}

int BOOT_TEXT_FLASH_LOC hal_cmu_lpu_init(enum HAL_CMU_LPU_CLK_CFG_T cfg)
{
    uint32_t lpu_clk;
    uint32_t timer_26m;
    uint32_t timer_pll;

    timer_26m = LPU_TIMER_US(TICKS_TO_US(HAL_CMU_26M_READY_TIMEOUT));
    timer_pll = LPU_TIMER_US(TICKS_TO_US(HAL_CMU_PLL_LOCKED_TIMEOUT));

    if (cfg >= HAL_CMU_LPU_CLK_QTY) {
        return 1;
    }
    if ((timer_26m & (CMU_TIMER_WT26_MASK >> CMU_TIMER_WT26_SHIFT)) != timer_26m) {
        return 2;
    }
    if ((timer_pll & (CMU_TIMER_WTPLL_MASK >> CMU_TIMER_WTPLL_SHIFT)) != timer_pll) {
        return 3;
    }
    if (hal_cmu_lpu_busy()) {
        return -1;
    }

    if (cfg == HAL_CMU_LPU_CLK_26M) {
        lpu_clk = CMU_LPU_AUTO_SWITCH26;
    } else if (cfg == HAL_CMU_LPU_CLK_PLL) {
        lpu_clk = CMU_LPU_AUTO_SWITCHPLL | CMU_LPU_AUTO_SWITCH26;
    } else {
        lpu_clk = 0;
    }

    if (lpu_clk & CMU_LPU_AUTO_SWITCH26) {
        // Disable RAM wakeup early
        cmu->MCU_TIMER &= ~CMU_RAM_RETN_UP_EARLY;
        // MCU/ROM/RAM auto clock gating (which depends on RAM gating signal)
        cmu->HCLK_MODE &= ~(SYS_HCLK_MCU | SYS_HCLK_ROM0 |
            SYS_HCLK_RAM0 | SYS_HCLK_RAM1 | SYS_HCLK_RAM2 | SYS_HCLK_RAMRET | SYS_HCLK_RAM3 |
            SYS_HCLK_RAM4 | SYS_HCLK_RAM5 | SYS_HCLK_RAM6);
        // AON_CMU enable auto switch 26M (AON_CMU must have selected 26M and disabled 52M/32K already)
        aoncmu->CLK_SELECT |= AON_CMU_LPU_AUTO_SWITCH26;
    } else {
        // AON_CMU disable auto switch 26M
        aoncmu->CLK_SELECT &= ~AON_CMU_LPU_AUTO_SWITCH26;
    }

    aoncmu->TIMER_WT24M |= AON_CMU_TIMER_WT24_EN;
    aoncmu->TIMER_WT24M |= AON_CMU_OSC_READY_MODE;
    aoncmu->TIMER_WT24M = SET_BITFIELD(aoncmu->TIMER_WT24M, AON_CMU_TIMER_WT24, timer_26m);
    cmu->WAKEUP_CLK_CFG = CMU_TIMER_WT26(timer_26m) | CMU_TIMER_WTPLL(0) | lpu_clk;
    if (timer_pll) {
        hal_sys_timer_delay(US_TO_TICKS(60));
        cmu->WAKEUP_CLK_CFG = CMU_TIMER_WT26(timer_26m) | CMU_TIMER_WTPLL(timer_pll) | lpu_clk;
    }
    return 0;
}

#ifdef CORE_SLEEP_POWER_DOWN

static int SRAM_TEXT_LOC hal_cmu_lpu_sleep_pd(void)
{
    uint32_t start;
    uint32_t timeout;
    uint32_t saved_hclk;
    uint32_t saved_oclk;
    uint32_t saved_top_clk;
    uint32_t saved_clk_cfg;
    uint32_t saved_periph_clk;
    uint32_t saved_sys_div;
    uint32_t saved_uart_clk;
    uint32_t saved_codec_div;
    uint32_t pll_locked;
    uint32_t saved_cpu_regs[50];
    register uint32_t sp asm("sp");
    uint32_t stack_limit;

#ifdef ROM_BUILD
    extern uint32_t __rom_StackLimit[];
    stack_limit = (uint32_t)__rom_StackLimit;
#else
    extern uint32_t __StackLimit[];
    stack_limit = (uint32_t)__StackLimit;
#endif
    if (sp < stack_limit + 20 * 4) {
        do {
            asm volatile("nop; nop; nop; nop");
        } while (1);
    }

    NVIC_PowerDownSleep(saved_cpu_regs, ARRAY_SIZE(saved_cpu_regs));

    saved_hclk = cmu->HCLK_ENABLE;
    saved_oclk = cmu->OCLK_ENABLE;
    saved_periph_clk = cmu->PERIPH_CLK;
    saved_sys_div = cmu->SYS_DIV;
    saved_uart_clk = cmu->UART_CLK;
    saved_codec_div = aoncmu->CODEC_DIV;

    saved_top_clk = aoncmu->TOP_CLK_ENABLE;
    saved_clk_cfg = cmu->SYS_CLK_ENABLE;

    // Switch VAD clock to AON and disable codec HCLK
    aoncmu->CODEC_DIV |= AON_CMU_SEL_CODEC_HCLK_AON;
    aoncmu->TOP_CLK_DISABLE = AON_CMU_EN_CLK_CODEC_HCLK_DISABLE;

    // Disable memory/flash clock
    cmu->OCLK_DISABLE = SYS_OCLK_FLASH;
    cmu->HCLK_DISABLE = SYS_HCLK_FLASH;

#ifndef ROM_BUILD
    // Reset pll div if pll is enabled
    if (saved_top_clk & AON_CMU_PU_PLLAUD_ENABLE) {
        pmu_pll_div_reset_set(HAL_CMU_PLL_AUD);
    }
    if (saved_top_clk & AON_CMU_PU_PLLBB_ENABLE) {
        pmu_pll_div_reset_set(HAL_CMU_PLL_USB);
    }
#endif

    // Switch memory/flash freq to 26M or 32K
    cmu->SYS_CLK_ENABLE = CMU_SEL_OSC_FLS_ENABLE;
    cmu->SYS_CLK_DISABLE = CMU_SEL_OSCX2_FLS_DISABLE | CMU_SEL_PLL_FLS_DISABLE;
    cmu->SYS_CLK_DISABLE = CMU_RSTN_DIV_FLS_DISABLE;
    // Switch system freq to 26M
#ifdef LOW_SYS_FREQ
    enum HAL_CMU_FREQ_T saved_sys_freq = cmu_sys_freq;
    hal_cmu_sys_set_freq(HAL_CMU_FREQ_26M);
#else
    cmu->SYS_CLK_ENABLE = CMU_SEL_OSC_SYS_ENABLE;
    cmu->SYS_CLK_DISABLE = CMU_SEL_OSCX2_SYS_DISABLE | CMU_SEL_PLL_SYS_DISABLE;
    cmu->SYS_CLK_DISABLE = CMU_RSTN_DIV_SYS_DISABLE;
#endif

    // Shutdown PLLs
    if (saved_top_clk & AON_CMU_PU_PLLAUD_ENABLE) {
        aoncmu->TOP_CLK_DISABLE = AON_CMU_EN_CLK_TOP_PLLAUD_DISABLE;
        aoncmu->TOP_CLK_DISABLE = AON_CMU_PU_PLLAUD_DISABLE;
    }
    if (saved_top_clk & AON_CMU_PU_PLLBB_ENABLE) {
        aoncmu->TOP_CLK_DISABLE = AON_CMU_EN_CLK_TOP_PLLBB_DISABLE;
        aoncmu->TOP_CLK_DISABLE = AON_CMU_PU_PLLBB_DISABLE;
    }
    if (saved_top_clk & AON_CMU_PU_PLLUSB_ENABLE) {
        aoncmu->TOP_CLK_DISABLE = AON_CMU_EN_CLK_TOP_PLLUSB_DISABLE;
        aoncmu->TOP_CLK_DISABLE = AON_CMU_PU_PLLUSB_DISABLE;
    }

    // Set power down wakeup bootmode
    aoncmu->BOOTMODE = (aoncmu->BOOTMODE | HAL_SW_BOOTMODE_POWER_DOWN_WAKEUP) & HAL_SW_BOOTMODE_MASK;
    // Set AON_CMU clock to 32K
    aoncmu->CODEC_DIV &= ~(AON_CMU_SEL_CLK_OSC | AON_CMU_SEL_CLK_OSCX2);

    hal_sleep_core_power_down();

    while ((cmu->WAKEUP_CLK_CFG & CMU_LPU_STATUS_26M) == 0);

    // Restore AON_CMU clock
    aoncmu->CODEC_DIV = saved_codec_div;
    // Clear power down wakeup bootmode
    aoncmu->BOOTMODE = (aoncmu->BOOTMODE & ~HAL_SW_BOOTMODE_POWER_DOWN_WAKEUP) & HAL_SW_BOOTMODE_MASK;

    // Disable memory/flash clock
    cmu->OCLK_DISABLE = SYS_OCLK_FLASH;
    cmu->HCLK_DISABLE = SYS_HCLK_FLASH;

    // Restore PLLs
    if (saved_top_clk & (AON_CMU_PU_PLLAUD_ENABLE | AON_CMU_PU_PLLUSB_ENABLE | AON_CMU_PU_PLLBB_ENABLE)) {
        pll_locked = 0;
        if (saved_top_clk & AON_CMU_PU_PLLAUD_ENABLE) {
            aoncmu->TOP_CLK_ENABLE = AON_CMU_PU_PLLAUD_ENABLE;
            pll_locked |= AON_CMU_LOCK_PLLAUD;
        }
        if (saved_top_clk & AON_CMU_PU_PLLBB_ENABLE) {
            aoncmu->TOP_CLK_ENABLE = AON_CMU_PU_PLLBB_ENABLE;
            pll_locked |= AON_CMU_LOCK_PLLBB;
        }
        if (saved_top_clk & AON_CMU_PU_PLLUSB_ENABLE) {
            aoncmu->TOP_CLK_ENABLE = AON_CMU_PU_PLLUSB_ENABLE;
            pll_locked |= AON_CMU_LOCK_PLLUSB;
        }
#ifndef ROM_BUILD
        hal_sys_timer_delay_us(10);
        // Clear pll div reset if pll is enabled
        if (saved_top_clk & AON_CMU_PU_PLLAUD_ENABLE) {
            pmu_pll_div_reset_clear(HAL_CMU_PLL_AUD);
        }
        if (saved_top_clk & AON_CMU_PU_PLLBB_ENABLE) {
            pmu_pll_div_reset_clear(HAL_CMU_PLL_USB);
        }
#endif
        start = hal_sys_timer_get();
        timeout = HAL_CMU_PLL_LOCKED_TIMEOUT;
        while (//(aoncmu->CODEC_DIV & pll_locked) != pll_locked &&
                (hal_sys_timer_get() - start) < timeout);
        if (saved_top_clk & AON_CMU_EN_CLK_TOP_PLLAUD_ENABLE) {
            aoncmu->TOP_CLK_ENABLE = AON_CMU_EN_CLK_TOP_PLLAUD_ENABLE;
        }
        if (saved_top_clk & AON_CMU_EN_CLK_TOP_PLLBB_ENABLE) {
            aoncmu->TOP_CLK_ENABLE = AON_CMU_EN_CLK_TOP_PLLBB_ENABLE;
        }
        if (saved_top_clk & AON_CMU_EN_CLK_TOP_PLLUSB_ENABLE) {
            aoncmu->TOP_CLK_ENABLE = AON_CMU_EN_CLK_TOP_PLLUSB_ENABLE;
        }
    }

    // Restore system freq
#ifdef LOW_SYS_FREQ
    hal_cmu_sys_set_freq(saved_sys_freq);
#endif
    cmu->SYS_CLK_ENABLE = saved_clk_cfg &
        (CMU_RSTN_DIV_FLS_ENABLE | CMU_RSTN_DIV_SYS_ENABLE);
    cmu->SYS_CLK_ENABLE = saved_clk_cfg;
    // The original system freq are at least 26M
    //cmu->SYS_CLK_DISABLE = ~saved_clk_cfg;

    cmu->PERIPH_CLK = saved_periph_clk;
    cmu->SYS_DIV = saved_sys_div;
    cmu->UART_CLK = saved_uart_clk;

    // Switch VAD clock to MCU and enable codec HCLK if it is on before entering sleep
    //aoncmu->CODEC_DIV &= ~AON_CMU_SEL_CODEC_HCLK_AON;
    aoncmu->TOP_CLK_ENABLE = saved_top_clk & AON_CMU_EN_CLK_CODEC_HCLK_ENABLE;

    NVIC_PowerDownWakeup(saved_cpu_regs, ARRAY_SIZE(saved_cpu_regs));

    // TODO:
    // 1) Restore hardware modules, e.g., timer, cache, flash, psram, dma, usb, uart, spi, i2c, sdmmc, codec
    // 2) Recover system timer in rt_suspend() and rt_resume()
    // 3) Dynamically select 32K sleep or power down sleep

    if (saved_oclk & SYS_OCLK_FLASH) {
        // Enable memory/flash clock
        cmu->HCLK_ENABLE = saved_hclk;
        cmu->OCLK_ENABLE = saved_oclk;
        // Wait until memory/flash clock ready
        hal_sys_timer_delay_us(2);
    }

    return 0;
}

#endif

#ifdef MANUAL_RAM_RETENTION
void NOINLINE BOOT_TEXT_SRAM_LOC _manual_ram_sleep(uint32_t wakeup_cfg, enum HAL_CMU_LPU_SLEEP_MODE_T mode, uint32_t saved_codec_div)
{
    const uint32_t ret1n0 = 0x1;
    const uint32_t ret2n0 = 0x1;
    uint32_t ret1n1;
    uint32_t ret2n1;
    int i;
    uint32_t timeout;

    // CAUTION:
    // 1) The whole function should be in the first half of RAM0
    // 2) Stack should have been switched to the memory in the first half of RAM0

    __DSB();

    cmu->BOOT_DVS &= ~CMU_RF_RET2N1;
    __NOP(); __NOP();
    cmu->BOOT_DVS &= ~CMU_RF_RET1N1;
    __NOP(); __NOP();

    ret1n1 = 0xFFFF;
    ret2n1 = 0xFFFF;

    aoncmu->RAM_RET1 = AON_CMU_RAM_RET1N0(ret1n0) | AON_CMU_RAM_RET2N0(ret2n0);
    aoncmu->RAM_RET2 = AON_CMU_RAM_RET1N1(ret1n1) | AON_CMU_RAM_RET2N1(ret2n1);
    aoncmu->RAM_RET3 &= ~AON_CMU_PG_AUTO_EN_REG;
    aoncmu->FLASH_IOEN |= AON_CMU_RAM_PWR_RDYN_BYPASS;

    for (i = 1; i < 16; i++) {
        ret2n1 &= ~(1 << i);
        aoncmu->RAM_RET2 = AON_CMU_RAM_RET1N1(ret1n1) | AON_CMU_RAM_RET2N1(ret2n1);
        __NOP(); __NOP();
        ret1n1 &= ~(1 << i);
        aoncmu->RAM_RET2 = AON_CMU_RAM_RET1N1(ret1n1) | AON_CMU_RAM_RET2N1(ret2n1);
        __NOP(); __NOP();
    }

    if ((wakeup_cfg & (CMU_LPU_AUTO_SWITCHPLL | CMU_LPU_AUTO_SWITCH26)) == 0) {
        // Manually switch AON_CMU clock to 32K
        aoncmu->CODEC_DIV &= ~(AON_CMU_SEL_CLK_OSC | AON_CMU_SEL_CLK_OSCX2);
        // Switch system freq to 32K
        cmu->SYS_CLK_DISABLE = CMU_SEL_OSC_SYS_DISABLE;
        __NOP(); __NOP();
    }

    __DSB();
    __WFI();

    if ((wakeup_cfg & (CMU_LPU_AUTO_SWITCHPLL | CMU_LPU_AUTO_SWITCH26)) == 0) {
        if (mode == HAL_CMU_LPU_SLEEP_MODE_CHIP) {
            timeout = HAL_CMU_26M_READY_TIMEOUT;
            hal_sys_timer_delay(timeout);
        }
        // Switch system freq to 26M
        cmu->SYS_CLK_ENABLE = CMU_SEL_OSC_SYS_ENABLE;
        // Restore AON_CMU clock
        aoncmu->CODEC_DIV = saved_codec_div;
    }

    for (i = 1; i < 16; i++) {
        ret1n1 |= (1 << i);
        aoncmu->RAM_RET2 = AON_CMU_RAM_RET1N1(ret1n1) | AON_CMU_RAM_RET2N1(ret2n1);
        __NOP(); __NOP();
        ret2n1 |= (1 << i);
        aoncmu->RAM_RET2 = AON_CMU_RAM_RET1N1(ret1n1) | AON_CMU_RAM_RET2N1(ret2n1);
        __NOP(); __NOP();
    }

    cmu->BOOT_DVS |= CMU_RF_RET1N1;
    __NOP(); __NOP();
    cmu->BOOT_DVS |= CMU_RF_RET2N1;
    __NOP(); __NOP();

    __DSB();
}

static ALIGNED(8) BOOT_BSS_LOC uint32_t ram_sleep_stack[20];

void NOINLINE BOOT_TEXT_SRAM_LOC NAKED hal_cmu_manual_ram_sleep(uint32_t wakeup_cfg, enum HAL_CMU_LPU_SLEEP_MODE_T mode, uint32_t saved_codec_div)
{
    asm volatile (
        "push {r4-r7};"
        "mov r4, sp;"
        "mrs r5, msplim;"
        "movs r6, lr;"
        "ldr r7, =%0;"
        "msr msplim, r7;"
        "ldr sp, =%1;"
        "bl _manual_ram_sleep;"
        "movs lr, r6;"
        "msr msplim, r5;"
        "mov sp, r4;"
        "pop {r4-r7};"
        "bx lr;"
        :
        : "X"((uint32_t)ram_sleep_stack), "X"((uint32_t)ram_sleep_stack + sizeof(ram_sleep_stack))
    );
}
#endif

static int SRAM_TEXT_LOC hal_cmu_lpu_sleep_normal(enum HAL_CMU_LPU_SLEEP_MODE_T mode)
{
    uint32_t start;
    uint32_t timeout;
    uint32_t saved_hclk;
    uint32_t saved_oclk;
    uint32_t saved_top_clk;
    uint32_t saved_clk_cfg;
    uint32_t saved_codec_div;
    uint32_t wakeup_cfg;
    bool pd_aud_pll;
    bool pd_bb_pll;
    bool wait_pll_locked;

#ifdef MANUAL_RAM_RETENTION
    // TODO: Ensure the whole _manual_ram_sleep() function and hal_sys_timer_delay() function are in the first half of RAM0
    ASSERT((uint32_t)_manual_ram_sleep < RAMX0_BASE + (RAMX1_BASE - RAMX0_BASE) / 2,
        "Bad ram sleep function address: %p", hal_cmu_manual_ram_sleep);
    ASSERT((uint32_t)hal_sys_timer_delay < RAMX0_BASE + (RAMX1_BASE - RAMX0_BASE) / 2,
        "Bad timer delay function address: %p", hal_sys_timer_delay);
    ASSERT((uint32_t)ram_sleep_stack+ sizeof(ram_sleep_stack) < RAM0_BASE + (RAM1_BASE - RAM0_BASE) / 2,
        "Bad timer delay function address: %p", hal_sys_timer_delay);
#endif

    pd_aud_pll = true;
    pd_bb_pll = true;

    saved_hclk = cmu->HCLK_ENABLE;
    saved_oclk = cmu->OCLK_ENABLE;
    saved_codec_div = aoncmu->CODEC_DIV;
    saved_top_clk = aoncmu->TOP_CLK_ENABLE;
    saved_clk_cfg = cmu->SYS_CLK_ENABLE;

    if (mode == HAL_CMU_LPU_SLEEP_MODE_CHIP) {
        wakeup_cfg = cmu->WAKEUP_CLK_CFG;
    } else {
        wakeup_cfg = 0;
        if (pll_user_map[HAL_CMU_PLL_AUD] & (1 << HAL_CMU_PLL_USER_AUD)) {
            pd_aud_pll = false;
        }
        if (pll_user_map[HAL_CMU_PLL_USB] & (1 << HAL_CMU_PLL_USER_AUD)) {
            pd_bb_pll = false;
        }
    }

    // Switch VAD clock to AON and disable codec HCLK
    aoncmu->CODEC_DIV |= AON_CMU_SEL_CODEC_HCLK_AON;
    if (mode == HAL_CMU_LPU_SLEEP_MODE_CHIP) {
        aoncmu->TOP_CLK_DISABLE = AON_CMU_EN_CLK_CODEC_HCLK_DISABLE;
    } else {
        // Avoid auto-gating OSC and OSCX2
        aoncmu->TOP_CLK_ENABLE = AON_CMU_EN_CLK_TOP_OSCX2_ENABLE | AON_CMU_EN_CLK_TOP_OSC_ENABLE;
    }

    // Disable memory/flash clock
    cmu->OCLK_DISABLE = SYS_OCLK_FLASH;
    cmu->HCLK_DISABLE = SYS_HCLK_FLASH;

#ifdef LOW_SYS_FREQ
    enum HAL_CMU_FREQ_T saved_sys_freq = cmu_sys_freq;
    hal_cmu_sys_set_freq(HAL_CMU_FREQ_26M);
#endif

#ifndef ROM_BUILD
    // Reset pll div if pll is enabled
    if (pd_aud_pll && (saved_top_clk & AON_CMU_PU_PLLAUD_ENABLE)) {
        pmu_pll_div_reset_set(HAL_CMU_PLL_AUD);
    }
    if (pd_bb_pll && (saved_top_clk & AON_CMU_PU_PLLBB_ENABLE)) {
        pmu_pll_div_reset_set(HAL_CMU_PLL_USB);
    }
#endif

    // Setup wakeup mask
    cmu->WAKEUP_MASK0 = NVIC->ISER[0];
    cmu->WAKEUP_MASK1 = NVIC->ISER[1];

    if (wakeup_cfg & CMU_LPU_AUTO_SWITCHPLL) {
        // Do nothing
        // Hardware will switch system freq to 32K and shutdown PLLs automatically
    } else {
        // Switch system freq to 26M
        cmu->SYS_CLK_ENABLE = CMU_SEL_OSC_SYS_ENABLE;
        cmu->SYS_CLK_DISABLE = CMU_SEL_OSCX2_SYS_DISABLE | CMU_SEL_PLL_SYS_DISABLE;
        cmu->SYS_CLK_DISABLE = CMU_RSTN_DIV_SYS_DISABLE;
        // Shutdown PLLs
        if (pd_aud_pll && (saved_top_clk & AON_CMU_PU_PLLAUD_ENABLE)) {
            aoncmu->TOP_CLK_DISABLE = AON_CMU_EN_CLK_TOP_PLLAUD_DISABLE;
            aoncmu->TOP_CLK_DISABLE = AON_CMU_PU_PLLAUD_DISABLE;
        }
        if (pd_bb_pll && (saved_top_clk & AON_CMU_PU_PLLBB_ENABLE)) {
            aoncmu->TOP_CLK_DISABLE = AON_CMU_EN_CLK_TOP_PLLBB_DISABLE;
            aoncmu->TOP_CLK_DISABLE = AON_CMU_PU_PLLBB_DISABLE;
        }
        if (saved_top_clk & AON_CMU_PU_PLLUSB_ENABLE) {
            aoncmu->TOP_CLK_DISABLE = AON_CMU_EN_CLK_TOP_PLLUSB_DISABLE;
            aoncmu->TOP_CLK_DISABLE = AON_CMU_PU_PLLUSB_DISABLE;
        }
        if (wakeup_cfg & CMU_LPU_AUTO_SWITCH26) {
            // Do nothing
            // Hardware will switch system freq to 32K automatically
        } else {
#ifndef MANUAL_RAM_RETENTION
            // Manually switch AON_CMU clock to 32K
            aoncmu->CODEC_DIV &= ~(AON_CMU_SEL_CLK_OSC | AON_CMU_SEL_CLK_OSCX2);
            // Switch system freq to 32K
            cmu->SYS_CLK_DISABLE = CMU_SEL_OSC_SYS_DISABLE;
#endif
        }
    }

    if (wakeup_cfg & CMU_LPU_AUTO_SWITCH26) {
        // Enable auto memory retention
        cmu->SLEEP = (cmu->SLEEP & ~CMU_MANUAL_RAM_RETN) |
            CMU_DEEPSLEEP_EN | CMU_DEEPSLEEP_ROMRAM_EN | CMU_DEEPSLEEP_START;
    } else {
        // Disable auto memory retention
        cmu->SLEEP = (cmu->SLEEP & ~CMU_DEEPSLEEP_ROMRAM_EN) |
            CMU_DEEPSLEEP_EN | CMU_MANUAL_RAM_RETN | CMU_DEEPSLEEP_START;
    }

    if (mode == HAL_CMU_LPU_SLEEP_MODE_CHIP) {
        SCB->SCR = SCB_SCR_SLEEPDEEP_Msk;
    } else {
        SCB->SCR = 0;
    }

#ifdef MANUAL_RAM_RETENTION
    hal_cmu_manual_ram_sleep(wakeup_cfg, mode, saved_codec_div);
#else
   __DSB();

    // Cache retention sequence (there is a timing violation in cache h/w auto switch)
    cmu->BOOT_DVS &= ~CMU_RF_RET2N1;
    __NOP(); __NOP();
    cmu->BOOT_DVS &= ~CMU_RF_RET1N1;
    __NOP(); __NOP();

    __DSB();
    __WFI();

    // Cache wakeup sequence
    __DSB();
    cmu->BOOT_DVS |= CMU_RF_RET1N1;
    __NOP(); __NOP();
    cmu->BOOT_DVS |= CMU_RF_RET2N1;
    __NOP(); __NOP();

    __DSB();
#endif

    if (wakeup_cfg & CMU_LPU_AUTO_SWITCHPLL) {
        start = hal_sys_timer_get();
        timeout = HAL_CMU_26M_READY_TIMEOUT + HAL_CMU_PLL_LOCKED_TIMEOUT + HAL_CMU_LPU_EXTRA_TIMEOUT;
        while ((cmu->WAKEUP_CLK_CFG & CMU_LPU_STATUS_PLL) == 0 &&
            (hal_sys_timer_get() - start) < timeout);
        // !!! CAUTION !!!
        // Hardware will switch system freq to PLL divider and enable PLLs automatically
#ifndef ROM_BUILD
        hal_sys_timer_delay_us(10);
        // Clear pll div reset if pll is enabled
        if (saved_top_clk & AON_CMU_PU_PLLAUD_ENABLE) {
            pmu_pll_div_reset_clear(HAL_CMU_PLL_AUD);
        }
        if (saved_top_clk & AON_CMU_PU_PLLBB_ENABLE) {
            pmu_pll_div_reset_clear(HAL_CMU_PLL_USB);
        }
#endif
    } else {
        // Wait for 26M ready
        if (wakeup_cfg & CMU_LPU_AUTO_SWITCH26) {
            start = hal_sys_timer_get();
            timeout = HAL_CMU_26M_READY_TIMEOUT + HAL_CMU_LPU_EXTRA_TIMEOUT;
            while ((cmu->WAKEUP_CLK_CFG & CMU_LPU_STATUS_26M) == 0 &&
                (hal_sys_timer_get() - start) < timeout);
            // Hardware will switch system freq to 26M automatically
        } else {
#ifndef MANUAL_RAM_RETENTION
            if (mode == HAL_CMU_LPU_SLEEP_MODE_CHIP) {
                timeout = HAL_CMU_26M_READY_TIMEOUT;
                hal_sys_timer_delay(timeout);
            }
            // Switch system freq to 26M
            cmu->SYS_CLK_ENABLE = CMU_SEL_OSC_SYS_ENABLE;
            // Restore AON_CMU clock
            aoncmu->CODEC_DIV = saved_codec_div;
#endif
        }
        // System freq is 26M now and will be restored later
        // Restore PLLs
        if (saved_top_clk & (AON_CMU_PU_PLLAUD_ENABLE | AON_CMU_PU_PLLUSB_ENABLE | AON_CMU_PU_PLLBB_ENABLE)) {
            wait_pll_locked = false;
            if (pd_aud_pll && (saved_top_clk & AON_CMU_PU_PLLAUD_ENABLE)) {
                aoncmu->TOP_CLK_ENABLE = AON_CMU_PU_PLLAUD_ENABLE;
                wait_pll_locked = true;
            }
            if (pd_bb_pll && (saved_top_clk & AON_CMU_PU_PLLBB_ENABLE)) {
                aoncmu->TOP_CLK_ENABLE = AON_CMU_PU_PLLBB_ENABLE;
                wait_pll_locked = true;
            }
            if (saved_top_clk & AON_CMU_PU_PLLUSB_ENABLE) {
                aoncmu->TOP_CLK_ENABLE = AON_CMU_PU_PLLUSB_ENABLE;
                wait_pll_locked = true;
            }
            if (wait_pll_locked) {
#ifndef ROM_BUILD
                hal_sys_timer_delay_us(10);
                // Clear pll div reset if pll is enabled
                if (pd_aud_pll && (saved_top_clk & AON_CMU_PU_PLLAUD_ENABLE)) {
                    pmu_pll_div_reset_clear(HAL_CMU_PLL_AUD);
                }
                if (pd_bb_pll && (saved_top_clk & AON_CMU_PU_PLLBB_ENABLE)) {
                    pmu_pll_div_reset_clear(HAL_CMU_PLL_USB);
                }
#endif
                start = hal_sys_timer_get();
                timeout = HAL_CMU_PLL_LOCKED_TIMEOUT;
                while ((hal_sys_timer_get() - start) < timeout);
            }
            if (pd_aud_pll && (saved_top_clk & AON_CMU_EN_CLK_TOP_PLLAUD_ENABLE)) {
                aoncmu->TOP_CLK_ENABLE = AON_CMU_EN_CLK_TOP_PLLAUD_ENABLE;
            }
            if (pd_bb_pll && (saved_top_clk & AON_CMU_EN_CLK_TOP_PLLBB_ENABLE)) {
                aoncmu->TOP_CLK_ENABLE = AON_CMU_EN_CLK_TOP_PLLBB_ENABLE;
            }
            if (saved_top_clk & AON_CMU_EN_CLK_TOP_PLLUSB_ENABLE) {
                aoncmu->TOP_CLK_ENABLE = AON_CMU_EN_CLK_TOP_PLLUSB_ENABLE;
            }
        }
    }

    // Restore system freq
#ifdef LOW_SYS_FREQ
    hal_cmu_sys_set_freq(saved_sys_freq);
#endif
    cmu->SYS_CLK_ENABLE = saved_clk_cfg &
        (CMU_RSTN_DIV_FLS_ENABLE | CMU_RSTN_DIV_SYS_ENABLE);
    cmu->SYS_CLK_ENABLE = saved_clk_cfg &
        (CMU_BYPASS_DIV_FLS_ENABLE | CMU_BYPASS_DIV_SYS_ENABLE);
    cmu->SYS_CLK_ENABLE = saved_clk_cfg;
    // The original system freq are at least 26M
    //cmu->SYS_CLK_DISABLE = ~saved_clk_cfg;

    // Switch VAD clock to MCU and enable codec HCLK if it is on before entering sleep
    aoncmu->CODEC_DIV &= ~AON_CMU_SEL_CODEC_HCLK_AON;
    if (mode == HAL_CMU_LPU_SLEEP_MODE_CHIP) {
        aoncmu->TOP_CLK_ENABLE = saved_top_clk & AON_CMU_EN_CLK_CODEC_HCLK_ENABLE;
    } else {
        // Restore the original top clock settings
        aoncmu->TOP_CLK_DISABLE = ~saved_top_clk;
    }

    if (saved_oclk & SYS_OCLK_FLASH) {
        // Enable memory/flash clock
        cmu->HCLK_ENABLE = saved_hclk;
        cmu->OCLK_ENABLE = saved_oclk;
        // Wait until memory/flash clock ready
        hal_sys_timer_delay_us(2);
    }

    return 0;
}

int SRAM_TEXT_LOC hal_cmu_lpu_sleep(enum HAL_CMU_LPU_SLEEP_MODE_T mode)
{
#ifdef CORE_SLEEP_POWER_DOWN
    if (mode == HAL_CMU_LPU_SLEEP_MODE_POWER_DOWN) {
        return hal_cmu_lpu_sleep_pd();
    }
#endif
    return hal_cmu_lpu_sleep_normal(mode);
}

void hal_cmu_bt_clock_enable(void)
{
    aoncmu->TOP_CLK_ENABLE = AON_CMU_EN_CLK_OSCX2_BT_ENABLE | AON_CMU_EN_CLK_OSC_BT_ENABLE | AON_CMU_EN_CLK_32K_BT_ENABLE;
    aocmu_reg_update_wait();
}

void hal_cmu_bt_clock_disable(void)
{
    aoncmu->TOP_CLK_DISABLE = AON_CMU_EN_CLK_OSCX2_BT_DISABLE | AON_CMU_EN_CLK_OSC_BT_DISABLE | AON_CMU_EN_CLK_32K_BT_DISABLE;
}

void hal_cmu_bt_reset_set(void)
{
    aoncmu->RESET_SET = AON_CMU_SOFT_RSTN_BT_SET | AON_CMU_SOFT_RSTN_BTCPU_SET;
}

void hal_cmu_bt_reset_clear(void)
{
    aoncmu->RESET_CLR = AON_CMU_SOFT_RSTN_BT_CLR | AON_CMU_SOFT_RSTN_BTCPU_CLR;
    aocmu_reg_update_wait();
}

void hal_cmu_bt_module_init(void)
{
    //btcmu->CLK_MODE = 0;
    cmu->PERIPH_CLK |= (CMU_PLAYTIME_STAMP_CORE0_INTR_MSK|CMU_PLAYTIME_STAMP_CORE1_INTR_MSK);
}

void hal_cmu_bt_sys_clock_force_on(void)
{
    uint32_t lock;

    lock = int_lock();
    aoncmu->CODEC_DIV |= AON_CMU_EN_BT_SYS_CLK;
    int_unlock(lock);
    aocmu_reg_update_wait();
}

void hal_cmu_bt_sys_clock_auto(void)
{
    uint32_t lock;

    lock = int_lock();
    aoncmu->CODEC_DIV &= ~AON_CMU_EN_BT_SYS_CLK;
    int_unlock(lock);
}

void hal_cmu_bt_sys_force_ram_on(void)
{
    uint32_t lock;

    lock = int_lock();
    aoncmu->FLASH_IOEN &= ~AON_CMU_FORCE_BT_RAM_SLP_N;
    int_unlock(lock);
    aocmu_reg_update_wait();
}



void hal_cmu_bt_sys_force_ram_auto(void)
{
    uint32_t lock;

    lock = int_lock();
    aoncmu->FLASH_IOEN |= AON_CMU_FORCE_BT_RAM_SLP_N;
    int_unlock(lock);
    aocmu_reg_update_wait();
}

void hal_cmu_bt_sys_set_freq(enum HAL_CMU_FREQ_T freq)
{
    ASSERT(freq <= HAL_CMU_FREQ_52M, "%s: speed %u should <= HAL_CMU_FREQ_52M", __func__, freq);
    switch (freq) {
        case HAL_CMU_FREQ_52M:
          //  ASSERT(aoncmu->TOP_CLK_ENABLE&AON_CMU_EN_X4_ANA_ENABLE, "%s: TOP_CLK_ENABLE %08x need X4_ANA_ENABLE", __func__, aoncmu->TOP_CLK_ENABLE);
            aoncmu->CLK_SELECT |= AON_CMU_BYPASS_DIV_BTSYS;
            aoncmu->TOP_CLK_ENABLE |= AON_CMU_EN_CLK_PLL_BT_ENABLE;
            break;
        default:
            aoncmu->CLK_SELECT &= ~AON_CMU_BYPASS_DIV_BTSYS;
            aoncmu->TOP_CLK_ENABLE &= ~AON_CMU_EN_CLK_PLL_BT_ENABLE;
            break;
    }
}

uint32_t hal_cmu_get_aon_chip_id(void)
{
    return aoncmu->CHIP_ID;
}

uint32_t hal_cmu_get_aon_revision_id(void)
{
    return GET_BITFIELD(aoncmu->CHIP_ID, AON_CMU_REVISION_ID);
}

void hal_cmu_cp_enable(uint32_t sp, uint32_t entry)
{
    struct CP_STARTUP_CFG_T * cp_cfg;
    uint32_t cfg_addr;

    // Use (sp - 128) as the default vector. The Address must aligned to 128-byte boundary.
    cfg_addr = (sp - (1 << 7)) & ~((1 << 7) - 1);

    cmu->CP_VTOR = (cmu->CP_VTOR & ~CMU_VTOR_CORE1_MASK) | ((cfg_addr >> 2) & CMU_VTOR_CORE1_MASK);
    cp_cfg = (struct CP_STARTUP_CFG_T *)cfg_addr;

    cp_cfg->stack = sp;
    cp_cfg->reset_hdlr = (uint32_t)system_cp_reset_handler;
    cp_entry = entry;

    hal_cmu_clock_enable(HAL_CMU_MOD_H_CP);
    hal_cmu_reset_clear(HAL_CMU_MOD_H_CP);
}

void hal_cmu_cp_disable(void)
{
    hal_cmu_reset_set(HAL_CMU_MOD_H_CP);
    hal_cmu_clock_disable(HAL_CMU_MOD_H_CP);
}

uint32_t hal_cmu_cp_get_entry_addr(void)
{
    return cp_entry;
}

uint32_t hal_cmu_get_ram_boot_addr(void)
{
    return aoncmu->WAKEUP_PC;
}

#ifdef DCDC_CLOCK_CONTROL
void hal_cmu_dcdc_clock_enable(enum HAL_CMU_DCDC_CLOCK_USER_T user)
{
    if (user >= HAL_CMU_DCDC_CLOCK_USER_QTY) {
        return;
    }

    if (user == HAL_CMU_DCDC_CLOCK_USER_GPADC) {
        pmu_big_bandgap_enable(PMU_BIG_BANDGAP_USER_GPADC, true);
    }
}

void hal_cmu_dcdc_clock_disable(enum HAL_CMU_DCDC_CLOCK_USER_T user)
{
    if (user >= HAL_CMU_DCDC_CLOCK_USER_QTY) {
        return;
    }

    if (user == HAL_CMU_DCDC_CLOCK_USER_GPADC) {
        pmu_big_bandgap_enable(PMU_BIG_BANDGAP_USER_GPADC, false);
    }
}
#endif

