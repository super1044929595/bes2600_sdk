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
#include "string.h"
#include "app_tws_ibrt_trace.h"
#include "app_tws_ctrl_thread.h"
#include "app_tws_ibrt_cmd_handler.h"
#include "app_ibrt_ota_cmd.h"

#include "ota_control.h"
#include "ota_spp.h"

#ifdef BES_OTA_BASIC

static void app_ibrt_ota_get_version_cmd_send(uint8_t *p_buff, uint16_t length);
void app_ibrt_ota_get_version_cmd_send_handler(uint16_t rsp_seq, uint8_t *p_buff, uint16_t length);
static void app_ibrt_ota_get_version_cmd_send_rsp_handler(uint16_t rsp_seq, uint8_t *p_buff, uint16_t length);
static void app_ibrt_ota_get_version_cmd_send_rsp_timeout_handler(uint16_t rsp_seq, uint8_t *p_buff, uint16_t length);

static void app_ibrt_ota_select_side_cmd_send(uint8_t *p_buff, uint16_t length);
void app_ibrt_ota_select_side_cmd_send_handler(uint16_t rsp_seq, uint8_t *p_buff, uint16_t length);
static void app_ibrt_ota_select_side_cmd_send_rsp_handler(uint16_t rsp_seq, uint8_t *p_buff, uint16_t length);
static void app_ibrt_ota_select_side_cmd_send_rsp_timeout_handler(uint16_t rsp_seq, uint8_t *p_buff, uint16_t length);

static void app_ibrt_ota_bp_check_cmd_send(uint8_t *p_buff, uint16_t length);
void app_ibrt_ota_bp_check_cmd_send_handler(uint16_t rsp_seq, uint8_t *p_buff, uint16_t length);
static void app_ibrt_ota_bp_check_cmd_send_rsp_handler(uint16_t rsp_seq, uint8_t *p_buff, uint16_t length);
static void app_ibrt_ota_bp_check_cmd_send_rsp_timeout_handler(uint16_t rsp_seq, uint8_t *p_buff, uint16_t length);

static void app_ibrt_ota_start_cmd_send(uint8_t *p_buff, uint16_t length);
void app_ibrt_ota_start_cmd_send_handler(uint16_t rsp_seq, uint8_t *p_buff, uint16_t length);
static void app_ibrt_ota_start_cmd_send_rsp_handler(uint16_t rsp_seq, uint8_t *p_buff, uint16_t length);
static void app_ibrt_ota_start_cmd_send_rsp_timeout_handler(uint16_t rsp_seq, uint8_t *p_buff, uint16_t length);

static void app_ibrt_ota_config_cmd_send(uint8_t *p_buff, uint16_t length);
void app_ibrt_ota_config_cmd_send_handler(uint16_t rsp_seq, uint8_t *p_buff, uint16_t length);
static void app_ibrt_ota_config_cmd_send_rsp_handler(uint16_t rsp_seq, uint8_t *p_buff, uint16_t length);
static void app_ibrt_ota_config_cmd_send_rsp_timeout_handler(uint16_t rsp_seq, uint8_t *p_buff, uint16_t length);

static void app_ibrt_ota_segment_crc_cmd_send(uint8_t *p_buff, uint16_t length);
void app_ibrt_ota_segment_crc_cmd_send_handler(uint16_t rsp_seq, uint8_t *p_buff, uint16_t length);
static void app_ibrt_ota_segment_crc_cmd_send_rsp_handler(uint16_t rsp_seq, uint8_t *p_buff, uint16_t length);
static void app_ibrt_ota_segment_crc_cmd_send_rsp_timeout_handler(uint16_t rsp_seq, uint8_t *p_buff, uint16_t length);

static void app_ibrt_ota_image_crc_cmd_send(uint8_t *p_buff, uint16_t length);
void app_ibrt_ota_image_crc_cmd_send_handler(uint16_t rsp_seq, uint8_t *p_buff, uint16_t length);
static void app_ibrt_ota_image_crc_cmd_send_rsp_handler(uint16_t rsp_seq, uint8_t *p_buff, uint16_t length);
static void app_ibrt_ota_image_crc_cmd_send_rsp_timeout_handler(uint16_t rsp_seq, uint8_t *p_buff, uint16_t length);

static void app_ibrt_ota_image_overwrite_cmd_send(uint8_t *p_buff, uint16_t length);
void app_ibrt_ota_image_overwrite_cmd_send_handler(uint16_t rsp_seq, uint8_t *p_buff, uint16_t length);
static void app_ibrt_ota_image_overwrite_cmd_send_rsp_handler(uint16_t rsp_seq, uint8_t *p_buff, uint16_t length);
static void app_ibrt_ota_image_overwrite_cmd_send_rsp_timeout_handler(uint16_t rsp_seq, uint8_t *p_buff, uint16_t length);

// static void app_ibrt_ota_start_role_switch_cmd_send(uint8_t *p_buff, uint16_t length);
// static void app_ibrt_ota_start_role_switch_cmd_send_handler(uint16_t rsp_seq, uint8_t *p_buff, uint16_t length);

static void app_ibrt_ota_image_buff_cmd_send(uint8_t *p_buff, uint16_t length);
static void app_ibrt_ota_image_buff_cmd_send_handler(uint16_t rsp_seq, uint8_t *p_buff, uint16_t length);

//static void app_ibrt_common_ota_cmd_send(uint8_t *p_buff, uint16_t length);
//static void app_ibrt_common_ota_cmd_received(uint16_t rsp_seq, uint8_t *p_buff, uint16_t length);

#ifdef QIOT_ENABLED
static void app_ibrt_ota_qiot_ota_cmd_send(uint8_t *p_buff, uint16_t length);
static void app_ibrt_ota_qiot_ota_cmd_send_handler(uint16_t rsp_seq, uint8_t *p_buff, uint16_t length);
static void app_ibrt_ota_qiot_ota_cmd_send_rsp_handler(uint16_t rsp_seq, uint8_t *p_buff, uint16_t length);
#include "ble_qiot_llsync_ota.h"
#endif

static void app_ibrt_ota_pkt_ack_cmd_send(uint8_t *p_buff, uint16_t length);
static void app_ibrt_ota_pkt_ack_cmd_send_handler(uint16_t rsp_seq, uint8_t *p_buff, uint16_t length);
static void app_ibrt_ota_user_cmd_send(uint8_t *p_buff, uint16_t length);
static void app_ibrt_ota_user_cmd_send_handler(uint16_t rsp_seq, uint8_t *p_buff, uint16_t length);
static void app_ibrt_ota_user_cmd_send_rsp_timeout_handler(uint16_t rsp_seq, uint8_t *p_buff, uint16_t length);
static void app_ibrt_ota_user_cmd_send_rsp_handler(uint16_t rsp_seq, uint8_t *p_buff, uint16_t length);

static const app_tws_cmd_instance_t g_ibrt_ota_tws_cmd_handler_table[]=
{
    {
        IBRT_OTA_TWS_SET_USER_CMD,                                 "SET_USER",
        app_ibrt_ota_user_cmd_send,
        app_ibrt_ota_user_cmd_send_handler,                        RSP_TIMEOUT_DEFAULT,
        app_ibrt_ota_user_cmd_send_rsp_timeout_handler,            app_ibrt_ota_user_cmd_send_rsp_handler
    },
    {
        IBRT_OTA_TWS_GET_VERSION_CMD,                               "OTA_GET_VERSION",
        app_ibrt_ota_get_version_cmd_send,
        app_ibrt_ota_get_version_cmd_send_handler,                  RSP_TIMEOUT_DEFAULT,
        app_ibrt_ota_get_version_cmd_send_rsp_timeout_handler,      app_ibrt_ota_get_version_cmd_send_rsp_handler
    },
    {
        IBRT_OTA_TWS_SELECT_SIDE_CMD,                               "OTA_SELECT_SIDE",
        app_ibrt_ota_select_side_cmd_send,
        app_ibrt_ota_select_side_cmd_send_handler,                  RSP_TIMEOUT_DEFAULT,
        app_ibrt_ota_select_side_cmd_send_rsp_timeout_handler,      app_ibrt_ota_select_side_cmd_send_rsp_handler
    },
    {
        IBRT_OTA_TWS_BP_CHECK_CMD,                                  "OTA_BP_CHECK",//Break-point
        app_ibrt_ota_bp_check_cmd_send,
        app_ibrt_ota_bp_check_cmd_send_handler,                     RSP_TIMEOUT_DEFAULT,
        app_ibrt_ota_bp_check_cmd_send_rsp_timeout_handler,         app_ibrt_ota_bp_check_cmd_send_rsp_handler
    },
    {
        IBRT_OTA_TWS_START_OTA_CMD,                                 "OTA_START",
        app_ibrt_ota_start_cmd_send,
        app_ibrt_ota_start_cmd_send_handler,                        RSP_TIMEOUT_DEFAULT,
        app_ibrt_ota_start_cmd_send_rsp_timeout_handler,            app_ibrt_ota_start_cmd_send_rsp_handler
    },
    {
        IBRT_OTA_TWS_OTA_CONFIG_CMD,                                "OTA_CONFIG",
        app_ibrt_ota_config_cmd_send,
        app_ibrt_ota_config_cmd_send_handler,                       RSP_TIMEOUT_DEFAULT,
        app_ibrt_ota_config_cmd_send_rsp_timeout_handler,           app_ibrt_ota_config_cmd_send_rsp_handler
    },
    {
        IBRT_OTA_TWS_SEGMENT_CRC_CMD,                               "OTA_SEGMENT_CRC",
        app_ibrt_ota_segment_crc_cmd_send,
        app_ibrt_ota_segment_crc_cmd_send_handler,                  RSP_TIMEOUT_DEFAULT,
        app_ibrt_ota_segment_crc_cmd_send_rsp_timeout_handler,      app_ibrt_ota_segment_crc_cmd_send_rsp_handler
    },
    {
        IBRT_OTA_TWS_IMAGE_CRC_CMD,                                 "OTA_IMAGE_CRC",
        app_ibrt_ota_image_crc_cmd_send,
        app_ibrt_ota_image_crc_cmd_send_handler,                    RSP_TIMEOUT_DEFAULT,
        app_ibrt_ota_image_crc_cmd_send_rsp_timeout_handler,        app_ibrt_ota_image_crc_cmd_send_rsp_handler
    },
    {
        IBRT_OTA_TWS_IMAGE_OVERWRITE_CMD,                           "OTA_IMAGE_OVERWRITE",
        app_ibrt_ota_image_overwrite_cmd_send,
        app_ibrt_ota_image_overwrite_cmd_send_handler,              RSP_TIMEOUT_DEFAULT,
        app_ibrt_ota_image_overwrite_cmd_send_rsp_timeout_handler,  app_ibrt_ota_image_overwrite_cmd_send_rsp_handler
    },
    // {
    //     IBRT_OTA_TWS_ROLE_SWITCH_CMD,                              "OTA_ROLE_SWITCH",
    //     app_ibrt_ota_start_role_switch_cmd_send,
    //     app_ibrt_ota_start_role_switch_cmd_send_handler,           0,
    //     app_ibrt_ota_image_buff_cmd_rsp_timeout_handler_null,      app_ibrt_ota_image_buff_cmd_rsp_handler_null
    // },
    {
        IBRT_OTA_TWS_IMAGE_BUFF,                                    "OTA_IMAGE_BUFF",
        app_ibrt_ota_image_buff_cmd_send,
        app_ibrt_ota_image_buff_cmd_send_handler,                   0,
        app_ibrt_ota_image_buff_cmd_rsp_timeout_handler_null,       app_ibrt_ota_image_buff_cmd_rsp_handler_null
    },
    // {
    //     IBRT_COMMON_OTA,                                             "COMMON_OTA_CMD",
    //     app_ibrt_common_ota_cmd_send,
    //     app_ibrt_common_ota_cmd_received,                            0,
    //     app_ibrt_ota_image_buff_cmd_rsp_timeout_handler_null,       app_ibrt_ota_image_buff_cmd_rsp_handler_null
    // },
    {
        IBRT_OTA_TWS_ACK_CMD,                                       "OTA_PKT_ACK",
        app_ibrt_ota_pkt_ack_cmd_send,
        app_ibrt_ota_pkt_ack_cmd_send_handler,                      0,
        app_ibrt_ota_image_buff_cmd_rsp_timeout_handler_null,       app_ibrt_ota_image_buff_cmd_rsp_handler_null
    },
#ifdef QIOT_ENABLED
    {
        IBRT_OTA_TWS_QIOT_OTA_CMD,                                    "QIOT_OTA_CMD",
        app_ibrt_ota_qiot_ota_cmd_send,
        app_ibrt_ota_qiot_ota_cmd_send_handler,                     0,
        app_ibrt_ota_image_buff_cmd_rsp_timeout_handler_null,       app_ibrt_ota_qiot_ota_cmd_send_rsp_handler
    },

#endif
};

int app_ibrt_ota_tws_cmd_table_get(void **cmd_tbl, uint16_t *cmd_size)
{
    *cmd_tbl = (void *)&g_ibrt_ota_tws_cmd_handler_table;
    *cmd_size = ARRAY_SIZE(g_ibrt_ota_tws_cmd_handler_table);
    return 0;
}

extern OTA_IBRT_TWS_CMD_EXECUTED_RESULT_FROM_SLAVE_T receivedResultAlreadyProcessedBySlave;

uint32_t ibrt_ota_cmd_type = 0;
uint32_t twsBreakPoint = 0;
uint8_t errOtaCode = 0;
static uint8_t get_version_resend_num = RESEND_TIME;
static uint8_t select_side_resend_num = RESEND_TIME;
static uint8_t bp_check_resend_num = RESEND_TIME;
static uint8_t ota_start_resend_num = RESEND_TIME;
static uint8_t ota_config_resend_num = RESEND_TIME;
static uint8_t segment_crc_resend_num = RESEND_TIME;
static uint8_t image_crc_resend_num = RESEND_TIME;
static uint8_t image_overwrite_resend_num = RESEND_TIME;
static uint8_t set_user_resend_num = RESEND_TIME;

static void app_ibrt_ota_cache_slave_info(uint8_t typeCode, uint16_t rsp_seq, uint8_t *p_buff, uint16_t length)
{
    memcpy(&receivedResultAlreadyProcessedBySlave.typeCode, &typeCode, sizeof(typeCode));
    memcpy(&receivedResultAlreadyProcessedBySlave.rsp_seq, &rsp_seq, sizeof(rsp_seq));
    memcpy(&receivedResultAlreadyProcessedBySlave.length, &length, sizeof(length));
    memcpy(receivedResultAlreadyProcessedBySlave.p_buff, p_buff, length);
}

static void app_ibrt_ota_get_version_cmd_send(uint8_t *p_buff, uint16_t length)
{
    app_ibrt_send_cmd_with_rsp(IBRT_OTA_TWS_GET_VERSION_CMD, p_buff, length);
    TRACE(1,"%s", __func__);
}

void app_ibrt_ota_get_version_cmd_send_handler(uint16_t rsp_seq, uint8_t *p_buff, uint16_t length)
{
    if(ibrt_ota_cmd_type == OTA_RSP_VERSION)
    {
        tws_ctrl_send_rsp(IBRT_OTA_TWS_GET_VERSION_CMD, rsp_seq, p_buff, length);
        ibrt_ota_cmd_type = 0;
    }
    else
    {
        app_ibrt_ota_cache_slave_info(OTA_RSP_VERSION, rsp_seq, p_buff, length);
    }
    TRACE(1,"%s", __func__);
}

static void app_ibrt_ota_get_version_cmd_send_rsp_handler(uint16_t rsp_seq, uint8_t *p_buff, uint16_t length)
{
    get_version_resend_num = RESEND_TIME;
    ibrt_ota_send_version_rsp();
    TRACE(1,"%s", __func__);
}

static void app_ibrt_ota_get_version_cmd_send_rsp_timeout_handler(uint16_t rsp_seq, uint8_t *p_buff, uint16_t length)
{
    if(get_version_resend_num > 0)
    {
        tws_ctrl_send_cmd(IBRT_OTA_TWS_GET_VERSION_CMD, p_buff, length);
        get_version_resend_num--;
    }
    else
    {
        get_version_resend_num = RESEND_TIME;
    }
    TRACE(2,"%s, %d", __func__, get_version_resend_num);
}

static void app_ibrt_ota_select_side_cmd_send(uint8_t *p_buff, uint16_t length)
{
    app_ibrt_send_cmd_with_rsp(IBRT_OTA_TWS_SELECT_SIDE_CMD, p_buff, length);
    TRACE(1,"%s", __func__);
}

void app_ibrt_ota_select_side_cmd_send_handler(uint16_t rsp_seq, uint8_t *p_buff, uint16_t length)
{
    if(ibrt_ota_cmd_type == OTA_RSP_SIDE_SELECTION)
    {
        tws_ctrl_send_rsp(IBRT_OTA_TWS_SELECT_SIDE_CMD, rsp_seq, p_buff, length);
        ibrt_ota_cmd_type = 0;
    }
    else
    {
        app_ibrt_ota_cache_slave_info(OTA_RSP_SIDE_SELECTION, rsp_seq, p_buff, length);
    }
    TRACE(1,"%s", __func__);
}

static void app_ibrt_ota_select_side_cmd_send_rsp_handler(uint16_t rsp_seq, uint8_t *p_buff, uint16_t length)
{
    select_side_resend_num = RESEND_TIME;
    if(1 == p_buff[1])
    {
        ota_control_side_selection_rsp(true);
    }
    else
    {
        ota_control_side_selection_rsp(false);
    }
    TRACE(1,"%s", __func__);
}

static void app_ibrt_ota_select_side_cmd_send_rsp_timeout_handler(uint16_t rsp_seq, uint8_t *p_buff, uint16_t length)
{
    if(select_side_resend_num > 0)
    {
        tws_ctrl_send_cmd(IBRT_OTA_TWS_SELECT_SIDE_CMD, p_buff, length);
        select_side_resend_num--;
    }
    else
    {
        select_side_resend_num = RESEND_TIME;
    }
    TRACE(2,"%s, %d", __func__, select_side_resend_num);
}

static void app_ibrt_ota_bp_check_cmd_send(uint8_t *p_buff, uint16_t length)
{
    app_ibrt_send_cmd_with_rsp(IBRT_OTA_TWS_BP_CHECK_CMD, p_buff, length);
    TRACE(1,"%s", __func__);
}

void app_ibrt_ota_bp_check_cmd_send_handler(uint16_t rsp_seq, uint8_t *p_buff, uint16_t length)
{
    OTA_RSP_RESUME_VERIFY_T* tRsp = (OTA_RSP_RESUME_VERIFY_T *)p_buff;
    if(ibrt_ota_cmd_type == OTA_RSP_RESUME_VERIFY)
    {
        TRACE(2,"breakPoint %d, tRsp->breakPoint %d", twsBreakPoint, tRsp->breakPoint);
        if(twsBreakPoint == tRsp->breakPoint)
        {
            if(twsBreakPoint == 0)
            {
                TRACE(0,"reset random code:");
                LOG_DUMP("%02x ", (uint8_t *)tRsp, sizeof(OTA_RSP_RESUME_VERIFY_T));
                ota_randomCode_log((uint8_t *)&tRsp->randomCode);
            }
            ibrt_ota_cmd_type = 0;
        }
        else
        {
            TRACE(1,"%s tws break-point is not synchronized", __func__);
            ota_upgradeLog_destroy();
            ota_randomCode_log((uint8_t *)&tRsp->randomCode);
            ota_control_reset_env();
            ota_status_change(true);
            tRsp->breakPoint = 0xFFFFFFFF;
            twsBreakPoint = 0;
        }
        tws_ctrl_send_rsp(IBRT_OTA_TWS_BP_CHECK_CMD, rsp_seq, (uint8_t *)tRsp, length);
    }
    else
    {
        app_ibrt_ota_cache_slave_info(OTA_RSP_RESUME_VERIFY, rsp_seq, p_buff, length);
    }
    TRACE(1,"%s", __func__);
}

static void app_ibrt_ota_bp_check_cmd_send_rsp_handler(uint16_t rsp_seq, uint8_t *p_buff, uint16_t length)
{
    bp_check_resend_num = RESEND_TIME;
    OTA_RSP_RESUME_VERIFY_T* tRsp = (OTA_RSP_RESUME_VERIFY_T *)p_buff;

    if(tRsp->breakPoint == 0xFFFFFFFF)
    {
        TRACE(1,"%s tws break-point is not synchronized", __func__);
        ota_upgradeLog_destroy();
        ota_randomCode_log((uint8_t *)&tRsp->randomCode);
        ota_control_reset_env();
        ota_status_change(true);
        tRsp->breakPoint = 0;
        tws_ctrl_send_cmd(IBRT_OTA_TWS_BP_CHECK_CMD, (uint8_t *)tRsp, length);
    }
    else
    {
        ota_control_send_resume_response(tRsp->breakPoint, tRsp->randomCode);
    }
    TRACE(1,"%s", __func__);
}
static void app_ibrt_ota_bp_check_cmd_send_rsp_timeout_handler(uint16_t rsp_seq, uint8_t *p_buff, uint16_t length)
{
    if(bp_check_resend_num > 0)
    {
        tws_ctrl_send_cmd(IBRT_OTA_TWS_BP_CHECK_CMD, p_buff, length);
        bp_check_resend_num--;
    }
    else
    {
        bp_check_resend_num = RESEND_TIME;
    }
    TRACE(2,"%s, %d", __func__, bp_check_resend_num);
}

static void app_ibrt_ota_start_cmd_send(uint8_t *p_buff, uint16_t length)
{
    app_ibrt_send_cmd_with_rsp(IBRT_OTA_TWS_START_OTA_CMD, p_buff, length);
    TRACE(1,"%s", __func__);
}

void app_ibrt_ota_start_cmd_send_handler(uint16_t rsp_seq, uint8_t *p_buff, uint16_t length)
{
    if(ibrt_ota_cmd_type == OTA_RSP_START)
    {
        ibrt_ota_cmd_type = 0;
        tws_ctrl_send_rsp(IBRT_OTA_TWS_START_OTA_CMD, rsp_seq, p_buff, length);
    }
    else
    {
        app_ibrt_ota_cache_slave_info(OTA_RSP_START, rsp_seq, p_buff, length);
    }
    TRACE(1,"%s", __func__);
}

static void app_ibrt_ota_start_cmd_send_rsp_handler(uint16_t rsp_seq, uint8_t *p_buff, uint16_t length)
{
    ota_start_resend_num = RESEND_TIME;
    ibrt_ota_send_start_response(*p_buff);
    TRACE(1,"%s", __func__);
}
static void app_ibrt_ota_start_cmd_send_rsp_timeout_handler(uint16_t rsp_seq, uint8_t *p_buff, uint16_t length)
{
    if(ota_start_resend_num > 0)
    {
        tws_ctrl_send_cmd(IBRT_OTA_TWS_START_OTA_CMD, p_buff, length);
        ota_start_resend_num--;
    }
    else
    {
        ota_start_resend_num = RESEND_TIME;
    }
    TRACE(2,"%s, %d", __func__, ota_start_resend_num);
}

static void app_ibrt_ota_user_cmd_send(uint8_t *p_buff, uint16_t length)
{
    TRACE(1,"%s", __func__);
    app_ibrt_send_cmd_with_rsp(IBRT_OTA_TWS_SET_USER_CMD, p_buff, length);
}

static void app_ibrt_ota_user_cmd_send_handler(uint16_t rsp_seq, uint8_t *p_buff, uint16_t length)
{
    uint8_t isDone = 1;

    if(ibrt_ota_cmd_type == OTA_RSP_SET_USER)
    {
        ibrt_ota_cmd_type = 0;
        tws_ctrl_send_rsp(IBRT_OTA_TWS_SET_USER_CMD, rsp_seq, &isDone, length);
    }
    else
    {
        isDone = 0;
        app_ibrt_ota_cache_slave_info(OTA_RSP_SET_USER, rsp_seq, &isDone, length);
    }
    TRACE(1,"%s", __func__);
}

static void app_ibrt_ota_user_cmd_send_rsp_handler(uint16_t rsp_seq, uint8_t *p_buff, uint16_t length)
{
    set_user_resend_num = RESEND_TIME;
    ibrt_ota_send_user_response(*p_buff);
    TRACE(1,"%s", __func__);
}
static void app_ibrt_ota_user_cmd_send_rsp_timeout_handler(uint16_t rsp_seq, uint8_t *p_buff, uint16_t length)
{
    if(set_user_resend_num > 0)
    {
        tws_ctrl_send_cmd(IBRT_OTA_TWS_SET_USER_CMD, p_buff, length);
        set_user_resend_num--;
    }
    else
    {
        set_user_resend_num = RESEND_TIME;
    }
    TRACE(2,"%s, %d", __func__, set_user_resend_num);
}

static void app_ibrt_ota_config_cmd_send(uint8_t *p_buff, uint16_t length)
{
    app_ibrt_send_cmd_with_rsp(IBRT_OTA_TWS_OTA_CONFIG_CMD, p_buff, length);
    TRACE(1,"%s", __func__);
}

void app_ibrt_ota_config_cmd_send_handler(uint16_t rsp_seq, uint8_t *p_buff, uint16_t length)
{
    if(ibrt_ota_cmd_type == OTA_RSP_CONFIG)
    {
        if(*p_buff == 1 && errOtaCode == 1)
        {
            tws_ctrl_send_rsp(IBRT_OTA_TWS_OTA_CONFIG_CMD, rsp_seq, p_buff, length);
        }
        else if(*p_buff == OTA_RESULT_ERR_IMAGE_SIZE || errOtaCode == OTA_RESULT_ERR_IMAGE_SIZE)
        {
            uint8_t errImageSize = OTA_RESULT_ERR_IMAGE_SIZE;
            tws_ctrl_send_rsp(IBRT_OTA_TWS_OTA_CONFIG_CMD, rsp_seq, (uint8_t *)&errImageSize, length);
        }
        else if(*p_buff == OTA_RESULT_ERR_BREAKPOINT || errOtaCode == OTA_RESULT_ERR_BREAKPOINT)
        {
            uint8_t errBreakPoint = OTA_RESULT_ERR_BREAKPOINT;
            ota_upgradeLog_destroy();
            tws_ctrl_send_rsp(IBRT_OTA_TWS_OTA_CONFIG_CMD, rsp_seq, (uint8_t *)&errBreakPoint, length);
        }
        else
        {
            tws_ctrl_send_rsp(IBRT_OTA_TWS_OTA_CONFIG_CMD, rsp_seq, p_buff, length);
        }
        errOtaCode = 0;
        ibrt_ota_cmd_type = 0;
    }
    else
    {
        app_ibrt_ota_cache_slave_info(OTA_RSP_CONFIG, rsp_seq, p_buff, length);
    }
    TRACE(1,"%s", __func__);
}

static void app_ibrt_ota_config_cmd_send_rsp_handler(uint16_t rsp_seq, uint8_t *p_buff, uint16_t length)
{
    ota_config_resend_num = RESEND_TIME;

    if(*p_buff == 1)
    {
        ibrt_ota_send_configuration_response(true);
    }
    else if(*p_buff == OTA_RESULT_ERR_IMAGE_SIZE || errOtaCode == OTA_RESULT_ERR_IMAGE_SIZE)
    {
        ibrt_ota_send_result_response(OTA_RESULT_ERR_IMAGE_SIZE);
    }
    else if(*p_buff == OTA_RESULT_ERR_BREAKPOINT || errOtaCode == OTA_RESULT_ERR_BREAKPOINT)
    {
        ota_upgradeLog_destroy();
        ibrt_ota_send_result_response(OTA_RESULT_ERR_BREAKPOINT);
    }
    else
    {
        ibrt_ota_send_configuration_response(false);
    }
    TRACE(1,"%s", __func__);
}

static void app_ibrt_ota_config_cmd_send_rsp_timeout_handler(uint16_t rsp_seq, uint8_t *p_buff, uint16_t length)
{
    if(ota_config_resend_num > 0)
    {
        tws_ctrl_send_cmd(IBRT_OTA_TWS_OTA_CONFIG_CMD, p_buff, length);
        ota_config_resend_num--;
    }
    else
    {
        ota_config_resend_num = RESEND_TIME;
    }
    TRACE(2,"%s, %d", __func__, ota_config_resend_num);
}

static void app_ibrt_ota_segment_crc_cmd_send(uint8_t *p_buff, uint16_t length)
{
    app_ibrt_send_cmd_with_rsp(IBRT_OTA_TWS_SEGMENT_CRC_CMD, p_buff, length);
    TRACE(1,"%s", __func__);
}

void app_ibrt_ota_segment_crc_cmd_send_handler(uint16_t rsp_seq, uint8_t *p_buff, uint16_t length)
{
    if(ibrt_ota_cmd_type == OTA_RSP_SEGMENT_VERIFY)
    {
        TRACE(3,"%s: %d %d", __func__,*p_buff,errOtaCode);
        if(*p_buff == 1 && errOtaCode == 1)
        {
            ota_update_flash_offset_after_segment_crc(true);
            tws_ctrl_send_rsp(IBRT_OTA_TWS_SEGMENT_CRC_CMD, rsp_seq, p_buff, length);
        }
        else if(*p_buff == OTA_RESULT_ERR_SEG_VERIFY || errOtaCode == OTA_RESULT_ERR_SEG_VERIFY)
        {
            uint8_t errSegVerify = OTA_RESULT_ERR_SEG_VERIFY;
            ota_upgradeLog_destroy();
            tws_ctrl_send_rsp(IBRT_OTA_TWS_SEGMENT_CRC_CMD, rsp_seq, (uint8_t *)&errSegVerify, 1);
        }
        else if(*p_buff == OTA_RESULT_ERR_FLASH_OFFSET || errOtaCode == OTA_RESULT_ERR_FLASH_OFFSET)
        {
            uint8_t errFlashOffset = OTA_RESULT_ERR_FLASH_OFFSET;
            ota_upgradeLog_destroy();
            tws_ctrl_send_rsp(IBRT_OTA_TWS_SEGMENT_CRC_CMD, rsp_seq, (uint8_t *)&errFlashOffset, 1);
        }
        else
        {
            uint8_t err = 0;
            ota_update_flash_offset_after_segment_crc(false);
            tws_ctrl_send_rsp(IBRT_OTA_TWS_SEGMENT_CRC_CMD, rsp_seq, (uint8_t *)&err, 1);
        }
        errOtaCode = 0;
        ibrt_ota_cmd_type = 0;
    }
    else
    {
        app_ibrt_ota_cache_slave_info(OTA_RSP_SEGMENT_VERIFY, rsp_seq, p_buff, length);
    }
    TRACE(1,"%s", __func__);
}

static void app_ibrt_ota_segment_crc_cmd_send_rsp_handler(uint16_t rsp_seq, uint8_t *p_buff, uint16_t length)
{
    segment_crc_resend_num = RESEND_TIME;

    if(*p_buff == 1)
    {
        ota_update_flash_offset_after_segment_crc(true);
        ibrt_ota_send_segment_verification_response(true);
    }
    else if(*p_buff == 0)
    {
        ota_update_flash_offset_after_segment_crc(false);
        ibrt_ota_send_segment_verification_response(false);
    }
    else if(*p_buff == OTA_RESULT_ERR_SEG_VERIFY)
    {
        ota_upgradeLog_destroy();
        ibrt_ota_send_result_response(OTA_RESULT_ERR_SEG_VERIFY);
    }
    else if(*p_buff == OTA_RESULT_ERR_FLASH_OFFSET)
    {
        ota_upgradeLog_destroy();
        ibrt_ota_send_result_response(OTA_RESULT_ERR_FLASH_OFFSET);
    }
    TRACE(1,"%s", __func__);
}

static void app_ibrt_ota_segment_crc_cmd_send_rsp_timeout_handler(uint16_t rsp_seq, uint8_t *p_buff, uint16_t length)
{
    if(segment_crc_resend_num > 0)
    {
        tws_ctrl_send_cmd(IBRT_OTA_TWS_SEGMENT_CRC_CMD, p_buff, length);
        segment_crc_resend_num--;
    }
    else
    {
        segment_crc_resend_num = RESEND_TIME;
    }
    TRACE(2,"%s, %d", __func__, segment_crc_resend_num);
}

static void app_ibrt_ota_image_crc_cmd_send(uint8_t *p_buff, uint16_t length)
{
    app_ibrt_send_cmd_with_rsp(IBRT_OTA_TWS_IMAGE_CRC_CMD, p_buff, length);
    TRACE(1,"%s", __func__);
}

void app_ibrt_ota_image_crc_cmd_send_handler(uint16_t rsp_seq, uint8_t *p_buff, uint16_t length)
{
    TRACE(4,"%s cmd:0x%0x data:%d, err:%d", __func__,ibrt_ota_cmd_type,*p_buff, errOtaCode);

    if(ibrt_ota_cmd_type == OTA_RSP_RESULT)
    {
        if(*p_buff == 1 && errOtaCode == 1)
        {
            tws_ctrl_send_rsp(IBRT_OTA_TWS_IMAGE_CRC_CMD, rsp_seq, p_buff, length);
            errOtaCode = 0;
        }
        else
        {
            uint8_t crcErr = 0;
            ota_upgradeLog_destroy();
            ota_control_reset_env();
            tws_ctrl_send_rsp(IBRT_OTA_TWS_IMAGE_CRC_CMD, rsp_seq, (uint8_t *)&crcErr, 1);
        }
        ibrt_ota_cmd_type = 0;
    }
    else
    {
        app_ibrt_ota_cache_slave_info(OTA_RSP_RESULT, rsp_seq, p_buff, length);
    }
}

static void app_ibrt_ota_image_crc_cmd_send_rsp_handler(uint16_t rsp_seq, uint8_t *p_buff, uint16_t length)
{
    image_crc_resend_num = RESEND_TIME;
    if(*p_buff == 1)
    {
        ibrt_ota_send_result_response(true);
    }
    else if(*p_buff == 0)
    {
        ibrt_ota_send_result_response(false);
        ota_upgradeLog_destroy();
        ota_control_reset_env();
    }
    TRACE(1,"%s", __func__);
}

static void app_ibrt_ota_image_crc_cmd_send_rsp_timeout_handler(uint16_t rsp_seq, uint8_t *p_buff, uint16_t length)
{
    if(image_crc_resend_num > 0)
    {
        tws_ctrl_send_cmd(IBRT_OTA_TWS_IMAGE_CRC_CMD, p_buff, length);
        image_crc_resend_num--;
    }
    else
    {
        image_crc_resend_num = RESEND_TIME;
    }
    TRACE(2,"%s, %d", __func__, image_crc_resend_num);
}

static void app_ibrt_ota_image_overwrite_cmd_send(uint8_t *p_buff, uint16_t length)
{
    app_ibrt_send_cmd_with_rsp(IBRT_OTA_TWS_IMAGE_OVERWRITE_CMD, p_buff, length);
    TRACE(1,"%s", __func__);
}

void app_ibrt_ota_image_overwrite_cmd_send_handler(uint16_t rsp_seq, uint8_t *p_buff, uint16_t length)
{
    if(ibrt_ota_cmd_type == OTA_RSP_IMAGE_APPLY)
    {
        if(*p_buff == 1 && errOtaCode == 1)
        {
            int ret = tws_ctrl_send_rsp(IBRT_OTA_TWS_IMAGE_OVERWRITE_CMD, rsp_seq, p_buff, length);
            if(0 == ret)
            {
                ota_apply();
            }
        }
        else
        {
            uint8_t cantOverWrite = 0;
            tws_ctrl_send_rsp(IBRT_OTA_TWS_IMAGE_OVERWRITE_CMD, rsp_seq, (uint8_t *)&cantOverWrite, length);
        }
        errOtaCode = 0;
        ibrt_ota_cmd_type = 0;
    }
    TRACE(1,"%s", __func__);
}

static void app_ibrt_ota_image_overwrite_cmd_send_rsp_handler(uint16_t rsp_seq, uint8_t *p_buff, uint16_t length)
{
    image_overwrite_resend_num = RESEND_TIME;
    if(*p_buff == 1)
    {
        ota_control_image_apply_rsp(true);
        ota_apply();
    }
    else
    {
        ota_control_image_apply_rsp(false);
    }
    TRACE(1,"%s", __func__);
}

static void app_ibrt_ota_image_overwrite_cmd_send_rsp_timeout_handler(uint16_t rsp_seq, uint8_t *p_buff, uint16_t length)
{
    if(image_overwrite_resend_num > 0)
    {
        tws_ctrl_send_cmd(IBRT_OTA_TWS_IMAGE_OVERWRITE_CMD, p_buff, length);
        image_overwrite_resend_num--;
    }
    else
    {
        image_overwrite_resend_num = RESEND_TIME;
    }
    TRACE(2,"%s, %d", __func__, image_overwrite_resend_num);
}

static void app_ibrt_ota_image_buff_cmd_send(uint8_t *p_buff, uint16_t length)
{
    app_ibrt_send_cmd_without_rsp(IBRT_OTA_TWS_IMAGE_BUFF, p_buff, length);
    TRACE(1,"%s", __func__);
}

static void app_ibrt_ota_image_buff_cmd_send_handler(uint16_t rsp_seq, uint8_t *p_buff, uint16_t length)
{
    ota_bes_handle_received_data(p_buff, true, length);
    TRACE(1,"%s", __func__);
}

static void app_ibrt_ota_pkt_ack_cmd_send(uint8_t *p_buff, uint16_t length)
{
    app_ibrt_send_cmd_without_rsp(IBRT_OTA_TWS_ACK_CMD, p_buff, length);
    TRACE(1,"%s", __func__);
}

static void app_ibrt_ota_pkt_ack_cmd_send_handler(uint16_t rsp_seq, uint8_t *p_buff, uint16_t length)
{
    app_send_ota_pkt_sync_ack_response(true);
}

#ifdef QIOT_ENABLED
static void app_ibrt_ota_qiot_ota_cmd_send(uint8_t *p_buff, uint16_t length)
{
    app_ibrt_send_cmd_without_rsp(IBRT_OTA_TWS_QIOT_OTA_CMD, p_buff, length);
    TRACE(1,"%s", __func__);
}

static void app_ibrt_ota_qiot_ota_cmd_send_handler(uint16_t rsp_seq, uint8_t *p_buff, uint16_t length)
{

    qiot_ota_handle_received_cmd(rsp_seq,p_buff, length);
    //TRACE(1,"%s", __func__);
}

static void app_ibrt_ota_qiot_ota_cmd_send_rsp_handler(uint16_t rsp_seq, uint8_t *p_buff, uint16_t length)
{
    qiot_ota_handle_received_cmd(rsp_seq,p_buff, length);
    TRACE(1,"%s", __func__);
}

#endif

#endif
