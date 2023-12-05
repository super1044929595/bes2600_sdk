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
#include "bluetooth.h"
#include "hal_cmu.h"
#include "app_tota_cmd_code.h"
#include "app_tota.h"
#include "app_tota_cmd_handler.h"
#include "cmsis.h"
#include "app_hfp.h"
#include "app_key.h"
#include "app_tota_general.h"
#include "app_spp_tota.h"
#include "nvrecord_ble.h"
#include "app_ibrt_rssi.h"
#include "app_tota_if.h"
#include "besbt.h"
#include "app_tota_common.h"
#include "app_ibrt_ui_test.h"
#include "app_ibrt_ui_test_cmd_if.h"
#include "app_tws_ctrl_thread.h"
#include "apps.h"
#include "app_bt_stream.h"
#if ANC_APP
#include "app_anc.h"
#endif
#if defined(ANC_ASSIST_USE_PILOT)
#include "app_voice_assist_custom_leak_detect.h"
#endif


#define TOTA_CRC_CHECK  0x54534542
#define LEN_OF_IMAGE_TAIL_TO_FINDKEY_WORD    512


/*
** general info struct
**  ->  bt  name
**  ->  ble name
**  ->  bt  local/peer addr
**  ->  ble local/peer addr
**  ->  ibrt role
**  ->  crystal freq
**  ->  xtal fcap
**  ->  bat volt/level/status
**  ->  fw version
**  ->  ear location
**  ->  rssi info
*/

/*------------------------------------------------------------------------------------------------------------------------*/
typedef struct{
    uint32_t    address;
    uint32_t    length;
}TOTA_DUMP_INFO_STRUCT_T;

typedef struct{
    uint32_t    crc;            //fw crc, generate by generate_crc32_of_image.py
    uint8_t     version[4];     //fw version
    uint8_t     build_date[32]; //fw build data,auto generate by compiler when build
}TOTA_CRC_CHECK_STRUCT_T;

#ifdef FIRMWARE_REV
extern "C" void system_get_info(uint8_t *fw_rev_0, uint8_t *fw_rev_1,  uint8_t *fw_rev_2, uint8_t *fw_rev_3);
#endif
extern const char sys_build_info[];

static const char* image_info_sanity_crc_key_word = "CRC32_OF_IMAGE=0x";
static const char* image_info_build_data = "BUILD_DATE=";

static int32_t find_key_word(uint8_t* targetArray, uint32_t targetArrayLen,
    uint8_t* keyWordArray,
    uint32_t keyWordArrayLen);

static uint8_t asciiToHex(uint8_t asciiCode);
static APP_TOTA_CMD_RET_STATUS_E tota_get_sanity_crc(uint32_t *sanityCrc32);
static APP_TOTA_CMD_RET_STATUS_E tota_get_buidl_data(uint8_t *buildData);



static void general_get_flash_dump_info(TOTA_DUMP_INFO_STRUCT_T * p_flash_info);

/*
**  general info
*/
static general_info_t general_info;


/*
**  get general info
*/
static void __get_general_info();
extern void bt_audio_updata_eq(uint8_t index);

static void app_tota_general_cmd_deal_with_custom_cmd(uint8_t* ptrParam, uint32_t paramLen)
{
    uint8_t custom_cmd_type = ptrParam[0];
    uint8_t resData[60]={0};
    uint32_t resLen=1;
    TRACE(1,"custom_cmd_type is %d", custom_cmd_type);

    if(APP_TOTA_BUTTON_SETTINGS_CONTROL_CMD == custom_cmd_type)
    {
        if(BUTTON_SETTING_LEFT_EARPHONE_CMD == ptrParam[1])
        {
            if(BUTTON_SETTING_PRE_SONG_CMD == ptrParam[2])
            {
                app_ibrt_ui_audio_backward_test();
            }
            else if(BUTTON_SETING_NEXT_SONG_CMD == ptrParam[2])
            {
                app_ibrt_ui_audio_forward_test();
            }
#if ANC_APP
            else if(BUTTON_SETTING_ANC_CMD == ptrParam[2])
            {
                app_anc_loop_switch();
            }
#endif
            else if(BUTTON_SETTING_CALL_CMD == ptrParam[2])
            {
                app_ibrt_ui_call_redial_test();
            }
            else
            {
                TRACE(0,"error cmd");
            }
            
        }

        if(BUTTON_SETTING_RIGHT_EARPHONE_CMD == ptrParam[1])
        {

            if(BUTTON_SETTING_PRE_SONG_CMD == ptrParam[2])
            {
                app_ibrt_ui_audio_backward_test();
            }
            else if(BUTTON_SETING_NEXT_SONG_CMD == ptrParam[2])
            {
                app_ibrt_ui_audio_forward_test();
            }
#if ANC_APP
            else if(BUTTON_SETTING_ANC_CMD == ptrParam[2])
            {
                app_anc_loop_switch();
            }
#endif
            else if(BUTTON_SETTING_CALL_CMD == ptrParam[2])
            {
                app_ibrt_ui_call_redial_test();
            }
            else
            {
                TRACE(0,"error cmd");
            }
        }

    }
    else if( APP_TOTA_FACTORY_RESET_CMD == custom_cmd_type)
    {
        TRACE(1,"custom: factory reset  connect path %d",tota_get_connect_path());
        if(APP_TOTA_VIA_NOTIFICATION == tota_get_connect_path())
        {
             ///>send to slave
             tws_ctrl_send_cmd(APP_TWS_CMD_SYNC_TOTA_FACTORY_RESET, NULL, 0);
        }
        nv_record_rebuild();
        app_reset();
    }
    else if(APP_TOTA_MUSIC_PLAY_SETTINGS_CMD == custom_cmd_type)
    {
        if(MUSIC_PLAY_SETTINGS_PLAY_CMD == ptrParam[1])
        {
            TRACE(0,"play");
            app_ibrt_ui_audio_play_test();
        }
        else if(MUSIC_PLAY_SETTINGS_PAUSE_CMD == ptrParam[1])
        {
            TRACE(0,"pause");
            app_ibrt_ui_audio_pause_test();
        }
        else if(MUSIC_PLAY_SETTINGS_NEXT_CMD == ptrParam[1])
        {
            TRACE(0,"forward");
            app_ibrt_ui_audio_forward_test();
        }
        else if(MUSIC_PLAY_SETTINGS_PRE_CMD == ptrParam[1])
        {
            TRACE(0,"backward");
            app_ibrt_ui_audio_backward_test();
        }
        else
        {
            TRACE(0,"error command");
        }
    }
    else if(APP_TOTA_BATTERY_LEVEL_CMD == custom_cmd_type)
    {
        uint8_t battery_levels = (app_battery_current_level()+1) * 10;
        resData[0] = APP_TOTA_BATTERY_LEVEL_CMD;
        resData[1] = battery_levels;
        resLen = 0x02;
        app_tota_send_rsp(OP_TOTA_SET_CUSTOMER_CMD, TOTA_NO_ERROR, resData, resLen);

        return ;
        
    }
#if defined(ANC_ASSIST_USE_PILOT)
    else if(APP_TOTA_EARBUD_FIT_TEST_CMD == custom_cmd_type)
    {
        TRACE(0,"earbud fit test");
        app_voice_assist_custom_leak_detect_open();

        while(app_voice_get_leak_detect_semaphore()==0)
        {
             if(app_voice_get_leak_detect_semaphore())
                 break;
        }
        bool status = app_voice_assistant_get_status();
        resData[0] = APP_TOTA_EARBUD_FIT_TEST_CMD;
        resData[1] = status;
        resLen = 0x02;
        app_tota_send_rsp(OP_TOTA_SET_CUSTOMER_CMD, TOTA_NO_ERROR, resData, resLen);
        app_voice_get_leak_detect_status_semaphore_release();
        return ;
    }
#endif
    else if(APP_TOAT_EQ_CMD == custom_cmd_type)
    {
        TRACE(0,"EQ set");
        bt_audio_updata_eq(ptrParam[1]);
   
    }
    else
    {
        TRACE(0,"error custom cmd type");
    }

    app_tota_send_rsp(OP_TOTA_SET_CUSTOMER_CMD,TOTA_NO_ERROR,resData,resLen);
    
}

/*
**  handle general cmd
*/
static void __tota_general_cmd_handle(APP_TOTA_CMD_CODE_E funcCode, uint8_t* ptrParam, uint32_t paramLen);

/*------------------------------------------------------------------------------------------------------------------------*/


/*------------------------------------------------------------------------------------------------------------------------*/

static void __tota_general_cmd_handle(APP_TOTA_CMD_CODE_E funcCode, uint8_t* ptrParam, uint32_t paramLen)
{
    TOTA_LOG_DBG(3,"[%s]: opCode:0x%x, paramLen:%d", __func__, funcCode, paramLen);
    DUMP8("%02x ", ptrParam, paramLen);
    uint8_t resData[60]={0};
    uint32_t resLen=1;
    uint8_t volume_level;
    switch (funcCode)
    {
        case OP_TOTA_GENERAL_INFO_CMD:
            __get_general_info();
            app_tota_send_data(funcCode, (uint8_t*)&general_info, sizeof(general_info_t));
            return ;
        case OP_TOTA_EXCHANGE_MTU_CMD:
            tota_set_trans_MTU(*(uint16_t *)ptrParam);
           break;
        case OP_TOTA_VOLUME_PLUS_CMD:
            app_bt_volumeup();
            volume_level = app_bt_stream_local_volume_get();
            // resData[0] = volume_level;
            TRACE(1,"volume = %d",volume_level);
            break;
        case OP_TOTA_VOLUME_DEC_CMD:
            app_bt_volumedown();
            volume_level = app_bt_stream_local_volume_get();
            // resData[0] = volume_level;
            // resLen = 1;
            TRACE(1,"volume = %d",volume_level);
            break;
        case OP_TOTA_VOLUME_SET_CMD:
            //uint8_t scolevel = ptrParam[0];
            //uint8_t a2dplevel = ptrParam[1];
            app_bt_set_volume(APP_BT_STREAM_HFP_PCM,ptrParam[0]);
            app_bt_set_volume(APP_BT_STREAM_A2DP_SBC,ptrParam[1]);
            btapp_hfp_report_speak_gain();
            btapp_a2dp_report_speak_gain();
            break;
        case OP_TOTA_VOLUME_GET_CMD:
            resData[0] = app_bt_stream_hfpvolume_get();
            resData[1] = app_bt_stream_a2dpvolume_get();
            resLen = 2;
            app_tota_send_data(funcCode, resData, resLen);
            return;
        case OP_TOTA_GET_RSSI_CMD:
            app_ibrt_rssi_get_stutter(resData, &resLen);
            app_tota_send_data(funcCode, resData, resLen);
            return;
        case OP_TOTA_GET_DUMP_INFO_CMD:
        {
            TOTA_DUMP_INFO_STRUCT_T dump_info;
            general_get_flash_dump_info(&dump_info);
            app_tota_send_data(funcCode, (uint8_t*)&dump_info, sizeof(TOTA_DUMP_INFO_STRUCT_T));
            return ;
        }
        case OP_TOTA_SET_CUSTOMER_CMD:
             app_tota_general_cmd_deal_with_custom_cmd(ptrParam, paramLen);
        
            TRACE(0, "[%s] OP_TOTA_SET_CUSTOMER_CMD ...", __func__);
            return;
        case OP_TOTA_GET_CUSTOMER_CMD:
            TRACE(0, "[%s] OP_TOTA_GET_CUSTOMER_CMD ...", __func__);
            break;
        case OP_TOTA_CRC_CHECK_CMD:
        {
            APP_TOTA_CMD_RET_STATUS_E status = TOTA_NO_ERROR;
            TRACE(1, "[%s] OP_TOTA_CRC_CHECK_CMD ...", __func__);

            if(*(int*)ptrParam == TOTA_CRC_CHECK){

                TOTA_CRC_CHECK_STRUCT_T *pdata = (TOTA_CRC_CHECK_STRUCT_T *)resData;

                status = tota_get_sanity_crc(&pdata->crc);

                status = tota_get_buidl_data(pdata->build_date);
#ifdef FIRMWARE_REV
                system_get_info(&(pdata->version[0]),&(pdata->version[1]),&(pdata->version[2]),&(pdata->version[3]));
#endif
                TRACE(0, "|--------------------------------|");
                TRACE(1, "CRC: 0x%x", pdata->crc);
                TRACE(1, "Version: %d.%d.%d.%d", pdata->version[0],pdata->version[1],pdata->version[2],pdata->version[3]);
                TRACE(1, "Build Data: %s", pdata->build_date);
                TRACE(0, "|--------------------------------|");
            }else{
                status =TOTA_INVALID_DATA_PACKET;
            }

            resLen = sizeof(TOTA_CRC_CHECK_STRUCT_T);
            app_tota_send_rsp(funcCode, status, resData, resLen);
            return;
        }
        case OP_TOTA_RAW_DATA_SET_CMD:
            app_ibrt_debug_parse(ptrParam, paramLen);
            break;
       //end
        default:
            // TRACE(1,"wrong cmd 0x%x",funcCode);
            // resData[0] = -1;
            return;
    }
    app_tota_send_rsp(funcCode,TOTA_NO_ERROR,resData,resLen);
}
/* get general info */
static void __get_general_info()
{

    /* get bt-ble name */
    uint8_t* factory_name_ptr =factory_section_get_bt_name();
    if ( factory_name_ptr != NULL )
    {
        uint16_t valid_len = strlen((char*)factory_name_ptr) > BT_BLE_LOCAL_NAME_LEN? BT_BLE_LOCAL_NAME_LEN:strlen((char*)factory_name_ptr);
        memcpy(general_info.btName,factory_name_ptr,valid_len);
    }

    factory_name_ptr =factory_section_get_ble_name();
    if ( factory_name_ptr != NULL )
    {
        uint16_t valid_len = strlen((char*)factory_name_ptr) > BT_BLE_LOCAL_NAME_LEN? BT_BLE_LOCAL_NAME_LEN:strlen((char*)factory_name_ptr);
        memcpy(general_info.bleName,factory_name_ptr,valid_len);
    }
#ifdef IBRT
    /* get bt-ble peer addr */
    ibrt_config_t addrInfo;
    app_ibrt_ui_test_config_load(&addrInfo);
    general_info.ibrtRole = addrInfo.nv_role;
    memcpy(general_info.btLocalAddr.address, addrInfo.local_addr.address, 6);
    memcpy(general_info.btPeerAddr.address, addrInfo.peer_addr.address, 6);

    #ifdef __IAG_BLE_INCLUDE__
    memcpy(general_info.bleLocalAddr.address, bt_get_ble_local_address(), 6);
    #ifdef TWS_SYSTEM_ENABLED
    memcpy(general_info.blePeerAddr.address, nv_record_tws_get_peer_ble_addr(), 6);
    #endif
    #endif
#endif
    /* get crystal info */
    general_info.crystal_freq = hal_cmu_get_crystal_freq();

    /* factory_section_xtal_fcap_get */
    factory_section_xtal_fcap_get(&general_info.xtal_fcap);
    
    /* get battery info (volt level)*/
    app_battery_get_info(&general_info.battery_volt,&general_info.battery_level,&general_info.battery_status);

    /* get firmware version */
#ifdef FIRMWARE_REV
    system_get_info(&general_info.fw_version[0],&general_info.fw_version[1],&general_info.fw_version[2],&general_info.fw_version[3]);
    TRACE(4,"firmware version = %d.%d.%d.%d",general_info.fw_version[0],general_info.fw_version[1],general_info.fw_version[2],general_info.fw_version[3]);
#endif

    /* get ear location info */
    if ( app_tws_is_right_side() )      general_info.ear_location = EAR_SIDE_RIGHT;
    else if ( app_tws_is_left_side() )  general_info.ear_location = EAR_SIDE_LEFT;
    else                                general_info.ear_location = EAR_SIDE_UNKNOWN;

    app_ibrt_rssi_get_stutter(general_info.rssi, &general_info.rssi_len);
}

static int32_t find_key_word(uint8_t* targetArray, uint32_t targetArrayLen,
    uint8_t* keyWordArray,
    uint32_t keyWordArrayLen)
{
    if ((keyWordArrayLen > 0) && (targetArrayLen >= keyWordArrayLen))
    {
        uint32_t index = 0, targetIndex = 0;
        for (targetIndex = 0;targetIndex < targetArrayLen;targetIndex++)
        {
            for (index = 0;index < keyWordArrayLen;index++)
            {
                if (targetArray[targetIndex + index] != keyWordArray[index])
                {
                    break;
                }
            }

            if (index == keyWordArrayLen)
            {
                return targetIndex;
            }
        }

        return -1;
    }
    else
    {
        return -1;
    }
}

static uint8_t asciiToHex(uint8_t asciiCode)
{
    if ((asciiCode >= '0') && (asciiCode <= '9'))
    {
        return asciiCode - '0';
    }
    else if ((asciiCode >= 'a') && (asciiCode <= 'f'))
    {
        return asciiCode - 'a' + 10;
    }
    else if ((asciiCode >= 'A') && (asciiCode <= 'F'))
    {
        return asciiCode - 'A' + 10;
    }
    else
    {
        return 0xff;
    }
}

static APP_TOTA_CMD_RET_STATUS_E tota_get_sanity_crc(uint32_t *sanityCrc32) 
{
    if(NULL == sanityCrc32){
        return TOTA_CMD_HANDLING_FAILED;
    }

    int32_t found = find_key_word((uint8_t*)&sys_build_info,
        LEN_OF_IMAGE_TAIL_TO_FINDKEY_WORD,
        (uint8_t*)image_info_sanity_crc_key_word,
        strlen(image_info_sanity_crc_key_word));
    if (-1 == found){
        return TOTA_CMD_HANDLING_FAILED;
    }

    uint8_t* crcString = (uint8_t*)&sys_build_info+found+strlen(image_info_sanity_crc_key_word);

    for (uint8_t index = 0;index < 8;index++)
    {
        *sanityCrc32 |= (asciiToHex(crcString[index]) << (28-4*index));
    }

    TRACE(1,"sanityCrc32 is 0x%x", *sanityCrc32);

    return TOTA_NO_ERROR;
}

static APP_TOTA_CMD_RET_STATUS_E tota_get_buidl_data(uint8_t *buildData) 
{
    if(NULL == buildData){
        return TOTA_CMD_HANDLING_FAILED;
    }
    int32_t found = find_key_word((uint8_t*)&sys_build_info,
        LEN_OF_IMAGE_TAIL_TO_FINDKEY_WORD,
        (uint8_t*)image_info_build_data,
        strlen(image_info_build_data));
    if (-1 == found){
        return TOTA_CMD_HANDLING_FAILED;
    }

    memcpy(buildData, (uint8_t*)&sys_build_info+found+strlen(image_info_build_data), 20);

    TRACE(1,"buildData is 0x%s", buildData);

    return TOTA_NO_ERROR;
}



/* general command */
TOTA_COMMAND_TO_ADD(OP_TOTA_GENERAL_INFO_CMD, __tota_general_cmd_handle, false, 0, NULL );
TOTA_COMMAND_TO_ADD(OP_TOTA_EXCHANGE_MTU_CMD, __tota_general_cmd_handle, false, 0, NULL );
TOTA_COMMAND_TO_ADD(OP_TOTA_VOLUME_PLUS_CMD, __tota_general_cmd_handle, false, 0, NULL );
TOTA_COMMAND_TO_ADD(OP_TOTA_VOLUME_DEC_CMD, __tota_general_cmd_handle, false, 0, NULL );
TOTA_COMMAND_TO_ADD(OP_TOTA_VOLUME_SET_CMD, __tota_general_cmd_handle, false, 0, NULL );
TOTA_COMMAND_TO_ADD(OP_TOTA_VOLUME_GET_CMD, __tota_general_cmd_handle, false, 0, NULL );
TOTA_COMMAND_TO_ADD(OP_TOTA_GET_RSSI_CMD, __tota_general_cmd_handle, false, 0, NULL );
TOTA_COMMAND_TO_ADD(OP_TOTA_GET_DUMP_INFO_CMD, __tota_general_cmd_handle, false, 0, NULL );
TOTA_COMMAND_TO_ADD(OP_TOTA_SET_CUSTOMER_CMD, __tota_general_cmd_handle, false, 0, NULL );
TOTA_COMMAND_TO_ADD(OP_TOTA_GET_CUSTOMER_CMD, __tota_general_cmd_handle, false, 0, NULL );
TOTA_COMMAND_TO_ADD(OP_TOTA_CRC_CHECK_CMD, __tota_general_cmd_handle, false, 0, NULL );
TOTA_COMMAND_TO_ADD(OP_TOTA_RAW_DATA_SET_CMD, __tota_general_cmd_handle, false, 0, NULL );

static void general_get_flash_dump_info(TOTA_DUMP_INFO_STRUCT_T * p_flash_info)
{
#ifdef DUMP_LOG_ENABLE
    p_flash_info->address =  (uint32_t)&__log_dump_start;
    p_flash_info->length = LOG_DUMP_SECTION_SIZE;
#else
    p_flash_info->address =  0;
    p_flash_info->length = 0;
#endif
}
