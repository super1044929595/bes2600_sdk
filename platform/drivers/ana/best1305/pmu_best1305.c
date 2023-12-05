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
#include "pmu.h"
#include CHIP_SPECIFIC_HDR(reg_pmu)
#include "analog.h"
#include "cmsis.h"
#include "cmsis_nvic.h"
#include "hal_aud.h"
#include "hal_bootmode.h"
#include "hal_cache.h"
#include "hal_chipid.h"
#include "hal_cmu.h"
#include "hal_location.h"
#include "hal_sysfreq.h"
#include "hal_timer.h"
#include "hal_trace.h"
#include "patch.h"
#include "tgt_hardware.h"

#ifdef PMU_IRQ_UNIFIED
#define PMU_IRQ_HDLR_PARAM              uint16_t irq_status
#else
#define PMU_IRQ_HDLR_PARAM              void
#endif

#define ana_read(reg,val)               hal_analogif_reg_read(ANA_REG(reg),val)
#define ana_write(reg,val)              hal_analogif_reg_write(ANA_REG(reg),val)
#define rf_read(reg,val)                hal_analogif_reg_read(RF_REG(reg),val)
#define rf_write(reg,val)               hal_analogif_reg_write(RF_REG(reg),val)

// LDO soft start interval is about 1000 us
#define PMU_LDO_PU_STABLE_TIME_US       1800
#define PMU_DCDC_PU_STABLE_TIME_US      100
#define PMU_VANA_STABLE_TIME_US         10
#define PMU_VCORE_STABLE_TIME_US        10
#define PMU_BIG_BG_STABLE_TIME_US       200

#define HPPA_RAMP_UP_VOLT_MV            1700

#ifdef __PMU_VIO_DYNAMIC_CTRL_MODE__
#define IO_VOLT_ACTIVE_NORMAL           PMU_IO_2_6V
#else
#define IO_VOLT_ACTIVE_NORMAL           PMU_IO_2_6V
#endif
#ifdef DIGMIC_HIGH_VOLT
#define IO_VOLT_ACTIVE_RISE             PMU_IO_3_0V
#else
#define IO_VOLT_ACTIVE_RISE             PMU_IO_2_8V
#endif
#define IO_VOLT_SLEEP                   PMU_IO_2_6V

#define PMU_DCDC_ANA_1_8V               0x9F
#define PMU_DCDC_ANA_1_7V               0x91
#define PMU_DCDC_ANA_1_6V               0x84
#define PMU_DCDC_ANA_1_5V               0x77
#define PMU_DCDC_ANA_1_4V               0x69
#define PMU_DCDC_ANA_1_35V              0x63
#define PMU_DCDC_ANA_1_3V               0x5C
#define PMU_DCDC_ANA_1_2V               0x4F
#define PMU_DCDC_ANA_1_1V               0x41
#define PMU_DCDC_ANA_1_0V               0x34

#define PMU_DCDC_ANA_SLEEP_1_3V         PMU_DCDC_ANA_1_3V
#define PMU_DCDC_ANA_SLEEP_1_2V         PMU_DCDC_ANA_1_2V
#define PMU_DCDC_ANA_SLEEP_1_1V         PMU_DCDC_ANA_1_1V
#define PMU_DCDC_ANA_SLEEP_1_0V         PMU_DCDC_ANA_1_0V

#define PMU_DCDC_DIG_1_1V               0xD4
#define PMU_DCDC_DIG_1_05V              0xC7
#define PMU_DCDC_DIG_1_0V               0xBA
#define PMU_DCDC_DIG_0_95V              0xAC
#define PMU_DCDC_DIG_0_9V               0x9F
#define PMU_DCDC_DIG_0_85V              0x92
#define PMU_DCDC_DIG_0_8V               0x84
#define PMU_DCDC_DIG_0_75V              0x77
#define PMU_DCDC_DIG_0_7V               0x6A

#define PMU_VDIG_1_35V                  0xF
#define PMU_VDIG_1_3V                   0xE
#define PMU_VDIG_1_25V                  0xD
#define PMU_VDIG_1_2V                   0xC
#define PMU_VDIG_1_15V                  0xB
#define PMU_VDIG_1_1V                   0xA
#define PMU_VDIG_1_05V                  0x9
#define PMU_VDIG_1_0V                   0x8
#define PMU_VDIG_0_95V                  0x7
#define PMU_VDIG_0_9V                   0x6
#define PMU_VDIG_0_85V                  0x5
#define PMU_VDIG_0_8V                   0x4
#define PMU_VDIG_0_75V                  0x3
#define PMU_VDIG_0_7V                   0x2
#define PMU_VDIG_0_65V                  0x1
#define PMU_VDIG_0_6V                   0x0
#define PMU_VDIG_MAX                    PMU_VDIG_1_2V

#define PMU_IO_3_3V                     0x15
#define PMU_IO_3_2V                     0x14
#define PMU_IO_3_1V                     0x13
#define PMU_IO_3_0V                     0x12
#define PMU_IO_2_9V                     0x11
#define PMU_IO_2_8V                     0x10
#define PMU_IO_2_7V                     0xF
#define PMU_IO_2_6V                     0xE
#define PMU_IO_2_5V                     0xD
#define PMU_IO_2_4V                     0xC
#define PMU_IO_2_3V                     0xB
#define PMU_IO_2_2V                     0xA
#define PMU_IO_2_1V                     0x9
#define PMU_IO_2_0V                     0x8
#define PMU_IO_1_9V                     0x7
#define PMU_IO_1_8V                     0x6
#define PMU_IO_1_7V                     0x5

#define PMU_VMEM_2_8V                   0x10
#define PMU_VMEM_2_0V                   0x8
#define PMU_VMEM_1_9V                   0x7
#define PMU_VMEM_1_8V                   0x6

#define PMU_CODEC_2_8V                  0x16
#define PMU_CODEC_2_7V                  0xF
#define PMU_CODEC_2_6V                  0xE
#define PMU_CODEC_2_5V                  0xD
#define PMU_CODEC_2_4V                  0xC
#define PMU_CODEC_2_3V                  0xB
#define PMU_CODEC_2_2V                  0xA
#define PMU_CODEC_2_1V                  0x9
#define PMU_CODEC_2_0V                  0x8
#define PMU_CODEC_1_9V                  0x7
#define PMU_CODEC_1_8V                  0x6
#define PMU_CODEC_1_7V                  0x5
#define PMU_CODEC_1_6V                  0x4
#define PMU_CODEC_1_5V                  0x3

#define PMU_USB_3_3V                    0xC
#define PMU_USB_3_2V                    0xB
#define PMU_USB_3_1V                    0xA
#define PMU_USB_3_0V                    0x9
#define PMU_USB_2_9V                    0x8
#define PMU_USB_2_8V                    0x7
#define PMU_USB_2_7V                    0x6
#define PMU_USB_2_6V                    0x5
#define PMU_USB_2_5V                    0x4
#define PMU_USB_2_4V                    0x3

#define PMU_DCDC_HPPA_2_1V              0xC7
#define PMU_DCDC_HPPA_2_0V              0xB9
#define PMU_DCDC_HPPA_1_95V             0xB3
#define PMU_DCDC_HPPA_1_9V              0xAC
#define PMU_DCDC_HPPA_1_8V              0x9F
#define PMU_DCDC_HPPA_1_7V              0x91
#define PMU_DCDC_HPPA_1_65V             0x8B
#define PMU_DCDC_HPPA_1_6V              0x84
#define PMU_DCDC_HPPA_1_5V              0x77
#define PMU_DCDC_HPPA_1_4V              0x69

#define MAX_BUCK_VANA_BIT_VAL               (BUCK_VANA_BIT_NORMAL_MASK >> BUCK_VANA_BIT_NORMAL_SHIFT)
#define MAX_BUCK_VCORE_BIT_VAL              (BUCK_VCORE_BIT_NORMAL_MASK >> BUCK_VCORE_BIT_NORMAL_SHIFT)
#define MAX_BUCK_VHPPA_BIT_VAL              (REG_BUCK_HPPA_VBIT_NORMAL_MASK >> REG_BUCK_HPPA_VBIT_NORMAL_SHIFT)

#define INTR_MSKED_CHARGE_OUT               INTR_MSKED_CHARGE(1 << 1)
#define INTR_MSKED_CHARGE_IN                INTR_MSKED_CHARGE(1 << 0)

// RF_REG_BB
#define REG_BBPLL_FREQ_15_0_SHIFT           0
#define REG_BBPLL_FREQ_15_0_MASK            (0xFFFF << REG_BBPLL_FREQ_15_0_SHIFT)
#define REG_BBPLL_FREQ_15_0(n)              BITFIELD_VAL(REG_BBPLL_FREQ_15_0, n)

// RF_REG_BC
#define REG_BBPLL_FREQ_31_16_SHIFT          0
#define REG_BBPLL_FREQ_31_16_MASK           (0xFFFF << REG_BBPLL_FREQ_31_16_SHIFT)
#define REG_BBPLL_FREQ_31_16(n)             BITFIELD_VAL(REG_BBPLL_FREQ_31_16, n)

// RF_REG_BD
#define REG_BBPLL_FREQ_34_32_SHIFT          0
#define REG_BBPLL_FREQ_34_32_MASK           (0x7 << REG_BBPLL_FREQ_34_32_SHIFT)
#define REG_BBPLL_FREQ_34_32(n)             BITFIELD_VAL(REG_BBPLL_FREQ_34_32, n)
#define REG_BBPLL_FREQ_EN                   (1 << 3)
#define REG_BBPLL_RSTN_DR                   (1 << 4)
#define REG_BBPLL_RSTN                      (1 << 5)
#define REG_BBPLL_CLK_FBC_EDGE              (1 << 6)
#define REG_BBPLL_INT_DEC_SEL_SHIFT         7
#define REG_BBPLL_INT_DEC_SEL_MASK          (0x7 << REG_BBPLL_INT_DEC_SEL_SHIFT)
#define REG_BBPLL_INT_DEC_SEL(n)            BITFIELD_VAL(REG_BBPLL_INT_DEC_SEL, n)
#define REG_BBPLL_DITHER_BYPASS             (1 << 10)
#define REG_BBPLL_PRESCALER_DEL_SEL_SHIFT   11
#define REG_BBPLL_PRESCALER_DEL_SEL_MASK    (0xF << REG_BBPLL_PRESCALER_DEL_SEL_SHIFT)
#define REG_BBPLL_PRESCALER_DEL_SEL(n)      BITFIELD_VAL(REG_BBPLL_PRESCALER_DEL_SEL, n)
#define REG_CLK_BG_EN                       (1 << 15)

// RF_REG_F5
#define REG_BT_BBPLL_DIVN_CODEC_SHIFT       0
#define REG_BT_BBPLL_DIVN_CODEC_MASK        (0x1F << REG_BT_BBPLL_DIVN_CODEC_SHIFT)
#define REG_BT_BBPLL_DIVN_CODEC(n)          BITFIELD_VAL(REG_BT_BBPLL_DIVN_CODEC, n)
#define REG_RXCALI_REVERSE                  (1 << 5)
#define REG_SIG_LOSS_SHIFT                  6
#define REG_SIG_LOSS_MASK                   (0xF << REG_SIG_LOSS_SHIFT)
#define REG_SIG_LOSS(n)                     BITFIELD_VAL(REG_SIG_LOSS, n)
#define REG_BT_IPTAT_ISEL_SHIFT             10
#define REG_BT_IPTAT_ISEL_MASK              (0x3F << REG_BT_IPTAT_ISEL_SHIFT)
#define REG_BT_IPTAT_ISEL(n)                BITFIELD_VAL(REG_BT_IPTAT_ISEL, n)

enum RF_ANA_REG_T {
    ANA_REG_60                  = 0x60,

    RF_REG_80                   = 0x80,
    RF_REG_BB                   = 0xBB,
    RF_REG_BC                   = 0xBC,
    RF_REG_BD                   = 0xBD,
    RF_REG_F5                   = 0xF5,
    RF_REG_1F4                  = 0x1F4,
};

enum PMU_VCORE_REQ_T {
    PMU_VCORE_FLASH_WRITE_ENABLED   = (1 << 0),
    PMU_VCORE_FLASH_FREQ_HIGH       = (1 << 1),
    PMU_VCORE_RS_FREQ_HIGH          = (1 << 2),
    PMU_VCORE_SYS_FREQ_LOW          = (1 << 3),
    PMU_VCORE_SYS_FREQ_MEDIUM_LOW   = (1 << 4),
    PMU_VCORE_SYS_FREQ_MEDIUM       = (1 << 5),
    PMU_VCORE_SYS_FREQ_HIGH         = (1 << 6),
};

union BOOT_SETTINGS_T {
    struct {
        unsigned short usb_dld_dis      :1;
        unsigned short uart_dld_en      :1;
        unsigned short uart_trace_en    :1;
        unsigned short pll_dis          :1;
        unsigned short uart_baud_div2   :1;
        unsigned short sec_freq_div2    :1;
        unsigned short crystal_freq     :2;
        unsigned short timeout_div      :2;
        unsigned short uart_connected   :1;
        unsigned short uart_1p8v        :1;
        unsigned short chksum           :4;
    };
    unsigned short reg;
};

enum PMU_MODUAL_T {
    PMU_ANA,
    PMU_DIG,
    PMU_IO,
    PMU_GP,
    PMU_USB,
    PMU_CODEC,
    PMU_MEM,
    PMU_PA,

    PMU_MODULE_QTY,
};

struct PMU_MODULE_CFG_T {
    unsigned short manual_bit;
    unsigned short ldo_en;
    unsigned short lp_en_dr;
    unsigned short lp_en;
    unsigned short dsleep_mode;
    unsigned short dsleep_v;
    unsigned short dsleep_v_shift;
    unsigned short normal_v;
    unsigned short normal_v_shift;
};

#define PMU_MOD_CFG_VAL(m)              { \
    REG_PU_LDO_V##m##_DR, REG_PU_LDO_V##m##_REG, \
    LP_EN_V##m##_LDO_DR, LP_EN_V##m##_LDO_REG, \
    REG_PU_LDO_V##m##_DSLEEP, \
    LDO_V##m##_VBIT_DSLEEP_MASK, LDO_V##m##_VBIT_DSLEEP_SHIFT,\
    LDO_V##m##_VBIT_NORMAL_MASK, LDO_V##m##_VBIT_NORMAL_SHIFT }

static const struct PMU_MODULE_CFG_T pmu_module_cfg[] = {
    PMU_MOD_CFG_VAL(ANA),
    PMU_MOD_CFG_VAL(DIG),
    PMU_MOD_CFG_VAL(IO),
    PMU_MOD_CFG_VAL(GP),
    PMU_MOD_CFG_VAL(USB),
    PMU_MOD_CFG_VAL(CODEC),
    PMU_MOD_CFG_VAL(MEM),
};

#define OPT_TYPE                        const

static OPT_TYPE POSSIBLY_UNUSED bool vcodec_off =
#ifdef VCODEC_OFF
    true;
#else
    false;
#endif
static OPT_TYPE uint8_t ana_act_dcdc =
    PMU_DCDC_ANA_1_3V;

static  POSSIBLY_UNUSED uint16_t vcodec_mv = (uint16_t)(VCODEC_VOLT * 1000);
static  POSSIBLY_UNUSED uint16_t vcodec_mv_efuse11_is_0 = (uint16_t)(1.7 * 1000);
static  POSSIBLY_UNUSED uint16_t vhppa_mv = (uint16_t)(VHPPA_VOLT * 1000);
static  POSSIBLY_UNUSED uint16_t vhppa_mv_efuse11_is_0 = (uint16_t)(1.8 * 1000);

static enum PMU_POWER_MODE_T BOOT_DATA_LOC pmu_power_mode = PMU_POWER_MODE_NONE;

static enum PMU_VCORE_REQ_T BOOT_BSS_LOC pmu_vcore_req;

static uint16_t SRAM_BSS_DEF(dcdc_ramp_map);

static PMU_CHARGER_IRQ_HANDLER_T charger_irq_handler;

#ifdef PMU_IRQ_UNIFIED
static bool gpio_irq_en[2];
static HAL_GPIO_PIN_IRQ_HANDLER gpio_irq_handler[2];

static PMU_WDT_IRQ_HANDLER_T wdt_irq_handler;

static PMU_IRQ_UNIFIED_HANDLER_T pmu_irq_hdlrs[PMU_IRQ_TYPE_QTY];
#endif

static uint8_t SRAM_BSS_DEF(vio_risereq_map);
STATIC_ASSERT(sizeof(vio_risereq_map) * 8 >= PMU_VIORISE_REQ_USER_QTY, "vio_risereq_map size too small");

static uint8_t SRAM_DATA_DEF(vio_act_normal) = IO_VOLT_ACTIVE_NORMAL;
static uint8_t SRAM_DATA_DEF(vio_act_rise) = IO_VOLT_ACTIVE_RISE;
static uint8_t SRAM_DATA_DEF(vio_lp) = IO_VOLT_SLEEP;

static const uint8_t ana_lp_dcdc = PMU_DCDC_ANA_SLEEP_1_0V;

// Move all the data/bss invovled in pmu_open() to .sram_data/.sram_bss,
// so that pmu_open() can be called at the end of BootInit(),
// for data/bss is initialized after BootInit().
static  uint8_t dig_lp_ldo = PMU_VDIG_0_75V;
static  uint8_t dig_lp_dcdc = PMU_DCDC_DIG_0_75V;
static  uint8_t dig_lp_ldo_efuse11_is_0 = PMU_VDIG_0_8V;
static  uint8_t dig_lp_dcdc_efuse11_is_0 = PMU_DCDC_DIG_0_85V;

static uint8_t BOOT_DATA_LOC bbpll_codec_div = 32;

static uint32_t BOOT_BSS_LOC pmu_metal_id;

static uint16_t wdt_irq_timer;
static uint16_t wdt_reset_timer;
static uint8_t check_efuse_dccalib2_l_11;

#if defined(MCU_HIGH_PERFORMANCE_MODE)
static const uint16_t high_perf_freq_mhz =
#if defined(MTEST_ENABLED) && defined(MTEST_CLK_MHZ)
    MTEST_CLK_MHZ;
#else
    300;
#endif
static bool high_perf_on;
#endif

#ifdef PMU_FORCE_LP_MODE
static enum PMU_BIG_BANDGAP_USER_T big_bandgap_user_map;
#endif

static enum PMU_BOOT_CAUSE_T BOOT_BSS_LOC pmu_boot_reason = PMU_BOOT_CAUSE_NULL;

#if defined(PMU_INIT) || (!defined(FPGA) && !defined(PROGRAMMER))
static void pmu_hppa_dcdc_to_ldo(void);
#endif

#if defined(_AUTO_TEST_)
static bool at_skip_shutdown = false;

void pmu_at_skip_shutdown(bool enable)
{
    at_skip_shutdown = enable;
}
#endif

#ifdef RTC_ENABLE
struct PMU_RTC_CTX_T {
    bool enabled;
    bool alarm_set;
    uint32_t alarm_val;
};

static struct PMU_RTC_CTX_T BOOT_BSS_LOC rtc_ctx;

static PMU_RTC_IRQ_HANDLER_T rtc_irq_handler;

static void BOOT_TEXT_SRAM_LOC pmu_rtc_save_context(void)
{
    if (pmu_rtc_enabled()) {
        rtc_ctx.enabled = true;
        if (pmu_rtc_alarm_status_set()) {
            rtc_ctx.alarm_set = true;
            rtc_ctx.alarm_val = pmu_rtc_get_alarm();
        }
    } else {
        rtc_ctx.enabled = false;
    }
}

static void pmu_rtc_restore_context(void)
{
    uint32_t rtc_val;

    if (rtc_ctx.enabled) {
        pmu_rtc_enable();
        if (rtc_ctx.alarm_set) {
            rtc_val = pmu_rtc_get();
            if (rtc_val - rtc_ctx.alarm_val <= 1 || rtc_ctx.alarm_val - rtc_val < 5) {
                rtc_ctx.alarm_val = rtc_val + 5;
            }
            pmu_rtc_set_alarm(rtc_ctx.alarm_val);
        }
    }
}
#endif

#ifdef PMU_LDO_VCORE_CALIB
union LDO_DIG_COMP_T {
    struct LDO_DIG_COMP_FIELD_T {
        uint16_t reserved1      : 5;     //reserved1 bit[4:0]
        uint16_t dig_m_normal_v : 2;     //vcore0p8_1:  bit[6:5]: -3 ~ 3
        uint16_t dig_m_normal_f : 1;     //bit[7] : 1: negative, 0: positive
        uint16_t dig_m_lp_v     : 2;     //vcore0p8_1:  bit[9:8]: -3 ~ 3
        uint16_t dig_m_lp_f     : 1;     //bit[10]: 1: negative, 0: positive
        uint16_t reserved2      : 5;     //reserved2 bit[15:11]
    } f;
    uint16_t v;
};

static int8_t pmu_ldo_dig_m_normal_comp = 0;
static int8_t pmu_ldo_dig_m_lp_comp = 0;

void pmu_get_ldo_dig_calib_value(void)
{
    uint16_t efuse_b_val;

    pmu_get_efuse(PMU_EFUSE_PAGE_DCCALIB_L, &efuse_b_val);
    if (efuse_b_val != 0) {
        TRACE(0, "%s, Invalid efuse, use default value.", __func__);
        return;
    }

    union LDO_DIG_COMP_T lg;

    pmu_get_efuse(PMU_EFUSE_PAGE_DCCALIB2_L, &lg.v);

    if (lg.f.dig_m_normal_f) {
        pmu_ldo_dig_m_normal_comp = -(int8_t)(lg.f.dig_m_normal_v);
    } else {
        pmu_ldo_dig_m_normal_comp =  (int8_t)(lg.f.dig_m_normal_v);
    }

    if (lg.f.dig_m_lp_f) {
        pmu_ldo_dig_m_lp_comp = -(int8_t)(lg.f.dig_m_lp_v);
    } else {
        pmu_ldo_dig_m_lp_comp =  (int8_t)(lg.f.dig_m_lp_v);
    }

#ifdef FORCE_BIG_BANDGAP
    pmu_ldo_dig_m_lp_comp = pmu_ldo_dig_m_normal_comp;
#elif defined(PMU_FORCE_LP_MODE)
    pmu_ldo_dig_m_normal_comp = pmu_ldo_dig_m_lp_comp;
#endif

    TRACE(0, "pmu_ldo_dig_m_normal_comp:%d", pmu_ldo_dig_m_normal_comp);
    TRACE(0, "pmu_ldo_dig_m_lp_comp:%d", pmu_ldo_dig_m_lp_comp);
}
#endif

#ifdef PMU_DCDC_CALIB
union VOLT_COMP_T {
    struct VOLT_COMP_FIELD_T {
        uint16_t dcdc1_v: 5; //bit[4:0]: 0 ~ 31
        uint16_t dcdc1_f: 1; //bit[5]  : 1: negative, 0: positive;
        uint16_t dcdc2_v: 4; //bit[9:6]: 0 ~ 15
        uint16_t dcdc2_f: 1; //bit[10] :
        uint16_t dcdc3_v: 4; //bit[14:11]: 0 ~ 15
        uint16_t dcdc3_f: 1; //bit[15]
    } f;
    uint16_t v;
};

static int8_t pmu_dcdc_dig_comp = 0;
static int8_t pmu_dcdc_ana_comp = 0;
static int8_t pmu_dcdc_hppa_comp = 0;

static int8_t pmu_dig_lp_comp = 0;
static int8_t pmu_ana_lp_comp = 0;
static int8_t pmu_hppa_lp_comp = 0;

static POSSIBLY_UNUSED void pmu_get_dcdc_calib_value(void)
{
    union VOLT_COMP_T cv;
    uint16_t value;
    bool low_precision_calib = true;

    //efuse_5 bit7 mark high-precision calib, no need to shift
    pmu_get_efuse(PMU_EFUSE_PAGE_SW_CFG, &value);
    if (value & 0x0080) {
        low_precision_calib = false;
    }

    pmu_get_efuse(PMU_EFUSE_PAGE_RESERVED_7, &cv.v);
    if (cv.f.dcdc1_f) { //digital
        pmu_dcdc_dig_comp = -(int8_t)(cv.f.dcdc1_v);
    } else {
        pmu_dcdc_dig_comp = (int8_t)(cv.f.dcdc1_v);
    }
    if (cv.f.dcdc2_f) { //ana
        pmu_dcdc_ana_comp = -(int8_t)(cv.f.dcdc2_v);
    } else {
        pmu_dcdc_ana_comp = (int8_t)(cv.f.dcdc2_v);
    }
    if (cv.f.dcdc3_f) { //hppa
        pmu_dcdc_hppa_comp = -(int8_t)(cv.f.dcdc3_v);
    } else {
        pmu_dcdc_hppa_comp = (int8_t)(cv.f.dcdc3_v);
    }
    if (low_precision_calib) {
        pmu_dcdc_dig_comp  <<= 2;
        pmu_dcdc_ana_comp  <<= 2;
        pmu_dcdc_hppa_comp <<= 2;
    }

    pmu_get_efuse(PMU_EFUSE_PAGE_RESERVED_13, &cv.v);
    if (cv.f.dcdc1_f) { //digital
        pmu_dig_lp_comp = -(int8_t)(cv.f.dcdc1_v);
    } else {
        pmu_dig_lp_comp = (int8_t)(cv.f.dcdc1_v);
    }
    if (cv.f.dcdc2_f) { //ana
        pmu_ana_lp_comp = -(int8_t)(cv.f.dcdc2_v);
    } else {
        pmu_ana_lp_comp = (int8_t)(cv.f.dcdc2_v);
    }
    if (cv.f.dcdc3_f) { //hppa
        pmu_hppa_lp_comp = -(int8_t)(cv.f.dcdc3_v);
    } else {
        pmu_hppa_lp_comp = (int8_t)(cv.f.dcdc3_v);
    }
    if (low_precision_calib) {
        pmu_dig_lp_comp  <<= 2;
        pmu_ana_lp_comp  <<= 2;
        pmu_hppa_lp_comp <<= 2;
    }

    // 4 bits not enough.
    if (pmu_ana_lp_comp == 15) {
        TRACE(0, "corner chips");
        pmu_ana_lp_comp += 6;
    }
    // The default value of small bg voltage needs to be improved.
    pmu_dig_lp_comp += 5;
    pmu_ana_lp_comp += 6;
    pmu_hppa_lp_comp += 6;


#ifdef FORCE_BIG_BANDGAP
    pmu_dig_lp_comp  = pmu_dcdc_dig_comp;
    pmu_ana_lp_comp  = pmu_dcdc_ana_comp;
    pmu_hppa_lp_comp = pmu_dcdc_hppa_comp;
#elif defined(PMU_FORCE_LP_MODE)
    pmu_dcdc_dig_comp  = pmu_dig_lp_comp;
    pmu_dcdc_ana_comp  = pmu_ana_lp_comp;
    pmu_dcdc_hppa_comp = pmu_hppa_lp_comp;
#endif

    TRACE(0, "pmu_dcdc_dig_comp:%d", pmu_dcdc_dig_comp);
    TRACE(0, "pmu_dcdc_ana_comp:%d", pmu_dcdc_ana_comp);
    TRACE(0, "pmu_dcdc_hppa_comp:%d", pmu_dcdc_hppa_comp);
    TRACE(0, "pmu_dig_lp_comp:%d", pmu_dig_lp_comp);
    TRACE(0, "pmu_ana_lp_comp:%d", pmu_ana_lp_comp);
    TRACE(0, "pmu_hppa_lp_comp:%d", pmu_hppa_lp_comp);
}

static POSSIBLY_UNUSED unsigned short pmu_reg_val_add(unsigned short val, int delta, unsigned short max)
{
    int result = val + delta;

    if (result > max) {
        result = max;
    } else if (result < 0) {
        result = 0;
    }

    return (unsigned short)result;
}
#endif

uint32_t BOOT_TEXT_FLASH_LOC read_hw_metal_id(void)
{
    uint16_t val;
    uint32_t metal_id;

    pmu_read(PMU_REG_METAL_ID, &val);
    pmu_metal_id = GET_BITFIELD(val, REVID);

    metal_id = hal_cmu_get_aon_revision_id();

    return metal_id;
}

static void BOOT_TEXT_FLASH_LOC pmu_boot_cause_init(void)
{
    uint16_t val = 0;
    union HAL_HW_BOOTMODE_T hw_bm;

    hw_bm = hal_hw_bootmode_get();

    if (hw_bm.watchdog) {
        pmu_boot_reason = PMU_BOOT_CAUSE_DIG_WDT;
    } else if (hw_bm.global) {
        pmu_boot_reason = PMU_BOOT_CAUSE_DIG_REBOOT;
    } else {
        pmu_read(PMU_REG_BOOT_STATUS, &val);
        val = GET_BITFIELD(val, PMU_LDO_ON_SOURCE);
        if (val == 1) {
            pmu_boot_reason = PMU_BOOT_CAUSE_POWER_KEY;
        } else if (val == 2) {
            pmu_boot_reason = PMU_BOOT_CAUSE_RTC;
        } else if (val == 3) {
            pmu_boot_reason = PMU_BOOT_CAUSE_CHARGER_IN;
        } else if (val == 4) {
            pmu_boot_reason = PMU_BOOT_CAUSE_CHARGER_OUT;
        } else {
            pmu_boot_reason = PMU_BOOT_CAUSE_NULL;
        }
    }
}

enum PMU_BOOT_CAUSE_T pmu_boot_cause_get(void)
{
    return pmu_boot_reason;
}

void BOOT_TEXT_FLASH_LOC pmu_boot_init(void)
{
#if !defined(PROGRAMMER) && defined(EFUSE_READ_WORKAROUND)
    if ((hal_sw_bootmode_get() & HAL_SW_BOOTMODE_FORCE_USB_DLD) == 0 &&
            hal_iomux_uart0_connected()) {
        hal_sw_bootmode_set(HAL_SW_BOOTMODE_FORCE_USB_DLD);
        patch_open(0);
        patch_code_enable_id(0, 0x20e94, 0x22012201);
        patch_code_enable_id(1, 0x20e9c, 0xbd10b002);
        hal_cmu_reset_pulse(HAL_CMU_MOD_H_MCU);
    }
#endif

    pmu_boot_cause_init();

#ifdef RTC_ENABLE
    // RTC will be restored in pmu_open()
    pmu_rtc_save_context();
#endif

#ifdef __WATCHER_DOG_RESET__
    pmu_wdt_save_context();
#endif

#if !defined(FPGA) && !defined(PROGRAMMER)
    pmu_charger_save_context();
#endif

    // Reset PMU (to recover from a possible insane state, e.g., ESD reset)
    pmu_write(PMU_REG_METAL_ID, 0xCAFE);
    pmu_write(PMU_REG_METAL_ID, 0x5FEE);
    hal_sys_timer_delay(US_TO_TICKS(500));

#ifdef AC_OUT_POWER_ON
    uint16_t val;

    pmu_read(PMU_REG_CHARGER_CFG,&val);
    val |= REG_CHARGE_INTR_EN;
    pmu_write(PMU_REG_CHARGER_CFG, val);

    pmu_read(PMU_REG_USB_CFG_3F,&val);
    val |= AC_OUT_LDO_ON_EN;
    pmu_write(PMU_REG_USB_CFG_3F, val);
#endif

#ifdef __WATCHER_DOG_RESET__
    pmu_wdt_restore_context();
#else
    pmu_wdt_stop();
#endif

    pmu_rf_ana_init();
}

static unsigned int NOINLINE BOOT_TEXT_SRAM_LOC pmu_count_zeros(unsigned int val, unsigned int bits)
{
    int cnt = 0;
    int i;

    for (i = 0; i < bits; i++) {
        if ((val & (1 << i)) == 0) {
            cnt++;
        }
    }

    return cnt;
}

void pmu_pll_freq_reg_set(uint16_t low, uint16_t high, uint16_t high2)
{
    int ret;
    uint16_t val;

    ret = rf_write(RF_REG_BB, low);
    if (ret) {
        return;
    }

    ret = rf_write(RF_REG_BC, high);
    if (ret) {
        return;
    }

    ret = rf_read(RF_REG_BD, &val);
    if (ret) {
        return;
    }
    val = SET_BITFIELD(val, REG_BBPLL_FREQ_34_32, high2) | REG_BBPLL_FREQ_EN;
    ret = rf_write(RF_REG_BD, val);
    if (ret) {
        return;
    }

    // Delay at least for 7us
    hal_sys_timer_delay(US_TO_TICKS(100));
}

void BOOT_TEXT_FLASH_LOC bbpll_freq_pll_config(uint32_t freq)
{
    uint64_t PLL_cfg_val;
    uint16_t v[3];
    uint32_t crystal = hal_cmu_get_crystal_freq();

    PLL_cfg_val = ((uint64_t)(1 << 28) * (freq / 2) + crystal / 2) / crystal;
    v[0] = PLL_cfg_val & 0xFFFF;
    v[1] = (PLL_cfg_val >> 16) & 0xFFFF;
    v[2] = (PLL_cfg_val >> 32) & 0xFFFF;

    pmu_pll_freq_reg_set(v[0], v[1], v[2]);
}

void pmu_pll_codec_clock_enable(bool en)
{
    static bool codec_clk_en = false;

    uint16_t val;

    if (en == codec_clk_en) {
        return;
    }
    rf_read(RF_REG_1F4, &val);
    if (en) {
        val |= (1<<12);
    } else {
        val &= ~(1<<12);
    }
    rf_write(RF_REG_1F4, val);
    codec_clk_en = en;
}

void BOOT_TEXT_FLASH_LOC pmu_rf_ana_init(void)
{
    int ret;
    uint16_t val;
    union BOOT_SETTINGS_T boot;

    ret = pmu_get_efuse(PMU_EFUSE_PAGE_BOOT, &boot.reg);
    if (ret) {
        boot.reg = 0;
    } else {
        if (pmu_count_zeros(boot.reg, 12) != boot.chksum) {
            boot.reg = 0;
        }
    }
    hal_cmu_set_crystal_freq_index(boot.crystal_freq);
    // Update ISPI cfg
    ret = hal_analogif_open();
    if (ret) {
        SAFE_PROGRAM_STOP();
    }

#if !defined(FPGA) && !defined(PROGRAMMER) && !defined(MCU_HIGH_PERFORMANCE_MODE)
    if (hal_cmu_get_crystal_freq() != hal_cmu_get_default_crystal_freq()) {
        // Update bbpll freq after resetting RF and getting crystal freq
        bbpll_freq_pll_config(384000000);
    }
#endif

#if defined(PMU_FULL_INIT) || (!defined(FPGA) && !defined(PROGRAMMER))
    // Reset RF
    rf_write(RF_REG_80, 0xCAFE);
    rf_write(RF_REG_80, 0x5FEE);
    hal_sys_timer_delay(US_TO_TICKS(500));

    // Reset ANA
    ana_write(ANA_REG_60, 0xCAFE);
    ana_write(ANA_REG_60, 0x5FEE);
    hal_sys_timer_delay(US_TO_TICKS(500));
#endif

    //enhance xtal drv
    rf_write(0x9D,0x8C6C);
    rf_write(0x9c, 0x1817);

    // Enable 26M doubler (52M)
    rf_write(0xB5, 0x8000);
#ifdef ANA_26M_X4_ENABLE
    rf_write(0xBF, 0x0400);
#endif

    // Power up bbpll and audpll clk buf
    // Clk buf bits:
    // 0-rfpll 1-bt_dac 2-codec_resample&audpll_prechr 3-bbpll 4-audpll 5-usbhspll 6-lbrt 7-dig
    rf_read(0xC4, &val);
    val &= ~0xFF;
    val |= (1 << 2) | (1 << 3) | (1 << 7);
    rf_write(0xC4, val);

    // Cfg bbpll
    rf_write(0x9F, 0xC22F);
    rf_write(0xA0, 0x2788);

    // Clear reg_bt_tst_buf_sel_in/out to avoid impacting P00-P03 and P30-P33 pins
    rf_write(0xA2, 0x01C2);

    // Init pll dividers
    rf_read(RF_REG_F5, &val);
    val = SET_BITFIELD(val, REG_BT_BBPLL_DIVN_CODEC, bbpll_codec_div);
    rf_write(RF_REG_F5, val);
}

void BOOT_TEXT_SRAM_LOC pmu_pll_div_reset_set(enum HAL_CMU_PLL_T pll)
{
    uint32_t lock;
    uint16_t val;

    lock = int_lock();
    if (pll == HAL_CMU_PLL_AUD) {
        rf_read(RF_REG_F5, &val);
        val |= REG_BT_BBPLL_DIVN_CODEC_MASK;
        rf_write(RF_REG_F5, val);
    }
    int_unlock(lock);
}

void BOOT_TEXT_SRAM_LOC pmu_pll_div_reset_clear(enum HAL_CMU_PLL_T pll)
{
    uint32_t lock;
    uint16_t val;

    lock = int_lock();
    if (pll == HAL_CMU_PLL_AUD) {
        rf_read(RF_REG_F5, &val);
        val = SET_BITFIELD(val, REG_BT_BBPLL_DIVN_CODEC, bbpll_codec_div);
        rf_write(RF_REG_F5, val);
    }
    int_unlock(lock);
}

void pmu_pll_div_set(enum HAL_CMU_PLL_T pll, enum PMU_PLL_DIV_TYPE_T type, uint32_t div)
{
    uint32_t lock;
    uint16_t val;

    if (type != PMU_PLL_DIV_CODEC) {
        return;
    }

    lock = int_lock();
    if (pll == HAL_CMU_PLL_AUD) {
        if (div != bbpll_codec_div) {
            bbpll_codec_div = div;
            rf_read(RF_REG_F5, &val);
            val |= REG_BT_BBPLL_DIVN_CODEC_MASK;
            rf_write(RF_REG_F5, val);
            if (div != (REG_BT_BBPLL_DIVN_CODEC_MASK >> REG_BT_BBPLL_DIVN_CODEC_SHIFT)) {
                val = SET_BITFIELD(val, REG_BT_BBPLL_DIVN_CODEC, bbpll_codec_div);
                rf_write(RF_REG_F5, val);
            }
        }
    }
    int_unlock(lock);
}

int pmu_get_security_value(union SECURITY_VALUE_T *val)
{
    int ret;

    ret = pmu_get_efuse(PMU_EFUSE_PAGE_SECURITY, &val->reg);
    if (ret) {
        // Error
        goto _no_security;
    }

    if (!val->security_en) {
        // OK
        goto _no_security;
    }
    ret = 1;
    if (pmu_count_zeros(val->key_id, 3) != val->key_chksum) {
        // Error
        goto _no_security;
    }
    if (pmu_count_zeros(val->vendor_id, 6) != val->vendor_chksum) {
        // Error
        goto _no_security;
    }
    if ((pmu_count_zeros(val->reg, 15) & 1) != val->chksum) {
        // Error
        goto _no_security;
    }

    // OK
    return 0;

_no_security:
    val->reg = 0;

    return ret;
}

int BOOT_TEXT_SRAM_LOC pmu_get_efuse(enum PMU_EFUSE_PAGE_T page, unsigned short *efuse)
{
    int ret;
    unsigned short val;
    unsigned short tmp[2];

#ifdef EFUSE_READ_WORKAROUND
    *efuse = 0;
    return 0;
#endif

    //hal_cmu_pmu_fast_clock_enable();

    // Enable CLK_EN
    val = REG_EFUSE_CLK_EN;
    ret = pmu_write(PMU_REG_EFUSE_CTRL, val);
    if (ret) {
        goto _exit;
    }

    // Enable TURN_ON
    val |= REG_EFUSE_TURN_ON;
    ret = pmu_write(PMU_REG_EFUSE_CTRL, val);
    if (ret) {
        goto _exit;
    }

    // Write Address
#ifdef PMU_EFUSE_NO_REDUNDANCY
    val |= REG_EFUSE_ADDRESS(page / 2);
#else
    val |= REG_EFUSE_ADDRESS(page); //redundancy
#endif
    ret = pmu_write(PMU_REG_EFUSE_CTRL, val);
    if (ret) {
        goto _exit;
    }

    // Set Strobe Trigger = 1
    val |= REG_EFUSE_STROBE_TRIGGER;
    ret = pmu_write(PMU_REG_EFUSE_CTRL, val);
    if (ret) {
        goto _exit;
    }

    // set Strobe Trigger = 0
    val &= ~REG_EFUSE_STROBE_TRIGGER;
    ret = pmu_write(PMU_REG_EFUSE_CTRL, val);
    if (ret) {
        goto _exit;
    }

    // Read Efuse High 16 bits
    ret = pmu_read(PMU_REG_EFUSE_DATA_LOW, &tmp[0]);
    if (ret) {
        goto _exit;
    }

    // Read Efuse Low 16 bits
    ret = pmu_read(PMU_REG_EFUSE_DATA_HIGH, &tmp[1]);
    if (ret) {
        goto _exit;
    }
#ifdef PMU_EFUSE_NO_REDUNDANCY
    *efuse = tmp[page % 2];
#else
    *efuse = (tmp[0] | tmp[1]); //redundancy
#endif

    _exit:
    // Disable TURN_ON
    val &= ~(REG_EFUSE_TURN_ON | REG_EFUSE_ADDRESS_MASK);
    ret = pmu_write(PMU_REG_EFUSE_CTRL, val);

    // Disable CLK_EN
    val &= ~REG_EFUSE_CLK_EN;
    ret = pmu_write(PMU_REG_EFUSE_CTRL, val);

    //hal_cmu_pmu_fast_clock_disable();

    return ret;
}

void pmu_charger_out_poweron_enable(void)
{
    uint16_t val;
    uint32_t lock;

    lock = int_lock();
    pmu_read(PMU_REG_CHARGER_CFG,&val);
    val |= REG_CHARGE_INTR_EN;
    pmu_write(PMU_REG_CHARGER_CFG, val);

    pmu_read(PMU_REG_USB_CFG_3F,&val);
    val |= AC_OUT_LDO_ON_EN;
    pmu_write(PMU_REG_USB_CFG_3F, val);
    int_unlock(lock);
}

void pmu_charger_out_poweron_disable(void)
{
    uint16_t val;
    uint32_t lock;

    lock = int_lock();
    pmu_read(PMU_REG_CHARGER_CFG,&val);
    val &= ~REG_CHARGE_INTR_EN;
    pmu_write(PMU_REG_CHARGER_CFG, val);

    pmu_read(PMU_REG_USB_CFG_3F,&val);
    val &= ~AC_OUT_LDO_ON_EN;
    pmu_write(PMU_REG_USB_CFG_3F, val);
    int_unlock(lock);
}

static void pmu_charger_out_poweron_init(void)
{
#if defined(CHG_OUT_PWRON)
    pmu_charger_out_poweron_enable();
#else
    pmu_charger_out_poweron_disable();
#endif
}

static void pmu_sys_ctrl(bool shutdown)
{
    uint16_t val;
    uint32_t lock = int_lock();

    PMU_INFO_TRACE_IMM(0, "Start pmu %s", shutdown ? "shutdown" : "reboot");

#if defined(PMU_INIT) || (!defined(FPGA) && !defined(PROGRAMMER))
#if defined(MCU_HIGH_PERFORMANCE_MODE)
    // Default vcore might not be high enough to support high performance mode
    pmu_high_performance_mode_enable(false);
    hal_cmu_sys_set_freq(HAL_CMU_FREQ_26M);
#endif
    #ifndef HPPA_LDO_ON
    pmu_hppa_dcdc_to_ldo();
#endif
    pmu_mode_change(PMU_POWER_MODE_LDO);
    hal_sys_timer_delay(MS_TO_TICKS(1));
#endif

#ifdef RTC_ENABLE
    pmu_rtc_save_context();
#endif

    // Reset PMU
    pmu_write(PMU_REG_METAL_ID, 0xCAFE);
    pmu_write(PMU_REG_METAL_ID, 0x5FEE);
    hal_sys_timer_delay(4);

#ifdef RTC_ENABLE
    pmu_rtc_restore_context();
#endif

    pmu_charger_out_poweron_init();

#ifdef AC_OUT_POWER_ON
    pmu_read(PMU_REG_CHARGER_CFG,&val);
    val |= REG_CHARGE_INTR_EN;
    pmu_write(PMU_REG_CHARGER_CFG, val);

    pmu_read(PMU_REG_USB_CFG_3F,&val);
    val |= AC_OUT_LDO_ON_EN;
    pmu_write(PMU_REG_USB_CFG_3F, val);
#endif

    if (shutdown) {
#if defined(_AUTO_TEST_)
        if (at_skip_shutdown) {
            hal_cmu_sys_reboot();
            return;
        }
#endif

#if defined(PMU_INIT) || (!defined(FPGA) && !defined(PROGRAMMER))
        pmu_wdt_config(3*1000,3*1000);
        pmu_wdt_start();
        pmu_charger_shutdown_config();
#endif

        // Power off
        pmu_read(PMU_REG_POWER_OFF,&val);
        val |= SOFT_POWER_OFF;
        for (int i = 0; i < 100; i++) {
            pmu_write(PMU_REG_POWER_OFF,val);
            hal_sys_timer_delay(MS_TO_TICKS(5));
        }

        hal_sys_timer_delay(MS_TO_TICKS(50));

        //can't reach here
        PMU_INFO_TRACE_IMM(0, "\nError: pmu shutdown failed!\n");
        hal_sys_timer_delay(MS_TO_TICKS(5));
    } else {
#if defined(PMU_FULL_INIT) || (!defined(FPGA) && !defined(PROGRAMMER))
        // CAUTION:
        // 1) Never reset RF because system or flash might be using X2/X4, which are off by default
        // 2) Never reset RF/ANA because system or flash might be using PLL, and the reset might cause clock glitch
        // TODO:
        // Restore BBPLL settings in RF
#endif
    }

    hal_cmu_sys_reboot();

    int_unlock(lock);
}

void pmu_shutdown(void)
{
    pmu_sys_ctrl(true);
}

void pmu_reboot(void)
{
    pmu_sys_ctrl(false);
}

static inline uint16_t pmu_get_module_addr(enum PMU_MODUAL_T module)
{
    if (module == PMU_MEM) {
        return PMU_REG_MEM_CFG;
    } else if (module == PMU_CODEC) {
        return PMU_REG_CODEC_CFG;
    } else if (module == PMU_PA) {
        return PMU_REG_PA_CFG;
    } else {
        return module + PMU_REG_MODULE_START;
    }
}

void pmu_module_config(enum PMU_MODUAL_T module,unsigned short is_manual,unsigned short ldo_on,unsigned short lp_mode,unsigned short dpmode)
{
    unsigned short val;
    unsigned short module_address;
    const struct PMU_MODULE_CFG_T *module_cfg_p = &pmu_module_cfg[module];

    module_address = pmu_get_module_addr(module);

    pmu_read(module_address, &val);
    if (is_manual) {
        val |= module_cfg_p->manual_bit;
    } else {
        val &= ~module_cfg_p->manual_bit;
    }
    if (ldo_on) {
        val |= module_cfg_p->ldo_en;
    } else {
        val &= ~module_cfg_p->ldo_en;
    }

#ifdef PMU_FORCE_LP_MODE
    if (pmu_power_mode != PMU_POWER_MODE_NONE) {
        val |= module_cfg_p->lp_en_dr;
        if (ldo_on) {
            val |= module_cfg_p->lp_en;
        } else {
            val &= ~module_cfg_p->lp_en;
        }
    }
#else
    if (lp_mode) {
        val &= ~module_cfg_p->lp_en_dr;
    } else {
        val = (val & ~module_cfg_p->lp_en) | module_cfg_p->lp_en_dr;
    }
#endif

    if (dpmode) {
        val |= module_cfg_p->dsleep_mode;
    } else {
        val &= ~module_cfg_p->dsleep_mode;
    }
    pmu_write(module_address, val);
}

#ifdef PMU_FORCE_LP_MODE
void pmu_module_force_lp_config(void)
{
    uint16_t val;
    uint16_t module_address;
    const struct PMU_MODULE_CFG_T *module_cfg_p;
    enum PMU_MODUAL_T module;
    bool ldo_on;

    for (module = 0; module < PMU_MODULE_QTY; module++) {
        module_cfg_p = &pmu_module_cfg[module];
        module_address = pmu_get_module_addr(module);

        pmu_read(module_address, &val);
        ldo_on = !!(val & module_cfg_p->ldo_en);
        val |= module_cfg_p->lp_en_dr;
        if (ldo_on) {
            val |= module_cfg_p->lp_en;
        } else {
            val &= ~module_cfg_p->lp_en;
        }
        pmu_write(module_address, val);
    }
}
#endif

void pmu_module_set_volt(unsigned char module, unsigned short sleep_v,unsigned short normal_v)
{
    unsigned short val;
    unsigned short module_address;
    const struct PMU_MODULE_CFG_T *module_cfg_p = &pmu_module_cfg[module];

    module_address = pmu_get_module_addr(module);

#ifdef PMU_LDO_VCORE_CALIB
    if (module == PMU_DIG) {
        normal_v = pmu_reg_val_add(normal_v, pmu_ldo_dig_m_normal_comp, MAX_LDO_VCORE_VBIT);
        sleep_v = pmu_reg_val_add(sleep_v, pmu_ldo_dig_m_lp_comp, MAX_LDO_VCORE_VBIT);
    }
#endif

    pmu_read(module_address, &val);
    val &= ~module_cfg_p->normal_v;
    val |= (normal_v << module_cfg_p->normal_v_shift) & module_cfg_p->normal_v;
    val &= ~module_cfg_p->dsleep_v;
    val |= (sleep_v << module_cfg_p->dsleep_v_shift) & module_cfg_p->dsleep_v;
    pmu_write(module_address, val);
}

int pmu_module_get_volt(unsigned char module, unsigned short *sleep_vp,unsigned short *normal_vp)
{
    unsigned short val;
    unsigned short module_address;
    const struct PMU_MODULE_CFG_T *module_cfg_p = &pmu_module_cfg[module];

    module_address = pmu_get_module_addr(module);

    pmu_read(module_address, &val);
    if (normal_vp) {
        *normal_vp = (val & module_cfg_p->normal_v) >> module_cfg_p->normal_v_shift;
    }
    if (sleep_vp) {
        *sleep_vp = (val & module_cfg_p->dsleep_v) >> module_cfg_p->dsleep_v_shift;
    }

#ifdef PMU_LDO_VCORE_CALIB
    if (module == PMU_DIG) {
        if (normal_vp) {
            *normal_vp = pmu_reg_val_add(*normal_vp, -pmu_ldo_dig_m_normal_comp, MAX_LDO_VCORE_VBIT);
        }
        if (sleep_vp) {
            *sleep_vp = pmu_reg_val_add(*sleep_vp, -pmu_ldo_dig_m_lp_comp, MAX_LDO_VCORE_VBIT);
        }
    }
#endif

    return 0;
}

static void pmu_module_ramp_volt(unsigned char module, unsigned short sleep_v, unsigned short normal_v)
{
    uint16_t old_normal_v;
    uint16_t old_sleep_v;

    pmu_module_get_volt(module, &old_sleep_v, &old_normal_v);

    if (old_normal_v < normal_v) {
        while (old_normal_v++ < normal_v) {
            pmu_module_set_volt(module, sleep_v, old_normal_v);
        }
    } else if (old_normal_v != normal_v || old_sleep_v != sleep_v) {
        pmu_module_set_volt(module, sleep_v, normal_v);
    }
}

static void pmu_dcdc_ana_get_volt(unsigned short *normal_vp, unsigned short *dsleep_vp)
{
    unsigned short val;

    pmu_read(PMU_REG_DCDC_ANA_VOLT, &val);
    if (normal_vp) {
        *normal_vp = GET_BITFIELD(val, BUCK_VANA_BIT_NORMAL);
    }
    if (dsleep_vp) {
        *dsleep_vp = GET_BITFIELD(val, BUCK_VANA_BIT_DSLEEP);
    }

#ifdef PMU_DCDC_CALIB
    if (normal_vp) {
        *normal_vp = pmu_reg_val_add(*normal_vp, -pmu_dcdc_ana_comp, MAX_BUCK_VANA_BIT_VAL);
    }
    if (dsleep_vp) {
        *dsleep_vp = pmu_reg_val_add(*dsleep_vp, -pmu_ana_lp_comp, MAX_BUCK_VANA_BIT_VAL);
    }
#endif
}

static void pmu_dcdc_ana_set_volt(unsigned short normal_v,unsigned short dsleep_v)
{
    unsigned short val;

#ifdef PMU_DCDC_CALIB
    normal_v = pmu_reg_val_add(normal_v, pmu_dcdc_ana_comp, MAX_BUCK_VANA_BIT_VAL);
    dsleep_v = pmu_reg_val_add(dsleep_v, pmu_ana_lp_comp, MAX_BUCK_VANA_BIT_VAL);
#endif

    pmu_read(PMU_REG_DCDC_ANA_VOLT, &val);
    val &= ~BUCK_VANA_BIT_DSLEEP_MASK;
    val &= ~BUCK_VANA_BIT_NORMAL_MASK;
    val |= BUCK_VANA_BIT_DSLEEP(dsleep_v);
    val |= BUCK_VANA_BIT_NORMAL(normal_v);
    pmu_write(PMU_REG_DCDC_ANA_VOLT, val);

    if (normal_v > dsleep_v) {
        dcdc_ramp_map |= REG_BUCK_VANA_RAMP_EN;
    } else {
        dcdc_ramp_map &= ~REG_BUCK_VANA_RAMP_EN;
    }
}

static void pmu_ana_set_volt(enum PMU_POWER_MODE_T mode)
{
    uint16_t old_act_dcdc;
    uint16_t old_lp_dcdc;
    uint16_t new_act_dcdc;

    new_act_dcdc = ana_act_dcdc;

    if (mode == PMU_POWER_MODE_ANA_DCDC || mode == PMU_POWER_MODE_DIG_DCDC) {
        pmu_dcdc_ana_get_volt(&old_act_dcdc, &old_lp_dcdc);
        if (old_act_dcdc < new_act_dcdc) {
            while (old_act_dcdc++ < new_act_dcdc) {
                pmu_dcdc_ana_set_volt(old_act_dcdc, ana_lp_dcdc);
            }
            hal_sys_timer_delay_us(PMU_VANA_STABLE_TIME_US);
        } else if (old_act_dcdc != new_act_dcdc || old_lp_dcdc != ana_lp_dcdc) {
            pmu_dcdc_ana_set_volt(new_act_dcdc, ana_lp_dcdc);
        }
    } else {
        pmu_dcdc_ana_set_volt(new_act_dcdc, ana_lp_dcdc);
    }
}

static void pmu_dcdc_dig_get_volt(unsigned short *normal_vp, unsigned short *dsleep_vp)
{
    unsigned short val;

    pmu_read(PMU_REG_DCDC_DIG_VOLT, &val);
    if (normal_vp) {
        *normal_vp = GET_BITFIELD(val, BUCK_VCORE_BIT_NORMAL);
    }
    if (dsleep_vp) {
        *dsleep_vp = GET_BITFIELD(val, BUCK_VCORE_BIT_DSLEEP);
    }

#ifdef PMU_DCDC_CALIB
    if (normal_vp) {
        *normal_vp = pmu_reg_val_add(*normal_vp, -pmu_dcdc_dig_comp, MAX_BUCK_VCORE_BIT_VAL);
    }
    if (dsleep_vp) {
        *dsleep_vp = pmu_reg_val_add(*dsleep_vp, -pmu_dig_lp_comp, MAX_BUCK_VCORE_BIT_VAL);
    }
#endif
}

static void pmu_dcdc_dig_set_volt(unsigned short normal_v,unsigned short dsleep_v)
{
    unsigned short val;

#ifdef PMU_DCDC_CALIB
    normal_v = pmu_reg_val_add(normal_v, pmu_dcdc_dig_comp, MAX_BUCK_VCORE_BIT_VAL);
    dsleep_v = pmu_reg_val_add(dsleep_v, pmu_dig_lp_comp, MAX_BUCK_VCORE_BIT_VAL);
#endif

    pmu_read(PMU_REG_DCDC_DIG_VOLT, &val);
    val &= ~BUCK_VCORE_BIT_NORMAL_MASK;
    val &= ~BUCK_VCORE_BIT_DSLEEP_MASK;
    val |= BUCK_VCORE_BIT_NORMAL(normal_v);
    val |= BUCK_VCORE_BIT_DSLEEP(dsleep_v);
    pmu_write(PMU_REG_DCDC_DIG_VOLT, val);

    if (normal_v > dsleep_v) {
        dcdc_ramp_map |= REG_BUCK_VCORE_RAMP_EN;
    } else {
        dcdc_ramp_map &= ~REG_BUCK_VCORE_RAMP_EN;
    }
}

static void BOOT_TEXT_SRAM_LOC pmu_dig_get_target_volt(uint16_t *ldo, uint16_t *dcdc)
{
    uint16_t ldo_volt;
    uint16_t dcdc_volt;

    if (0) {
#if defined(MCU_HIGH_PERFORMANCE_MODE)
    } else if (pmu_vcore_req & (PMU_VCORE_SYS_FREQ_HIGH)) {
        if (high_perf_freq_mhz <= 260) {
            ldo_volt = PMU_VDIG_1_05V;
            dcdc_volt = PMU_DCDC_DIG_1_05V;
        } else if (high_perf_freq_mhz <= 300) {
            ldo_volt = PMU_VDIG_1_1V;
            dcdc_volt = PMU_DCDC_DIG_1_1V;
        } else {
            ldo_volt = PMU_VDIG_1_2V;
            dcdc_volt = PMU_DCDC_DIG_1_1V;
        }
#endif
    } else if (pmu_vcore_req & (PMU_VCORE_RS_FREQ_HIGH | PMU_VCORE_SYS_FREQ_MEDIUM)) {
        ldo_volt = PMU_VDIG_0_9V;
        dcdc_volt = PMU_DCDC_DIG_0_9V;
    } else if (pmu_vcore_req & (PMU_VCORE_FLASH_FREQ_HIGH | PMU_VCORE_FLASH_WRITE_ENABLED | PMU_VCORE_SYS_FREQ_MEDIUM_LOW)) {
        ldo_volt = PMU_VDIG_0_85V;
        dcdc_volt = PMU_DCDC_DIG_0_85V;
    } else if(pmu_vcore_req & PMU_VCORE_SYS_FREQ_LOW){
        ldo_volt = PMU_VDIG_0_8V;//new  11bit=0
        dcdc_volt = PMU_DCDC_DIG_0_85V;//new  11bit=0  
    } else {
        // Common cases
        ldo_volt = PMU_VDIG_0_8V;//new  11bit=0
        dcdc_volt = PMU_DCDC_DIG_0_85V;//new  11bit=0       
        if(check_efuse_dccalib2_l_11)
        {
            ldo_volt = PMU_VDIG_0_75V;//new  11bit=1
            dcdc_volt = PMU_DCDC_DIG_0_75V;//new  11bit=1
        }
    }

#if (defined(PROGRAMMER) || defined(__BES_OTA_MODE__)) && !defined(PMU_FULL_INIT)
    // Try to keep the same vcore voltage as ROM (hardware default)
    if (ldo_volt < PMU_VDIG_0_9V) {
        ldo_volt = PMU_VDIG_0_9V;
    }
    if (dcdc_volt < PMU_DCDC_DIG_0_9V) {
        dcdc_volt = PMU_DCDC_DIG_0_9V;
    }
#endif

#if defined(MTEST_ENABLED) && defined(MTEST_VOLT)
#ifdef DIG_DCDC_MODE
    dcdc_volt = MTEST_VOLT;
#else
    ldo_volt  = MTEST_VOLT;
#endif
#endif

    if (ldo) {
        *ldo = ldo_volt;
    }
    if (dcdc) {
        *dcdc = dcdc_volt;
    }
}

static void pmu_dig_set_volt(enum PMU_POWER_MODE_T mode)
{
    uint32_t lock;
    uint16_t dcdc_volt, old_act_dcdc, old_lp_dcdc;
    uint16_t ldo_volt, old_act_ldo, old_lp_ldo;
    bool volt_inc = false;

    lock = int_lock();

    if (mode == PMU_POWER_MODE_NONE) {
        mode = pmu_power_mode;
    }

    pmu_dig_get_target_volt(&ldo_volt, &dcdc_volt);

    pmu_module_get_volt(PMU_DIG, &old_lp_ldo, &old_act_ldo);
    pmu_dcdc_dig_get_volt(&old_act_dcdc, &old_lp_dcdc);

    if (mode == PMU_POWER_MODE_DIG_DCDC) {
        if (old_act_dcdc < dcdc_volt) {
            volt_inc = true;
            while (old_act_dcdc++ < dcdc_volt) {
                pmu_dcdc_dig_set_volt(old_act_dcdc, dig_lp_dcdc);
            }
        } else if (old_act_dcdc != dcdc_volt || old_lp_dcdc != dig_lp_dcdc) {
            pmu_dcdc_dig_set_volt(dcdc_volt, dig_lp_dcdc);
        }
        // Update the voltage of the other mode
        pmu_module_set_volt(PMU_DIG, dig_lp_ldo, ldo_volt);
    } else {
        if (old_act_ldo < ldo_volt) {
            volt_inc = true;
        }
        pmu_module_ramp_volt(PMU_DIG, dig_lp_ldo, ldo_volt);
        // Update the voltage of the other mode
        pmu_dcdc_dig_set_volt(dcdc_volt, dig_lp_dcdc);
    }

    if (volt_inc) {
        hal_sys_timer_delay_us(PMU_VCORE_STABLE_TIME_US);
    }

    int_unlock(lock);
}

static void pmu_ldo_mode_en(void)
{
    unsigned short val;

    // Enable vana ldo
    pmu_module_config(PMU_ANA,PMU_AUTO_MODE,PMU_LDO_ON,PMU_LP_MODE_ON,PMU_DSLEEP_MODE_ON);
    // Enable vcore ldo
    pmu_module_config(PMU_DIG,PMU_AUTO_MODE,PMU_LDO_ON,PMU_LP_MODE_ON,PMU_DSLEEP_MODE_ON);
    hal_sys_timer_delay_us(PMU_LDO_PU_STABLE_TIME_US);

    // Disable vana dcdc and vcore dcdc
    pmu_read(PMU_REG_DCDC_EN, &val);
    val = val & TEST_MODE_MASK;
    pmu_write(PMU_REG_DCDC_EN, val);
}

static void pmu_dcdc_ana_mode_en(void)
{
    unsigned short val;

    if (pmu_power_mode == PMU_POWER_MODE_DIG_DCDC) {
        // Enable vcore ldo
        pmu_module_config(PMU_DIG,PMU_AUTO_MODE,PMU_LDO_ON,PMU_LP_MODE_ON,PMU_DSLEEP_MODE_ON);
        hal_sys_timer_delay_us(PMU_LDO_PU_STABLE_TIME_US);

        // Disable vcore dcdc
        pmu_read(PMU_REG_DCDC_EN, &val);
        val &= ~(REG_BUCK_CC_MODE | PU_DCDC_DIG_DR | PU_DCDC_DIG_REG);
        pmu_write(PMU_REG_DCDC_EN, val);
    } else {
        // Enable vana dcdc
        pmu_read(PMU_REG_DCDC_EN, &val);
        val |= DCDC_ANA_LP_EN_DSLEEP | PU_DCDC_ANA_DSLEEP;
        val |= PU_DCDC_ANA_DR | PU_DCDC_ANA_REG;
        val &= ~REG_BUCK_CC_MODE;
        pmu_write(PMU_REG_DCDC_EN, val);
        hal_sys_timer_delay_us(PMU_DCDC_PU_STABLE_TIME_US);

        // Disable vana ldo
        pmu_module_config(PMU_ANA,PMU_MANUAL_MODE,PMU_LDO_OFF,PMU_LP_MODE_ON,PMU_DSLEEP_MODE_OFF);
    }
}

static void pmu_dcdc_dual_mode_en(void)
{
    unsigned short val;

    // Enable vana dcdc and vcore dcdc
    pmu_read(PMU_REG_DCDC_EN, &val);
    val |= REG_BUCK_CC_MODE;
    val |= PU_DCDC_ANA_DR | PU_DCDC_ANA_REG;
    val |= PU_DCDC_DIG_DR | PU_DCDC_DIG_REG;
    val |= DCDC_ANA_LP_EN_DSLEEP | PU_DCDC_ANA_DSLEEP;
    val |= DCDC_DIG_LP_EN_DSLEEP | PU_DCDC_DIG_DSLEEP;
    pmu_write(PMU_REG_DCDC_EN, val);
    hal_sys_timer_delay_us(PMU_DCDC_PU_STABLE_TIME_US);

    // Disable vana ldo
    pmu_module_config(PMU_ANA,PMU_MANUAL_MODE,PMU_LDO_OFF,PMU_LP_MODE_ON,PMU_DSLEEP_MODE_OFF);
    // Disable vcore ldo
    pmu_module_config(PMU_DIG,PMU_MANUAL_MODE,PMU_LDO_OFF,PMU_LP_MODE_ON,PMU_DSLEEP_MODE_OFF);
}

void pmu_mode_change(enum PMU_POWER_MODE_T mode)
{
    uint32_t lock;

    if (pmu_power_mode == mode || mode == PMU_POWER_MODE_NONE) {
        return;
    }

    lock = int_lock();

    pmu_ana_set_volt(pmu_power_mode);
    pmu_dig_set_volt(pmu_power_mode);

    if (mode == PMU_POWER_MODE_ANA_DCDC) {
        pmu_dcdc_ana_mode_en();
    } else if (mode == PMU_POWER_MODE_DIG_DCDC) {
        pmu_dcdc_dual_mode_en();
    } else if (mode == PMU_POWER_MODE_LDO) {
        pmu_ldo_mode_en();
    }

    pmu_power_mode = mode;

    int_unlock(lock);
}

void pmu_sleep_en(unsigned char sleep_en)
{
    unsigned short val;

    pmu_read(PMU_REG_SLEEP_CFG, &val);
    if(sleep_en) {
        val |= SLEEP_ALLOW;
    } else {
        val &= ~SLEEP_ALLOW;
    }
    pmu_write(PMU_REG_SLEEP_CFG, val);
}

#if defined(PMU_INIT) || (!defined(FPGA) && !defined(PROGRAMMER))
static POSSIBLY_UNUSED uint32_t pmu_vcodec_mv_to_val(uint16_t mv)
{
    switch(mv) {
    case 1500:
        return PMU_CODEC_1_5V;
    case 1600:
        return PMU_CODEC_1_6V;
    case 1660:
    case 1700:
        return PMU_CODEC_1_7V;
    case 1900:
        return PMU_CODEC_1_9V;
    case 2000:
        return PMU_CODEC_2_0V;
    case 2500:
        return PMU_CODEC_2_5V;
    case 2800:
        return PMU_CODEC_2_8V;
    default:
        return PMU_CODEC_1_8V;
    }
}

#ifndef HPPA_LDO_ON
static uint32_t pmu_dcdc_hppa_mv_to_val(uint16_t mv)
{
    uint32_t val;

    if (mv == 1600) {
        val = PMU_DCDC_HPPA_1_6V;
    } else if (mv == 1650) {
        val = PMU_DCDC_HPPA_1_65V;
    } else if (mv == 1700) {
        val = PMU_DCDC_HPPA_1_7V;
    } else if (mv == 1900) {
        val = PMU_DCDC_HPPA_1_9V;
    } else if (mv >= 1950) {
        val = PMU_DCDC_HPPA_1_95V;
    } else {
        val = PMU_DCDC_HPPA_1_8V;
    }

    return val;
}

static void pmu_dcdc_hppa_get_volt(unsigned short *normal_vp, unsigned short *dsleep_vp)
{
    unsigned short val;

    pmu_read(PMU_REG_DCDC_HPPA_VOLT, &val);
    if (normal_vp) {
        *normal_vp = GET_BITFIELD(val, REG_BUCK_HPPA_VBIT_NORMAL);
    }
    if (dsleep_vp) {
        *dsleep_vp = GET_BITFIELD(val, REG_BUCK_HPPA_VBIT_DSLEEP);
    }

#ifdef PMU_DCDC_CALIB
    if (normal_vp) {
        *normal_vp = pmu_reg_val_add(*normal_vp, -pmu_dcdc_hppa_comp, MAX_BUCK_VHPPA_BIT_VAL);
    }
    if (dsleep_vp) {
        *dsleep_vp = pmu_reg_val_add(*dsleep_vp, -pmu_hppa_lp_comp, MAX_BUCK_VHPPA_BIT_VAL);
    }
#endif
}

static void pmu_dcdc_hppa_set_volt(unsigned short normal_v,unsigned short dsleep_v)
{
    unsigned short val;

#ifdef PMU_DCDC_CALIB
    normal_v = pmu_reg_val_add(normal_v, pmu_dcdc_hppa_comp, MAX_BUCK_VHPPA_BIT_VAL);
    dsleep_v = pmu_reg_val_add(dsleep_v, pmu_hppa_lp_comp, MAX_BUCK_VHPPA_BIT_VAL);
#endif

    pmu_read(PMU_REG_DCDC_HPPA_VOLT, &val);
    val &= ~REG_BUCK_HPPA_VBIT_DSLEEP_MASK;
    val &= ~REG_BUCK_HPPA_VBIT_NORMAL_MASK;
    val |= REG_BUCK_HPPA_VBIT_DSLEEP(dsleep_v);
    val |= REG_BUCK_HPPA_VBIT_NORMAL(normal_v);
    pmu_write(PMU_REG_DCDC_HPPA_VOLT, val);

    if (normal_v > dsleep_v) {
        dcdc_ramp_map |= REG_BUCK_HPPA_RAMP_EN;
    } else {
        dcdc_ramp_map &= ~REG_BUCK_HPPA_RAMP_EN;
    }
}

static void pmu_dcdc_hppa_en(int enable)
{
    uint16_t val;

    pmu_read(PMU_REG_DCDC_HPPA_EN, &val);
    if (enable) {
        val |= DCDC_HPPA_LP_EN_DSLEEP | PU_DCDC_HPPA_DR| PU_DCDC_HPPA_REG | PU_DCDC_HPPA_DSLEEP |
            REG_BUCK_HPPA_BURST_MODE_NORMAL | REG_BUCK_HPPA_BURST_MODE_DSLEEP;
    } else {
        val &= ~(DCDC_HPPA_LP_EN_DSLEEP | PU_DCDC_HPPA_DR| PU_DCDC_HPPA_REG | PU_DCDC_HPPA_DSLEEP |
            REG_BUCK_HPPA_BURST_MODE_NORMAL | REG_BUCK_HPPA_BURST_MODE_DSLEEP);
    }
    pmu_write(PMU_REG_DCDC_HPPA_EN, val);
}

static void pmu_hppa_dcdc_to_ldo(void)
{
    pmu_module_set_volt(PMU_CODEC, PMU_CODEC_1_8V, PMU_CODEC_1_8V);
    pmu_module_config(PMU_CODEC,PMU_MANUAL_MODE,PMU_LDO_ON,PMU_LP_MODE_OFF,PMU_DSLEEP_MODE_OFF);
    hal_sys_timer_delay_us(PMU_LDO_PU_STABLE_TIME_US);
    pmu_dcdc_hppa_en(false);
}
#endif

static void BOOT_TEXT_FLASH_LOC pmu_dig_init_volt(void)
{
    uint16_t ldo_volt;
    uint16_t val;

    pmu_dig_get_target_volt(&ldo_volt, NULL);

    pmu_read(PMU_REG_DIG_CFG, &val);
    if (GET_BITFIELD(val, LDO_DIG_VBIT_NORMAL) < ldo_volt) {
        val = SET_BITFIELD(val, LDO_DIG_VBIT_NORMAL, ldo_volt);
        pmu_write(PMU_REG_DIG_CFG, val);
    }
}

static uint32_t pmu_vhppa_mv_to_val(uint16_t mv)
{
#ifdef HPPA_LDO_ON
    return pmu_ldo_hppa_mv_to_val(mv);
#else
    return pmu_dcdc_hppa_mv_to_val(mv);
#endif
}

static void POSSIBLY_UNUSED pmu_hppa_get_volt(unsigned short *normal_vp, unsigned short *dsleep_vp)
{
#ifdef HPPA_LDO_ON
    pmu_ldo_hppa_get_volt(normal_vp, dsleep_vp);
#else
    pmu_dcdc_hppa_get_volt(normal_vp, dsleep_vp);
#endif
}

static void pmu_hppa_set_volt(unsigned short normal_v,unsigned short dsleep_v)
{
#ifdef HPPA_LDO_ON
    pmu_ldo_hppa_set_volt(normal_v, dsleep_v);
#else
    pmu_dcdc_hppa_set_volt(normal_v, dsleep_v);
#endif
}

int pmu_codec_volt_ramp_up(void)
{
#ifndef VMEM_ON
    unsigned short normal, dsleep;
    unsigned short target;

    // No need to ramp if VCM_LPF is enabled
    if (vcodec_off) {
        target = pmu_vhppa_mv_to_val(HPPA_RAMP_UP_VOLT_MV);
        pmu_hppa_get_volt(&normal, &dsleep);
        if (normal < target) {
            pmu_hppa_set_volt(target, dsleep);
#ifdef HPPA_LDO_ON
            osDelay((PMU_LDO_PU_STABLE_TIME_US + (1000 - 1)) / 1000)
#else
            hal_sys_timer_delay_us(PMU_DCDC_PU_STABLE_TIME_US);
#endif
        }
    }
#endif
    return 0;
}


int pmu_codec_volt_ramp_down(void)
{
#ifndef VMEM_ON
    unsigned short normal, dsleep;
    unsigned short target;

    // No need to ramp if VCM_LPF is enabled
    if (vcodec_off) {
        target = pmu_vhppa_mv_to_val(vhppa_mv);
        pmu_hppa_get_volt(&normal, &dsleep);
        if (normal > target) {
            pmu_hppa_set_volt(target, dsleep);
        }
    }
#endif
    return 0;
}
#endif

int BOOT_TEXT_FLASH_LOC pmu_open(void)
{
#if defined(PMU_INIT) || (!defined(FPGA) && !defined(PROGRAMMER))

    uint16_t val;
    enum PMU_POWER_MODE_T mode;
    ASSERT(!vcodec_off || vcodec_mv == vhppa_mv,
        "Invalid vcodec/vhppa cfg: vcodec_off=%d vcodec_mv=%u vhppa_mv=%u", vcodec_off, vcodec_mv, vhppa_mv);
    ASSERT(vcodec_mv == 1600 || vcodec_mv == 1650 || vcodec_mv == 1700 || vcodec_mv == 1800 || vcodec_mv == 1900 || vcodec_mv == 1950,
        "Invalid vcodec cfg: vcodec_mv=%u", vcodec_mv);
    ASSERT(vhppa_mv == 1600 || vhppa_mv == 1650 || vhppa_mv == 1700 || vhppa_mv == 1800 || vhppa_mv == 1900 || vhppa_mv == 1950,
        "Invalid vhppa cfg: vhppa_mv=%u", vhppa_mv);
    TRACE(0, "%s vcodec_mv:%d vhppa_mv:%d", __func__, vcodec_mv, vhppa_mv);

    // Disable and clear all PMU irqs by default
    pmu_write(PMU_REG_INT_MASK, 0);
    pmu_write(PMU_REG_INT_EN, 0);
    // PMU irqs cannot be cleared by PMU soft reset
    pmu_read(PMU_REG_CHARGER_STATUS, &val);
    pmu_write(PMU_REG_CHARGER_STATUS, val);
    pmu_read(PMU_REG_INT_STATUS, &val);
    pmu_write(PMU_REG_INT_CLR, val);

    // Allow PMU to sleep when power key is pressed
    pmu_read(PMU_REG_POWER_KEY_CFG, &val);
    val &= ~POWERKEY_WAKEUP_OSC_EN;
    pmu_write(PMU_REG_POWER_KEY_CFG, val);

    // Increase big bandgap startup time (stable time)
    pmu_read(PMU_REG_CRYSTAL_CFG, &val);
    val = SET_BITFIELD(val, REG_VCORE_SSTIME_MODE, 2);
    pmu_write(PMU_REG_CRYSTAL_CFG, val);

    pmu_read(PMU_REG_BIAS_CFG, &val);
#if defined(DIG_DCDC_MODE) || defined(ANA_DCDC_MODE)
    if (hal_get_chip_metal_id() == HAL_CHIP_METAL_ID_0) {
        val |= BG_VBG_SEL_DR | BG_VBG_SEL_REG |
            BG_CONSTANT_GM_BIAS_DR | BG_CONSTANT_GM_BIAS_REG |
            BG_CORE_EN_DR | BG_CORE_EN_REG;
    } else
#endif
    {
#ifndef PMU_FORCE_LP_MODE
        // Allow low power bandgap
        val &= ~BG_VBG_SEL_DR;
#endif
    }
    pmu_write(PMU_REG_BIAS_CFG, val);

    // Vhppa cap bits
    pmu_read(PMU_REG_DCDC_HPPA_CAP, &val);
    val = SET_BITFIELD(val, REG_BUCK_HPPA_CAP_BIT, 0xE);
    pmu_write(PMU_REG_DCDC_HPPA_CAP, val);

    // Disable DCDC sync
    pmu_read(PMU_REG_DCDC_DIG_SYNC, &val);
    val |= REG_BUCK_VCORE_SYNC_DISABLE;
    pmu_write(PMU_REG_DCDC_DIG_SYNC, val);

    // Enable GPADC
    pmu_read(PMU_REG_SAR_EN, &val);
    val = (val & ~REG_SAR_VCM_CORE_SEL_MASK) | REG_SAR_VCM_CORE_SEL(0) | REG_SAR_BUF_EN;
    pmu_write(PMU_REG_SAR_EN, val);

#ifdef PMU_IRQ_UNIFIED
    pmu_read(PMU_REG_WDT_CFG, &val);
    val = (val & ~POWERON_DETECT_EN) | MERGE_INTR;
    pmu_write(PMU_REG_WDT_CFG, val);
#endif

#ifdef PMU_DCDC_CALIB
    pmu_get_dcdc_calib_value();
#endif

#ifdef PMU_LDO_VCORE_CALIB
    pmu_get_ldo_dig_calib_value();
#endif

#ifndef NO_SLEEP
    pmu_sleep_en(1);  //enable sleep
#endif

#ifndef HPPA_LDO_ON
    // enable vhppa dcdc
    val = pmu_dcdc_hppa_mv_to_val(vhppa_mv);
    pmu_dcdc_hppa_set_volt(val, val);
    pmu_dcdc_hppa_en(true);
    hal_sys_timer_delay_us(PMU_DCDC_PU_STABLE_TIME_US);
    pmu_module_config(PMU_CODEC,PMU_MANUAL_MODE,PMU_LDO_OFF,PMU_LP_MODE_ON,PMU_DSLEEP_MODE_OFF);
#else
    val = pmu_vcodec_mv_to_val(vcodec_mv);
    pmu_module_ramp_volt(PMU_CODEC, val, val);
#endif

#if !defined(PROGRAMMER) && !defined(VUSB_ON)
    pmu_module_config(PMU_USB,PMU_MANUAL_MODE,PMU_LDO_OFF,PMU_LP_MODE_ON,PMU_DSLEEP_MODE_OFF); //disable VUSB
#endif

#ifdef DIG_DCDC_MODE
    mode = PMU_POWER_MODE_DIG_DCDC;
#elif defined(ANA_DCDC_MODE)
    mode = PMU_POWER_MODE_ANA_DCDC;
#else // LDO_MODE
    mode = PMU_POWER_MODE_LDO;
#endif

    pmu_mode_change(mode);

    pmu_read(PMU_REG_LPO_VBG_CFG, &val);
    val |= REG_BYPASS_VBG_RF_BUFFER_DR | REG_BYPASS_VBG_RF_BUFFER_REG | PU_VBG_RF_BUFFER_DR;
    val &= ~PU_VBG_RF_BUFFER_REG;
    pmu_write(PMU_REG_LPO_VBG_CFG, val);

#ifdef PMU_FORCE_LP_MODE
#if defined(DIG_DCDC_MODE) || defined(ANA_DCDC_MODE)
    if (hal_get_chip_metal_id() == HAL_CHIP_METAL_ID_0) {
    } else
#endif
    {
        // Reduce LDO voltage ripple
        pmu_read(PMU_REG_AVDD_EN, &val);
        val = SET_BITFIELD(val, REG_LP_BIAS_SEL_LDO, 0);
        pmu_write(PMU_REG_AVDD_EN, val);

        // Force LP mode
        pmu_module_force_lp_config();
        pmu_codec_mic_bias_lowpower_mode(AUD_VMIC_MAP_VMIC1 | AUD_VMIC_MAP_VMIC2, true);
        hal_sys_timer_delay(MS_TO_TICKS(1));

        // Switch to little bandgap
        pmu_read(PMU_REG_BIAS_CFG, &val);
        val = (val & ~BG_VBG_SEL_REG) | BG_VBG_SEL_DR;
        pmu_write(PMU_REG_BIAS_CFG, val);
        hal_sys_timer_delay(MS_TO_TICKS(1));

        if (big_bandgap_user_map == 0) {
            // Disable big bandgap
            val = (val & ~(PU_BIAS_LDO_REG | BG_CONSTANT_GM_BIAS_REG | BG_CORE_EN_REG | BG_VTOI_EN_REG | BG_NOTCH_EN_REG)) |
                PU_BIAS_LDO_DR | BG_CONSTANT_GM_BIAS_DR | BG_CORE_EN_DR | BG_VTOI_EN_DR | BG_NOTCH_EN_DR;
            pmu_write(PMU_REG_BIAS_CFG, val);
        }
    }
#endif

#ifdef RTC_ENABLE
    pmu_rtc_restore_context();
#endif

#if defined(MCU_HIGH_PERFORMANCE_MODE)
    // Increase bbpll voltage
    rf_write(0xA0, 0x3FE8); // div=2 (bit14=0)
    rf_write(0xA1, 0xF918);

    pmu_high_performance_mode_enable(true);
#endif

#ifdef USE_POWER_KEY_RESET
    pmu_power_key_hw_reset_enable(15);
#endif
    pmu_charger_out_poweron_init();
#endif // PMU_INIT || (!FPGA && !PROGRAMMER)

    return 0;
}

void pmu_big_bandgap_enable(enum PMU_BIG_BANDGAP_USER_T user, int enable)
{
#ifdef PMU_FORCE_LP_MODE
    uint16_t val;
    bool update = false;

    if (enable) {
        if (big_bandgap_user_map == 0) {
            update = true;
        }
        big_bandgap_user_map |= user;
    } else {
        if (big_bandgap_user_map & user) {
            big_bandgap_user_map &= ~user;
            if (big_bandgap_user_map == 0) {
                update = true;
            }
        }
    }

    if (!update) {
        return;
    }

    pmu_read(PMU_REG_BIAS_CFG, &val);
    val |= PU_BIAS_LDO_DR | BG_CONSTANT_GM_BIAS_DR | BG_CORE_EN_DR | BG_VTOI_EN_DR | BG_NOTCH_EN_DR | BG_VBG_SEL_DR;
    if (enable) {
        val |= PU_BIAS_LDO_REG | BG_CONSTANT_GM_BIAS_REG | BG_CORE_EN_REG | BG_VTOI_EN_REG | BG_NOTCH_EN_REG;
    } else {
        val &= ~(PU_BIAS_LDO_REG | BG_CONSTANT_GM_BIAS_REG | BG_CORE_EN_REG | BG_VTOI_EN_REG | BG_NOTCH_EN_REG | BG_VBG_SEL_REG);
    }
    pmu_write(PMU_REG_BIAS_CFG, val);
    if (enable) {
        val |= BG_VBG_SEL_REG;
        hal_sys_timer_delay_us(PMU_BIG_BG_STABLE_TIME_US);
        pmu_write(PMU_REG_BIAS_CFG, val);
    }
#endif
}

void pmu_sleep(void)
{
    uint16_t val;

    if (dcdc_ramp_map) {
        // Enable DCDC ramp
        pmu_read(PMU_REG_DCDC_RAMP_EN, &val);
        val |= dcdc_ramp_map;
        pmu_write(PMU_REG_DCDC_RAMP_EN, val);
    }
}

void pmu_wakeup(void)
{
    uint16_t val;

    if (dcdc_ramp_map) {
        // Disable DCDC ramp so that s/w can control the voltages freely
        pmu_read(PMU_REG_DCDC_RAMP_EN, &val);
        val &= ~dcdc_ramp_map;
        pmu_write(PMU_REG_DCDC_RAMP_EN, val);
    }
}

void pmu_codec_config(int enable)
{
#ifdef HPPA_LDO_ON
    if (!vcodec_off) {
        if (enable) {
            pmu_module_config(PMU_CODEC,PMU_MANUAL_MODE,PMU_LDO_ON,PMU_LP_MODE_ON,PMU_DSLEEP_MODE_OFF);
        } else {
            pmu_module_config(PMU_CODEC,PMU_MANUAL_MODE,PMU_LDO_OFF,PMU_LP_MODE_ON,PMU_DSLEEP_MODE_OFF);
        }
    }
#endif
}

void pmu_codec_hppa_enable(int enable)
{
}

void pmu_codec_mic_bias_enable(uint32_t map)
{
    uint16_t val_a, val_b, vsel;

    // Increase LDO_RES if higher vmic is required
#ifdef DIGMIC_HIGH_VOLT
    val_a = REG_MIC_LDO_RES(0xF);
    vsel =REG_MIC_BIASA_VSEL(0x3F);
#else
    val_a = REG_MIC_LDO_RES(7);
    vsel =REG_MIC_BIASA_VSEL(0x1F);
#endif
    val_b = 0;
    if (map & AUD_VMIC_MAP_VMIC1) {
        val_a |= REG_MIC_BIASA_EN | REG_MIC_BIASA_ENLPF | vsel;
        val_b |= REG_MIC_LDO_EN;
    }
    if (map & AUD_VMIC_MAP_VMIC2) {
        val_b |= REG_MIC_BIASB_EN | REG_MIC_BIASB_ENLPF | vsel | REG_MIC_LDO_EN;
    }
    pmu_write(PMU_REG_MIC_BIAS_A, val_a);
    pmu_write(PMU_REG_MIC_BIAS_B, val_b);
}

void pmu_codec_mic_bias_lowpower_mode(uint32_t map, int enable)
{
    uint16_t val;

    if (map & (AUD_VMIC_MAP_VMIC1 | AUD_VMIC_MAP_VMIC2)) {
        pmu_read(PMU_REG_MIC_PULL_DOWN, &val);
        if (enable) {
            val |= REG_MIC_LP_ENABLE;
        } else {
            val &= ~REG_MIC_LP_ENABLE;
        }
        pmu_write(PMU_REG_MIC_PULL_DOWN, val);
    }
}

SRAM_TEXT_LOC void pmu_flash_write_config(void)
{
#ifdef FLASH_WRITE_AT_HIGH_VCORE
    uint32_t lock;

    if (pmu_vcore_req & PMU_VCORE_FLASH_WRITE_ENABLED) {
        return;
    }

    lock = int_lock();
    pmu_vcore_req |= PMU_VCORE_FLASH_WRITE_ENABLED;
    int_unlock(lock);

    pmu_dig_set_volt(pmu_power_mode);
#endif
}

SRAM_TEXT_LOC void pmu_flash_read_config(void)
{
#ifdef FLASH_WRITE_AT_HIGH_VCORE
    uint32_t lock;

    if ((pmu_vcore_req & PMU_VCORE_FLASH_WRITE_ENABLED) == 0) {
        return;
    }

    lock = int_lock();
    pmu_vcore_req &= ~PMU_VCORE_FLASH_WRITE_ENABLED;
    int_unlock(lock);

    pmu_dig_set_volt(pmu_power_mode);
#endif
}

void BOOT_TEXT_FLASH_LOC pmu_flash_freq_config(uint32_t freq)
{
#if defined(PMU_INIT) || (!defined(FPGA) && !defined(PROGRAMMER))
    uint32_t lock;

    lock = int_lock();
    if (freq > 52000000) {
        // The real max freq is 120M
        //pmu_vcore_req |= PMU_VCORE_FLASH_FREQ_HIGH;
    } else {
        pmu_vcore_req &= ~PMU_VCORE_FLASH_FREQ_HIGH;
    }
    int_unlock(lock);

    if (pmu_power_mode == PMU_POWER_MODE_NONE) {
        // PMU not init yet
        pmu_dig_init_volt();
        return;
    }

    pmu_dig_set_volt(pmu_power_mode);
#endif
}

void BOOT_TEXT_FLASH_LOC pmu_psram_freq_config(uint32_t freq)
{
}

void pmu_anc_config(int enable)
{
}

void pmu_fir_high_speed_config(int enable)
{
}

void pmu_iir_freq_config(uint32_t freq)
{
}

void pmu_rs_freq_config(uint32_t freq)
{
    uint32_t lock;

    lock = int_lock();
    if (freq >= 60000000) {
        pmu_vcore_req |= PMU_VCORE_RS_FREQ_HIGH;
    } else {
        pmu_vcore_req &= ~PMU_VCORE_RS_FREQ_HIGH;
    }
    int_unlock(lock);

    pmu_dig_set_volt(pmu_power_mode);
}

void BOOT_TEXT_SRAM_LOC pmu_sys_freq_config(enum HAL_CMU_FREQ_T freq)
{
#if defined(PMU_INIT) || (!defined(FPGA) && !defined(PROGRAMMER))
#if defined(MCU_HIGH_PERFORMANCE_MODE) || defined(ULTRA_LOW_POWER) || !defined(OSC_26M_X4_AUD2BB)
    uint32_t lock;
    enum PMU_VCORE_REQ_T old_req;
    bool update = false;

    lock = int_lock();
    old_req = pmu_vcore_req;
    pmu_vcore_req &= ~(PMU_VCORE_SYS_FREQ_HIGH | PMU_VCORE_SYS_FREQ_MEDIUM | PMU_VCORE_SYS_FREQ_MEDIUM_LOW | PMU_VCORE_SYS_FREQ_LOW);
#if defined(MCU_HIGH_PERFORMANCE_MODE)
    if (freq > HAL_CMU_FREQ_104M) {
        if (high_perf_on) {
            // The real freq is 350M
            pmu_vcore_req |= PMU_VCORE_SYS_FREQ_HIGH;
        } else {
            pmu_vcore_req |= PMU_VCORE_SYS_FREQ_MEDIUM;
        }
    } else {
#ifdef OSC_26M_X4_AUD2BB
        if (freq >= HAL_CMU_FREQ_78M) {
            // The real freq is 96M
            pmu_vcore_req |= PMU_VCORE_SYS_FREQ_MEDIUM_LOW;
        } else if (freq >= HAL_CMU_FREQ_52M) {  
            pmu_vcore_req |= PMU_VCORE_SYS_FREQ_LOW;
        }
#else
        if (high_perf_on) {
            if (freq == HAL_CMU_FREQ_104M) {
                // The real freq is 150M
                pmu_vcore_req |= PMU_VCORE_SYS_FREQ_MEDIUM;
            } else if (freq == HAL_CMU_FREQ_78M) {
                // The real freq is 100M
                pmu_vcore_req |= PMU_VCORE_SYS_FREQ_MEDIUM_LOW;
            } else if (freq == HAL_CMU_FREQ_52M) {
                pmu_vcore_req |= PMU_VCORE_SYS_FREQ_LOW;
            }
        } else {
            if (freq >= HAL_CMU_FREQ_78M) {
                pmu_vcore_req |= PMU_VCORE_SYS_FREQ_MEDIUM_LOW;
            } else if (freq == HAL_CMU_FREQ_52M) {
                pmu_vcore_req |= PMU_VCORE_SYS_FREQ_LOW;
            }
        }
#endif
    }
#else
    if (freq > HAL_CMU_FREQ_104M) {
        pmu_vcore_req |= PMU_VCORE_SYS_FREQ_MEDIUM;
#ifdef OSC_26M_X4_AUD2BB
    } else if (freq >= HAL_CMU_FREQ_78M) {
        // The real freq is 96M
        pmu_vcore_req |= PMU_VCORE_SYS_FREQ_MEDIUM_LOW;
    }else if (freq == HAL_CMU_FREQ_52M) {
        pmu_vcore_req |= PMU_VCORE_SYS_FREQ_LOW;
    }
#else
    } else if (freq == HAL_CMU_FREQ_104M) {
        pmu_vcore_req |= PMU_VCORE_SYS_FREQ_MEDIUM_LOW;
    } else if (freq >= HAL_CMU_FREQ_52M) {
        pmu_vcore_req |= PMU_VCORE_SYS_FREQ_LOW;
    }
#endif
#endif
    if (old_req != pmu_vcore_req) {
        update = true;
    }
    int_unlock(lock);

    if (!update) {
        // Nothing changes
        return;
    }

    if (pmu_power_mode == PMU_POWER_MODE_NONE) {
        // PMU not init yet
        pmu_dig_init_volt();
        return;
    }

    pmu_dig_set_volt(pmu_power_mode);
#endif
#endif
}

void pmu_high_performance_mode_enable(bool enable)
{
#if defined(MCU_HIGH_PERFORMANCE_MODE)
    uint16_t val;

    if (high_perf_on == enable) {
        return;
    }
    high_perf_on = enable;

    if (!enable) {
        if (high_perf_freq_mhz > 300) {
            // Switch to 52M to avoid using PLL
            hal_cmu_sys_set_freq(HAL_CMU_FREQ_52M);
            // Restore the default div
            rf_read(0xA0, &val);
            val &= ~(1 << 14); // div=2 (bit14=0)
            rf_write(0xA0, val);
            // Restore the sys freq
            hal_cmu_sys_set_freq(hal_sysfreq_get_hw_freq());
        }
        // Restore the default PLL freq (384M)
        bbpll_freq_pll_config(192 * 1000000 * 2);
    }

    pmu_sys_freq_config(hal_sysfreq_get_hw_freq());

    if (enable) {
        uint32_t pll_freq;

        // Change freq first, and then change divider.
        // Otherwise there will be an instant very high freq sent to digital domain.

        if (high_perf_freq_mhz <= 300) {
            pll_freq = high_perf_freq_mhz * 1000000 * 2;
        } else {
            pll_freq = high_perf_freq_mhz * 1000000;
        }
        bbpll_freq_pll_config(pll_freq);

        if (high_perf_freq_mhz > 300) {
            // Switch to 52M to avoid using PLL
            hal_cmu_sys_set_freq(HAL_CMU_FREQ_52M);
            rf_read(0xA0, &val);
            val |= (1 << 14); // div=1 (bit14=1)
            rf_write(0xA0, val);
            // Restore the sys freq
            hal_cmu_sys_set_freq(hal_sysfreq_get_hw_freq());
        }
    }
#endif
}

void pmu_usb_config(enum PMU_USB_CONFIG_TYPE_T type)
{

}

#if !defined(FPGA) && !defined(PROGRAMMER)
struct PMU_CHG_CTX_T {
    uint16_t pmu_chg_status;
};

struct PMU_CHG_CTX_T BOOT_BSS_LOC pmu_chg_ctx;

void BOOT_TEXT_SRAM_LOC pmu_charger_save_context(void)
{
    pmu_read(PMU_REG_CHARGER_STATUS, &pmu_chg_ctx.pmu_chg_status);
}

enum PMU_POWER_ON_CAUSE_T pmu_charger_poweron_status(void)
{
    enum PMU_POWER_ON_CAUSE_T pmu_power_on_cause = PMU_POWER_ON_CAUSE_NONE;

    if (pmu_chg_ctx.pmu_chg_status & AC_ON_DET_OUT){
        pmu_power_on_cause = PMU_POWER_ON_CAUSE_CHARGER_ACOFF;
    }else if (pmu_chg_ctx.pmu_chg_status & AC_ON){
        pmu_power_on_cause = PMU_POWER_ON_CAUSE_CHARGER_ACON;
    }
    return pmu_power_on_cause;
}
#endif

void pmu_charger_init(void)
{
    unsigned short readval_cfg;
    uint32_t lock;

    lock = int_lock();
    pmu_read(PMU_REG_CHARGER_CFG, &readval_cfg);
    readval_cfg &= ~(REG_CHARGE_OUT_INTR_MSK | REG_CHARGE_IN_INTR_MSK |
        REG_AC_ON_OUT_EN | REG_AC_ON_IN_EN | REG_CHARGE_INTR_EN);
    pmu_write(PMU_REG_CHARGER_CFG ,readval_cfg);
    int_unlock(lock);

    hal_sys_timer_delay(MS_TO_TICKS(1));

    lock = int_lock();
    pmu_read(PMU_REG_CHARGER_CFG, &readval_cfg);
    readval_cfg |= REG_AC_ON_OUT_EN | REG_AC_ON_IN_EN | REG_CHARGE_INTR_EN;
    readval_cfg = SET_BITFIELD(readval_cfg, REG_AC_ON_DB_VALUE, 8);
    pmu_write(PMU_REG_CHARGER_CFG ,readval_cfg);
    int_unlock(lock);
}

void pmu_charger_shutdown_config(void)
{

}

static void pmu_charger_irq_handler(PMU_IRQ_HDLR_PARAM)
{
    enum PMU_CHARGER_STATUS_T status = PMU_CHARGER_UNKNOWN;
    unsigned short readval;

#ifdef PMU_IRQ_UNIFIED
    readval = irq_status;
#else
    uint32_t lock;

    lock = int_lock();
    pmu_read(PMU_REG_CHARGER_STATUS, &readval);
    pmu_write(PMU_REG_CHARGER_STATUS, readval);
    int_unlock(lock);
#endif
    PMU_DEBUG_TRACE(3,"%s REG_%02X=0x%04X", __func__, PMU_REG_CHARGER_STATUS, readval);

    if ((readval & (INTR_MSKED_CHARGE_IN | INTR_MSKED_CHARGE_OUT)) == 0){
        PMU_DEBUG_TRACE(1,"%s SKIP", __func__);
        return;
    } else if ((readval & (INTR_MSKED_CHARGE_IN | INTR_MSKED_CHARGE_OUT)) ==
            (INTR_MSKED_CHARGE_IN | INTR_MSKED_CHARGE_OUT)) {
        PMU_DEBUG_TRACE(1,"%s DITHERING", __func__);
        hal_sys_timer_delay(2);
    } else {
        PMU_DEBUG_TRACE(1,"%s NORMAL", __func__);
    }

    status = pmu_charger_get_status();

    if (charger_irq_handler) {
        charger_irq_handler(status);
    }
}

void pmu_charger_set_irq_handler(PMU_CHARGER_IRQ_HANDLER_T handler)
{
    uint32_t lock;
    uint16_t val;

    charger_irq_handler = handler;

    lock = int_lock();
    pmu_read(PMU_REG_CHARGER_CFG, &val);
    if (handler) {
        val |= REG_CHARGE_IN_INTR_MSK | REG_CHARGE_OUT_INTR_MSK;
    } else {
        val &= ~(REG_CHARGE_IN_INTR_MSK | REG_CHARGE_OUT_INTR_MSK);
    }
    pmu_write(PMU_REG_CHARGER_CFG, val);

#ifdef PMU_IRQ_UNIFIED
    pmu_set_irq_unified_handler(PMU_IRQ_TYPE_CHARGER, handler ? pmu_charger_irq_handler : NULL);
#else
    if (handler) {
        NVIC_SetVector(CHARGER_IRQn, (uint32_t)pmu_charger_irq_handler);
        NVIC_SetPriority(CHARGER_IRQn, IRQ_PRIORITY_NORMAL);
        NVIC_ClearPendingIRQ(CHARGER_IRQn);
        NVIC_EnableIRQ(CHARGER_IRQn);
    } else {
        NVIC_DisableIRQ(CHARGER_IRQn);
    }
#endif
    int_unlock(lock);
}

void pmu_charger_plugin_config(void)
{
}

void pmu_charger_plugout_config(void)
{
}

enum PMU_CHARGER_STATUS_T pmu_charger_get_status(void)
{
    unsigned short readval;
    enum PMU_CHARGER_STATUS_T status;

    pmu_read(PMU_REG_CHARGER_STATUS, &readval);
    if (readval & AC_ON)
        status = PMU_CHARGER_PLUGIN;
    else
        status = PMU_CHARGER_PLUGOUT;

    return status;
}

#ifdef RTC_ENABLE
void pmu_rtc_enable(void)
{
    uint16_t readval;
    uint32_t lock;

#ifdef SIMU
    // Set RTC counter to 1KHz
    pmu_write(PMU_REG_RTC_DIV_1HZ, 32 - 2);
#else
    // Set RTC counter to 1Hz
    pmu_write(PMU_REG_RTC_DIV_1HZ, CONFIG_SYSTICK_HZ * 2 - 2);
#endif

    lock = int_lock();
    pmu_read(PMU_REG_POWER_KEY_CFG, &readval);
    readval |= RTC_POWER_ON_EN | PU_LPO_DR | PU_LPO_REG;
    pmu_write(PMU_REG_POWER_KEY_CFG, readval);
    int_unlock(lock);
}

void pmu_rtc_disable(void)
{
    uint16_t readval;
    uint32_t lock;

    pmu_rtc_clear_alarm();

    lock = int_lock();
    pmu_read(PMU_REG_POWER_KEY_CFG, &readval);
    readval &= ~(RTC_POWER_ON_EN | PU_LPO_DR);
    pmu_write(PMU_REG_POWER_KEY_CFG, readval);
    int_unlock(lock);
}

int BOOT_TEXT_SRAM_LOC pmu_rtc_enabled(void)
{
    uint16_t readval;

    pmu_read(PMU_REG_POWER_KEY_CFG, &readval);

    return !!(readval & RTC_POWER_ON_EN);
}

void pmu_rtc_set(uint32_t seconds)
{
    uint16_t high, low;

    // Need 3 seconds to load a new value
    seconds += 3;

    high = seconds >> 16;
    low = seconds & 0xFFFF;

    pmu_write(PMU_REG_RTC_LOAD_LOW, low);
    pmu_write(PMU_REG_RTC_LOAD_HIGH, high);
}

uint32_t pmu_rtc_get(void)
{
    uint16_t high, low, high2;

    pmu_read(PMU_REG_RTC_VAL_HIGH, &high);
    pmu_read(PMU_REG_RTC_VAL_LOW, &low);
    // Handle counter wrap
    pmu_read(PMU_REG_RTC_VAL_HIGH, &high2);
    if (high != high2) {
        high = high2;
        pmu_read(PMU_REG_RTC_VAL_LOW, &low);
    }

    return (high << 16) | low;
}

void pmu_rtc_set_alarm(uint32_t seconds)
{
    uint16_t readval;
    uint16_t high, low;
    uint32_t lock;

    // Need 1 second to raise the interrupt
    if (seconds > 0) {
        seconds -= 1;
    }

    high = seconds >> 16;
    low = seconds & 0xFFFF;

    pmu_write(PMU_REG_INT_CLR, RTC_INT_CLR_1);

    pmu_write(PMU_REG_RTC_MATCH1_LOW, low);
    pmu_write(PMU_REG_RTC_MATCH1_HIGH, high);

    lock = int_lock();
    pmu_read(PMU_REG_INT_EN, &readval);
    readval |= RTC_INT_EN_1;
    pmu_write(PMU_REG_INT_EN, readval);
    int_unlock(lock);
}

uint32_t BOOT_TEXT_SRAM_LOC pmu_rtc_get_alarm(void)
{
    uint16_t high, low;

    pmu_read(PMU_REG_RTC_MATCH1_LOW, &low);
    pmu_read(PMU_REG_RTC_MATCH1_HIGH, &high);

    // Compensate the alarm offset
    return (uint32_t)((high << 16) | low) + 1;
}

void pmu_rtc_clear_alarm(void)
{
    uint16_t readval;
    uint32_t lock;

    lock = int_lock();
    pmu_read(PMU_REG_INT_EN, &readval);
    readval &= ~RTC_INT_EN_1;
    pmu_write(PMU_REG_INT_EN, readval);
    int_unlock(lock);

    pmu_write(PMU_REG_INT_CLR, RTC_INT_CLR_1);
}

int BOOT_TEXT_SRAM_LOC pmu_rtc_alarm_status_set(void)
{
    uint16_t readval;

    pmu_read(PMU_REG_INT_EN, &readval);

    return !!(readval & RTC_INT_EN_1);
}

int pmu_rtc_alarm_alerted()
{
    uint16_t readval;

    pmu_read(PMU_REG_INT_STATUS, &readval);

    return !!(readval & RTC_INT_1);
}

static void pmu_rtc_irq_handler(PMU_IRQ_HDLR_PARAM)
{
    uint32_t seconds;
    bool alerted;

#ifdef PMU_IRQ_UNIFIED
    alerted = !!(irq_status & RTC_INT_1);
#else
    alerted = pmu_rtc_alarm_alerted();
#endif

    if (alerted) {
        pmu_rtc_clear_alarm();

        if (rtc_irq_handler) {
            seconds = pmu_rtc_get();
            rtc_irq_handler(seconds);
        }
    }
}

void pmu_rtc_set_irq_handler(PMU_RTC_IRQ_HANDLER_T handler)
{
    uint16_t readval;
    uint32_t lock;

    rtc_irq_handler = handler;

    lock = int_lock();
    pmu_read(PMU_REG_INT_MASK, &readval);
    if (handler) {
        readval |= RTC_INT1_MSK;
    } else {
        readval &= ~RTC_INT1_MSK;
    }
    pmu_write(PMU_REG_INT_MASK, readval);

#ifdef PMU_IRQ_UNIFIED
    pmu_set_irq_unified_handler(PMU_IRQ_TYPE_RTC, handler ? pmu_rtc_irq_handler : NULL);
#else
    if (handler) {
        NVIC_SetVector(RTC_IRQn, (uint32_t)pmu_rtc_irq_handler);
        NVIC_SetPriority(RTC_IRQn, IRQ_PRIORITY_NORMAL);
        NVIC_ClearPendingIRQ(RTC_IRQn);
        NVIC_EnableIRQ(RTC_IRQn);
    } else {
        NVIC_DisableIRQ(RTC_IRQn);
    }
#endif
    int_unlock(lock);
}
#endif

#ifdef PMU_IRQ_UNIFIED
static void pmu_general_irq_handler(void)
{
    uint32_t lock;
    uint16_t val;
    bool pwrkey, charger, gpadc, rtc, gpio, wdt;

    pwrkey = false;
    charger = false;
    gpadc = false;
    rtc = false;
    gpio = false;
    wdt = false;

    lock = int_lock();
    pmu_read(PMU_REG_CHARGER_STATUS, &val);
    if (val & (POWER_ON_RELEASE | POWER_ON_PRESS)) {
        pwrkey = true;
    }
    if (val & (INTR_MSKED_CHARGE_IN | INTR_MSKED_CHARGE_OUT)) {
        charger = true;
    }
    if (pwrkey || charger) {
        pmu_write(PMU_REG_CHARGER_STATUS, val);
    }
    int_unlock(lock);

    if (charger) {
        if (pmu_irq_hdlrs[PMU_IRQ_TYPE_CHARGER]) {
            pmu_irq_hdlrs[PMU_IRQ_TYPE_CHARGER](val);
        }
    }

    lock = int_lock();
    pmu_read(PMU_REG_INT_MSKED_STATUS, &val);
    if (hal_gpadc_masked_irq_valid(val)) {
        val = hal_gpadc_filter_out_unmasked_irq(val);
        gpadc = true;
    }
    if (val & (RTC_INT1_MSKED | RTC_INT0_MSKED)) {
        rtc = true;
    }
    if (gpadc || rtc) {
        pmu_write(PMU_REG_INT_CLR, val);
    }
    int_unlock(lock);

    if (gpadc) {
        if (pmu_irq_hdlrs[PMU_IRQ_TYPE_GPADC]) {
            pmu_irq_hdlrs[PMU_IRQ_TYPE_GPADC](val);
        }
    }
    if (rtc) {
        if (pmu_irq_hdlrs[PMU_IRQ_TYPE_RTC]) {
            pmu_irq_hdlrs[PMU_IRQ_TYPE_RTC](val);
        }
    }

    lock = int_lock();
    pmu_read(PMU_REG_LED_IO_IN, &val);
    if (val & (PMU_GPIO_INTR_MSKED1 | PMU_GPIO_INTR_MSKED2)) {
        gpio = true;
    }
    if (val & WDT_INTR_MSKED) {
        wdt = true;
    }
    if (gpio || wdt) {
        uint16_t clr;

        clr = val;
        if (wdt) {
            clr |= REG_WDT_INTR_CLR;
        }
        pmu_write(PMU_REG_LED_IO_IN, clr);
    }
    int_unlock(lock);

    if (gpio) {
        if (pmu_irq_hdlrs[PMU_IRQ_TYPE_GPIO]) {
            pmu_irq_hdlrs[PMU_IRQ_TYPE_GPIO](val);
        }
    }
    if (wdt) {
        if (pmu_irq_hdlrs[PMU_IRQ_TYPE_WDT]) {
            pmu_irq_hdlrs[PMU_IRQ_TYPE_WDT](val);
        }
    }
}

int pmu_set_irq_unified_handler(enum PMU_IRQ_TYPE_T type, PMU_IRQ_UNIFIED_HANDLER_T hdlr)
{
    bool update;
    uint32_t lock;
    int i;

    if (type >= PMU_IRQ_TYPE_QTY) {
        return 1;
    }

    enum PMU_REG_T reg;
    uint16_t val;
    uint16_t mask;

    if (type == PMU_IRQ_TYPE_GPADC) {
        reg = PMU_REG_USB_CFG2;
        mask = GPADC_INTR_MERGED_MSK;
    } else if (type == PMU_IRQ_TYPE_RTC) {
        reg = PMU_REG_USB_CFG2;
        mask = RTC_INTR_TMP_MERGED_MSK;
    } else if (type == PMU_IRQ_TYPE_CHARGER) {
        reg = PMU_REG_USB_CFG2;
        mask = CHARGE_INTR_MERGED_MSK;
    } else if (type == PMU_IRQ_TYPE_GPIO) {
        reg = PMU_REG_USB_CFG3;
        mask = PMU_GPIO_INTR_MSKED1_MERGED_MSK | PMU_GPIO_INTR_MSKED2_MERGED_MSK;
    } else if (type == PMU_IRQ_TYPE_WDT) {
        reg = PMU_REG_USB_CFG3;
        mask = WDT_INTR_MSKED_MERGED_MSK;
    } else {
        return 2;
    }

    update = false;

    lock = int_lock();

    for (i = 0; i < PMU_IRQ_TYPE_QTY; i++) {
        if (pmu_irq_hdlrs[i]) {
            break;
        }
    }

    pmu_irq_hdlrs[type] = hdlr;

    pmu_read(reg, &val);
    if (hdlr) {
        val |= mask;
    } else {
        val &= ~mask;
    }
    pmu_write(reg, val);

    if (hdlr) {
        update = (i >= PMU_IRQ_TYPE_QTY);
    } else {
        if (i == type) {
            for (; i < PMU_IRQ_TYPE_QTY; i++) {
                if (pmu_irq_hdlrs[i]) {
                    break;
                }
            }
            update = (i >= PMU_IRQ_TYPE_QTY);
        }
    }

    if (update) {
        if (hdlr) {
            NVIC_SetVector(RTC_IRQn, (uint32_t)pmu_general_irq_handler);
            NVIC_SetPriority(RTC_IRQn, IRQ_PRIORITY_NORMAL);
            NVIC_ClearPendingIRQ(RTC_IRQn);
            NVIC_EnableIRQ(RTC_IRQn);
        } else {
            NVIC_DisableIRQ(RTC_IRQn);
        }
    }

    int_unlock(lock);

    return 0;
}
#endif

void pmu_viorise_req(enum PMU_VIORISE_REQ_USER_T user, bool rise)
{
    uint32_t lock;

    lock = int_lock();
    if (rise) {
        if (vio_risereq_map == 0) {
            pmu_module_set_volt(PMU_IO,vio_lp,vio_act_rise);
        }
        vio_risereq_map |= (1 << user);
    } else {
        vio_risereq_map &= ~(1 << user);
        if (vio_risereq_map == 0) {
            pmu_module_set_volt(PMU_IO,vio_lp,vio_act_normal);
        }
    }
    int_unlock(lock);
}

int pmu_debug_config_ana(uint16_t volt)
{
    return 0;
}

int pmu_debug_config_codec(uint16_t volt)
{
    return 0;
}

int pmu_debug_config_vcrystal(bool on)
{
    return 0;
}

int pmu_debug_config_audio_output(bool diff)
{
    return 0;
}

void pmu_debug_reliability_test(int stage)
{
    uint16_t volt;

    if (stage == 0) {
        volt = PMU_DCDC_ANA_1_2V;
    } else {
        volt = PMU_DCDC_ANA_1_3V;
    }
    pmu_dcdc_ana_set_volt(volt, ana_lp_dcdc);
}

void pmu_led_set_hiz(enum HAL_GPIO_PIN_T pin)
{
    uint16_t val;
    uint32_t lock;

    if (pin == HAL_GPIO_PIN_LED1 || pin == HAL_GPIO_PIN_LED2) {
        lock = int_lock();
        if (pin == HAL_GPIO_PIN_LED1) {
            pmu_read(PMU_REG_LED_CFG_IO1, &val);
            val = (val | REG_LED_IO1_PU | REG_LED_IO1_OENB_PRE) & ~(REG_LED_IO1_PUEN | REG_LED_IO1_PDEN | REG_LED_IO1_RX_EN);
            pmu_write(PMU_REG_LED_CFG_IO1, val);
        } else {
            pmu_read(PMU_REG_LED_CFG_IO2, &val);
            val = (val | REG_LED_IO2_PU | REG_LED_IO2_OENB_PRE) & ~(REG_LED_IO2_PUEN | REG_LED_IO2_PDEN | REG_LED_IO2_RX_EN);
            pmu_write(PMU_REG_LED_CFG_IO2, val);
        }
        int_unlock(lock);
    }
}

void pmu_led_set_direction(enum HAL_GPIO_PIN_T pin, enum HAL_GPIO_DIR_T dir)
{
    uint16_t val;
    uint32_t lock;

    if (pin == HAL_GPIO_PIN_LED1 || pin == HAL_GPIO_PIN_LED2) {
        lock = int_lock();
        if (pin == HAL_GPIO_PIN_LED1) {
            pmu_read(PMU_REG_LED_CFG_IO1, &val);
            val |= REG_LED_IO1_PU;
            if (dir == HAL_GPIO_DIR_IN) {
                val |= REG_LED_IO1_OENB_PRE | REG_LED_IO1_RX_EN;
            } else {
                val &= ~(REG_LED_IO1_OENB_PRE | REG_LED_IO1_RX_EN);
            }
            pmu_write(PMU_REG_LED_CFG_IO1, val);
        } else {
            pmu_read(PMU_REG_LED_CFG_IO2, &val);
            val |= REG_LED_IO2_PU;
            if (dir == HAL_GPIO_DIR_IN) {
                val |= REG_LED_IO2_OENB_PRE | REG_LED_IO2_RX_EN;
            } else {
                val &= ~(REG_LED_IO2_OENB_PRE | REG_LED_IO2_RX_EN);
            }
            pmu_write(PMU_REG_LED_CFG_IO2, val);
        }
        int_unlock(lock);
    }
}

enum HAL_GPIO_DIR_T pmu_led_get_direction(enum HAL_GPIO_PIN_T pin)
{
    uint16_t val;

    if (pin == HAL_GPIO_PIN_LED1 || pin == HAL_GPIO_PIN_LED2) {
        if (pin == HAL_GPIO_PIN_LED1) {
            pmu_read(PMU_REG_LED_CFG_IO1, &val);
            return (val & REG_LED_IO1_OENB_PRE) ? HAL_GPIO_DIR_IN : HAL_GPIO_DIR_OUT;
        } else {
            pmu_read(PMU_REG_LED_CFG_IO2, &val);
            return (val & REG_LED_IO2_OENB_PRE) ? HAL_GPIO_DIR_IN : HAL_GPIO_DIR_OUT;
        }
    } else {
        return HAL_GPIO_DIR_IN;
    }
}

void pmu_led_set_voltage_domains(enum HAL_IOMUX_PIN_T pin, enum HAL_IOMUX_PIN_VOLTAGE_DOMAINS_T volt)
{
    enum PMU_LED_VOLT_T {
        PMU_LED_VOLT_VBAT,
        PMU_LED_VOLT_VMEM,
        PMU_LED_VOLT_VIO,
    };
    enum PMU_LED_VOLT_T sel;
    uint16_t val;
    uint32_t lock;

    if (pin == HAL_IOMUX_PIN_LED1 || pin == HAL_IOMUX_PIN_LED2) {
        if (volt == HAL_IOMUX_PIN_VOLTAGE_VIO) {
            sel = PMU_LED_VOLT_VIO;
        } else if (volt == HAL_IOMUX_PIN_VOLTAGE_MEM) {
            sel = PMU_LED_VOLT_VMEM;
        } else {
            sel = PMU_LED_VOLT_VBAT;
        }

        lock = int_lock();
        if (pin == HAL_IOMUX_PIN_LED1) {
            pmu_read(PMU_REG_LED_CFG_IO1, &val);
            val = SET_BITFIELD(val, REG_LED_IO1_SEL, sel);
            pmu_write(PMU_REG_LED_CFG_IO1, val);
        } else {
            pmu_read(PMU_REG_LED_CFG_IO2, &val);
            val = SET_BITFIELD(val, REG_LED_IO2_SEL, sel);
            pmu_write(PMU_REG_LED_CFG_IO2, val);
        }
        int_unlock(lock);
    }
}

void pmu_led_set_pull_select(enum HAL_IOMUX_PIN_T pin, enum HAL_IOMUX_PIN_PULL_SELECT_T pull_sel)
{
    uint16_t val;
    uint32_t lock;

    if (pin == HAL_IOMUX_PIN_LED1 || pin == HAL_IOMUX_PIN_LED2) {
        lock = int_lock();
        if (pin == HAL_IOMUX_PIN_LED1) {
            pmu_read(PMU_REG_LED_CFG_IO1, &val);
            val &= ~(REG_LED_IO1_PDEN | REG_LED_IO1_PUEN);
            if (pull_sel == HAL_IOMUX_PIN_PULLUP_ENABLE) {
                val |= REG_LED_IO1_PUEN;
            } else if (pull_sel == HAL_IOMUX_PIN_PULLDOWN_ENABLE) {
                val |= REG_LED_IO1_PDEN;
            }
            pmu_write(PMU_REG_LED_CFG_IO1, val);
        } else {
            pmu_read(PMU_REG_LED_CFG_IO2, &val);
            val &= ~(REG_LED_IO2_PDEN | REG_LED_IO2_PUEN);
            if (pull_sel == HAL_IOMUX_PIN_PULLUP_ENABLE) {
                val |= REG_LED_IO2_PUEN;
            } else if (pull_sel == HAL_IOMUX_PIN_PULLDOWN_ENABLE) {
                val |= REG_LED_IO2_PDEN;
            }
            pmu_write(PMU_REG_LED_CFG_IO2, val);
        }
        int_unlock(lock);
    }
}

void pmu_led_set_value(enum HAL_GPIO_PIN_T pin, int data)
{
    uint16_t val;
    uint32_t lock;
#ifdef PMU_LED_VIA_PWM
    uint16_t br_val;
#endif
    if (pin == HAL_GPIO_PIN_LED1 || pin == HAL_GPIO_PIN_LED2) {
        lock = int_lock();
        if (pin == HAL_GPIO_PIN_LED1) {
#ifdef PMU_LED_VIA_PWM
            pmu_write(PMU_REG_PWM2_TOGGLE, 0xFFFF);
            pmu_read(PMU_REG_PWM2_BR_EN, &br_val);
            br_val &= ~REG_PWM2_BR_EN;
            val |= PWM_SELECT_EN;
            if (data) {
                val &= ~PWM_SELECT_INV;
            } else {
                val |= PWM_SELECT_INV;
            }
            pmu_write(PMU_REG_PWM2_BR_EN, br_val);
#else
            pmu_read(PMU_REG_PWM2_EN, &val);
            if (data) {
                val |= REG_LED0_OUT;
            } else {
                val &= ~REG_LED0_OUT;
            }
            pmu_write(PMU_REG_PWM2_EN, val);
#endif
        }else {
#ifdef PMU_LED_VIA_PWM
            pmu_write(PMU_REG_PWMB_TOGGLE, 0xFFFF);
            pmu_read(PMU_REG_PWMB_BR_EN, &br_val);
            br_val &= ~REG_PWMB_BR_EN;
            val |= PWMB_SELECT_EN;
            if (data) {
                val &= ~PWMB_SELECT_INV;
            } else {
                val |= PWMB_SELECT_INV;
            }
            pmu_write(PMU_REG_PWMB_BR_EN, br_val);
#else
            pmu_read(PMU_REG_PWMB_EN, &val);
            if (data) {
                val |= REG_LEDB_OUT;
            } else {
                val &= ~REG_LEDB_OUT;
            }
            pmu_write(PMU_REG_PWMB_EN, val);
#endif
        }
        int_unlock(lock);
    }
}

int pmu_led_get_value(enum HAL_GPIO_PIN_T pin)
{
    uint16_t val;
    int data = 0;

    if (pin == HAL_GPIO_PIN_LED1 || pin == HAL_GPIO_PIN_LED2) {
        pmu_read(PMU_REG_LED_IO_IN, &val);
        if (pin == HAL_GPIO_PIN_LED1) {
            data = LED_IO1_IN_DB;
        } else {
            data = LED_IO2_IN_DB;
        }
        data &= val;
    }

    return !!data;
}

void pmu_led_uart_enable(enum HAL_IOMUX_PIN_T pin)
{
    uint16_t val;
    if (pin == HAL_IOMUX_PIN_LED1) {
        pmu_read(PMU_REG_UART1_CFG, &val);
        val &= ~(REG_UART_LEDA_SEL | REG_PMU_UART_DR1);
        val |=  REG_GPIO_I_SEL;
        pmu_write(PMU_REG_UART1_CFG, val);

        pmu_read(PMU_REG_UART2_CFG, &val);
        val |= REG_PMU_UART_DR2;
        pmu_write(PMU_REG_UART2_CFG, val);

        pmu_read(PMU_REG_LED_CFG_IO1, &val);
        val |= REG_LED_IO1_PUEN;
        pmu_write(PMU_REG_LED_CFG_IO1, val);
    } else {
        pmu_read(PMU_REG_UART1_CFG, &val);
        val &= ~ REG_GPIO_I_SEL;
        val |= REG_PMU_UART_DR1;
        pmu_write(PMU_REG_UART1_CFG, val);

        pmu_read(PMU_REG_UART2_CFG, &val);
        val &= ~(REG_UART_LEDB_SEL | REG_PMU_UART_DR2);
        pmu_write(PMU_REG_UART2_CFG, val);

        pmu_read(PMU_REG_LED_CFG_IO2, &val);
        val |= REG_LED_IO2_PUEN;
        pmu_write(PMU_REG_LED_CFG_IO2, val);
    }
}

void pmu_led_uart_disable(enum HAL_IOMUX_PIN_T pin)
{
    uint16_t val;
    if (pin == HAL_IOMUX_PIN_LED1) {
        pmu_read(PMU_REG_UART1_CFG, &val);
        val |= (REG_UART_LEDA_SEL | REG_PMU_UART_DR1);
        val &=  ~REG_GPIO_I_SEL;
        pmu_write(PMU_REG_UART1_CFG, val);

        pmu_read(PMU_REG_UART2_CFG, &val);
        val &= ~REG_PMU_UART_DR2;
        pmu_write(PMU_REG_UART2_CFG, val);

        pmu_read(PMU_REG_LED_CFG_IO1, &val);
        val &= ~REG_LED_IO1_PUEN;
        pmu_write(PMU_REG_LED_CFG_IO1, val);
    } else {
        pmu_read(PMU_REG_UART1_CFG, &val);
        val |= REG_GPIO_I_SEL;
        val &= ~REG_PMU_UART_DR1;
        pmu_write(PMU_REG_UART1_CFG, val);

        pmu_read(PMU_REG_UART2_CFG, &val);
        val |= (REG_UART_LEDB_SEL | REG_PMU_UART_DR2);
        pmu_write(PMU_REG_UART2_CFG, val);

        pmu_read(PMU_REG_LED_CFG_IO2, &val);
        val &= ~REG_LED_IO2_PUEN;
        pmu_write(PMU_REG_LED_CFG_IO2, val);
    }
}

void pmu_led_breathing_enable(enum HAL_IOMUX_PIN_T pin, const struct PMU_LED_BR_CFG_T *cfg)
{
    uint32_t st1;
    uint32_t st2;
    uint32_t subcnt_data;
    uint8_t tg;
    uint16_t val;
    uint32_t lock;
    if (pin == HAL_IOMUX_PIN_LED1 || pin == HAL_IOMUX_PIN_LED2) {
        st1 = MS_TO_TICKS(cfg->off_time_ms);
        if (st1 > 0xFFFF) {
            st1 = 0xFFFF;
        }
        st2 = MS_TO_TICKS(cfg->on_time_ms);
        if (st2 > 0xFFFF) {
            st2 = 0xFFFF;
        }
        subcnt_data = MS_TO_TICKS(cfg->fade_time_ms);
        subcnt_data = integer_sqrt_nearest(subcnt_data);
        if (subcnt_data > (SUBCNT_DATA2_MASK >> SUBCNT_DATA2_SHIFT)) {
            subcnt_data = (SUBCNT_DATA2_MASK >> SUBCNT_DATA2_SHIFT);
        }
        // TODO: Keep compatible with digital PWM module (can be removed after 2500)
        if (subcnt_data > 0xFE) {
            subcnt_data = 0xFE;
        }
        tg = 1;

        pmu_led_set_direction((enum HAL_GPIO_PIN_T)pin, HAL_GPIO_DIR_OUT);
        pmu_led_set_pull_select(pin, HAL_IOMUX_PIN_NOPULL);
        pmu_led_set_voltage_domains(pin, HAL_IOMUX_PIN_VOLTAGE_VBAT);

        lock = int_lock();
        if (pin == HAL_IOMUX_PIN_LED1) {
            pmu_write(PMU_REG_PWM2_TOGGLE, st2);
            pmu_write(PMU_REG_PWM2_ST1, st1);
            val = SUBCNT_DATA2(subcnt_data) | TG_SUBCNT_D2_ST(tg);
            pmu_write(PMU_REG_PWM2_EN, val);
            pmu_read(PMU_REG_PWM2_BR_EN, &val);
            val = (val & ~REG_CLK_PWM_DIV_MASK) | REG_CLK_PWM_DIV(0) |
                REG_PWM_CLK_EN | REG_PWM2_BR_EN | PWM_SELECT_EN;
            pmu_write(PMU_REG_PWM2_BR_EN, val);
        } else {
            pmu_write(PMU_REG_PWMB_TOGGLE, st2);
            pmu_write(PMU_REG_PWMB_ST1, st1);
            val = SUBCNT_DATAB(subcnt_data) | TG_SUBCNT_DB_ST(tg);
            pmu_write(PMU_REG_PWMB_EN, val);
            pmu_read(PMU_REG_PWMB_BR_EN, &val);
            val = (val & ~REG_CLK_PWMB_DIV_MASK) | REG_CLK_PWMB_DIV(0) |
                REG_PWMB_CLK_EN | REG_PWMB_BR_EN | PWMB_SELECT_EN;
            pmu_write(PMU_REG_PWMB_BR_EN, val);
        }
        int_unlock(lock);
    }
}

void pmu_led_breathing_disable(enum HAL_IOMUX_PIN_T pin)
{
    uint16_t val;
    uint32_t lock;
    if (pin == HAL_IOMUX_PIN_LED1 || pin == HAL_IOMUX_PIN_LED2) {
        lock = int_lock();
        if (pin == HAL_IOMUX_PIN_LED1) {
            pmu_read(PMU_REG_PWM2_BR_EN, &val);
            val &= ~(REG_PWM_CLK_EN | REG_PWM2_BR_EN | PWM_SELECT_EN);
            pmu_write(PMU_REG_PWM2_BR_EN, val);
        } else {
            pmu_read(PMU_REG_PWMB_BR_EN, &val);
            val &= ~(REG_PWMB_CLK_EN | REG_PWMB_BR_EN | PWMB_SELECT_EN);
            pmu_write(PMU_REG_PWMB_BR_EN, val);
        }
        int_unlock(lock);
        pmu_led_set_direction((enum HAL_GPIO_PIN_T)pin, HAL_GPIO_DIR_IN);
        pmu_led_set_pull_select(pin, HAL_IOMUX_PIN_PULLUP_ENABLE);
    }
}

#ifdef PMU_IRQ_UNIFIED
static void pmu_gpio_irq_handler(uint16_t irq_status)
{
    if (irq_status & PMU_GPIO_INTR_MSKED1) {
        if (gpio_irq_handler[0]) {
            gpio_irq_handler[0](HAL_GPIO_PIN_LED1);
        }
    }
    if (irq_status & PMU_GPIO_INTR_MSKED2) {
        if (gpio_irq_handler[1]) {
            gpio_irq_handler[1](HAL_GPIO_PIN_LED2);
        }
    }
}

uint8_t pmu_gpio_setup_irq(enum HAL_GPIO_PIN_T pin, const struct HAL_GPIO_IRQ_CFG_T *cfg)
{
    uint32_t lock;
    uint16_t val;
    bool old_en;

    if (pin != HAL_GPIO_PIN_LED1 && pin != HAL_GPIO_PIN_LED2) {
        return 1;
    }

    lock = int_lock();

    old_en = (gpio_irq_en[0] || gpio_irq_en[1]);

    if (pin == HAL_GPIO_PIN_LED1) {
        gpio_irq_en[0] = cfg->irq_enable;
        gpio_irq_handler[0] = cfg->irq_handler;

        if (cfg->irq_enable) {
        }

        pmu_read(PMU_REG_UART1_CFG, &val);
        if (cfg->irq_enable) {
            val |= REG_PMU_GPIO_INTR_MSK1 | REG_PMU_GPIO_INTR_EN1;
            if (cfg->irq_debounce) {
                val &= ~PMU_DB_BYPASS1;
            } else {
                val |= PMU_DB_BYPASS1;
            }
        } else {
            val &= ~(REG_PMU_GPIO_INTR_MSK1 | REG_PMU_GPIO_INTR_EN1);
        }
        pmu_write(PMU_REG_UART1_CFG, val);
    } else {
        gpio_irq_en[1] = cfg->irq_enable;
        gpio_irq_handler[1] = cfg->irq_handler;

        pmu_read(PMU_REG_UART2_CFG, &val);
        if (cfg->irq_enable) {
            val |= REG_PMU_GPIO_INTR_MSK2 | REG_PMU_GPIO_INTR_EN2;
            if (cfg->irq_debounce) {
                val &= ~PMU_DB_BYPASS2;
            } else {
                val |= PMU_DB_BYPASS2;
            }
        } else {
            val &= ~(REG_PMU_GPIO_INTR_MSK1 | REG_PMU_GPIO_INTR_EN1);
        }
        pmu_write(PMU_REG_UART2_CFG, val);
    }

    if (cfg->irq_enable) {
        uint16_t type;
        uint16_t pol;

        type = (pin == HAL_GPIO_PIN_LED1) ? REG_EDGE_INTR_SEL1 : REG_EDGE_INTR_SEL2;
        pol = (pin == HAL_GPIO_PIN_LED1) ? REG_POS_INTR_SEL1 : REG_POS_INTR_SEL2;
        pmu_read(PMU_REG_MIC_BIAS_C, &val);
        if (cfg->irq_type == HAL_GPIO_IRQ_TYPE_EDGE_SENSITIVE) {
            val |= type;
            if (cfg->irq_polarity == HAL_GPIO_IRQ_POLARITY_LOW_FALLING) {
                val &= ~pol;
            } else {
                val |= pol;
            }
        } else {
            val &= ~type;
        }
        pmu_write(PMU_REG_MIC_BIAS_C, val);

        if (cfg->irq_type != HAL_GPIO_IRQ_TYPE_EDGE_SENSITIVE) {
            if (pin == HAL_GPIO_PIN_LED1) {
                pmu_read(PMU_REG_WDT_INT_CFG, &val);
                if (cfg->irq_polarity == HAL_GPIO_IRQ_POLARITY_LOW_FALLING) {
                    val |= REG_LOW_LEVEL_INTR_SEL1;
                } else {
                    val &= ~REG_LOW_LEVEL_INTR_SEL1;
                }
                pmu_write(PMU_REG_WDT_INT_CFG, val);
            } else {
                pmu_read(PMU_REG_UART2_CFG, &val);
                if (cfg->irq_polarity == HAL_GPIO_IRQ_POLARITY_LOW_FALLING) {
                    val |= REG_LOW_LEVEL_INTR_SEL2;
                } else {
                    val &= ~REG_LOW_LEVEL_INTR_SEL2;
                }
                pmu_write(PMU_REG_UART2_CFG, val);
            }
        }
    }

    if (old_en != cfg->irq_enable) {
        pmu_set_irq_unified_handler(PMU_IRQ_TYPE_GPIO, cfg->irq_enable ? pmu_gpio_irq_handler : NULL);
    }

    int_unlock(lock);

    return 0;
}

static void pmu_wdt_irq_handler(uint16_t irq_status)
{
    if (wdt_irq_handler) {
        wdt_irq_handler();
    }
}

void pmu_wdt_set_irq_handler(PMU_WDT_IRQ_HANDLER_T handler)
{
    uint32_t lock;
    uint16_t val;

    lock = int_lock();

    wdt_irq_handler = handler;

    pmu_read(PMU_REG_WDT_INT_CFG, &val);
    if (handler) {
        val |= REG_WDT_INTR_EN | REG_WDT_INTR_MSK;
    } else {
        val &= ~(REG_WDT_INTR_EN | REG_WDT_INTR_MSK);
    }
    pmu_write(PMU_REG_WDT_INT_CFG, val);

    pmu_set_irq_unified_handler(PMU_IRQ_TYPE_WDT, handler ? pmu_wdt_irq_handler : NULL);

    int_unlock(lock);
}
#else
uint8_t pmu_gpio_setup_irq(enum HAL_GPIO_PIN_T pin, const struct HAL_GPIO_IRQ_CFG_T *cfg)
{
    ASSERT(false, "PMU_IRQ_UNIFIED must defined to use PMU GPIO IRQ");
    return 1;
}

void pmu_wdt_set_irq_handler(PMU_WDT_IRQ_HANDLER_T handler)
{
}
#endif

#ifdef __WATCHER_DOG_RESET__
struct PMU_WDT_CTX_T {
    bool enabled;
    uint16_t wdt_irq_timer;
    uint16_t wdt_reset_timer;
    uint16_t wdt_cfg;
};

static struct PMU_WDT_CTX_T BOOT_BSS_LOC wdt_ctx;

void BOOT_TEXT_SRAM_LOC pmu_wdt_save_context(void)
{
    uint16_t wdt_cfg = 0, timer = 0;
    pmu_read(PMU_REG_WDT_CFG, &wdt_cfg);
    if (wdt_cfg & (REG_WDT_RESET_EN | REG_WDT_EN)){
        wdt_ctx.enabled = true;
        wdt_ctx.wdt_cfg = wdt_cfg;
        pmu_read(PMU_REG_WDT_IRQ_TIMER, &timer);
        wdt_ctx.wdt_irq_timer = timer;
        pmu_read(PMU_REG_WDT_RESET_TIMER, &timer);
        wdt_ctx.wdt_reset_timer = timer;
    }
}

void BOOT_TEXT_SRAM_LOC pmu_wdt_restore_context(void)
{
    if (wdt_ctx.enabled) {
        pmu_write(PMU_REG_WDT_IRQ_TIMER, wdt_ctx.wdt_irq_timer);
        pmu_write(PMU_REG_WDT_RESET_TIMER, wdt_ctx.wdt_reset_timer);
        pmu_write(PMU_REG_WDT_CFG, wdt_ctx.wdt_cfg);
    }
}
#endif

int pmu_wdt_config(uint32_t irq_ms, uint32_t reset_ms)
{
    if (irq_ms > 0xFFFF) {
        return 1;
    }
    if (reset_ms > 0xFFFF) {
        return 1;
    }
    wdt_irq_timer = irq_ms;
    wdt_reset_timer = reset_ms;

    pmu_write(PMU_REG_WDT_IRQ_TIMER, wdt_irq_timer);
    pmu_write(PMU_REG_WDT_RESET_TIMER, wdt_reset_timer);

    return 0;
}

void pmu_wdt_start(void)
{
    uint16_t val;

    if (wdt_irq_timer == 0 && wdt_reset_timer == 0) {
        return;
    }

    pmu_read(PMU_REG_WDT_CFG, &val);
    val |= (REG_WDT_RESET_EN | REG_WDT_EN);
    pmu_write(PMU_REG_WDT_CFG, val);
}

#ifndef __WATCHER_DOG_RESET__
BOOT_TEXT_SRAM_LOC
#endif
void pmu_wdt_stop(void)
{
    uint16_t val;

    pmu_read(PMU_REG_WDT_CFG, &val);
    val &= ~(REG_WDT_RESET_EN | REG_WDT_EN);
    pmu_write(PMU_REG_WDT_CFG, val);

    pmu_read(PMU_REG_LED_IO_IN, &val);
    val |= REG_WDT_INTR_CLR;
    pmu_write(PMU_REG_LED_IO_IN, val);
}

void pmu_wdt_feed(void)
{
    if (wdt_irq_timer == 0 && wdt_reset_timer == 0) {
        return;
    }

    pmu_write(PMU_REG_WDT_IRQ_TIMER, wdt_irq_timer);
    pmu_write(PMU_REG_WDT_RESET_TIMER, wdt_reset_timer);
}

void pmu_ntc_capture_enable(void)
{
}

void pmu_ntc_capture_disable(void)
{
}

void pmu_bt_reconn(bool en)
{
    uint16_t pmu_val = 0;
    // Force big bandgap
    pmu_read(PMU_REG_BIAS_CFG, &pmu_val);
    //TRACE(1,"origin: PMU_REG_BIAS_CFG=0x%x", pmu_val);
    if(en){
        pmu_val |= BG_CONSTANT_GM_BIAS_DR | BG_CONSTANT_GM_BIAS_REG;
        pmu_val |= BG_CORE_EN_DR | BG_CORE_EN_REG;
        pmu_write(PMU_REG_BIAS_CFG, pmu_val);
        hal_sys_timer_delay_us(20);
        pmu_val |= BG_VBG_SEL_DR | BG_VBG_SEL_REG;
        pmu_write(PMU_REG_BIAS_CFG, pmu_val);
    } else{
        pmu_val &= ~(BG_VBG_SEL_DR | BG_VBG_SEL_REG);
        pmu_write(PMU_REG_BIAS_CFG, pmu_val);
        hal_sys_timer_delay_us(20);
        pmu_val &= ~(BG_CONSTANT_GM_BIAS_DR | BG_CONSTANT_GM_BIAS_REG);
        pmu_val &= ~(BG_CORE_EN_DR | BG_CORE_EN_REG);
        pmu_write(PMU_REG_BIAS_CFG, pmu_val);
    }

    pmu_read(0x38, &pmu_val);
    //TRACE(1,"origin: PMU_38=0x%x", pmu_val);
    pmu_val &= 0xfff0;      //clear [3:0]

    if(en){
        pmu_val |= 0x1;
    }
    pmu_write(0x38, pmu_val);

    //pmu_read(0x38, &pmu_val);
    //TRACE(1,"final: PMU_38=0x%x", pmu_val);
}


void check_efuse_for_different_chip(void)
{
    uint16_t flag;
    pmu_get_efuse(PMU_EFUSE_PAGE_DCCALIB2_L, &flag);
    if(flag & (1<<11))
    {
        check_efuse_dccalib2_l_11 = 1;
    }else
    {
        //other 1305  vcore=0.85v VCODEC=1.7v
        check_efuse_dccalib2_l_11 = 0;
        dig_lp_ldo = dig_lp_ldo_efuse11_is_0;
        dig_lp_dcdc = dig_lp_dcdc_efuse11_is_0;

        vcodec_mv =  vcodec_mv_efuse11_is_0;
        if(true == vcodec_off)
        {
            vhppa_mv = vcodec_mv;
        }else
        {
            vhppa_mv = vhppa_mv_efuse11_is_0;
        }
    }
}

#ifdef USE_POWER_KEY_RESET
//Long press the power key to reset the chip
//seconds:0~60s
void pmu_power_key_hw_reset_enable(uint8_t seconds)
{
    uint16_t val;
    uint32_t lock;

#ifdef SIMU
    pmu_write(PMU_REG_DIV_HW_RESET_CFG, 32 - 2);
#else
    pmu_write(PMU_REG_DIV_HW_RESET_CFG, CONFIG_SYSTICK_HZ * 2 - 2);
#endif

    lock = int_lock();
    pmu_read(PMU_REG_WDT_CFG, &val);
    val = SET_BITFIELD(val, REG_HW_RESET_TIME, seconds) | REG_HW_RESET_EN;
    pmu_write(PMU_REG_WDT_CFG, val);
    int_unlock(lock);
}
#endif