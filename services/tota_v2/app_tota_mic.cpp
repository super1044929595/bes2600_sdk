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

#include "tota_stream_data_transfer.h"
#include "app_tota_cmd_code.h"
#include "app_bt_stream.h"
#include "app_audio.h"
#include "app_tota.h"
#include "app_tota_cmd_handler.h"
#include "app_tota_mic.h"
#include "cmsis_os.h"
typedef struct{
    uint8_t total_mic_channel_num;
    uint8_t under_test_mic_channel;
    uint8_t bypass_algo;
}__attribute__ ((__packed__))TOTA_MIC_TST_CHANNEL_STRUCT_T;

extern int speech_set_mic_ch(uint16_t mic_map);

/*-----------------------------------------------------------------------------*/
static void _mic_cmd_handle(APP_TOTA_CMD_CODE_E funcCode, uint8_t* ptrParam, uint32_t paramLen)
{
    TOTA_LOG_DBG(3,"%s Func code 0x%x, param len %d", __func__, funcCode, paramLen);
    TOTA_LOG_DBG(0,"Param content:");
    DUMP8("%02x ", ptrParam, paramLen);
    APP_TOTA_CMD_RET_STATUS_E rsp_status = TOTA_NO_ERROR;
    uint8_t resData[48]={0};
    uint32_t resLen=1;
    switch (funcCode)
    {
        case OP_TOTA_MIC_TEST_ON:
            TRACE(0,"#####mic test on:");
#ifdef __FACTORY_MODE_SUPPORT__
            app_audio_sendrequest(APP_BT_STREAM_INVALID, (uint8_t)APP_BT_SETTING_CLOSEALL, 0);
            app_audio_sendrequest(APP_FACTORYMODE_AUDIO_LOOP, (uint8_t)APP_BT_SETTING_OPEN, 0);
#endif
            break;
        case OP_TOTA_MIC_TEST_OFF:
            TRACE(0,"#####mic test off:");
#ifdef __FACTORY_MODE_SUPPORT__
            app_audio_sendrequest(APP_FACTORYMODE_AUDIO_LOOP, (uint8_t)APP_BT_SETTING_CLOSE, 0);
#endif
            break;
        case OP_TOTA_MIC_SWITCH:
            {
                TRACE(0,"####mic switch test");
                //TOTA_MIC_TST_CHANNEL_STRUCT_T *mic_switch_tst = (TOTA_MIC_TST_CHANNEL_STRUCT_T *)ptrParam;
                //app_audio_sendrequest(APP_BT_STREAM_INVALID, (uint8_t)APP_BT_SETTING_CLOSEALL, 0);
                //asume bytes sequence:
                //byte 0:    mic open channel num;
                //byte 1:    mic channel need to do freq-rsp test
                //byte 2:    mic channel need bypass algo
                //speech_set_mic_ch(ptrParam[1]);
            }
            break;
        case OP_TOTA_MIC_FULL_DUT_TEST_CLOSE:
            TRACE(0,"####mic switch close test");
            app_audio_sendrequest(APP_BT_STREAM_INVALID, (uint8_t)APP_BT_SETTING_CLOSEALL, 0);
            break;
        default:
            break;
    }
    app_tota_send_rsp(funcCode,rsp_status,resData,resLen);
}

TOTA_COMMAND_TO_ADD(OP_TOTA_MIC_TEST_ON, _mic_cmd_handle, false, 0, NULL );
TOTA_COMMAND_TO_ADD(OP_TOTA_MIC_TEST_OFF, _mic_cmd_handle, false, 0, NULL );
TOTA_COMMAND_TO_ADD(OP_TOTA_MIC_SWITCH, _mic_cmd_handle, false, 0, NULL );
