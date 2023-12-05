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
#include <stdio.h>
#include "string.h"
#include "plat_types.h"
#include "plat_addr_map.h"
#include "hal_i2c.h"
#include "hal_uart.h"
#include "bt_drv.h"
#include "bt_drv_internal.h"
#include "bt_drv_2500i_internal.h"
#include "bt_drv_reg_op.h"
#include "bt_drv_interface.h"
#include "hal_timer.h"
#include "hal_intersys.h"
#include "hal_trace.h"
#include "hal_psc.h"
#include "hal_cmu.h"
#include "hal_sysfreq.h"
#include "hal_chipid.h"
#include "hal_iomux.h"
#include "hal_gpio.h"
#include "pmu.h"
#include "nvrecord_dev.h"
#include "tgt_hardware.h"
#include "besbt_string.h"
#include "hal_btdump.h"
#include "iqcorrect.h"

extern "C" void hal_iomux_set_controller_log(void);
bool btdrv_dut_mode_enable = false;

static volatile uint32_t btdrv_tx_flag = 1;
void btdrv_tx(const unsigned char *data, unsigned int len)
{
    BT_DRV_TRACE(0,"tx");
    btdrv_tx_flag = 1;
}

void btdrv_dut_accessible_mode_manager(const unsigned char *data);

static unsigned int btdrv_rx(const unsigned char *data, unsigned int len)
{
    hal_intersys_stop_recv(HAL_INTERSYS_ID_0);

    BT_DRV_TRACE(2,"%s len:%d", __func__, len);
    BT_DRV_DUMP("%02x ", data, len>7?7:len);
    btdrv_dut_accessible_mode_manager(data);
    hal_intersys_start_recv(HAL_INTERSYS_ID_0);

    return len;
}

void btdrv_SendData(const uint8_t *buff,uint8_t len)
{
    btdrv_tx_flag = 0;

    hal_intersys_send(HAL_INTERSYS_ID_0, HAL_INTERSYS_MSG_HCI, buff, len);
    BT_DRV_TRACE(1,"%s", __func__);
    BT_DRV_DUMP("%02x ", buff, len);
    while( (btdrv_dut_mode_enable==0) && btdrv_tx_flag == 0);
}

////open intersys interface for hci data transfer
static bool hci_has_opened = false;

void btdrv_hciopen(void)
{
    int ret = 0;

    if (hci_has_opened)
    {
        return;
    }

    hci_has_opened = true;

    ret = hal_intersys_open(HAL_INTERSYS_ID_0, HAL_INTERSYS_MSG_HCI, btdrv_rx, btdrv_tx, false);

    if (ret)
    {
        BT_DRV_TRACE(0,"Failed to open intersys");
        return;
    }

    hal_intersys_start_recv(HAL_INTERSYS_ID_0);
}

////open intersys interface for hci data transfer
void btdrv_hcioff(void)
{
    if (!hci_has_opened)
    {
        return;
    }
    hci_has_opened = false;

    hal_intersys_close(HAL_INTERSYS_ID_0,HAL_INTERSYS_MSG_HCI);
}

/*  btdrv power on or off the bt controller*/
void btdrv_poweron(uint8_t en)
{
    //power on bt controller
    if(en)
    {
        hal_psc_bt_enable();
        hal_cmu_bt_clock_enable();
        hal_cmu_bt_reset_clear();
        hal_cmu_bt_module_init();
     //   hal_cmu_bt_sys_force_ram_on();
        btdrv_delay(10);
        // BTDM mode 4.2
        BTDIGITAL_REG(0xC0000050) = 0x42;
        btdrv_delay(100);
    }
    else
    {
        btdrv_delay(10);
        hal_cmu_bt_reset_set();
        hal_cmu_bt_clock_disable();
        hal_psc_bt_disable();
    }
}

void btdrv_rf_init_ext(void)
{
    unsigned int xtal_fcap;

    if (!nvrec_dev_get_xtal_fcap(&xtal_fcap))
    {
        btdrv_rf_init_xtal_fcap(xtal_fcap);
        btdrv_delay(1);
        BT_DRV_TRACE(2,"%s 0xc2=0x%x", __func__, xtal_fcap);
    }
    else
    {
        btdrv_rf_init_xtal_fcap(DEFAULT_XTAL_FCAP);
        BT_DRV_TRACE(1,"%s failed", __func__);
    }
}

void btdrv_2500i_rccal(void)
{
    uint16_t value;
    uint16_t value_tmp;
    uint16_t val_tmp1;
    uint16_t val_tmp2;

    btdrv_write_rf_reg(0x80, 0xa010);
    btdrv_write_rf_reg(0xc4, 0xffff);//[5:4]=11,open rcosc bias
    btdrv_write_rf_reg(0x80, 0xa000);

    btdrv_write_rf_reg(0xb3,0x33f3);//[9:8]=11,pwup rcosc

    btdrv_delay(1);

    BTDIGITAL_REG(0xd02201e4) = 0x00000000;
    btdrv_delay(10);
    BTDIGITAL_REG(0xd02201e4) = 0x000a0080;
    btdrv_delay(10);

    btdrv_write_rf_reg(0x80, 0xa010);
    btdrv_write_rf_reg(0xd6,0xf858);//[15]=1,enable clk counter
    btdrv_write_rf_reg(0x80, 0xa000);

    btdrv_delay(10);

    btdrv_read_rf_reg(0xc0,&value);
    BT_DRV_TRACE(1,"btdrv_rccal 0xc0 value:%x\n",value);

    btdrv_read_rf_reg(0x8b,&val_tmp1);
    BT_DRV_TRACE(1,"0x8b val_tmp1=%x\n",val_tmp1);

    btdrv_read_rf_reg(0x8d,&val_tmp2);
    BT_DRV_TRACE(1,"0x8d val_tmp2=%x\n",val_tmp2);

    value_tmp = value & 0x0fff;
    if((value_tmp < 0x0ff0)&&(value_tmp > 0x0200)&&((value|0xefff)==0xffff))
    {
        BT_DRV_TRACE(0,"0xc0 0x200 < value < 0xff0 done \n");
        btdrv_write_rf_reg(0x8b,(((( 0x7c4 * 1000 / (value & 0x0fff)) * 0x90 / 1000) << 8) | (val_tmp1 & 0x00ff)));
        BT_DRV_TRACE(1,"0x8b:%x\n",(((( 0x7c4 * 1000 / (value & 0x0fff)) * 0x90 / 1000) << 8) | (val_tmp1 & 0x00ff)));
        btdrv_read_rf_reg(0x8b,&val_tmp1);
        BT_DRV_TRACE(1,"chk 0x8b val_tmp1=%x\n",val_tmp1);
        btdrv_write_rf_reg(0x8d,((((0x7c4 * 1000 / (value & 0x0fff)) *  0x28 / 1000) << 10) | (val_tmp2 & 0x03ff)));
        BT_DRV_TRACE(1,"0x8d:%x\n",((((0x7c4 * 1000 / (value & 0x0fff)) *  0x28 / 1000) << 10) | (val_tmp2 & 0x03ff)));
        btdrv_read_rf_reg(0x8d,&val_tmp2);
        BT_DRV_TRACE(1,"chk 0x8d val_tmp2=%x\n",val_tmp2);
    }
    else
    {
        btdrv_write_rf_reg(0x8b,((0x9c << 8) | (val_tmp1 & 0x00ff)));
        BT_DRV_TRACE(1,"0x8b:%x\n",((0x9c << 8) | (val_tmp1 & 0x00ff)));
        btdrv_write_rf_reg(0x8d,((0x28 << 10) | (val_tmp2 & 0x03ff)));
        BT_DRV_TRACE(1,"0x8d:%x\n",((0x28 << 10) | (val_tmp2 & 0x03ff)));
    }

    btdrv_read_rf_reg(0x8d,&val_tmp2);
    val_tmp2 &= ~(1<<6);
    val_tmp2 |= (1<<7);
    btdrv_write_rf_reg(0x8d,val_tmp2);

    BTDIGITAL_REG(0xd02201e4) = 0x00000000;

    btdrv_write_rf_reg(0x80, 0xa010);
    btdrv_write_rf_reg(0xc4, 0xffcf);//[5:4]=00,close rcosc bias
    btdrv_write_rf_reg(0x80, 0xa000);

    btdrv_write_rf_reg(0xb3,0x30f3);//[9:8]=00,pwup rcosc
}


#ifdef __PWR_FLATNESS__
#define PWR_FLATNESS_CONST_VAL                         0xF
void btdrv_2500i_channel_pwr_flatness(void)
{
    uint16_t read_value = 0;
    uint16_t tmp_val = 0;

    btdrv_read_rf_reg(0xc0,&read_value);
    BT_DRV_TRACE(1,"btdrv_2500i_channel_pwr_flatness 0xc0=%x\n",read_value);

    read_value = (read_value & 0x0f00)>>8;//[11:8]
    int16_t calib_val = PWR_FLATNESS_CONST_VAL - read_value;
    if(calib_val<0)
    {
        BT_DRV_TRACE(2,"calib_val<0 const_val=%d,read_val=%x",PWR_FLATNESS_CONST_VAL,read_value);
        btdrv_read_rf_reg(0x92,&tmp_val);
        tmp_val &= 0xf0ff;//[11:8]
        BT_DRV_TRACE(1,"0x92=%x\n",tmp_val);
        btdrv_write_rf_reg(0x92,tmp_val);
        return;
    }
    else
    {
        BT_DRV_TRACE(2,"const_val=%d,calib_val =%x",PWR_FLATNESS_CONST_VAL,calib_val);
    }

    //write calibrated value into 0x92 register
    btdrv_read_rf_reg(0x92,&tmp_val);
    tmp_val &= 0xf0ff;
    tmp_val |= ((calib_val & 0xffff)<<8);
    BT_DRV_TRACE(1,"write reg 0x92 val=%x",tmp_val);
    btdrv_write_rf_reg(0x92,tmp_val);
}
#endif

void btdrv_enable_jtag(void)
{
    *(uint32_t*)0x400000F8 &= 0x7FFFFFFF;//clear bit31

    hal_iomux_set_jtag();
    hal_cmu_jtag_enable();
    hal_cmu_jtag_clock_enable();
}

void btdrv_enable_rf_sw(int rx_on, int tx_on)
{
    hal_iomux_set_bt_rf_sw(rx_on, tx_on);
    //maybe affect the use of Tport
    BTDIGITAL_REG(0x40086004) =(BTDIGITAL_REG(0x40086004) & ~0xF) | 0x6;
    BTDIGITAL_REG(0xD0340020) |= 1<<5;
}

///start active bt controller

//#define BT_DRV_ENABLE_LMP_TRACE
#ifdef RX_IQ_CAL
extern uint32_t bt_rand_seed;
#endif 
void btdrv_start_bt(void)
{
    hal_sysfreq_req(HAL_SYSFREQ_USER_BT, HAL_CMU_FREQ_26M);

#if INTERSYS_DEBUG
#ifdef BT_DRV_ENABLE_LMP_TRACE
    btdrv_trace_config(BT_CONTROLER_TRACE_TYPE_INTERSYS   |
                       BT_CONTROLER_TRACE_TYPE_CONTROLLER |
                       BT_CONTROLER_FILTER_TRACE_TYPE_A2DP_STREAM |
                       BT_CONTROLER_TRACE_TYPE_LMP_TRACE |
                       BT_CONTROLER_TRACE_TYPE_ACL_PACKET);
#else
    btdrv_trace_config(BT_CONTROLER_TRACE_TYPE_INTERSYS   |
                       BT_CONTROLER_TRACE_TYPE_CONTROLLER |
                       BT_CONTROLER_FILTER_TRACE_TYPE_A2DP_STREAM |
                       BT_CONTROLER_TRACE_TYPE_ACL_PACKET);
#endif
#endif

#if defined(BLE_ONLY_ENABLED)
    btdrv_enable_sleep_checker(false);
#else
    btdrv_enable_sleep_checker(false);
#endif

    hal_iomux_ispi_access_enable(HAL_IOMUX_ISPI_MCU_RF);

#ifndef NO_SLEEP
    pmu_sleep_en(0);
#endif

    bt_drv_reg_op_global_symbols_init();
    btdrv_poweron(BT_POWERON);

    btdrv_hciopen();

    btdrv_rf_init();


    btdrv_txpower_calib();
    btdrv_tx_pwr_cal_enhanced();
    bt_drv_tx_pwr_init();

    btdrv_rf_init_ext();
    btdrv_config_init();

#ifdef RX_IQ_CAL
    if(!btdrv_freqcalib_mode_get())
    {
        //can not sleep when switching frequency points
        //btdrv_sleep_config(0);
        hal_btdump_clk_enable();
        btdrv_rx_iq_cal();
        hal_btdump_clk_disable();
        //btdrv_sleep_config(1);
    }
#endif

    bt_drv_bbpll_cal();

    //rom patch init
    btdrv_ins_patch_init();
    btdrv_patch_en(1);
#ifdef RX_IQ_CAL    
    bt_drv_reg_op_set_btclk_seed(bt_rand_seed);
#endif
    //btdrv_2500i_rccal();
#ifdef __PWR_FLATNESS__
    btdrv_2500i_channel_pwr_flatness();
#endif

#if defined(BT_EXT_LNA)||defined(BT_EXT_PA)
    int LNA_flag = 0,PA_flag = 0;
#ifdef BT_EXT_LNA
    LNA_flag = 1;
#endif
#ifdef BT_EXT_PA
    PA_flag = 1;
#endif
    btdrv_enable_rf_sw(LNA_flag,PA_flag);
#endif
    bt_drv_reg_op_dgb_link_gain_ctrl_init();

#ifdef BT_FAST_LOCK_ENABLE
    btdrv_fast_lock_config(FAST_LOCK_ENABLE);
#else
    btdrv_fast_lock_config(FAST_LOCK_DISABLE);
#endif

#ifdef __NEW_SWAGC_MODE__
    btdrv_switch_agc_mode(BT_IDLE_MODE);
#endif
    bt_drv_reg_op_rssi_adjust_init(true);

    /*reg controller crash dump*/
    hal_trace_crash_dump_register(HAL_TRACE_CRASH_DUMP_MODULE_BT, btdrv_btc_fault_dump);

#ifdef BT_UART_LOG
    uint16_t cmd_filter_buf[1] = {HCI_HOST_NB_CMP_PKTS_CMD_OPCODE};
    uint8_t evt_filter_buf[1] = {HCI_NB_CMP_PKTS_EVT_CODE};
    bt_drv_reg_op_config_controller_log(TRC_DEFAULT_TYPE, cmd_filter_buf, ARRAY_SIZE(cmd_filter_buf), evt_filter_buf, ARRAY_SIZE(evt_filter_buf));
#endif

#ifndef NO_SLEEP
    pmu_sleep_en(1);
#endif

    btdrv_config_end();

    btdrv_hcioff();

    hal_iomux_ispi_access_enable(HAL_IOMUX_ISPI_MCU_RF);
    hal_analogif_rf_access_config(HAL_ANALOGIF_RF_ACCESS_NO_PAGE);

    hal_sysfreq_req(HAL_SYSFREQ_USER_BT, HAL_CMU_FREQ_32K);

#ifdef PCM_PRIVATE_DATA_FLAG
    bt_drv_reg_op_sco_pri_data_init();
#endif
}

const uint8_t hci_cmd_enable_dut[] =
{
    0x01,0x03, 0x18, 0x00
};
const uint8_t hci_cmd_enable_allscan[] =
{
    0x01, 0x1a, 0x0c, 0x01, 0x03
};
const uint8_t hci_cmd_disable_scan[] =
{
    0x01, 0x1a, 0x0c, 0x01, 0x00
};
const uint8_t hci_cmd_enable_pagescan[] =
{
    0x01, 0x1a, 0x0c, 0x01, 0x02
};
const uint8_t hci_cmd_autoaccept_connect[] =
{
    0x01,0x05, 0x0c, 0x03, 0x02, 0x00, 0x02
};
const uint8_t hci_cmd_hci_reset[] =
{
    0x01,0x03,0x0c,0x00
};

const uint8_t hci_cmd_nonsig_tx_dh1_pn9_t0[] =
{
    0x01, 0x87, 0xfc, 0x1c, 0x00, 0xe8, 0x03, 0x00, 0x00, 0x00, 0x00, 0x06, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x01, 0x00, 0x04, 0x04, 0x1b, 0x00,
    0x00, 0x00, 0x00, 0x00, 0xff, 0xff, 0xff, 0xff
};
const uint8_t hci_cmd_nonsig_tx_2dh1_pn9_t0[] =
{
    0x01, 0x87, 0xfc, 0x1c, 0x00, 0xe8, 0x03, 0x00, 0x00, 0x00, 0x00, 0x06, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x01, 0x01, 0x04, 0x04, 0x36, 0x00,
    0x00, 0x00, 0x00, 0x00, 0xff, 0xff, 0xff, 0xff
};
const uint8_t hci_cmd_nonsig_tx_3dh1_pn9_t0[] =
{
    0x01, 0x87, 0xfc, 0x1c, 0x00, 0xe8, 0x03, 0x00, 0x00, 0x00, 0x00, 0x06, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x01, 0x01, 0x08, 0x04, 0x53, 0x00,
    0x00, 0x00, 0x00, 0x00, 0xff, 0xff, 0xff, 0xff
};
const uint8_t hci_cmd_nonsig_tx_2dh3_pn9_t0[] =
{
    0x01, 0x87, 0xfc, 0x1c, 0x00, 0xe8, 0x03, 0x00, 0x00, 0x00, 0x00, 0x06, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x01, 0x01, 0x0a, 0x04, 0x6f, 0x01,
    0x00, 0x00, 0x00, 0x00, 0xff, 0xff, 0xff, 0xff
};
const uint8_t hci_cmd_nonsig_tx_3dh3_pn9_t0[] =
{
    0x01, 0x87, 0xfc, 0x1c, 0x00, 0xe8, 0x03, 0x00, 0x00, 0x00, 0x00, 0x06, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x01, 0x01, 0x0b, 0x04, 0x28, 0x02,
    0x00, 0x00, 0x00, 0x00, 0xff, 0xff, 0xff, 0xff
};

const uint8_t hci_cmd_nonsig_rx_dh1_pn9_t0[] =
{
    0x01, 0x87, 0xfc, 0x1c, 0x01, 0xe8, 0x03, 0x00, 0x00, 0x00, 0x00, 0x06, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x01, 0x00, 0x04, 0x00, 0x1b, 0x00,
    0x00, 0x00, 0x00, 0x00, 0xff, 0xff, 0xff, 0xff
};
const uint8_t hci_cmd_nonsig_rx_2dh1_pn9_t0[] =
{
    0x01, 0x87, 0xfc, 0x1c, 0x01, 0xe8, 0x03, 0x00, 0x00, 0x00, 0x00, 0x06, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x01, 0x01, 0x04, 0x00, 0x36, 0x00,
    0x00, 0x00, 0x00, 0x00, 0xff, 0xff, 0xff, 0xff
};
const uint8_t hci_cmd_nonsig_rx_3dh1_pn9_t0[] =
{
    0x01, 0x87, 0xfc, 0x1c, 0x01, 0xe8, 0x03, 0x00, 0x00, 0x00, 0x00, 0x06, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x01, 0x01, 0x08, 0x00, 0x53, 0x00,
    0x00, 0x00, 0x00, 0x00, 0xff, 0xff, 0xff, 0xff
};
const uint8_t hci_cmd_nonsig_rx_2dh3_pn9_t0[] =
{
    0x01, 0x87, 0xfc, 0x1c, 0x01, 0xe8, 0x03, 0x00, 0x00, 0x00, 0x00, 0x06, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x01, 0x01, 0x0a, 0x00, 0x6f, 0x01,
    0x00, 0x00, 0x00, 0x00, 0xff, 0xff, 0xff, 0xff
};
const uint8_t hci_cmd_nonsig_rx_3dh3_pn9_t0[] =
{
    0x01, 0x87, 0xfc, 0x1c, 0x01, 0xe8, 0x03, 0x00, 0x00, 0x00, 0x00, 0x06, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x01, 0x01, 0x0b, 0x00, 0x28, 0x02,
    0x00, 0x00, 0x00, 0x00, 0xff, 0xff, 0xff, 0xff
};

//vco test
const uint8_t hci_cmd_start_bt_vco_test[] =
{
    0x01, 0xaa, 0xfc, 0x02, 0x00,0x02
};
const uint8_t hci_cmd_stop_bt_vco_test[] =
{
    0x01, 0xaa, 0xfc, 0x02, 0x00,0x04
};

const uint8_t hci_cmd_enable_ibrt_test[] =
{
    0x01, 0xb4, 0xfc, 0x01, 0x01
};

const uint8_t hci_cmd_set_ibrt_mode[] =
{
    0x01, 0xa2, 0xfc, 0x02, 0x01,0x00
};

void bt_drv_reg_op_init_swagc_3m_thd(void);
void btdrv_testmode_start(void)
{
#ifdef __NEW_SWAGC_MODE__
    btdrv_switch_agc_mode(BT_TEST_MODE);
#endif
    bt_drv_reg_op_init_swagc_3m_thd();

    bt_drv_reg_op_init_nosync_info();

    bt_drv_tx_pwr_init_for_testmode();
#ifdef FORCE_NOSIGNALINGMODE
    BTDIGITAL_REG_SET_FIELD(0xd02201e8, 0x1,  0, 0);    //close second rf spi in NOSIGNALINGMODE
#endif
    bt_drv_reg_op_set_nosig_sch_flag(1);
}

void btdrv_write_localinfo(const char *name, uint8_t len, const uint8_t *addr)
{
    int sRet = 0;

    uint8_t hci_cmd_write_addr[5+6] =
    {
        0x01, 0x72, 0xfc, 0x07, 0x00
    };

    uint8_t hci_cmd_write_name[248+4] =
    {
        0x01, 0x13, 0x0c, 0xF8
    };
    sRet = memset_s(&hci_cmd_write_name[4], sizeof(hci_cmd_write_name)-4, 0, sizeof(hci_cmd_write_name)-4);
    if (sRet){
        BT_DRV_TRACE(1, "%s line:%d sRet:%d", __func__, __LINE__, sRet);
    }
    sRet = memcpy_s(&hci_cmd_write_name[4], len, name, len);
    if (sRet){
        BT_DRV_TRACE(1, "%s line:%d sRet:%d", __func__, __LINE__, sRet);
    }
    btdrv_SendData(hci_cmd_write_name, sizeof(hci_cmd_write_name));
    btdrv_delay(50);
    memcpy(&hci_cmd_write_addr[5], addr, 6);
    btdrv_SendData(hci_cmd_write_addr, sizeof(hci_cmd_write_addr));
    btdrv_delay(20);
}

void btdrv_enable_dut(void)
{
    btdrv_SendData(hci_cmd_enable_dut, sizeof(hci_cmd_enable_dut));
    btdrv_delay(20);
    btdrv_SendData(hci_cmd_enable_allscan, sizeof(hci_cmd_enable_allscan));
    btdrv_delay(20);
    btdrv_SendData(hci_cmd_autoaccept_connect, sizeof(hci_cmd_autoaccept_connect));
    btdrv_delay(20);
    btdrv_dut_mode_enable = true;
}

void btdrv_enable_ibrt_test(void)
{
    btdrv_SendData(hci_cmd_enable_ibrt_test, sizeof(hci_cmd_enable_ibrt_test));
    btdrv_delay(20);
    btdrv_SendData(hci_cmd_set_ibrt_mode, sizeof(hci_cmd_set_ibrt_mode));
    btdrv_delay(20);
}

void btdrv_connect_ibrt_device(uint8_t *addr)
{
    uint8_t hci_cmd_connect_device[17] =
    {
        0x01, 0x05, 0x04, 0x0D, 0xab, 0x89, 0x67, 0x45, 0x23, 0x01, 0x18, 0xCC,
        0x01, 0x00, 0x00, 0x00, 0x00
    };
    memcpy(&hci_cmd_connect_device[4],addr,6);
    btdrv_SendData(hci_cmd_connect_device, sizeof(hci_cmd_connect_device));
    btdrv_delay(50);

}

void btdrv_disable_scan(void)
{
    btdrv_SendData(hci_cmd_disable_scan, sizeof(hci_cmd_disable_scan));
    btdrv_delay(20);
}

static uint32_t dut_connect_status = DUT_CONNECT_STATUS_DISCONNECTED;

uint32_t btdrv_dut_get_connect_status(void)
{
    return dut_connect_status;
}

void btdrv_dut_accessible_mode_manager(const unsigned char *data)
{
    if(btdrv_dut_mode_enable)
    {
        if(data[0]==0x04&&data[1]==0x03&&data[2]==0x0b&&data[3]==0x00)
        {
#ifdef __IBRT_IBRT_TESTMODE__
            if(memcmp(&data[6],bt_addr,6))
            {
                btdrv_disable_scan();
            }
#else
            btdrv_disable_scan();
#endif
            dut_connect_status = DUT_CONNECT_STATUS_CONNECTED;
        }
        else if(data[0]==0x04&&data[1]==0x05&&data[2]==0x04&&data[3]==0x00)
        {
            btdrv_enable_dut();
            dut_connect_status = DUT_CONNECT_STATUS_DISCONNECTED;
        }
    }
}

void btdrv_hci_reset(void)
{
    btdrv_SendData(hci_cmd_hci_reset, sizeof(hci_cmd_hci_reset));
    btdrv_delay(350);
}

void btdrv_enable_nonsig_tx(uint8_t index)
{
    BT_DRV_TRACE(1,"%s\n", __func__);

    if (index == 0)
        btdrv_SendData(hci_cmd_nonsig_tx_2dh1_pn9_t0, sizeof(hci_cmd_nonsig_tx_2dh1_pn9_t0));
    else if (index == 1)
        btdrv_SendData(hci_cmd_nonsig_tx_3dh1_pn9_t0, sizeof(hci_cmd_nonsig_tx_3dh1_pn9_t0));
    else if (index == 2)
        btdrv_SendData(hci_cmd_nonsig_tx_2dh3_pn9_t0, sizeof(hci_cmd_nonsig_tx_2dh1_pn9_t0));
    else if (index == 3)
        btdrv_SendData(hci_cmd_nonsig_tx_3dh3_pn9_t0, sizeof(hci_cmd_nonsig_tx_3dh1_pn9_t0));
    else
        btdrv_SendData(hci_cmd_nonsig_tx_dh1_pn9_t0, sizeof(hci_cmd_nonsig_tx_dh1_pn9_t0));

    btdrv_delay(20);

}

void btdrv_enable_nonsig_rx(uint8_t index)
{
    BT_DRV_TRACE(1,"%s\n", __func__);

    if (index == 0)
        btdrv_SendData(hci_cmd_nonsig_rx_2dh1_pn9_t0, sizeof(hci_cmd_nonsig_rx_2dh1_pn9_t0));
    else if (index == 1)
        btdrv_SendData(hci_cmd_nonsig_rx_3dh1_pn9_t0, sizeof(hci_cmd_nonsig_rx_3dh1_pn9_t0));
    else if (index == 2)
        btdrv_SendData(hci_cmd_nonsig_rx_2dh3_pn9_t0, sizeof(hci_cmd_nonsig_rx_2dh1_pn9_t0));
    else if (index == 3)
        btdrv_SendData(hci_cmd_nonsig_rx_3dh3_pn9_t0, sizeof(hci_cmd_nonsig_rx_3dh1_pn9_t0));
    else
        btdrv_SendData(hci_cmd_nonsig_rx_dh1_pn9_t0, sizeof(hci_cmd_nonsig_rx_dh1_pn9_t0));

    btdrv_delay(20);
}

static bool btdrv_vco_test_running = false;
static unsigned short vco_test_reg_val_b6 = 0;
static unsigned short vco_test_reg_val_1f3 = 0;
#ifdef VCO_TEST_TOOL
static unsigned short vco_test_hack_flag = 0;
static unsigned short vco_test_channel = 0xff;

unsigned short btdrv_get_vco_test_process_flag(void)
{
    return vco_test_hack_flag;
}

bool btdrv_vco_test_bridge_intsys_callback(const unsigned char *data)
{
    bool status = false;
    if(data[0]==0x01 &&data[1]==0xaa&&data[2]==0xfc &&data[3]==0x02)
    {
        status = true;
        vco_test_hack_flag = data[5];
        vco_test_channel = data[4];
    }

    return status;
}

void btdrv_vco_test_process(uint8_t op)
{
    if(op == 0x02)//vco test start
    {
        if(vco_test_channel != 0xff)
            btdrv_vco_test_start(vco_test_channel);
    }
    else if(op ==0x04)//vco test stop
    {
        btdrv_vco_test_stop();
    }
    vco_test_channel =0xff;
    vco_test_hack_flag = 0;
}
#endif

void btdrv_vco_test_start(uint8_t chnl)
{
    if (!btdrv_vco_test_running)
    {
        btdrv_vco_test_running = true;
        btdrv_read_rf_reg(0xb6, &vco_test_reg_val_b6);
        btdrv_read_rf_reg(0x1f3, &vco_test_reg_val_1f3);
        btdrv_write_rf_reg(0x1f3, 0);
        btdrv_write_rf_reg(0xb6, vco_test_reg_val_b6|(0x03<<14));
        btdrv_write_rf_reg(0x1d7, 0xc4ff);

        BTDIGITAL_REG(0xd02201e4) = (chnl & 0x7f) | 0xa0000;
        btdrv_delay(10);
        BTDIGITAL_REG(0xd02201e4) = 0;
        btdrv_delay(10);
        BTDIGITAL_REG(0xd0340020) &= (~0x7);
        BTDIGITAL_REG(0xd0340020) |= 6;
        btdrv_delay(10);
    }
}

void btdrv_vco_test_stop(void)
{
    if (btdrv_vco_test_running)
    {
        btdrv_vco_test_running = false;
        BTDIGITAL_REG(0xd02201bc) = 0;
        BTDIGITAL_REG(0xd0340020) &=(~0x7);
        if (vco_test_reg_val_b6 != 0)
        {
            btdrv_write_rf_reg(0xb6, vco_test_reg_val_b6);
        }
        if (vco_test_reg_val_1f3 != 0)
        {
            btdrv_write_rf_reg(0x1f3, vco_test_reg_val_1f3);
        }
        btdrv_delay(10);
    }
}

void btdrv_stop_bt(void)
{
    btdrv_poweron(BT_POWEROFF);
}

void btdrv_write_memory(uint8_t wr_type,uint32_t address,const uint8_t *value,uint8_t length)
{
    uint8_t buff[256];
    if(length ==0 || length >128)
        return;
    buff[0] = 0x01;
    buff[1] = 0x02;
    buff[2] = 0xfc;
    buff[3] = length + 6;
    buff[4] = address & 0xff;
    buff[5] = (address &0xff00)>>8;
    buff[6] = (address &0xff0000)>>16;
    buff[7] = address>>24;
    buff[8] = wr_type;
    buff[9] = length;
    memcpy(&buff[10],value,length);
    btdrv_SendData(buff,length+10);
    btdrv_delay(2);


}

void btdrv_send_cmd(uint16_t opcode,uint8_t cmdlen,const uint8_t *param)
{
    uint8_t buff[256];
    buff[0] = 0x01;
    buff[1] = opcode & 0xff;
    buff[2] = (opcode &0xff00)>>8;
    buff[3] = cmdlen;
    if(cmdlen>0)
        memcpy(&buff[4],param,cmdlen);
    btdrv_SendData(buff,cmdlen+4);
}

//[26:0] 0x07ffffff
//[27:0] 0x0fffffff

uint32_t btdrv_syn_get_curr_ticks(void)
{
    //local BT clock in 312.5us half slots
    uint32_t value;
    BT_DRV_REG_OP_ENTER();
    BT_DRV_REG_OP_CLK_ENB();

    value = BTDIGITAL_REG(0xd0220490) & 0x0fffffff;

    BT_DRV_REG_OP_CLK_DIB();
    BT_DRV_REG_OP_EXIT();
    return value;
}

// Clear trigger signal with software
void  btdrv_syn_clr_trigger(void)
{
    BT_DRV_REG_OP_ENTER();
    BT_DRV_REG_OP_CLK_ENB();

    BTDIGITAL_REG(0xd02201f0) = BTDIGITAL_REG(0xd02201f0) | (1<<31);

    BT_DRV_REG_OP_CLK_DIB();
    BT_DRV_REG_OP_EXIT();
}

void bt_syn_cancel_tg_ticks(void)
{
    BT_DRV_REG_OP_ENTER();
    BT_DRV_REG_OP_CLK_ENB();

    BTDIGITAL_REG(0xd02204a4) &= ~(1<<2);

    BT_DRV_REG_OP_CLK_DIB();
    BT_DRV_REG_OP_EXIT();
}

static void btdrv_syn_set_tg_ticks(uint32_t num, uint8_t mode)
{
    BT_DRV_REG_OP_ENTER();
    BT_DRV_REG_OP_CLK_ENB();

    if (mode == BT_TRIG_MASTER_ROLE)
    {
        BTDIGITAL_REG(0xd02204a4) = 0x80000006;
        BTDIGITAL_REG(0xd02201f0) = (BTDIGITAL_REG(0xd02201f0) & 0x70000000) | (num & 0x0fffffff) | 0x10000000;
        //BT_DRV_TRACE(1,"master mode d02201f0:0x%x\n",BTDIGITAL_REG(0xd02201f0));
    }
    else
    {
        BTDIGITAL_REG(0xd02204a4) = 0x80000006;
        BTDIGITAL_REG(0xd02201f0) = (BTDIGITAL_REG(0xd02201f0) & 0x60000000) | (num & 0x0fffffff);
        //BT_DRV_TRACE(1,"slave mode d02201f0:0x%x\n",BTDIGITAL_REG(0xd02201f0));
    }

    BT_DRV_REG_OP_CLK_DIB();
    BT_DRV_REG_OP_EXIT();
}

void btdrv_syn_trigger_codec_en(uint32_t v)
{
}

uint32_t bt_syn_get_curr_ticks(uint16_t conhdl)
{
    //Piconet clock in 312.5us half slots
    int32_t curr,offset;

    curr = btdrv_syn_get_curr_ticks();
    BT_DRV_REG_OP_ENTER();

    if (btdrv_is_link_index_valid(btdrv_conhdl_to_linkid(conhdl)))
        offset = 2*bt_drv_reg_op_get_clkoffset(btdrv_conhdl_to_linkid(conhdl));
    else
        offset = 0;
//    BT_DRV_TRACE(4,"[%s] curr(%d) + offset(%d) = %d", __func__, curr , offset,curr + offset);
    BT_DRV_REG_OP_EXIT();

    return (curr + offset) & 0x0fffffff;
}

void bt_syn_trig_checker(uint16_t conhdl)
{
    int32_t clock_offset = 0;
    uint16_t bit_offset = 0;

    bt_drv_reg_op_piconet_clk_offset_get(conhdl, &clock_offset, &bit_offset);
    BT_DRV_REG_OP_ENTER();
    BT_DRV_REG_OP_CLK_ENB();

    BT_DRV_TRACE(3,"bt_syn_set_tg_tick checker d0220498=0x%08x d02204a4=0x%08x d02201f0=0x%08x",
        BTDIGITAL_REG(0xd0220498), BTDIGITAL_REG(0xd02204a4), BTDIGITAL_REG(0xd02201f0));
    BT_DRV_TRACE(2,"bt_syn_set_tg_tick checker clock_offset:0x%08x bit_offset=0x%04x", clock_offset, bit_offset);

    BT_DRV_REG_OP_CLK_DIB();
    BT_DRV_REG_OP_EXIT();
}

// Can be used by master or slave
// Ref: Master bt clk
void bt_syn_set_tg_ticks(uint32_t val,uint16_t conhdl, uint8_t mode)
{
    //set trigger time using local BT clock in 312.5us half slots

    int32_t offset;
    if (btdrv_is_link_index_valid(btdrv_conhdl_to_linkid(conhdl)))
        offset = 2*bt_drv_reg_op_get_clkoffset(btdrv_conhdl_to_linkid(conhdl));
    else
        offset = 0;

    BT_DRV_REG_OP_ENTER();
    BT_DRV_REG_OP_CLK_ENB();

    if(conhdl==0x80)
    {
        BTDIGITAL_REG(0xd0220498)=(BTDIGITAL_REG(0xd0220498)&0xfffffff0)|0x1;
    }
    else if(conhdl==0x81)
    {
        BTDIGITAL_REG(0xd0220498)=(BTDIGITAL_REG(0xd0220498)&0xfffffff0)|0x2;
    }
    else if(conhdl==0x82)
    {
        BTDIGITAL_REG(0xd0220498)=(BTDIGITAL_REG(0xd0220498)&0xfffffff0)|0x3;
    }

    if ((mode == BT_TRIG_MASTER_ROLE) && (offset !=0))
        BT_DRV_TRACE(0,"ERROR OFFSET !!");

    val = val>>1;
    val = val<<1;
    val += 1;

    BT_DRV_REG_OP_CLK_DIB();
    BT_DRV_REG_OP_EXIT();

    bt_syn_cancel_tg_ticks();
    BT_DRV_TRACE(4,"bt_syn_set_tg_ticks val:%d num:%d mode:%d conhdl:%02x", val, val - offset, mode, conhdl);
    btdrv_syn_set_tg_ticks(val - offset, mode);
    bt_syn_trig_checker(conhdl);
}

void btdrv_enable_playback_triggler(uint8_t triggle_mode)
{
    BT_DRV_REG_OP_ENTER();
    BT_DRV_REG_OP_CLK_ENB();

    if(triggle_mode == ACL_TRIGGLE_MODE)
    {
        //clear SCO trigger
        BTDIGITAL_REG(0xd02201f0) &= (~0x60000000);
        //set ACL trigger
        BTDIGITAL_REG(0xd02201f0) |= 0x20000000;
    }
    else if(triggle_mode == SCO_TRIGGLE_MODE)
    {
        //clear ACL trigger
        BTDIGITAL_REG(0xd02201f0) &= (~0x60000000);
        //set SCO trigger
        BTDIGITAL_REG(0xd02201f0) |= 0x40000000;
    }

    BT_DRV_REG_OP_CLK_DIB();
    BT_DRV_REG_OP_EXIT();
}

void btdrv_disable_playback_triggler(void)
{
    BT_DRV_REG_OP_ENTER();
    BT_DRV_REG_OP_CLK_ENB();
    //clear ACL and SOC  trigger
    BTDIGITAL_REG(0xd02201f0) &= (~0x60000000);
    BT_DRV_REG_OP_CLK_DIB();
    BT_DRV_REG_OP_EXIT();
}

void btdrv_set_bt_pcm_triggler_en(uint8_t  en)
{
    BT_DRV_REG_OP_ENTER();
    BT_DRV_REG_OP_CLK_ENB();

    if(en)
    {
        BTDIGITAL_REG(0xd022046c) &= (~0x1);
    }
    else
    {
        BTDIGITAL_REG(0xd022046c) |= 0x1;
    }
    BT_DRV_REG_OP_CLK_DIB();
    BT_DRV_REG_OP_EXIT();
}

void btdrv_set_bt_pcm_triggler_delay(uint8_t  delay)
{
    BT_DRV_REG_OP_ENTER();
    BT_DRV_REG_OP_CLK_ENB();

    if(delay > 0x3f)
    {
        BT_DRV_TRACE(0,"delay is error value");
        return;
    }
    BT_DRV_TRACE(1,"0XD022045c=%x",BTDIGITAL_REG(0xd022045c));
    BTDIGITAL_REG(0xd022045c) &= ~0x7f;
    BTDIGITAL_REG(0xd022045c) |= (delay);
    BT_DRV_TRACE(1,"exit :0XD022045c=%x",BTDIGITAL_REG(0xd022045c));

    BT_DRV_REG_OP_CLK_DIB();
    BT_DRV_REG_OP_EXIT();
}


void btdrv_set_bt_pcm_en(uint8_t  en)
{
    BT_DRV_REG_OP_ENTER();
    BT_DRV_REG_OP_CLK_ENB();

    if(en)
        BTDIGITAL_REG(0xd02201b0) |= 1;
    //else
        //BTDIGITAL_REG(0xd02201b0) &= (~1);

    BT_DRV_REG_OP_CLK_DIB();
    BT_DRV_REG_OP_EXIT();
}

//pealse use btdrv_is_link_index_valid() check link index whether valid
uint8_t btdrv_conhdl_to_linkid(uint16_t connect_hdl)
{
    //invalid hci handle,such as link disconnected
    if(connect_hdl < HCI_HANDLE_MIN || connect_hdl > HCI_HANDLE_MAX)
    {
        BT_DRV_TRACE(0, "ERROR Connect Handle=0x%x",connect_hdl);
        return HCI_LINK_INDEX_INVALID;
    }
    else
    {
        return (connect_hdl - HCI_HANDLE_MIN);
    }
}

void btdrv_pcm_enable(void)
{
    BT_DRV_REG_OP_ENTER();
    BT_DRV_REG_OP_CLK_ENB();

    *(volatile uint32_t *)(0xd02201b0) |= 0x01; //pcm enable

    BT_DRV_REG_OP_CLK_DIB();
    BT_DRV_REG_OP_EXIT();
}

void btdrv_pcm_disable(void)
{
    BT_DRV_REG_OP_ENTER();
    BT_DRV_REG_OP_CLK_ENB();

    //*(volatile uint32_t *)(0xd02201b0) &= 0xfffffffe; //pcm disable

    BT_DRV_REG_OP_CLK_DIB();
    BT_DRV_REG_OP_EXIT();
}

// Trace tport
static const struct HAL_IOMUX_PIN_FUNCTION_MAP pinmux_tport[] =
{
    {HAL_IOMUX_PIN_P0_0, HAL_IOMUX_FUNC_AS_GPIO, HAL_IOMUX_PIN_VOLTAGE_VIO, HAL_IOMUX_PIN_PULLUP_ENABLE},
};

int btdrv_host_gpio_tport_open(void)
{
    uint32_t i;

    for (i=0; i<ARRAY_SIZE(pinmux_tport); i++)
    {
        hal_iomux_init((struct HAL_IOMUX_PIN_FUNCTION_MAP *)&pinmux_tport[i], 1);
        hal_gpio_pin_set_dir((enum HAL_GPIO_PIN_T)pinmux_tport[i].pin, HAL_GPIO_DIR_OUT, 0);
    }
    return 0;
}

int btdrv_gpio_port_set(int port)
{
    hal_gpio_pin_set((enum HAL_GPIO_PIN_T)pinmux_tport[port].pin);
    return 0;
}

int btdrv_gpio_tport_clr(int port)
{
    hal_gpio_pin_clr((enum HAL_GPIO_PIN_T)pinmux_tport[port].pin);
    return 0;
}

#if defined(TX_RX_PCM_MASK)
uint8_t  btdrv_is_pcm_mask_enable(void)
{
    return 1;
}
#endif

#ifdef PCM_FAST_MODE
void btdrv_open_pcm_fast_mode_enable(void)
{
    BT_DRV_REG_OP_ENTER();
    BT_DRV_REG_OP_CLK_ENB();

    *(volatile uint32_t *)(0xd0220464) |= 1<<22;///pcm fast mode en bit22
    *(volatile uint32_t *)(0xd02201b8) = (*(volatile uint32_t *)(0xd02201b8)&0xFFFFFF00)|0x8;///pcm clk [8:0]

    //sample num in one frame [16:10]
    BTDIGITAL_REG_SET_FIELD(0xd0220460, 0x7f, 10, 0x3b);

    BT_DRV_TRACE(0,"pcm fast mode config 0x46c=0x%x,0x1b8=0x%x,0x460=0x%x",
        BTDIGITAL_REG(0xd0220464),
        BTDIGITAL_REG(0xd02201b8),
        BTDIGITAL_REG(0xd0220460));

    BT_DRV_REG_OP_CLK_DIB();
    BT_DRV_REG_OP_EXIT();
}

void btdrv_open_pcm_fast_mode_disable(void)
{
    //BT_DRV_REG_OP_ENTER();
    //BT_DRV_REG_OP_CLK_ENB();

    //BT_DRV_TRACE(0,"pcm fast mode disable\n");
    //*(volatile uint32_t *)(0xd0220464) = (*(volatile uint32_t *)(0xd0220464)&0xFFBFFFFF);///disable pcm fast mode
    //*(volatile uint32_t *)(0xd02201b8) = (*(volatile uint32_t *)(0xd02201b8)&0xFFFFFF00);

    //BT_DRV_REG_OP_CLK_DIB();
    //BT_DRV_REG_OP_EXIT();
}
#endif

void btdrv_set_btpcm_mute_pattern(uint16_t pattern)
{
    BT_DRV_REG_OP_ENTER();
    BT_DRV_REG_OP_CLK_ENB();

    BTDIGITAL_REG(0xD0220144) |= pattern;

    BT_DRV_REG_OP_CLK_DIB();
    BT_DRV_REG_OP_EXIT();
}

#if defined(CVSD_BYPASS)
void btdrv_cvsd_bypass_enable(void)
{
    BT_DRV_REG_OP_ENTER();
    BT_DRV_REG_OP_CLK_ENB();

    // BTDIGITAL_REG(0xD02201E8) |= 0x04000000; //test sequecy
    BTDIGITAL_REG(0xD02201A0) &= ~(1<<7); //soft cvsd
    //BTDIGITAL_REG(0xD02201b8) |= (1<<31); //revert clk

    BT_DRV_REG_OP_CLK_DIB();
    BT_DRV_REG_OP_EXIT();
}
#endif

void bt_drv_extra_config_after_init(void)
{
    BT_DRV_REG_OP_ENTER();
    BT_DRV_REG_OP_CLK_ENB();

    btdrv_ecc_config();
    bt_drv_reg_op_fa_agc_init();

    BT_DRV_REG_OP_CLK_DIB();
    BT_DRV_REG_OP_EXIT();
}

#define FREQ_MAX    97000000
#define FREQ_MIN    95000000
#define VCO_SPD_MAX 7
#define VCO_SPD_MIN 0

void bt_drv_bbpll_cal(void)
{
    uint8_t cal_count = 0;
    uint8_t vco_spd_temp, vco_spd_original, vco_spd_current, vco_spd_final= 0;
    uint32_t frequency = 0;

    if (0) {
        //Reserve the case obtained val from NV
    } else {

        enum HAL_CMU_FREQ_T save_sys_freq = hal_sysfreq_get();

        BTRF_REG_GET_FIELD(0xA1, 7, 11, vco_spd_original);
        vco_spd_final = vco_spd_original;

        hal_sysfreq_req(HAL_SYSFREQ_USER_BT, HAL_CMU_FREQ_104M);

        do {
            BTRF_REG_GET_FIELD(0xA1, 7, 11, vco_spd_current);

            frequency = hal_sys_timer_calc_cpu_freq(5, 0);
            TRACE_IMM(1, "frequency=%d", frequency);

            vco_spd_temp = vco_spd_current;
            if((frequency > FREQ_MIN) && (frequency < FREQ_MAX)) {
                break;
            } else if (frequency <= FREQ_MIN) {
                vco_spd_temp = vco_spd_current + 1;
            } else if (frequency >= FREQ_MAX) {
                vco_spd_temp = vco_spd_current - 1;
            }

            if ((vco_spd_temp >= VCO_SPD_MAX) || (vco_spd_temp <= 0)) {
                TRACE_IMM(2,"[WARNING]%s failed, vco_spd:%d out of range.", __func__, vco_spd_temp);
                BTRF_REG_SET_FIELD(0xA1, 7, 11, vco_spd_original);
                break;
            }

            BTRF_REG_SET_FIELD(0xA1, 7, 11, vco_spd_temp);

            cal_count ++;
            vco_spd_final = vco_spd_temp;
        } while(cal_count < VCO_SPD_MAX);

        TRACE(3, "%s cal_count=%d, vco_spd_final=%d, save_sys_freq=%d.",
            __func__, cal_count, vco_spd_final, save_sys_freq);

        hal_sysfreq_req(HAL_SYSFREQ_USER_BT, save_sys_freq);
    }
}
