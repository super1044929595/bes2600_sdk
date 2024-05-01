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
 *
 *	Leonardo		2018/12/20
 ****************************************************************************/
#ifdef BES_OTA_BASIC
 
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
#include "nvrecord_env.h"
#include "bluetooth.h"
#include "cqueue.h"
#include "resources.h"
#include "ota_spp.h"
#include "ota_bes.h"
#include "app_spp.h"
#include "plat_types.h"

#if 0
static void ota_spp_read_thread(const void *arg);
osThreadDef(ota_spp_read_thread, osPriorityAboveNormal, 1, 3072, "ota_spp_read");
osThreadId  ota_spp_read_thread_id = NULL;
#endif

osMutexDef(ota_spp_mutex);
static void app_spp_ota_send_data(uint8_t* ptrData, uint16_t length);


static app_spp_ota_tx_done_t app_spp_ota_tx_done_func = NULL;

static uint32_t occupiedOtaSppTxBufSize;
static uint32_t offsetToFillOtaSppTxData;
static uint8_t* ptrOtaSppTxBuf;

OtaContext ota;

uint16_t app_spp_ota_tx_buf_size(void)
{
    return OTA_SPP_TX_BUF_SIZE;
}

void app_spp_ota_init_tx_buf(uint8_t* ptr)
{
    ptrOtaSppTxBuf = ptr;
    occupiedOtaSppTxBufSize = 0;
    offsetToFillOtaSppTxData = 0;
}

static void app_spp_ota_free_tx_buf(uint8_t* ptrData, uint32_t dataLen)
{
    if (occupiedOtaSppTxBufSize > 0)
    {
        occupiedOtaSppTxBufSize -= dataLen;
    }
    LOG_DBG("occupiedOtaSppTxBufSize %d", occupiedOtaSppTxBufSize);
}

uint8_t* app_spp_ota_fill_data_into_tx_buf(uint8_t* ptrData, uint32_t dataLen)
{
    ASSERT((occupiedOtaSppTxBufSize + dataLen) < OTA_SPP_TX_BUF_SIZE, 
        "Pending SPP tx data has exceeded the tx buffer size !");

        
    if ((offsetToFillOtaSppTxData + dataLen) > OTA_SPP_TX_BUF_SIZE)
    {
        offsetToFillOtaSppTxData = 0;
    }

    uint8_t* filledPtr = ptrOtaSppTxBuf + offsetToFillOtaSppTxData;
    memcpy(filledPtr, ptrData, dataLen);

    offsetToFillOtaSppTxData += dataLen;

    occupiedOtaSppTxBufSize += dataLen;

    LOG_DBG("dataLen %d offsetToFillOtaSppTxData %d occupiedOtaSppTxBufSize %d",
        dataLen, offsetToFillOtaSppTxData, occupiedOtaSppTxBufSize);
    
    return filledPtr;
}

/****************************************************************************
 * OTA SPP SDP Entries
 ****************************************************************************/

static const uint8_t OTA_SPP_UUID_128[16] = BES_OTA_UUID_128;
/*---------------------------------------------------------------------------
 *
 * ServiceClassIDList 
 */
static const U8 OtaSppClassId[] = {
  SDP_ATTRIB_HEADER_8BIT(17),        /* Data Element Sequence, 17 bytes */
  SDP_UUID_128BIT(OTA_SPP_UUID_128),    /* 128 bit UUID in Big Endian */
};

static const U8 OtaSppProtoDescList[] = {
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
    SDP_UINT_8BIT(RFCOMM_CHANNEL_BES_OTA)
};

/*
 * BluetoothProfileDescriptorList
 */
static const U8 OtaSppProfileDescList[] = {
    SDP_ATTRIB_HEADER_8BIT(8),        /* Data element sequence, 8 bytes */

    /* Data element sequence for ProfileDescriptor, 6 bytes */
    SDP_ATTRIB_HEADER_8BIT(6),

    SDP_UUID_16BIT(SC_SERIAL_PORT),   /* Uuid16 SPP */
    SDP_UINT_16BIT(0x0102)            /* As per errata 2239 */ 
};

/*
 * * OPTIONAL *  ServiceName
 */
static const U8 OtaSppServiceName[] = {
    SDP_TEXT_8BIT(5),          /* Null terminated text string */ 
    'O', 'T', 'A', '1', '\0'
};

/* SPP attributes.
 *
 * This is a ROM template for the RAM structure used to register the
 * SPP SDP record.
 */
//static const SdpAttribute OtaSppSdpAttributes[] = {
static const sdp_attribute_t OtaSppSdpAttributes[] = {

    SDP_ATTRIBUTE(AID_SERVICE_CLASS_ID_LIST, OtaSppClassId), 

    SDP_ATTRIBUTE(AID_PROTOCOL_DESC_LIST, OtaSppProtoDescList),

    SDP_ATTRIBUTE(AID_BT_PROFILE_DESC_LIST, OtaSppProfileDescList),

    /* SPP service name*/
    SDP_ATTRIBUTE((AID_SERVICE_NAME + 0x0100), OtaSppServiceName),
};

int ota_spp_handle_data_event_func(void *pDev, uint8_t process, uint8_t *pData, uint16_t dataLen)
{
    LOG_DBG("SPP1 receive pData %p length=%d", pData, dataLen);
    LOG_DUMP("0x%02x ", pData, (dataLen>15)?15:dataLen);
    ota_bes_handle_received_data(pData, false, dataLen);
	
	return 0;
}

#if 0
static void app_spp_ota_create_read_thread(void)
{
    LOG_DBG("%s %d\n", __func__, __LINE__);
    ota_spp_read_thread_id = osThreadCreate(osThread(ota_spp_read_thread), NULL);
}

static void app_spp_ota_close_read_thread(void)
{
    LOG_DBG("%s %d\n", __func__, __LINE__);
    if(ota_spp_read_thread_id)
    {
        osThreadTerminate(ota_spp_read_thread_id);
        ota_spp_read_thread_id = NULL;
    }
}
#endif

static void spp_ota_callback(struct spp_device *locDev, struct spp_callback_parms *Info)
{
	if (BTIF_SPP_EVENT_REMDEV_CONNECTED == Info->event)
	{
		LOG_DBG("::BTIF_SPP_EVENT_REMDEV_CONNECTED %d\n", Info->event);
		 ota.permissionToApply = 0;
        //app_spp_ota_create_read_thread();
         app_ota_connected(APP_OTA_CONNECTED);
	ota_control_update_MTU(OTA_SPP_MAX_PACKET_SIZE);
	ota_control_set_datapath_type(DATA_PATH_SPP);
	ota_control_register_transmitter(app_ota_send_data_via_spp);
	}
	else if (BTIF_SPP_EVENT_REMDEV_DISCONNECTED == Info->event)
	{
		LOG_DBG("::BTIF_SPP_EVENT_REMDEV_DISCONNECTED %d\n", Info->event);
        //app_spp_ota_close_read_thread();
        app_ota_disconnected(APP_OTA_DISCONNECTED);
        ota_control_set_datapath_type(0);
        app_spp_ota_tx_done_func = NULL;
		osDelay(100);
	}
	else if (BTIF_SPP_EVENT_DATA_SENT == Info->event)
    {
        //app_spp_ota_free_tx_buf(Info->tx_buf, Info->tx_data_len);
        struct spp_tx_done *pTxDone = (struct spp_tx_done *)(Info->p.other);
        app_spp_ota_free_tx_buf(pTxDone->tx_buf, pTxDone->tx_data_length);
        if (app_spp_ota_tx_done_func)
        {
            app_spp_ota_tx_done_func();
        }		
    }
    else
    {
        LOG_DBG("::unknown event %d\n", Info->event);
    }
}

static void app_spp_ota_send_data(uint8_t* ptrData, uint16_t length)
{
    if (!app_is_in_ota_mode() && (ota_control_get_datapath_type() != DATA_PATH_SPP))
    {
        return;
    }

    btif_spp_write(ota.ota_spp_dev, (char *)ptrData, &length);
}

void app_ota_send_cmd_via_spp(uint8_t* ptrData, uint16_t length)
{
    uint8_t* ptrBuf = app_spp_ota_fill_data_into_tx_buf(ptrData, length);
    app_spp_ota_send_data(ptrBuf, (uint16_t)length);
}

void app_ota_send_data_via_spp(uint8_t* ptrData, uint32_t length)
{
	 if (!app_is_in_ota_mode() && (ota_control_get_datapath_type() != DATA_PATH_SPP))
    {
        return;
    }
    uint8_t* ptrBuf = app_spp_ota_fill_data_into_tx_buf(ptrData, length);
    app_spp_ota_send_data(ptrBuf, (uint16_t)length);
}

void app_spp_ota_register_tx_done(app_spp_ota_tx_done_t callback)
{
	app_spp_ota_tx_done_func = callback;
}

void app_spp_ota_init(void)
{
    uint8_t *rx_buf;
    osMutexId mid;
    btif_sdp_record_param_t param;

    if( ota.ota_spp_dev == NULL )
    {
        ota.ota_spp_dev = btif_create_spp_device();
    }
    rx_buf = ota.ota_spp_dev->rx_buffer = &ota.otaSppRxBuf[0];
	app_spp_ota_init_tx_buf(ota.otaSppTxBuf);
	btif_spp_init_rx_buf(ota.ota_spp_dev, rx_buf, OTA_SPP_RECV_BUFFER_SIZE);

    mid = osMutexCreate(osMutex(ota_spp_mutex));
    if (!mid) {
        ASSERT(0, "cannot create mutex");
    }

    LOG_DBG("spp_ota_open : ota_sdp_record=0x%x", ota.ota_sdp_record);
    if (ota.ota_sdp_record == NULL) {
        ota.ota_sdp_record = btif_sdp_create_record();
        LOG_DBG("spp_ota_open : new ota_sdp_record=0x%x", ota.ota_sdp_record);
    }

    param.attrs = (sdp_attribute_t*)&OtaSppSdpAttributes[0];
    param.attr_count = ARRAY_SIZE(OtaSppSdpAttributes);
    param.COD = BTIF_COD_MAJOR_PERIPHERAL;

    btif_sdp_record_setup(ota.ota_sdp_record, &param);

	ota.otaSppService.rf_service.serviceId = RFCOMM_CHANNEL_BES_OTA;
	ota.otaSppService.numPorts = 0;
    btif_spp_service_setup(ota.ota_spp_dev, &ota.otaSppService, ota.ota_sdp_record);

	ota.ota_spp_dev->portType = BTIF_SPP_SERVER_PORT;
    ota.ota_spp_dev->app_id = BTIF_APP_SPP_SERVER_BES_OTA_ID;
    ota.ota_spp_dev->spp_handle_data_event_func = ota_spp_handle_data_event_func;
    btif_spp_init_device(ota.ota_spp_dev, OTA_SPP_MAX_PACKET_NUM, mid);

    bt_status_t ret = btif_spp_open(ota.ota_spp_dev, NULL,  spp_ota_callback);
    LOG_DBG("spp_ota_open ret = %d", ret);
}

#endif
