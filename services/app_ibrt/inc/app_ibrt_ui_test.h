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
#ifndef __APP_IBRT_UI_TEST_H__
#define __APP_IBRT_UI_TEST_H__
#include <stdint.h>

#if defined(IBRT)
#define  IBRT_OPEN_BOX_TEST_EVENT       (1)
#define  IBRT_FETCH_OUT_TEST_EVENT      (2)
#define  IBRT_PUT_IN_TEST_EVENT         (3)
#define  IBRT_CLOSE_BOX_TEST_EVENT      (4)

#define  IBRT_WEAR_UP_TEST_EVENT        (5)
#define  IBRT_WEAR_DOWN_TEST_EVENT      (6)
#define  IBRT_RECONNECT_TEST_EVENT      (7)
#define  IBRT_PHONE_CONNECT_TEST_EVENT  (8)

typedef void (*app_uart_test_function_handle)(void);
typedef struct
{
    const char* string;
    app_uart_test_function_handle function;
} app_uart_handle_t;
app_uart_test_function_handle app_ibrt_ui_find_uart_handle(unsigned char* buf);

void app_ibrt_ui_test_key_init(void);

void app_ibrt_ui_test_init(void);

int app_ibrt_ui_test_config_load(void *config);

void app_ibrt_ui_sync_anc_status(uint8_t *buf, uint32_t len);

void app_ibrt_ui_sync_psap_status(uint8_t *buf, uint32_t len);

void app_ibrt_ui_sync_anc_assist_status(uint8_t *buf, uint32_t len);

void app_ibrt_ui_test_voice_assistant_key(APP_KEY_STATUS *status, void *param);
void app_ibrt_ui_audio_pause(void);
void app_ibrt_ui_audio_play(void);
#endif
#endif
