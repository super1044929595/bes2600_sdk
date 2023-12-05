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
#ifndef __BT_DRV_TEST_MODE_H_
#define __BT_DRV_TEST_MODE_H_
    

#ifdef __cplusplus
extern "C" {
#endif

enum BTDRV_DBG_NONSIG_PKT_TYPE {
    BTDRV_DBG_NONSIG_TYPE_ID_NUL = 0,
    BTDRV_DBG_NONSIG_TYPE_POLL   = 1,
    BTDRV_DBG_NONSIG_TYPE_FHS    = 2,
    BTDRV_DBG_NONSIG_TYPE_DM1    = 3,
    BTDRV_DBG_NONSIG_TYPE_DH1    = 4,
    BTDRV_DBG_NONSIG_TYPE_DH1_2  = 5,
    BTDRV_DBG_NONSIG_TYPE_DH1_3  = 6,
    BTDRV_DBG_NONSIG_TYPE_HV1    = 7,
    BTDRV_DBG_NONSIG_TYPE_HV2    = 8,
    BTDRV_DBG_NONSIG_TYPE_EV3_2  = 9,
    BTDRV_DBG_NONSIG_TYPE_HV3    = 10,
    BTDRV_DBG_NONSIG_TYPE_EV3    = 11,
    BTDRV_DBG_NONSIG_TYPE_EV3_3  = 12,
    BTDRV_DBG_NONSIG_TYPE_DV     = 13,
    BTDRV_DBG_NONSIG_TYPE_AUX1   = 14,
    BTDRV_DBG_NONSIG_TYPE_DM3    = 15,
    BTDRV_DBG_NONSIG_TYPE_DH3    = 16,
    BTDRV_DBG_NONSIG_TYPE_DH3_2  = 17,
    BTDRV_DBG_NONSIG_TYPE_DH3_3  = 18,
    BTDRV_DBG_NONSIG_TYPE_EV4    = 19,
    BTDRV_DBG_NONSIG_TYPE_EV5_2  = 20,
    BTDRV_DBG_NONSIG_TYPE_EV5    = 21,
    BTDRV_DBG_NONSIG_TYPE_EV5_3  = 22,
    BTDRV_DBG_NONSIG_TYPE_DM5    = 23,
    BTDRV_DBG_NONSIG_TYPE_DH5    = 24,
    BTDRV_DBG_NONSIG_TYPE_DH5_2  = 25,
    BTDRV_DBG_NONSIG_TYPE_DH5_3  = 26,
    BTDRV_DBG_NONSIG_TYPE_END    = 27,
};

typedef struct
{
    enum BTDRV_DBG_NONSIG_PKT_TYPE enum_pkt_type;
    uint8_t packet_type;
    uint8_t edr_enabled;
    uint16_t payload_length;
}BTDRV_DBG_NONSIG_TESTER_PKT_TYPE_T;

typedef struct
{
    uint8_t tx_freq;
    uint8_t power_level;
    uint8_t addr[6];
    enum BTDRV_DBG_NONSIG_PKT_TYPE btdrv_pkt_type;
}BTDRV_DBG_NONSIG_TESTER_SETUP_SLIM_CMD_TX_T;

typedef struct
{
    uint8_t rx_freq;
    uint8_t addr[6];
    enum BTDRV_DBG_NONSIG_PKT_TYPE btdrv_pkt_type;
}BTDRV_DBG_NONSIG_TESTER_SETUP_SLIM_CMD_RX_T;

typedef struct
{
    uint16_t pkt_counters;
    uint16_t head_errors;
    uint16_t payload_errors;
}BTDRV_DBG_NONSIG_TESTER_RESULT_BT_T;

typedef struct
{
    uint8_t tx_freq;
}BTDRV_DBG_NONSIG_TESTER_SETUP_CMD_BLE_TX_T;

typedef struct
{
    uint8_t rx_freq;
}BTDRV_DBG_NONSIG_TESTER_SETUP_CMD_BLE_RX_T;

typedef struct
{
    uint16_t pkt_counters;
}BTDRV_DBG_NONSIG_TESTER_RESULT_BLE_T;

int btdrv_testmode_enable_scan(void);
int btdrv_testmode_disable_scan(void);
int btdrv_testmode_enter(void);
int btdrv_testmode_exit(void);
int btdrv_testmode_bt_nonsig_tx(BTDRV_DBG_NONSIG_TESTER_SETUP_SLIM_CMD_TX_T *tx_cfg);
int btdrv_testmode_bt_nonsig_rx(BTDRV_DBG_NONSIG_TESTER_SETUP_SLIM_CMD_RX_T *rx_cfg);
int btdrv_testmode_bt_nonsig_endtest(void);
int btdrv_testmode_bt_nonsig_result_get(BTDRV_DBG_NONSIG_TESTER_RESULT_BT_T *bt_result);
int btdrv_testmode_ble_nonsig_tx(BTDRV_DBG_NONSIG_TESTER_SETUP_CMD_BLE_TX_T *ble_tx_cfg);
int btdrv_testmode_ble_nonsig_rx(BTDRV_DBG_NONSIG_TESTER_SETUP_CMD_BLE_RX_T *ble_rx_cfg);
int btdrv_testmode_ble_nonsig_endtest(void);
int btdrv_testmode_ble_nonsig_result_get(BTDRV_DBG_NONSIG_TESTER_RESULT_BLE_T *ble_result);
int btdrv_testmode_enable_dut(void);

#ifdef __cplusplus
}
#endif

#endif

