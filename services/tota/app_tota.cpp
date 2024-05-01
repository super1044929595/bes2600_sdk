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
#include "hal_trace.h"
#include "hal_timer.h"
#include "app_audio.h"
#include "app_utils.h"
#include "hal_aud.h"
#include "hal_norflash.h"
#include "pmu.h"
#include "string.h"
#include "cmsis_os.h"
#include "app_tota.h"
#include "app_tota_cmd_code.h"
#include "app_tota_cmd_handler.h"
#include "app_tota_data_handler.h"
#include "app_spp_tota.h"
#include "cqueue.h"
#include "app_ble_rx_handler.h"
#include "rwapp_config.h"
#include "btapp.h"
#include "app_bt.h"
#include "apps.h"
#include "app_thread.h"
#include "cqueue.h"
#include "hal_location.h"
#include "app_hfp.h"
#include "bt_drv_reg_op.h"
#if defined(IBRT) 
#include "app_tws_ibrt.h"
#endif
#include "cmsis.h"
#include "app_battery.h"
#include "crc32.h"
#include "factory_section.h"
#include "app_ibrt_rssi.h"
#include "app_spp_tota_general_service.h"
#include "app_tws_if.h"


typedef struct
{
    uint8_t connectedType;
    APP_TOTA_TRANSMISSION_PATH_E dataPath;
} APP_TOTA_ENV_T;

static APP_TOTA_ENV_T app_tota_env= 
    {
        0,
    };

bool app_is_in_tota_mode(void)
{
    return app_tota_env.connectedType;
}

void app_tota_init(void)
{
    TOTA_LOG_DBG(0,"Init application test over the air.");
    app_spp_tota_init();
    app_spp_tota_gen_init();

    app_tota_cmd_handler_init();
    app_tota_data_reset_env();
#if (BLE_APP_TOTA)
    if(!get_init_state())
    {
        app_ble_rx_handler_init();
    }
#endif
}

extern "C" void bulk_read_done(void);
void app_tota_connected(uint8_t connType)
{
    app_tota_data_reset_env();
    TOTA_LOG_DBG(0,"Tota is connected.");
    app_tota_env.connectedType |= connType;
#if defined(APP_ANC_TEST)
	app_spp_tota_register_tx_done(bulk_read_done);
#endif
}

void app_tota_disconnected(uint8_t disconnType)
{
    TOTA_LOG_DBG(0,"Tota is disconnected.");
    app_tota_env.connectedType &= disconnType;

	app_spp_tota_register_tx_done(NULL);
}

void app_tota_general_connected(uint8_t connType)
{
    app_tota_data_reset_env();
    TOTA_LOG_DBG(0,"Tota gen is connected.");
    app_tota_env.connectedType |= connType;
}

void app_tota_update_datapath(APP_TOTA_TRANSMISSION_PATH_E dataPath)
{
    app_tota_env.dataPath = dataPath;
}

APP_TOTA_TRANSMISSION_PATH_E app_tota_get_datapath(void)
{
    return app_tota_env.dataPath;
}

void app_tota_data_ack_received(uint32_t dataLength)
{
    TOTA_LOG_DBG(1,"%d bytes have been received by the peer device.", dataLength);
}

static void app_tota_demo_cmd_handler(APP_TOTA_CMD_CODE_E funcCode, uint8_t* ptrParam, uint32_t paramLen)
{
    TOTA_LOG_DBG(2,"Func code 0x%x, param len %d", funcCode, paramLen);
    TOTA_LOG_DBG(0,"Param content:");
    DUMP8("%02x ", ptrParam, paramLen);

}

TOTA_COMMAND_TO_ADD(OP_TOTA_DEMO_CMD, app_tota_demo_cmd_handler, false, 0, NULL );

extern "C" int8_t app_battery_current_level(void);

void app_bt_volumeup();
void app_bt_volumedown();
int app_bt_stream_local_volume_get(void);
uint8_t app_audio_get_eq();
int app_audio_set_eq(uint8_t index);
bool app_meridian_eq(bool onoff);
bool app_is_meridian_on();


#define RES_DATA_LEN 48
#define MAX_FLASH_HANDLE_SIZE 650


typedef struct{
    uint32_t    address;
    uint32_t    length;
}TOTA_DUMP_INFO_STRUCT_T;

typedef struct{
    uint32_t    address;
    uint16_t    length;
}TOTA_READ_FLASH_STRUCT_T;

typedef struct{
    uint32_t    address;
    uint16_t    length;
    uint8_t     dataBuf[MAX_FLASH_HANDLE_SIZE];
}TOTA_FLASH_DATA_T;

#define TOTA_DUMP_LOG_ENABLE
#define TOTA_SPPLOG_SECTION_SIZE 0x8000
extern uint32_t __log_dump_start[];
static TOTA_FLASH_DATA_T flashData;

static void general_get_flash_dump_info(TOTA_DUMP_INFO_STRUCT_T * p_flash_info)
{
#ifdef TOTA_DUMP_LOG_ENABLE
    p_flash_info->address =  (uint32_t)&__log_dump_start;
    p_flash_info->length = TOTA_SPPLOG_SECTION_SIZE;
#else
    p_flash_info->address =  (uint32_t)&__log_dump_start;
    p_flash_info->length = 0;
#endif
}

static TOTA_FLASH_DATA_T * _read_flash_by_address(uint32_t readAddr, uint16_t length)
{
    flashData.address = readAddr;
    flashData.length = length;
    memcpy(flashData.dataBuf, (uint8_t*)readAddr, length);
    return &flashData;
}


static void app_tota_vendor_cmd_handler(APP_TOTA_CMD_CODE_E funcCode, uint8_t* ptrParam, uint32_t paramLen)
{
    TOTA_LOG_DBG(2,"Func code 0x%x, param len %d", funcCode, paramLen);
    TOTA_LOG_DBG(0,"Param content:");
    DUMP8("%02x ", ptrParam, paramLen);
    //uint8_t resData[20]={0};
    uint8_t resData[RES_DATA_LEN]={0};
    uint32_t resLen=1;

    TOTA_FLASH_DATA_T * pflashData = NULL;
    TOTA_READ_FLASH_STRUCT_T * pflash_read_struct= NULL;
    switch (funcCode)
    {
        case OP_TOTA_BATTERY_STATUS_CMD:
            {
                uint8_t level = app_battery_current_level();
                resData[0] = level;
                resLen = 1;
                TRACE(1,"battery_current = %d",level);
            }
            break;
        case OP_TOTA_MERIDIAN_EFFECT_CMD:
            resData[0] = app_meridian_eq(ptrParam[0]);
            break;
        case OP_TOTA_EQ_SELECT_CMD:
            break;
        case OP_TOTA_VOLUME_PLUS_CMD:
            {
                app_bt_volumeup();
                uint8_t level = app_bt_stream_local_volume_get();
                resData[0] = level;
                TRACE(1,"volume = %d",level);
            }
            break;
        case OP_TOTA_VOLUME_DEC_CMD:
            {
                app_bt_volumedown();
                uint8_t level = app_bt_stream_local_volume_get();
                resData[0] = level;
                resLen = 1;
                TRACE(1,"volume = %d",level);
            }
            break;
        case OP_TOTA_VOLUME_SET_CMD:
            {
                uint8_t scolevel = ptrParam[0];
                uint8_t a2dplevel = ptrParam[1];
                app_bt_set_volume(APP_BT_STREAM_HFP_PCM,scolevel);
                app_bt_set_volume(APP_BT_STREAM_A2DP_SBC,a2dplevel);
                btapp_hfp_report_speak_gain();
                btapp_a2dp_report_speak_gain();
            }
            break;
        case OP_TOTA_VOLUME_GET_CMD:
            {
                resData[0] = app_bt_stream_hfpvolume_get();
                resData[1] = app_bt_stream_a2dpvolume_get();
                resLen = 2;
            }
            break;
        case OP_TOTA_EQ_SET_CMD:
#if defined(TOTA_EQ_TUNING)
            resData[0] = hal_cmd_list_process(ptrParam);
            resLen = 1;
#else
            {
                int eq_index = ptrParam[0];
                if (eq_index == 3)
                    resData[0] = app_meridian_eq(true);
                else
                    resData[0] = app_audio_set_eq(eq_index);
                resLen = 1;
            }
#endif
            break;
        case OP_TOTA_EQ_GET_CMD:
#if defined(TOTA_EQ_TUNING) && defined(AUDIO_SECTION_ENABLE)
            uint8_t * pdata = (uint8_t *)audio_cfg_get_default_audio_section();
            if(NULL != pdata)
            {
                memcpy(resData, (uint8_t *)pdata, sizeof_audio_section());
				resLen = sizeof_audio_section();
            }
#else
            {
                resData[0] = app_audio_get_eq();
                resLen = 1;
            }
#endif
            break;
        case OP_TOTA_GET_SECINFO_EQ:
#if defined(TOTA_EQ_TUNING) && defined(AUDIO_SECTION_ENABLE)
            {
                APP_TOTA_AUDIO_EQ_SECTION_INFO_T secInfo;
                audio_cfg_get_eq_section_info(&secInfo->startAddr, &secInfo->totalSize, &secInfo->version);
			    memcpy(resData, (uint8_t *)(&secInfo), sizeof(APP_TOTA_AUDIO_EQ_SECTION_INFO_T));
		    	resLen = sizeof(APP_TOTA_AUDIO_EQ_SECTION_INFO_T);
            }
#endif
            break;
        case OP_TOTA_EQ_SET_DATA:
		    break;
        case OP_TOTA_FWVERSION_GET_CMD:
            {
                resData[0] = 0xA;
                resData[1] = 0XB;
                resData[2] = 0xC;
                resData[3] = 0XD;
                resLen = 4;
            }
            break;
        case OP_TOTA_RAW_DATA_SET_CMD:
            {
                app_ibrt_debug_parse(ptrParam, paramLen);
            }
            break;
        case OP_TOTA_RSSI_GET_CMD:
#if defined(IBRT)
            app_ibrt_rssi_get_stutter(resData, &resLen); //New interface to get stutter data
#endif
            break;
        case OP_TOTA_GET_DUMP_INFO_CMD:
            TOTA_DUMP_INFO_STRUCT_T dump_info;
            general_get_flash_dump_info(&dump_info);
            memcpy(resData, (uint8_t *)(&dump_info), sizeof(TOTA_DUMP_INFO_STRUCT_T));
            resLen = sizeof(TOTA_DUMP_INFO_STRUCT_T);
            break;
        case OP_TOTA_READ_FLASH_CMD:
            pflash_read_struct = (TOTA_READ_FLASH_STRUCT_T *)ptrParam;
            TRACE(2,"read flash: addr: %x, len: %d", pflash_read_struct->address, pflash_read_struct->length);
            pflashData = _read_flash_by_address(pflash_read_struct->address, pflash_read_struct->length);
            app_tota_send_response_to_command(funcCode,TOTA_NO_ERROR,
                (uint8_t*)pflashData, pflashData->length+6,app_tota_get_datapath());
            return;
        case OP_TOTA_READ_FINISH_CMD:
            TRACE(0,"spp dump log read done");
            return;
        case OP_TOTA_WRITE_FLASH_CMD:
            break;
        case OP_TOTA_ERASE_FLASH_CMD:
            break;
        default:
            TRACE(1,"wrong cmd 0x%x",funcCode);
            resData[0] = -1;
            return;
    }
    app_tota_send_response_to_command(funcCode,TOTA_NO_ERROR,resData,resLen,app_tota_get_datapath());
    return;
}

#if defined(IBRT) && defined(ANC_APP) && defined(DEBUG_ANC_BY_PHONE)
extern "C" bool app_anc_work_status(void);
extern "C" void app_anc_status_post_extend(uint32_t param0, uint32_t param1, uint32_t param2);
extern "C" void app_anc_status_sync_extend(uint32_t param0, uint32_t param1, uint32_t param2);
extern "C" int anc_set_gain(int32_t gain_ch_l, int32_t gain_ch_r,enum ANC_TYPE_T anc_type);

static void app_tota_ibrt_anc_cmd_handler(APP_TOTA_CMD_CODE_E funcCode, uint8_t* ptrParam, uint32_t paramLen)
{
    TOTA_LOG_DBG(3,"[%s] Func code 0x%x, param len %d", __func__, funcCode, paramLen);
    TOTA_LOG_DBG(1,"[%s] Param content:", __func__);
    DUMP8("%02x ", ptrParam, paramLen);
    
    uint8_t temp = 0;
    int32_t temp1 = 0;
    
    switch (funcCode)
    {
        case OP_TOTA_ANC_STATUS_SYNC_CMD:
        {
            TRACE(2,"[%s] OP_TOTA_ANC_STATUS_SYNC_CMD, param0=%d", __func__, ptrParam[0]);
            
            temp = !app_anc_work_status();
            app_anc_status_sync_extend(temp, (uint32_t)OP_TOTA_ANC_STATUS_SYNC_CMD, ptrParam[0]);
        }
        break;
        case OP_TOTA_ANC_ONOFF_WRITE_CMD:
        {
            TRACE(2,"[%s] OP_TOTA_ANC_ONOFF_WRITE_CMD param0=%d", __func__, ptrParam[0]);
            temp = !app_anc_work_status();
            app_anc_status_post_extend(temp, (uint32_t)OP_TOTA_ANC_ONOFF_WRITE_CMD, ptrParam[0]);
        }
        break;
        case OP_TOTA_ANC_LEVEL_WRITE_CMD:
        {
            TRACE(2,"[%s] OP_TOTA_ANC_LEVEL_WRITE_CMD param0=%d", __func__, ptrParam[0]);
            temp = !app_anc_work_status();
            app_anc_status_post_extend(temp, (uint32_t)OP_TOTA_ANC_LEVEL_WRITE_CMD, ptrParam[0]);
        }
        break;
        case OP_TOTA_ANC_TOTAL_GAIN_WRITE_CMD:
        {
            temp1 = ptrParam[0];
            temp1 = (temp1<<8)|ptrParam[1];
            temp1 = (temp1<<8)|ptrParam[2];
            temp1 = (temp1<<8)|ptrParam[3];
            
            TRACE(2,"[%s] OP_TOTA_ANC_TOTAL_GAIN_WRITE_CMD param0=%d", __func__, temp1);
            anc_set_gain(temp1, temp1, ANC_FEEDFORWARD);
        }
        break;
        
        default:TRACE(2,"[%s] wrong cmd 0x%x",__func__, funcCode);break;
    }
}
#endif

#if defined(BISTO_ENABLED)||defined(GFPS_ENABLED)

extern uint32_t __custom_parameter_start[];
extern uint32_t __custom_parameter_end[];
static TOTA_GENERAL_CMD_STRUCT_T g_msg;
static TOTA_SERVICE_GENERAL_CMD_PARSER_STATE g_parset;
static uint16_t g_running_cmd = TOTA_SERVICE_CMD_COUNT;
static TOTA_SERVICE_DATA_WRITE_STRUCT_T g_tota_data_write;
static TOTA_SERVICE_DATA_READ_STRUCT_T g_tota_data_read;
static uint32_t g_tota_data_write_buf[TOTA_SERVICE_DATA_WRITE_LENGTH/sizeof(uint32_t)];
static uint32_t g_tota_data_read_buf[TOTA_SERVICE_DATA_READ_LENGTH/sizeof(uint32_t)];

static void tota_flush_data_to_flash(uint8_t* ptrSource, uint32_t lengthToBurn, uint32_t offsetInFlashToProgram)
{
    uint32_t lock;
	uint32_t flash_sector_size_in_bytes = FLASH_SECTOR_SIZE_IN_BYTES;
    TRACE(2,"flush %d bytes to flash offset 0x%x", lengthToBurn, offsetInFlashToProgram);

    uint32_t preBytes = (flash_sector_size_in_bytes - (offsetInFlashToProgram%flash_sector_size_in_bytes))%flash_sector_size_in_bytes;
    if (lengthToBurn < preBytes)
    {
        preBytes = lengthToBurn;
    }
	TRACE(1,"preBytes = 0x%x",preBytes);

    uint32_t middleBytes = 0;
    if (lengthToBurn > preBytes)
    {
       middleBytes = (lengthToBurn - preBytes);
	   TRACE(1,"middleBytes = 0x%x",middleBytes);
	   middleBytes = middleBytes/flash_sector_size_in_bytes;

	   TRACE(1,"middleBytes = 0x%x",middleBytes);

	   middleBytes = middleBytes*flash_sector_size_in_bytes;

	   TRACE(1,"middleBytes = 0x%x",middleBytes);
//       middleBytes = ((lengthToBurn - preBytes)/FLASH_SECTOR_SIZE_IN_BYTES*FLASH_SECTOR_SIZE_IN_BYTES);
    }
    uint32_t postBytes = 0;
    if (lengthToBurn > (preBytes + middleBytes))
    {
        postBytes = (offsetInFlashToProgram + lengthToBurn)%flash_sector_size_in_bytes;
    }

    TRACE(3,"Prebytes is %d middlebytes is %d postbytes is %d", preBytes, middleBytes, postBytes);

    if (preBytes > 0)
    {
        lock=int_lock_global();
        pmu_flash_write_config();
        hal_norflash_write(HAL_FLASH_ID_0,(uint32_t)(offsetInFlashToProgram),(uint8_t *)ptrSource,preBytes);
        pmu_flash_read_config();
        int_unlock_global(lock);

        ptrSource += preBytes;
        offsetInFlashToProgram += preBytes;
    }

    uint32_t sectorIndexInFlash = offsetInFlashToProgram/flash_sector_size_in_bytes;

    if (middleBytes > 0)
    {
        uint32_t sectorCntToProgram = middleBytes/flash_sector_size_in_bytes;
        for (uint32_t sector = 0;sector < sectorCntToProgram;sector++)
        {

            lock=int_lock_global();
            pmu_flash_write_config();
            hal_norflash_erase(HAL_FLASH_ID_0,(uint32_t)(sectorIndexInFlash * flash_sector_size_in_bytes),flash_sector_size_in_bytes);
            hal_norflash_write(HAL_FLASH_ID_0,(uint32_t)(sectorIndexInFlash * flash_sector_size_in_bytes),(uint8_t *)(ptrSource + sector * flash_sector_size_in_bytes),flash_sector_size_in_bytes);
            pmu_flash_read_config();
            int_unlock_global(lock);

            sectorIndexInFlash++;
        }

        ptrSource += middleBytes;
    }

    if (postBytes > 0)
    {

        lock=int_lock_global();
        pmu_flash_write_config();
        hal_norflash_erase(HAL_FLASH_ID_0,(uint32_t)(sectorIndexInFlash * flash_sector_size_in_bytes),flash_sector_size_in_bytes);
        hal_norflash_write(HAL_FLASH_ID_0,(uint32_t)(sectorIndexInFlash * flash_sector_size_in_bytes),(uint8_t *)ptrSource,postBytes);
        pmu_flash_read_config();
        int_unlock_global(lock);
    }
}

static void app_tota_data_xfer_init()
{
    static bool tota_data_xfer_init = false;
    if(tota_data_xfer_init == false)
    {
        memset((uint8_t*)&g_tota_data_write,0,sizeof(g_tota_data_write));
        memset((uint8_t*)&g_tota_data_read,0,sizeof(g_tota_data_read));
        g_tota_data_write.tota_write_buff_ptr = (uint8_t*)g_tota_data_write_buf;
        g_tota_data_write.tota_data_write_buf_block_lenth = TOTA_SERVICE_DATA_WRITE_LENGTH;
        g_tota_data_read.tota_read_buff_ptr = (uint8_t*)g_tota_data_read_buf;
        g_tota_data_read.tota_data_read_buf_block_lenth = TOTA_SERVICE_DATA_READ_LENGTH;

        tota_data_xfer_init = true;
    }
}

static void app_tota_data_write_read_begin_addr_set(bool write_read,uint32_t address,uint32_t length)
{
    TRACE(4,"%s write_read = 0x%x adress = 0x%x length =0x%x",__func__,write_read,address,length);
    if(write_read == TOTA_SERVICE_DATA_XFER_ADDR_WRITE){
        g_tota_data_write.tota_data_need_write_flash_begin_addr = address;
        g_tota_data_write.tota_data_tota_need_write_lenth = length;
    }else{
        g_tota_data_read.tota_data_need_read_flash_begin_addr = address;
        g_tota_data_read.tota_data_tota_need_read_lenth = length;

    }
}


static void app_tota_general_cmd_reset(void)
{
	TRACE(1,"%s",__func__);
    memset((uint8_t*)&g_msg , 0 ,sizeof(g_msg));
    g_parset = TOTA_SERVICE_GENERAL_CMD_PARSER_NONE;
    g_running_cmd = TOTA_SERVICE_CMD_COUNT;
}

static void app_tota_tx_env_reset(void)
{
	TRACE(1,"%s",__func__);
    //app_tota_general_cmd_reset();
    app_tota_data_received_callback_handler_register(NULL);
    memset((uint8_t*)&g_tota_data_write,0,sizeof(g_tota_data_write));
    g_tota_data_write.tota_write_buff_ptr = (uint8_t*)g_tota_data_write_buf;
    g_tota_data_write.tota_data_write_buf_block_lenth = TOTA_SERVICE_DATA_WRITE_LENGTH;
}

#if 0
static void app_tota_rx_env_reset(void)
{
    app_tota_general_cmd_reset();
    memset((uint8_t*)&g_tota_data_read,0,sizeof(g_tota_data_read));
    g_tota_data_read.tota_read_buff_ptr = (uint8_t*)g_tota_data_read_buf;
    g_tota_data_read.tota_data_read_buf_block_lenth = TOTA_SERVICE_DATA_READ_LENGTH;

}
#endif
static void app_tota_service_data_receive_handler(uint8_t* ptrData, uint32_t dataLength)
{
    TRACE(1,"%s",__func__);
    if(g_parset < TOTA_SERVICE_GENERAL_CMD_STATE_PARSER_DATA)
        return;
    if((g_running_cmd == TOTA_SERVICE_CMD_WRITE_AI_ENV_DATA)||(g_running_cmd == TOTA_SERVICE_CMD_WRITE_FLASH))
    {
//        DUMP8("%02x ",ptrData,dataLength);
        uint32_t leftDataSize = dataLength;
        uint32_t offsetInReceivedRawData = 0;
        uint32_t bytesToCopy;
        uint32_t write_pos_in_flash = 0;
        if(g_tota_data_write.tota_data_tota_already_receive_length < g_tota_data_write.tota_data_tota_need_write_lenth){
            do
            {
                write_pos_in_flash = g_tota_data_write.tota_data_need_write_flash_begin_addr + g_tota_data_write.tota_data_written_in_flash_pos;
                // copy to data buffer
                if ((g_tota_data_write.tota_data_written_lenth_in_block + leftDataSize) >
                    g_tota_data_write.tota_data_write_buf_block_lenth)
                {
                    bytesToCopy = g_tota_data_write.tota_data_write_buf_block_lenth - g_tota_data_write.tota_data_written_lenth_in_block;
                }
                else
                {
                    bytesToCopy = leftDataSize;
                }

                leftDataSize -= bytesToCopy;
				TRACE(2,"length = 0x%x leftDataSize = 0x%x",dataLength,leftDataSize);
				TRACE(1,"tota_data_write_buf_block_lenth = 0x%x",g_tota_data_write.tota_data_write_buf_block_lenth);
            	TRACE(3,"before memcpy lengthInblock = 0x%x offsetInReceivedRawData = 0x%x Copy = 0x%x",g_tota_data_write.tota_data_written_lenth_in_block,
					offsetInReceivedRawData,bytesToCopy);
				osDelay(10);
                memcpy(&g_tota_data_write.tota_write_buff_ptr[g_tota_data_write.tota_data_written_lenth_in_block],
                        &ptrData[offsetInReceivedRawData], bytesToCopy);
                offsetInReceivedRawData += bytesToCopy;
                g_tota_data_write.tota_data_written_lenth_in_block += bytesToCopy;
                TRACE(1,"tota_data_written_lenth_in_block is %d", g_tota_data_write.tota_data_written_lenth_in_block);
				osDelay(10);

                if(g_tota_data_write.tota_data_tota_already_receive_length +  dataLength >= g_tota_data_write.tota_data_tota_need_write_lenth){
                    TRACE(0,"the last packet to flush");
                    tota_flush_data_to_flash(g_tota_data_write.tota_write_buff_ptr, g_tota_data_write.tota_data_tota_need_write_lenth - g_tota_data_write.tota_data_tota_already_receive_length,
                                            write_pos_in_flash);
                    g_tota_data_write.tota_data_written_in_flash_pos += g_tota_data_write.tota_data_tota_need_write_lenth - g_tota_data_write.tota_data_tota_already_receive_length;
                    g_tota_data_write.tota_data_written_lenth_in_block = 0;
                    break;
                }else if (g_tota_data_write.tota_data_write_buf_block_lenth == g_tota_data_write.tota_data_written_lenth_in_block){
                    TRACE(0,"Program the image to flash.");
                    tota_flush_data_to_flash(g_tota_data_write.tota_write_buff_ptr, TOTA_DATA_BUFFER_SIZE_FOR_BURNING,
                        write_pos_in_flash);
                    g_tota_data_write.tota_data_written_in_flash_pos += TOTA_DATA_BUFFER_SIZE_FOR_BURNING;
                    g_tota_data_write.tota_data_written_lenth_in_block = 0;
                }
            } while (offsetInReceivedRawData < dataLength);

            if(g_tota_data_write.tota_data_tota_need_write_lenth == g_tota_data_write.tota_data_written_in_flash_pos){
                g_tota_data_write.tota_data_tota_already_receive_length += g_tota_data_write.tota_data_tota_need_write_lenth - g_tota_data_write.tota_data_tota_already_receive_length;
            }else{
                g_tota_data_write.tota_data_tota_already_receive_length += dataLength;
            }
        }

        TRACE(1,"Image already received %d", g_tota_data_write.tota_data_tota_already_receive_length);
    }
}

#if 0
static uint32_t start_addr = 0;
static uint32_t check_length = 0;
#define CHECK_LINE_NUM  4096

static uint32_t check_tmp_buf[CHECK_LINE_NUM / sizeof(uint32_t)];
void app_flash_flush_data_check_crc(void)
{
	uint32_t i =0;
	uint32_t enum_lenth = check_length / sizeof(uint32_t);
	uint32_t check_crc = 0;

	for(;i<enum_lenth;i++){
		check_tmp_buf[i] = 0x55555555;
	}
	check_crc = crc32(0,(uint8_t*)check_tmp_buf,check_length);
	TRACE(1,"check_crc = 0x%x",check_crc);
}
#endif

static APP_TOTA_CMD_RET_STATUS_E app_tota_service_receive_data_verify(APP_TOTA_CMD_CODE_E funcCode,uint8_t* ptrData, uint32_t dataLength)
{
    TOTA_SEGMENT_VERIFY_T* ptVerifyCmd = (TOTA_SEGMENT_VERIFY_T *)(ptrData);
    APP_TOTA_CMD_RET_STATUS_E rsp_status;

    uint32_t flash_crc32 = 0;
    uint32_t startFlashAddr = g_tota_data_write.tota_data_need_write_flash_begin_addr + g_tota_data_write.tota_offset_of_data_of_current_block;
    uint32_t lengthToDoCrcCheck = g_tota_data_write.tota_data_tota_already_receive_length-g_tota_data_write.tota_offset_of_data_of_current_block;

	uint32_t write_pos_in_flash = 0;
	if(g_tota_data_write.tota_data_written_in_flash_pos < g_tota_data_write.tota_data_tota_need_write_lenth){
		write_pos_in_flash = g_tota_data_write.tota_data_need_write_flash_begin_addr + g_tota_data_write.tota_data_written_in_flash_pos;

		tota_flush_data_to_flash(g_tota_data_write.tota_write_buff_ptr, g_tota_data_write.tota_data_tota_already_receive_length,
							write_pos_in_flash);
		g_tota_data_write.tota_data_written_lenth_in_block = 0;
		g_tota_data_write.tota_data_written_in_flash_pos += g_tota_data_write.tota_data_tota_already_receive_length;
	}

    TRACE(0,"Calculate the crc32 of the segment.");

    flash_crc32 = crc32(0, (uint8_t *)(startFlashAddr), lengthToDoCrcCheck);
    TRACE(1,"CRC32 of the segement is 0x%x", flash_crc32);
    TRACE(2,"startFlashAddr =0x%x  lengthToDoCrcCheck =0x%x",startFlashAddr,lengthToDoCrcCheck);
	//start_addr = startFlashAddr;
	//check_length = lengthToDoCrcCheck;

    if ((TOTA_SERVICE_SYNC_WORD == ptVerifyCmd->magicCode) &&
        (ptVerifyCmd->crc32OfSegment == flash_crc32))
    {
        rsp_status = TOTA_NO_ERROR;
        // backup of the information in case the verification of current segment failed
        g_tota_data_write.tota_offset_in_flash_of_current_block = g_tota_data_write.tota_data_written_in_flash_pos;
        g_tota_data_write.tota_offset_of_data_of_current_block = g_tota_data_write.tota_data_tota_already_receive_length;
    }
    else
    {
        // restore the offset
        g_tota_data_write.tota_data_written_in_flash_pos = g_tota_data_write.tota_offset_in_flash_of_current_block;
        g_tota_data_write.tota_data_tota_already_receive_length = g_tota_data_write.tota_offset_of_data_of_current_block;
        rsp_status = TOTA_INVALID_DATA_PACKET;
		//app_flash_flush_data_check_crc();
    }

    if(g_tota_data_write.tota_offset_of_data_of_current_block == g_tota_data_write.tota_data_tota_need_write_lenth){
        app_tota_tx_env_reset();
    }

    // reset the data buffer
    TRACE(2,"total size is %d already received %d", g_tota_data_write.tota_data_tota_need_write_lenth,
        g_tota_data_write.tota_data_tota_already_receive_length);
    return rsp_status;
}

static TOTA_SERVICE_SEND_STATUS_E app_tota_service_read_flash_env_set(uint8_t* pData)
{
    TOTA_SERVICE_SEND_STATUS_E nRet = TOTA_SERVICE_SEND_STATUS_NO_ERROR;
    TOTA_SERVICE_CMD_READ_AI_ENV_DATA_BODY_STRUCT_T * read_flash = (TOTA_SERVICE_CMD_READ_AI_ENV_DATA_BODY_STRUCT_T *)pData;

    if (read_flash->is_to_start)
    {
        TRACE(1,"is_to_start =0X%x",read_flash->is_to_start);
        TRACE(2,"Getreading flash content request start addr 0x%x size %d",
                    read_flash->start_addr, read_flash->len);

        // check the sanity of the request
        TRACE(1,"__custom_parameter_end =0X%x",(uint32_t)__custom_parameter_end);
        if ((read_flash->start_addr >= (uint32_t)__custom_parameter_end) ||
            ((read_flash->start_addr + read_flash->len) >
            (uint32_t)__custom_parameter_end))
        {
            nRet = TOTA_SERVICE_SEND_STATUS_INVALID_CMD;
        }
        else
        {
            g_tota_data_read.tota_data_need_read_flash_begin_addr = read_flash->start_addr;
            g_tota_data_read.tota_data_tota_need_read_lenth = read_flash->len;
            if(read_flash->start_addr == (uint32_t)__custom_parameter_start)
            {
                g_tota_data_read.tota_data_have_read_in_flash_pos = 0;
                g_tota_data_read.tota_data_tota_already_sent_length = 0;
            }
            TRACE(1,"%s:", __func__);
            DUMP8("0x%02x ", (uint8_t*)g_tota_data_read.tota_data_need_read_flash_begin_addr, 32);
            TRACE(2,"Start sending flash content start addr 0x%x size %d",
                read_flash->start_addr, read_flash->len);
        }
    }
    else
    {
        TRACE(0,"Get stop reading flash content request.");
        g_tota_data_read.tota_data_tota_need_read_lenth = 0;

    }
    return nRet;
}

static void app_tota_service_data_packet_fill_in(uint8_t* send_data,uint16_t send_data_length)
{
    uint16_t packet_size = 0;
    uint32_t read_addr_offset = 0;

    TRACE(2,"have_read_in_flash_pos=0x%x, need_read_lenth=0x%x", g_tota_data_read.tota_data_have_read_in_flash_pos, 
        g_tota_data_read.tota_data_tota_need_read_lenth);

    packet_size = (g_tota_data_read.tota_data_tota_need_read_lenth > TOTA_SERVICE_XFER_OUT_DATA_SIZE)?(TOTA_SERVICE_XFER_OUT_DATA_SIZE):(g_tota_data_read.tota_data_tota_need_read_lenth);
    read_addr_offset = g_tota_data_read.tota_data_need_read_flash_begin_addr;
    memcpy(g_tota_data_read.tota_read_buff_ptr,(uint8_t*)read_addr_offset,packet_size);
    g_tota_data_read.tota_data_have_read_in_flash_pos += packet_size;
}

static void app_tota_service_data_send(uint8_t* pData,uint16_t length)
{
    APP_TOTA_TRANSMISSION_PATH_E dataPath = app_tota_get_datapath();
    TRACE(3,"dataPath=%d, already_sent_length=0x%x, need_read_lenth=0x%x", dataPath, g_tota_data_read.tota_data_tota_already_sent_length, 
        g_tota_data_read.tota_data_tota_need_read_lenth);

    TRACE(2,"tota_read_buff_ptr=0x%x, have_read_in_flash_pos=0x%x", (uint32_t)g_tota_data_read.tota_read_buff_ptr, 
        g_tota_data_read.tota_data_have_read_in_flash_pos);
    app_tota_send_data(dataPath,g_tota_data_read.tota_read_buff_ptr,g_tota_data_read.tota_data_tota_need_read_lenth);
    g_tota_data_read.tota_data_tota_already_sent_length = g_tota_data_read.tota_data_have_read_in_flash_pos;
}

static TOTA_SERVICE_SEND_STATUS_E app_tota_service_data_xfer_out_handler(uint8_t* pData)
{
    TOTA_SERVICE_SEND_STATUS_E nRet = TOTA_SERVICE_SEND_STATUS_NO_ERROR;
    nRet = app_tota_service_read_flash_env_set(pData);
    TRACE(2,"%s nRet = %d",__func__,nRet);
    if(nRet == TOTA_SERVICE_SEND_STATUS_NO_ERROR){
        app_tota_service_data_packet_fill_in(NULL,0);
        app_tota_service_data_send(NULL,0);
    }else{

    }

    return nRet;
}

#ifdef FIRMWARE_REV
extern "C" void system_get_info(uint8_t *fw_rev_0, uint8_t *fw_rev_1,
    uint8_t *fw_rev_2, uint8_t *fw_rev_3);

#endif


static int app_tota_general_cmd_parser(APP_TOTA_CMD_CODE_E funcCode,uint8_t* ptrParam, uint32_t paramLen)
{
    APP_TOTA_CMD_RET_STATUS_E status = TOTA_NO_ERROR;
    APP_TOTA_CMD_RET_STATUS_E rsp_status;
    TOTA_SERVICE_SEND_STATUS_E nRet = TOTA_SERVICE_SEND_STATUS_NO_ERROR;
    APP_TOTA_TRANSMISSION_PATH_E dataPath = app_tota_get_datapath();
    uint8_t cmd_rsp_data_buf[TOTA_SERVICE_CMD_PARAM_LENGTH] = {0};
    uint8_t cmd_rsp_data_len = 0;
	uint16_t cmd = 0;
    uint8_t read_count = 0;
    msg_header_t *hdr;
	TRACE(3,"%s %d funcCode = 0x%x", __func__, __LINE__,funcCode);
    if(paramLen > sizeof(g_msg)){
        status = TOTA_PARAM_LEN_OUT_OF_RANGE;
        goto error;
    }
    memcpy((uint8_t*)&g_msg,ptrParam,paramLen);

    hdr = (msg_header_t *)&g_msg.hdr;
    if(hdr->prefix != TOTA_SERVICE_SYNC_WORD){
        status = TOTA_INVALID_CMD;
        goto error;;
    }

    if((hdr->cmd <TOTA_SERVICE_INIT_SYNC) && (hdr->cmd > TOTA_SERVICE_CMD_COUNT)){
        if(g_parset < TOTA_SERVICE_GENERAL_CMD_STATE_PARSER_SYNC){
            status = TOTA_INVALID_CMD;
            goto error;;
        }
    }
    TRACE(1,"hdr->cmd = 0x%x",hdr->cmd);
	cmd = g_running_cmd;
    g_running_cmd = hdr->cmd;
    app_tota_data_xfer_init();
    switch(hdr->cmd)
    {
        case TOTA_SERVICE_INIT_SYNC:
                if(hdr->len != 0x02){
                    /*wrong error length*/
                    status = TOTA_INVALID_CMD;
                    goto error;
                }else{
                    if((g_msg.cmd_param[0] == TOTA_SERVICE_SYNC_DATA_1) && (g_msg.cmd_param[1] == TOTA_SERVICE_SYNC_DATA_2)){
                        g_parset = TOTA_SERVICE_GENERAL_CMD_STATE_PARSER_SYNC;
                        app_tota_tx_env_reset();
                        app_tota_general_cmd_reset();
                        TOTA_SERVICE_CMD_SYNC_ACK *sync_ack = (TOTA_SERVICE_CMD_SYNC_ACK *)cmd_rsp_data_buf;
                        sync_ack->cmd_rsp_header.cmd = hdr->cmd;
                        sync_ack->cmd_rsp_header.len = 0x01;
                        sync_ack->ack_val = TOTA_NO_ERROR;
                        cmd_rsp_data_len = sizeof(TOTA_SERVICE_CMD_SYNC_ACK);
                    }
                }
            break;
        case TOTA_SERVICE_CMD_WRITE_FLASH:
            TRACE(0,"currently do not support directly write flash");
            break;
        case TOTA_SERVICE_CMD_READ_FLASH:
            TRACE(0,"currently do not support directly read flash");
            break;
        case TOTA_SERVICE_CMD_WRITE_AI_ENV_DATA:
        	{
                TRACE(0,"write ai env data");
                if(hdr->len !=4 ){
                    status = TOTA_INVALID_CMD;
                    goto error;
                }
    			if(g_msg.cmd_param[0] > (__custom_parameter_end - __custom_parameter_start)){
    			    status = TOTA_PARAM_LEN_OUT_OF_RANGE;
                    goto error;
    			}
                g_parset = TOTA_SERVICE_GENERAL_CMD_STATE_PARSER_DATA;
                app_tota_data_received_callback_handler_register(app_tota_service_data_receive_handler);
                app_tota_data_write_read_begin_addr_set(TOTA_SERVICE_DATA_XFER_ADDR_WRITE,(uint32_t)__custom_parameter_start,*(uint32_t*)&g_msg.cmd_param[0]);
                TOTA_SERVICE_CMD_WRITE_AI_ENV_DATA_ACK_STRUCT_T *write_ai_ack = (TOTA_SERVICE_CMD_WRITE_AI_ENV_DATA_ACK_STRUCT_T *)cmd_rsp_data_buf;
                write_ai_ack->cmd_rsp_header.cmd = hdr->cmd;
                write_ai_ack->cmd_rsp_header.len = 0x01;
                write_ai_ack->write_ai_env_ack = TOTA_NO_ERROR;
                cmd_rsp_data_len = sizeof(TOTA_SERVICE_CMD_WRITE_AI_ENV_DATA_ACK_STRUCT_T);
    	    }
            break;
        case TOTA_SERVICE_CMD_VERIFY_AI_ENV_DATA:
        	{
                if(hdr->len !=8 ){
                    status = TOTA_INVALID_CMD;
                    goto error;
                }
                g_parset = TOTA_SERVICE_GENERAL_CMD_STATE_PARSER_DATA;
    			if(cmd == TOTA_SERVICE_CMD_WRITE_AI_ENV_DATA)
    				g_running_cmd = cmd;
                status = app_tota_service_receive_data_verify(funcCode,(uint8_t*)&g_msg.cmd_param[0],g_msg.hdr.len);
                TOTA_SERVICE_CMD_VERIFY_AI_ENV_DATA_ACK_STRUCT_T *verify_ai_ack = (TOTA_SERVICE_CMD_VERIFY_AI_ENV_DATA_ACK_STRUCT_T *)cmd_rsp_data_buf;
    			verify_ai_ack->cmd_rsp_header.cmd = hdr->cmd;
                verify_ai_ack->cmd_rsp_header.len = 0x01;
                verify_ai_ack->verify_ai_env_ack = status;
                cmd_rsp_data_len = sizeof(TOTA_SERVICE_CMD_VERIFY_AI_ENV_DATA_ACK_STRUCT_T);
    			app_tota_general_cmd_reset();
        	}
            break;
        case TOTA_SERVICE_CMD_READ_AI_ENV_DATA:
        	{
            if(hdr->len !=5 ){
                status = TOTA_INVALID_CMD;
                goto error;
            }

            g_parset = TOTA_SERVICE_GENERAL_CMD_STATE_PARSER_DATA;
            TOTA_SERVICE_CMD_READ_AI_ENV_DATA_BODY_STRUCT_T *read_env = (TOTA_SERVICE_CMD_READ_AI_ENV_DATA_BODY_STRUCT_T *)cmd_rsp_data_buf;

            read_env->is_to_start = g_msg.cmd_param[0];
            read_count = read_env->is_to_start;
            memcpy(&(read_env->len), (uint32_t*)(&g_msg.cmd_param[1]), 4);
            read_env->start_addr = (uint32_t)(__custom_parameter_start + (read_count-1)*DANGLE_TRANS_MAX_SIZE/4);
			cmd_rsp_data_len = sizeof(TOTA_SERVICE_CMD_READ_AI_ENV_DATA_BODY_STRUCT_T);               
			nRet = app_tota_service_data_xfer_out_handler((uint8_t*)read_env);
            
            if(nRet == TOTA_SERVICE_SEND_STATUS_NO_ERROR){
                TRACE(1,"succussfully send out %d times.", read_count);
                status =TOTA_NO_ERROR;
            }
			else{
                status =TOTA_INVALID_CMD;
			}
			read_env->read_ai_env_ack = status;
        	}
            break;
        case TOTA_SERVICE_CMD_READ_ACK:
            TRACE(0,"currently do not support read ack");
            break;
        case TOTA_SERVICE_CMD_GET_BAT:
            {
                uint16_t bat_val = 0;
                app_battery_get_info(&bat_val,NULL,NULL);
				TRACE(1,"bat_val = %04x",bat_val);
                TOTA_SERVICE_CMD_GET_BAT_VAL_BODY_STRUCT_T *bat_body_ptr = (TOTA_SERVICE_CMD_GET_BAT_VAL_BODY_STRUCT_T *)cmd_rsp_data_buf;
                bat_body_ptr->bat_val = bat_val;
                bat_body_ptr->cmd_rsp_header.cmd = hdr->cmd;
                bat_body_ptr->cmd_rsp_header.len = sizeof(BAT_VAL_BYTES);
                cmd_rsp_data_len = sizeof(TOTA_SERVICE_CMD_GET_BAT_VAL_BODY_STRUCT_T);
            }
            break;
        case TOTA_SERVICE_CMD_GET_FW_VERSION:
        	{
#ifdef FIRMWARE_REV
                system_get_info(&cmd_rsp_data_buf[0],&cmd_rsp_data_buf[1],&cmd_rsp_data_buf[2],&cmd_rsp_data_buf[3]);
    			TRACE(4,"firmware version = %d.%d.%d.%d",cmd_rsp_data_buf[0],cmd_rsp_data_buf[1],cmd_rsp_data_buf[2],cmd_rsp_data_buf[3]);
#endif
    			TOTA_SERVICE_CMD_GET_FW_VERSION_BODY_STRUCT_T *fw_ptr = (TOTA_SERVICE_CMD_GET_FW_VERSION_BODY_STRUCT_T *)cmd_rsp_data_buf;
    			fw_ptr->fw_version[0] = cmd_rsp_data_buf[0];
    			fw_ptr->fw_version[1] = cmd_rsp_data_buf[1];
    			fw_ptr->fw_version[2] = cmd_rsp_data_buf[2];
    			fw_ptr->fw_version[3] = cmd_rsp_data_buf[3];
    			fw_ptr->cmd_rsp_header.cmd = hdr->cmd;
    			fw_ptr->cmd_rsp_header.len = 4*sizeof(uint8_t);
                cmd_rsp_data_len = sizeof(TOTA_SERVICE_CMD_GET_FW_VERSION_BODY_STRUCT_T);
        	}
            break;
        case TOTA_SERVICE_CMD_GET_BT_LOCAL_NAME:
        	{
    			TOTA_SERVICE_CMD_GET_BT_LOCAL_NAME_BODY_STRUCT_T *bt_local_name_ptr = (TOTA_SERVICE_CMD_GET_BT_LOCAL_NAME_BODY_STRUCT_T *)cmd_rsp_data_buf;
    			uint8_t name[BT_LOCAL_NAME_LEN] = {0};
    			uint8_t* factory_bt_name_ptr =factory_section_get_bt_name();
    			uint16_t valid_len =0;
    			if(factory_bt_name_ptr != NULL){
    				valid_len = (strlen((char*)factory_bt_name_ptr)> sizeof(name))?(sizeof(name)):strlen((char*)factory_bt_name_ptr);
    				memcpy(name,factory_bt_name_ptr,valid_len);
    			}
    			memcpy(bt_local_name_ptr->bt_local_name,name,valid_len);
    			bt_local_name_ptr->cmd_rsp_header.cmd = hdr->cmd;
    			bt_local_name_ptr->cmd_rsp_header.len = valid_len;
                cmd_rsp_data_len = sizeof(TOTA_SERVICE_CMD_RSP_HEADER_T) +valid_len ;
        	}
		    break;
		case TOTA_SERVICE_CMD_GET_BT_ADDR:
        	{
     			TOTA_SERVICE_CMD_GET_BT_ADDR_BODY_STRUCT_T *bt_addr_ptr = (TOTA_SERVICE_CMD_GET_BT_ADDR_BODY_STRUCT_T *)cmd_rsp_data_buf;
     			uint8_t bt_addr[6] = {0};
     			uint8_t* factory_bt_addr_ptr =factory_section_get_bt_address();
     		    memcpy(bt_addr,factory_bt_addr_ptr,6);
     			memcpy(bt_addr_ptr->bt_addr,bt_addr,6);
     			bt_addr_ptr->cmd_rsp_header.cmd = hdr->cmd;
     			bt_addr_ptr->cmd_rsp_header.len = sizeof(bt_addr);
                cmd_rsp_data_len = sizeof(TOTA_SERVICE_CMD_GET_BT_ADDR_BODY_STRUCT_T);
        	}
		    break;
        case TOTA_SERVICE_CMD_CLEAR_PAIRING_INFO:
            {
                TRACE(0,"clear all pairing info.");
            }
            break;
        case TOTA_SERVICE_CMD_GENERAL_TEST:
            {
                TRACE(0,"general test.");
            }
            break;
        case TOTA_SERVICE_CMD_SHUTDOWM:
            {
                TRACE(0,"shutdown");
                app_shutdown();
            }
            break;
        case TOTA_SERVICE_CMD_MIC_TEST_ON:
            TRACE(0,"#####mic test on:");
#ifdef __FACTORY_MODE_SUPPORT__
            app_audio_sendrequest(APP_BT_STREAM_INVALID, (uint8_t)APP_BT_SETTING_CLOSEALL, 0);
            app_audio_sendrequest(APP_FACTORYMODE_AUDIO_LOOP, (uint8_t)APP_BT_SETTING_OPEN, 0);
#endif
            break;
        case TOTA_SERVICE_CMD_MIC_TEST_OFF:
            TRACE(0,"#####mic test off:");
#ifdef __FACTORY_MODE_SUPPORT__
            app_audio_sendrequest(APP_FACTORYMODE_AUDIO_LOOP, (uint8_t)APP_BT_SETTING_CLOSE, 0);
#endif
            break;
        case TOTA_SERVICE_CMD_MIC_SWITCH:
            TRACE(0,"####mic switch test");
            break;
        default:
            break;
    }

    app_tota_send_command(funcCode, (uint8_t *)&cmd_rsp_data_buf, cmd_rsp_data_len, dataPath);
    return status;

error:
    rsp_status = status;
    app_tota_send_command(funcCode, (uint8_t *)&rsp_status, sizeof(rsp_status), dataPath);
    return status;
}



static void app_tota_general_cmd_handler(APP_TOTA_CMD_CODE_E funcCode, uint8_t* ptrParam, uint32_t paramLen)
{
    app_tota_general_cmd_parser(funcCode,ptrParam,paramLen);
}
#endif


TOTA_COMMAND_TO_ADD(OP_TOTA_BATTERY_STATUS_CMD, app_tota_vendor_cmd_handler, false, 0, NULL );
TOTA_COMMAND_TO_ADD(OP_TOTA_MERIDIAN_EFFECT_CMD, app_tota_vendor_cmd_handler, false, 0, NULL );
TOTA_COMMAND_TO_ADD(OP_TOTA_EQ_SELECT_CMD, app_tota_vendor_cmd_handler, false, 0, NULL );
TOTA_COMMAND_TO_ADD(OP_TOTA_VOLUME_PLUS_CMD, app_tota_vendor_cmd_handler, false, 0, NULL );
TOTA_COMMAND_TO_ADD(OP_TOTA_VOLUME_DEC_CMD, app_tota_vendor_cmd_handler, false, 0, NULL );
TOTA_COMMAND_TO_ADD(OP_TOTA_VOLUME_SET_CMD, app_tota_vendor_cmd_handler, false, 0, NULL );
TOTA_COMMAND_TO_ADD(OP_TOTA_VOLUME_GET_CMD, app_tota_vendor_cmd_handler, false, 0, NULL );
TOTA_COMMAND_TO_ADD(OP_TOTA_EQ_SET_CMD, app_tota_vendor_cmd_handler, false, 0, NULL );
TOTA_COMMAND_TO_ADD(OP_TOTA_EQ_GET_CMD, app_tota_vendor_cmd_handler, false, 0, NULL );
TOTA_COMMAND_TO_ADD(OP_TOTA_FWVERSION_GET_CMD, app_tota_vendor_cmd_handler, false, 0, NULL );
TOTA_COMMAND_TO_ADD(OP_TOTA_RSSI_GET_CMD, app_tota_vendor_cmd_handler, false, 0, NULL );
TOTA_COMMAND_TO_ADD(OP_TOTA_GET_DUMP_INFO_CMD, app_tota_vendor_cmd_handler, false, 0, NULL );
TOTA_COMMAND_TO_ADD(OP_TOTA_READ_FLASH_CMD, app_tota_vendor_cmd_handler, false, 0, NULL );
TOTA_COMMAND_TO_ADD(OP_TOTA_WRITE_FLASH_CMD, app_tota_vendor_cmd_handler, false, 0, NULL );
TOTA_COMMAND_TO_ADD(OP_TOTA_READ_FINISH_CMD, app_tota_vendor_cmd_handler, false, 0, NULL );
TOTA_COMMAND_TO_ADD(OP_TOTA_ERASE_FLASH_CMD, app_tota_vendor_cmd_handler, false, 0, NULL );

#if defined(IBRT) && defined(ANC_APP) && defined(DEBUG_ANC_BY_PHONE)
TOTA_COMMAND_TO_ADD(OP_TOTA_ANC_STATUS_SYNC_CMD, app_tota_ibrt_anc_cmd_handler, false, 0, NULL );
TOTA_COMMAND_TO_ADD(OP_TOTA_ANC_ONOFF_WRITE_CMD, app_tota_ibrt_anc_cmd_handler, false, 0, NULL );
TOTA_COMMAND_TO_ADD(OP_TOTA_ANC_LEVEL_WRITE_CMD, app_tota_ibrt_anc_cmd_handler, false, 0, NULL );
TOTA_COMMAND_TO_ADD(OP_TOTA_ANC_STATUS_REPORT_CMD, app_tota_ibrt_anc_cmd_handler, false, 0, NULL );
TOTA_COMMAND_TO_ADD(OP_TOTA_ANC_TOTAL_GAIN_WRITE_CMD, app_tota_ibrt_anc_cmd_handler, false, 0, NULL );
#endif
#if defined(BISTO_ENABLED)||defined(GFPS_ENABLED)
TOTA_COMMAND_TO_ADD(OP_TOTA_GENERAL_CMD, app_tota_general_cmd_handler, false, 0, NULL );
#endif
TOTA_COMMAND_TO_ADD(OP_TOTA_RAW_DATA_SET_CMD, app_tota_vendor_cmd_handler, false, 0, NULL );

TOTA_COMMAND_TO_ADD(OP_TOTA_EQ_SET_DATA, app_tota_vendor_cmd_handler, false, 0, NULL );
TOTA_COMMAND_TO_ADD(OP_TOTA_GET_SECINFO_EQ, app_tota_vendor_cmd_handler, false, 0, NULL );

