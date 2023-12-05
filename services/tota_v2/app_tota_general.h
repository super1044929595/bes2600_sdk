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

#ifndef __APP_TOTA_GENERAL_H__
#define __APP_TOTA_GENERAL_H__

#include "app_battery.h"
#include "factory_section.h"
#include "app_ibrt_rssi.h"
#include "app_tws_ibrt.h"
#include "app_bt_stream.h"
#include "app_ibrt_if.h"
#include "app_tws_if.h"

#define BT_BLE_LOCAL_NAME_LEN       32

#define APP_TOTA_BUTTON_SETTINGS_CONTROL_CMD              0x01
#define APP_TOTA_FACTORY_RESET_CMD                        0x02
#define APP_TOTA_MUSIC_PLAY_SETTINGS_CMD                  0x03
#define APP_TOTA_BATTERY_LEVEL_CMD                        0x04
#define APP_TOTA_EARBUD_FIT_TEST_CMD                      0x05
#define APP_TOAT_EQ_CMD                                   0x06

#define BUTTON_SETTING_LEFT_EARPHONE_CMD       0x01
#define BUTTON_SETTING_RIGHT_EARPHONE_CMD      0x02
#define BUTTON_SETTING_PRE_SONG_CMD            0x01
#define BUTTON_SETING_NEXT_SONG_CMD            0x02
#define BUTTON_SETTING_ANC_CMD                 0x03
#define BUTTON_SETTING_CALL_CMD                0x04


#define MUSIC_PLAY_SETTINGS_PLAY_CMD                      0x01
#define MUSIC_PLAY_SETTINGS_PAUSE_CMD                     0x02
#define MUSIC_PLAY_SETTINGS_NEXT_CMD                      0x03
#define MUSIC_PLAY_SETTINGS_PRE_CMD                       0x04



typedef struct{
    /* bt-ble info */
    char btName[BT_BLE_LOCAL_NAME_LEN];         // 32 bytes
    char bleName[BT_BLE_LOCAL_NAME_LEN];        // 32 bytes
    bt_bdaddr_t btLocalAddr;                    // 6 bytes
    bt_bdaddr_t btPeerAddr;                     // 6 bytes
    bt_bdaddr_t bleLocalAddr;                   // 6 bytes
    bt_bdaddr_t blePeerAddr;                    // 6 bytes

    /* ibrt info */
    ibrt_role_e ibrtRole;                       // 1 byte

    /* crystal info */
    uint32_t crystal_freq;                      // 4 bytes
    unsigned int xtal_fcap;                         // 4 bytes
    
    /* battery info */
    APP_BATTERY_MV_T        battery_volt;       // 2 bytes
    uint8_t                 battery_level;      // 1 byte
    APP_BATTERY_STATUS_T    battery_status;     // 4 bytes

    /* firmware info */
    uint8_t fw_version[4];                      // 4 bytes
    
    /* ear location info */
    APP_TWS_SIDE_T          ear_location;       // 4 byte

    /* rssi info */
    uint8_t                 rssi[48];           // 48 bytes
    uint32_t                rssi_len;           // 4 bytes
} general_info_t;

/*--functions from other file--*/
#ifdef FIRMWARE_REV
extern "C" void system_get_info(uint8_t *fw_rev_0, uint8_t *fw_rev_1, uint8_t *fw_rev_2, uint8_t *fw_rev_3);
#endif

extern void app_bt_volumeup();
extern void app_bt_volumedown();
extern int app_bt_stream_local_volume_get(void);

/*--functions from other file--*/


/*--interface--*/
void app_tota_general_init();



#endif

