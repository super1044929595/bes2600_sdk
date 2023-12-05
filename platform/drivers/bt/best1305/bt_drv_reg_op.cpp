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
#include "cmsis.h"
#include "bt_drv_reg_op.h"
#include "bt_drv_internal.h"
#include "bt_drv_2500i_internal.h"
#include "bt_drv_interface.h"
#include "bt_drv.h"
#include "hal_sysfreq.h"
#include "hal_chipid.h"
#include "hal_trace.h"
#include "hal_cmu.h"
#include "hal_iomux.h"
#include <string.h>
#include "CrashCatcher.h"
#include "bt_drv_internal.h"
#include "besbt_string.h"
#include "bt_patch_1305_t0.h"
#include "bt_patch_1305_t0_testmode.h"


/***************************************************************************
*
*Warning: Prohibit reading and writing EM memory in Bluetooth drivers
*
*****************************************************************************/

#ifdef __CLK_GATE_DISABLE__
static uint32_t reg_op_clk_enb = 0;
void BT_DRV_REG_OP_CLK_ENB(void)
{
    if (!reg_op_clk_enb)
    {
        hal_cmu_bt_sys_clock_force_on();
        hal_cmu_bt_sys_force_ram_on();
    }
    reg_op_clk_enb++;
}

void BT_DRV_REG_OP_CLK_DIB(void)
{
    reg_op_clk_enb--;
    if (!reg_op_clk_enb)
    {
        hal_cmu_bt_sys_clock_auto();
        hal_cmu_bt_sys_force_ram_auto();
    }
}
#else
#error "please open CLK_GATE_DISABLE macro"
#endif

#define GAIN_TBL_SIZE           0x0F

static uint32_t bt_ram_start_addr=0;
static uint32_t hci_fc_env_addr=0;
static uint32_t ld_acl_env_addr=0;
static uint32_t bt_util_buf_env_addr=0;
static uint32_t em_buf_env_addr=0;
static uint32_t ld_ibrt_env_addr=0;
static uint32_t dbg_state_addr=0;
static uint32_t lc_state_addr=0;
static uint32_t task_message_buffer_addr=0;
static uint32_t lmp_message_buffer_addr=0;
static uint32_t ld_sco_env_addr=0;
static uint32_t rx_monitor_addr=0;
static uint32_t lc_env_addr=0;
static uint32_t dbg_bt_setting_addr=0;
static uint32_t lm_nb_sync_active_addr=0;
static uint32_t lm_env_addr=0;
static uint32_t pending_seq_error_link_addr=0;
static uint32_t hci_env_addr=0;
static uint32_t lc_sco_env_addr=0;
static uint32_t llm_le_env_addr=0;
static uint32_t ld_env_addr=0;
static uint32_t rwip_env_addr=0;
static uint32_t MIC_BUFF_addr=0;
static uint32_t g_mem_dump_ctrl_addr=0;
static uint32_t ble_rx_monitor_addr=0;
static uint32_t reconnecting_flag_addr=0;
static uint32_t link_connect_bak_addr=0;
static uint32_t llc_env_addr=0;
static uint32_t dbg_bt_setting_ext1_addr=0;
static uint32_t rwip_rf_addr=0;
static uint32_t ld_acl_metrics_addr = 0;
static uint32_t rf_rx_hwgain_tbl_addr = 0;
static uint32_t rf_hwagc_rssi_correct_tbl_addr = 0;
//static uint32_t curr_cw_addr = 0;
static uint32_t rf_rx_gain_fixed_tbl_addr = 0;
static uint32_t ebq_test_setting_addr = 0;
static uint32_t host_ref_clk_addr = 0;
//static uint32_t cmd_filter_addr = 0;
//static uint32_t evt_filter_addr = 0;
static uint32_t hci_dbg_bt_setting_ext2_addr = 0;
static uint32_t m_dbg_log_tl_env_addr = 0;
static uint32_t dbg_log_mem_env_addr = 0;
static uint32_t dbg_bt_setting_ext2_addr=0;
static uint32_t rf_rx_gain_ths_tbl_bt_addr = 0;
static uint32_t rf_rx_gain_ths_tbl_le_addr = 0;
static uint32_t dbg_bt_setting_ext3_addr=0;
static uint32_t hci_dbg_set_sw_rssi_addr = 0;
static uint32_t rob_connection_addr = 0;
static uint32_t bt_rx_nosync_info_addr=0;
static uint32_t fa_agc_cfg_addr = 0;
static uint32_t pcm_need_start_flag_addr = 0;
static uint32_t nosig_sch_flag_addr = 0;
static uint32_t snr_buff_addr = 0;
static uint32_t clk_seed_addr=0;
static uint32_t bt_pcm_valid_clk_addr = 0;
static uint32_t bt_pcm_invalid_clk_addr = 0;

void bt_drv_reg_op_global_symbols_init(void)
{
    bt_ram_start_addr = 0xc0000000;
    enum HAL_CHIP_METAL_ID_T metal_id = hal_get_chip_metal_id();
    if (metal_id >= HAL_CHIP_METAL_ID_0)
    {
        hci_fc_env_addr = 0xc00080ac;
        ld_acl_env_addr = 0xc0007508;
        bt_util_buf_env_addr = 0xc0006808;
        em_buf_env_addr = 0xc0007e04;
        dbg_state_addr = 0xc0005df4;
        lc_state_addr = 0xc0006cc4;
        ld_sco_env_addr = 0xc000751c;
        rx_monitor_addr = 0xc000651c;
        lc_env_addr = 0xc0006cb8;
        dbg_bt_setting_addr = 0xc0005e00;
        lm_nb_sync_active_addr = 0xc0006cb2;
        lm_env_addr = 0xc0006a60;
        hci_env_addr = 0xc0007ffc;
        lc_sco_env_addr = 0xc0006c94;
        llm_le_env_addr = 0xc0007c84;
        ld_env_addr = 0xc0006d7c;
        rwip_env_addr = 0xc0005d0c;
        MIC_BUFF_addr = 0xc00065d4;
        g_mem_dump_ctrl_addr = 0xc00063d4;
        ble_rx_monitor_addr = 0xc00064bc;
        llc_env_addr = 0xc0007d8c;
        dbg_bt_setting_ext1_addr = 0xc0005eec;
        rwip_rf_addr = 0xc0000b20;
        ld_acl_metrics_addr = 0xc00072dc;
        rf_rx_hwgain_tbl_addr =  0xc00004b0;
        rf_hwagc_rssi_correct_tbl_addr = 0xc0000430;
        ld_ibrt_env_addr = 0xc00079d8;
        rf_rx_gain_fixed_tbl_addr = 0xc00064b0;
        ebq_test_setting_addr = 0xc0005f08;
        host_ref_clk_addr = 0xc0000654;
        hci_dbg_bt_setting_ext2_addr = 0xc0005e74;
        m_dbg_log_tl_env_addr = 0xc0006378;
        dbg_log_mem_env_addr = 0xc0005f58;
        dbg_bt_setting_ext2_addr=0xc0005e74;
        rf_rx_gain_ths_tbl_bt_addr = 0xc0000470;
        rf_rx_gain_ths_tbl_le_addr = 0xc0000490;
        dbg_bt_setting_ext3_addr = 0xc0005ec4;
        hci_dbg_set_sw_rssi_addr = 0xc0005f1c;
        bt_rx_nosync_info_addr = 0xc0007548;
        rob_connection_addr = ROB_CONNECTION_ADDR;
        fa_agc_cfg_addr = FA_AGC_CFG_ADDR;
        pcm_need_start_flag_addr = PCM_NEED_START_FLAG_ADDR;
        nosig_sch_flag_addr = TESTMODE_NOSIG_SCH_FLAG_ADDR;
        snr_buff_addr = LD_SNR_BUF_ADDR;
        clk_seed_addr = CLK_SEED_ADDR;
        bt_pcm_valid_clk_addr = LAST_VALID_LOCAL_CLOCK_ADDR;
        bt_pcm_invalid_clk_addr = LAST_INVALID_LOCAL_CLOCK_ADDR;
    }
}

uint32_t bt_drv_reg_op_get_host_ref_clk(void)
{
    uint32_t ret = 0;
    BT_DRV_REG_OP_ENTER();
    BT_DRV_REG_OP_CLK_ENB();

    ret = BT_DRIVER_GET_U32_REG_VAL(host_ref_clk_addr);

    BT_DRV_REG_OP_CLK_DIB();
    BT_DRV_REG_OP_EXIT();
    return ret;
}
void bt_drv_reg_op_set_controller_log_buffer(void)
{
    BT_DRV_REG_OP_ENTER();
    BT_DRV_REG_OP_CLK_ENB();

    uint32_t *p_write_addr1 = (uint32_t*)(uintptr_t)(dbg_log_mem_env_addr+0x200);//5d08
    uint32_t *p_write_addr2 = (uint32_t*)(uintptr_t)(dbg_log_mem_env_addr+0x410);//5f18

    uint32_t *p_start_addr1 = (uint32_t*)(uintptr_t)(dbg_log_mem_env_addr+0x204);//5d0c
    uint32_t *p_start_addr2 = (uint32_t*)(uintptr_t)(dbg_log_mem_env_addr+0x414);//5f1c

    uint16_t *p_max_len1 = (uint16_t*)(uintptr_t)(dbg_log_mem_env_addr+0x208);//5d10
    uint16_t *p_max_len2 = (uint16_t*)(uintptr_t)(dbg_log_mem_env_addr+0x418);//5f20

    *p_write_addr1 = 0xC0009C90;
    *p_start_addr1 = 0xC0009C90;

    *p_write_addr2 = (0xC0009C90+0x400);
    *p_start_addr2 = (0xC0009C90+0x400);

    *p_max_len1 = 0x3e8;
    *p_max_len2 = 0x3e8;

    BT_DRV_REG_OP_CLK_DIB();
    BT_DRV_REG_OP_EXIT();
}
extern "C" void hal_iomux_set_p16_controller_log(void);
extern "C" void hal_iomux_set_p20_p21_controller_log(void);
void bt_drv_reg_op_config_controller_log(uint32_t log_mask, uint16_t* p_cmd_filter, uint8_t cmd_nb, uint8_t* p_evt_filter, uint8_t evt_nb)
{
    int sRet = 0;
#ifdef BT_UART_LOG_P16
    hal_iomux_set_p16_controller_log();
#else
    hal_iomux_set_p20_p21_controller_log();
#endif
    BT_DRV_REG_OP_ENTER();
    BT_DRV_REG_OP_CLK_ENB();

    uint16_t cmd_filter_buf[CMD_FILTER_LEN];
    uint8_t evt_filter_buf[EVT_FILTER_LEN];
    sRet = memset_s((void*)cmd_filter_buf, CMD_FILTER_LEN*2, 0xff, CMD_FILTER_LEN*2);
    if (sRet)
    {
        BT_DRV_TRACE(1, "%s line:%d sRet:%d", __func__, __LINE__, sRet);
    }
    sRet = memset_s((void*)evt_filter_buf, EVT_FILTER_LEN, 0xff, EVT_FILTER_LEN);
    if (sRet)
    {
        BT_DRV_TRACE(1, "%s line:%d sRet:%d", __func__, __LINE__, sRet);
    }
    for(uint32_t i = 0; i < cmd_nb; i++)
    {
        cmd_filter_buf[i] = *p_cmd_filter++;
    }
    for(uint32_t j = 0; j < evt_nb; j++)
    {
        evt_filter_buf[j] = *p_evt_filter++;
    }
    bt_drv_reg_op_set_controller_log_buffer();
    bt_drv_reg_op_config_cotroller_cmd_filter(cmd_filter_buf);
    bt_drv_reg_op_config_cotroller_evt_filter(evt_filter_buf);
    bt_drv_reg_op_set_controller_trace_curr_sw(log_mask);

    BT_DRV_REG_OP_CLK_DIB();
    BT_DRV_REG_OP_EXIT();
}

void bt_drv_reg_op_set_controller_trace_curr_sw(uint32_t log_mask)
{
    BT_DRV_REG_OP_ENTER();
    BT_DRV_REG_OP_CLK_ENB();

    uint32_t* p_curr_sw = (uint32_t *)(uintptr_t)(m_dbg_log_tl_env_addr + 4);
    if(p_curr_sw != NULL)
    {
        *p_curr_sw = log_mask;
    }
    BT_DRV_REG_OP_CLK_DIB();
    BT_DRV_REG_OP_EXIT();
}

void bt_drv_reg_op_config_cotroller_cmd_filter(uint16_t* p_cmd_filter)
{
    int sRet = 0;

    BT_DRV_REG_OP_ENTER();
    BT_DRV_REG_OP_CLK_ENB();

    uint32_t* p_cmd_filter_addr = (uint32_t *)(uintptr_t)(m_dbg_log_tl_env_addr + 0x3a);
    if(p_cmd_filter != NULL)
    {
        sRet = memcpy_s((void*)p_cmd_filter_addr, CMD_FILTER_LEN*sizeof(uint16_t), (void*)p_cmd_filter, CMD_FILTER_LEN*sizeof(uint16_t));
        if (sRet)
        {
            BT_DRV_TRACE(1, "%s line:%d sRet:%d", __func__, __LINE__, sRet);
        }
    }

    BT_DRV_REG_OP_CLK_DIB();
    BT_DRV_REG_OP_EXIT();
}

void bt_drv_reg_op_config_cotroller_evt_filter(uint8_t* p_evt_filter)
{
    int sRet = 0;

    BT_DRV_REG_OP_ENTER();
    BT_DRV_REG_OP_CLK_ENB();

    uint32_t* p_evt_filter_addr = (uint32_t*)(uintptr_t)(m_dbg_log_tl_env_addr + 0x4e);
    if(p_evt_filter != NULL)
    {
        sRet = memcpy_s((void*)p_evt_filter_addr, EVT_FILTER_LEN, (void*)p_evt_filter, EVT_FILTER_LEN);
        if (sRet)
        {
            BT_DRV_TRACE(1, "%s line:%d sRet:%d", __func__, __LINE__, sRet);
        }
    }

    BT_DRV_REG_OP_CLK_DIB();
    BT_DRV_REG_OP_EXIT();
}



#if (LL_MONITOR == 1)

void bt_drv_reg_op_ll_monitor(uint16_t connhdl, uint8_t metric_type, uint32_t* ret_val)
{
    BT_DRV_REG_OP_ENTER();
    BT_DRV_REG_OP_CLK_ENB();

    uint8_t link_id = btdrv_conhdl_to_linkid(connhdl);
    uint32_t ld_acl_metrics_ptr = 0;
    uint32_t metric_off = 12 + metric_type*4;
    if(ld_acl_metrics_addr != 0)
    {
        ld_acl_metrics_ptr = ld_acl_metrics_addr + link_id * 156;
    }
    *ret_val = BT_DRIVER_GET_U32_REG_VAL(ld_acl_metrics_ptr + metric_off);

    BT_DRV_REG_OP_CLK_DIB();
    BT_DRV_REG_OP_EXIT();
}
#endif

void bt_drv_reg_op_set_rf_rx_hwgain_tbl(int8_t (*p_tbl)[3])
{
    BT_DRV_REG_OP_ENTER();
    BT_DRV_REG_OP_CLK_ENB();

    int i, j ;
    int8_t (*p_tbl_addr)[3] = (int8_t (*)[3])rf_rx_hwgain_tbl_addr;

    if(p_tbl != NULL)
    {
        for(i = 0; i < GAIN_TBL_SIZE; i++)
        {
            for(j = 0; j < 3; j++)
            {
                p_tbl_addr[i][j] = p_tbl[i][j];
            }
        }
    }

    BT_DRV_REG_OP_CLK_DIB();
    BT_DRV_REG_OP_EXIT();
}

void bt_drv_reg_op_set_rf_hwagc_rssi_correct_tbl(int8_t* p_tbl)
{
    BT_DRV_REG_OP_ENTER();
    BT_DRV_REG_OP_CLK_ENB();
    int i;
    int8_t* p_tbl_addr = (int8_t*)rf_hwagc_rssi_correct_tbl_addr;
    if(p_tbl != NULL)
    {
        for(i = 0; i < 16; i++)
        {
            p_tbl_addr[i] = p_tbl[i];
        }
    }
    BT_DRV_REG_OP_CLK_DIB();
    BT_DRV_REG_OP_EXIT();
}


#if (defined(ACL_DATA_CRC_TEST))
uint32_t crc32_ieee_update(uint32_t crc, const uint8_t *data, size_t len)
{
    crc = ~crc;
    for (size_t i = 0; i < len; i++)
    {
        crc = crc ^ data[i];

        for (uint8_t j = 0; j < 8; j++)
        {
            crc = (crc >> 1) ^ (0xEDB88320 & -(crc & 1));
        }
    }
    return (~crc);
}

uint32_t crc32_ieee(const uint8_t *data, size_t len)
{
    uint32_t crc = 0;
    BT_DRV_REG_OP_ENTER();
    BT_DRV_REG_OP_CLK_ENB();

    crc = crc32_ieee_update(0x0, data, len);

    BT_DRV_REG_OP_CLK_DIB();
    BT_DRV_REG_OP_EXIT();
    return crc;
}
#endif

void bt_drv_reg_op_hci_vender_ibrt_ll_monitor(uint8_t* ptr, uint16_t* p_sum_err,uint16_t* p_sum_err2,uint16_t* p_rx_total)
{
    POSSIBLY_UNUSED const char *monitor_str[37] =
    {
        "TX DM1",
        "TX DH1",
        "TX DM3",
        "TX DH3",
        "TX DM5",
        "TX DH5",
        "TX 2DH1",
        "TX 3DH1",
        "TX 2DH3",
        "TX 3DH3",
        "TX 2DH5",
        "TX 3DH5",

        "RX DM1",
        "RX DH1",
        "RX DM3",
        "RX DH3",
        "RX DM5",
        "RX DH5",
        "RX 2DH1",
        "RX 3DH1",
        "RX 2DH3",
        "RX 3DH3",
        "RX 2DH5",
        "RX 3DH5",
        "hec error",
        "crc error",
        "fec error",
        "gard error",
        "ecc count",

        "radio_count",
        "sleep_duration_count",
        "radio_tx_succss_count",
        "radio_tx_count",
        "sco_no_sync",
        "pcm_invalid",
        "rxseqerr_count",
        "revfa_count",
    };
    BT_DRV_REG_OP_ENTER();

    uint8_t *p = ( uint8_t * )ptr;
    uint32_t sum_err = 0;
    uint32_t sum_err2 = 0;
    uint32_t rx_data_sum = 0;
    uint32_t val;

    BT_DRV_TRACE(0,"ibrt_ui_log:ll_monitor");

    for (uint8_t i = 0; i < 37; i++)
    {
        val = co_read32p(p);
        if (val)
        {
            if(i>= 12 && i<=23)
            {
                rx_data_sum += val;
            }

            if(i == 34)
                sum_err = val;

            if((i > 34) && (i <= 36))
            {
                sum_err2 += val;
            }

            BT_DRV_TRACE(2,"%s %d", monitor_str[i], val);
        }
        p+=4;
    }
    *p_sum_err = sum_err;
    *p_sum_err2 = sum_err2;
    *p_rx_total = rx_data_sum;

    BT_DRV_REG_OP_EXIT();
}

void bt_drv_reg_op_trigger_time_checker(void)
{
    BT_DRV_REG_OP_ENTER();
    BT_DRV_REG_OP_CLK_ENB();

    BT_DRV_TRACE(1,"BT_REG_OP:0xd02201f0 = %x", BT_DRIVER_GET_U32_REG_VAL(0xd02201f0));

    BT_DRV_REG_OP_CLK_DIB();
    BT_DRV_REG_OP_EXIT();
}

///BD Address structure
struct bd_addr
{
    ///6-byte array address value
    uint8_t  addr[6];
};
///device info structure
struct ld_device_info
{
    struct bd_addr bd_addr;
    uint8_t link_id;
    uint8_t state;
};

void bt_drv_reg_op_ibrt_env_reset(void)
{
}

int bt_drv_reg_op_currentfreeaclbuf_get(void)
{
    int nRet = 0;
    BT_DRV_REG_OP_ENTER();
    BT_DRV_REG_OP_CLK_ENB();
    //ACL packert number of host set - ACL number of controller has been send to host
    //hci_fc_env.host_set.acl_pkt_nb - hci_fc_env.cntr.acl_pkt_sent
    if(hci_fc_env_addr != 0)
    {
        nRet = (BT_DRIVER_GET_U16_REG_VAL(hci_fc_env_addr+0x4) - BT_DRIVER_GET_U16_REG_VAL(hci_fc_env_addr+0xc));//acl_pkt_nb - acl_pkt_sent
    }
    else
    {
        nRet = 0;
    }
    BT_DRV_REG_OP_CLK_DIB();
    BT_DRV_REG_OP_EXIT();
    return nRet;
}

void bt_drv_reg_op_lsto_hack(uint16_t hciHandle, uint16_t lsto)
{
    BT_DRV_REG_OP_ENTER();
    BT_DRV_REG_OP_CLK_ENB();

    uint32_t acl_par_ptr = 0;
    //ld_acl_env

    if(ld_acl_env_addr)
    {
        acl_par_ptr = BT_DRIVER_GET_U32_REG_VAL(ld_acl_env_addr+(hciHandle-0x80)*4);
    }

    if(acl_par_ptr)
    {
        //lsto off:0x9a
        BT_DRV_TRACE(3,"BT_REG_OP:Set the lsto for hciHandle=0x%x, from:0x%x to 0x%x",
                     hciHandle, BT_DRIVER_GET_U16_REG_VAL(acl_par_ptr+0x9a),lsto);

        BT_DRIVER_GET_U16_REG_VAL(acl_par_ptr+0x9a) = lsto;
    }
    else
    {
        BT_DRV_TRACE(0,"BT_REG_OP:ERROR,acl par address error");
    }

    BT_DRV_REG_OP_CLK_DIB();
    BT_DRV_REG_OP_EXIT();
}

uint16_t bt_drv_reg_op_get_lsto(uint16_t hciHandle)
{
    uint32_t acl_par_ptr = 0;
    uint16_t lsto = 0;
    //ld_acl_env
    BT_DRV_REG_OP_ENTER();
    BT_DRV_REG_OP_CLK_ENB();
    if(ld_acl_env_addr)
    {
        acl_par_ptr = BT_DRIVER_GET_U32_REG_VAL(ld_acl_env_addr+(hciHandle-0x80)*4);
    }


    if(acl_par_ptr)
    {
        //lsto off:0x9a
        lsto = BT_DRIVER_GET_U16_REG_VAL(acl_par_ptr+0x9a) ;
        BT_DRV_TRACE(2,"BT_REG_OP:lsto=0x%x for hciHandle=0x%x",lsto,hciHandle);
    }
    else
    {
        lsto= 0xffff;
        BT_DRV_TRACE(0,"BT_REG_OP:ERROR,acl par null ptr");
    }

    BT_DRV_REG_OP_CLK_DIB();
    BT_DRV_REG_OP_EXIT();
    return lsto;
}

int bt_drv_reg_op_acl_chnmap(uint16_t hciHandle, uint8_t *chnmap, uint8_t chnmap_len)
{
    int nRet  = 0;
    int sRet = 0;
    uint32_t acl_evt_ptr = 0;
    uint8_t *chnmap_ptr = 0;
    uint8_t link_id = btdrv_conhdl_to_linkid(hciHandle);

    BT_DRV_REG_OP_ENTER();
    BT_DRV_REG_OP_CLK_ENB();

    if (!btdrv_is_link_index_valid(link_id))
    {
        sRet = memset_s(chnmap, chnmap_len, 0, chnmap_len);
        if (sRet)
        {
            BT_DRV_TRACE(1, "%s line:%d sRet:%d", __func__, __LINE__, sRet);
        }
        nRet = -1;
        goto exit;
    }
    if (chnmap_len < 10)
    {
        sRet = memset_s(chnmap, chnmap_len, 0, chnmap_len);
        if (sRet)
        {
            BT_DRV_TRACE(1, "%s line:%d sRet:%d", __func__, __LINE__, sRet);
        }
        nRet = -1;
        goto exit;
    }


    if(ld_acl_env_addr)
    {
        acl_evt_ptr = BT_DRIVER_GET_U32_REG_VAL(ld_acl_env_addr+link_id*4);
    }


    if(acl_evt_ptr != 0)
    {
        //afh_map off:0x34
        chnmap_ptr = (uint8_t *)(uintptr_t)(acl_evt_ptr+0x34);
    }

    if (!chnmap_ptr)
    {
        sRet = memset_s(chnmap, chnmap_len, 0, chnmap_len);
        if (sRet)
        {
            BT_DRV_TRACE(1, "%s line:%d sRet:%d", __func__, __LINE__, sRet);
        }
        nRet = -1;
        goto exit;
    }
    else
    {
        sRet = memcpy_s(chnmap, chnmap_len, chnmap_ptr, chnmap_len);
        if (sRet)
        {
            BT_DRV_TRACE(1, "%s line:%d sRet:%d", __func__, __LINE__, sRet);
        }
    }

exit:
    BT_DRV_REG_OP_CLK_DIB();
    BT_DRV_REG_OP_EXIT();

    return nRet;
}

extern "C" uint32_t hci_current_left_tx_packets_left(void);
extern "C" uint32_t hci_current_left_rx_packets_left(void);
extern "C" uint32_t hci_current_rx_packet_complete(void);
extern "C" uint8_t hci_current_rx_aclfreelist_cnt(void);
void bt_drv_reg_op_bt_info_checker(void)
{
    uint32_t *rx_buf_ptr=NULL;
    uint32_t *tx_buf_ptr=NULL;
    uint8_t rx_free_buf_count=0;
    uint8_t tx_free_buf_count=0;

    BT_DRV_REG_OP_ENTER();
    BT_DRV_REG_OP_CLK_ENB();

    if(bt_util_buf_env_addr)
    {
        //acl_rx_free off:0x14
        //acl_tx_free off:0x28
        rx_buf_ptr = (uint32_t *)(uintptr_t)(bt_util_buf_env_addr+0x14); //bt_util_buf_env.acl_rx_free
        tx_buf_ptr = (uint32_t *)(uintptr_t)(bt_util_buf_env_addr+0x28); //bt_util_buf_env.acl_tx_free
    }

    while(rx_buf_ptr && *rx_buf_ptr)
    {
        rx_free_buf_count++;
        rx_buf_ptr = (uint32_t *)(uintptr_t)(*rx_buf_ptr);
    }

    BT_DRV_TRACE(3,"BT_REG_OP:acl_rx_free ctrl rxbuff %d, host free %d,%d", \
                 rx_free_buf_count, \
                 hci_current_left_rx_packets_left(), \
                 bt_drv_reg_op_currentfreeaclbuf_get());

    //check tx buff
    while(tx_buf_ptr && *tx_buf_ptr)
    {
        tx_free_buf_count++;
        tx_buf_ptr = (uint32_t *)(uintptr_t)(*tx_buf_ptr);
    }

    BT_DRV_TRACE(2,"BT_REG_OP:acl_tx_free ctrl txbuff %d, host consider ctrl free %d", \
                 tx_free_buf_count, \
                 hci_current_left_tx_packets_left());

    BT_DRV_REG_OP_CLK_DIB();
    BT_DRV_REG_OP_EXIT();

    bt_drv_reg_op_controller_state_checker();
}

struct ke_timer
{
    /// next ke timer
    struct ke_timer *next;
    /// message identifier
    uint16_t     id;
    /// task identifier
    uint16_t    task;
    /// time value
    uint32_t    time;
};

struct co_list_hdr
{
    /// Pointer to next co_list_hdr
    struct co_list_hdr *next;
};

/// structure of a list
struct co_list_con
{
    /// pointer to first element of the list
    struct co_list_hdr *first;
    /// pointer to the last element
    struct co_list_hdr *last;

    /// number of element in the list
    uint32_t cnt;
    /// max number of element in the list
    uint32_t maxcnt;
    /// min number of element in the list
    uint32_t mincnt;
};

struct mblock_free
{
    /// Next free block pointer
    struct mblock_free* next;
    /// Previous free block pointer
    struct mblock_free* previous;
    /// Size of the current free block (including delimiter)
    uint16_t free_size;
    /// Used to check if memory block has been corrupted or not
    uint16_t corrupt_check;
};
bool bt_drv_reg_op_get_dbg_state(void)
{
    bool nRet = false;
    BT_DRV_REG_OP_CLK_ENB();
    //dbg_state
    if(dbg_state_addr)
    {
        nRet = BT_DRIVER_GET_U8_REG_VAL(dbg_state_addr);
    }
    else
    {
        nRet = false;
    }
    BT_DRV_REG_OP_CLK_DIB();
    return nRet;
}

uint8_t bt_drv_reg_op_get_controller_tx_free_buffer(void)
{
    uint32_t *tx_buf_ptr=NULL;
    uint8_t tx_free_buf_count = 0;

    BT_DRV_REG_OP_ENTER();
    BT_DRV_REG_OP_CLK_ENB();

    if(bt_util_buf_env_addr)
    {
        //acl_tx_free off:0x28
        tx_buf_ptr = (uint32_t *)(uintptr_t)(bt_util_buf_env_addr+0x28); //bt_util_buf_env.acl_tx_free
    }
    else
    {
        BT_DRV_TRACE(1, "BT_REG_OP:please fix %s", __func__);
        tx_free_buf_count = 0;
        goto exit;
    }

    //check tx buff
    while(tx_buf_ptr && *tx_buf_ptr)
    {
        tx_free_buf_count++;
        tx_buf_ptr = (uint32_t *)(uintptr_t)(*tx_buf_ptr);
    }

exit:
    BT_DRV_REG_OP_CLK_DIB();
    BT_DRV_REG_OP_EXIT();

    return tx_free_buf_count;
}

void bt_drv_reg_op_controller_state_checker(void)
{
    POSSIBLY_UNUSED uint8_t sync_agc_flag = bt_drv_reg_op_bt_sync_swagc_en_get();

    BT_DRV_REG_OP_ENTER();
    BT_DRV_REG_OP_CLK_ENB();

    if(lc_state_addr != 0)
    {
        BT_DRV_TRACE(1,"BT_REG_OP: LC_STATE=0x%x,sync agc=%d curr bt clk=%x,%x",BT_DRIVER_GET_U32_REG_VAL(lc_state_addr),sync_agc_flag,
            btdrv_syn_get_curr_ticks(),*(uint32_t *)0xd022001c);
    }

#ifdef BT_UART_LOG
    BT_DRV_TRACE(1,"current controller clk=%x", bt_drv_reg_op_get_host_ref_clk());
#endif

    BT_DRV_REG_OP_CLK_DIB();
    BT_DRV_REG_OP_EXIT();
}

uint8_t bt_drv_reg_op_force_get_lc_state(uint16_t conhdl)
{
    uint8_t state = 0;

    BT_DRV_REG_OP_ENTER();
    BT_DRV_REG_OP_CLK_ENB();
    if(lc_state_addr != 0)
    {
        //BT_DRV_TRACE(1,"BT_REG_OP: read LC_STATE=0x%x", BT_DRIVER_GET_U32_REG_VAL(lc_state_addr));
        uint8_t idx = btdrv_conhdl_to_linkid(conhdl);

        if (btdrv_is_link_index_valid(idx))
        {
            uint8_t *lc_state = (uint8_t *)(uintptr_t)lc_state_addr;
            state = lc_state[idx];
        }
    }
    BT_DRV_REG_OP_CLK_DIB();
    BT_DRV_REG_OP_EXIT();

    return state;
}
void bt_drv_reg_op_force_set_lc_state(uint16_t conhdl, uint8_t state)
{
    BT_DRV_REG_OP_ENTER();
    BT_DRV_REG_OP_CLK_ENB();
    if(lc_state_addr != 0)
    {
        BT_DRV_TRACE(1,"BT_REG_OP: read LC_STATE=0x%x",BT_DRIVER_GET_U32_REG_VAL(lc_state_addr));
        uint8_t idx = btdrv_conhdl_to_linkid(conhdl);

        if (btdrv_is_link_index_valid(idx))
        {
            uint8_t *lc_state = (uint8_t *)(uintptr_t)lc_state_addr;
            lc_state[idx] = state;
        }
    }
    BT_DRV_REG_OP_CLK_DIB();
    BT_DRV_REG_OP_EXIT();
}

void bt_drv_reg_op_crash_dump(void)
{
    BT_DRV_REG_OP_CLK_ENB();

    uint8_t *bt_dump_mem_start = (uint8_t*)bt_ram_start_addr;
    uint32_t bt_dump_len_max   = 0x10000;

    uint8_t *em_dump_area_2_start = (uint8_t*)0xd0210000;
    uint32_t em_area_2_len_max    = 0x8000;

    BT_DRV_TRACE(1,"BT_REG_OP:BT 1305: metal id=%d,patch version=%08x", hal_get_chip_metal_id(),*(uint32_t *)WORKMODE_PATCH_VERSION_ADDR);
    //first move R3 to R9, lost R9
    BT_DRV_TRACE(1,"BT controller BusFault_Handler:\nREG:[LR] = 0x%08x", BT_DRIVER_GET_U32_REG_VAL(BT_CONTROLLER_CRASH_DUMP_ADDR_BASE));
    BT_DRV_TRACE(1,"REG:[R0] = 0x%08x", BT_DRIVER_GET_U32_REG_VAL(BT_CONTROLLER_CRASH_DUMP_ADDR_BASE +160));
    BT_DRV_TRACE(1,"REG:[R1] = 0x%08x", BT_DRIVER_GET_U32_REG_VAL(BT_CONTROLLER_CRASH_DUMP_ADDR_BASE +164));
    BT_DRV_TRACE(1,"REG:[R2] = 0x%08x", BT_DRIVER_GET_U32_REG_VAL(BT_CONTROLLER_CRASH_DUMP_ADDR_BASE +168));
    BT_DRV_TRACE(1,"REG:[R3] = 0x%08x", BT_DRIVER_GET_U32_REG_VAL(BT_CONTROLLER_CRASH_DUMP_ADDR_BASE +172));
    BT_DRV_TRACE(1,"REG:[R4] = 0x%08x", BT_DRIVER_GET_U32_REG_VAL(BT_CONTROLLER_CRASH_DUMP_ADDR_BASE +4));
    BT_DRV_TRACE(1,"REG:[R5] = 0x%08x", BT_DRIVER_GET_U32_REG_VAL(BT_CONTROLLER_CRASH_DUMP_ADDR_BASE +8));
    BT_DRV_TRACE(1,"REG:[R6] = 0x%08x", BT_DRIVER_GET_U32_REG_VAL(BT_CONTROLLER_CRASH_DUMP_ADDR_BASE +12));
    BT_DRV_TRACE(1,"REG:[R7] = 0x%08x", BT_DRIVER_GET_U32_REG_VAL(BT_CONTROLLER_CRASH_DUMP_ADDR_BASE +16));
    BT_DRV_TRACE(1,"REG:[R8] = 0x%08x", BT_DRIVER_GET_U32_REG_VAL(BT_CONTROLLER_CRASH_DUMP_ADDR_BASE +20));
    hal_sys_timer_delay(MS_TO_TICKS(100));

    //BT_DRV_TRACE(1,"REG:[R9] = 0x%08x", BT_DRIVER_GET_U32_REG_VAL(BT_CONTROLLER_CRASH_DUMP_ADDR_BASE +24));
    BT_DRV_TRACE(1,"REG:[sl] = 0x%08x", BT_DRIVER_GET_U32_REG_VAL(BT_CONTROLLER_CRASH_DUMP_ADDR_BASE +28));
    BT_DRV_TRACE(1,"REG:[fp] = 0x%08x", BT_DRIVER_GET_U32_REG_VAL(BT_CONTROLLER_CRASH_DUMP_ADDR_BASE +32));
    BT_DRV_TRACE(1,"REG:[ip] = 0x%08x", BT_DRIVER_GET_U32_REG_VAL(BT_CONTROLLER_CRASH_DUMP_ADDR_BASE +36));
    BT_DRV_TRACE(1,"REG:[SP,#0] = 0x%08x", BT_DRIVER_GET_U32_REG_VAL(BT_CONTROLLER_CRASH_DUMP_ADDR_BASE +40));
    BT_DRV_TRACE(1,"REG:[SP,#4] = 0x%08x", BT_DRIVER_GET_U32_REG_VAL(BT_CONTROLLER_CRASH_DUMP_ADDR_BASE +44));
    BT_DRV_TRACE(1,"REG:[SP,#8] = 0x%08x", BT_DRIVER_GET_U32_REG_VAL(BT_CONTROLLER_CRASH_DUMP_ADDR_BASE +48));
    BT_DRV_TRACE(1,"REG:[SP,#12] = 0x%08x", BT_DRIVER_GET_U32_REG_VAL(BT_CONTROLLER_CRASH_DUMP_ADDR_BASE +52));
    BT_DRV_TRACE(1,"REG:[SP,#16] = 0x%08x", BT_DRIVER_GET_U32_REG_VAL(BT_CONTROLLER_CRASH_DUMP_ADDR_BASE +56));
    BT_DRV_TRACE(1,"REG:[SP,#20] = 0x%08x", BT_DRIVER_GET_U32_REG_VAL(BT_CONTROLLER_CRASH_DUMP_ADDR_BASE +60));
    BT_DRV_TRACE(1,"REG:[SP,#24] = 0x%08x", BT_DRIVER_GET_U32_REG_VAL(BT_CONTROLLER_CRASH_DUMP_ADDR_BASE +64));
    BT_DRV_TRACE(1,"REG:[SP,#28] = 0x%08x", BT_DRIVER_GET_U32_REG_VAL(BT_CONTROLLER_CRASH_DUMP_ADDR_BASE +68));
    hal_sys_timer_delay(MS_TO_TICKS(100));

    BT_DRV_TRACE(1,"REG:[SP,#32] = 0x%08x", BT_DRIVER_GET_U32_REG_VAL(BT_CONTROLLER_CRASH_DUMP_ADDR_BASE +72));
    BT_DRV_TRACE(1,"REG:[SP,#36] = 0x%08x", BT_DRIVER_GET_U32_REG_VAL(BT_CONTROLLER_CRASH_DUMP_ADDR_BASE +76));
    BT_DRV_TRACE(1,"REG:[SP,#40] = 0x%08x", BT_DRIVER_GET_U32_REG_VAL(BT_CONTROLLER_CRASH_DUMP_ADDR_BASE +80));
    BT_DRV_TRACE(1,"REG:[SP,#44] = 0x%08x", BT_DRIVER_GET_U32_REG_VAL(BT_CONTROLLER_CRASH_DUMP_ADDR_BASE +84));
    BT_DRV_TRACE(1,"REG:[SP,#48] = 0x%08x", BT_DRIVER_GET_U32_REG_VAL(BT_CONTROLLER_CRASH_DUMP_ADDR_BASE +88));
    BT_DRV_TRACE(1,"REG:[SP,#52] = 0x%08x", BT_DRIVER_GET_U32_REG_VAL(BT_CONTROLLER_CRASH_DUMP_ADDR_BASE +92));
    BT_DRV_TRACE(1,"REG:[SP,#56] = 0x%08x", BT_DRIVER_GET_U32_REG_VAL(BT_CONTROLLER_CRASH_DUMP_ADDR_BASE +96));
    BT_DRV_TRACE(1,"REG:[SP,#60] = 0x%08x", BT_DRIVER_GET_U32_REG_VAL(BT_CONTROLLER_CRASH_DUMP_ADDR_BASE +100));
    BT_DRV_TRACE(1,"REG:[SP,#64] = 0x%08x", BT_DRIVER_GET_U32_REG_VAL(BT_CONTROLLER_CRASH_DUMP_ADDR_BASE +104));
    BT_DRV_TRACE(1,"REG:SP = 0x%08x",  BT_DRIVER_GET_U32_REG_VAL(BT_CONTROLLER_CRASH_DUMP_ADDR_BASE +108));
    BT_DRV_TRACE(1,"REG:MSP = 0x%08x", BT_DRIVER_GET_U32_REG_VAL(BT_CONTROLLER_CRASH_DUMP_ADDR_BASE +112));
    BT_DRV_TRACE(1,"REG:PSP = 0x%08x", BT_DRIVER_GET_U32_REG_VAL(BT_CONTROLLER_CRASH_DUMP_ADDR_BASE +116));
    BT_DRV_TRACE(1,"REG:CFSR = 0x%08x", BT_DRIVER_GET_U32_REG_VAL(BT_CONTROLLER_CRASH_DUMP_ADDR_BASE +120));
    BT_DRV_TRACE(1,"REG:BFAR = 0x%08x", BT_DRIVER_GET_U32_REG_VAL(BT_CONTROLLER_CRASH_DUMP_ADDR_BASE +124));
    BT_DRV_TRACE(1,"REG:HFSR = 0x%08x", BT_DRIVER_GET_U32_REG_VAL(BT_CONTROLLER_CRASH_DUMP_ADDR_BASE +128));
    BT_DRV_TRACE(1,"REG:ICSR = 0x%08x", BT_DRIVER_GET_U32_REG_VAL(BT_CONTROLLER_CRASH_DUMP_ADDR_BASE +132));
    BT_DRV_TRACE(1,"REG:AIRCR = 0x%08x", BT_DRIVER_GET_U32_REG_VAL(BT_CONTROLLER_CRASH_DUMP_ADDR_BASE +136));
    BT_DRV_TRACE(1,"REG:SCR = 0x%08x", BT_DRIVER_GET_U32_REG_VAL(BT_CONTROLLER_CRASH_DUMP_ADDR_BASE +140));
    BT_DRV_TRACE(1,"REG:CCR = 0x%08x", BT_DRIVER_GET_U32_REG_VAL(BT_CONTROLLER_CRASH_DUMP_ADDR_BASE +144));
    BT_DRV_TRACE(1,"REG:SHCSR = 0x%08x", BT_DRIVER_GET_U32_REG_VAL(BT_CONTROLLER_CRASH_DUMP_ADDR_BASE +148));
    BT_DRV_TRACE(1,"REG:AFSR = 0x%08x", BT_DRIVER_GET_U32_REG_VAL(BT_CONTROLLER_CRASH_DUMP_ADDR_BASE +152));
    BT_DRV_TRACE(1,"REG:MMFAR = 0x%08x", BT_DRIVER_GET_U32_REG_VAL(BT_CONTROLLER_CRASH_DUMP_ADDR_BASE +156));
    hal_sys_timer_delay(MS_TO_TICKS(100));
    //task_message_buffer
    uint32_t buff_addr = 0;

    if(task_message_buffer_addr)
    {
        buff_addr = task_message_buffer_addr;
    }

    BT_DRV_TRACE(2,"0xd0330050: 0x%x, 54:0x%x", BT_DRIVER_GET_U32_REG_VAL(0xd0330050), BT_DRIVER_GET_U32_REG_VAL(0xd0330054));
    BT_DRV_TRACE(2,"0x400000a0: 0x%x, a4:0x%x", BT_DRIVER_GET_U32_REG_VAL(0x400000a0), BT_DRIVER_GET_U32_REG_VAL(0x400000a4));

    if(g_mem_dump_ctrl_addr)
    {
        BT_DRV_TRACE(1,"LMP addr=0x%x", BT_DRIVER_GET_U32_REG_VAL(g_mem_dump_ctrl_addr+4));
        BT_DRV_TRACE(1,"STA addr=0x%x", BT_DRIVER_GET_U32_REG_VAL(g_mem_dump_ctrl_addr+0x10));
        BT_DRV_TRACE(1,"MSG addr=0x%x", BT_DRIVER_GET_U32_REG_VAL(g_mem_dump_ctrl_addr+0x1c));
        BT_DRV_TRACE(1,"SCH addr=0x%x", BT_DRIVER_GET_U32_REG_VAL(g_mem_dump_ctrl_addr+0x28));
        BT_DRV_TRACE(1,"ISR addr=0x%x", BT_DRIVER_GET_U32_REG_VAL(g_mem_dump_ctrl_addr+0x34));
    }

    BT_DRV_TRACE(0,"task msg buff:");
    if(buff_addr != 0)
    {
        for(uint8_t j=0; j<5; j++)
        {
            DUMP8("%02x ", (uint8_t *)(uintptr_t)(buff_addr+j*20), 20);
        }
    }
    hal_sys_timer_delay(MS_TO_TICKS(100));

    BT_DRV_TRACE(0," ");
    BT_DRV_TRACE(0,"lmp buff:");
    //lmp_message_buffer

    if(lmp_message_buffer_addr)
    {
        buff_addr = lmp_message_buffer_addr;
    }

    if(buff_addr != 0)
    {
        for(uint8_t j=0; j<10; j++)
        {
            DUMP8("%02x ",(uint8_t *)(uintptr_t)(buff_addr+j*20), 20);
        }
    }
    hal_sys_timer_delay(MS_TO_TICKS(100));

    uint8_t link_id = 0;
    uint32_t evt_ptr = 0;
    POSSIBLY_UNUSED uint32_t acl_par_ptr = 0;
    for(link_id=0; link_id<3; link_id++)
    {
        BT_DRV_TRACE(1,"acl_par: link id %d",link_id);

        if(ld_acl_env_addr)
        {
            evt_ptr = BT_DRIVER_GET_U32_REG_VAL(ld_acl_env_addr+link_id*4);
        }


        if (evt_ptr)
        {
            acl_par_ptr = evt_ptr;
            BT_DRV_TRACE(5,"acl_par: acl_par_ptr 0x%x, clk off 0x%x, bit off 0x%x, last sync clk off 0x%x, last sync bit off 0x%x",
                         acl_par_ptr, BT_DRIVER_GET_U32_REG_VAL(acl_par_ptr+140), BT_DRIVER_GET_U16_REG_VAL(acl_par_ptr+150),
                         BT_DRIVER_GET_U32_REG_VAL(acl_par_ptr+136),(BT_DRIVER_GET_U32_REG_VAL(acl_par_ptr+150)&0xFFFF0000)>>16);
        }
    }
    hal_sys_timer_delay(MS_TO_TICKS(100));

    //ld_sco_env
    evt_ptr = 0;

    if(ld_sco_env_addr)
    {
        evt_ptr = BT_DRIVER_GET_U32_REG_VAL(ld_sco_env_addr);
    }

    if(evt_ptr != 0)
    {
        BT_DRV_TRACE(1,"esco linkid :%d", BT_DRIVER_GET_U8_REG_VAL(evt_ptr+70));
        for(link_id=0; link_id<MAX_NB_ACTIVE_ACL; link_id++)
        {
            BT_DRV_TRACE(2,"bt_linkcntl_linklbl 0x%x: link id %d", BT_DRIVER_GET_U16_REG_VAL(EM_BT_LINKCNTL_ADDR+link_id*BT_EM_SIZE),link_id);
            BT_DRV_TRACE(1,"rxcount :%x", BT_DRIVER_GET_U16_REG_VAL(EM_BT_RXDESCCNT_ADDR+link_id*BT_EM_SIZE));
        }
    }
    btdrv_dump_mem(bt_dump_mem_start, bt_dump_len_max, BT_SUB_SYS_TYPE);
    //btdrv_dump_mem(em_dump_area_1_start, em_area_1_len_max, BT_EM_AREA_1_TYPE);
    btdrv_dump_mem(em_dump_area_2_start, em_area_2_len_max, BT_EM_AREA_2_TYPE);
    //btdrv_dump_mem(mcu_dump_mem_start, mcu_dump_len_max, MCU_SYS_TYPE);

    BT_DRV_TRACE(0,"BT crash dump complete!");

    BT_DRV_REG_OP_CLK_DIB();
}


void bt_drv_reg_op_set_swagc_mode(uint8_t mode)
{
}

void bt_drv_reg_op_force_sco_retrans(bool enable)
{
#ifdef __FORCE_SCO_MAX_RETX__
    BT_DRV_REG_OP_ENTER();
    BT_DRV_REG_OP_CLK_ENB();
    if (enable)
    {
        BTDIGITAL_REG_SET_FIELD(0xd0220468,3,24,3);
    }
    else
    {
        BTDIGITAL_REG_SET_FIELD(0xd0220468,3,24,0);
    }
    BT_DRV_REG_OP_CLK_DIB();
    BT_DRV_REG_OP_EXIT();
#endif
}

bool bt_drv_reg_op_read_rssi_in_dbm(uint16_t connHandle,rx_agc_t* rx_val)
{
#ifdef BT_RSSI_MONITOR
    BT_DRV_REG_OP_ENTER();
    BT_DRV_REG_OP_CLK_ENB();

    if (!rx_val)
    {
        goto exit;
    }

    if (connHandle == 0xFFFF)
    {
        goto exit;
    }

    {
        uint8_t idx = btdrv_conhdl_to_linkid(connHandle);
        /// Accumulated RSSI (to compute an average value)
        //  int16_t rssi_acc = 0;
        /// Counter of received packets used in RSSI average
        //     uint8_t rssi_avg_cnt = 1;
        rx_agc_t * rx_monitor_env = NULL;
        uint32_t acl_par_ptr = 0;
        uint32_t rssi_record_ptr=0;

        if (!btdrv_is_link_index_valid(idx))
        {
            goto exit;
        }

        if(ld_acl_env_addr)
        {
            acl_par_ptr = BT_DRIVER_GET_U32_REG_VAL(ld_acl_env_addr+idx*4);
            if(acl_par_ptr)
                rssi_record_ptr = acl_par_ptr+0xb0;
        }

        //rx_monitor
        if(rx_monitor_addr)
        {
            rx_monitor_env = (rx_agc_t*)rx_monitor_addr;
        }

        if(rssi_record_ptr != 0 && rx_monitor_env != 0)
        {
        #ifdef __NEW_SWAGC_MODE__
            // BT_DRV_TRACE(1,"addr=%x,rssi=%d",rssi_record_ptr,*(int8 *)rssi_record_ptr);
            if(bt_drv_reg_op_bt_sync_swagc_en_get())
            {
                rx_val->rssi = *(int8_t *)rssi_record_ptr; // work mode NEW_SYNC_SWAGC_MODE
            }
            else
            {
                rx_val->rssi = rx_monitor_env[idx].rssi; //idle mode OLD_SWAGC_MODE
            }
        #else
            rx_val->rssi = rx_monitor_env[idx].rssi; //idle mode OLD_SWAGC_MODE
        #endif
            rx_val->rxgain = rx_monitor_env[idx].rxgain;
        }
    }

exit:
    BT_DRV_REG_OP_CLK_DIB();
    BT_DRV_REG_OP_EXIT();
    return true;
#else
    return false;
#endif
}

bool bt_drv_reg_op_read_ble_rssi_in_dbm(uint16_t connHandle,rx_agc_t* rx_val)
{
#ifdef BT_RSSI_MONITOR
    BT_DRV_REG_OP_ENTER();
    BT_DRV_REG_OP_CLK_ENB();

    if (connHandle == 0xFFFF)
    {
        goto exit;
    }

    {
        uint8_t idx = connHandle;
        /// Accumulated RSSI (to compute an average value)
        int16_t rssi_acc = 0;
        /// Counter of received packets used in RSSI average
        uint8_t rssi_avg_cnt = 1;
        rx_agc_t * rx_monitor_env = NULL;
        if(idx > BLE_IDX_IN_TESTMODE)
        {
            goto exit;
        }

        //rx_monitor

        if(ble_rx_monitor_addr)
        {
            rx_monitor_env = (rx_agc_t*)ble_rx_monitor_addr;
        }

        if(rx_monitor_env != NULL)
        {
            for(int i=0; i< rssi_avg_cnt; i++)
            {
                rssi_acc += rx_monitor_env[idx].rssi;
            }
            rx_val->rssi = rssi_acc / rssi_avg_cnt;
            rx_val->rxgain = rx_monitor_env[idx].rxgain;
        }
    }
exit:
    BT_DRV_REG_OP_CLK_DIB();
    BT_DRV_REG_OP_EXIT();
    return true;
#else
    return false;
#endif
}

void bt_drv_reg_op_ibrt_retx_att_nb_set(uint8_t retx_nb)
{
    BT_DRV_REG_OP_ENTER();
    BT_DRV_REG_OP_CLK_ENB();

    int ret = -1;
    uint32_t sco_evt_ptr = 0x0;
    // TODO: [ld_sco_env address] based on CHIP id

    if(ld_sco_env_addr)
    {
        sco_evt_ptr = BT_DRIVER_GET_U32_REG_VAL(ld_sco_env_addr);
        ret = 0;
    }

    if(ret == 0)
    {
        uint32_t retx_ptr=0x0;
        if(sco_evt_ptr !=0)
        {
            //offsetof(struct ea_elt_tag, env) + sizeof(struct ld_sco_evt_params)
            //0x1c+0x28
            retx_ptr =sco_evt_ptr+0x44;
        }
        else
        {
            BT_DRV_TRACE(0,"BT_REG_OP:Error, ld_sco_env[0].evt ==NULL");
            ret = -2;
        }

        if(ret == 0)
        {
            *(volatile uint8_t *)retx_ptr = retx_nb;
        }
    }

    BT_DRV_TRACE(3,"BT_REG_OP:%s,ret=%d,retx nb=%d",__func__,ret,retx_nb);

    BT_DRV_REG_OP_CLK_DIB();
    BT_DRV_REG_OP_EXIT();
}

void bt_drv_reg_op_acl_tx_type_set(uint16_t hciHandle, uint8_t slot_sel)
{
    uint32_t lc_ptr=0;
    uint32_t acl_par_ptr = 0;
    uint32_t packet_type_addr = 0;
    //lc_env and ld_acl_env

    BT_DRV_REG_OP_ENTER();
    BT_DRV_REG_OP_CLK_ENB();

    if(lc_env_addr)
    {
        lc_ptr = BT_DRIVER_GET_U32_REG_VAL(lc_env_addr+(hciHandle-0x80)*4);
    }

    if(ld_acl_env_addr)
    {
        acl_par_ptr = BT_DRIVER_GET_U32_REG_VAL(ld_acl_env_addr+(hciHandle-0x80)*4);
    }

    //sanity check
    if(lc_ptr == 0)
    {
        goto exit;
    }
    else
    {
        //    off=b0 rssi_rcord
        //        b1 br_type
        //        b2 edr_type
        packet_type_addr = (uint32_t)(acl_par_ptr+0xb2);
        BT_DRV_TRACE(3,"BT_REG_OP:%s   hdl=0x%x packet_types=0x%x",__func__,hciHandle, BT_DRIVER_GET_U8_REG_VAL(packet_type_addr));
        BT_DRV_TRACE(2,"BT_REG_OP:lc_ptr 0x%x, acl_par_ptr 0x%x",lc_ptr,acl_par_ptr);
        uint16_t edr_type = 0;
        if(slot_sel == USE_1_SLOT_PACKET)
        {
#if defined(__3M_PACK__)
            edr_type = (1 << DM1_IDX) | (1 << DH1_2_IDX) | (1 << DH1_3_IDX);
#else
            edr_type = (1 << DM1_IDX) | (1 << DH1_2_IDX);
#endif
        }
        else if(slot_sel == USE_3_SLOT_PACKET)
        {
#if defined(__3M_PACK__)
            edr_type = (1 << DM1_IDX) | (1 << DH1_2_IDX) | (1 << DH1_3_IDX) | \
                       (1 << DH3_2_IDX) |(1 << DH3_3_IDX);
#else
            edr_type = (1 << DM1_IDX) | (1 << DH1_2_IDX) | (1 << DH3_2_IDX);
#endif
        }
        else if(slot_sel == USE_5_SLOT_PACKET)
        {
#if defined(__3M_PACK__)
            edr_type = (1 << DM1_IDX) | (1 << DH1_2_IDX) | (1 << DH1_3_IDX) | \
                       (1 << DH3_2_IDX) |(1 << DH3_3_IDX) |(1 << DH5_2_IDX) | (1 << DH5_3_IDX);
#else
            edr_type = (1 << DM1_IDX) | (1 << DH1_2_IDX) | (1 << DH3_2_IDX)|(1 << DH5_2_IDX);
#endif
        }

        BT_DRIVER_GET_U8_REG_VAL(packet_type_addr) = edr_type;

        BT_DRV_TRACE(1,"BT_REG_OP:After op,packet_types 0x%x", BT_DRIVER_GET_U8_REG_VAL(packet_type_addr) );
    }

exit:
    BT_DRV_REG_OP_CLK_DIB();
    BT_DRV_REG_OP_EXIT();
}

void bt_drv_reg_op_acl_tx_type_trace(uint16_t hciHandle)
{
    uint32_t acl_par_ptr = 0;
    uint32_t packet_type_addr = 0;
    //ld_acl_env

    BT_DRV_REG_OP_ENTER();
    BT_DRV_REG_OP_CLK_ENB();

    if(ld_acl_env_addr)
    {
        acl_par_ptr = BT_DRIVER_GET_U32_REG_VAL(ld_acl_env_addr+(hciHandle-0x80)*4);
    }

    //sanity check
    if(acl_par_ptr == 0)
    {
        goto exit;
    }
    else
    {
        //    off=b0 rssi_rcord
        //        b1 br_type
        //        b2 edr_type
        packet_type_addr = (uint32_t)(acl_par_ptr+0xb0);
        POSSIBLY_UNUSED uint16_t type = ((BT_DRIVER_GET_U32_REG_VAL(packet_type_addr))>>8) & 0xffff;
        BT_DRV_TRACE(1,"BT_REG_OP:tx packet_types=0x%x",type);
    }

exit:
    BT_DRV_REG_OP_CLK_DIB();
    BT_DRV_REG_OP_EXIT();
}

uint8_t bt_drv_reg_op_acl_tx_type_get(uint16_t hciHandle, uint8_t* br_type, uint8_t* edr_type)
{
    uint8_t status = 0xff;
    uint32_t lc_ptr=0;
    uint32_t acl_par_ptr = 0;
    uint32_t packet_type_addr = 0;

    BT_DRV_REG_OP_ENTER();
    BT_DRV_REG_OP_CLK_ENB();

    if(lc_env_addr)
    {
        lc_ptr = BT_DRIVER_GET_U32_REG_VAL(lc_env_addr+(hciHandle-0x80)*4);
    }

    if(ld_acl_env_addr)
    {
        acl_par_ptr = BT_DRIVER_GET_U32_REG_VAL(ld_acl_env_addr+(hciHandle-0x80)*4);
    }


    //sanity check
    if(lc_ptr == 0)
    {
        BT_DRV_TRACE(2,"BT_REG_OP:%s hdl=0x%x,lc_env_ptr = NULL",__func__,hciHandle);
    }
    else
    {
        packet_type_addr = (uint32_t)(acl_par_ptr+0xb0);
        uint32_t packet_type = BT_DRIVER_GET_U32_REG_VAL(packet_type_addr);
        //off=b0 rssi_rcord
        //    b1 br_type
        //    b2 edr_type
        if(br_type)
        {
            *br_type = (packet_type>>8)&0xff;
        }
        if(edr_type)
        {
            *edr_type = (packet_type>>16)&0xff;
        }
        status = 0;
        BT_DRV_TRACE(3,"BT_REG_OP:%s hdl=0x%x packet_types=0x%x",__func__,hciHandle, packet_type);
    }

    BT_DRV_REG_OP_CLK_DIB();
    BT_DRV_REG_OP_EXIT();
    return status;
}

uint8_t  bt_drv_reg_op_get_role(uint8_t linkid)
{
    uint32_t lc_evt_ptr=0;
    uint32_t role_ptr = 0;
    uint8_t role = 0xff;

    BT_DRV_REG_OP_ENTER();
    BT_DRV_REG_OP_CLK_ENB();
    if(lc_env_addr)
    {
        lc_evt_ptr = BT_DRIVER_GET_U32_REG_VAL(lc_env_addr+linkid*4);//lc_env
    }

    if(lc_evt_ptr !=0)
    {
        //role off:0x40
        role_ptr = lc_evt_ptr+0x40;
        role = BT_DRIVER_GET_U8_REG_VAL(role_ptr);
    }
    else
    {
        BT_DRV_TRACE(1,"BT_REG_OP:ERROR LINKID =%x",linkid);
        role = 0xff;
    }

    BT_DRV_REG_OP_CLK_DIB();
    BT_DRV_REG_OP_EXIT();

    return role;
}

void bt_drv_reg_op_set_tpoll(uint8_t linkid,uint16_t poll_interval)
{
    uint32_t acl_evt_ptr = 0x0;
    uint32_t poll_addr;

    BT_DRV_REG_OP_ENTER();
    BT_DRV_REG_OP_CLK_ENB();
    if(ld_acl_env_addr)
    {
        acl_evt_ptr = BT_DRIVER_GET_U32_REG_VAL(ld_acl_env_addr+linkid*4);
    }

    if (acl_evt_ptr != 0)
    {
        //poll_addr = acl_evt_ptr + 0xb8;
        //tpoll off:0x28+92
        poll_addr = acl_evt_ptr + 0xba;
        BT_DRIVER_GET_U16_REG_VAL(poll_addr) = poll_interval;
    }
    else
    {
        BT_DRV_TRACE(1,"BT_REG_OP:ERROR LINK ID FOR TPOLL %x", linkid);
    }
    BT_DRV_REG_OP_CLK_DIB();
    BT_DRV_REG_OP_EXIT();
}

uint16_t bt_drv_reg_op_get_tpoll(uint8_t linkid)
{
    uint32_t acl_evt_ptr = 0x0;
    uint32_t poll_addr;
    uint16_t poll_interval = 0;

    BT_DRV_REG_OP_ENTER();
    BT_DRV_REG_OP_CLK_ENB();
    if(ld_acl_env_addr)
    {
        acl_evt_ptr = BT_DRIVER_GET_U32_REG_VAL(ld_acl_env_addr+linkid*4);
    }

    if (acl_evt_ptr != 0)
    {
        //poll_addr = acl_evt_ptr + 0xb8;
        //tpoll off:0x28+92
        poll_addr = acl_evt_ptr + 0xba;
        poll_interval = BT_DRIVER_GET_U16_REG_VAL(poll_addr);
    }
    else
    {
        BT_DRV_TRACE(1,"BT_REG_OP:ERROR LINK ID FOR TPOLL %x", linkid);
    }
    BT_DRV_REG_OP_CLK_DIB();
    BT_DRV_REG_OP_EXIT();

    return poll_interval;
}

int8_t  bt_drv_reg_op_rssi_correction(int8_t rssi)
{
    return rssi;
}

void bt_drv_reg_op_set_music_link(uint8_t link_id)
{
    BT_DRV_REG_OP_ENTER();
    BT_DRV_REG_OP_CLK_ENB();
    //dbg_bt_setting.music_playing_link
    if(dbg_bt_setting_addr)
    {
        BT_DRIVER_GET_U8_REG_VAL(dbg_bt_setting_addr+0x10) = link_id;
    }
    BT_DRV_REG_OP_CLK_DIB();
    BT_DRV_REG_OP_EXIT();
}

void bt_drv_reg_op_set_music_link_duration_extra(uint8_t slot)
{
    BT_DRV_REG_OP_ENTER();
    BT_DRV_REG_OP_CLK_ENB();
    if(dbg_bt_setting_addr)
    {
        BT_DRIVER_GET_U32_REG_VAL(dbg_bt_setting_addr+0x14) = slot*625;
    }
    BT_DRV_REG_OP_CLK_DIB();
    BT_DRV_REG_OP_EXIT();
}

uint32_t bt_drv_reg_op_get_reconnecting_flag()
{
    uint32_t ret = 0;

    BT_DRV_REG_OP_ENTER();
    BT_DRV_REG_OP_CLK_ENB();
    if(reconnecting_flag_addr)
    {
        ret = BT_DRIVER_GET_U32_REG_VAL(reconnecting_flag_addr);
    }
    BT_DRV_REG_OP_CLK_DIB();
    BT_DRV_REG_OP_EXIT();

    return ret;
}

void bt_drv_reg_op_set_reconnecting_flag()
{
    BT_DRV_REG_OP_ENTER();
    BT_DRV_REG_OP_CLK_ENB();
    if(reconnecting_flag_addr)
    {
        BT_DRIVER_GET_U32_REG_VAL(reconnecting_flag_addr) = 1;
    }
    BT_DRV_REG_OP_CLK_DIB();
    BT_DRV_REG_OP_EXIT();
}

void bt_drv_reg_op_clear_reconnecting_flag()
{
    BT_DRV_REG_OP_ENTER();
    BT_DRV_REG_OP_CLK_ENB();
    if(reconnecting_flag_addr)
    {
        BT_DRIVER_GET_U32_REG_VAL(reconnecting_flag_addr) = 0;
    }
    BT_DRV_REG_OP_CLK_DIB();
    BT_DRV_REG_OP_EXIT();
}

void bt_drv_reg_op_music_link_config(uint16_t active_link,uint8_t active_role,uint16_t inactive_link,uint8_t inactive_role)
{
    BT_DRV_TRACE(4,"BT_REG_OP:bt_drv_reg_op_music_link_config %x %d %x %d",active_link,active_role,inactive_link,inactive_role);
    BT_DRV_REG_OP_ENTER();
    BT_DRV_REG_OP_CLK_ENB();
    if (active_role == 0) //MASTER
    {
        bt_drv_reg_op_set_tpoll(active_link-0x80, 0x10);
        if (inactive_role == 0)
        {
            bt_drv_reg_op_set_tpoll(inactive_link-0x80, 0x40);
        }
    }
    else
    {
        bt_drv_reg_op_set_music_link(active_link-0x80);
        bt_drv_reg_op_set_music_link_duration_extra(11);
        if (inactive_role == 0)
        {
            bt_drv_reg_op_set_tpoll(inactive_link-0x80, 0x40);
        }
    }
    BT_DRV_REG_OP_CLK_DIB();
    BT_DRV_REG_OP_EXIT();
}

bool bt_drv_reg_op_check_bt_controller_state(void)
{
    bool ret=true;
    if(hal_sysfreq_get() <= HAL_CMU_FREQ_32K)
        return ret;

    BT_DRV_REG_OP_ENTER();
    BT_DRV_REG_OP_CLK_ENB();

    if((BT_DRIVER_GET_U32_REG_VAL(BT_CONTROLLER_CRASH_DUMP_ADDR_BASE +0)==0x00)
       &&(BT_DRIVER_GET_U32_REG_VAL(BT_CONTROLLER_CRASH_DUMP_ADDR_BASE +8)==MIC_BUFF_addr))
        ret = true;
    else
        ret = false;

    if (false == ret)
    {
        BT_DRV_TRACE(0,"controller dead!!!");
        btdrv_trigger_coredump();
    }

    BT_DRV_REG_OP_CLK_DIB();
    BT_DRV_REG_OP_EXIT();
    return ret;
}

uint16_t bt_drv_reg_op_bitoff_getf(int elt_idx)
{
    uint16_t bitoff = 0;
    uint32_t acl_evt_ptr = 0x0;
    BT_DRV_REG_OP_ENTER();
    BT_DRV_REG_OP_CLK_ENB();
    if(ld_acl_env_addr)
    {
        acl_evt_ptr = *(uint32_t *)(ld_acl_env_addr+elt_idx*4);
    }

    if (acl_evt_ptr != 0)
    {
        //bitoff = acl_evt_ptr + 0x96;
        bitoff = *(uint16_t *)(acl_evt_ptr + 0x96);
        //BT_DRV_TRACE(1,"[%s] bitoff=%d\n",__func__,bitoff);
    }
    else
    {
        BT_DRV_TRACE(1,"BT_REG_OP:ERROR LINK ID FOR RD bitoff %x", elt_idx);
    }
    BT_DRV_REG_OP_CLK_DIB();
    BT_DRV_REG_OP_EXIT();

    return bitoff;
}

int32_t bt_drv_reg_op_get_clkoffset(uint16_t linkid)
{
    //clock offset in 625us BT slot
    uint32_t clkoff = 0;
    uint32_t acl_evt_ptr = 0x0;
    int32_t offset = 0;
    uint32_t local_offset;

    BT_DRV_REG_OP_CLK_ENB();
    if(ld_acl_env_addr)
    {
        acl_evt_ptr = *(uint32_t *)(ld_acl_env_addr+linkid*4);
    }

    if (acl_evt_ptr != 0)
    {
        clkoff = *(uint32_t *)(acl_evt_ptr + 0x8c);
        //BT_DRV_TRACE(1,"[%s] clkoff=%x\n",__func__,clkoff);
    }
    else
    {
        BT_DRV_TRACE(1,"BT_REG_OP:ERROR LINK ID FOR RD clkoff %x", linkid);
    }

    BT_DRV_REG_OP_CLK_DIB();
    local_offset = clkoff & 0x7FFFFFF;
    offset = local_offset;
    offset = (offset << 5)>>5;
   return offset;
}

void bt_drv_reg_op_piconet_clk_offset_get(uint16_t connHandle, int32_t *clock_offset, uint16_t *bit_offset)
{
    uint8_t index = 0;

    BT_DRV_REG_OP_ENTER();
    BT_DRV_REG_OP_CLK_ENB();
    if (connHandle)
    {
        index = btdrv_conhdl_to_linkid(connHandle);

        if (btdrv_is_link_index_valid(index))
        {
            *bit_offset = bt_drv_reg_op_bitoff_getf(index);
            *clock_offset = bt_drv_reg_op_get_clkoffset(index);
        }
        else
        {
            *bit_offset = 0;
            *clock_offset = 0;
        }
    }
    else
    {
        *bit_offset = 0;
        *clock_offset = 0;
    }
    BT_DRV_REG_OP_CLK_DIB();
    BT_DRV_REG_OP_EXIT();
}

void bt_drv_reg_op_dma_tc_clkcnt_get(uint32_t *btclk, uint16_t *btcnt)
{
    //DMA cnt in BT slot (625us)
    BT_DRV_REG_OP_ENTER();
    BT_DRV_REG_OP_CLK_ENB();
    *btclk = BT_DRIVER_GET_U32_REG_VAL(0xd02201fc);
    *btcnt = BT_DRIVER_GET_U32_REG_VAL(0xd02201f8);
    BT_DRV_REG_OP_CLK_DIB();
    BT_DRV_REG_OP_EXIT();
}

void bt_drv_reg_op_acl_tx_silence(uint16_t connHandle, uint8_t on)
{
}

void bt_drv_reg_op_acl_tx_silence_clear(uint16_t connHandle)
{
}

void bt_drv_reg_op_sw_seq_filter(uint16_t connHandle)
{
}

void bt_drv_reg_op_pcm_set(uint8_t en)
{
    BT_DRV_REG_OP_ENTER();
    BT_DRV_REG_OP_CLK_ENB();
    if(en)
        *(volatile unsigned int *)(0xd02204a8) &= 0xffffffdf;
    else
        *(volatile unsigned int *)(0xd02204a8) |= 1<<5;
    BT_DRV_REG_OP_CLK_DIB();
    BT_DRV_REG_OP_EXIT();
}

uint8_t bt_drv_reg_op_pcm_get()
{
    uint8_t ret = 1;
    BT_DRV_REG_OP_ENTER();
    BT_DRV_REG_OP_CLK_ENB();
    ret = (*(volatile unsigned int *)(0xd02204a8) &0x00000020)>>5;
    BT_DRV_REG_OP_CLK_DIB();
    BT_DRV_REG_OP_EXIT();
    return ~ret;
}

const uint8_t msbc_mute_patten[]=
{
    0x01,0x38,
    0xad, 0x0,  0x0,  0xc5, 0x0,  0x0, 0x0, 0x0,
    0x77, 0x6d, 0xb6, 0xdd, 0xdb, 0x6d, 0xb7,
    0x76, 0xdb, 0x6d, 0xdd, 0xb6, 0xdb, 0x77,
    0x6d, 0xb6, 0xdd, 0xdb, 0x6d, 0xb7, 0x76,
    0xdb, 0x6d, 0xdd, 0xb6, 0xdb, 0x77, 0x6d,
    0xb6, 0xdd, 0xdb, 0x6d, 0xb7, 0x76, 0xdb,
    0x6d, 0xdd, 0xb6, 0xdb, 0x77, 0x6d, 0xb6,
    0xdd, 0xdb, 0x6d, 0xb7, 0x76, 0xdb, 0x6c,
    0x00
};
#define SCO_TX_FIFO_BASE (0xd0210000)

void bt_drv_reg_op_sco_txfifo_reset(uint16_t codec_id)
{
    BT_DRV_REG_OP_ENTER();
    BT_DRV_REG_OP_CLK_ENB();
    uint32_t reg_val = BT_DRIVER_GET_U32_REG_VAL(0xd0220148);
    uint32_t reg_offset0, reg_offset1;
    uint16_t *patten_p = (uint16_t *)msbc_mute_patten;

    reg_offset0 = (reg_val & 0xffff);
    reg_offset1 = (reg_val >> 16) & 0xffff;

    if (codec_id == 2)
    {
        for (uint8_t i=0; i<60; i+=2)
        {
            BT_DRIVER_GET_U16_REG_VAL(SCO_TX_FIFO_BASE+reg_offset0+i) = *patten_p;
            BT_DRIVER_GET_U16_REG_VAL(SCO_TX_FIFO_BASE+reg_offset1+i) = *patten_p;
            patten_p++;
        }
    }
    else
    {
        for (uint8_t i=0; i<120; i+=2)
        {
            BT_DRIVER_GET_U16_REG_VAL(SCO_TX_FIFO_BASE+reg_offset0+i) = SCO_TX_MUTE_PATTERN;
            BT_DRIVER_GET_U16_REG_VAL(SCO_TX_FIFO_BASE+reg_offset1+i) = SCO_TX_MUTE_PATTERN;
        }
    }
    BT_DRV_REG_OP_CLK_DIB();
    BT_DRV_REG_OP_EXIT();
}

/*****************************************************************************
 Prototype    : btdrv_set_tws_acl_poll_interval
 Description  : in ibrt mode, set tws acl poll interval
 Input        : uint16_t poll_interval
 Output       : None
 Return Value :
 Calls        :
 Called By    :

 History        :
 Date         : 2019/4/19
 Author       : bestechnic
 Modification : Created function

*****************************************************************************/
void btdrv_reg_op_set_private_tws_poll_interval(uint16_t poll_interval, uint16_t poll_interval_in_sco)
{
    BT_DRV_TRACE(2,"BT_REG_OP:Set private tws interval,acl interv=%d,acl interv insco =%d", \
                 poll_interval,poll_interval_in_sco);

    BT_DRV_REG_OP_ENTER();
    BT_DRV_REG_OP_CLK_ENB();
    if(dbg_bt_setting_addr)
    {
        BT_DRIVER_GET_U16_REG_VAL(dbg_bt_setting_addr+0x40) = poll_interval;
        BT_DRIVER_GET_U16_REG_VAL(dbg_bt_setting_addr+0x42) = poll_interval_in_sco;
    }
    BT_DRV_REG_OP_CLK_DIB();
    BT_DRV_REG_OP_EXIT();
}

void btdrv_reg_op_set_tws_link_duration(uint8_t slot_num)
{
    BT_DRV_REG_OP_ENTER();
    BT_DRV_REG_OP_CLK_ENB();
    //dbg_setting address
    uint32_t op_addr = 0;

    if(dbg_bt_setting_addr)
    {
        op_addr = dbg_bt_setting_addr;
    }

    if(op_addr != 0)
    {
        //uint16_t acl_slot = dbg_setting->acl_slot_in_snoop_mode;
        //acl_slot_in_snoop_mode off:0x44
        uint16_t val = BT_DRIVER_GET_U16_REG_VAL(op_addr+0x44); //acl_slot_in_snoop_mode
        val&=0xff00;
        val|= slot_num;
        BT_DRIVER_GET_U16_REG_VAL(op_addr+0x44) = val;
        BT_DRV_TRACE(1,"BT_REG_OP:Set private tws link duration,val=%d",BT_DRIVER_GET_U16_REG_VAL(op_addr+0x44)&0xff);
    }
    BT_DRV_REG_OP_CLK_DIB();
    BT_DRV_REG_OP_EXIT();
}

void btdrv_reg_op_enable_private_tws_schedule(bool enable)
{
    BT_DRV_REG_OP_ENTER();
    BT_DRV_REG_OP_CLK_ENB();
    uint32_t op_addr = 0;

    if(ld_ibrt_env_addr !=0)
    {
        op_addr = ld_ibrt_env_addr;
    }

    if(op_addr!=0)
    {
        //ld_ibrt_env.acl_switch_flag.in_process
        BT_DRIVER_GET_U8_REG_VAL(op_addr+0x70) = enable;
        BT_DRV_TRACE(1,"BT_REG_OP:Enable private tws function,flag=%d", BT_DRIVER_GET_U8_REG_VAL(op_addr+0x70));
    }
    BT_DRV_REG_OP_CLK_DIB();
    BT_DRV_REG_OP_EXIT();
}

void bt_drv_reg_op_decrease_tx_pwr_when_reconnect(bool enable)
{
    BT_DRV_REG_OP_ENTER();
    BT_DRV_REG_OP_CLK_ENB();
    if(enable)
    {
        BT_DRV_TRACE(0,"BT_REG_OP:Decrese tx pwr");
        //drease defualt TX pwr
        BT_DRIVER_GET_U32_REG_VAL(0xd0350300) = 0x33;
    }
    else
    {
        BT_DRV_TRACE(0,"BT_REG_OP:Increase tx pwr");
        //resume defualt TX pwr
        BT_DRIVER_GET_U32_REG_VAL(0xd0350300) = 0x11;
    }
    BT_DRV_REG_OP_CLK_DIB();
    BT_DRV_REG_OP_EXIT();
}
/*
    rwip_heap_env
    from addr=0xc00027b8 left_length=6K
    rwip_heap_non_ret
    from addr=0xc00052b8 left_length=1.5K
*/
void bt_drv_reg_op_controller_mem_log_config(void)////
{
}

void bt_drv_reg_op_lm_nb_sync_hacker(uint8_t sco_status)
{
}

void bt_drv_reg_op_fastack_status_checker(uint16_t conhdl)
{
    BT_DRV_REG_OP_ENTER();
    BT_DRV_REG_OP_CLK_ENB();

    uint8_t elt_idx = btdrv_conhdl_to_linkid(conhdl);

    if (btdrv_is_link_index_valid(elt_idx))
    {
        POSSIBLY_UNUSED uint32_t fast_cs_addr = EM_BT_BT_EXT1_ADDR + elt_idx * BT_EM_SIZE;

        BT_DRV_TRACE(3,"BT_DRV_REG: fastack cs=0x%x,fast ack reg=0x%x,Seq filter by pass=0x%x",
                     BT_DRIVER_GET_U16_REG_VAL(fast_cs_addr), //fa rx bit10,tx bit9
                     BT_DRIVER_GET_U32_REG_VAL(BT_FASTACK_ADDR),//fast ack reg bit22
                     BT_DRIVER_GET_U32_REG_VAL(BT_SCO_TRIGGER_BYPASS_ADDR));//seq bypass reg bit 18
    }
    BT_DRV_REG_OP_CLK_DIB();
    BT_DRV_REG_OP_EXIT();

    bt_drv_reg_op_bt_info_checker();
}

void bt_drv_reg_op_clear_ibrt_snoop_status(void)
{
}

uint8_t bt_drv_reg_op_linkcntl_aclltaddr_getf(uint16_t conhdl)
{
    uint8_t nRet = 0;

    BT_DRV_REG_OP_ENTER();
    BT_DRV_REG_OP_CLK_ENB();
    uint8_t elt_idx = btdrv_conhdl_to_linkid(conhdl);
    if (btdrv_is_link_index_valid(elt_idx))
    {
        uint16_t localVal = BT_DRIVER_GET_U16_REG_VAL(EM_BT_LINKCNTL_ADDR + elt_idx * BT_EM_SIZE);
        nRet = ((localVal & ((uint16_t)0x00000700)) >> 8);
    }
    else
    {
        nRet = 0;
    }
    BT_DRV_REG_OP_CLK_DIB();
    BT_DRV_REG_OP_EXIT();
    return nRet;
}

uint8_t bt_drv_reg_op_sync_bt_param(uint8_t* param, uint16_t len)
{
    int sRet = 0;
    uint8_t status = 0xff;
    BT_DRV_REG_OP_ENTER();
    BT_DRV_REG_OP_CLK_ENB();

    //link_connect_bak
    uint32_t op_addr = 0;


    if(link_connect_bak_addr)
    {
        op_addr = link_connect_bak_addr;
    }

    if(op_addr!=0)
    {
        BT_DRV_TRACE(1,"BT_REG_OP:sync bt param len=%d",len);

        sRet = memcpy_s((uint8_t *)(uintptr_t)(op_addr), len, param, len);
        if (sRet)
        {
            BT_DRV_TRACE(1, "%s line:%d sRet:%d", __func__, __LINE__, sRet);
        }
        status = 0;
    }

    BT_DRV_REG_OP_CLK_DIB();
    BT_DRV_REG_OP_EXIT();
    return status;
}

uint32_t bt_drv_reg_op_get_lc_env_ptr(uint16_t conhdl, uint8_t type)
{
    uint32_t lc_ptr = 0;
    uint32_t op_ptr = 0;

    BT_DRV_REG_OP_ENTER();
    BT_DRV_REG_OP_CLK_ENB();

    if(lc_env_addr)
    {
        lc_ptr = BT_DRIVER_GET_U32_REG_VAL(lc_env_addr+(conhdl-0x80)*4);
    }

    BT_DRV_TRACE(3,"BT_REG_OP:lc_env_ptr =0x%x, conhdl=0x%x,type=%d",lc_ptr,conhdl,type);

    if(lc_ptr == 0)
    {
        goto exit;
    }

    switch(type)
    {
        case LC_LINK_TAG:
            //link tag len=108
            op_ptr = lc_ptr;
            break;
        case LC_INFO_TAG:
            op_ptr = lc_ptr +0x6c;
            break;
        case LC_ENC_TAG:
            //enc tag len=166
            op_ptr = lc_ptr +0x196;
            break;
        case LC_AFH_TAG:
            op_ptr = lc_ptr +0x26c;
            break;
        case LC_SP_TAG:
            //link tag len=108
            op_ptr = lc_ptr +0x28c;
            break;
        case LC_EPR_TAG:
            op_ptr = lc_ptr +0x2f8;
            break;
        case LC_EPC_TAG:
            op_ptr = lc_ptr +0x2fc;
            break;
        default:
            break;
    }

exit:
    BT_DRV_REG_OP_CLK_DIB();
    BT_DRV_REG_OP_EXIT();

    return op_ptr;
}

void bt_drv_reg_op_write_private_public_key(uint8_t* private_key,uint8_t* public_key)
{
    int sRet = 0;
    uint8_t* lm_env_ptr = 0;
    uint8_t* lm_private_key_ptr = 0;
    uint8_t* lm_public_key_ptr = 0;

    BT_DRV_REG_OP_ENTER();
    BT_DRV_REG_OP_CLK_ENB();
    //lm_env
    if(lm_env_addr)
    {
        lm_env_ptr = (uint8_t*)lm_env_addr;
    }
    else
    {
        goto exit;
    }
    //priv_key_192 off:6c
    lm_private_key_ptr = lm_env_ptr + 0x6c;
    //pub_key_192 off:0x84
    lm_public_key_ptr = lm_private_key_ptr + 0x18;
    sRet = memcpy_s(lm_private_key_ptr, 24, private_key, 24);
    if (sRet)
    {
        BT_DRV_TRACE(1, "%s line:%d sRet:%d", __func__, __LINE__, sRet);
    }
    sRet = memcpy_s(lm_public_key_ptr, 48, public_key, 48);
    if (sRet)
    {
        BT_DRV_TRACE(1, "%s line:%d sRet:%d", __func__, __LINE__, sRet);
    }
    BT_DRV_TRACE(0,"BT_REG_OP:private key");
    DUMP8("%02x",lm_private_key_ptr,24);
    BT_DRV_TRACE(0,"public key");
    DUMP8("%02x",lm_public_key_ptr,48);
exit:
    BT_DRV_REG_OP_CLK_DIB();
    BT_DRV_REG_OP_EXIT();
}

void bt_drv_reg_op_for_test_mode_disable(void)
{
    BT_DRV_REG_OP_ENTER();
    BT_DRV_REG_OP_CLK_ENB();
    if(dbg_bt_setting_addr)
    {
        BT_DRIVER_GET_U32_REG_VAL(dbg_bt_setting_addr+0x64) &= ~(1<<8);////sw_seq_filter_en set 0
        BT_DRIVER_GET_U32_REG_VAL(dbg_bt_setting_addr+0x64) &= ~(1<<0);////ecc_enable set 0
        BT_DRIVER_GET_U32_REG_VAL(dbg_bt_setting_addr+0x1c) &= 0x00ffffff;////dbg_trace_level set 0
    }
    BT_DRV_REG_OP_CLK_DIB();
    BT_DRV_REG_OP_EXIT();
}

uint16_t bt_drv_reg_op_get_ibrt_sco_hdl(uint16_t acl_hdl)
{
    // FIXME
    return acl_hdl|0x100;
}


void bt_drv_reg_op_get_ibrt_address(uint8_t *addr)
{
    int sRet = 0;
    struct ld_device_info * mobile_device_info=0;
    BT_DRV_REG_OP_ENTER();
    BT_DRV_REG_OP_CLK_ENB();

    if(ld_ibrt_env_addr)
    {
        mobile_device_info = (struct ld_device_info *)(ld_ibrt_env_addr+4);
    }

    if(mobile_device_info)
    {
        if (addr != NULL)
        {
            sRet = memcpy_s(addr, 6, (uint8_t *)(uintptr_t)mobile_device_info->bd_addr.addr, 6);
            if (sRet)
            {
                BT_DRV_TRACE(1, "%s line:%d sRet:%d", __func__, __LINE__, sRet);
            }
            BT_DRV_TRACE(0,"BT_REG_OP:get mobile address");
            DUMP8("%02x ",addr,6);
        }
    }

    BT_DRV_REG_OP_CLK_DIB();
    BT_DRV_REG_OP_EXIT();
}

void bt_drv_reg_op_set_tws_link_id(uint8_t link_id)
{
    BT_DRV_TRACE(1,"set tws link id =%x",link_id);
    ASSERT(link_id <3 || link_id == 0xff,"BT_REG_OP:error tws link id set");
    uint32_t tws_link_id_addr = 0;

    BT_DRV_REG_OP_ENTER();
    BT_DRV_REG_OP_CLK_ENB();
    if(ld_ibrt_env_addr != 0)
    {
        tws_link_id_addr = ld_ibrt_env_addr+ 0x74;
    }

    if(tws_link_id_addr != 0)
    {
        if(link_id == 0xff)
        {
            link_id = 3;
        }
        BT_DRIVER_GET_U8_REG_VAL(tws_link_id_addr) = link_id;
    }
    BT_DRV_REG_OP_CLK_DIB();
    BT_DRV_REG_OP_EXIT();
}

void bt_drv_reg_op_hack_max_slot(uint8_t link_id,uint8_t slot)
{
    uint32_t acl_evt_ptr = 0x0;
    uint32_t slot_addr;
    uint32_t stop_lantency_addr=0;


    BT_DRV_TRACE(2,"hack slot id=%d,slot=%d",link_id,slot);

    if(link_id>=3)
        return;

    BT_DRV_REG_OP_CLK_ENB();

    if(ld_acl_env_addr)
    {
        acl_evt_ptr = *(uint32_t *)(ld_acl_env_addr+link_id*4);
    }



    if(acl_evt_ptr)
    {
        slot_addr = acl_evt_ptr+0xc6;
        stop_lantency_addr = acl_evt_ptr+0x18;
        *(uint8_t *)slot_addr = slot;
        *(uint8_t *)stop_lantency_addr = slot+3+(*(uint16_t *)(acl_evt_ptr+150)>128?1:0);
    }
    BT_DRV_REG_OP_CLK_DIB();


}

void bt_drv_reg_op_fa_set(uint8_t en)
{
    BT_DRV_REG_OP_ENTER();
    BT_DRV_REG_OP_CLK_ENB();
    if(en)
    {
        BT_DRV_TRACE(1,"BT_REG_OP:%s", __func__);
        BT_DRIVER_GET_U32_REG_VAL(0xd0220468) |=0x00400000;
    }
    else
    {
        BT_DRIVER_GET_U32_REG_VAL(0xd0220468) &= ~0x00400000;
    }
    BT_DRV_REG_OP_CLK_DIB();
    BT_DRV_REG_OP_EXIT();
}


//////return true means controller had received a data packet
bool bt_drv_reg_op_check_seq_pending_status(void)
{
    bool nRet = true;
    BT_DRV_REG_OP_ENTER();
    BT_DRV_REG_OP_CLK_ENB();
    if(pending_seq_error_link_addr)
    {
        nRet = (BT_DRIVER_GET_U8_REG_VAL(pending_seq_error_link_addr) == 3);
    }
    else
    {
        nRet = true;
    }
    BT_DRV_REG_OP_CLK_DIB();
    BT_DRV_REG_OP_EXIT();
    return nRet;
}

///if the flag is not clear when disconnect happened  call this function
void bt_drv_reg_op_clear_seq_pending_status(void)
{
    BT_DRV_REG_OP_ENTER();
    BT_DRV_REG_OP_CLK_ENB();
    if(pending_seq_error_link_addr)
    {
        BT_DRIVER_GET_U8_REG_VAL(pending_seq_error_link_addr) = 3;
    }
    BT_DRV_REG_OP_CLK_DIB();
    BT_DRV_REG_OP_EXIT();
}

void bt_drv_reg_op_set_link_policy(uint8_t linkid, uint8_t policy)
{
    BT_DRV_REG_OP_ENTER();
    BT_DRV_REG_OP_CLK_ENB();

    uint32_t lc_evt_ptr = 0x0;
    uint32_t policy_addr;
    if(linkid>=0x3)
    {
        goto exit;
    }

    BT_DRV_TRACE(2,"BT_REG_OP: set link=%d, policy=%d",linkid,policy);

    if(lc_env_addr)
    {
        lc_evt_ptr = BT_DRIVER_GET_U32_REG_VAL(lc_env_addr+linkid*4);//lc_env
    }

    if(lc_evt_ptr)
    {
        //LinkPlicySettings off:2c
        policy_addr = lc_evt_ptr+0x2c;
        BT_DRIVER_GET_U8_REG_VAL(policy_addr) = policy;
    }
exit:
    BT_DRV_REG_OP_CLK_DIB();
    BT_DRV_REG_OP_EXIT();
}


void bt_drv_reg_op_voice_settings_set(uint16_t voice_settings)
{
    uint32_t voice_settings_addr = 0;

    BT_DRV_REG_OP_ENTER();
    BT_DRV_REG_OP_CLK_ENB();
    if(hci_env_addr)
    {
        //voice_setting off:5a
        voice_settings_addr = hci_env_addr+0x5a;
    }

    if(voice_settings_addr != 0)
    {
        BT_DRV_TRACE(2,"BT_REG_OP:Fast_bt_init1:%s %d",__func__,voice_settings);
        BT_DRIVER_GET_U16_REG_VAL(voice_settings_addr) = voice_settings;
    }
    BT_DRV_REG_OP_CLK_DIB();
    BT_DRV_REG_OP_EXIT();
}

uint8_t bt_drv_reg_op_lmp_sco_hdl_get(uint16_t sco_handle)
{
    return 0;
}

bool bt_drv_reg_op_lmp_sco_hdl_set(uint16_t sco_handle, uint8_t lmp_hdl)
{
    return false;
}

void bt_drv_reg_op_host_buff_and_flow_ctl_set
(uint16_t acl_pkt_len, uint16_t acl_pkt_num,uint16_t sync_pkt_len, uint16_t sync_pkt_num,bool flow_ctl_en)
{
    uint32_t acl_pkt_len_addr = 0;
    uint32_t acl_pkt_num_addr = 0;
    uint32_t sync_pkt_len_addr = 0;
    uint32_t sync_pkt_num_addr = 0;
    uint32_t flow_ctl_addr = 0;

    BT_DRV_REG_OP_ENTER();
    BT_DRV_REG_OP_CLK_ENB();

    if(hci_fc_env_addr)
    {
        flow_ctl_addr = hci_fc_env_addr;
    }

    if(flow_ctl_addr != 0)
    {
        //hci_fc_env.host_set.acl_pkt_len
        acl_pkt_len_addr = flow_ctl_addr+2;
        //hci_fc_env.host_set.acl_pkt_num
        acl_pkt_num_addr = flow_ctl_addr+4;
        //hci_fc_env.host_set.sync_pkt_len;
        sync_pkt_len_addr = flow_ctl_addr+9;
        //hci_fc_env.host_set.sync_pkt_nb;
        sync_pkt_num_addr = flow_ctl_addr+10;

        BT_DRV_TRACE(6,"Fast_bt_init2:%s,acl len=%x,acl num=%x,sync len=%x sync num=%x,fl_ctl=%d",
                     __func__,acl_pkt_len,acl_pkt_num,sync_pkt_len,sync_pkt_num,flow_ctl_en);

        BT_DRIVER_GET_U8_REG_VAL(flow_ctl_addr) = flow_ctl_en;
        BT_DRIVER_GET_U16_REG_VAL(acl_pkt_len_addr) = acl_pkt_len;
        BT_DRIVER_GET_U16_REG_VAL(acl_pkt_num_addr) = acl_pkt_num;
        BT_DRIVER_GET_U8_REG_VAL(sync_pkt_len_addr) = sync_pkt_len;
        BT_DRIVER_GET_U16_REG_VAL(sync_pkt_num_addr) = sync_pkt_num;
    }

    BT_DRV_REG_OP_CLK_DIB();
    BT_DRV_REG_OP_EXIT();
}

void bt_drv_reg_op_page_to_set(uint16_t page_to)
{
    BT_DRV_REG_OP_ENTER();
    BT_DRV_REG_OP_CLK_ENB();

    uint32_t page_to_addr = 0;

    if(lm_env_addr)
    {
        //page_to off:14
        page_to_addr = lm_env_addr + 0x14;
    }

    if(page_to_addr != 0)
    {
        BT_DRV_TRACE(2,"BT_REG_OP:Fast_bt_init3:%s,to=%x",__func__,page_to);
        BT_DRIVER_GET_U16_REG_VAL(page_to_addr) = page_to;
    }

    BT_DRV_REG_OP_CLK_DIB();
    BT_DRV_REG_OP_EXIT();
}

void bt_drv_reg_op_btdm_address_set(uint8_t* bt_addr, uint8_t* ble_addr)
{
    BT_DRV_REG_OP_ENTER();
    BT_DRV_REG_OP_CLK_ENB();

    uint32_t btaddr_addr = 0;
    uint32_t bleaddr_addr = 0;
    uint32_t bch_addr = 0;

    if(llm_le_env_addr)
    {
        //public off:8f
        bleaddr_addr = llm_le_env_addr+0x8f;
    }

    if(ld_env_addr)
    {
        btaddr_addr = ld_env_addr;
        //      bch_addr = btaddr_addr+451;
        //local bch:4db
        bch_addr = ld_env_addr  + 0x4db;
    }

    if(bleaddr_addr != 0)
    {
        BT_DRV_TRACE(1,"BT_REG_OP:Fast_bt_init4:%s",__func__);
        DUMP8("%02x ",bt_addr,6);
        DUMP8("%02x ",ble_addr,6);

        for(uint8_t i=0; i<6; i++)
        {
            BT_DRIVER_GET_U8_REG_VAL(bleaddr_addr+i) = ble_addr[i];
        }

        for(uint8_t i=0; i<6; i++)
        {
            BT_DRIVER_GET_U8_REG_VAL(btaddr_addr+i) = bt_addr[i];
        }
        ld_util_bch_create(bt_addr,(uint8_t *)(uintptr_t)bch_addr);
    }

    BT_DRV_REG_OP_CLK_DIB();
    BT_DRV_REG_OP_EXIT();
}


void bt_drv_reg_op_evtmask_set(uint8_t ble_en)
{
    int sRet = 0;
    BT_DRV_REG_OP_ENTER();
    BT_DRV_REG_OP_CLK_ENB();

    uint32_t bt_evtmsk_addr = 0;
    uint32_t ble_evtmask_addr = 0;

    if(llm_le_env_addr)
    {
        //eventmast off:64
        ble_evtmask_addr = llm_le_env_addr+0x64;
    }

    if(hci_env_addr)
    {
        bt_evtmsk_addr = hci_env_addr;
    }

    if(bt_evtmsk_addr != 0)
    {
        BT_DRV_TRACE(2,"BT_REG_OP:Fast_bt_init5:%s,ble enable=%x",__func__,ble_en);
        uint8_t bt_mask[8]= {0xff,0xff,0xff,0xff,0xff,0xff,0xff,0x1f};
        if(ble_en)
        {
            bt_mask[7] = 0x3f;
        }
        sRet = memcpy_s((uint8_t *)(uintptr_t)bt_evtmsk_addr,8,bt_mask,8);
        if (sRet)
        {
            BT_DRV_TRACE(1, "%s line:%d sRet:%d", __func__, __LINE__, sRet);
        }

        uint8_t ble_mask[8]= {0x7f,0x00,0x00,0x00,0x00,0x00,0x00,0x00};
        sRet = memcpy_s((uint8_t *)(uintptr_t)ble_evtmask_addr,8,ble_mask,8);
        if (sRet)
        {
            BT_DRV_TRACE(1, "%s line:%d sRet:%d", __func__, __LINE__, sRet);
        }
    }

    BT_DRV_REG_OP_CLK_DIB();
    BT_DRV_REG_OP_EXIT();
}

void bt_drv_reg_op_simplepair_mode_set(uint8_t en)
{
    BT_DRV_REG_OP_ENTER();
    BT_DRV_REG_OP_CLK_ENB();

    if(lm_env_addr)
    {
        //sp_mode off:2c
        BT_DRIVER_GET_U8_REG_VAL(lm_env_addr+0x2c) = en;
    }

    BT_DRV_REG_OP_CLK_DIB();
    BT_DRV_REG_OP_EXIT();
}

void bt_drv_reg_op_class_of_dev_set(uint8_t* class_of_dev,uint8_t len)
{
    int sRet = 0;
    BT_DRV_REG_OP_ENTER();
    BT_DRV_REG_OP_CLK_ENB();

    uint32_t class_of_dev_addr = 0;

    if(ld_env_addr)
    {
        //ld_env.class_of_dev.A
        class_of_dev_addr = ld_env_addr+6;
    }

    if(class_of_dev_addr != 0)
    {
        BT_DRV_TRACE(1,"BT_REG_OP:Fast_bt_init7:%s",__func__);
        DUMP8("%02x ",class_of_dev,len);
        sRet = memcpy_s((uint8_t *)(uintptr_t)class_of_dev_addr, len, class_of_dev, len);
        if (sRet)
        {
            BT_DRV_TRACE(1, "%s line:%d sRet:%d", __func__, __LINE__, sRet);
        }
    }

    BT_DRV_REG_OP_CLK_DIB();
    BT_DRV_REG_OP_EXIT();
}


void bt_drv_reg_op_sleep_set(bool en)
{
    BT_DRV_REG_OP_ENTER();
    BT_DRV_REG_OP_CLK_ENB();

    uint32_t sleep_addr = 0;

    if(rwip_env_addr)
    {
        //ext_wakeup_enable off:d
        sleep_addr = rwip_env_addr+0xc; //update by zhangjisen 2020.8.26
    }

    if(sleep_addr != 0)
    {
        BT_DRV_TRACE(2,"BT_REG_OP:Fast_bt_init:%s en=%d",__func__,en);
        BT_DRIVER_GET_U8_REG_VAL(sleep_addr) = en;
    }

    BT_DRV_REG_OP_CLK_DIB();
    BT_DRV_REG_OP_EXIT();
}

void bt_drv_reg_op_sco_fifo_reset(void)
{
    int sRet = 0;
    uint16_t sco_ptr0 = 0, sco_ptr1 = 0;
    //#define REG_EM_ET_BASE_ADDR 0xD0210000
    uint8_t *em_area_base = (uint8_t*)0xd0210000;

    BT_DRV_REG_OP_ENTER();
    BT_DRV_REG_OP_CLK_ENB();

    BTDIGITAL_REG_GET_FIELD(0xd02201cc, 0xffff, 0, sco_ptr0);
    BTDIGITAL_REG_GET_FIELD(0xd02201cc, 0xffff, 16, sco_ptr1);
    BT_DRV_TRACE(3,"BT_REG_OP:bt_drv_reg_op_sco_fifo_reset %d/%d %08x",sco_ptr0, sco_ptr1, BT_DRIVER_GET_U32_REG_VAL(0xd02201cc));
    sRet = memset_s(em_area_base+sco_ptr0, 60, 0x55, 60);
    if (sRet)
    {
        BT_DRV_TRACE(1, "%s line:%d sRet:%d", __func__, __LINE__, sRet);
    }
    sRet = memset_s(em_area_base+sco_ptr1, 60, 0x55, 60);
    if (sRet)
    {
        BT_DRV_TRACE(1, "%s line:%d sRet:%d", __func__, __LINE__, sRet);
    }

    BT_DRV_REG_OP_CLK_DIB();
    BT_DRV_REG_OP_EXIT();
}

void bt_drv_reg_sco_fifo_dump(void)
{
    uint16_t sco_ptr0 = 0, sco_ptr1 = 0;
    //#define REG_EM_ET_BASE_ADDR 0xD0210000
    uint8_t *em_area_base = (uint8_t*)0xd0210000;
    BT_DRV_REG_OP_ENTER();
    BT_DRV_REG_OP_CLK_ENB();

    BTDIGITAL_REG_GET_FIELD(0xd02201cc, 0xffff, 0, sco_ptr0);
    BTDIGITAL_REG_GET_FIELD(0xd02201cc, 0xffff, 16, sco_ptr1);
    BT_DRV_TRACE(3,"BT_REG_OP:bt_drv_reg_op_sco_fifo_reset %d/%d %08x",sco_ptr0, sco_ptr1, BT_DRIVER_GET_U32_REG_VAL(0xd02201cc));
    DUMP8("%02x ", em_area_base+sco_ptr0, 10);
    DUMP8("%02x ", em_area_base+sco_ptr1, 10);

    BT_DRV_REG_OP_CLK_DIB();
    BT_DRV_REG_OP_EXIT();
}

uint8_t bt_drv_reg_op_get_sync_id_op(uint8_t op, uint8_t sync_id)
{
    return 0xff;
}

void bt_drv_reg_op_ble_sup_timeout_set(uint16_t ble_conhdl, uint16_t sup_to)
{
    uint32_t llc_env_ptr = 0;

    BT_DRV_REG_OP_ENTER();
    BT_DRV_REG_OP_CLK_ENB();

    if(llc_env_addr)
    {
        uint32_t llc_env_address = llc_env_addr+ble_conhdl*4;
        llc_env_ptr = BT_DRIVER_GET_U32_REG_VAL(llc_env_address);
    }

    if(llc_env_ptr != 0)
    {
        //sup_to off:54
        uint32_t llc_env_sup_to_address = llc_env_ptr + 0x54;
        BT_DRIVER_GET_U16_REG_VAL(llc_env_sup_to_address) = sup_to;
        BT_DRV_TRACE(1,"BT_REG_OP:set ble sup_timeout to %d",sup_to);
    }

    BT_DRV_REG_OP_CLK_DIB();
    BT_DRV_REG_OP_EXIT();
}

void bt_drv_reg_op_disable_swagc_nosync_count(void)
{
    BT_DRV_REG_OP_ENTER();
    BT_DRV_REG_OP_CLK_ENB();
    if(dbg_bt_setting_ext1_addr)
    {
        BT_DRIVER_GET_U16_REG_VAL(dbg_bt_setting_ext1_addr+0x14) = 0xa604;
    }
    BT_DRV_REG_OP_CLK_DIB();
    BT_DRV_REG_OP_EXIT();
}

void bt_drv_reg_op_hw_sw_agc_select(uint8_t agc_mode)
{
    uint32_t hw_agc_select_addr = 0;
    BT_DRV_REG_OP_ENTER();
    BT_DRV_REG_OP_CLK_ENB();

    if(rwip_rf_addr)
    {
        //hw_or_sw_agc off:37
        hw_agc_select_addr = rwip_rf_addr+0x37;
    }
    if(hw_agc_select_addr != 0)
    {
        BT_DRIVER_PUT_U8_REG_VAL(hw_agc_select_addr, agc_mode);
    }

    BT_DRV_REG_OP_CLK_DIB();
    BT_DRV_REG_OP_EXIT();
}

void bt_drv_reg_op_afh_follow_en(bool enable, uint8_t be_followed_link_id, uint8_t follow_link_id)
{
#if BT_AFH_FOLLOW
    BT_DRV_REG_OP_ENTER();
    BT_DRV_REG_OP_CLK_ENB();

    BT_DRIVER_GET_U8_REG_VAL(lm_env_addr+0x168) = be_followed_link_id;//be follow follow link id //ok
    BT_DRIVER_GET_U8_REG_VAL(lm_env_addr+0x169) = follow_link_id;//follow link id
    BT_DRIVER_GET_U8_REG_VAL(lm_env_addr+0x167) = enable; //afh follow enable

    BT_DRV_REG_OP_CLK_DIB();
    BT_DRV_REG_OP_EXIT();
#endif
}

void bt_drv_reg_op_force_set_sniff_att(uint16_t conhdle)
{
    BT_DRV_REG_OP_ENTER();
    BT_DRV_REG_OP_CLK_ENB();

    uint8_t linkid = btdrv_conhdl_to_linkid(conhdle);
    uint32_t acl_evt_ptr = 0x0;

    if (ld_acl_env_addr != 0&& btdrv_is_link_index_valid(linkid))
    {
        acl_evt_ptr = BT_DRIVER_GET_U32_REG_VAL(ld_acl_env_addr+linkid*4);//ld_acl_env

        if (acl_evt_ptr != 0)
        {
            uint32_t sniff_att_addr = acl_evt_ptr + 0xe6;
            if(BT_DRIVER_GET_U16_REG_VAL(sniff_att_addr) <3)
            {
                BT_DRIVER_PUT_U16_REG_VAL(sniff_att_addr, 3);
            }
            //set sniff timeout to 0
            BT_DRIVER_PUT_U16_REG_VAL(sniff_att_addr+4, 0);
            BT_DRV_TRACE(0,"BT_REG_OP:force set sniff att");
        }
    }
    BT_DRV_REG_OP_CLK_DIB();
    BT_DRV_REG_OP_EXIT();
}

void bt_drv_reg_op_fa_gain_direct_set(uint8_t gain_idx)
{
}

uint8_t bt_drv_reg_op_fa_gain_direct_get(void)
{
    return 0;
}

struct rx_gain_fixed_t
{
    uint8_t     enable;//enable or disable
    uint8_t     bt_or_ble;//0:bt 1:ble
    uint8_t     cs_id;//link id
    uint8_t     gain_idx;//gain index
};

void bt_drv_reg_op_dgb_link_gain_ctrl_set(uint16_t connHandle, uint8_t bt_ble_mode, uint8_t gain_idx, uint8_t enable)
{
    BT_DRV_REG_OP_ENTER();
    BT_DRV_REG_OP_CLK_ENB();
    if(rf_rx_gain_fixed_tbl_addr != 0)
    {
        //struct rx_gain_fixed_t RF_RX_GAIN_FIXED_TBL[HOST_GAIN_TBL_MAX];
        struct rx_gain_fixed_t *rx_gain_fixed_p = (struct rx_gain_fixed_t *)rf_rx_gain_fixed_tbl_addr;
        uint8_t cs_id = btdrv_conhdl_to_linkid(connHandle);

        if (btdrv_is_link_index_valid(cs_id))
        {
            for (uint8_t i=0; i<3; i++)
            {
                if ((rx_gain_fixed_p->cs_id == cs_id) &&
                    (rx_gain_fixed_p->bt_or_ble == bt_ble_mode))
                {
                    rx_gain_fixed_p->enable    = enable;
                    rx_gain_fixed_p->bt_or_ble = bt_ble_mode;
                    rx_gain_fixed_p->gain_idx = gain_idx;
                    BT_DRV_TRACE(5,"BT_REG_OP:%s hdl:%x/%x mode:%d idx:%d", __func__, connHandle, cs_id, rx_gain_fixed_p->bt_or_ble, gain_idx);
                }
                rx_gain_fixed_p++;
            }
        }
    }
    BT_DRV_REG_OP_CLK_DIB();
    BT_DRV_REG_OP_EXIT();
}


void bt_drv_reg_op_dgb_link_gain_ctrl_clear(uint16_t connHandle, uint8_t bt_ble_mode)
{
    BT_DRV_REG_OP_ENTER();
    BT_DRV_REG_OP_CLK_ENB();
    if(rf_rx_gain_fixed_tbl_addr != 0)
    {
        //struct rx_gain_fixed_t RF_RX_GAIN_FIXED_TBL[HOST_GAIN_TBL_MAX];
        struct rx_gain_fixed_t *rx_gain_fixed_p = (struct rx_gain_fixed_t *)rf_rx_gain_fixed_tbl_addr;
        uint8_t cs_id = btdrv_conhdl_to_linkid(connHandle);

        if (btdrv_is_link_index_valid(cs_id))
        {
            for (uint8_t i=0; i<3; i++)
            {
                if ((rx_gain_fixed_p->cs_id == cs_id) &&
                    (rx_gain_fixed_p->bt_or_ble == bt_ble_mode))
                {
                    rx_gain_fixed_p->enable    = 0;
                    rx_gain_fixed_p->bt_or_ble = bt_ble_mode;
                    rx_gain_fixed_p->gain_idx = 0;
                }
                rx_gain_fixed_p++;
            }
        }
    }
    BT_DRV_REG_OP_CLK_DIB();
    BT_DRV_REG_OP_EXIT();
}


void bt_drv_reg_op_dgb_link_gain_ctrl_init(void)
{
    BT_DRV_REG_OP_ENTER();
    BT_DRV_REG_OP_CLK_ENB();
    if(rf_rx_gain_fixed_tbl_addr != 0)
    {
        //struct rx_gain_fixed_t RF_RX_GAIN_FIXED_TBL[HOST_GAIN_TBL_MAX];
        struct rx_gain_fixed_t *rx_gain_fixed_p = (struct rx_gain_fixed_t *)rf_rx_gain_fixed_tbl_addr;

        for (uint8_t i=0; i<3; i++)
        {
            rx_gain_fixed_p->cs_id = i;
            rx_gain_fixed_p++;
        }
    }
    BT_DRV_REG_OP_CLK_DIB();
    BT_DRV_REG_OP_EXIT();
}

void bt_drv_reg_op_rx_gain_fix(uint16_t connHandle, uint8_t bt_ble_mode, uint8_t gain_idx, uint8_t enable, uint8_t table_idx)
{
    BT_DRV_REG_OP_ENTER();
    BT_DRV_REG_OP_CLK_ENB();
    if(rf_rx_gain_fixed_tbl_addr != 0)
    {
        //struct rx_gain_fixed_t RF_RX_GAIN_FIXED_TBL[HOST_GAIN_TBL_MAX];
        struct rx_gain_fixed_t *rx_gain_fixed_p = (struct rx_gain_fixed_t *)rf_rx_gain_fixed_tbl_addr;
        uint8_t cs_id;

        if(bt_ble_mode == 0)//bt
        {
            cs_id = btdrv_conhdl_to_linkid(connHandle);
        }
        else if(bt_ble_mode == 1)//ble
        {
            cs_id = connHandle;
        }
        else
        {
            BT_DRV_TRACE(1,"BT_REG_OP:%s:fix gain fail",__func__);
            goto exit;
        }

        if(table_idx < 3)
        {
            rx_gain_fixed_p[table_idx].enable = enable;
            rx_gain_fixed_p[table_idx].bt_or_ble = bt_ble_mode;
            rx_gain_fixed_p[table_idx].cs_id = cs_id;
            rx_gain_fixed_p[table_idx].gain_idx = gain_idx;
        }
    }
exit:
    BT_DRV_REG_OP_CLK_DIB();
    BT_DRV_REG_OP_EXIT();
}

void bt_drv_reg_op_set_ibrt_auto_accept_sco(bool en)
{
}

void bt_drv_reg_op_update_dbg_state(void)
{
}

void bt_drv_reg_op_set_ibrt_reject_sniff_req(bool en)
{
    BT_DRV_REG_OP_ENTER();
    BT_DRV_REG_OP_CLK_ENB();
    uint8_t *accept_remote_enter_sniff_ptr = NULL;

    if (hci_dbg_bt_setting_ext2_addr == 0)
    {
        BT_DRV_TRACE(1, "BT_REG_OP:set reject sniff req %d, please fix it", en);
        goto exit;
    }

    accept_remote_enter_sniff_ptr = (uint8_t *)(uintptr_t)(hci_dbg_bt_setting_ext2_addr + 0x49);

    *accept_remote_enter_sniff_ptr = en ? 0 : 1;
exit:
    BT_DRV_REG_OP_CLK_DIB();
    BT_DRV_REG_OP_EXIT();
}

void bt_drv_reg_op_set_ibrt_second_sco_decision(uint8_t value)
{
}

uint8_t bt_drv_reg_op_get_esco_nego_airmode(uint8_t sco_link_id)
{
    uint8_t airmode = 0;
    BT_DRV_REG_OP_ENTER();
    BT_DRV_REG_OP_CLK_ENB();

    if(lc_sco_env_addr != 0)
    {
        uint32_t p_nego_params_addr = BT_DRIVER_GET_U32_REG_VAL(lc_sco_env_addr + 4 + sco_link_id * 8);

        if (sco_link_id > 2 || p_nego_params_addr == 0)
        {
            airmode = 0xff;
            goto exit;
        }
        //air_mode off:5e
        airmode = BT_DRIVER_GET_U8_REG_VAL(p_nego_params_addr + 0x5e);

        BT_DRV_TRACE(1,"BT_REG_OP:Get nego esco airmode=%d",airmode);
    }

exit:
    BT_DRV_REG_OP_CLK_DIB();
    BT_DRV_REG_OP_EXIT();
    return airmode;
}

#if defined(PCM_FAST_MODE) && defined(PCM_PRIVATE_DATA_FLAG)
void bt_drv_reg_op_set_pcm_flag()
{
    BT_DRV_REG_OP_ENTER();
    BT_DRV_REG_OP_CLK_ENB();
    BTDIGITAL_REG_SET_FIELD(0xd022046c, 1, 3, 1);
    BT_DRV_REG_OP_CLK_DIB();
    BT_DRV_REG_OP_EXIT();
}
#endif


void bt_drv_reg_op_ebq_test_setting(void)
{
    BT_DRV_REG_OP_ENTER();
    BT_DRV_REG_OP_CLK_ENB();
    struct dbg_set_ebq_test* set_ebq_test = (struct dbg_set_ebq_test*)ebq_test_setting_addr;
    if(set_ebq_test)
    {
        BT_DRV_TRACE(0,"BT_REG_OP:ebq_test_setting");
        set_ebq_test->ebq_testmode = 0x01;
        set_ebq_test->ble_set_privacy_mode = 0x01;
        set_ebq_test->ll_con_ini_bv2324 = 0x00;
        set_ebq_test->ll_con_ini_bv2021 = 0x00;
        set_ebq_test->ll_ddi_scn_bv26 = 0x00;
        set_ebq_test->ll_sec_adv_bv05 = 0x00;
        set_ebq_test->ll_sync_win_default = 0x20;
        set_ebq_test->auth_end_notify_host_lk = 0x01;
        set_ebq_test->ll_sec_adv_bv09 = 0x00;
        set_ebq_test->ll_sec_adv_bv17 = 0x00;
        set_ebq_test->ll_sec_adv_bv1819 = 0x00;
        set_ebq_test->LocEncKeyRefresh = 0x01;
        set_ebq_test->publickey_check = 0x00;
        set_ebq_test->Qos = 0x01;
        set_ebq_test->hv1 = 0x01;
        set_ebq_test->ssr = 0x01;
        set_ebq_test->aes_ccm_daycounter = 0x01;
        set_ebq_test->bb_prot_flh_bv01 = 0x01;
        set_ebq_test->bb_prot_arq_bv43 = 0x01;
        set_ebq_test->pause_aes_generate_rand = 0x01;
    }
    BT_DRV_REG_OP_CLK_DIB();
    BT_DRV_REG_OP_EXIT();
}

void bt_drv_reg_op_hw_spi_en_setf(int elt_idx, uint8_t hwspien)
{
    BT_DRV_REG_OP_ENTER();
    BT_DRV_REG_OP_CLK_ENB();

    BTDIGITAL_EM_BT_WR(EM_BT_BT_EXT2_ADDR + elt_idx * BT_EM_SIZE,
                       (BT_DRIVER_GET_U16_REG_VAL(EM_BT_BT_EXT2_ADDR + elt_idx * BT_EM_SIZE) & ~((uint16_t)0x00000008)) | ((uint16_t)hwspien << 3));

    BT_DRV_REG_OP_CLK_DIB();
    BT_DRV_REG_OP_EXIT();
}

void bt_drv_reg_op_set_rand_seed(uint32_t seed)
{
}

uint8_t bt_drv_reg_op_get_controller_ble_tx_free_buffer(void)
{
    uint32_t *tx_buf_ptr=NULL;
    uint8_t tx_free_buf_count=0;
    BT_DRV_REG_OP_ENTER();
    BT_DRV_REG_OP_CLK_ENB();

    if(em_buf_env_addr)
    {
        tx_buf_ptr = (uint32_t *)(uintptr_t)(em_buf_env_addr+0x14); //em_buf_env.tx_buff_free
    }
    else
    {
        BT_DRV_TRACE(1, "REG_OP: please fix %s", __func__);
        return 0;
    }

    //check tx buff
    while(tx_buf_ptr && *tx_buf_ptr)
    {
        tx_free_buf_count++;
        tx_buf_ptr = (uint32_t *)(uintptr_t)(*tx_buf_ptr);
    }
    BT_DRV_REG_OP_CLK_DIB();
    BT_DRV_REG_OP_EXIT();

    return tx_free_buf_count;
}


void bt_drv_reg_op_bt_sync_swagc_en_set(uint8_t en)
{
    BT_DRV_REG_OP_ENTER();
    BT_DRV_REG_OP_CLK_ENB();

    uint8_t *bt_sync_swagc_en = NULL;

    if (hci_dbg_bt_setting_ext2_addr != 0)
    {
        bt_sync_swagc_en = (uint8_t *)(uintptr_t)(hci_dbg_bt_setting_ext2_addr + 0x2c);

        *bt_sync_swagc_en = en;
    }

    BT_DRV_REG_OP_CLK_DIB();
    BT_DRV_REG_OP_EXIT();
}

uint8_t bt_drv_reg_op_bt_sync_swagc_en_get(void)
{
    uint8_t bt_sync_swagc_en = 0;

    BT_DRV_REG_OP_ENTER();
    BT_DRV_REG_OP_CLK_ENB();

    if (hci_dbg_bt_setting_ext2_addr != 0)
    {
        bt_sync_swagc_en = BT_DRIVER_GET_U8_REG_VAL(hci_dbg_bt_setting_ext2_addr + 0x2c);
    }
    BT_DRV_REG_OP_CLK_DIB();
    BT_DRV_REG_OP_EXIT();

    return bt_sync_swagc_en;
}

void bt_drv_reg_op_swagc_mode_set(uint8_t mode)
{
#ifdef __NEW_SWAGC_MODE__
    BT_DRV_REG_OP_ENTER();
    BT_DRV_REG_OP_CLK_ENB();

    uint32_t lock = int_lock_global();
    if(mode == NEW_SYNC_SWAGC_MODE)
    {
        //open second rf spi:
        BTDIGITAL_REG_SET_FIELD(0xd02201e8, 0x1, 0, 1);
        //open rf new sync agc mode
        bt_drv_rf_set_bt_sync_agc_enable(true);
        //open BT sync AGC process cbk
        bt_drv_reg_op_bt_sync_swagc_en_set(1);
    }
    else if(mode == OLD_SWAGC_MODE)
    {
        //close second rf spi
        BTDIGITAL_REG_SET_FIELD(0xd02201e8, 0x1,  0, 0);
        //close rf new sync agc mode
        bt_drv_rf_set_bt_sync_agc_enable(false);
        //close BT sync AGC process cbk
        bt_drv_reg_op_bt_sync_swagc_en_set(0);

        //[19:8]: rrcgain
        //[7]: rrc_engain
        BTDIGITAL_REG_SET_FIELD(0xd03502c0, 0x1,  7, 1);
        BTDIGITAL_REG_SET_FIELD(0xd03502c0, 0xfff,  8, 0x00);
    }
    int_unlock_global(lock);
    BT_DRV_REG_OP_CLK_DIB();
    BT_DRV_REG_OP_EXIT();
#endif
}

void bt_drv_reg_op_key_gen_after_reset(bool enable)
{
    BT_DRV_REG_OP_ENTER();
    BT_DRV_REG_OP_CLK_ENB();
    if(dbg_bt_setting_ext2_addr)
    {
        BT_DRIVER_PUT_U8_REG_VAL((dbg_bt_setting_ext2_addr+0x43),enable);
    }
    BT_DRV_REG_OP_CLK_DIB();
    BT_DRV_REG_OP_EXIT();
}

void bt_drv_reg_op_set_fa_rx_gain_idx(uint8_t rx_gain_idx)
{
    BT_DRV_REG_OP_ENTER();
    BT_DRV_REG_OP_CLK_ENB();
    uint8_t * fa_gain_idx = NULL;

    if (hci_dbg_bt_setting_ext2_addr != 0)
    {
        fa_gain_idx = (uint8_t *)(uintptr_t)(hci_dbg_bt_setting_ext2_addr + 0x22);

        *fa_gain_idx = rx_gain_idx;
    }

    BT_DRV_REG_OP_CLK_DIB();
    BT_DRV_REG_OP_EXIT();
}

void bt_drv_reg_op_set_tpc_rssi_threshold(int8_t rssi_high_thr,int8_t rssi_low_thr)
{
    int8_t* rssi_high_thr_ptr = 0;
    int8_t* rssi_low_thr_ptr = 0;
    BT_DRV_REG_OP_ENTER();
    BT_DRV_REG_OP_CLK_ENB();

    if(rwip_rf_addr)
    {
        rssi_high_thr_ptr = (int8_t*)(rwip_rf_addr+0x31);
        rssi_low_thr_ptr = (int8_t*)(rwip_rf_addr+0x32);
    }
    if(rssi_high_thr_ptr != 0)
    {
        *rssi_high_thr_ptr = rssi_high_thr;
        *rssi_low_thr_ptr = rssi_low_thr;
    }

    BT_DRV_REG_OP_CLK_DIB();
    BT_DRV_REG_OP_EXIT();
}

void bt_drv_reg_op_set_bt_rx_gain_threshold_table(uint8_t index,int8_t rssi)
{
    int8_t* rx_gain_threshold_table_ptr = 0;
    BT_DRV_REG_OP_ENTER();
    BT_DRV_REG_OP_CLK_ENB();

    if(rf_rx_gain_ths_tbl_bt_addr)
    {
        rx_gain_threshold_table_ptr = (int8_t*)rf_rx_gain_ths_tbl_bt_addr;
    }
    if(rx_gain_threshold_table_ptr != 0)
    {
        *(rx_gain_threshold_table_ptr+index*2) = rssi;
        *(rx_gain_threshold_table_ptr+index*2+1) = rssi;
    }

    BT_DRV_REG_OP_CLK_DIB();
    BT_DRV_REG_OP_EXIT();
}

void bt_drv_reg_op_set_le_rx_gain_threshold_table(uint8_t index,int8_t rssi)
{
    int8_t* rx_gain_threshold_table_ptr = 0;
    BT_DRV_REG_OP_ENTER();
    BT_DRV_REG_OP_CLK_ENB();

    if(rf_rx_gain_ths_tbl_le_addr)
    {
        rx_gain_threshold_table_ptr = (int8_t*)rf_rx_gain_ths_tbl_le_addr;
    }
    if(rx_gain_threshold_table_ptr != 0)
    {
        *(rx_gain_threshold_table_ptr+index*2) = rssi;
        *(rx_gain_threshold_table_ptr+index*2+1) = rssi;
    }

    BT_DRV_REG_OP_CLK_DIB();
    BT_DRV_REG_OP_EXIT();
}

#ifdef PCM_PRIVATE_DATA_FLAG
void bt_drv_reg_op_sco_pri_data_init()
{
    BT_DRV_REG_OP_ENTER();
    BT_DRV_REG_OP_CLK_ENB();

    BTDIGITAL_REG_WR(0xd0220478,BTDIGITAL_REG(0xd0220478)|=0xff000000);
    BTDIGITAL_REG_WR(0xd02204f0,BTDIGITAL_REG(0xd02204f0)|=0x00ffffff);

    BT_DRV_REG_OP_CLK_DIB();
    BT_DRV_REG_OP_EXIT();
}
#endif

///only used for testmode
void bt_drv_reg_op_init_swagc_3m_thd(void)
{
    int8_t rxgain_3m_thd_tbl[15][2] =
    {
        {-79, -79},
        {-75, -75},
        {-72, -72},
        {-69, -69},
        {-64, -64},
        {-59, -59},
        {-53, -53},
        {127, 127},
        {127,  127},
        {0x7f ,0x7f},
        {0x7f ,0x7f},
        {0x7f ,0x7f},
        {0x7f ,0x7f},
        {0x7f ,0x7f},
        {0x7f ,0x7f},
    };

    BT_DRV_REG_OP_ENTER();
    BT_DRV_REG_OP_CLK_ENB();

    int32_t gain_thd_addr =TESTMODE_RF_RX_GAIN_THS_TBL_BT_3M_ADDR;
    memcpy((int8_t *)gain_thd_addr,rxgain_3m_thd_tbl,sizeof(rxgain_3m_thd_tbl));

    BT_DRV_REG_OP_CLK_DIB();
    BT_DRV_REG_OP_EXIT();
}

///only used for testmode
bool bt_drv_reg_op_get_3m_flag(void)
{
    uint32_t flag_addr = TESTMODE_TESTMODE_3M_FLAG_ADDR;
    bool ret=false;
    BT_DRV_REG_OP_ENTER();
    BT_DRV_REG_OP_CLK_ENB();

    if(flag_addr)
    {
        ret = *(uint8_t *)flag_addr;
    }

    BT_DRV_REG_OP_CLK_DIB();
    BT_DRV_REG_OP_EXIT();
    BT_DRV_TRACE(0,"bt_drv_reg_op_get_3m_flag=0x%x\n",ret);
    return ret;
}

bool bt_drv_reg_op_get_3m_flag(void);
///only used for testmode
void bt_drv_reg_op_adjust_rf_param_for_testmode(void)
{
    if(bt_drv_reg_op_get_3m_flag())
    {
        BT_DRV_TRACE(0,"3M set 0x8c 0x9004");
        btdrv_write_rf_reg(0x8c,0x9004);
        btdrv_write_rf_reg(0x1B9,0x20A);
        btdrv_write_rf_reg(0x1BA,0x20C);
        btdrv_write_rf_reg(0x1BB,0x21F);
        btdrv_write_rf_reg(0x1BC,0x23F);
        btdrv_write_rf_reg(0x1B8,0x020A);
    }
    else
    {
        BT_DRV_TRACE(0,"else set 0x8c 0x9000");
        btdrv_write_rf_reg(0x8c,0x9000);

        btdrv_write_rf_reg(0x1B9,0x27F);
        btdrv_write_rf_reg(0x1BA,0x27F);
        btdrv_write_rf_reg(0x1BB,0x27F);
        btdrv_write_rf_reg(0x1BC,0x27F);
        btdrv_write_rf_reg(0x1B8,0x027F);
    }
}

bool btdrv_reg_op_get_local_name(uint8_t* name)
{
    uint32_t btname_ptr = 0;
    bool ret = false;
    BT_DRV_REG_OP_ENTER();
    BT_DRV_REG_OP_CLK_ENB();
    if(lm_env_addr)
    {
        btname_ptr = *(uint32_t *)(lm_env_addr+0x54);
        if(btname_ptr)
        {
            strncpy((char*)name, (char*)btname_ptr, BD_NAME_SIZE);
            BT_DRV_TRACE(0,"bt name %s\n",name);
            ret = true;
        }
        else
        {
            BT_DRV_TRACE(0,"bt name not set\n");
        }
    }
    BT_DRV_REG_OP_CLK_DIB();
    BT_DRV_REG_OP_EXIT();
    return ret;
}

void bt_drv_reg_op_rssi_adjust_param(const struct hci_dbg_set_sw_rssi_cmd *parm)
{
    BT_DRV_REG_OP_ENTER();
    BT_DRV_REG_OP_CLK_ENB();

    if(hci_dbg_set_sw_rssi_addr)
    {
         memcpy((uint8_t *)hci_dbg_set_sw_rssi_addr, parm, sizeof(struct hci_dbg_set_sw_rssi_cmd));
    }

    BT_DRV_REG_OP_CLK_DIB();
    BT_DRV_REG_OP_EXIT();
}
void bt_drv_reg_op_rssi_adjust_init(bool en)
{
    enum HAL_CHIP_METAL_ID_T metal_id = hal_get_chip_metal_id();

    BT_DRV_REG_OP_ENTER();
    BT_DRV_REG_OP_CLK_ENB();
    //RF metal ID change
    if (metal_id >= HAL_CHIP_METAL_ID_1)
    {
        *(uint32_t *)RSSI_I2V_CHECK_ENBALE_ADDR = en;
        bt_drv_rf_i2v_check_enable(en);
    }
    BT_DRV_REG_OP_CLK_DIB();
    BT_DRV_REG_OP_EXIT();
}
void bt_drv_reg_op_rssi_adjust_mode(uint32_t mode)
{
    extern const struct hci_dbg_set_sw_rssi_cmd  sw_rssi_idle_mode;
    extern const struct hci_dbg_set_sw_rssi_cmd  sw_rssi_work_mode;
	extern const struct hci_dbg_set_sw_rssi_cmd  sw_rssi_test_mode;
    static uint8_t rssi_adjust_mode_bak = RSSI_ADJUST_INIT_MODE;
    uint8_t rssi_adjust_mode = RSSI_ADJUST_IDLE_MODE;

    switch(mode)
    {
        case BT_WORK_MODE:
            rssi_adjust_mode = RSSI_ADJUST_WORK_MODE;

            break;
        case BT_IDLE_MODE:
            rssi_adjust_mode = RSSI_ADJUST_IDLE_MODE;
            break;
		case BT_TEST_MODE:
            rssi_adjust_mode = RSSI_ADJUST_TEST_MODE;
            break;
        default:
            BT_DRV_TRACE(1,"BT_DRV:rssi adjust mode set error mork mode=%d",mode);
            break;
    }

    if(rssi_adjust_mode != rssi_adjust_mode_bak)
    {
        rssi_adjust_mode_bak = rssi_adjust_mode;
        BT_DRV_TRACE(1,"BT_DRV:use RSSI ADJUST mode=%d[2:IDLE,1:WORK]",rssi_adjust_mode);
        if(rssi_adjust_mode == RSSI_ADJUST_WORK_MODE)
        {
            bt_drv_reg_op_rssi_adjust_param(&sw_rssi_work_mode);
        }
		else if(rssi_adjust_mode == RSSI_ADJUST_TEST_MODE)
        {
            bt_drv_reg_op_rssi_adjust_param(&sw_rssi_test_mode);
        }
        else
        {
            bt_drv_reg_op_rssi_adjust_param(&sw_rssi_idle_mode);
        }
    }
}

void btdrv_patch_set_business_state_flag(uint8_t input_data)
{
    BT_DRV_REG_OP_ENTER();
    BT_DRV_REG_OP_CLK_ENB();
    if(rob_connection_addr)
    {
        BT_DRV_TRACE(2,"BT_DRV:%s set rob:%0x",__func__,(*(volatile uint32_t *)(rob_connection_addr)));
        BT_PATCH_WR(rob_connection_addr,input_data);
    }

    BT_DRV_REG_OP_CLK_DIB();
    BT_DRV_REG_OP_EXIT();
}

void btdrv_patch_clear_business_state_flag(uint8_t input_data)
{
    BT_DRV_REG_OP_ENTER();
    BT_DRV_REG_OP_CLK_ENB();
    if(rob_connection_addr)
    {
        BT_DRV_TRACE(2,"BT_DRV:%s clear rob:%0x",__func__,(*(volatile uint32_t *)(rob_connection_addr)));
        uint8_t value = (*(volatile uint32_t *)(uintptr_t)(rob_connection_addr)) & (~input_data);
        BT_PATCH_WR(rob_connection_addr,value);
    }

    BT_DRV_REG_OP_CLK_DIB();
    BT_DRV_REG_OP_EXIT();
}

void bt_drv_reg_op_init_nosync_info(void)
{

    if(bt_rx_nosync_info_addr)
    {
        for(uint8_t i=0;i<3;i++)
        {
            *(uint8_t *)(bt_rx_nosync_info_addr+6*i) = 3;//control
            *(uint8_t *)(bt_rx_nosync_info_addr+6*i+1) = 12;//link_no_sync_thd
            *(int8_t *)(bt_rx_nosync_info_addr+6*i+2) = -99;//link_no_sync_rssi
            *(uint16_t *)(bt_rx_nosync_info_addr+6*i+4) = 0x100;//link_no_sync_timeout
        }
    }
}

struct bes_fa_agc_config
{
    bool en;
    uint8_t no_sync_adujust_type;
    uint8_t fa_gain_max;
    uint16_t no_sync_ts_thr;
    uint16_t no_sync_cnt_thr;
};

void bt_drv_reg_op_fa_agc_init(void)
{
#ifndef __FIX_FA_RX_GAIN___
    BT_DRV_REG_OP_ENTER();
    BT_DRV_REG_OP_CLK_ENB();

    if(fa_agc_cfg_addr)
    {
        struct bes_fa_agc_config cfg;

        cfg.en = true;//enable fa new agc

        cfg.fa_gain_max = 3;//max fa gain limit

        cfg.no_sync_adujust_type  = 0;//type 1 decrease 1| type 0 change to max

        cfg.no_sync_ts_thr = 100;//slot

        cfg.no_sync_cnt_thr = 6;//number

        *(struct bes_fa_agc_config*)fa_agc_cfg_addr = cfg;
    }

    BT_DRV_REG_OP_CLK_DIB();
    BT_DRV_REG_OP_EXIT();
#endif
}

bool bt_drv_reg_op_read_fa_rssi_in_dbm(rx_agc_t* rx_val)
{
    bool ret = false;
    uint32_t fa_gain = 0xff;
    BT_DRV_REG_OP_ENTER();
    BT_DRV_REG_OP_CLK_ENB();
    if(rx_val != NULL)
    {
        uint32_t localVal = BT_DRIVER_GET_U32_REG_VAL(BT_FA_CTRL_ADDR);
        fa_gain = ((localVal & ((uint32_t)0x000E0000)) >> 17);
        rx_val->rssi = 127;//invalid val
        rx_val->rxgain = fa_gain;
        ret = true;
    }

    BT_DRV_REG_OP_CLK_DIB();
    BT_DRV_REG_OP_EXIT();
    return ret;
}

void bt_drv_reg_op_set_btpcm_trig_flag(bool flag)
{
    BT_DRV_REG_OP_CLK_ENB();

    if(pcm_need_start_flag_addr != 0)
    {
        BTDIGITAL_REG(pcm_need_start_flag_addr) = flag;
    }
    BT_DRV_REG_OP_CLK_DIB();
}

bool bt_drv_error_check_handler(void)
{
    bool ret = false;
    BT_DRV_REG_OP_CLK_ENB();
    if(BTDIGITAL_REG(BT_ERRORTYPESTAT_ADDR) ||
        (BTDIGITAL_REG(BT_CONTROLLER_CRASH_DUMP_ADDR_BASE) !=0 &&
        BTDIGITAL_REG(BT_CONTROLLER_CRASH_DUMP_ADDR_BASE) !=0x42))
    {
        BT_DRV_TRACE(1,"BT_DRV:digital assert,error code=0x%x", BTDIGITAL_REG(BT_ERRORTYPESTAT_ADDR));
        ret = true;
    }
    BT_DRV_REG_OP_CLK_DIB();
    return ret;
}

void bt_drv_reg_op_set_nosig_sch_flag(uint8_t enable)
{
    BT_DRV_REG_OP_CLK_ENB();
    if(nosig_sch_flag_addr)
    {
        *(uint32_t *)nosig_sch_flag_addr = enable;
        BT_DRV_TRACE(0,"1305 nosig_sch_flag=%d",*(uint32_t *)nosig_sch_flag_addr);
    }
    BT_DRV_REG_OP_CLK_DIB();
}

uint8_t bt_drv_reg_op_get_max_acl_nb(void)
{
    return MAX_NB_ACTIVE_ACL;
}

void bt_drv_reg_op_read_snr_buf(uint8_t* p_buff_out)
{
    uint8_t* pbuf = (uint8_t*)(snr_buff_addr);
    BT_DRV_REG_OP_ENTER();
    BT_DRV_REG_OP_CLK_ENB();

    memcpy(p_buff_out, pbuf, 79);

    BT_DRV_REG_OP_CLK_DIB();
    BT_DRV_REG_OP_EXIT();
}


void bt_drv_reg_op_set_btclk_seed(uint32_t seed)
{
    BT_DRV_REG_OP_CLK_ENB();
    if(clk_seed_addr)
    {
        *(uint32_t *)clk_seed_addr = seed&0xfff;
      //  *(uint32_t *)0xc0000650 = seed&0xfff;
        BT_DRV_TRACE(0,"1305 clk seed=%d",seed);

    }
    BT_DRV_REG_OP_CLK_DIB();

}
void bt_drv_reg_op_set_accept_new_mobile_enable(void)
{
#ifdef ACCEPT_NEW_MOBILE_EN
    BT_DRV_REG_OP_ENTER();
    BT_DRV_REG_OP_CLK_ENB();
    if (hal_get_chip_metal_id() >= HAL_CHIP_METAL_ID_0)
    {
        if (bt_setting_2500i[97] == 0)
        {
            uint8_t *p_new_accept_enable_addr = (uint8_t*)(dbg_bt_setting_addr + 0x67);
            BT_DRV_TRACE(0,"BT_DRV_CONFIG:set_new_mobile enable");
            bt_setting_2500i[97] = 1;
            *p_new_accept_enable_addr = 1;
        }
    }
    BT_DRV_REG_OP_CLK_DIB();
    BT_DRV_REG_OP_EXIT();
#endif
}

void bt_drv_reg_op_clear_accept_new_mobile_enable(void)
{
#ifdef ACCEPT_NEW_MOBILE_EN
    BT_DRV_REG_OP_ENTER();
    BT_DRV_REG_OP_CLK_ENB();
    if (hal_get_chip_metal_id() >= HAL_CHIP_METAL_ID_0)
    {
        if (bt_setting_2500i[97] == 1)
        {
            uint8_t *p_new_accept_enable_addr = (uint8_t*)(dbg_bt_setting_addr + 0x67);
            BT_DRV_TRACE(0,"BT_DRV_CONFIG:clear_new_mobile enable");
            bt_setting_2500i[97] = 0;
            *p_new_accept_enable_addr = 0;
        }
    }
    BT_DRV_REG_OP_CLK_DIB();
    BT_DRV_REG_OP_EXIT();
#endif
}

uint32_t bt_drv_reg_op_get_bt_pcm_valid_clk(void)
{
    uint32_t ret = 0;
    BT_DRV_REG_OP_ENTER();
    BT_DRV_REG_OP_CLK_ENB();
    if(bt_pcm_valid_clk_addr)
    {
        ret = BT_DRIVER_GET_U32_REG_VAL(bt_pcm_valid_clk_addr);
    }
    BT_DRV_REG_OP_CLK_DIB();
    BT_DRV_REG_OP_EXIT();
    return ret;
}

uint32_t bt_drv_reg_op_get_bt_pcm_invalid_clk(void)
{
    uint32_t ret = 0;
    BT_DRV_REG_OP_ENTER();
    BT_DRV_REG_OP_CLK_ENB();

    if(bt_pcm_invalid_clk_addr)
    {
        ret = BT_DRIVER_GET_U32_REG_VAL(bt_pcm_invalid_clk_addr);
    }

    BT_DRV_REG_OP_CLK_DIB();
    BT_DRV_REG_OP_EXIT();
    return ret;
}

int8_t bt_drv_reg_op_get_tx_pwr_dbm(uint16_t conhdl)
{
    //0dbm
    return 0;
}