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
#include <besbt_string.h>
#include "plat_types.h"
#include "tgt_hardware.h"
#include "hal_i2c.h"
#include "hal_uart.h"
#include "bt_drv.h"
#include "hal_timer.h"
#include "hal_chipid.h"
#include "bt_drv_2500i_internal.h"
#include "bt_drv_interface.h"
#include "bt_drv_reg_op.h"
#include "bt_drv_internal.h"

extern void btdrv_send_cmd(uint16_t opcode,uint8_t cmdlen,const uint8_t *param);
extern void btdrv_write_memory(uint8_t wr_type,uint32_t address,const uint8_t *value,uint8_t length);

typedef struct
{
    uint8_t is_act;
    uint16_t opcode;
    uint8_t parlen;
    const uint8_t *param;

} BTDRV_CFG_TBL_STRUCT;


#define BTDRV_CONFIG_ACTIVE   1
#define BTDRV_CONFIG_INACTIVE 0
#define BTDRV_INVALID_TRACE_LEVEL  0xFF
/*
[0][0] = 63, [0][1] = 0,[0][2] = (-80),           472d
[1][0] = 51, [2][1] = 0,[2][2] = (-80),          472b
[2][0] = 42, [4][1] = 0,[4][2] = (-75),           4722
[3][0] = 36, [6][1] = 0,[6][2] = (-55),           c712
[4][0] = 30, [8][1] = 0,[8][2] = (-40),           c802
[5][0] = 21,[10][1] = 0,[10][2] = 0x7f,         c102
[6][0] = 12,[11][1] = 0,[11][2] = 0x7f,       c142
[7][0] = 3,[13][1] = 0,[13][2] = 0x7f,        c1c2
[8][0] = -3,[14][1] = 0,[14][2] = 0x7f};      c0c2
*/
static uint8_t g_controller_trace_level = BTDRV_INVALID_TRACE_LEVEL;
static bool  g_lmp_trace_enable = false;
const int8_t btdrv_rf_env_2500i[]=
{
    0x01,0x00,  //rf api
    0x01,   //rf env
    185,     //rf length
    0x3,     //txpwr_max
    -1,    ///rssi high thr
    -2,   //rssi low thr
    -100,  //rssi interf thr
    0xf,  //rssi interf gain thr
    2,  //wakeup delay
    0xe, 0, //skew
    0xe8,0x3,    //ble agc inv thr
#ifdef __HW_AGC__
    0x1,//hw_sw_agc_flag
#else
    0x0,//
#endif
    0xff,//sw gain set
    0xff,    //sw gain set
    -85,//bt_inq_page_iscan_pscan_dbm
#ifdef FORCE_NOSIGNALINGMODE
    -70,//ble_scan_adv_dbm
#else
    0x7f,//ble_scan_adv_dbm
#endif
    1,      //sw gain reset factor
    1,    //bt sw gain cntl enable
    1,   //ble sw gain cntl en
    1,  //bt interfere  detector en
    0,  //ble interfere detector en

    -21,-27,-19,
    -18,-24,-13,
    -15,-21,-10,
    -12,-18,-10,
    -9,-15,-10,
    -6,-12,-10,
    -3,-9,-10,
    0,-6,0,
    0x7f,-3,0x7f,
    0x7f,0,0x7f,
    0x7f,0x7f,0x7f,
    0x7f,0x7f,0x7f,
    0x7f,0x7f,0x7f,
    0x7f,0x7f,0x7f,
    0x7f,0x7f,0x7f,  //rx hwgain tbl ptr

    50,0,-80,//54,0,-80, //anwei 0430
    40,0,-80,
    36,0,-80,
    34,0,-80,
    29,0,-80,
    24,0,-80,
    18,0,-80,
    14,0,-80,

    0x7f,0x7f,0x7f,
    0x7f,0x7f,0x7f,
    0x7f,0x7f,0x7f,
    0x7f,0x7f,0x7f,
    0x7f,0x7f,0x7f,
    0x7f,0x7f,0x7f,
    0x7f,0x7f,0x7f,  //rx gain tbl ptr

    -89,-89,
    -85,-85,
    -82,-82,
    -79,-79,
    -74,-74,
    -69,-69,
    -63,-63,
    0x7f,0x7f,

    //  0x7f,0x7f,
    0x7f,0x7f,
    0x7f,0x7f,
    0x7f,0x7f,
    0x7f,0x7f,
    0x7f,0x7f,
    0x7f,0x7f,
    0x7f,0x7f,    //rx gain ths tbl ptr

    0,0,
    0,0,
    0,0,
    0,0,
    0,0,
    0,1,
    0,2,
    0,2,
    0,2,
    0x7f,0x7f,
    0x7f,0x7f,
    0x7f,0x7f,
    0x7f,0x7f,
    0x7f,0x7f,
    0x7f,0x7f,    //flpha filter factor ptr
    -23,-20,-17,-14,-11,-8,-5,-2,0x7f,0x7f,0x7f,0x7f,0x7f,0x7f,0x7f,   //tx pw onv tbl ptr
};


const int8_t btdrv_rxgain_ths_tbl_le[0xf * 2] =
{
    -85,-85,
    -75,-75,
    -71,-71,
    -67,-67,
    -60,-60,
    -48,-48,
    -32,-32,
    0x7f,0x7f,

    //  0x7f,0x7f,
    0x7f,0x7f,
    0x7f,0x7f,
    0x7f,0x7f,
    0x7f,0x7f,
    0x7f,0x7f,
    0x7f,0x7f,
    0x7f,0x7f,    //ble rx gain ths tbl ptr
};

const int8_t btdrv_afh_env[] =
{
    0x02,0x00,   //afh env
    0x00,      //ignore
    33,          //length
    5,   //nb_reass_chnl
    10,  //win_len
    -70,  //rf_rssi_interf_thr
    10,  //per_thres_bad
    20,  //reass_int
    20,   //n_min
    20,   //afh_rep_intv_max
    96,    //ths_min
    2,   //chnl_assess_report_style
    15,  //chnl_assess_diff_thres
    60, // chnl_assess_interfere_per_thres_bad
    9,  //chnl_assess_stat_cnt_max
    -9,  //chnl_assess_stat_cnt_min
    1,2,3,2,1,   //chnl_assess_stat_cnt_inc_mask[5]
    1,2,3,2,1,    //chnl_assess_stat_cnt_dec_mask
    0xd0,0x7,      //chnl_assess_timer
    -48,        //chnl_assess_min_rssi
    0x64,0,   //chnl_assess_nb_pkt
    0x32,0,     //chnl_assess_nb_bad_pkt
    6,    //chnl_reassess_cnt_val
    0x3c,0,     //chnl_assess_interfere_per_thres_bad
};

const uint8_t lpclk_drift_jitter[] =
{
    0xfa,0x00,  //  drift  250ppm
    0x0a,0x00    //jitter  +-10us

};

const uint8_t  wakeup_timing[] =
{
    TWOSC_TIMEOUT_US_LO, TWOSC_TIMEOUT_US_HI,   //exernal_wakeup_time
    TWOSC_TIMEOUT_US_LO, TWOSC_TIMEOUT_US_HI,   //oscillater_wakeup_time
    TWOSC_TIMEOUT_US_LO, TWOSC_TIMEOUT_US_HI,   //radio_wakeup_time
};


uint8_t  sleep_param[] =
{
    1,    // sleep_en;
    1,    // exwakeup_en;
    0xc8,0,    //  lpo_calib_interval;   lpo calibration interval
    0x32,0,0,0,    // lpo_calib_time;  lpo count lpc times
};

uint8_t  unsleep_param[] =
{
    0,    // sleep_en;
    1,    // exwakeup_en;
    0xc8,0,    //  lpo_calib_interval;   lpo calibration interval
    0x32,0,0,0,    // lpo_calib_time;  lpo count lpc times
};

const uint16_t me_bt_default_page_timeout = 0x2000;

const uint8_t  sync_config[] =
{
    1,1,   //sco path config   0:hci  1:pcm
    0,      //sync use max buff length   0:sync data length= packet length 1:sync data length = host sync buff len
    0,        //cvsd bypass     0:cvsd2pcm   1:cvsd transparent
};


//pcm general ctrl
#define PCM_PCMEN_POS            15
#define PCM_LOOPBCK_POS          14
#define PCM_MIXERDSBPOL_POS      11
#define PCM_MIXERMODE_POS        10
#define PCM_STUTTERDSBPOL_POS    9
#define PCM_STUTTERMODE_POS      8
#define PCM_CHSEL_POS            6
#define PCM_MSTSLV_POS           5
#define PCM_PCMIRQEN_POS         4
#define PCM_DATASRC_POS          0


//pcm phy ctrl
#define PCM_LRCHPOL_POS     15
#define PCM_CLKINV_POS      14
#define PCM_IOM_PCM_POS     13
#define PCM_BUSSPEED_LSB    10
#define PCM_SLOTEN_MASK     ((uint32_t)0x00000380)
#define PCM_SLOTEN_LSB      7
#define PCM_WORDSIZE_MASK   ((uint32_t)0x00000060)
#define PCM_WORDSIZE_LSB    5
#define PCM_DOUTCFG_MASK    ((uint32_t)0x00000018)
#define PCM_DOUTCFG_LSB     3
#define PCM_FSYNCSHP_MASK   ((uint32_t)0x00000007)
#define PCM_FSYNCSHP_LSB    0

/// Enumeration of PCM status
enum PCM_STAT
{
    PCM_DISABLE = 0,
    PCM_ENABLE
};

/// Enumeration of PCM channel selection
enum PCM_CHANNEL
{
    PCM_CH_0 = 0,
    PCM_CH_1
};

/// Enumeration of PCM role
enum PCM_MSTSLV
{
    PCM_SLAVE = 0,
    PCM_MASTER
};

/// Enumeration of PCM data source
enum PCM_SRC
{
    PCM_SRC_DPV = 0,
    PCM_SRC_REG
};

/// Enumeration of PCM left/right channel selection versus frame sync polarity
enum PCM_LR_CH_POL
{
    PCM_LR_CH_POL_RIGHT_LEFT = 0,
    PCM_LR_CH_POL_LEFT_RIGHT
};

/// Enumeration of PCM clock inversion
enum PCM_CLK_INV
{
    PCM_CLK_RISING_EDGE = 0,
    PCM_CLK_FALLING_EDGE
};

/// Enumeration of PCM mode selection
enum PCM_MODE
{
    PCM_MODE_PCM = 0,
    PCM_MODE_IOM
};

/// Enumeration of PCM bus speed
enum PCM_BUS_SPEED
{
    PCM_BUS_SPEED_128k = 0,
    PCM_BUS_SPEED_256k,
    PCM_BUS_SPEED_512k,
    PCM_BUS_SPEED_1024k,
    PCM_BUS_SPEED_2048k
};

/// Enumeration of PCM slot enable
enum PCM_SLOT
{
    PCM_SLOT_NONE = 0,
    PCM_SLOT_0,
    PCM_SLOT_0_1,
    PCM_SLOT_0_2,
    PCM_SLOT_0_3
};

/// Enumeration of PCM word size
enum PCM_WORD_SIZE
{
    PCM_8_BITS = 0,
    PCM_13_BITS,
    PCM_14_BITS,
    PCM_16_BITS
};

/// Enumeration of PCM DOUT pad configuration
enum PCM_DOUT_CFG
{
    PCM_OPEN_DRAIN = 0,
    PCM_PUSH_PULL_HZ,
    PCM_PUSH_PULL_0
};

/// Enumeration of PCM FSYNC physical shape
enum PCM_FSYNC
{
    PCM_FSYNC_LF = 0,
    PCM_FSYNC_FR,
    PCM_FSYNC_FF,
    PCM_FSYNC_LONG,
    PCM_FSYNC_LONG_16
};

const uint32_t pcm_setting[] =
{
//pcm_general_ctrl
    (PCM_DISABLE<<PCM_PCMEN_POS) |                      //enable auto
    (PCM_DISABLE << PCM_LOOPBCK_POS)  |                 //LOOPBACK test
    (PCM_DISABLE << PCM_MIXERDSBPOL_POS)  |
    (PCM_DISABLE << PCM_MIXERMODE_POS)  |
    (PCM_DISABLE <<PCM_STUTTERDSBPOL_POS) |
    (PCM_DISABLE <<PCM_STUTTERMODE_POS) |
    (PCM_CH_0<< PCM_CHSEL_POS) |
    (PCM_MASTER<<PCM_MSTSLV_POS) |                      //BT clock
    (PCM_DISABLE << PCM_PCMIRQEN_POS) |
    (PCM_SRC_DPV<<PCM_DATASRC_POS),

//pcm_phy_ctrl
    (PCM_LR_CH_POL_RIGHT_LEFT << PCM_LRCHPOL_POS) |
    (PCM_CLK_FALLING_EDGE << PCM_CLKINV_POS) |
    (PCM_MODE_PCM << PCM_IOM_PCM_POS) |
    (PCM_BUS_SPEED_2048k << PCM_BUSSPEED_LSB) |         //8k sample rate; 2048k = slot_num * sample_rate * bit= 16 * 8k * 16
    (PCM_SLOT_0_1 << PCM_SLOTEN_LSB) |
    (PCM_16_BITS << PCM_WORDSIZE_LSB) |
    (PCM_PUSH_PULL_0 << PCM_DOUTCFG_LSB) |
    (PCM_FSYNC_LF << PCM_FSYNCSHP_LSB),
};

#if 1
const uint8_t local_feature[] =
{
#if defined(__3M_PACK__)
    0xBF, 0xeE, 0xCD,0xFe,0xdb,0xFd,0x7b,0x87
#else
    0xBF, 0xeE, 0xCD,0xFa,0xdb,0xbd,0x7b,0x87

    //0xBF,0xFE,0xCD,0xFa,0xDB,0xFd,0x73,0x87   // disable simple pairing
#endif
};

#else
// disable simple pairing
uint8_t local_feature[] =
{
    0xBF,0xFE,0xCD,0xFE,0xDB,0xFd,0x73,0x87
};
#endif
const uint8_t local_ex_feature_page1[] =
{
    1,   //page
    0,0,0,0,0,0,0,0,   //page 1 feature
};

const uint8_t local_ex_feature_page2[] =
{
    2,   //page
    0x1f,0x03,0x00,0x00,0x00,0x00,0x00,0x00,   //page 2 feature
};

const uint8_t bt_rf_timing[] =
{
    0xE,   //tx delay
    0x13,    //rx delay
    0x42,  //rx pwrup
    0x0f, ///tx down
    0x56,  //tx pwerup

};

const uint8_t ble_rf_timing[] =
{
    0xC,    //tx delay   tx after rx delay
    0x2C,    //win count   rx window count
    0xe,    ///ble rtrip delay
    0x42,  ///ble rx pwrup
    0x7,    ///ble tx pwerdown
    0x40,   ///ble tx pwerup
};

const uint8_t ble_rl_size[] =
{
    10,    //rl size
};

uint8_t bt_setting_2500i[104] =
{
    0x00,//clk_off_force_even
    0x01,//msbc_pcmdout_zero_flag
    0x04,//ld_sco_switch_timeout
    0x00,//stop_latency2
    0x01,//force_max_slot
    0xc8,0x00,//send_connect_info_to
    0x20,0x03,//sco_to_threshold
    0x40,0x06,//acl_switch_to_threshold
    0x3a,0x00,//sync_win_size
    0x01,//polling_rxseqn_mode
#ifdef __BT_ONE_BRING_TWO__
    0x01,//two_slave_sched_en
#else
    0x00,//two_slave_sched_en
#endif
    0xff,//music_playing_link
    0x06,//wesco_nego = 3;
    0x17,0x11,0x00,0x00,//two_slave_extra_duration_add (7*625);
    0x01,//ble_adv_ignore_interval
    0x04,//slot_num_diff
    0x01,//csb_tx_complete_event_enable
    0x04,//csb_afh_update_period
    0x00,//csb_rx_fixed_len_enable
    0x01,//force_max_slot_acc
    0x00,//force_5_slot;
    0x00,//dbg_trace_level
    0x00,//bd_addr_switch
    0x00,//force_rx_error
    0x08,//data_tx_adjust
    0x02,//ble txpower need modify ble tx idx @ bt_drv_config.c;
    0x50,//sco_margin
    0x78,//acl_margin
    0x01,//pca_disable_in_nosync
    0x01,//master_2_poll
    0x3a,//sync2_add_win
    0x5e,//no_sync_add_win
    0x0b,//rwbt_sw_version_major
    0x04,0x00,//rwbt_sw_version_minor
    0x2c,0x00,//rwbt_sw_version_build

    0x01,//rwbt_sw_version_sub_build
    0x00,//public_key_check;
    0x00,//sco_txbuff_en;
    0x3f,//role_switch_packet_br;
    0x2b,//role_switch_packet_edr;
    0x00,//dual_slave_tws_en;
    0x07,//dual_slave_slot_add;
    0x00,//master_continue_tx_en;

    0x20,0x03,//get_rssi_to_threshold;
    0x05,//rssi_get_times;
    0x00,//ptt_check;
    0x0c,//protect_sniff_slot;
    0x01,//power_control_type;
#ifdef TX_RX_PCM_MASK
    0x01,//sco_buff_copy
#else
    0x00,//sco_buff_copy
#endif
    0x34,0x00,//acl_interv_in_snoop_mode;
    0x9a,0x00,//acl_interv_in_snoop_sco_mode;

    0x08,//acl_slot_in_snoop_mode;
    0x01,//calculate_en;
    0x00,//check_sniff_en;
    0x01,//sco_avoid_ble_en;
    0x01,//check_ble_en;
    0x01,//ble_slot;
    0x04,//same_link_slave_extra;
    0x04,//other_link_slave_extra;

    0x00,//master_extra;
    0x00,//ble_extra;
    0x00,//sco_extra;
    0x00,//sniff_extra;
    0xc4,0x09,0x00,0x00,//same_link_duration;
    0xc4,0x09,0x00,0x00,//other_link_duration;

    0x06,//dual_slave;
    0x14,//clk_check_threshold;
    0x90,0x01,//target_clk_add;
    0x01,//close_check_sco_to;
    0x00,0x00,//(tports_level)
    0x01, //close_loopbacken_flag
    0x1e,  //seq_error_num
    0x01,//check_role_switch
    0x01,//rsw_end_prority
#ifdef __FASTACK_ECC_ENABLE__
    0x01,//ecc_enable
#else
    0x00,//ecc_enable
#endif
    0x00,//sw_seq_filter_en
    0x01,//page_pagescan_coex_en
#ifdef ACCEPT_NEW_MOBILE_EN
    0x01,//accept_new_mobile_en
#else
    0x00,
#endif
    0x64,//reserve_slot_for_send_profile
    0x0e, //magic_cal_bitoff
    0x00,//sco_role_switch_mode
    0x00,//address_reset
    0x17,0x11,//adv_duration
};

uint8_t bt_setting_ext1_2500i[25] =
{
    0x00,0x01,//stop_notif_bit_off_thr
    0x32,0x00,//bit_off_diff
    0x00,//bt_wlan_onoff
    0x12,//tws_link_in_sco_prio
    0x01,//tws_link_rx_traficc
    0x00,//tws_link_ignore_1_sco;
    0x08,//default_tpoll
    0x00,//send_profile_via_ble
    0x0a,//swagc_thd
    0x98,//swagc_count
    0x14,//max_drift_limit
    0x00,//sync2_sco_cal_enable
#ifdef IBRT
    0x00,//accep_remote_bt_roleswitch
#else
    0x01,
#endif
    0x01,//hci_auto_accept_tws_link_en
    0x01,//enable_assert
    0x01,//clear_tx_silence
    0xc8,0x00,//hci_timeout_sleep
    0x50,//link_no_snyc_thd
    0xa6,//link_no_sync_rssi(-90dBm)
    0x64,0x00,//link_no_sync_timeout
    0x01,//ibrt_role_switch_check_sco
};

uint8_t bt_setting_ext2_2500i[79] =
{
    0x00,//trigger_mobile_tx_lmp;
    0x06,//scan_duration_in_sco;
    0x0f,//active_mode_prio;
    0x14,//sco_mode_prio;
    0x00,//pos_extra;
    0x00,//same_link_pos;
    0x06,// mobile_link_duration;
    0x01,// msg_filter_en;
    0x01,// only_support_slave_role;
    0x00,// fix_drift_en;
    0xff,// fix_drift_linkid;
    0x03,// sync_found_check_hecerror;
#ifdef __BES_FA_MODE__
    0x01,// bes_fa_mode enable;
#else
    0x00,// bes_fa_mode disable;
#endif
    0x02,// retx_in_tws_time;
    0x02,// retx_in_fine_interval;
    0x02,// retx_in_stop_cbk;
    0x01,// detach_directly;
    0x01,// rxheader_int_en;
    0x01,// sniffer_sco_auto_accept;
    0x00,//fa_to_en;
    0x55,0x00,0x00,0x00,//fa_to_type;
    0x55,0x00,0x00,0x00,//fa_disable_type;
    0x0f,// fa_to_num;
    0x00,// pscan_hwagc_flag;
    0x00,// iscan_hwagc_flag;
    0x00,// sniff_hwagc_flag;
    0x00,// blescan_hwagc_flag;
    0x00,// bleinit_hwagc_flag;
    0xff,// fa_rxgain slect tws agc mode;
    0xff,// fa_txpwr;
    0x04,// ble_adv_sco_pos;
    0x00,// ecc_data_flag;
    0x00,// rx_noync_chnl_assess_en;
    0x14,// rx_noync_thr_good;
    0x01,// rx_noync_thr_bad;
    0x00,// snr_chnl_assess_en;
    0x14,// snr_good_thr_value;
    0x28,// snr_bad_thr_value;
    0x00,// bt_sync_swagc_en;
#ifdef __BLE_NEW_SWAGC_MODE__
    0x01,// le_sync_swagc_en;
#else
    0x00,// le_sync_swagc_en;
#endif
    0x00,// fast_tws_unsniff_en;
    0x08,// tws_avoid_mobile_sniff;
    0xff,0xff,// sniff_win_lim;
    0x00,// tws_long_packet_en;
    0x00,// tws_long_packet_tx;
    0x00,// tws_long_packet_tx_to;
    0x02,// tws_long_packet_tx_to_thd;
    0x79,0x00,// tws_long_packet_tx_len_thd;
    0x79,0x00,// tws_long_packet_rx_len_thd;
    0x01,0x00,// sco_start_delay;
    0x00,// tws_long_packet_rx;
    0x00,// tws_long_packet_rx_to;
    0x64,// tws_long_packet_rx_to_thd;
    0x05,// lmp_to_before_con_complete;
    0x00,// acl_force_ack_flag;
    0x00,// ble_swagc_after_pkt_correct;
    0x00,// softbit_data_flag;
    0x01,// key_gen_after_reset;
    0x01,// rx_sync_irq_en;
    0x00,// bt_tx_irq_en;
    0x01,// block_sync_ind;
    0x02,// ibrt_lmp_to;
    0x01,// esco_retx_after_establish;
    0x01,// accept_remote_enter_sniff;
    0x00,// resume_lmp_list;
    0x2b,// start_ibrt_edr_txtype;
    0x01,// ignore_pwr_ctrl_sm_state;
    0x96,// acl_rssi_avg_nb_pkt;
    0x03,// delay_process_lmp_type;
};

uint8_t bt_setting_ext3_2500i[27] =
{
    0x00,0x00,0x00,0x00,//maxfrmtime;
    0x00,//fa_lowlayer_metric_en;
    0x05,//delay_process_lmp_to;
    0x00,//linkswint_en;
    0x00,//btswtobleint_en;
    0x00,//bleswtobtint_en;
    0x00,// pkterrint_en;
    0x00,// frmerrint_en;
    0x00,// lowlayer_metric_tx;
    0x01,// sync_force_max_slot;
    0x01,// ignore_lmp_flush;
    0x00,// sniff_rx_win_size_drift;
    0x01,// bv11c;
    0x00,// signal_test_en;
    0x00,// acl_rx_em2ram_enable;
    0x00,// afh_report;
    0x00,// enhance_fa_mode;
    0x00,// pkterr_isr_report_en;
    0x00,// frmerr_isr_reprot_en;
    0x05,// new_accpet_mobile_cmp_addr_len;
    0x00,//softbit_enable;
    0x05,//new_agc_adjust_dbm;
    0x00,//sco_start_delay_flag;
    0x0a,// sco_anchor_start_add;
};

bool btdrv_get_accept_new_mobile_enable(void)
{
    bool ret = false;
    BT_DRV_TRACE(1,"BT_DRV_CONFIG:accept_new_mobile enable=%d", bt_setting_2500i[97]);
    ret = bt_setting_2500i[97];
    return ret;
}

bool btdrv_get_page_pscan_coex_enable(void)
{
    bool ret = false;
    BT_DRV_TRACE(1,"BT_DRV_CONFIG:page_pscan_coex enable=%d", bt_setting_2500i[96]);
    ret = bt_setting_2500i[96];
    return ret;
}

const uint8_t bt_edr_thr[] =
{
    30,0,60,0,5,0,60,0,0,1,
    30,0,60,0,5,0,60,0,1,1,
    30,0,60,0,5,0,60,0,1,1,
    0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,
    0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,
    0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,
    0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,
};

const uint8_t bt_edr_algo[]=
{
    0,0,1,
    8,0,3,
    16,0,0,
    0xff,0xff,0xff,
    0xff,0xff,0xff,
    0xff,0xff,0xff,
    0xff,0xff,0xff,
};

const uint8_t bt_rssi_thr[]=
{
    -1, //high
    -2, //low
    -100, //interf
};


const uint8_t ble_dle_dft_value[]=
{
    0xfb,0x00, ///tx octets
    0x48,0x08, ///tx time
    0xfb,0x00, ///rx octets
    0x48,0x08, ///rx time
};

uint8_t bt_lmp_record[]=
{
    0,  //en
    0,  //only opcode
};

const uint8_t low_txpwr_mode[]=
{
    0xFF, //factor  1: -6dbm, 2:-12dbm
    0xFF, //ble_txpwr in adv and le control pdu,  bit 3:low pwr mode(8 + pwr)
    2, //low pwr  bt page txpwr  bit 3:low pwr mode(8 + pwr)
};

const struct hci_dbg_set_sw_rssi_cmd  sw_rssi_idle_mode =
{
    .sw_rssi_en = 1,
    .link_agc_thd_mobile = 64,
    .link_agc_thd_mobile_time = 300,
    .link_agc_thd_tws = 3,
    .link_agc_thd_tws_time = 500,
    .rssi_mobile_step = 5,
    .rssi_tws_step = 10,
    .rssi_min_value_mobile = -100,
    .rssi_min_value_tws = -100,
};

const struct hci_dbg_set_sw_rssi_cmd  sw_rssi_work_mode =
{
    .sw_rssi_en = 1,
    .link_agc_thd_mobile = 72,
    .link_agc_thd_mobile_time = 0x30,
    .link_agc_thd_tws = 3,
    .link_agc_thd_tws_time = 0x80,
    .rssi_mobile_step = 24,
    .rssi_tws_step = 48,
    .rssi_min_value_mobile = -80,
    .rssi_min_value_tws = -100,
};

const struct hci_dbg_set_sw_rssi_cmd  sw_rssi_test_mode =
{
    .sw_rssi_en = 1,
    .link_agc_thd_mobile = 5,
    .link_agc_thd_mobile_time = 0x30,
    .link_agc_thd_tws = 3,
    .link_agc_thd_tws_time = 0x80,
    .rssi_mobile_step = 24,
    .rssi_tws_step = 48,
    .rssi_min_value_mobile = -100,
    .rssi_min_value_tws = -100,
};


#if 0
const uint8_t sw_rssi_adjust[]=
{
    0x01, //sw_rssi_en
    20,0,0,0, //link_agc_thd_mobile
    0x40,0,0,0, //link_agc_thd_mobile_time
    2,0,0,0, //link_agc_thd_tws
    0x4,0,0,0, //link_agc_thd_tws_time
    5,//rssi_mobile_step
    20,//rssi_tws_step
    0x9c,//rssi_min_value_mobile
    0x9c,//rssi_min_value_tws
};
#endif

struct bt_cmd_chip_config_t g_bt_drv_btstack_chip_config = {
    HCI_DBG_SET_SYNC_CONFIG_CMD_OPCODE,
    HCI_DBG_SET_SCO_SWITCH_CMD_OPCODE,
};

static BTDRV_CFG_TBL_STRUCT  btdrv_cfg_tbl[] =
{
    {BTDRV_CONFIG_INACTIVE,HCI_DBG_LMP_MESSAGE_RECORD_CMD_OPCODE,sizeof(bt_lmp_record),(uint8_t *)&bt_lmp_record},
    {BTDRV_CONFIG_ACTIVE,HCI_DBG_SET_LOCAL_FEATURE_CMD_OPCODE,sizeof(local_feature),local_feature},
    {BTDRV_CONFIG_INACTIVE,HCI_DBG_SET_BT_SETTING_CMD_OPCODE,sizeof(bt_setting_2500i),bt_setting_2500i},
    {BTDRV_CONFIG_INACTIVE,HCI_DBG_SET_BT_SETTING_EXT1_CMD_OPCODE,sizeof(bt_setting_ext1_2500i),bt_setting_ext1_2500i},
    {BTDRV_CONFIG_INACTIVE,HCI_DBG_SET_BT_SETTING_EXT2_CMD_OPCODE,sizeof(bt_setting_ext2_2500i),bt_setting_ext2_2500i},
    {BTDRV_CONFIG_INACTIVE,HCI_DBG_SET_BT_SETTING_EXT3_CMD_OPCODE,sizeof(bt_setting_ext3_2500i),bt_setting_ext3_2500i},
    //{BTDRV_CONFIG_ACTIVE,HCI_DBG_SET_SLEEP_SETTING_CMD_OPCODE,sizeof(sleep_param),sleep_param},
    {BTDRV_CONFIG_ACTIVE,HCI_DBG_SET_CUSTOM_PARAM_CMD_OPCODE,189,(uint8_t *)&btdrv_rf_env_2500i},
    //{BTDRV_CONFIG_ACTIVE,HCI_DBG_SET_CUSTOM_PARAM_CMD_OPCODE,sizeof(btdrv_afh_env),(uint8_t *)&btdrv_afh_env},
    // {BTDRV_CONFIG_INACTIVE,HCI_DBG_SET_LPCLK_DRIFT_JITTER_CMD_OPCODE,sizeof(lpclk_drift_jitter),(uint8_t *)&lpclk_drift_jitter},
     {BTDRV_CONFIG_ACTIVE,HCI_DBG_SET_WAKEUP_TIME_CMD_OPCODE,sizeof(wakeup_timing),(uint8_t *)&wakeup_timing},
#ifdef _SCO_BTPCM_CHANNEL_
    {BTDRV_CONFIG_ACTIVE,HCI_DBG_SET_SYNC_CONFIG_CMD_OPCODE,sizeof(sync_config),(uint8_t *)&sync_config},
    {BTDRV_CONFIG_ACTIVE,HCI_DBG_SET_PCM_SETTING_CMD_OPCODE,sizeof(pcm_setting),(uint8_t *)&pcm_setting},
#endif
    {BTDRV_CONFIG_ACTIVE,HCI_DBG_SET_RSSI_THRHLD_CMD_OPCODE,sizeof(bt_rssi_thr),(uint8_t *)&bt_rssi_thr},
    {BTDRV_CONFIG_ACTIVE,HCI_DBG_SET_LOCAL_EX_FEATURE_CMD_OPCODE,sizeof(local_ex_feature_page2),(uint8_t *)&local_ex_feature_page2},
    {BTDRV_CONFIG_ACTIVE,HCI_DBG_SET_BT_RF_TIMING_CMD_OPCODE,sizeof(bt_rf_timing),(uint8_t *)&bt_rf_timing},
    {BTDRV_CONFIG_ACTIVE,HCI_DBG_SET_BLE_RF_TIMING_CMD_OPCODE,sizeof(ble_rf_timing),(uint8_t *)&ble_rf_timing},
    {BTDRV_CONFIG_ACTIVE,HCI_DBG_SET_TXPWR_MODE_CMD_OPCODE,sizeof(low_txpwr_mode),(uint8_t *)&low_txpwr_mode},
    //{BTDRV_CONFIG_ACTIVE,HCI_DBG_SET_SW_RSSI_CMD_OPCODE,sizeof(sw_rssi_adjust),(uint8_t *)&sw_rssi_adjust},
};

const static POSSIBLY_UNUSED uint32_t mem_config_2500i[][2] =
{
};

void btdrv_ble_rx_gain_thr_tbl_set(void)
{
    int sRet = 0;
    sRet = memcpy_s((uint8_t *)BLE_RXGAIN_THS_TBL_ADDR,sizeof(btdrv_rxgain_ths_tbl_le),btdrv_rxgain_ths_tbl_le,sizeof(btdrv_rxgain_ths_tbl_le));
    if (sRet){
        BT_DRV_TRACE(1, "%s line:%d sRet:%d", __func__, __LINE__, sRet);
    }
}

void btdrv_fa_txpwrup_timing_setting(uint8_t txpwrup)
{
    BTDIGITAL_REG_SET_FIELD(BT_FASTACK_ADDR, 0xff, 0, txpwrup);
}

void btdrv_fa_rxpwrup_timig_setting(uint8_t rxpwrup)
{
    BTDIGITAL_REG_SET_FIELD(BT_FASTACK_ADDR, 0xff, 8, rxpwrup);
}

void bt_fa_sync_invert_en_setf(uint8_t faacinven)
{
    BTDIGITAL_REG_WR(BT_FASTACK_ADDR,
                     (BTDIGITAL_REG(BT_FASTACK_ADDR) & ~((uint32_t)0x00020000)) | ((uint32_t)faacinven << 17));
}

void btdrv_fast_lock_en_setf(uint8_t rxonextenden)
{
    BTDIGITAL_REG_WR(BT_FASTACK_ADDR,
                     (BTDIGITAL_REG(BT_FASTACK_ADDR) & ~((uint32_t)0x04000000)) | ((uint32_t)rxonextenden << 26));
}

void btdrv_fa_xsco_wrbuf_mask_en_setf(uint8_t xscowrbufmasken)
{
    BTDIGITAL_REG_WR(BT_FASTACK_ADDR,
                     (BTDIGITAL_REG(BT_FASTACK_ADDR) & ~((uint32_t)0x10000000)) | ((uint32_t)xscowrbufmasken << 28));
}

void btdrv_btradiowrupdn_set_rtrip_delay(uint8_t rtrip_delay)
{
    BTDIGITAL_REG_SET_FIELD(0xd0220080, 0x7f, 24, (rtrip_delay & 0x7f));
}
void btdrv_btradiowrupdn_set_rxpwrup(uint8_t rxpwrup)
{
    BTDIGITAL_REG_SET_FIELD(0xd0220080, 0xff, 16, (rxpwrup & 0xff));
}
void btdrv_btradiowrupdn_set_txpwrdn(uint8_t txpwrdn)
{
    BTDIGITAL_REG_SET_FIELD(0xd0220080, 0x1f, 8, (txpwrdn & 0x1f));
}
void btdrv_btradiowrupdn_set_txpwrup(uint8_t txpwrup)
{
    BTDIGITAL_REG_SET_FIELD(0xd0220080, 0xff, 0, (txpwrup & 0xff));
}

void btdrv_bleradiowrupdn_set_rtrip_delay(uint8_t rtrip_delay)
{
    BTDIGITAL_REG_SET_FIELD(0xd0220280, 0x7f, 24, (rtrip_delay & 0x7f));
}
void btdrv_bleradiowrupdn_set_rxpwrup(uint8_t rxpwrup)
{
    BTDIGITAL_REG_SET_FIELD(0xd0220280, 0xff, 16, (rxpwrup & 0xff));
}
void btdrv_bleradiowrupdn_set_txpwrdn(uint8_t txpwrdn)
{
    BTDIGITAL_REG_SET_FIELD(0xd0220280, 0x1f, 8, (txpwrdn & 0x1f));
}
void btdrv_bleradiowrupdn_set_txpwrup(uint8_t txpwrup)
{
    BTDIGITAL_REG_SET_FIELD(0xd0220280, 0xff, 0, (txpwrup & 0xff));
}

#if (defined(SOFTBIT_EN))
void btdrv_softbit_config(uint16_t connhdl, uint8_t type1,uint8_t type2,uint8_t type3, uint8_t num)
{
    uint8_t link_id = 0;
    if(num == 1)
    {
        BTDIGITAL_REG_SET_FIELD(0xd0220464, 0xf, 0, (type1 & 0xf));
        BTDIGITAL_REG(0xd0220464) |= (1<<12);
    }
    else if(num == 2)
    {
        BTDIGITAL_REG_SET_FIELD(0xd0220464, 0xf, 0, (type1 & 0xf));
        BTDIGITAL_REG(0xd0220464) |= (1<<12);
        BTDIGITAL_REG_SET_FIELD(0xd0220464, 0xf, 4, (type2 & 0xf));
        BTDIGITAL_REG(0xd0220464) |= (1<<13);
    }
    else if(num == 3)
    {
        BTDIGITAL_REG_SET_FIELD(0xd0220464, 0xf, 0, (type1 & 0xf));
        BTDIGITAL_REG(0xd0220464) |= (1<<12);
        BTDIGITAL_REG_SET_FIELD(0xd0220464, 0xf, 4, (type2 & 0xf));
        BTDIGITAL_REG(0xd0220464) |= (1<<13);
        BTDIGITAL_REG_SET_FIELD(0xd0220464, 0xf, 8, (type3 & 0xf));
        BTDIGITAL_REG(0xd0220464) |= (1<<14);
    }
    if(connhdl & (0xff00))//sco
    {
        link_id = ((connhdl & 0xff00) >> 8) + 1;
        BTDIGITAL_REG_SET_FIELD(0xd022047c, 3, 24, (link_id & 3));
        BTDIGITAL_REG(0xd022046c) |= (1<<5);
    }
    else if(connhdl >= HCI_HANDLE_MIN && connhdl <= HCI_HANDLE_MAX) //acl
    {
        link_id = connhdl - HCI_HANDLE_MIN +  1;
        BTDIGITAL_REG_SET_FIELD(0xd022047c, 3, 24, (link_id & 3));
        BTDIGITAL_REG(0xd022046c) |= (1<<6);
    }
    BTDIGITAL_REG_SET_FIELD(0xd0220028, 0x1ff, 0, 0xe1);//rxguard time for sbc

    //[18]1:SNR weighting mode
#ifdef __SNR_WEIGHTING_MODE__
    BTDIGITAL_REG(0xd03503d8) |= (1<<18);
#else
    //[18]0:equally weighted mode
    BTDIGITAL_REG(0xd03503d8) &= ~(1<<18);
#endif
}
#endif

void btdrv_2m_band_wide_sel(uint8_t fa_2m_mode)
{
#ifdef BT_SYSTEM_52M
    //only BT system 52M can use fastack 2M mode
    BTDIGITAL_REG_WR(BT_ALWAYS_ACK_ADDR,
        (BTDIGITAL_REG(BT_ALWAYS_ACK_ADDR) & ~((uint32_t)0x20000000)) | ((uint32_t)fa_2m_mode << 29));
#endif
}

void btdrv_52m_sys_enable(void)
{
    hal_cmu_bt_sys_set_freq(HAL_CMU_FREQ_52M);
    BTDIGITAL_REG(0xd0330038)|=(1<<7)|(1<<11);//BT select 52M system
    BTDIGITAL_REG(0xd03300f0)|=1;
    BTDIGITAL_REG_SET_FIELD(0xd03502c4, 0x7ff, 0, (0x307 & 0x7ff));//te_timeinit;
    BTDIGITAL_REG_SET_FIELD(0xd03502c4, 0x3ff, 22, (0x2e & 0x3ff));// te_timeinit_ecc

    BTDIGITAL_REG_SET_FIELD(0xd03503a0, 1, 4, 1);//sample delay for 1305
}

void btdrv_new_mode_bt_corr_thr(uint8_t corr_thr)
{
    BTDIGITAL_REG_SET_FIELD(0xd03503a4, 0x7f, 0, (corr_thr & 0x7f));
}

void btdrv_new_mode_ble_corr_thr(uint8_t corr_thr)
{
    BTDIGITAL_REG_SET_FIELD(0xd03503a4, 0x7f, 8, (corr_thr & 0x7f));
}

void btdrv_old_mode_bt_corr_thr(uint8_t corr_thr)
{
    BTDIGITAL_REG_SET_FIELD(0xd0220000, 0xf, 0, (corr_thr & 0xf));
}

void btdrv_old_mode_ble_corr_thr(uint8_t corr_thr)
{
    BTDIGITAL_REG_SET_FIELD(0xd0220200, 0x7, 0, (corr_thr & 0x7));
}

void btdrv_set_fa_use_old_swagc(void)
{
    //second rf spi
#ifdef __NEW_SWAGC_MODE__
    BTDIGITAL_REG_SET_FIELD(0xd0220484, 0xffff, 0, 0x25c);
#else
    BTDIGITAL_REG_SET_FIELD(0xd0220484, 0xffff, 0, 0x21c);
#endif
    BTDIGITAL_REG_SET_FIELD(0xd0220480, 0xff, 0, 0xe2);
}

void btdrv_fa_corr_mode_setting(bool is_new_corr)
{
    //[1]: corr_preamble
    if(is_new_corr == true)
    {
        BTDIGITAL_REG(0xd03503a0) |= (1<<1);//enable new corr
    }
    else
    {
        BTDIGITAL_REG(0xd03503a0) &= ~(1<<1);//enable old corr
    }
}

void btdrv_fa_new_mode_corr_thr(uint8_t corr_thr)
{
    BTDIGITAL_REG_SET_FIELD(0xd03503cc, 0x7f, 17, (corr_thr & 0x7f));
}

void btdrv_fa_old_mode_corr_thr(uint8_t corr_thr)
{
    BTDIGITAL_REG_SET_FIELD(0xd0220000, 0xf, 4, (corr_thr & 0xf));
}

void btdrv_fa_config_vendor_syncword_en(bool enable)
{
    if(enable)
    {
        BTDIGITAL_REG(0xd0220498) |= (1<<30);
    }
    else
    {
        BTDIGITAL_REG(0xd0220498) &= ~(1<<30);
    }
}

void btdrv_set_fa_redundancy_time(uint8_t time)
{
    BTDIGITAL_REG_SET_FIELD(0xd0220488, 0x1f, 21, (time & 0x1f));
}

void btdrv_fa_config_vendor_syncword_content(uint32_t syncword_high, uint32_t syncword_low, uint8_t syncword_len)
{
    if(syncword_low == INVALID_SYNC_WORD || syncword_high == INVALID_SYNC_WORD)
    {
        BT_DRV_TRACE(0,"BT_DRV:vendor syncword invalid");
        return;
    }

    if(syncword_len == FA_SYNCWORD_32BIT)
    {
        BTDIGITAL_REG(0xd02204b8) = syncword_low;
    }
    else if(syncword_len == FA_SYNCWORD_64BIT)
    {
        BTDIGITAL_REG(0xd02204b4) = syncword_high;
        BTDIGITAL_REG(0xd02204b8) = syncword_low;
    }
}

void btdrv_fa_config_syncword_mode(uint8_t syncword_len)
{
    if(syncword_len == FA_SYNCWORD_32BIT)
    {
        BTDIGITAL_REG(0xd022048c) &= ~(1<<30);
        BTDIGITAL_REG(0xd03503cc) |= (1<<15);//syncword 32bit
    }
    else if(syncword_len == FA_SYNCWORD_64BIT)
    {
        BTDIGITAL_REG(0xd022048c) |= (1<<30);
        BTDIGITAL_REG(0xd03503cc) &= ~(1<<15);//syncword 64bit
    }
}

void btdrv_fa_config_tx_gain(bool ecc_tx_gain_en, uint8_t ecc_tx_gain_idx)//false :disable tx gain
{
    if(ecc_tx_gain_en == true)
    {
        BTDIGITAL_REG(0xd0220474) |= (1<<20);
        BTDIGITAL_REG_SET_FIELD(0xd0220474, 0xf, 21, (ecc_tx_gain_idx & 0xf));
    }
    else
    {
        BTDIGITAL_REG(0xd0220474) &= ~(1<<20);
    }
}

void btdrv_fa_config_rx_gain(bool ecc_rx_gain_en, uint8_t ecc_rx_gain_idx)//false: disable rx gain
{
    if(ecc_rx_gain_en == true)
    {
        BTDIGITAL_REG(0xd0220474) |= (1<<16);
        BTDIGITAL_REG_SET_FIELD(0xd0220474, 0x7, 17, (ecc_rx_gain_idx & 0x7));
    }
    else
    {
        BTDIGITAL_REG(0xd0220474) &= ~(1<<16);
    }
}

void bt_drv_enhance_fa_mode(bool enable)
{
    if(enable)
    {
        BTDIGITAL_REG_SET_FIELD(0xd022046c, 1, 11, 1);
    }
    else
    {
        BTDIGITAL_REG_SET_FIELD(0xd022046c, 1, 11, 0);
    }
}

void btdrv_fa_rx_winsz(uint8_t ecc_rx_winsz)
{
    BTDIGITAL_REG_SET_FIELD(0xd0220488, 0x7f, 14, (ecc_rx_winsz & 0x7f));
}

void btdrv_fa_basic_config(btdrv_fa_basic_config_t* p_fa_basic_config)
{
    if(p_fa_basic_config != NULL)
    {
        //fast ack TX power gain set
        btdrv_fa_config_tx_gain(p_fa_basic_config->fa_tx_gain_en, p_fa_basic_config->fa_tx_gain_idx);
#ifdef __FIX_FA_RX_GAIN___
        //fix fast ack rx gain
        btdrv_fa_config_rx_gain(p_fa_basic_config->fa_rx_gain_en, p_fa_basic_config->fa_rx_gain_idx);
        bt_drv_reg_op_set_fa_rx_gain_idx(p_fa_basic_config->fa_rx_gain_idx);
#endif
        // fa 2M mode select
        btdrv_2m_band_wide_sel(p_fa_basic_config->fa_2m_mode);
        //fa syncword mode
        btdrv_fa_config_syncword_mode(p_fa_basic_config->syncword_len);
        //ECC vendor syncword mode
        if(p_fa_basic_config->fa_vendor_syncword_en)
        {
            btdrv_fa_config_vendor_syncword_en(true);
            btdrv_fa_config_vendor_syncword_content(p_fa_basic_config->syncword_high,
                    p_fa_basic_config->syncword_low,
                    p_fa_basic_config->syncword_len);
        }
        else
        {
            btdrv_fa_config_vendor_syncword_en(false);
        }
        //fa win size
        btdrv_fa_rx_winsz(p_fa_basic_config->fa_rx_winsz);
        //fa corr mode
        btdrv_fa_corr_mode_setting(p_fa_basic_config->is_new_corr);

        if(p_fa_basic_config->is_new_corr)
        {
            btdrv_fa_new_mode_corr_thr(p_fa_basic_config->new_mode_corr_thr);
        }
        else
        {
            btdrv_fa_old_mode_corr_thr(p_fa_basic_config->old_mode_corr_thr);
        }
        //set enhance fa mode
        bt_drv_enhance_fa_mode(p_fa_basic_config->enhance_fa_mode_en);
    }

    //1303 FA mode can only use old agc
    btdrv_set_fa_use_old_swagc();
}

#ifdef __FASTACK_ECC_ENABLE__
//ecc usert data
void btdrv_ecc_config_len_mode_sel(uint8_t ecc_len_mode) //only for ecc 1 block
{
    if(ecc_len_mode == ECC_8_BYTE_MODE)
    {
        BTDIGITAL_REG(0xd02204a8) |= (1<<24);
    }
    else if(ecc_len_mode == ECC_16_BYTE_MODE)
    {
        BTDIGITAL_REG(0xd02204a8) &= ~(1<<24);
    }
}

void btdrv_ecc_config_usr_tx_dat_set(ecc_trx_dat_t* p_ecc_trx_dat)
{
    BTDIGITAL_REG(0xd02204b8) = p_ecc_trx_dat->trx_dat.dat_arr[0];
    BTDIGITAL_REG(0xd02204bc) = p_ecc_trx_dat->trx_dat.dat_arr[1];
    BTDIGITAL_REG(0xd02204c0) = p_ecc_trx_dat->trx_dat.dat_arr[2];
    BTDIGITAL_REG(0xd02204c4) = p_ecc_trx_dat->trx_dat.dat_arr[3];
}

void btdrv_ecc_config_usr_rx_dat_get(ecc_trx_dat_t* p_ecc_trx_dat)
{
    p_ecc_trx_dat->trx_dat.dat_arr[0] = BTDIGITAL_REG(0xd02204d4);
    p_ecc_trx_dat->trx_dat.dat_arr[1] = BTDIGITAL_REG(0xd02204d8);
    p_ecc_trx_dat->trx_dat.dat_arr[2] = BTDIGITAL_REG(0xd02204dc);
    p_ecc_trx_dat->trx_dat.dat_arr[3] = BTDIGITAL_REG(0xd02204e0);
}


void btdrv_ecc_disable_spec_pkt_type(uint32_t ptk_type) // 1 -> disable FA, ptk type enum in bt_drv_2500i_internal.h
{
    BTDIGITAL_REG(0xd02204a8) |= (1<<23);
    BTDIGITAL_REG(0xd02204ac) = (ptk_type & 0xfffff);
}


void btdrv_ecc_config_blk_mode(uint8_t ecc_blk_mode)
{
    if (ecc_blk_mode == ECC_1BLOCK)
    {
        BTDIGITAL_REG(0xd022048c) &= ~0x3ff;
        BTDIGITAL_REG(0xd022048c) |= 0xef; //set ecc len ECC 1 block
    }
    else if (ecc_blk_mode == ECC_2BLOCK)
    {
        BTDIGITAL_REG(0xd022048c) &= ~0x3ff;
        BTDIGITAL_REG(0xd022048c) |= 0x1de; //set ecc len,  ECC 2 block
    }
    else if (ecc_blk_mode == ECC_3BLOCK)
    {
        BTDIGITAL_REG(0xd022048c) &= ~0x3ff;
        BTDIGITAL_REG(0xd022048c) |= 0x2cd; //set ecc len ECC 3 block
    }
}

void btdrv_ecc_config_modu_mode_acl(uint8_t ecc_modu_mode_acl)
{
    if(ecc_modu_mode_acl == ECC_8PSK)
    {
        BTDIGITAL_REG_SET_FIELD(0xd02204a8, 3, 13, 3); //ECC 8PSK
    }
    else if(ecc_modu_mode_acl == ECC_DPSK)
    {
        BTDIGITAL_REG_SET_FIELD(0xd02204a8, 3, 13, 2); //ECC DPSK
    }
    else if(ecc_modu_mode_acl == ECC_GFSK)
    {
        BTDIGITAL_REG_SET_FIELD(0xd02204a8, 3, 13, 1); //ECC GFSK
    }
}

void btdrv_ecc_config_modu_mode_sco(uint8_t ecc_modu_mode_sco)
{
    if(ecc_modu_mode_sco == ECC_8PSK)
    {
        BTDIGITAL_REG_SET_FIELD(0xd02204a8, 3, 27, 3); //ECC 8PSK
    }
    else if(ecc_modu_mode_sco == ECC_DPSK)
    {
        BTDIGITAL_REG_SET_FIELD(0xd02204a8, 3, 27, 2); //ECC DPSK
    }
    else if(ecc_modu_mode_sco == ECC_GFSK)
    {
        BTDIGITAL_REG_SET_FIELD(0xd02204a8, 3, 27, 1); //ECC GFSK
    }
}

void btdrv_sco_ecc_enable(bool enable)
{
    if(enable == true)
    {
         BTDIGITAL_REG_SET_FIELD(0xd0220460, 1, 25, 1);
    }
    else
    {
         BTDIGITAL_REG_SET_FIELD(0xd0220460, 1, 25, 0);
    }
}

void btdrv_acl_ecc_enable(bool enable)
{
    if(enable == true)
    {
         BTDIGITAL_REG_SET_FIELD(0xd0220460, 1, 24, 1);
    }
    else
    {
         BTDIGITAL_REG_SET_FIELD(0xd0220460, 1, 24, 0);
    }
}

void btdrv_ecc_enable(bool enable)
{
    //ecc all flag, initialize all configurations
    BTDIGITAL_REG_SET_FIELD(0xd0220460, 1, 24, 0);//acl ecc flag, disabled by default
    //to enable acl ecc, please use btdrv_acl_ecc_enable later
    BTDIGITAL_REG_SET_FIELD(0xd0220460, 1, 25, 0);//sco ecc flag, disabled by default
    //to enable sco ecc, please use btdrv_sco_ecc_enable later

    if(enable == true)
    {
        BTDIGITAL_REG(0xd0220464) |= (1<<15);//ecc enable
#ifdef __FASTACK_SCO_ECC_ENABLE__
        btdrv_sco_ecc_enable(true);
#endif
#ifdef __FASTACK_ACL_ECC_ENABLE__
        btdrv_acl_ecc_enable(true);
#endif
    }
    else
    {
        BTDIGITAL_REG(0xd0220464) &= ~(1<<15);//disable
    }
}

void btdrv_ecc_basic_config(btdrv_ecc_basic_config_t* p_ecc_basic_config)
{
    if(p_ecc_basic_config && (p_ecc_basic_config->ecc_mode_enable == true))
    {
        btdrv_ecc_enable(true);
        btdrv_ecc_config_modu_mode_acl(p_ecc_basic_config->ecc_modu_mode_acl);
        btdrv_ecc_config_modu_mode_sco(p_ecc_basic_config->ecc_modu_mode_sco);
        btdrv_ecc_config_blk_mode(p_ecc_basic_config->ecc_blk_mode);
        btdrv_ecc_config_len_mode_sel(p_ecc_basic_config->ecc_len_mode_sel);
    }
}

void btdrv_ecc_content_config(void)
{
    btdrv_ecc_basic_config_t ecc_config;
    //ECC  config
    ecc_config.ecc_mode_enable = true;
    ecc_config.ecc_modu_mode_acl = ECC_MODU_MODE;
    ecc_config.ecc_modu_mode_sco = ECC_MODU_MODE;
    ecc_config.ecc_blk_mode = ECC_BLK_MODE;
    ecc_config.ecc_len_mode_sel = ECC_8_BYTE_MODE;
    //setting
    btdrv_ecc_basic_config(&ecc_config);
}
#endif

void btdrv_fast_ack_config(void)
{
    btdrv_fa_basic_config_t fa_config;
    //fast ack config
    fa_config.fa_2m_mode = false;
    fa_config.fa_vendor_syncword_en = false;
    fa_config.syncword_high = INVALID_SYNC_WORD;
    fa_config.syncword_low = INVALID_SYNC_WORD;
    fa_config.syncword_len = FA_SYNCWORD_64BIT;
    fa_config.is_new_corr = false;

    fa_config.old_mode_corr_thr = FA_OLD_CORR_VALUE;
    fa_config.new_mode_corr_thr = FA_NEW_CORR_VALUE;
    fa_config.fa_tx_gain_en = true;
    fa_config.fa_tx_gain_idx = FA_FIX_TX_GIAN_IDX;
    fa_config.fa_rx_winsz = FA_RX_WIN_SIZE;
#ifdef __FIX_FA_RX_GAIN___
    fa_config.fa_rx_gain_en = true;
    fa_config.fa_rx_gain_idx = FA_FIX_RX_GIAN_IDX;
#endif
    fa_config.enhance_fa_mode_en = false;
    //setting
    btdrv_fa_basic_config(&fa_config);
}

void btdrv_ecc_config(void)
{
    btdrv_fast_ack_config();
#ifdef __FASTACK_ECC_ENABLE__
    btdrv_ecc_content_config();
#endif
}

void btdrv_fast_lock_config(bool fastlock_on)
{
    uint16_t val = 0;

    if (fastlock_on)
    {
        btdrv_read_rf_reg(0x1AE, &val);
        btdrv_write_rf_reg(0x1AE, val | (1 << 9));
        btdrv_read_rf_reg(0xE2, &val);
        btdrv_write_rf_reg(0xE2, val | (1 << 9));
        btdrv_fast_lock_en_setf(1);//digital fast lock enable
        //FA trx pwrup/dn
        btdrv_fa_txpwrup_timing_setting(0x42);
        btdrv_fa_rxpwrup_timig_setting(0x47);
    }
    else
    {
        btdrv_read_rf_reg(0x1AE, &val);
        btdrv_write_rf_reg(0x1AE, val & (~((1 << 9) | (1 << 11))));
        btdrv_read_rf_reg(0xE2, &val);
        btdrv_write_rf_reg(0xE2, val & (~ (1 << 9)));
        btdrv_fast_lock_en_setf(0); //digital fast lock disable
        //FA trx pwrup/dn
        btdrv_fa_txpwrup_timing_setting(0x2c);
        btdrv_fa_rxpwrup_timig_setting(0x32);

        //fa pre win
        BTDIGITAL_REG_SET_FIELD(0xd02204f4, 0x7f,  0, 19);

        btdrv_set_fa_redundancy_time(4);
    }
}

void btdrv_digital_config_init_2500i(void)
{
    //[9:]sel_osc_52m
    BTDIGITAL_REG(0xd0330038) |= (1<<9);
#ifdef BT_SYSTEM_52M
    btdrv_52m_sys_enable();
#endif

//__CLK_GATE_DISABLE__
//CLK_GATE_DISABLE
    BTDIGITAL_REG(0xd0330024) &= (~(1<<5));
    BTDIGITAL_REG(0xd0330024) |= (1<<18);
    BTDIGITAL_REG(0xd0330038) |= (1<<21);
//CLK_GATE_DISABLE end
//reconfig @ btdrv_config_end

    BTDIGITAL_REG_SET_FIELD(0xd0350240, 0xf,  0, 7);
    BTDIGITAL_REG_SET_FIELD(0xd0350240, 0xf,  4, 0);
    BTDIGITAL_REG_SET_FIELD(0xd0350240, 0x4,  8, 4);
    BTDIGITAL_REG_SET_FIELD(0xd0350240, 0xf,  12, 0xa);
    BTDIGITAL_REG_SET_FIELD(0xd0350240, 0xfff1, 16, 0x1);

    BTDIGITAL_REG_SET_FIELD(0xd03502c8, 0x1ff, 0, 0x80);
    BTDIGITAL_REG_SET_FIELD(0xd03502cc, 0x1f, 0, 0x15);

    BTDIGITAL_REG_SET_FIELD(0xd0350360, 0xffff,  0, 0xe040);
    BTDIGITAL_REG_SET_FIELD(0xd0350360, 0xffff, 16, 0x003f);

    BTDIGITAL_REG_SET_FIELD(0xd0350360, 0xf, 22, 2);
    BTDIGITAL_REG_SET_FIELD(0xd0350360, 0xf, 28, 2);

    BTDIGITAL_REG_SET_FIELD(0xd0350340, 0xffffffff,  0, 1);

   // BTDIGITAL_REG_SET_FIELD(0xd0350280, 0xfffffff6, 0, 6);

    BTDIGITAL_REG_SET_FIELD(0xd035031c, 0xffff,  0, 0x4);
    BTDIGITAL_REG_SET_FIELD(0xd035031c, 0xffff, 16, 0x5);

    BTDIGITAL_REG_SET_FIELD(0xd0350320, 0xffff,  0, 0x10);  //fix modulation coefficient : set bit0 to 0 by walker mail 2020/07/31
    BTDIGITAL_REG_SET_FIELD(0xd0350320, 0xffff, 16, 0x32);

    BTDIGITAL_REG(0xd0350284) |= (1<<9) | (1<<25);

    BTDIGITAL_REG_SET_FIELD(0xd0350398, 0xffffffff,  0, 0x6e4ef79c);

    BTDIGITAL_REG_SET_FIELD(0xd0350364, 0xfff8,  0, 0xB948);
    BTDIGITAL_REG_SET_FIELD(0xd0350364, 0xff2f,  16, 0x2e);


    BTDIGITAL_REG_SET_FIELD(0xd035020C, 0xf,  0, 0xc);
    BTDIGITAL_REG_SET_FIELD(0xd035020C, 0xf,  4, 0x2);
    BTDIGITAL_REG_SET_FIELD(0xd035020C, 0x1,  16, 0x1);
    BTDIGITAL_REG_SET_FIELD(0xd035020C, 0xf,  20, 0xf);

    BTDIGITAL_REG_SET_FIELD(0xd0350210, 0xffff,  0, 0x82c);
    BTDIGITAL_REG_SET_FIELD(0xd0350210, 0xffff, 16, 0xf1);


    BTDIGITAL_REG_SET_FIELD(0xd0350300, 0xff, 0, 0x11);
    BTDIGITAL_REG(0xd0350300) &= ~(1<<17);

    //d022 BT core
    BTDIGITAL_REG(0xd02201e8) = (BTDIGITAL_REG(0xd02201e8)&(~0x7c0))|0x380;
    BTDIGITAL_REG(0xd0220470) |=0x40;
    BTDIGITAL_REG_SET_FIELD(0xd02204a8, 0xf, 9, 6);//enlarge fa tx off time 6us

    //BT trx pwrup/dn
    btdrv_btradiowrupdn_set_rtrip_delay(0x0e);
    btdrv_btradiowrupdn_set_rxpwrup(0x57);
    btdrv_btradiowrupdn_set_txpwrdn(0x14);
    btdrv_btradiowrupdn_set_txpwrup(0x40);
    //BLE trx pwrup/dn
    btdrv_bleradiowrupdn_set_rtrip_delay(0x0e);
    btdrv_bleradiowrupdn_set_rxpwrup(0x47);
    btdrv_bleradiowrupdn_set_txpwrdn(0x14);
    btdrv_bleradiowrupdn_set_txpwrup(0x45);
    bt_fa_sync_invert_en_setf(1);
    btdrv_fa_xsco_wrbuf_mask_en_setf(1);
#ifdef __HIGH_TX_POWER__
    BTDIGITAL_REG(0xd0350300) |= (1<<1) | (1<<5);
    //BTDIGITAL_REG(0xd0350308) |= (1<<3) | (1<<13);
    BTDIGITAL_REG_SET_FIELD(0xd0350308, 0xffff, 0, 0x1C07);
    //BTDIGITAL_REG(0xd0350340) = 0x3;
    BTDIGITAL_REG_SET_FIELD(0xd0350340, 0x7, 0, 3);
    //BTDIGITAL_REG(0xd0350344) = 0x6;
    BTDIGITAL_REG_SET_FIELD(0xd0350344, 0xf, 0, 0x7);
#endif

    BTDIGITAL_REG_SET_FIELD(0xd0350300, 0xf, 8, 3);//gsg_den_g_ble
    BTDIGITAL_REG_SET_FIELD(0xd0350300, 0xf, 12, 3);//gsg_den_i_ble
    BTDIGITAL_REG_SET_FIELD(0xd0350300, 0x3ff, 20, 5);//gsg_nom_q_ble

    BTDIGITAL_REG_SET_FIELD(0xd0350308, 0x3ff, 20, 5);//gsg_nom_i_ble

}

void btdrv_ibrt_dual_ant_ctrl_config()
{
#if 1       //P0_1 = fa flag
    BTDIGITAL_REG_SET_FIELD(0xd0220050, 0xffff,  0, 0xd3d3);
    BTDIGITAL_REG(0x40086014) |= (1<<1);
    BTDIGITAL_REG_SET_FIELD(0x40086018, 0xf,  0, 0x5);
    BTDIGITAL_REG_SET_FIELD(0x40086004, 0xf,  4, 0xA);
    BTDIGITAL_REG(0xd0340000) |= (1<<9);
    BTDIGITAL_REG_SET_FIELD(0xd0340000, 0xffff,  16, 0xA362);
#else       //P1_1 = fa flag
    BTDIGITAL_REG_SET_FIELD(0xd0220050, 0xffff,  0, 0xd3d3);
    BTDIGITAL_REG(0x40086014) |= (1<<9);
    BTDIGITAL_REG_SET_FIELD(0x40086018, 0xf,  0, 0x5);
    BTDIGITAL_REG_SET_FIELD(0x40086008, 0xf,  4, 0xA);
    BTDIGITAL_REG(0xd0340000) |= (1<<9);
    BTDIGITAL_REG_SET_FIELD(0xd0340000, 0xffff,  16, 0x2362);
#endif
}

void btdrv_ble_modem_config(void)
{
    ///new corr mode and old corr mode is valid
    btdrv_old_mode_ble_corr_thr(2);

#ifdef BLE_RF_OLD_CORR_MODE
    BTDIGITAL_REG_SET_FIELD(0xd03503a0, 1, 3, 0);
#else
    BTDIGITAL_REG_SET_FIELD(0xd03503a0, 1, 3, 1);
    //[22:16] ble sync position
    BTDIGITAL_REG_SET_FIELD(0xd0220278, 0x7f, 16, 9);
#endif

  //  BTDIGITAL_REG(0xD03503A4) |= (1<<12);
    //[10]: corr_dcc_en
  //  BTDIGITAL_REG(0xD03503CC) &= ~(1<<10);
   // BTDIGITAL_REG(0xD03503CC) &= ~(1<<26);
}

void btdrv_bt_modem_config(void)
{
    BT_DRV_TRACE(1,"%s",__func__);
    //[7]:le_dfe_forceraw
    //[3]:bt_dfe_forceraw
    //[2]: gfo_enpl
    //[1]: gfo_ensw
    //[0]: psd_avgen
    BTDIGITAL_REG_SET_FIELD(0xd0350280, 1, 0, 1);
    BTDIGITAL_REG_SET_FIELD(0xd0350280, 1, 1, 1);
    BTDIGITAL_REG_SET_FIELD(0xd0350280, 1, 2, 1);
    BTDIGITAL_REG_SET_FIELD(0xd0350280, 1, 3, 0);
    BTDIGITAL_REG_SET_FIELD(0xd0350280, 1, 7, 0);

    //[31:22]: te_timeinit_2m
    //[21:16]: before_timeinit
    //[10:0]: te_timeinit
    BTDIGITAL_REG_SET_FIELD(0xd03502c4, 0x7ff,  0, 0x302);
    BTDIGITAL_REG_SET_FIELD(0xd03502c4, 0x3f,  16, 0x3f);
    BTDIGITAL_REG_SET_FIELD(0xd03502c4, 0x3ff, 22, 0x2a);

    //[31:24]: corr_c1
    //[23:16]: corr_c0
    //[2]: force_gfoen
    //[0]: corr_new_en
#ifdef BT_RF_OLD_CORR_MODE
    BTDIGITAL_REG_SET_FIELD(0xd03503a0, 1, 0, 0);
#else
    BTDIGITAL_REG_SET_FIELD(0xd03503a0, 1, 0, 1);
    BTDIGITAL_REG_SET_FIELD(0xd03503a0, 1, 2, 1);
    BTDIGITAL_REG_SET_FIELD(0xd03503a0, 0xff, 16, 0x04);
    BTDIGITAL_REG_SET_FIELD(0xd03503a0, 0xff, 24, 0x1f);
    //[29]: corr_mode_ble
    //[28:25]corr_delay_ble
    //[24]: corr_mode
    //[10]: corr_dcc_en
    //[8]: err_chk_mode_ble
    //[7]: err_chk_mode
    //[6]: sp_adjust_en
    //[5:3]: corr_preamble_ble
    //[2:0]: corr_preamble
    BTDIGITAL_REG_SET_FIELD(0xd03503cc, 7, 0, 4);
    BTDIGITAL_REG_SET_FIELD(0xd03503cc, 7, 3, 2);

    BTDIGITAL_REG_SET_FIELD(0xd03503cc, 1, 6, 1);
    BTDIGITAL_REG_SET_FIELD(0xd03503cc, 1, 7, 1);
    BTDIGITAL_REG_SET_FIELD(0xd03503cc, 1, 8, 1);
    BTDIGITAL_REG_SET_FIELD(0xd03503cc, 1, 10, 1);
    BTDIGITAL_REG_SET_FIELD(0xd03503cc, 1, 24, 1);
    BTDIGITAL_REG_SET_FIELD(0xd03503cc, 0xf, 25, 5);
    BTDIGITAL_REG_SET_FIELD(0xd03503cc, 1, 29, 0);

    btdrv_new_mode_ble_corr_thr(0x40);

    btdrv_new_mode_bt_corr_thr(0x40);


#endif



    //1302:[29:26]:lau_gain
    BTDIGITAL_REG_SET_FIELD(0xd0350364, 0x0f, 26, 0x08);

    //[13:0]: lau_h
    BTDIGITAL_REG_SET_FIELD(0xd0350370, 0x3fff, 0, 0x9EB);
    //[14]: lau_pol_ref
    BTDIGITAL_REG_SET_FIELD(0xd0350370, 0x1, 14, 0);
    //[15]: lau_pol_out
    BTDIGITAL_REG_SET_FIELD(0xd0350370, 0x1, 15, 0);
    //[16-21]: lau_train_time
    BTDIGITAL_REG_SET_FIELD(0xd0350370, 0x3f, 16, 0x33);
    //[22]: lau_mode_2
    BTDIGITAL_REG_SET_FIELD(0xd0350370, 0x1, 22, 0);
    //[26:23]: sample_phase(13M cycle)
    BTDIGITAL_REG_SET_FIELD(0xd0350370, 0xf, 23, 7);
    //[27]: laurent_en
    BTDIGITAL_REG_SET_FIELD(0xd0350370, 1, 27, 0);
    //[28]: freq_fb(eco1:en_gfsklp_ble)
    BTDIGITAL_REG_SET_FIELD(0xd0350370, 1, 28, 1);
    //[29]: freq_obsv(eco1:en_gfsklp)
    BTDIGITAL_REG_SET_FIELD(0xd0350370, 1, 29, 1);
    //[30]:  freq_sign(eco1:lau_header)
    BTDIGITAL_REG_SET_FIELD(0xd0350370, 1, 30, 1);
    // [31]: Laurent_en_ble
    BTDIGITAL_REG_SET_FIELD(0xd0350370, 1, 31, 0);

    //[26:16]: lau_c2 (signed); 1302:[29:16]
    //[10:0]: lau_c1 (signed); 1302:[13:0]
    BTDIGITAL_REG_SET_FIELD(0xd0350374, 0x3fff,  0, 0x0dcd);
    BTDIGITAL_REG_SET_FIELD(0xd0350374, 0x3fff, 16, 0x00a4);

    //1303:[31]: lau_header
    //1302:[29:16]: lau_c2_t
    //1302:[13:0]: lau_c1_t
    BTDIGITAL_REG_SET_FIELD(0xd03503c8, 0x3ff, 0, 0xccd);
    BTDIGITAL_REG_SET_FIELD(0xd03503c8, 0x3ff, 16, 0x84);
    BTDIGITAL_REG_SET_FIELD(0xd03503c8, 1, 31, 1);


    //[7:0]: spwr_corr_th
    BTDIGITAL_REG_SET_FIELD(0xd03503a8, 0xff,  0, 0x60);
    //[15:8]: spwr_corr_th_ble
    BTDIGITAL_REG_SET_FIELD(0xd03503a8, 0xff,  8, 0x62);
    //[23:16]: spwr_th
    BTDIGITAL_REG_SET_FIELD(0xd03503a8, 0xff,  16, 0x60);
    //[31:24]: spwr_th_ble
    BTDIGITAL_REG_SET_FIELD(0xd03503a8, 0xff,  24, 0x80);

    //[3]:mac laurent_en off
    BTDIGITAL_REG_SET_FIELD(0xd0220460, 1, 3, 0);

#ifdef LAURENT_ALGORITHM


    //[27]: laurent_en
    BTDIGITAL_REG_SET_FIELD(0xd0350370, 1, 27, 1);
    //[3]:mac laurent_en off
    BTDIGITAL_REG_SET_FIELD(0xd0220460, 1, 3, 1);

#endif

#ifdef BT_IF_1P05M
    BTDIGITAL_REG_SET_FIELD(0xD0350244, 0xffff, 0, 0x14AD);
    BTDIGITAL_REG_SET_FIELD(0xD0350244, 0xffff, 16, 0x1B92);
#endif
}

void btdrv_config_end(void)
{
#ifdef __CLK_GATE_DISABLE__
    BTDIGITAL_REG(0xD0330024) &= (~(1<<5));
    BTDIGITAL_REG(0xD0330024) |= (1<<18);
  //  BTDIGITAL_REG(0xD0330038) |= (1<<21);
    BTDIGITAL_REG(0xD0330038) &= ~(1<<21);
    BTDIGITAL_REG(0xd0330008) &= ~(1<<7);

#else
    BTDIGITAL_REG(0xD0330024) |= (1<<5);
    BTDIGITAL_REG(0xD0330024) &= ~(1<<18);
    BTDIGITAL_REG(0xD0330038) &= ~(1<<21);
#endif
}

static  void bt_drv_disable_correct_rxwin_after_hec_error(void)
{
    /* BT core will send an RF silenced TX packet to correct the position of
    *  RX window after HEC error occurs.This feature caused the LMP TX missing.
    *  WARNING !!!!Ensure to disable this function forever!!!
    */
    BTDIGITAL_REG_SET_FIELD(0xd02204e8, 1, 31, 0);
}

static void bt_drv_sleep_param_init(void)
{
    uint8_t wait_26m_cycle = LPU_TIMER_US(BT_CMU_26M_READY_TIMEOUT_US);
    BTDIGITAL_REG_SET_FIELD(0xD0330024,0xff, 8, wait_26m_cycle);
    BT_DRV_TRACE(1, "%s wait 26m cyc=%d,twosc=%d", __func__, wait_26m_cycle, BT_CMU_OSC_READY_TIMEOUT_US);
}
extern void bt_drv_reg_op_controller_mem_log_config(void);
void btdrv_config_init(void)
{
    BT_DRV_TRACE(1,"%s",__func__);

    // RF/RAM retention config (RegFile no retention, RAM retention)
    // WARNING: The config might be different for different chipsets
    BTDIGITAL_REG(0xd033005C) = 0x6D8;

    if (btdrv_get_lmp_trace_enable())
    {
        //enable lmp trace
        bt_lmp_record[0] = 1;
        bt_lmp_record[1] = 1;
        btdrv_cfg_tbl[0].is_act= BTDRV_CONFIG_ACTIVE;
        ASSERT((btdrv_cfg_tbl[0].opcode == HCI_DBG_LMP_MESSAGE_RECORD_CMD_OPCODE), "lmp config not match");
    }

    if (btdrv_get_controller_trace_level() != BTDRV_INVALID_TRACE_LEVEL)
    {
        //enable controller trace
        bt_setting_2500i[28] = btdrv_get_controller_trace_level();
    }

    for(uint8_t i=0; i<ARRAY_SIZE(btdrv_cfg_tbl); i++)
    {
        if (btdrv_cfg_tbl[i].opcode == HCI_DBG_SET_BT_SETTING_CMD_OPCODE)
        {
            if(btdrv_cfg_tbl[i].parlen == sizeof(bt_setting_2500i))
            {
                btdrv_send_cmd(btdrv_cfg_tbl[i].opcode,btdrv_cfg_tbl[i].parlen,btdrv_cfg_tbl[i].param);
                btdrv_delay(1);
            }
        }

        if(btdrv_cfg_tbl[i].opcode == HCI_DBG_SET_BT_SETTING_EXT1_CMD_OPCODE)
        {
            if(btdrv_cfg_tbl[i].parlen == sizeof(bt_setting_ext1_2500i))
            {
                btdrv_send_cmd(btdrv_cfg_tbl[i].opcode,btdrv_cfg_tbl[i].parlen,btdrv_cfg_tbl[i].param);
                btdrv_delay(1);
            }
        }
        if(btdrv_cfg_tbl[i].opcode == HCI_DBG_SET_BT_SETTING_EXT2_CMD_OPCODE)
        {
            if(btdrv_cfg_tbl[i].parlen == sizeof(bt_setting_ext2_2500i))
            {
                btdrv_send_cmd(btdrv_cfg_tbl[i].opcode,btdrv_cfg_tbl[i].parlen,btdrv_cfg_tbl[i].param);
                btdrv_delay(1);
            }
        }

        if(btdrv_cfg_tbl[i].opcode == HCI_DBG_SET_BT_SETTING_EXT3_CMD_OPCODE)
        {
            if(btdrv_cfg_tbl[i].parlen == sizeof(bt_setting_ext3_2500i))
            {
                btdrv_send_cmd(btdrv_cfg_tbl[i].opcode,btdrv_cfg_tbl[i].parlen,btdrv_cfg_tbl[i].param);
                btdrv_delay(1);
            }
        }

        //BT other config
        if(btdrv_cfg_tbl[i].is_act == BTDRV_CONFIG_ACTIVE)
        {
            btdrv_send_cmd(btdrv_cfg_tbl[i].opcode,btdrv_cfg_tbl[i].parlen,btdrv_cfg_tbl[i].param);
            btdrv_delay(1);
        }
    }


    //BT registers config
    for(uint8_t i=0; i<ARRAY_SIZE(mem_config_2500i); i++)
    {
        btdrv_write_memory(_32_Bit,mem_config_2500i[i][0],(uint8_t *)&mem_config_2500i[i][1],4);
        btdrv_delay(1);
    }

    btdrv_digital_config_init_2500i();
    btdrv_bt_modem_config();
    btdrv_ble_modem_config();

#ifdef IBRT_DUAL_ANT_CTRL
    btdrv_ibrt_dual_ant_ctrl_config();
#endif

    if(btdrv_get_controller_trace_dump_enable())
    {
        bt_drv_reg_op_controller_mem_log_config();
    }

    btdrv_ble_rx_gain_thr_tbl_set();
#ifdef __EBQ_TEST__
    bt_drv_reg_op_ebq_test_setting();
#endif
    bt_drv_disable_correct_rxwin_after_hec_error();

    bt_drv_sleep_param_init();
}

bool btdrv_is_ecc_enable(void)
{
    bool ret = false;
#ifdef  __FASTACK_ECC_ENABLE__
    ret = true;
#endif
    return ret;
}

////////////////////////////////////////test mode////////////////////////////////////////////




void btdrv_sleep_config(uint8_t sleep_en)
{
    sleep_param[0] = sleep_en;
    btdrv_send_cmd(HCI_DBG_SET_SLEEP_SETTING_CMD_OPCODE,8,sleep_param);
    btdrv_delay(1);
}

void btdrv_feature_default(void)
{
#ifdef __EBQ_TEST__
    const uint8_t feature[] = {0xBF, 0xFE, 0xCF,0xFe,0xdb,0xFF,0x5b,0x87};
#else
    const uint8_t feature[] = {0xBF, 0xeE, 0xCD,0xFe,0xc3,0xFf,0x7b,0x87};
#endif
    btdrv_send_cmd(HCI_DBG_SET_LOCAL_FEATURE_CMD_OPCODE,8,feature);
    btdrv_delay(1);
}

const uint8_t test_mode_addr[6] = {0x77,0x77,0x77,0x77,0x77,0x77};
void btdrv_test_mode_addr_set(void)
{
    return;

    btdrv_send_cmd(HCI_DBG_SET_BD_ADDR_CMD_OPCODE,sizeof(test_mode_addr),test_mode_addr);
    btdrv_delay(1);
}
//HW SPI TRIG

#define REG_SPI_TRIG_SELECT_LINK0_ADDR EM_BT_BT_EXT1_ADDR    //114a+66
#define REG_SPI_TRIG_SELECT_LINK1_ADDR (EM_BT_BT_EXT1_ADDR+BT_EM_SIZE)  //11b8+66
#define REG_SPI_TRIG_NUM_ADDR 0xd0220400
#define REG_SPI0_TRIG_POS_ADDR 0xd0220454
#define REG_SPI1_TRIG_POS_ADDR 0xd0220458

struct SPI_TRIG_NUM_T
{
    uint32_t spi0_txon_num:3;//spi0 number of tx rising edge
    uint32_t spi0_txoff_num:3;//spi0 number of tx falling edge
    uint32_t spi0_rxon_num:2;//spi0 number of rx rising edge
    uint32_t spi0_rxoff_num:2;//spi0 number of rx falling edge
    uint32_t spi0_fast_mode:1;
    uint32_t spi0_gap:4;
    uint32_t hwspi0_en:1;
    uint32_t spi1_txon_num:3;//spi1 number of tx rising edge
    uint32_t spi1_txoff_num:3;//spi1 number of tx falling edge
    uint32_t spi1_rxon_num:2;//spi1 number of rx rising edge
    uint32_t spi1_rxoff_num:2;//spi1 number of rx falling edge
    uint32_t spi1_fast_mode:1;
    uint32_t spi1_gap:4;
    uint32_t hwspi1_en:1;
};

struct SPI_TRIG_POS_T
{
    uint32_t spi_txon_pos:7;
    uint32_t spi_txoff_pos:9;
    uint32_t spi_rxon_pos:7;
    uint32_t spi_rxoff_pos:9;
};

struct spi_trig_data
{
    uint32_t reg;
    uint32_t value;
};

#ifdef BT_FAST_LOCK_ENABLE
static const struct spi_trig_data spi0_trig_data_tbl[] =
{
    //{addr,data([23:0])}
    {0xd0220404,0x10a080},//spi0_trig_txdata1
    {0xd0220408,0x0068ae},//spi0_trig_txdata2
    {0xd022040c,0x00a080},//spi0_trig_txdata3
    {0xd0220410,0x000000},//spi0_trig_txdata4
    {0xd022041c,0x10a080},//spi0_trig_rxdata1
    {0xd0220420,0x006aae},//spi0_trig_rxdata2
    {0xd0220424,0x00a080},//spi0_trig_rxdata3
    {0xd0220428,0x000000},//spi0_trig_rxdata4
    {0xd0220414,0x000000},//spi0_trig_trxdata5
    {0xd0220418,0x000000},//spi0_trig_trxdata6
};
#else
static const struct spi_trig_data spi0_trig_data_tbl[] =
{
    //{addr,data([23:0])}
    {0xd0220404,0x000000},//spi0_trig_txdata1
    {0xd0220408,0x000000},//spi0_trig_txdata2
    {0xd022040c,0x000000},//spi0_trig_txdata3
    {0xd0220410,0x000000},//spi0_trig_txdata4
    {0xd022041c,0x000000},//spi0_trig_rxdata1
    {0xd0220420,0x000000},//spi0_trig_rxdata2
    {0xd0220424,0x000000},//spi0_trig_rxdata3
    {0xd0220428,0x000000},//spi0_trig_rxdata4
    {0xd0220414,0x000000},//spi0_trig_trxdata5
    {0xd0220418,0x000000},//spi0_trig_trxdata6
};
#endif

static const struct spi_trig_data spi1_trig_data_tbl[] =
{
    //{addr,data([23:0])}
    {0xd022042c,0x000000},//spi1_trig_txdata1
    {0xd0220430,0x000000},//spi1_trig_txdata2
    {0xd0220434,0x000000},//spi1_trig_txdata3
    {0xd0220438,0x000000},//spi1_trig_txdata4
    {0xd0220444,0x000000},//spi1_trig_rxdata1
    {0xd0220448,0x000000},//spi1_trig_rxdata2
    {0xd022044c,0x000000},//spi1_trig_rxdata3
    {0xd0220450,0x000000},//spi1_trig_rxdata4
    {0xd022043c,0x000000},//spi1_trig_trxdata5
    {0xd0220440,0x000000},//spi1_trig_trxdata6
};

void btdrv_spi_trig_data_change(uint8_t spi_sel, uint8_t index, uint32_t value)
{
    if(!spi_sel)
    {
        BTDIGITAL_REG(spi0_trig_data_tbl[index].reg) = value & 0xFFFFFF;
    }
    else
    {
        BTDIGITAL_REG(spi1_trig_data_tbl[index].reg) = value & 0xFFFFFF;
    }
}

void btdrv_spi_trig_data_set(uint8_t spi_sel)
{
    if(!spi_sel)
    {
        for(uint8_t i = 0; i < ARRAY_SIZE(spi0_trig_data_tbl); i++)
        {
            BTDIGITAL_REG(spi0_trig_data_tbl[i].reg) = spi0_trig_data_tbl[i].value;
        }
    }
    else
    {
        for(uint8_t i = 0; i < ARRAY_SIZE(spi1_trig_data_tbl); i++)
        {
            BTDIGITAL_REG(spi1_trig_data_tbl[i].reg) = spi1_trig_data_tbl[i].value;
        }
    }
}

void btdrv_spi_trig_num_set(uint8_t spi_sel, struct SPI_TRIG_NUM_T *spi_trig_num)
{
    uint8_t tx_onoff_total_num;
    uint8_t rx_onoff_total_num;

    if(!spi_sel)
    {
        tx_onoff_total_num = spi_trig_num->spi0_txon_num + spi_trig_num->spi0_txoff_num;
        rx_onoff_total_num = spi_trig_num->spi0_rxon_num + spi_trig_num->spi0_rxoff_num;
    }
    else
    {
        tx_onoff_total_num = spi_trig_num->spi1_txon_num + spi_trig_num->spi1_txoff_num;
        rx_onoff_total_num = spi_trig_num->spi1_rxon_num + spi_trig_num->spi1_rxoff_num;
    }
    ASSERT((tx_onoff_total_num <= 6), "spi trig tx_onoff_total_num>6");
    ASSERT((rx_onoff_total_num <= 6), "spi trig rx_onoff_total_num>6");

    BTDIGITAL_REG(REG_SPI_TRIG_NUM_ADDR) = *(uint32_t *)spi_trig_num;
}

void btdrv_spi_trig_pos_set(uint8_t spi_sel, struct SPI_TRIG_POS_T *spi_trig_pos)
{
    if(!spi_sel)
    {
        BTDIGITAL_REG(REG_SPI0_TRIG_POS_ADDR) = *(uint32_t *)spi_trig_pos;
    }
    else
    {
        BTDIGITAL_REG(REG_SPI1_TRIG_POS_ADDR) = *(uint32_t *)spi_trig_pos;
    }
}

void btdrv_set_spi_trig_pos_enable(void)
{
    struct SPI_TRIG_POS_T spi0_trig_pos;
    struct SPI_TRIG_POS_T spi1_trig_pos;

    spi0_trig_pos.spi_txon_pos = SPI0_TXON_POS;
    spi0_trig_pos.spi_txoff_pos = 0;
    spi0_trig_pos.spi_rxon_pos = SPI0_RXON_POS;
    spi0_trig_pos.spi_rxoff_pos = 0;

    spi1_trig_pos.spi_txon_pos = 0;
    spi1_trig_pos.spi_txoff_pos = 0;
    spi1_trig_pos.spi_rxon_pos = 0;
    spi1_trig_pos.spi_rxoff_pos = 0;

    btdrv_spi_trig_pos_set(0,&spi0_trig_pos);
    btdrv_spi_trig_pos_set(1,&spi1_trig_pos);
}

void btdrv_clear_spi_trig_pos_enable(void)
{
    int sRet = 0;

    struct SPI_TRIG_POS_T spi0_trig_pos;
    struct SPI_TRIG_POS_T spi1_trig_pos;

    sRet = memset_s(&spi0_trig_pos, sizeof(struct SPI_TRIG_POS_T), 0, sizeof(struct SPI_TRIG_POS_T));
    if (sRet){
        BT_DRV_TRACE(1, "%s line:%d sRet:%d", __func__, __LINE__, sRet);
    }
    sRet = memset_s(&spi1_trig_pos, sizeof(struct SPI_TRIG_POS_T), 0, sizeof(struct SPI_TRIG_POS_T));
    if (sRet){
        BT_DRV_TRACE(1, "%s line:%d sRet:%d", __func__, __LINE__, sRet);
    }

    btdrv_spi_trig_pos_set(0,&spi0_trig_pos);
    btdrv_spi_trig_pos_set(1,&spi1_trig_pos);
}

void btdrv_set_spi_trig_num_enable(void)
{
    struct SPI_TRIG_NUM_T spi_trig_num;

    spi_trig_num.spi0_txon_num = SPI0_TXON_NUM;
    spi_trig_num.spi0_txoff_num = 0;
    spi_trig_num.spi0_rxon_num = SPI0_RXON_NUM;
    spi_trig_num.spi0_rxoff_num = 0;
    spi_trig_num.spi0_fast_mode = 0;
    spi_trig_num.spi0_gap = 4;
    spi_trig_num.hwspi0_en = SPI0_EN;

    spi_trig_num.spi1_txon_num = 0;
    spi_trig_num.spi1_txoff_num = 0;
    spi_trig_num.spi1_rxon_num = 0;
    spi_trig_num.spi1_rxoff_num = 0;
    spi_trig_num.spi1_fast_mode = 0;
    spi_trig_num.spi1_gap = 0;
    spi_trig_num.hwspi1_en = 0;

    btdrv_spi_trig_num_set(0,&spi_trig_num);
    btdrv_spi_trig_num_set(1,&spi_trig_num);
}

void btdrv_spi_trig_init(void)
{
    //spi number set
    btdrv_set_spi_trig_num_enable();
    //spi position set
    btdrv_set_spi_trig_pos_enable();
    //spi data set
    btdrv_spi_trig_data_set(0);
    btdrv_spi_trig_data_set(1);
}

void btdrv_spi_trig_select(uint8_t link_id, bool spi_set)
{
    BTDIGITAL_BT_EM(EM_BT_BT_EXT1_ADDR+link_id*BT_EM_SIZE) |= (spi_set<<14);
}

uint8_t btdrv_get_spi_trig_enable(uint8_t spi_sel)
{
    if(!spi_sel)
    {
        return ((BTDIGITAL_REG(REG_SPI_TRIG_NUM_ADDR) & 0x8000) >> 15);
    }
    else
    {
        return ((BTDIGITAL_REG(REG_SPI_TRIG_NUM_ADDR) & 0x80000000) >> 31);
    }
}

void btdrv_set_spi_trig_enable(uint8_t spi_sel)
{
    if(!spi_sel)
    {
        BTDIGITAL_REG(REG_SPI_TRIG_NUM_ADDR) |= (1<<15);//spi0
    }
    else
    {
        BTDIGITAL_REG(REG_SPI_TRIG_NUM_ADDR) |= (1<<31);//spi1
    }
}

void btdrv_clear_spi_trig_enable(uint8_t spi_sel)
{
    if(!spi_sel)
    {
        BTDIGITAL_REG(REG_SPI_TRIG_NUM_ADDR) &= ~0x8000;
    }
    else
    {
        BTDIGITAL_REG(REG_SPI_TRIG_NUM_ADDR) &= ~0x80000000;
    }
}

bool btdrv_get_lmp_trace_enable(void)
{
    return g_lmp_trace_enable;
}
void btdrv_set_lmp_trace_enable(void)
{
    g_lmp_trace_enable = true;
}
void btdrv_set_controller_trace_enable(uint8_t trace_level)
{
    g_controller_trace_level = trace_level;
}

uint8_t btdrv_get_controller_trace_level(void)
{
    return g_controller_trace_level;
}

uint8_t btdrv_get_ble_idx_in_testmode(void)
{
    return BLE_IDX_IN_TESTMODE;
}