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
#include "plat_types.h"
#include "spp_api.h"
#include "sdp_api.h"
#include "tota_stream_data_transfer.h"
#include "app_tota_common.h"
#if(OTA_OVER_TOTA_ENABLED)
#include "ota_control.h"
#include "ota_bes.h"
#include "ota_spp.h"
#endif

tota_spp_ctl_t tota_spp_ctl = {
    .isBusy         = false,
    .isConnected    = false,
    .pSppDevice     = NULL,
};


// Semaphore
osSemaphoreDef(tota_spp_tx_sem);
// Mutex
osMutexDef(tota_spp_mutex);
osMutexDef(tota_credit_mutex);
osMutexDef(tota_spp_tx_mutex);

// static inline void _update_tx_buf(void);
// static inline uint8_t * _get_tx_buf_ptr(void);

/* is tota busy, use to handle sniff */
bool spp_tota_in_progress(void)
{
    return is_stream_data_running();
}

/****************************************************************************
 * TOTA SPP SDP Entries
 ****************************************************************************/

/*---------------------------------------------------------------------------
 *
 * ServiceClassIDList 
 */
static const U8 TotaSppClassId[] = {
    SDP_ATTRIB_HEADER_8BIT(3),        /* Data Element Sequence, 6 bytes */ 
    SDP_UUID_16BIT(SC_SERIAL_PORT),     /* Hands-Free UUID in Big Endian */ 
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

// static const U8 TotaSppServiceName2[] = {
//     SDP_TEXT_8BIT(5),          /* Null terminated text string */ 
//     'S', 'p', 'p', '2', '\0'
// };

/* SPP attributes.
 *
 * This is a ROM template for the RAM structure used to register the
 * SPP SDP record.
 */
//static const SdpAttribute TotaSppSdpAttributes1[] = {
static sdp_attribute_t TotaSppSdpAttributes1[] = {

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

int tota_spp_handle_data_event_func(void *pDev, uint8_t process, uint8_t *pData, uint16_t dataLen)
{
    TOTA_LOG_DBG(1,"spp rx:%d", dataLen);
    TOTA_LOG_DUMP("[0x%x]", pData, dataLen);

    tota_spp_ctl.callBack->rx_cb(pData, dataLen);
    return 0;
}


static void spp_tota_callback(struct spp_device *locDev, struct spp_callback_parms *Info)
{
    switch ( Info->event )
    {
    case BTIF_SPP_EVENT_REMDEV_CONNECTED:
        TOTA_LOG_DBG(0, "::BTIF_SPP_EVENT_REMDEV_CONNECTED");
        tota_spp_ctl.isConnected = true;
#if(OTA_OVER_TOTA_ENABLED)
        ota_control_update_MTU(MAX_SPP_PACKET_SIZE - TOTA_PACKET_VERIFY_SIZE);
        ota_control_register_transmitter(ota_spp_tota_send_data);
#endif
        if (tota_spp_ctl.callBack->connected_cb)
        {
            tota_spp_ctl.callBack->connected_cb();
        }
        break;
    case BTIF_SPP_EVENT_REMDEV_DISCONNECTED:
        TOTA_LOG_DBG(0, "::BTIF_SPP_EVENT_REMDEV_DISCONNECTED");
        tota_spp_ctl.isConnected = false;
#if(OTA_OVER_TOTA_ENABLED)
        ota_control_set_datapath_type(NO_OTA_CONNECTION);
#endif
        if (tota_spp_ctl.callBack->disconnected_cb)
        {
            tota_spp_ctl.callBack->disconnected_cb();
        }
        break;
    case BTIF_SPP_EVENT_DATA_SENT:
        TOTA_LOG_DBG(0, "::BTIF_SPP_EVENT_DATA_SENT");
        //osSemaphoreRelease(tota_spp_ctl.txSem);
        if (tota_spp_ctl.callBack->tx_done_cb)
        {
            tota_spp_ctl.callBack->tx_done_cb();
        }
        break;
    default:
        TOTA_LOG_DBG(1,"::unknown event 0x%x", Info->event);
        break;
    }
}

void app_spp_tota_init(const tota_callback_func_t *tota_callback_func)
{
    osMutexId mid;
    btif_sdp_record_param_t param;
    btif_sdp_record_t *pSdpRecord = NULL;
    struct spp_service *pSppService = NULL;

    tota_spp_ctl.txSem = osSemaphoreCreate(osSemaphore(tota_spp_tx_sem), 1);
    tota_spp_ctl.txMutex = osMutexCreate(osMutex(tota_spp_tx_mutex));
    tota_spp_ctl.callBack = tota_callback_func;

    if( tota_spp_ctl.pSppDevice == NULL )
    {
        tota_spp_ctl.pSppDevice = btif_create_spp_device();
    }
    
    tota_spp_ctl.pSppDevice->rx_buffer = tota_spp_ctl.prxBuff;
    btif_spp_init_rx_buf(tota_spp_ctl.pSppDevice, tota_spp_ctl.prxBuff, MAX_SPP_PACKET_SIZE*MAX_SPP_PACKET_NUM);

    mid = osMutexCreate(osMutex(tota_spp_mutex));
    if (!mid)
    {
        ASSERT(0, "cannot create mutex");
    }

    //tota_spp_ctl.creditMutex = osMutexCreate(osMutex(tota_credit_mutex));

    pSdpRecord = btif_sdp_create_record();

    param.attrs = &TotaSppSdpAttributes1[0],
    param.attr_count = ARRAY_SIZE(TotaSppSdpAttributes1);
    param.COD = BTIF_COD_MAJOR_PERIPHERAL;
    btif_sdp_record_setup(pSdpRecord, &param);

    pSppService = btif_create_spp_service();

    pSppService->rf_service.serviceId = RFCOMM_CHANNEL_TOTA;
    pSppService->numPorts = 0;
    btif_spp_service_setup(tota_spp_ctl.pSppDevice, pSppService, pSdpRecord);

    tota_spp_ctl.pSppDevice->portType = BTIF_SPP_SERVER_PORT;
    tota_spp_ctl.pSppDevice->app_id = BTIF_APP_SPP_SERVER_TOTA_ID;
    tota_spp_ctl.pSppDevice->spp_handle_data_event_func = tota_spp_handle_data_event_func;
    btif_spp_init_device(tota_spp_ctl.pSppDevice, MAX_SPP_PACKET_NUM, mid);

    btif_spp_open(tota_spp_ctl.pSppDevice, NULL, spp_tota_callback);
}

/* this func is safe in thread */
bool app_spp_tota_send_data(uint8_t* ptrData, uint16_t length)
{
    if (!tota_spp_ctl.isConnected)
    {
        return false;
    }

    bt_status_t ret = BT_STS_SUCCESS;
    osMutexWait(tota_spp_ctl.txMutex, osWaitForever);
    //osSemaphoreWait(tota_spp_ctl.txSem, osWaitForever);

    memcpy(tota_spp_ctl.ptxBuff, ptrData, length);
    TOTA_LOG_DBG(1, "spp tx:%d", length);
    ret = btif_spp_write(tota_spp_ctl.pSppDevice, (char *)tota_spp_ctl.ptxBuff, &length);
    osMutexRelease(tota_spp_ctl.txMutex);

    if (BT_STS_SUCCESS != ret)
    {
        return false;
    }
    else
    {
        return true;
    }
}

// static inline void _update_tx_buf(void)
// {
//     tota_spp_ctl.txIndex = (tota_spp_ctl.txIndex + 1) % MAX_SPP_PACKET_NUM;
// }

// static inline uint8_t * _get_tx_buf_ptr(void)
// {
//     return (tota_spp_ctl.txBuff + tota_spp_ctl.txIndex*MAX_SPP_PACKET_SIZE);
// }
