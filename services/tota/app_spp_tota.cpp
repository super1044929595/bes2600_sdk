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
#include <stdio.h>
#include "cmsis_os.h"
#include "hal_uart.h"
#include "hal_timer.h"
#include "audioflinger.h"
#include "lockcqueue.h"
#include "hal_trace.h"
#include "hal_cmu.h"
#include "hal_chipid.h"
#include "analog.h"
#include "app_audio.h"
#include "app_status_ind.h"
#include "app_bt_stream.h"
#include "nvrecord.h"
#include "nvrecord_env.h"
//#include "nvrecord_dev.h"

#include "bluetooth.h"
#include "cqueue.h"
#include "resources.h"
#include "app_spp_tota.h"
#include "app_tota_cmd_code.h"
#include "app_tota.h"
#include "app_spp.h"
#include "app_tota_cmd_handler.h"
#include "app_tota_data_handler.h"
//#include "anc_prase_data.h"
#include "plat_types.h"
#include "spp_api.h"
#include "sdp_api.h"
//#include "app_bt_conn_mgr.h"
#ifdef APP_ANC_TEST
#include "anc_parse_data.h"
#endif

#ifdef IBRT
#include "app_tws_ibrt.h"
#include "app_ibrt_ui.h"
#include "app_ibrt_if.h"
#endif

#if 0
static void tota_spp_read_thread(const void *arg);
osThreadDef(tota_spp_read_thread, osPriorityAboveNormal, 1, 3072, "tota_spp_read");
osThreadId  tota_spp_read_thread_id = NULL;
#endif

static bool isTotaSppConnected = false;
static struct spp_device  *tota_spp_dev = NULL;
static struct spp_service  *totaSppService = NULL;
osMutexDef(tota_spp_mutex);

static btif_sdp_record_t *tota_sdp_record = NULL;

static app_spp_tota_tx_done_t app_spp_tota_tx_done_func = NULL;

static uint8_t totaSppTxBuf[TOTA_SPP_TX_BUF_SIZE];
static uint8_t spp_rx_buf[SPP_RECV_BUFFER_SIZE];

static uint32_t occupiedTotaSppTxBufSize;
static uint32_t offsetToFillTotaSppTxData;
static uint8_t* ptrTotaSppTxBuf;

#if (TOTA_SHARE_TX_RX_BUF==1)
static const bool spp_share_application_rx_tx_buf = true;
#else
static const bool spp_share_application_rx_tx_buf = false;
#endif
void app_spp_tota_share_buf_create(uint8_t** tota_tx_buf, uint8_t** tota_rx_buf)
{
    *tota_tx_buf = NULL;
    *tota_rx_buf = NULL;

    if(spp_share_application_rx_tx_buf == true){
        *tota_tx_buf = totaSppTxBuf;
        *tota_rx_buf = spp_rx_buf;
    }
}

bool app_spp_tota_is_to_share_buf(void)
{
    return spp_share_application_rx_tx_buf;
}

uint8_t * app_spp_tota_share_tx_buf_get(void)
{
    return (spp_share_application_rx_tx_buf == true)?(totaSppTxBuf):(NULL);
}
uint8_t * app_spp_tota_share_rx_buf_get(void)
{
    return (spp_share_application_rx_tx_buf == true)?(spp_rx_buf):(NULL);
}

uint16_t app_spp_tota_tx_buf_size(void)
{
    return TOTA_SPP_TX_BUF_SIZE;
}

void app_spp_tota_init_tx_buf(uint8_t* ptr)
{
    ptrTotaSppTxBuf = ptr;
    occupiedTotaSppTxBufSize = 0;
    offsetToFillTotaSppTxData = 0;
}

static void app_spp_tota_free_tx_buf(uint8_t* ptrData, uint32_t dataLen)
{
    if (occupiedTotaSppTxBufSize > 0)
    {
        occupiedTotaSppTxBufSize -= dataLen;
    }
    TOTA_LOG_DBG(1,"occupiedTotaSppTxBufSize %d", occupiedTotaSppTxBufSize);
}

#if 1
uint8_t* app_spp_tota_fill_data_into_tx_buf(uint8_t* ptrData, uint32_t dataLen)
{
    ASSERT((occupiedTotaSppTxBufSize + dataLen) < TOTA_SPP_TX_BUF_SIZE, 
        "Pending SPP tx data has exceeded the tx buffer size !");

        
    if ((offsetToFillTotaSppTxData + dataLen) > TOTA_SPP_TX_BUF_SIZE)
    {
        offsetToFillTotaSppTxData = 0;
    }

    uint8_t* filledPtr = ptrTotaSppTxBuf + offsetToFillTotaSppTxData;
    memcpy(filledPtr, ptrData, dataLen);

    offsetToFillTotaSppTxData += dataLen;

    occupiedTotaSppTxBufSize += dataLen;

    TOTA_LOG_DBG(3,"dataLen %d offsetToFillTotaSppTxData %d occupiedTotaSppTxBufSize %d",
        dataLen, offsetToFillTotaSppTxData, occupiedTotaSppTxBufSize);
    
    return filledPtr;
}
#else
uint8_t* app_spp_tota_fill_data_into_tx_buf(uint8_t* ptrData, uint32_t dataLen)
{
    TOTA_LOG_DBG(3,"dataLen %d offsetToFillTotaSppTxData %d occupiedTotaSppTxBufSize %d",
        dataLen, offsetToFillTotaSppTxData, occupiedTotaSppTxBufSize);

    ASSERT((occupiedTotaSppTxBufSize + dataLen) < TOTA_SPP_TX_BUF_SIZE, 
        "Pending SPP tx data has exceeded the tx buffer size !");

    uint8_t* filledPtr = ptrTotaSppTxBuf + offsetToFillTotaSppTxData;
    memcpy(filledPtr, ptrData, dataLen);
        
    if ((offsetToFillTotaSppTxData + dataLen) > TOTA_SPP_TX_BUF_SIZE)
    {        
        offsetToFillTotaSppTxData = 0;
    }
    else
    {
        offsetToFillTotaSppTxData += dataLen;
    }

    occupiedTotaSppTxBufSize += dataLen;

    TOTA_LOG_DBG(3,"dataLen %d offsetToFillTotaSppTxData %d occupiedTotaSppTxBufSize %d",
        dataLen, offsetToFillTotaSppTxData, occupiedTotaSppTxBufSize);
    
    return filledPtr;
}
#endif

extern "C" APP_TOTA_CMD_RET_STATUS_E app_tota_data_received(uint8_t* ptrData, uint32_t dataLength);
extern "C" APP_TOTA_CMD_RET_STATUS_E app_tota_cmd_received(uint8_t* ptrData, uint32_t dataLength);


/****************************************************************************
 * TOTA SPP SDP Entries
 ****************************************************************************/

/*---------------------------------------------------------------------------
 *
 * ServiceClassIDList 
 */
static const U8 TotaSppClassId[] = {
#if 0
    SDP_ATTRIB_HEADER_8BIT(3),        /* Data Element Sequence, 6 bytes */ 
    SDP_UUID_16BIT(SC_SERIAL_PORT),     /* Hands-Free UUID in Big Endian */ 
#else
    SDP_ATTRIB_HEADER_8BIT(17),
    DETD_UUID + DESD_16BYTES,
     0x8a,
     0x48,\
     0x2a,\
     0x08,\
     0x55,\
     0x07,\
     0x42,\
     0xac,\
     0xb6,\
     0x73,\
     0xa8,\
     0x8d,\
     0xf4,\
     0x8b,\
     0x3f,\
     0xc7,

#endif
};

static const U8 TotaSppProtoDescList[] = {
    SDP_ATTRIB_HEADER_8BIT(12),  /* Data element sequence, 12 bytes */ 

    /* Each element of the list is a Protocol descriptor which is a 
     * data element sequence. The first element is L2CAP which only 
     * has a UUID element.  
     */ 
    SDP_ATTRIB_HEADER_8BIT(3),   /* Data element sequence for L2CAP, 3 
                                  * bytes 
                                  */ 

    SDP_UUID_16BIT(PROT_L2CAP),  /* Uuid16 L2CAP */ 

    /* Next protocol descriptor in the list is RFCOMM. It contains two 
     * elements which are the UUID and the channel. Ultimately this 
     * channel will need to filled in with value returned by RFCOMM.  
     */ 

    /* Data element sequence for RFCOMM, 5 bytes */
    SDP_ATTRIB_HEADER_8BIT(5),

    SDP_UUID_16BIT(PROT_RFCOMM), /* Uuid16 RFCOMM */

    /* Uint8 RFCOMM channel number - value can vary */
    SDP_UINT_8BIT(RFCOMM_CHANNEL_TOTA)
};

/*
 * BluetoothProfileDescriptorList
 */
static const U8 TotaSppProfileDescList[] = {
    SDP_ATTRIB_HEADER_8BIT(8),        /* Data element sequence, 8 bytes */

    /* Data element sequence for ProfileDescriptor, 6 bytes */
    SDP_ATTRIB_HEADER_8BIT(6),

    SDP_UUID_16BIT(SC_SERIAL_PORT),   /* Uuid16 SPP */
    SDP_UINT_16BIT(0x0102)            /* As per errata 2239 */ 
};

/*
 * * OPTIONAL *  ServiceName
 */
static const U8 TotaSppServiceName1[] = {
    SDP_TEXT_8BIT(5),          /* Null terminated text string */ 
    'S', 'p', 'p', '1', '\0'
};

static const U8 TotaSppServiceName2[] = {
    SDP_TEXT_8BIT(5),          /* Null terminated text string */ 
    'S', 'p', 'p', '2', '\0'
};

/* SPP attributes.
 *
 * This is a ROM template for the RAM structure used to register the
 * SPP SDP record.
 */
//static const SdpAttribute TotaSppSdpAttributes1[] = {
static const sdp_attribute_t TotaSppSdpAttributes1[] = {

    SDP_ATTRIBUTE(AID_SERVICE_CLASS_ID_LIST, TotaSppClassId), 

    SDP_ATTRIBUTE(AID_PROTOCOL_DESC_LIST, TotaSppProtoDescList),

    SDP_ATTRIBUTE(AID_BT_PROFILE_DESC_LIST, TotaSppProfileDescList),

    /* SPP service name*/
    SDP_ATTRIBUTE((AID_SERVICE_NAME + 0x0100), TotaSppServiceName1),
};

/*
static sdp_attribute_t TotaSppSdpAttributes2[] = {

    SDP_ATTRIBUTE(AID_SERVICE_CLASS_ID_LIST, TotaSppClassId), 

    SDP_ATTRIBUTE(AID_PROTOCOL_DESC_LIST, TotaSppProtoDescList),

    SDP_ATTRIBUTE(AID_BT_PROFILE_DESC_LIST, TotaSppProfileDescList),

    
    SDP_ATTRIBUTE((AID_SERVICE_NAME + 0x0100), TotaSppServiceName2),
};
*/

extern "C" void reset_programmer_state(unsigned char **buf, size_t *len);
extern unsigned char *g_buf;
extern size_t g_len;

int tota_spp_handle_data_event_func(void *pDev, uint8_t process, uint8_t *pData, uint16_t dataLen)
{
    TOTA_LOG_DBG(2,"[%s]data receive length = %d", __func__, dataLen);
    TOTA_LOG_DUMP("[0x%x]", pData, dataLen);
#if defined(APP_ANC_TEST)
    app_anc_tota_cmd_received(pData, (uint32_t)dataLen);
#else
    // the first two bytes of the data packet is the fixed value 0xFFFF
    app_tota_handle_received_data(pData, dataLen);
#endif

    return 0;
}

#if 0
static void app_spp_tota_create_read_thread(void)
{
    TOTA_LOG_DBG(2,"%s %d\n", __func__, __LINE__);
    tota_spp_read_thread_id = osThreadCreate(osThread(tota_spp_read_thread), NULL);
}

static void app_spp_tota_close_read_thread(void)
{
    TOTA_LOG_DBG(2,"%s %d\n", __func__, __LINE__);
    if(tota_spp_read_thread_id)
    {
        osThreadTerminate(tota_spp_read_thread_id);
        tota_spp_read_thread_id = NULL;
    }
}
#endif

#if defined(APP_ANC_TEST)
static void app_synccmd_timehandler(void const *param);
osTimerDef (APP_SYNCCMD, app_synccmd_timehandler);
osTimerId app_check_send_synccmd_timer = NULL;
extern "C" void send_sync_cmd_to_tool();
static void app_synccmd_timehandler(void const *param)
{
    send_sync_cmd_to_tool();
}
#endif

static void spp_tota_enter_handler(void)
{
#ifdef IBRT
    ibrt_ctrl_t *p_ibrt_ctrl = app_tws_ibrt_get_bt_ctrl_ctx();

    app_ibrt_ui_judge_link_policy(TOTA_START_TRIGGER, BTIF_BLP_DISABLE_ALL);

    if (app_tws_ibrt_tws_link_connected() && \
        (p_ibrt_ctrl->nv_role == IBRT_MASTER) && \
        p_ibrt_ctrl->p_tws_remote_dev)
    {
        btif_me_stop_sniff(p_ibrt_ctrl->p_tws_remote_dev);
    }
    if(app_tws_ibrt_mobile_link_connected() &&
        p_ibrt_ctrl->p_mobile_remote_dev)
    {
        btif_me_stop_sniff(p_ibrt_ctrl->p_mobile_remote_dev);
    }
#else
    app_bt_active_mode_set(ACTIVE_MODE_KEEPER_OTA, UPDATE_ACTIVE_MODE_FOR_ALL_LINKS);
#endif

}

static void spp_tota_exit_handler()
{
#ifdef IBRT
    app_ibrt_ui_judge_link_policy(OTA_STOP_TRIGGER, BTIF_BLP_SNIFF_MODE);
#else
    app_bt_active_mode_clear(ACTIVE_MODE_KEEPER_OTA, UPDATE_ACTIVE_MODE_FOR_ALL_LINKS);
#endif
    
}


static void spp_tota_callback(struct spp_device *locDev, struct spp_callback_parms *Info)
{
    if (BTIF_SPP_EVENT_REMDEV_CONNECTED == Info->event)
    {
        TOTA_LOG_DBG(1,"::BTIF_SPP_EVENT_REMDEV_CONNECTED %d\n", Info->event);
        isTotaSppConnected = true;
        spp_tota_enter_handler();
        //app_spp_tota_create_read_thread();
        app_tota_connected(APP_TOTA_CONNECTED);
        app_tota_update_datapath(APP_TOTA_VIA_SPP);
        //conn_stop_connecting_mobile_supervising();
#if defined(APP_ANC_TEST)
        anc_data_buff_init();
        //add a send sync timer
        osTimerStop(app_check_send_synccmd_timer);
        osTimerStart(app_check_send_synccmd_timer, 2000);
#endif
    }
    else if (BTIF_SPP_EVENT_REMDEV_DISCONNECTED == Info->event)
    {
        TOTA_LOG_DBG(1,"::BTIF_SPP_EVENT_REMDEV_DISCONNECTED %d\n", Info->event);
        isTotaSppConnected = false;
        //app_spp_tota_close_read_thread();
        app_tota_disconnected(APP_TOTA_DISCONNECTED);
        app_tota_update_datapath(APP_TOTA_PATH_IDLE);

#if defined(APP_ANC_TEST)
        anc_data_buff_deinit();
        osTimerStop(app_check_send_synccmd_timer);
#endif
        app_spp_tota_tx_done_func = NULL;
        spp_tota_exit_handler();
    }
    else if (BTIF_SPP_EVENT_DATA_SENT == Info->event)
    {
        //app_spp_tota_free_tx_buf(Info->tx_buf, Info->tx_data_len);
        struct spp_tx_done *pTxDone = (struct spp_tx_done *)(Info->p.other);
        app_spp_tota_free_tx_buf(pTxDone->tx_buf, pTxDone->tx_data_length);
        if (app_spp_tota_tx_done_func)
        {
            app_spp_tota_tx_done_func();
        }
    }
    else
    {
        TOTA_LOG_DBG(1,"::unknown event %d\n", Info->event);
    }
}

static void app_spp_tota_send_data(uint8_t* ptrData, uint16_t length)
{
    if (!isTotaSppConnected)
    {
        return;
    }

    btif_spp_write(tota_spp_dev, (char *)ptrData, &length);
}

void app_tota_send_cmd_via_spp(uint8_t* ptrData, uint32_t length)
{
    uint8_t* ptrBuf = app_spp_tota_fill_data_into_tx_buf(ptrData, length);
    app_spp_tota_send_data(ptrBuf, (uint16_t)length);
}

void app_tota_send_data_via_spp(uint8_t* ptrData, uint32_t length)
{
    TOTA_LOG_DBG(2,"[%s]tota send data length = %d",__func__,length);
    uint8_t* ptrBuf = app_spp_tota_fill_data_into_tx_buf(ptrData, length);
    app_spp_tota_send_data(ptrBuf, (uint16_t)length);
}

void app_spp_tota_register_tx_done(app_spp_tota_tx_done_t callback)
{
    app_spp_tota_tx_done_func = callback;
}

void app_spp_tota_init(void)
{
    uint8_t *rx_buf;
    uint8_t * tx_buf;
    osMutexId mid;
    btif_sdp_record_param_t param;

    if(tota_spp_dev == NULL)
        tota_spp_dev = btif_create_spp_device();

    if(app_spp_tota_is_to_share_buf() == true){
        app_spp_tota_share_buf_create(&tx_buf,&rx_buf);
    }else{
        rx_buf = &spp_rx_buf[0];
        tx_buf = &totaSppTxBuf[0];
    }
    
    tota_spp_dev->rx_buffer = rx_buf;

    app_spp_tota_init_tx_buf(tx_buf);
    btif_spp_init_rx_buf(tota_spp_dev, rx_buf, SPP_RECV_BUFFER_SIZE);


    mid = osMutexCreate(osMutex(tota_spp_mutex));
    if (!mid) {
        ASSERT(0, "cannot create mutex");
    }

    if (tota_sdp_record == NULL)
        tota_sdp_record = btif_sdp_create_record();

    param.attrs = (sdp_attribute_t*)&TotaSppSdpAttributes1[0],
    param.attr_count = ARRAY_SIZE(TotaSppSdpAttributes1);
    param.COD = BTIF_COD_MAJOR_PERIPHERAL;
    btif_sdp_record_setup(tota_sdp_record, &param);

    if(totaSppService == NULL)
        totaSppService = btif_create_spp_service();

    totaSppService->rf_service.serviceId = RFCOMM_CHANNEL_TOTA;
    totaSppService->numPorts = 0;
    btif_spp_service_setup(tota_spp_dev, totaSppService, tota_sdp_record);

    tota_spp_dev->portType = BTIF_SPP_SERVER_PORT;
    tota_spp_dev->app_id = BTIF_APP_SPP_SERVER_TOTA_ID;
    tota_spp_dev->spp_handle_data_event_func = tota_spp_handle_data_event_func;
    btif_spp_init_device(tota_spp_dev, 5, mid);
#if defined(APP_ANC_TEST)
    reset_programmer_state(&g_buf, &g_len);
#endif

    btif_spp_open(tota_spp_dev, NULL,  spp_tota_callback);
#if defined(APP_ANC_TEST)
    if (app_check_send_synccmd_timer == NULL)
        app_check_send_synccmd_timer = osTimerCreate (osTimer(APP_SYNCCMD), osTimerPeriodic, NULL);
#endif
}

/* for sniff */
bool spp_tota_in_progress(void)
{
    TOTA_LOG_DBG(2,"[%s] isTotaSppConnected:%d", __func__, isTotaSppConnected);
    if(isTotaSppConnected == true)
        return true;
    else
        return false;
}

