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
#include "bt_drv.h"
#include "bt_drv_testmode.h"
#include "bt_drv_reg_op.h"
#include "bt_drv_interface.h"
#include "hal_intersys.h"
#include "tgt_hardware.h"
#include "nvrecord_dev.h"
#include "intersyshci.h"

#define ID_NUL_TYPE     0x0
#define POLL_TYPE       0x1
#define FHS_TYPE        0x2
#define DM1_TYPE        0x3
#define DH1_TYPE        0x4
#define DH1_2_TYPE      0x4
#define DH1_3_TYPE      0x8
#define HV1_TYPE        0x5
#define HV2_TYPE        0x6
#define EV3_2_TYPE      0x6
#define HV3_TYPE        0x7
#define EV3_TYPE        0x7
#define EV3_3_TYPE      0x7
#define DV_TYPE         0x8
#define AUX1_TYPE       0x9
#define DM3_TYPE        0xA
#define DH3_TYPE        0xB
#define DH3_2_TYPE      0xA
#define DH3_3_TYPE      0xB
#define EV4_TYPE        0xC
#define EV5_2_TYPE      0xC
#define EV5_TYPE        0xD
#define EV5_3_TYPE      0xD
#define DM5_TYPE        0xE
#define DH5_TYPE        0xF
#define DH5_2_TYPE      0xE
#define DH5_3_TYPE      0xF

#define FHS_PACKET_SIZE         18
#define DM1_PACKET_SIZE         17
#define DH1_PACKET_SIZE         27
#define DV_ACL_PACKET_SIZE      9
#define DM3_PACKET_SIZE         121
#define DH3_PACKET_SIZE         183
#define DM5_PACKET_SIZE         224
#define DH5_PACKET_SIZE         339
#define AUX1_PACKET_SIZE        29
#define HV1_PACKET_SIZE         10
#define HV2_PACKET_SIZE         20
#define HV3_PACKET_SIZE         30
#define EV3_PACKET_SIZE         30
#define EV4_PACKET_SIZE         120
#define EV5_PACKET_SIZE         180
#define DH1_2_PACKET_SIZE        54
#define DH1_3_PACKET_SIZE        83
#define DH3_2_PACKET_SIZE        367
#define DH3_3_PACKET_SIZE        552
#define DH5_2_PACKET_SIZE        679
#define DH5_3_PACKET_SIZE        1021
#define EV3_2_PACKET_SIZE       60
#define EV3_3_PACKET_SIZE       90
#define EV5_2_PACKET_SIZE       360
#define EV5_3_PACKET_SIZE       540

#define EDR_DISABLED    0 /* Erroneous Data Reporting disabled */
#define EDR_ENABLED     1 /* Erroneous Data Reporting enabled */

#define BTDRV_STORELE16(buff,num) (((buff)[1] = (uint8_t) ((num)>>8)), ((buff)[0] = (uint8_t) (num)))

static BTDRV_DBG_NONSIG_TESTER_PKT_TYPE_T btdrv_dbg_nonsig_pkt_type_table[BTDRV_DBG_NONSIG_TYPE_END] =
{
    {BTDRV_DBG_NONSIG_TYPE_ID_NUL,  ID_NUL_TYPE,    EDR_DISABLED,   0},
    {BTDRV_DBG_NONSIG_TYPE_POLL,    POLL_TYPE,      EDR_DISABLED,   0},
    {BTDRV_DBG_NONSIG_TYPE_FHS,     FHS_TYPE,       EDR_DISABLED,   FHS_PACKET_SIZE},
    {BTDRV_DBG_NONSIG_TYPE_DM1,     DM1_TYPE,       EDR_DISABLED,   DM1_PACKET_SIZE},
    {BTDRV_DBG_NONSIG_TYPE_DH1,     DH1_TYPE,       EDR_DISABLED,   DH1_PACKET_SIZE},
    {BTDRV_DBG_NONSIG_TYPE_DH1_2,   DH1_2_TYPE,     EDR_ENABLED,    DH1_2_PACKET_SIZE},
    {BTDRV_DBG_NONSIG_TYPE_DH1_3,   DH1_3_TYPE,     EDR_ENABLED,    DH1_3_PACKET_SIZE},
    {BTDRV_DBG_NONSIG_TYPE_HV1,     HV1_TYPE,       EDR_DISABLED,   HV1_PACKET_SIZE},
    {BTDRV_DBG_NONSIG_TYPE_HV2,     HV2_TYPE,       EDR_DISABLED,   HV2_PACKET_SIZE},
    {BTDRV_DBG_NONSIG_TYPE_EV3_2,   EV3_2_TYPE,     EDR_ENABLED,    EV3_2_PACKET_SIZE},
    {BTDRV_DBG_NONSIG_TYPE_HV3,     HV3_TYPE,       EDR_DISABLED,   HV3_PACKET_SIZE},
    {BTDRV_DBG_NONSIG_TYPE_EV3,     EV3_TYPE,       EDR_DISABLED,   EV3_PACKET_SIZE},
    {BTDRV_DBG_NONSIG_TYPE_EV3_3,   EV3_3_TYPE,     EDR_ENABLED,    EV3_3_PACKET_SIZE},
    {BTDRV_DBG_NONSIG_TYPE_DV,      DV_TYPE,        EDR_DISABLED,   DV_ACL_PACKET_SIZE},
    {BTDRV_DBG_NONSIG_TYPE_AUX1,    AUX1_TYPE,      EDR_DISABLED,   AUX1_PACKET_SIZE},
    {BTDRV_DBG_NONSIG_TYPE_DM3,     DM3_TYPE,       EDR_DISABLED,   DM3_PACKET_SIZE},
    {BTDRV_DBG_NONSIG_TYPE_DH3,     DH3_TYPE,       EDR_DISABLED,   DH3_PACKET_SIZE},
    {BTDRV_DBG_NONSIG_TYPE_DH3_2,   DH3_2_TYPE,     EDR_ENABLED,    DH3_2_PACKET_SIZE},
    {BTDRV_DBG_NONSIG_TYPE_DH3_3,   DH3_3_TYPE,     EDR_ENABLED,    DH3_3_PACKET_SIZE},
    {BTDRV_DBG_NONSIG_TYPE_EV4,     EV4_TYPE,       EDR_DISABLED,   EV4_PACKET_SIZE},
    {BTDRV_DBG_NONSIG_TYPE_EV5_2,   EV5_2_TYPE,     EDR_ENABLED,    EV5_2_PACKET_SIZE},
    {BTDRV_DBG_NONSIG_TYPE_EV5,     EV5_TYPE,       EDR_DISABLED,   EV5_PACKET_SIZE},
    {BTDRV_DBG_NONSIG_TYPE_EV5_3,   EV5_3_TYPE,     EDR_ENABLED,    EV5_3_PACKET_SIZE},
    {BTDRV_DBG_NONSIG_TYPE_DM5,     DM5_TYPE,       EDR_DISABLED,   DM5_PACKET_SIZE},
    {BTDRV_DBG_NONSIG_TYPE_DH5,     DH5_TYPE,       EDR_DISABLED,   DH5_PACKET_SIZE},
    {BTDRV_DBG_NONSIG_TYPE_DH5_2,   DH5_2_TYPE,     EDR_ENABLED,    DH5_2_PACKET_SIZE},
    {BTDRV_DBG_NONSIG_TYPE_DH5_3,   DH5_3_TYPE,     EDR_ENABLED,    DH5_3_PACKET_SIZE},
};

typedef struct
{
    uint16_t    pkt_counters;
    uint16_t    head_errors;
    uint16_t    payload_errors;
    int16_t    avg_estsw;
    int16_t    avg_esttpl;
    uint32_t    payload_bit_errors;
}DBG_NONSIG_TEST_EVENT_IND_T;

typedef struct
{
    uint16_t pkt_counters;
}DBG_NONSIG_TEST_EVENT_IND_BLE_T;

const uint8_t hci_cmd_enable_dut[] =
{
    0x01,0x03, 0x18, 0x00
};

const uint8_t hci_cmd_enable_allscan[] =
{
    0x01, 0x1a, 0x0c, 0x01, 0x03
};

const uint8_t hci_cmd_disable_allscan[] =
{
    0x01, 0x1a, 0x0c, 0x01, 0x00
};

const uint8_t hci_cmd_autoaccept_connect[] =
{
    0x01,0x05, 0x0c, 0x03, 0x02, 0x00, 0x02
};

const uint8_t hci_cmd_hci_reset[] =
{
    0x01,0x03,0x0c,0x00
};

BTDRV_DBG_NONSIG_TESTER_RESULT_BT_T btdrv_testmode_test_result_bt;
BTDRV_DBG_NONSIG_TESTER_RESULT_BLE_T btdrv_testmode_test_result_ble;
static dev_addr_name btdrv_testmode_devinfo;
static uint32_t btdrv_testmode_dut_connect_status = DUT_CONNECT_STATUS_DISCONNECTED;
static volatile uint32_t btdrv_testmode_tx_flag = 1;
static bool btdrv_testmode_hci_has_opened = false;
bool btdrv_testmode_dut_mode_enable = false;
uint8_t testmode_enable = 0;

extern "C" int app_wdt_close(void);
extern "C" void pmu_sleep_en(unsigned char sleep_en);
extern "C" void BESHCI_Close(void);
extern "C" void BESHCI_Open(uint8_t sysfreq_flag);
extern void BESHCI_Clear_Intersys_Tx(void);

uint32_t btdrv_testmode_dut_get_connect_status(void)
{
    return btdrv_testmode_dut_connect_status;
}

void btdrv_testmode_send_data(const uint8_t *buff,uint8_t len)
{
    uint8_t cnt = 250;

    BT_DRV_TRACE(0, "test mode send data");
    BT_DRV_DUMP("%02x ", buff, len);

    btdrv_testmode_tx_flag = 0;

    hal_intersys_send(HAL_INTERSYS_ID_0, HAL_INTERSYS_MSG_HCI, buff, len);

    while((btdrv_testmode_tx_flag == 0) && cnt) {
            cnt--;
            btdrv_delay(1);
        }

    BT_DRV_TRACE(0, "test mode send data exit cnt:%d", cnt);
}

int btdrv_testmode_enable_dut(void)
{
    btdrv_testmode_send_data(hci_cmd_enable_dut, sizeof(hci_cmd_enable_dut));
    btdrv_delay(20);
    btdrv_testmode_send_data(hci_cmd_enable_allscan, sizeof(hci_cmd_enable_allscan));
    btdrv_delay(20);
    btdrv_testmode_send_data(hci_cmd_autoaccept_connect, sizeof(hci_cmd_autoaccept_connect));
    btdrv_delay(20);
    btdrv_testmode_dut_mode_enable = true;
	btdrv_testmode_start();

    return 0;
}

void btdrv_testmode_tx(const unsigned char *data, unsigned int len)
{
    BT_DRV_TRACE(0, "test mode tx irq");
    DUMP8("%02x ", data, len);

    btdrv_testmode_tx_flag = 1;
}

int btdrv_testmode_nonsig_test_result_save(const unsigned char *data, unsigned int len)
{
    BT_DRV_TRACE(1,"%s", __func__);

    const unsigned char nonsig_test_report_bt[] = {0x04, 0x0e, 0x12};

    DBG_NONSIG_TEST_EVENT_IND_T base_test_result_bt;
    DBG_NONSIG_TEST_EVENT_IND_T *pBaseResultBt = NULL;

    const unsigned char nonsig_test_report_ble[] = {0x04, 0x0e, 0x06};

    DBG_NONSIG_TEST_EVENT_IND_BLE_T base_test_result_ble;
    DBG_NONSIG_TEST_EVENT_IND_BLE_T *pBaseResultBle = NULL;

    if (0 == memcmp(data, nonsig_test_report_bt, sizeof(nonsig_test_report_bt))) {
        pBaseResultBt = (DBG_NONSIG_TEST_EVENT_IND_T *)(data + 7);
        if (pBaseResultBt->pkt_counters != 0) {
            memcpy(&base_test_result_bt, pBaseResultBt, sizeof(base_test_result_bt));
            btdrv_testmode_test_result_bt.pkt_counters = base_test_result_bt.pkt_counters;
            btdrv_testmode_test_result_bt.head_errors = base_test_result_bt.head_errors;
            btdrv_testmode_test_result_bt.payload_errors = base_test_result_bt.payload_errors;
            BT_DRV_TRACE(3, "bt result save cnt:%d head:%d payload:%d", btdrv_testmode_test_result_bt.pkt_counters,
                                                        btdrv_testmode_test_result_bt.head_errors, btdrv_testmode_test_result_bt.payload_errors);
        }
    }
    else if (0 == memcmp(data, nonsig_test_report_ble, sizeof(nonsig_test_report_ble))) {
        pBaseResultBle = (DBG_NONSIG_TEST_EVENT_IND_BLE_T *)(data + 7);
        if (pBaseResultBle->pkt_counters != 0) {
            memcpy(&base_test_result_ble, pBaseResultBle, sizeof(base_test_result_ble));
            btdrv_testmode_test_result_ble.pkt_counters = base_test_result_ble.pkt_counters;
            BT_DRV_TRACE(1, "ble result save cnt:%d", btdrv_testmode_test_result_ble.pkt_counters);
        }
    }
    return 0;
}

int btdrv_testmode_bt_nonsig_result_clear(void)
{
    if(btdrv_testmode_test_result_bt.pkt_counters == 0) {
        return 0;
    }

    btdrv_testmode_test_result_bt.pkt_counters = 0;
    btdrv_testmode_test_result_bt.payload_errors = 0;
    btdrv_testmode_test_result_bt.head_errors = 0;

    BT_DRV_TRACE(0, "clear bt test reult");

    return 0;
}

int btdrv_testmode_ble_nonsig_result_clear(void)
{
    if(btdrv_testmode_test_result_ble.pkt_counters == 0) {
        return 0;
    }

    btdrv_testmode_test_result_ble.pkt_counters = 0;

    BT_DRV_TRACE(0, "clear ble test reult");

    return 0;
}


static unsigned int btdrv_test_mode_rx(const unsigned char *data, unsigned int len)
{
    hal_intersys_stop_recv(HAL_INTERSYS_ID_0);

    BT_DRV_TRACE(0, "test mode rx irq");

    DUMP8("%02x ", data, len);

    btdrv_testmode_nonsig_test_result_save(data, len);

    hal_intersys_start_recv(HAL_INTERSYS_ID_0);

    return len;
}

void btdrv_testmode_hci_reset(void)
{
     btdrv_testmode_send_data(hci_cmd_hci_reset, sizeof(hci_cmd_hci_reset));

     btdrv_delay(500);
}

int btdrv_testmode_enable_scan(void)
{
     btdrv_testmode_send_data(hci_cmd_enable_allscan, sizeof(hci_cmd_enable_allscan));
     btdrv_delay(500);

     return 0;
}

int btdrv_testmode_disable_scan(void)
{
     btdrv_testmode_send_data(hci_cmd_disable_allscan, sizeof(hci_cmd_disable_allscan));
     btdrv_delay(500);
     
     return 0;
}

void btdrv_testmode_enable(void)
{
    testmode_enable = 1;
}

uint8_t btdrv_testmode_get_status(void)
{
    return testmode_enable;
}

void btdrv_testmode_disable(void)
{
    testmode_enable = 0;
}

void btdrv_testmode_hciopen(void)
{
    BT_DRV_TRACE(2, "%s:%d", __func__, btdrv_testmode_hci_has_opened);
    int ret = 0;

    if (btdrv_testmode_hci_has_opened)
    {
        return;
    }

    btdrv_testmode_hci_has_opened = true;

    ret = hal_intersys_open(HAL_INTERSYS_ID_0, HAL_INTERSYS_MSG_HCI, btdrv_test_mode_rx, btdrv_testmode_tx, false);

    if (ret)
    {
        BT_DRV_TRACE(0,"Failed to open intersys");
        return;
    }

    hal_intersys_start_recv(HAL_INTERSYS_ID_0);
}

void btdrv_testmode_hcioff(void)
{
    BT_DRV_TRACE(2, "%s:%d", __func__, btdrv_testmode_hci_has_opened);
    if (!btdrv_testmode_hci_has_opened)
    {
        return;
    }
    btdrv_testmode_hci_has_opened = false;

    hal_intersys_close(HAL_INTERSYS_ID_0,HAL_INTERSYS_MSG_HCI);
}

void btdrv_testmode_save_dev_addr_name(void)
{
    btdrv_testmode_devinfo.btd_addr = bt_addr;
    btdrv_testmode_devinfo.ble_addr = ble_addr;
    btdrv_testmode_devinfo.localname = BT_LOCAL_NAME;
    btdrv_testmode_devinfo.ble_name= BT_LOCAL_NAME;
    nvrec_dev_localname_addr_init(&btdrv_testmode_devinfo);
    btdrv_write_localinfo((char *)btdrv_testmode_devinfo.localname, strlen(btdrv_testmode_devinfo.localname) + 1, btdrv_testmode_devinfo.btd_addr);
}

void btdrv_testmode_set_txpower_br(uint32_t power_level_br)
{
    //enter 0~127 to calib br tx power
    BTDIGITAL_REG_SET_FIELD(0xd0350308, 0x3ff, 0, power_level_br);//gsg_nom_q
    BTDIGITAL_REG_SET_FIELD(0xd0350308, 0x3ff, 10, power_level_br);//gsg_nom_i
    BTDIGITAL_REG_SET_FIELD(0xd0350300, 0xf, 0, 7);//gsg_den_q
    BTDIGITAL_REG_SET_FIELD(0xd0350300, 0xf, 4, 7);//gsg_den_i
}

void btdrv_testmode_set_txpower_edr(uint32_t power_level_edr)
{
    //enter 0~127 to calib edr tx power
    BTDIGITAL_REG_SET_FIELD(0xd0350344, 0x3ff, 0, power_level_edr);//dsg_nom
    BTDIGITAL_REG_SET_FIELD(0xd0350340, 0xf, 0, 7);//dsg_den
}

int btdrv_testmode_enter(void)
{
    if (!testmode_enable) 
    {
        BESHCI_Clear_Intersys_Tx();
        btdrv_testmode_enable();

        pmu_sleep_en(0);
        BESHCI_Close();
        //btdrv_hciopen();
        btdrv_ins_patch_test_init();
        bt_drv_reg_op_key_gen_after_reset(false);
        btdrv_testmode_hci_reset();

        btdrv_delay(1000);
        btdrv_testmode_start();
        btdrv_testmode_save_dev_addr_name();
        bt_drv_extra_config_after_init();
        btdrv_hcioff();

        btdrv_testmode_hciopen();
    }
    
     return 0;
}

int btdrv_testmode_exit(void)
{
    if (testmode_enable) {
         btdrv_testmode_hci_reset();
         btdrv_delay(1000);
         btdrv_testmode_hcioff();
         btdrv_poweron(BT_POWEROFF);
         btdrv_start_bt();

         bt_drv_extra_config_after_init();
         BESHCI_Open(0);
         btdrv_testmode_disable();
    }

    return 0;
}

int btdrv_testmode_bt_nonsig_tx(BTDRV_DBG_NONSIG_TESTER_SETUP_SLIM_CMD_TX_T *tx_cfg)
{
     BT_DRV_TRACE(1,"%s\n", __func__);

     btdrv_testmode_hci_reset();

     BTDRV_DBG_NONSIG_TESTER_PKT_TYPE_T *nonsig_pkt_type_p = NULL;

     uint8_t hci_cmd_nonsig_tx_buf[] =
     {
         0x01, 0x87, 0xfc, 0x1c, 0x00, 0xe8, 0x03, 0x00, 0x00, 0x00, 0x00, 0x02, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x01, 0x00, 0x04, 0x04, 0x1b, 0x00,
         0x00, 0x00, 0x00, 0x00, 0xff, 0xff, 0xff, 0xff
     };

     hci_cmd_nonsig_tx_buf[9] = tx_cfg->tx_freq;
     hci_cmd_nonsig_tx_buf[11] = tx_cfg->power_level;

     memcpy(&hci_cmd_nonsig_tx_buf[12], tx_cfg->addr, 6);

     for(uint8_t i = 0; i < BTDRV_DBG_NONSIG_TYPE_END; i++)
     {
         if(btdrv_dbg_nonsig_pkt_type_table[i].enum_pkt_type == tx_cfg->btdrv_pkt_type)
         {
             nonsig_pkt_type_p = &btdrv_dbg_nonsig_pkt_type_table[i];
             break;
         }
     }

     if (!nonsig_pkt_type_p){
         return -1;
     }

     hci_cmd_nonsig_tx_buf[19] = nonsig_pkt_type_p->edr_enabled;
     hci_cmd_nonsig_tx_buf[20] = nonsig_pkt_type_p->packet_type;
     BTDRV_STORELE16(&hci_cmd_nonsig_tx_buf[22], ((nonsig_pkt_type_p->payload_length)));

     btdrv_testmode_send_data(hci_cmd_nonsig_tx_buf, sizeof(hci_cmd_nonsig_tx_buf));

     return 0;
}

int btdrv_testmode_bt_nonsig_rx(BTDRV_DBG_NONSIG_TESTER_SETUP_SLIM_CMD_RX_T *rx_cfg)
{
     BT_DRV_TRACE(1,"%s\n", __func__);

     btdrv_testmode_hci_reset();

     BTDRV_DBG_NONSIG_TESTER_PKT_TYPE_T *nonsig_pkt_type_p = NULL;

     uint8_t hci_cmd_nonsig_rx_buf[] =
     {
         0x01, 0x87, 0xfc, 0x1c, 0x01, 0xe8, 0x03, 0x00, 0x00, 0x00, 0x00, 0x02, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x01, 0x00, 0x04, 0x04, 0x1b, 0x00,
         0x00, 0x00, 0x00, 0x00, 0xff, 0xff, 0xff, 0xff
     };

     hci_cmd_nonsig_rx_buf[10] = rx_cfg->rx_freq;

     memcpy(&hci_cmd_nonsig_rx_buf[12], rx_cfg->addr, 6);

     for(uint8_t i = 0; i < BTDRV_DBG_NONSIG_TYPE_END; i++)
     {
         if(btdrv_dbg_nonsig_pkt_type_table[i].enum_pkt_type == rx_cfg->btdrv_pkt_type)
         {
             nonsig_pkt_type_p = &btdrv_dbg_nonsig_pkt_type_table[i];
             break;
         }
     }

     if (!nonsig_pkt_type_p){
         return -1;
     }

     hci_cmd_nonsig_rx_buf[19] = nonsig_pkt_type_p->edr_enabled;
     hci_cmd_nonsig_rx_buf[20] = nonsig_pkt_type_p->packet_type;
     BTDRV_STORELE16(&hci_cmd_nonsig_rx_buf[22], ((nonsig_pkt_type_p->payload_length)));

     btdrv_testmode_send_data(hci_cmd_nonsig_rx_buf, sizeof(hci_cmd_nonsig_rx_buf));

     return 0;
}

int btdrv_testmode_bt_nonsig_endtest()
{
    uint16_t val;

    btdrv_read_rf_reg(0xA7, &val);

    BT_DRV_TRACE(2, "%s RF REG 0xA7:0x%04x", __func__, val);

     uint8_t hci_cmd_nonsig_end_test_buf[] =
     {
         0x01, 0x87, 0xfc, 0x1c, 0x02, 0xe8, 0x03, 0x00, 0x00, 0x00, 0x00, 0x02, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x01, 0x01, 0x04, 0x04, 0x36, 0x00,
         0x00, 0x00, 0x00, 0x00, 0xff, 0xff, 0xff, 0xff
     };

     btdrv_testmode_send_data(hci_cmd_nonsig_end_test_buf, sizeof(hci_cmd_nonsig_end_test_buf));

     return 0;
}

int btdrv_testmode_bt_nonsig_result_get(BTDRV_DBG_NONSIG_TESTER_RESULT_BT_T *bt_result)
{
    if(!bt_result) {
        return -1;
    }

    bt_result->pkt_counters = btdrv_testmode_test_result_bt.pkt_counters;
    bt_result->payload_errors = btdrv_testmode_test_result_bt.payload_errors;
    bt_result->head_errors = btdrv_testmode_test_result_bt.head_errors;

    BT_DRV_TRACE(3, "final bt test result cnt:%d head:%d payload:%d", bt_result->pkt_counters, bt_result->head_errors, bt_result->payload_errors);

    btdrv_testmode_bt_nonsig_result_clear();

    return 0;
}

int btdrv_testmode_ble_nonsig_tx(BTDRV_DBG_NONSIG_TESTER_SETUP_CMD_BLE_TX_T *ble_tx_cfg)
{
     BT_DRV_TRACE(1,"%s\n", __func__);

     btdrv_testmode_hci_reset();

     uint8_t hci_cmd_nonsig_ble_tx_buf[] =
     {
         0x01, 0x1e, 0x20, 0x03, 0x00, 0x25, 0x00
     };

     hci_cmd_nonsig_ble_tx_buf[4] = ble_tx_cfg->tx_freq;

     btdrv_testmode_send_data(hci_cmd_nonsig_ble_tx_buf, sizeof(hci_cmd_nonsig_ble_tx_buf));

     return 0;
}

int btdrv_testmode_ble_nonsig_rx(BTDRV_DBG_NONSIG_TESTER_SETUP_CMD_BLE_RX_T *ble_rx_cfg)
{
    BT_DRV_TRACE(1,"%s\n", __func__);

    btdrv_testmode_hci_reset();

    uint8_t hci_cmd_nonsig_ble_rx_buf[] =
    {
        0x01, 0x1d, 0x20, 0x01, 0x00
    };

    hci_cmd_nonsig_ble_rx_buf[4] = ble_rx_cfg->rx_freq;

    btdrv_testmode_send_data(hci_cmd_nonsig_ble_rx_buf, sizeof(hci_cmd_nonsig_ble_rx_buf));

    return 0;
}

int btdrv_testmode_ble_nonsig_endtest(void)
{
    BT_DRV_TRACE(1,"%s\n", __func__);

    uint8_t hci_cmd_nonsig_ble_end_test_buf[] =
    {
        0x01, 0x1f, 0x20, 0x00
    };

    btdrv_testmode_send_data(hci_cmd_nonsig_ble_end_test_buf, sizeof(hci_cmd_nonsig_ble_end_test_buf));

    return 0;
}

int btdrv_testmode_ble_nonsig_result_get(BTDRV_DBG_NONSIG_TESTER_RESULT_BLE_T *ble_result)
{
    if(!ble_result) {
        return -1;
    }

    ble_result->pkt_counters = btdrv_testmode_test_result_ble.pkt_counters;

    BT_DRV_TRACE(1, "final ble test cnt:%d", ble_result->pkt_counters);

    btdrv_testmode_ble_nonsig_result_clear();

    return 0;
}
