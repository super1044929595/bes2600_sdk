
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
#include "btif_sys_config.h"

#ifdef BTIF_DIP_DEVICE

#include <stdio.h>
#include "cmsis_os.h"
#include "hal_trace.h"
#include "dip_api.h"
#include "plat_types.h"
#include "app_bt.h"
#include "app_dip.h"
#include "nvrecord_bt.h"

#if defined(ENHANCED_STACK)
#include "sdp_api.h"
#include "me_api.h"
#endif

#if defined(IBRT)
#include "app_tws_ibrt_cmd_handler.h"
#include "app_tws_ctrl_thread.h"
#include "app_ibrt_peripheral_manager.h"
#include "app_ibrt_if.h"
#include "app_tws_if.h"
#endif

#if defined(__AI_VOICE__) || defined(BISTO_ENABLED)
#include "app_ai_if.h"
#endif

#if defined(__AI_VOICE__)
#include "ai_thread.h"
#endif

#ifdef CUSTOM_BITRATE
void a2dp_avdtpcodec_sbc_user_configure(uint32_t bitpool,uint8_t user_configure);
void a2dp_avdtpcodec_aac_user_configure(uint32_t bitrate,uint8_t user_configure);
void app_audio_dynamic_update_dest_packet_mtu(uint8_t codec_index, uint8_t packet_mtu, uint8_t user_configure);
#endif

void app_dip_sync_dip_info(void);
static uint16_t target_device_type = 0;

uint16_t app_dip_get_target_device_type(void)
{
    TRACE(2, "%s type = %d",__func__,target_device_type);
    return target_device_type;
}

bool app_dip_is_target_device_is_samsung()
{
    return (target_device_type == DEVICE_TYPE_ANDROID_SAMSUNG);
}

static void app_dip_callback(bt_bdaddr_t *_addr, uint16_t device_type , bool target_get)
{
    btif_remote_device_t *p_remote_dev = btif_me_get_remote_device_by_bdaddr(_addr);

    TRACE(2,"%s dev %p addr :", __func__, p_remote_dev);
    DUMP8("%x ", _addr->address, 6);

#if defined(IBRT)
    if (TWS_LINK == app_tws_ibrt_get_remote_link_type(p_remote_dev))
    {
        TRACE(1,"%s connect type is TWS", __func__);
        return;
    }

    app_dip_sync_dip_info();
#endif

#ifdef CUSTOM_BITRATE
    if(target_get == true)
    {
        a2dp_avdtpcodec_sbc_user_configure(41, true);
        a2dp_avdtpcodec_aac_user_configure(156*1024, true);
    }
    else
    {
        a2dp_avdtpcodec_sbc_user_configure(nv_record_get_extension_entry_ptr()->a2dp_user_info.sbc_bitpool, true);
        a2dp_avdtpcodec_aac_user_configure(nv_record_get_extension_entry_ptr()->a2dp_user_info.aac_bitrate, true);
    }
#endif    
    target_device_type = device_type;
    switch(device_type)
    {
        case DEVICE_TYPE_IOS:
            break;
        case DEVICE_TYPE_ANDROID_SAMSUNG:
            break;
        case DEVICE_TYPE_ANDROID_SONY:
            break;
        default:
            break;
    }

#if defined(__AI_VOICE__) || defined(BISTO_ENABLED)
    app_ai_if_mobile_connect_handle(_addr);
#endif
}

#if defined(IBRT)
void app_dip_sync_info_prepare_handler(uint8_t *buf, uint16_t *length)
{
    uint32_t offset = 0;
    ibrt_ctrl_t *p_g_ibrt_ctrl = app_tws_ibrt_get_bt_ctrl_ctx();
    bt_bdaddr_t *mobile_addr = &p_g_ibrt_ctrl->mobile_addr;
    uint16_t vend_id = 0;
    uint16_t vend_id_source = 0;

    btif_dip_get_record_vend_id_and_source(mobile_addr, &vend_id, &vend_id_source);
    memcpy(buf, mobile_addr->address, 6);
    offset += 6;
    memcpy(buf+offset, (uint8_t *)&vend_id, 2);
    offset += 2;
    memcpy(buf+offset, (uint8_t *)&vend_id_source, 2);
    offset += 2;

    *length = offset;
}

void app_dip_sync_info_received_handler(uint8_t *buf, uint16_t length)
{
    bt_bdaddr_t mobile_addr;
    uint16_t vend_id = 0;
    uint16_t vend_id_source = 0;
    uint32_t offset = 0;
    nvrec_btdevicerecord *record = NULL;

    memcpy(mobile_addr.address, buf, 6);
    offset += 6;
    memcpy((uint8_t *)&vend_id, buf+offset, 2);
    offset += 2;
    memcpy((uint8_t *)&vend_id_source, buf+offset, 2);
    offset += 2;
    TRACE(2, "%s vend_id 0x%x vend_id_source 0x%x addr:", __func__, vend_id, vend_id_source);
    DUMP8("0x%x ", mobile_addr.address, 6);

    if (vend_id && !nv_record_btdevicerecord_find(&mobile_addr, &record))
    {
        nv_record_btdevicerecord_set_vend_id_and_source(record, vend_id, vend_id_source);
    }
}

void app_dip_sync_init(void)
{
    TWS_SYNC_USER_T user_app_dip_t = {
        app_dip_sync_info_prepare_handler,
        app_dip_sync_info_received_handler,
        NULL,
        NULL,
        NULL,
    };

    app_tws_if_register_sync_user(TWS_SYNC_USER_DIP, &user_app_dip_t);
}

void app_dip_sync_dip_info(void)
{
    app_tws_if_prepare_sync_info();
    app_tws_if_sync_info(TWS_SYNC_USER_DIP);
    app_tws_if_flush_sync_info();
}
#endif

void app_dip_init(void)
{
    btif_dip_init(app_dip_callback);
}
#endif

