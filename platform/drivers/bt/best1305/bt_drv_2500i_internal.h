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
#ifndef __BT_DRV_2500I_INTERNAL_H__
#define __BT_DRV_2500I_INTERNAL_H__

#define __HIGH_TX_POWER__       //BR 14dbm EDR 11dbm by luobin

//#define __RECONN_CONFIG_FOR_TDD_NOISE__
#define BT_IF_1P05M
/***************************************************************************
 *Patch entry registers
 ****************************************************************************/
///enable m33 patch func
#define BTDRV_PATCH_EN_REG                  0xa01ffe00

//instruction patch compare src address
#define BTDRV_PATCH_INS_COMP_ADDR_START     0xa01ffe00

#define BTDRV_PATCH_INS_REMAP_ADDR_START    0xa01fff00

#define BT_PATCH_WR(addr, value)         (*(volatile uint32_t *)(addr)) = (value)
/***************************************************************************
 *RF conig macro
 ****************************************************************************/
#define __FANG_HW_AGC_CFG__
#define __FANG_LNA_CFG__
//#define __PWR_FLATNESS__

#ifdef __HW_AGC__
#define __HW_AGC_I2V_DISABLE_DC_CAL__
#endif

//#define ACCEPT_NEW_MOBILE_EN
#define  __1302_8_AGC__

//#define __SNR_WEIGHTING_MODE__

//#define BT_RF_OLD_CORR_MODE
/***************************************************************************
 *BT clock gate disable
 ****************************************************************************/
#define __CLK_GATE_DISABLE__

/***************************************************************************
 *BT read controller rssi
 ****************************************************************************/
#define  BT_RSSI_MONITOR  1

/***************************************************************************
 *PCM config macro
 ****************************************************************************/
//#define APB_PCM
//#define SW_INSERT_MSBC_TX_OFFSET

/***************************************************************************
 *BT afh follow function
 ****************************************************************************/
#define  BT_AFH_FOLLOW  0

//#define IBRT_DUAL_ANT_CTRL
#define BLE_IDX_IN_TESTMODE 3

#define BT_CMU_26M_READY_TIMEOUT_US    (2500)
#define BT_CMU_OSC_READY_TIMEOUT_US    (3000)
#define TWOSC_TIMEOUT_US_HI ((BT_CMU_OSC_READY_TIMEOUT_US&0xFF00)>>8)
#define TWOSC_TIMEOUT_US_LO (BT_CMU_OSC_READY_TIMEOUT_US&0xFF)
#ifdef __cplusplus
extern "C" {
#endif
#define TRC_KE_MSG_TYPE              0x00000001
#define TRC_KE_TMR_TYPE              0x00000002
#define TRC_KE_EVT_TYPE              0x00000004
#define TRC_MEM_TYPE                 0x00000008
#define TRC_SLEEP_TYPE               0x00000010
#define TRC_SW_ASS_TYPE              0x00000020
#define TRC_PROG_TYPE                0x00000040
#define TRC_CS_BLE_TYPE              0x00000080
#define TRC_CS_BT_TYPE               0x00000100
#define TRC_RX_DESC_TYPE             0x00000200
#define TRC_LLCP_TYPE                0x00000400
#define TRC_LMP_TYPE                 0x00000800
#define TRC_L2CAP_TYPE               0x00001000
#define TRC_ARB_TYPE                 0x00002000
#define TRC_LLC_STATE_TRANS_TYPE     0x00004000
#define TRC_KE_STATE_TRANS_TYPE      0x00008000
#define TRC_HCI_TYPE                 0x00010000
#define TRC_ADV_TYPE                 0x00020000
#define TRC_ACL_TYPE                 0x00040000
#define TRC_CUSTOM_TYPE              0x00080000
#define TRC_IBRT_TYPE                0x00100000
#define TRC_ALL_TYPE                 0xffffffff
#define TRC_ALL_EXCEP_CS_BT_TYPE     0xfffffeff
#define TRC_DEFAULT_TYPE             (TRC_SW_ASS_TYPE|TRC_LMP_TYPE|TRC_LLC_STATE_TRANS_TYPE|TRC_KE_STATE_TRANS_TYPE|TRC_IBRT_TYPE)

#define FAST_LOCK_ENABLE 1
#define FAST_LOCK_DISABLE 0

#define SWAGC_INIT_MODE   0
#define NEW_SYNC_SWAGC_MODE   1
#define OLD_SWAGC_MODE   2

#define RSSI_ADJUST_INIT_MODE   0
#define RSSI_ADJUST_WORK_MODE   1
#define RSSI_ADJUST_IDLE_MODE   2
#define RSSI_ADJUST_TEST_MODE   3


#if (__FASTACK_ECC_CONFIG_BLOCK__ == 1)    // 1 BLOCK
    #define ECC_MODU_MODE ECC_DPSK
    #define ECC_BLK_MODE ECC_1BLOCK
#elif (__FASTACK_ECC_CONFIG_BLOCK__ == 2)    // 2BLOCK
    #define ECC_MODU_MODE ECC_DPSK
    #define ECC_BLK_MODE ECC_2BLOCK
#elif (__FASTACK_ECC_CONFIG_BLOCK__ == 3)    // 3BLOCK
    #define ECC_MODU_MODE ECC_8PSK
    #define ECC_BLK_MODE ECC_3BLOCK
#endif


#define  INVALID_SYNC_WORD  (0)

#define BT_FASTACK_ADDR   0xD0220468
#define BT_SCO_TRIGGER_BYPASS_ADDR   0xD022046C
#define BT_ALWAYS_ACK_ADDR   0xD0220464

#define BLE_RXGAIN_THS_TBL_ADDR   0xc0000490

//#define __FIX_FA_RX_GAIN___

#define FA_RX_WIN_SIZE      (0x8)
#define FA_FIX_TX_GIAN_IDX  (0x4)
#define FA_FIX_RX_GIAN_IDX  (0x1)
#define FA_NEW_CORR_VALUE   (0x40)
//lager than 2 will causes error synchronization
#define FA_OLD_CORR_VALUE   (0x4)

#define CMD_FILTER_LEN              (10)
#define EVT_FILTER_LEN              (10)

#define BT_FA_CTRL_ADDR   (0xD0220474)

#ifdef BT_FAST_LOCK_ENABLE
#define SPI0_EN        (1)
#define SPI0_TXON_NUM  (3)
#define SPI0_RXON_NUM  (3)
#define SPI0_TXON_POS  (6)
#define SPI0_RXON_POS  (36)
#else
#define SPI0_EN        (0)
#define SPI0_TXON_NUM  (0)
#define SPI0_RXON_NUM  (0)
#define SPI0_TXON_POS  (0)
#define SPI0_RXON_POS  (0)
#endif

enum{
    ECC_8PSK = 0,
    ECC_DPSK = 1,
    ECC_GFSK = 2,
    ECC_MODU_MAX,
};

enum{
    ECC_1BLOCK = 0,
    ECC_2BLOCK = 1,
    ECC_3BLOCK = 2,
    ECC_BLOCK_MAX,
};

enum{
    FA_SYNCWORD_32BIT = 0,
    FA_SYNCWORD_64BIT = 1,
    FA_SYNCWORD_MAX,
};

enum{
    ECC_8_BYTE_MODE = 0,
    ECC_16_BYTE_MODE = 1,
    ECC_MAX_BYTE_MODE,
};

enum{
    ECC_DISABLE_PKT_TYPE_2DH3   = 0,
    ECC_DISABLE_PKT_TYPE_DM3    = 1,
    ECC_DISABLE_PKT_TYPE_3DH3   = 2,
    ECC_DISABLE_PKT_TYPE_DH3    = 3,
    ECC_DISABLE_PKT_TYPE_2DH5   = 4,
    ECC_DISABLE_PKT_TYPE_DM5    = 5,
    ECC_DISABLE_PKT_TYPE_3DH5   = 6,
    ECC_DISABLE_PKT_TYPE_DH5    = 7,
    ECC_DISABLE_PKT_TYPE_3DH1   = 8,
    ECC_DISABLE_PKT_TYPE_2DH1   = 9,
    ECC_DISABLE_PKT_TYPE_DH1    = 10,
    ECC_DISABLE_PKT_TYPE_DM1    = 11,
    ECC_DISABLE_PKT_TYPE_2EV3   = 12,
    ECC_DISABLE_PKT_TYPE_3EV3   = 13,
    ECC_DISABLE_PKT_TYPE_EV3    = 14,
    ECC_DISABLE_PKT_TYPE_2EV5   = 15,
    ECC_DISABLE_PKT_TYPE_EV4    = 16,
    ECC_DISABLE_PKT_TYPE_3EV5   = 17,
    ECC_DISABLE_PKT_TYPE_EV5    = 18,
    ECC_DISABLE_PKT_TYPE_Others = 19,
    ECC_DISABLE_PKT_TYPE_MAX,
};

enum
{
    METRIC_TYPE_TX_DM1_COUNTS = 0,
    METRIC_TYPE_TX_DH1_COUNTS,//1
    METRIC_TYPE_TX_DM3_COUNTS,//2
    METRIC_TYPE_TX_DH3_COUNTS,//3
    METRIC_TYPE_TX_DM5_COUNTS,//4
    METRIC_TYPE_TX_DH5_COUNTS,//5
    METRIC_TYPE_TX_2_DH1_COUNTS,//6
    METRIC_TYPE_TX_3_DH1_COUNTS,//7
    METRIC_TYPE_TX_2_DH3_COUNTS,//8
    METRIC_TYPE_TX_3_DH3_COUNTS,//9
    METRIC_TYPE_TX_2_DH5_COUNTS,//10
    METRIC_TYPE_TX_3_DH5_COUNTS,//11
    METRIC_TYPE_RX_DM1_COUNTS,//12
    METRIC_TYPE_RX_DH1_COUNTS,//13
    METRIC_TYPE_RX_DM3_COUNTS,//14
    METRIC_TYPE_RX_DH3_COUNTS,//15
    METRIC_TYPE_RX_DM5_COUNTS,//16
    METRIC_TYPE_RX_DH5_COUNTS,//17
    METRIC_TYPE_RX_2_DH1_COUNTS,//18
    METRIC_TYPE_RX_3_DH1_COUNTS,//19
    METRIC_TYPE_RX_2_DH3_COUNTS,//20
    METRIC_TYPE_RX_3_DH3_COUNTS,//21
    METRIC_TYPE_RX_2_DH5_COUNTS,//22
    METRIC_TYPE_RX_3_DH5_COUNTS,//23
    METRIC_TYPE_RX_HEC_ERROR_COUNTS,//24
    METRIC_TYPE_RX_CRC_ERROR_COUNTS,//25
    METRIC_TYPE_RX_FEC_ERROR_COUNTS,//26
    METRIC_TYPE_RX_GUARD_ERROR_COUNTS,//27
    METRIC_TYPE_RX_WRONGPKTFLAG_ERROR_COUNTS,//28
    METRIC_TYPE_RADIO_COUNTS,//29
    METRIC_TYPE_SLEEP_DURATION_COUNTS,//30
    METRIC_TYPE_RADIO_TX_SUCCESS_COUNTS,//31
    METRIC_TYPE_RADIO_TX_COUNTS,//32
    METRIC_TYPE_SOFTBIT_SUCCESS_COUNTS,//33
    METRIC_TYPE_SOFTBIT_COUNTS,//34
    METRIC_TYPE_MAX,
};
typedef struct
{
    bool fa_tx_gain_en;
    uint8_t fa_tx_gain_idx;
    bool fa_rx_gain_en;
    uint8_t fa_rx_gain_idx;
    bool fa_2m_mode;
    uint8_t fa_rx_winsz;
    uint8_t syncword_len;
    bool fa_vendor_syncword_en;
    uint32_t syncword_high;
    uint32_t syncword_low;
    bool is_new_corr;
    uint8_t new_mode_corr_thr;
    uint8_t old_mode_corr_thr;
    bool enhance_fa_mode_en;
}btdrv_fa_basic_config_t;

typedef struct
{
    bool ecc_mode_enable;
    uint8_t ecc_modu_mode_acl;
    uint8_t ecc_modu_mode_sco;
    uint8_t ecc_blk_mode;
    uint8_t ecc_len_mode_sel;
}btdrv_ecc_basic_config_t;

typedef struct
{
    uint32_t dat_arr[4];
}ecc_trx_dat_arr_t;


typedef struct
{
    ecc_trx_dat_arr_t trx_dat;
}ecc_trx_dat_t;


struct hci_acl_data_pkt
{
    uint16_t handle  :10;
    uint16_t ecc_flg : 1;
    uint16_t softbit_flg : 1;

    //0b00 First non-automatically-flushable packet of a higher layer message
    //(start of a non-automatically-flushable L2CAP PDU) from Host to Controller.
    //0b01 Continuing fragment of a higher layer message
    //0b10 First automatically flushable packet of a higher layer message (start of an automatically-flushable L2CAP PDU)
    //0b11 A complete L2CAP PDU. Automatically flushable.

    uint16_t pb_flag :2;
    uint16_t bc_flag :2;
    uint16_t data_load_len;
    uint8_t sdu_data[0];
}__attribute__((packed));

struct hci_dbg_set_sw_rssi_cmd
{
    bool sw_rssi_en;
    uint32_t link_agc_thd_mobile;
    uint32_t link_agc_thd_mobile_time;
    uint32_t link_agc_thd_tws;
    uint32_t link_agc_thd_tws_time;
    uint8_t rssi_mobile_step;
    uint8_t rssi_tws_step;
    int8_t rssi_min_value_mobile;
    int8_t rssi_min_value_tws;
};

void bt_drv_reg_op_ll_monitor(uint16_t connhdl, uint8_t metric_type, uint32_t* ret_val);
void btdrv_ecc_basic_config(btdrv_ecc_basic_config_t* p_ecc_basic_config);
void btdrv_ecc_disable_spec_pkt_type(uint32_t ptk_type); // 1 -> disable FA, ptk type enum in bt_drv_2500i_internal.h
void btdrv_ecc_config_usr_tx_dat_set(ecc_trx_dat_t* p_ecc_trx_dat);
void btdrv_ecc_config_usr_rx_dat_get(ecc_trx_dat_t* p_ecc_trx_dat);
void bt_drv_reg_op_set_rf_rx_hwgain_tbl(int8_t (*p_tbl)[3]);
void bt_drv_reg_op_set_rf_hwagc_rssi_correct_tbl(int8_t* p_tbl);
void btdrv_52m_sys_enable();
void btdrv_26m_sys_enable();
void btdrv_fa_corr_mode_setting(bool is_new_corr);
void btdrv_rf_rx_gain_adjust_req(uint32_t user, bool lowgain);
void bt_drv_reg_op_for_test_mode_disable(void);
void bt_drv_reg_op_config_controller_log(uint32_t log_mask, uint16_t* p_cmd_filter, uint8_t cmd_nb, uint8_t* p_evt_filter, uint8_t evt_nb);
void bt_drv_reg_op_set_controller_trace_curr_sw(uint32_t log_mask);
void btdrv_txpower_calib(void);
void btdrv_tx_pwr_cal_enhanced(void);

void btdrv_fa_config_tx_gain(bool ecc_tx_gain_en, uint8_t ecc_tx_gain_idx);
void btdrv_fa_config_rx_gain(bool ecc_rx_gain_en, uint8_t ecc_rx_gain_idx);
void bt_drv_reg_op_set_fa_rx_gain_idx(uint8_t rx_gain_idx);
void bt_drv_reg_op_config_cotroller_cmd_filter(uint16_t* p_cmd_filter);
void bt_drv_reg_op_config_cotroller_evt_filter(uint8_t* p_evt_filter);
void bt_drv_rf_set_bt_sync_agc_enable(bool enable);
void bt_drv_rf_set_ble_sync_agc_enable(bool enable);
void btdrv_set_spi_trig_pos_enable(void);
void btdrv_clear_spi_trig_pos_enable(void);
void BT_DRV_REG_OP_CLK_ENB(void);
void BT_DRV_REG_OP_CLK_DIB(void);
void bt_drv_reg_op_rssi_adjust_init(bool en);
void bt_drv_reg_op_rssi_adjust_mode(uint32_t mode);
void bt_drv_rf_i2v_check_enable(bool enable);
void bt_drv_reg_op_init_nosync_info(void);
void bt_drv_reg_op_fa_agc_init(void);
void btdrv_function_testmode_patch_init(void);
void btdrv_ins_patch_test_init(void);
void btdrv_config_init(void);
void bt_dccalib_set_value(void);
void btdrv_poweron(uint8_t en);
void bt_drv_bbpll_cal(void);

#ifdef __cplusplus
}
#endif

#endif
