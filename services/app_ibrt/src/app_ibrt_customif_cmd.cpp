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
#include "app_ibrt_customif_cmd.h"
#include "app_tws_if.h"
#if defined(USE_LOWLATENCY_LIB)
#include "app_ally.h"
#endif
#if defined(IBRT)
/*
* custom cmd handler add here, this is just a example
*/

#define app_ibrt_custom_cmd_rsp_timeout_handler_null   (0)
#define app_ibrt_custom_cmd_rsp_handler_null           (0)
#define app_ibrt_custom_cmd_rx_handler_null            (0)

static void app_ibrt_customif_test1_cmd_send(uint8_t *p_buff, uint16_t length);
static void app_ibrt_customif_test1_cmd_send_handler(uint16_t rsp_seq, uint8_t *p_buff, uint16_t length);

static void app_ibrt_customif_test2_cmd_send(uint8_t *p_buff, uint16_t length);
static void app_ibrt_customif_test2_cmd_send_handler(uint16_t rsp_seq, uint8_t *p_buff, uint16_t length);
static void app_ibrt_customif_test2_cmd_send_rsp_timeout_handler(uint16_t rsp_seq, uint8_t *p_buff, uint16_t length);
static void app_ibrt_customif_test2_cmd_send_rsp_handler(uint16_t rsp_seq, uint8_t *p_buff, uint16_t length);
#if defined(USE_LOWLATENCY_LIB)
void app_ibrt_lly_cmd_send(uint8_t *p_buff, uint16_t length);
void app_ibrt_lly_sync_receive_handler(uint16_t rsp_seq, uint8_t *p_buff, uint16_t length);
#endif

static const app_tws_cmd_instance_t g_ibrt_custom_cmd_handler_table[]=
{
#ifdef GFPS_ENABLED
    {
        APP_TWS_CMD_SHARE_FASTPAIR_INFO,                        "SHARE_FASTPAIR_INFO",
        app_ibrt_share_fastpair_info,
        app_ibrt_shared_fastpair_info_received_handler,         0,
        app_ibrt_custom_cmd_rsp_timeout_handler_null,           app_ibrt_custom_cmd_rsp_handler_null
    },
    {
        APP_TWS_CMD_GFPS_RING_STOP_SYNC,                        "TWS_CMD_GFPS_STOP_RING_STATE",
        app_ibrt_customif_gfps_stop_ring_sync_send,
        app_ibrt_customif_gfps_stop_ring_sync_recv,             0,
        app_ibrt_custom_cmd_rsp_timeout_handler_null,           app_ibrt_custom_cmd_rsp_handler_null
    },
#endif
    {
        APP_IBRT_CUSTOM_CMD_TEST1,                              "TWS_CMD_TEST1",
        app_ibrt_customif_test1_cmd_send,
        app_ibrt_customif_test1_cmd_send_handler,               0,
        app_ibrt_custom_cmd_rsp_timeout_handler_null,           app_ibrt_custom_cmd_rsp_handler_null
    },
    {
        APP_IBRT_CUSTOM_CMD_TEST2,                              "TWS_CMD_TEST2",
        app_ibrt_customif_test2_cmd_send,
        app_ibrt_customif_test2_cmd_send_handler,               RSP_TIMEOUT_DEFAULT,
        app_ibrt_customif_test2_cmd_send_rsp_timeout_handler,   app_ibrt_customif_test2_cmd_send_rsp_handler
    },
#if defined(USE_LOWLATENCY_LIB)
    {
        APP_IBRT_CUSTOM_CMD_LLY_SYNC,                           "TWS_CMD_SYNC_LLY",
        app_ibrt_lly_cmd_send,
        app_ibrt_lly_sync_receive_handler,                      0,
        app_ibrt_custom_cmd_rsp_timeout_handler_null,           app_ibrt_custom_cmd_rsp_handler_null
    },
#endif
};

int app_ibrt_customif_cmd_table_get(void **cmd_tbl, uint16_t *cmd_size)
{
    *cmd_tbl = (void *)&g_ibrt_custom_cmd_handler_table;
    *cmd_size = ARRAY_SIZE(g_ibrt_custom_cmd_handler_table);
    return 0;
}


void app_ibrt_customif_cmd_test(ibrt_custom_cmd_test_t *cmd_test)
{
    tws_ctrl_send_cmd(APP_IBRT_CUSTOM_CMD_TEST1, (uint8_t*)cmd_test, sizeof(ibrt_custom_cmd_test_t));
    tws_ctrl_send_cmd(APP_IBRT_CUSTOM_CMD_TEST2, (uint8_t*)cmd_test, sizeof(ibrt_custom_cmd_test_t));
}

static void app_ibrt_customif_test1_cmd_send(uint8_t *p_buff, uint16_t length)
{
    app_ibrt_send_cmd_without_rsp(APP_IBRT_CUSTOM_CMD_TEST1, p_buff, length);
    TRACE(1,"%s", __func__);
}

static void app_ibrt_customif_test1_cmd_send_handler(uint16_t rsp_seq, uint8_t *p_buff, uint16_t length)
{
    TRACE(1,"%s", __func__);
}

static void app_ibrt_customif_test2_cmd_send(uint8_t *p_buff, uint16_t length)
{
    TRACE(1,"%s", __func__);
}

static void app_ibrt_customif_test2_cmd_send_handler(uint16_t rsp_seq, uint8_t *p_buff, uint16_t length)
{
    tws_ctrl_send_rsp(APP_IBRT_CUSTOM_CMD_TEST2, rsp_seq, NULL, 0);
    TRACE(1,"%s", __func__);

}

static void app_ibrt_customif_test2_cmd_send_rsp_timeout_handler(uint16_t rsp_seq, uint8_t *p_buff, uint16_t length)
{
    TRACE(1,"%s", __func__);

}

static void app_ibrt_customif_test2_cmd_send_rsp_handler(uint16_t rsp_seq, uint8_t *p_buff, uint16_t length)
{
    TRACE(1,"%s", __func__);
}
#endif
#if defined(USE_LOWLATENCY_LIB)
extern void app_low_latency_sync_status_receive(void *info, uint16_t length);

void app_ibrt_lly_cmd(uint8_t *p_buff, uint16_t length)
{

    if(length&&p_buff){
        tws_ctrl_send_cmd(APP_IBRT_CUSTOM_CMD_LLY_SYNC, (uint8_t*)p_buff, length);
    }

}

void app_ibrt_lly_sync_receive_handler(uint16_t rsp_seq, uint8_t *p_buff, uint16_t length)
{
    app_low_latency_sync_status_receive(p_buff, length);
}

void app_ibrt_lly_sync_receive_handler_rsp(uint16_t rsp_seq, uint8_t *p_buff, uint16_t length)
{
    TRACE(1,"%s", __func__);
}

void app_ibrt_lly_cmd_send(uint8_t *p_buff, uint16_t length)
{
    TRACE(3,"%s  %x %d", __func__, p_buff, length);
    if(length&&p_buff){
        app_ibrt_send_cmd_without_rsp(APP_IBRT_CUSTOM_CMD_LLY_SYNC, p_buff, length);
    }
}
#endif
