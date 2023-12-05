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
#include "string.h"
#include "cmsis_os.h"
#include "ota_bes.h"
#include "rwapp_config.h"
#include "ota_spp.h"
#include "cqueue.h"
#if (BLE_APP_OTA)
#include "app_ble_rx_handler.h"
#endif
#include "btapp.h"
#include "app_bt.h"
#include "apps.h"
#include "app_thread.h"
#include "cqueue.h"
#include "hal_location.h"

typedef struct
{
    uint8_t connectedType;
} APP_OTA_ENV_T;

static APP_OTA_ENV_T app_ota_env=
    {
        0,
    };

bool app_is_in_ota_mode(void)
{
    return app_ota_env.connectedType;
}

extern "C" void bulk_read_done(void);
void app_ota_connected(uint8_t connType)
{
    LOG_DBG("ota is connected.");
    app_ota_env.connectedType |= connType;
}

void app_ota_disconnected(uint8_t disconnType)
{
    LOG_DBG("Ota is disconnected.");
    app_ota_env.connectedType &= disconnType;
    Bes_exit_ota_state();
    app_spp_ota_register_tx_done(NULL);
}

void bes_ota_init(void)
{
    app_spp_ota_init();
#if (BLE_APP_OTA)
    if(!get_init_state())
    {
        app_ble_rx_handler_init();
    }
#endif
}

