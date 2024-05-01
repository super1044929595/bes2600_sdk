#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "cmsis.h"
#include "cmsis_os.h"
#include "hal_trace.h"
#include "os_api.h"
#include "bt_if.h"
#include "app_bt.h"
#include "app_spp.h"
#include "spp_api.h"
#include "sdp_api.h"
#include "app_bt_func.h"
#include "app_rfcomm_mgr.h"

#ifdef IBRT
#include "app_tws_ibrt.h"
#include "app_tws_if.h"
#include "app_tws_ctrl_thread.h"
#include "app_ibrt_customif_cmd.h"
#endif
#ifdef GFPS_ENABLED

#include "app_fp_rfcomm.h"
#include "app_gfps.h"
#ifdef SASS_ENABLED
#include "app_fp_sass.h"
#include "gfps_crypto.h"
#include "nvrecord_fp_account_key.h"
#include "../../utils/encrypt/aes.h"
#endif

osMutexDef(fp_rfcomm_mutex);
osMutexDef(fp_rfcomm_credit_mutex);


#define FP_RFCOMM_TX_PKT_CNT 6

/* 128 bit UUID in Big Endian df21fe2c-2515-4fdb-8886-f12c4d67927c */
static const uint8_t FP_RFCOMM_UUID_128[16] = {
    0x7C, 0x92, 0x67, 0x4D, 0x2C, 0xF1, 0x86, 0x88, 0xDB, 0x4F, 0x15, 0x25, 0x2C, 0xFE, 0x21, 0xDF};

typedef struct
{
    uint8_t    isConnected;
    int8_t     serviceIndex;
    uint8_t    isRfcommInitialized;
} FpRFcommServiceEnv_t;

typedef union
{
    struct
    {
        uint8_t     isCompanionAppInstalled :   1;
        uint8_t     isSilentModeSupported   :   1;
        uint8_t     reserve                 :   6;
    } env;
    uint8_t content;
} FpCapabilitiesEnv_t;

typedef struct
{
    uint8_t isRightRinging  :   1;
    uint8_t isLeftRinging   :   1;
    uint8_t reserve         :   6;
} FpRingStatus_t;

static FpRFcommServiceEnv_t fp_rfcomm_service = {false, -1, false};

static FpCapabilitiesEnv_t fp_capabilities = {false, false, 0};

static __attribute__((unused)) FpRingStatus_t fp_ring_status = {false, false, 0};

btif_sdp_record_t* fpSppSdpRecord;

extern "C" void     app_gfps_get_battery_levels(uint8_t *pCount, uint8_t *pBatteryLevel);
extern "C" uint8_t *appm_get_current_ble_addr(void);

static int fp_rfcomm_data_received(void *pDev, uint8_t process, uint8_t *pData, uint16_t dataLen);
static void app_fp_msg_send_active_components_rsp(void);
static void app_fp_msg_send_message_ack(uint8_t msgGroup, uint8_t msgCode);
static void app_fp_msg_send_message_nak(uint8_t reason, uint8_t msgGroup, uint8_t msgCode);
static void fp_rfcomm_data_handler(uint8_t* ptr, uint16_t len);
// update this value if the maximum possible tx data size is bigger than current value
#define FP_RFCOMM_TX_BUF_CHUNK_SIZE 64
#define FP_RFCOMM_TX_BUF_CHUNK_CNT FP_RFCOMM_TX_PKT_CNT
#define FP_RFCOMM_TX_BUF_SIZE (FP_RFCOMM_TX_BUF_CHUNK_CNT * FP_RFCOMM_TX_BUF_CHUNK_SIZE)

static uint32_t fp_rfcomm_tx_buf_next_allocated_chunk = 0;
static uint32_t fp_rfcomm_tx_buf_allocated_chunk_cnt  = 0;
static uint8_t  fp_rfcomm_tx_buf[FP_RFCOMM_TX_BUF_CHUNK_CNT][FP_RFCOMM_TX_BUF_CHUNK_SIZE];

static uint8_t *fp_rfcomm_tx_buf_addr(uint32_t chunk)
{
    return fp_rfcomm_tx_buf[chunk];
}

static int32_t fp_rfcomm_alloc_tx_chunk(void)
{
    uint32_t lock = int_lock_global();
    
    if (fp_rfcomm_tx_buf_allocated_chunk_cnt >= FP_RFCOMM_TX_BUF_CHUNK_CNT)
    {
        int_unlock_global(lock);
        return -1;
    }

    uint32_t returnedChunk = fp_rfcomm_tx_buf_next_allocated_chunk; 
    
    fp_rfcomm_tx_buf_allocated_chunk_cnt++;
    fp_rfcomm_tx_buf_next_allocated_chunk++;
    if (FP_RFCOMM_TX_BUF_CHUNK_CNT == fp_rfcomm_tx_buf_next_allocated_chunk)
    {
        fp_rfcomm_tx_buf_next_allocated_chunk = 0;
    }

    int_unlock_global(lock);
    return returnedChunk;
}

static bool fp_rfcomm_free_tx_chunk(void)
{
    uint32_t lock = int_lock_global();
    if (0 == fp_rfcomm_tx_buf_allocated_chunk_cnt)
    {
        int_unlock_global(lock);
        return false;
    }

    fp_rfcomm_tx_buf_allocated_chunk_cnt--;
    int_unlock_global(lock);
    return true;
}

static void fp_rfcomm_reset_tx_buf(void)
{
    uint32_t lock = int_lock_global();
    fp_rfcomm_tx_buf_allocated_chunk_cnt = 0;
    fp_rfcomm_tx_buf_next_allocated_chunk = 0;
    int_unlock_global(lock);
}


#include "app_status_ind.h"
#include "apps.h"
osTimerId ring_timeout_timer_id = NULL;
static void gfps_find_devices_ring_timeout_handler(void const *param);
osTimerDef (GFPS_FIND_DEVICES_RING_TIMEOUT, gfps_find_devices_ring_timeout_handler);
#define GFPS_FIND_MY_BUDS_CMD_STOP_DUAL 0x0
#define GFPS_FIND_MY_BUDS_CMD_START_MASTER_ONLY 0x1
#define GFPS_FIND_MY_BUDS_CMD_START_SLAVE_ONLY 0x2
#define GFPS_FIND_MY_BUDS_CMD_START_DUAL 0x3

static bool left_is_ring=0;
static bool right_is_ring=0;
static void gfps_set_find_my_buds(uint8_t cmd)
{
    TRACE(2,"%s, cmd = %d", __func__, cmd);
    if(GFPS_FIND_MY_BUDS_CMD_STOP_DUAL == cmd)
    {
        app_gfps_ring_mode_set(GFPS_RING_MODE_BOTH_OFF);
        app_gfps_find_sm(false);
        left_is_ring=0;
        right_is_ring=0;
    }
    else if(GFPS_FIND_MY_BUDS_CMD_START_MASTER_ONLY == cmd)    //right ring, stop left
    {
        app_gfps_ring_mode_set(GPFS_RING_MODE_RIGHT_ON);
        if(app_tws_is_left_side())
        {
            app_gfps_find_sm(false);
        }
        else
        {
            app_gfps_find_sm(true);
        }
        right_is_ring=1;
        left_is_ring=0;
    }
    else if(GFPS_FIND_MY_BUDS_CMD_START_SLAVE_ONLY == cmd)    //left ring, stop right
    {
        app_gfps_ring_mode_set(GFPS_RING_MODE_LEFT_ON);
        if(app_tws_is_left_side())
        {
            app_gfps_find_sm(true);
        }
        else
        {
            app_gfps_find_sm(false);
        }
        left_is_ring=1;
        right_is_ring=0;
    }
    else if(GFPS_FIND_MY_BUDS_CMD_START_DUAL == cmd)    //both ring
    {
        app_gfps_ring_mode_set(GFPS_RING_MODE_BOTH_ON);
        app_gfps_find_sm(true);
        left_is_ring=1;
        right_is_ring=1;
    }
}
static void gfps_find_devices_ring_timeout_handler(void const *param)
{
    TRACE(0,"gfps_find_devices_ring_timeout_handler");
    app_bt_start_custom_function_in_bt_thread(GFPS_FIND_MY_BUDS_CMD_STOP_DUAL, 0, \
                                (uint32_t)gfps_set_find_my_buds);
}

void fp_rfcomm_ring_timer_set(uint8_t period)
{
    TRACE(2,"%s, period = %d", __func__, period);
    if (ring_timeout_timer_id == NULL)
    {
        ring_timeout_timer_id = osTimerCreate(osTimer(GFPS_FIND_DEVICES_RING_TIMEOUT), osTimerOnce, NULL);
    }
    
    osTimerStop(ring_timeout_timer_id);
    if(period)
    {
        osTimerStart(ring_timeout_timer_id, period*1000);
    }
}

static void fp_rfcomm_ring_request_handling(uint8_t* requestdata, uint16_t datalen)
{
    TRACE(1,"%s,[RFCOMM][FMD] request",__func__);
    DUMP8("%02x ", requestdata, datalen);
    app_fp_msg_send_message_ack(FP_MSG_GROUP_DEVICE_ACTION, FP_MSG_DEVICE_ACTION_RING);
     if (datalen > 1)
    {
        fp_rfcomm_ring_timer_set(requestdata[1]);
    }

    gfps_set_find_my_buds(requestdata[0]);
}

#define FP_ACCUMULATED_DATA_BUF_SIZE    128
static uint8_t fp_accumulated_data_buf[FP_ACCUMULATED_DATA_BUF_SIZE];
static uint16_t fp_accumulated_data_size = 0;

static void fp_rfcomm_reset_data_accumulator(void)
{
    fp_accumulated_data_size = 0;
    memset(fp_accumulated_data_buf, 0, sizeof(fp_accumulated_data_buf));
}

static void fp_rfcomm_data_accumulator(uint8_t* ptr, uint16_t len)
{
    ASSERT((fp_accumulated_data_size + len) < sizeof(fp_accumulated_data_buf),
        "fp accumulcate buffer is overflow!");

    memcpy(&fp_accumulated_data_buf[fp_accumulated_data_size], ptr, len);
    fp_accumulated_data_size += len;

    uint16_t msgTotalLen;
    FP_MESSAGE_STREAM_T* msgStream;
    
    while (fp_accumulated_data_size >= FP_MESSAGE_RESERVED_LEN)
    {
        msgStream = (FP_MESSAGE_STREAM_T *)fp_accumulated_data_buf;
        msgTotalLen = ((msgStream->dataLenHighByte << 8)|msgStream->dataLenLowByte) + 
            FP_MESSAGE_RESERVED_LEN;
        ASSERT(msgTotalLen < sizeof(fp_accumulated_data_buf),
            "Wrong fp msg len %d received!", msgTotalLen);
        if (fp_accumulated_data_size >= msgTotalLen)
        {
            fp_rfcomm_data_handler(fp_accumulated_data_buf, msgTotalLen);
            fp_accumulated_data_size -= msgTotalLen;
            memmove(fp_accumulated_data_buf, &fp_accumulated_data_buf[msgTotalLen],
                fp_accumulated_data_size);
        }
        else
        {
            break;
        }
    }
}

static int fp_rfcomm_data_received(void *pDev, uint8_t process, uint8_t *pData, uint16_t dataLen)
{
    TRACE(1,"%s",__func__);
    //DUMP8("0x%02x ",pData, dataLen);
    fp_rfcomm_data_accumulator(pData, dataLen);
    return 0;
}

static void app_fp_sync_ring_status_to_mobile(uint8_t ring_status)
{
    TRACE(2,"%s %d",__func__,ring_status);
    FP_MESSAGE_STREAM_T  ring_cmd;
    ring_cmd.messageGroup = FP_MSG_GROUP_DEVICE_ACTION;
    ring_cmd.messageCode = FP_MSG_DEVICE_ACTION_RING;
    ring_cmd.dataLenHighByte=0x00;
    ring_cmd.dataLenLowByte=0x01;
    ring_cmd.data[0]=ring_status;
    app_fp_rfcomm_send((uint8_t *)&ring_cmd, FP_MESSAGE_RESERVED_LEN+1);
}

void app_fp_master_stop_ring_buds()
{
    ibrt_ctrl_t *p_ibrt_ctrl = app_tws_ibrt_get_bt_ctrl_ctx();
    uint8_t send_cmd=0;
    if(p_ibrt_ctrl->current_role != IBRT_MASTER)
    {
        return;
    }

    if((!app_tws_is_left_side())&&(right_is_ring))//right need stop ring
    {
        if(left_is_ring)
        {
            send_cmd=GFPS_FIND_MY_BUDS_CMD_START_SLAVE_ONLY;
        }
        else
        {
            send_cmd=GFPS_FIND_MY_BUDS_CMD_STOP_DUAL;
        }
        app_gfps_find_sm(false);
        app_fp_sync_ring_status_to_mobile(send_cmd);
        right_is_ring=0;
    }
    else if(app_tws_is_left_side()&&(left_is_ring))//left need stop ring
    {
        if(right_is_ring)
        {
            send_cmd=GFPS_FIND_MY_BUDS_CMD_START_MASTER_ONLY;
        }
        else
        {
            send_cmd=GFPS_FIND_MY_BUDS_CMD_STOP_DUAL;
        }
        app_gfps_find_sm(false);
        app_fp_sync_ring_status_to_mobile(send_cmd);
        left_is_ring=0;
    }
}

void app_fp_slave_stop_ring_buds()
{
    ibrt_ctrl_t *p_ibrt_ctrl = app_tws_ibrt_get_bt_ctrl_ctx();
    uint8_t ring_status;
    if(p_ibrt_ctrl->current_role != IBRT_SLAVE)
    {
        return;
    }
    TRACE(3,"%s %d %d",__func__,left_is_ring,right_is_ring);
    if((!app_tws_is_left_side())&&(right_is_ring))
    {
        ring_status=1;
        tws_ctrl_send_cmd(APP_TWS_CMD_GFPS_RING_STOP_SYNC, &ring_status, 1);
        app_gfps_find_sm(false);
    }
    else if(app_tws_is_left_side()&&(left_is_ring))
    {
        ring_status=1;
        tws_ctrl_send_cmd(APP_TWS_CMD_GFPS_RING_STOP_SYNC, &ring_status, 1);
        app_gfps_find_sm(false);
    }
}

void app_fp_master_handle_slave_stop_ring_buds()
{
    TRACE(3,"%s left=%d right=%d",__func__,left_is_ring,right_is_ring);
    ibrt_ctrl_t *p_ibrt_ctrl = app_tws_ibrt_get_bt_ctrl_ctx();
    uint8_t send_cmd=0;
    if(p_ibrt_ctrl->current_role != IBRT_MASTER)
    {
        return;
    }

    if((!app_tws_is_left_side())&&(left_is_ring))//;left need stop ring
    {
        if(right_is_ring)
        {
            send_cmd=GFPS_FIND_MY_BUDS_CMD_START_MASTER_ONLY;
        }
        else
        {
            send_cmd=GFPS_FIND_MY_BUDS_CMD_STOP_DUAL;
        }
        left_is_ring=0;
        app_fp_sync_ring_status_to_mobile(send_cmd);
    }
    else if(app_tws_is_left_side()&&(right_is_ring))//right need stop ring
    {
        if(left_is_ring)
        {
            send_cmd=GFPS_FIND_MY_BUDS_CMD_START_SLAVE_ONLY;
        }
        else
        {
            send_cmd=GFPS_FIND_MY_BUDS_CMD_STOP_DUAL;
        }
        right_is_ring=0;
        app_fp_sync_ring_status_to_mobile(send_cmd);
    }
}

static void app_fp_disconnect_rfcomm_handler(void)
{
    if (fp_rfcomm_service.isConnected)
    {
        app_rfcomm_close(fp_rfcomm_service.serviceIndex);
    }
}

void app_fp_disconnect_rfcomm(void)
{
    app_bt_start_custom_function_in_bt_thread(0,
                                           0,
                                           ( uint32_t )app_fp_disconnect_rfcomm_handler);
}

static void app_fp_rfcomm_send_handler(uint8_t *ptrData, uint32_t length)
{
    int8_t ret = app_rfcomm_write(fp_rfcomm_service.serviceIndex, ptrData, length);
    if (0 != ret)
    {
        fp_rfcomm_free_tx_chunk();
    }
}

void app_fp_rfcomm_send(uint8_t *ptrData, uint32_t length)
{
    if (!fp_rfcomm_service.isConnected)
    {
        return;
    }

    int32_t chunk = fp_rfcomm_alloc_tx_chunk();
    if (-1 == chunk)
    {
        TRACE(0,"Fast pair rfcomm tx buffer used out!");
        return;
    }

    ASSERT(length < FP_RFCOMM_TX_BUF_CHUNK_SIZE,
           "FP_RFCOMM_TX_BUF_CHUNK_SIZE is %d which is smaller than %d, need to increase!",
           FP_RFCOMM_TX_BUF_CHUNK_SIZE,
           length);

    uint8_t *txBufAddr = fp_rfcomm_tx_buf_addr(chunk);
    memcpy(txBufAddr, ptrData, length);
    app_bt_start_custom_function_in_bt_thread(( uint32_t )txBufAddr,
                                           ( uint32_t )length,
                                           ( uint32_t )app_fp_rfcomm_send_handler);
}

bool app_spp_gfps_is_already_connected(int8_t server_channel)
{
	TRACE(3,"%s server_channel %d,Connected %d",__func__,server_channel,fp_rfcomm_service.isConnected);
    if((server_channel == RFCOMM_CHANNEL_FP) && (fp_rfcomm_service.isConnected == true)){
        return true;
    }
    return false;
}

bool app_is_fp_rfcomm_connected(void)
{
    return fp_rfcomm_service.isConnected;
}

// use cases for fp message stream
void app_fp_msg_enable_bt_silence_mode(bool isEnable)
{
    if (fp_capabilities.env.isSilentModeSupported)
    {
        FP_MESSAGE_STREAM_T req = {FP_MSG_GROUP_BLUETOOTH_EVENT, 0, 0, 0};
        if (isEnable)
        {
            req.messageCode = FP_MSG_BT_EVENT_ENABLE_SILENCE_MODE;
        }
        else
        {
            req.messageCode = FP_MSG_BT_EVENT_DISABLE_SILENCE_MODE;
        }

        app_fp_rfcomm_send((uint8_t *)&req, FP_MESSAGE_RESERVED_LEN);
    }
    else
    {
        TRACE(0,"fp silence mode is not supported.");
    }
}

void app_fp_msg_send_model_id(void)
{
    TRACE(1,"%s",__func__);
#ifndef IS_USE_CUSTOM_FP_INFO
    uint32_t model_id = 0x2B677D;
#else
    uint32_t model_id = app_bt_get_model_id();
#endif
    uint8_t  modelID[3];
    modelID[0] = (model_id >> 16) & 0xFF;
    modelID[1] = (model_id >> 8) & 0xFF;
    modelID[2] = ( model_id )&0xFF;

    uint16_t rawDataLen = sizeof(modelID);

    FP_MESSAGE_STREAM_T req =
        {FP_MSG_GROUP_DEVICE_INFO,
         FP_MSG_DEVICE_INFO_MODEL_ID,
         (uint8_t)(rawDataLen >> 8),
         (uint8_t)(rawDataLen & 0xFF)};
    memcpy(req.data, modelID, sizeof(modelID));

    app_fp_rfcomm_send(( uint8_t * )&req, FP_MESSAGE_RESERVED_LEN + rawDataLen);
}

void app_fp_msg_send_updated_ble_addr(void)
{
    FP_MESSAGE_STREAM_T req =
        {FP_MSG_GROUP_DEVICE_INFO,
         FP_MSG_DEVICE_INFO_BLE_ADD_UPDATED,
         0,
         6};

    uint8_t *ptr = appm_get_current_ble_addr();

    for (uint8_t index = 0; index < 6; index++)
    {
        req.data[index] = ptr[5 - index];
    }

    app_fp_rfcomm_send(( uint8_t * )&req, FP_MESSAGE_RESERVED_LEN + 6);
}

void app_fp_msg_send_battery_levels(void)
{
    TRACE(1,"%s",__func__);
    FP_MESSAGE_STREAM_T req =
        {FP_MSG_GROUP_DEVICE_INFO,
         FP_MSG_DEVICE_INFO_BATTERY_UPDATED,
         0,
         3};

    uint8_t batteryLevelCount = 0;
    app_gfps_get_battery_levels(&batteryLevelCount, req.data);

    app_fp_rfcomm_send(( uint8_t * )&req, FP_MESSAGE_RESERVED_LEN + 3);
}

static __attribute__((unused)) void app_fp_msg_send_active_components_rsp(void)
{
    FP_MESSAGE_STREAM_T req = {FP_MSG_GROUP_DEVICE_INFO, FP_MSG_DEVICE_INFO_ACTIVE_COMPONENTS_RSP, 0, 1};

#if defined(IBRT)
    if (app_tws_ibrt_tws_link_connected())
    {
        req.data[0] = FP_MSG_BOTH_BUDS_ACTIVE;
    }
    else
    {
        if (app_tws_is_left_side())
        {   
            req.data[0] = FP_MSG_LEFT_BUD_ACTIVE;
        }
        else
        {
            req.data[0] = FP_MSG_RIGHT_BUD_ACTIVE;
        }
    }
#else
    req.data[0] = FP_MSG_BOTH_BUDS_ACTIVE;
#endif

    app_fp_rfcomm_send((uint8_t *)&req, FP_MESSAGE_RESERVED_LEN+1);
}

static __attribute__((unused)) void app_fp_msg_send_message_ack(uint8_t msgGroup, uint8_t msgCode)
{
    FP_MESSAGE_STREAM_T req = {FP_MSG_GROUP_ACKNOWLEDGEMENT, FP_MSG_ACK, 0, 2};

    req.data[0] = msgGroup;
    req.data[1] = msgCode;

    app_fp_rfcomm_send((uint8_t *)&req, FP_MESSAGE_RESERVED_LEN+2);
}

static __attribute__((unused)) void app_fp_msg_send_message_nak(uint8_t reason, uint8_t msgGroup, uint8_t msgCode)
{
    FP_MESSAGE_STREAM_T req = {FP_MSG_GROUP_ACKNOWLEDGEMENT, FP_MSG_NAK, 0, 3};

    req.data[0] = reason;
    req.data[1] = msgGroup;
    req.data[2] = msgCode;

    app_fp_rfcomm_send((uint8_t *)&req, FP_MESSAGE_RESERVED_LEN+3);
}

#ifdef SASS_ENABLED
static __attribute__((unused)) void app_fp_msg_send_session_nonce(uint8_t device_id)
{
    FP_MESSAGE_STREAM_T req = {FP_MSG_GROUP_DEVICE_INFO, FP_MSG_DEVICE_INFO_SESSION_NONCE, 0, 8};
    app_fp_sass_get_session_nonce(device_id, req.data);
    app_fp_rfcomm_send((uint8_t *)&req, FP_MESSAGE_RESERVED_LEN+8);
}

void app_fp_msg_sass_get_capability(uint8_t device_id)
{
    TRACE(1,"%s",__func__);
    FP_MESSAGE_STREAM_T req =
        {FP_MSG_GROUP_SASS,
         FP_MSG_SASS_GET_CAPBILITY,
         0,
         0};
    app_fp_rfcomm_send(( uint8_t * )&req, FP_MESSAGE_RESERVED_LEN);
}

void app_fp_msg_sass_ntf_capability(uint8_t device_id)
{
    TRACE(1,"%s",__func__);
    uint32_t dataLen = 0;
    FP_MESSAGE_STREAM_T req =
        {FP_MSG_GROUP_SASS,
         FP_MSG_SASS_NTF_CAPBILITY};
    app_fp_sass_get_cap(req.data, &dataLen);
    req.dataLenHighByte = (uint8_t)(dataLen & 0xFF00);
    req.dataLenLowByte = (uint8_t)(dataLen & 0xFF);
    app_fp_rfcomm_send(( uint8_t * )&req, FP_MESSAGE_RESERVED_LEN + dataLen);
}

void app_fp_msg_sass_set_capability(uint8_t device_id,uint8_t *data)
{
    TRACE(1,"%s",__func__);
    uint16_t sassVer =( data[0] << 8) | data[1];
    app_fp_sass_set_sass_mode(device_id, sassVer);
}

void app_fp_msg_sass_set_switch_pref_hdl(uint8_t device_id, uint8_t *data)
{
    uint8_t pref = data[0];
    TRACE(2,"%s pref:0x%0x",__func__, pref);
    app_fp_msg_send_message_ack( FP_MSG_GROUP_SASS, FP_MSG_SASS_SET_SWITCH_PREFERENCE);
    app_fp_sass_set_switch_pref(pref);
}

void app_fp_msg_sass_ntf_switch_pref(uint8_t device_id)
{
    TRACE(1,"%s",__func__);
    uint16_t dataLen = 2;
    FP_MESSAGE_STREAM_T req =
        {FP_MSG_GROUP_SASS,
         FP_MSG_SASS_NTF_SWITCH_PREFERENCE,
         0,
         2};
    req.data[0] = app_fp_sass_get_switch_pref();
    req.data[1] = 0;
    app_fp_rfcomm_send(( uint8_t * )&req, FP_MESSAGE_RESERVED_LEN + dataLen);
}

void app_fp_msg_sass_ntf_switch_evt(uint8_t device_id, uint8_t reason)
{
    uint32_t nameLen = 0;
    uint8_t *namePtr = NULL;
    uint8_t activeId = app_fp_sass_get_active_dev();

    namePtr = nv_record_fp_get_name_ptr(&nameLen);
    FP_MESSAGE_STREAM_T req =
        {FP_MSG_GROUP_SASS,
         FP_MSG_SASS_NTF_SWITCH_EVT,
         0,
         (uint8_t)(2+nameLen)};

    req.data[0] = reason;
    req.data[1] = (device_id == activeId) ? SASS_DEV_THIS_DEVICE : SASS_DEV_ANOTHER;
    if(namePtr)
    {
        DUMP8("%02x ", namePtr, nameLen);
        memcpy(req.data + 2, namePtr, nameLen);
    }
    TRACE(3,"sass_ntf_switch id:%d, activeId:%d, reason:%d", device_id, activeId, reason);

    app_fp_rfcomm_send(( uint8_t * )&req, FP_MESSAGE_RESERVED_LEN + 2 + nameLen);
}

void app_fp_msg_sass_switch_src_hdl(uint8_t device_id, uint8_t *data)
{
    uint8_t evt = data[0];
    uint8_t reason;
    reason = app_fp_sass_switch_src_evt_hdl(device_id, evt);
    if(reason == SASS_STATUS_OK)
    {
        app_fp_msg_send_message_ack( FP_MSG_GROUP_SASS, FP_MSG_SASS_SWITCH_ACTIVE_SOURCE);
    }else if(reason == SASS_STATUS_REDUNTANT){
        app_fp_msg_send_message_nak( FP_MSG_NAK_REASON_REDUNDANT_ACTION, FP_MSG_GROUP_SASS,\
                                    FP_MSG_SASS_SWITCH_ACTIVE_SOURCE);
    }else
    {
        app_fp_msg_send_message_nak( FP_MSG_NAK_REASON_NOT_ALLOWED, FP_MSG_GROUP_SASS,\
                                    FP_MSG_SASS_SWITCH_ACTIVE_SOURCE);
    }
    TRACE(3,"%s evt:0x%0x, reason:%d",__func__, evt, reason);
}

void app_fp_msg_sass_switch_back_hdl(uint8_t device_id, uint8_t *data)
{
    uint8_t evt = data[0];
    uint8_t reason = app_fp_sass_switch_back_hdl(device_id, evt);

    if(reason == SASS_STATUS_OK)
    {
        app_fp_msg_send_message_ack( FP_MSG_GROUP_SASS, FP_MSG_SASS_SWITCH_BACK);
    }else
    {
        app_fp_msg_send_message_nak( FP_MSG_NAK_REASON_NOT_ALLOWED, FP_MSG_GROUP_SASS,\
                                    FP_MSG_SASS_SWITCH_BACK);
    }

    app_fp_msg_sass_ntf_switch_evt(device_id, SASS_REASON_UNSPECIFIED);
    TRACE(3,"%s evt:0x%0x, reason:%d",__func__, evt, reason);
}

void app_fp_msg_sass_ntf_conn_status(uint8_t device_id)
{
    uint8_t adv[16];
    uint8_t advlen, len, activeId;
    uint8_t account[FP_ACCOUNT_KEY_SIZE] = {0};
    uint8_t outBuf[FP_ACCOUNT_KEY_SIZE] = {0};
    uint8_t iv[FP_ACCOUNT_KEY_SIZE] = {0};
    uint8_t memNonce[SESSION_NOUNCE_NUM] = {0};

    if(!app_fp_sass_get_inuse_acckey_by_id(device_id, account)) {
        return;
    }

    if(!app_fp_sass_get_session_nonce(device_id, iv)) {
        return;
    }

    for(int i = 0; i < SESSION_NOUNCE_NUM; i++)
    {
         memNonce[i] = (uint8_t)rand();
         iv[SESSION_NOUNCE_NUM + i] = memNonce[i];
    }
    TRACE(0, "message nounce is:");
    DUMP8("0x%2x ", memNonce, SESSION_NOUNCE_NUM);
    app_fp_sass_get_adv_data(adv, &advlen);
    AES128_CTR_encrypt_buffer(adv+1, advlen-1, account, iv, outBuf);
    len = (advlen-1) + 1 + SESSION_NOUNCE_NUM;
    FP_MESSAGE_STREAM_T req =
        {FP_MSG_GROUP_SASS,
         FP_MSG_SASS_NTF_CONN_STATUS,
         0,
         len};

    activeId = app_fp_sass_get_active_dev();
    req.data[0] = (activeId == device_id);
    memcpy(req.data + 1, outBuf, advlen-1);
    memcpy(req.data + advlen, memNonce, 8);
    app_fp_rfcomm_send((uint8_t * )&req, FP_MESSAGE_RESERVED_LEN + len);

    TRACE(1,"%s dev:%d",__func__, device_id);
}

void app_fp_msg_sass_get_conn_hdl(uint8_t device_id)
{
    app_fp_msg_sass_ntf_conn_status(device_id);
    TRACE(1,"%s",__func__);
}

void app_fp_msg_sass_set_init_conn(uint8_t device_id, uint8_t *data)
{
    bool isSass = data[0];
    app_fp_sass_set_init_conn(device_id, isSass);
    TRACE(1,"connection is triggered by sass? %d", isSass);
}

void app_fp_msg_sass_ind_inuse_acckey(uint8_t device_id, uint8_t *data)
{
    char str[8] = "in-use";
    uint8_t auth[8] = {0};
    uint8_t nonce[16] = {0};
    uint8_t keyCount = 0;
    uint8_t accKey[16] = {0};
    uint8_t output[8] = {0};
    if(memcmp(data, str, 6))
    {
        TRACE(1, "%s data error!", __func__);
        return;
    }

    app_fp_sass_get_session_nonce(device_id, nonce);
    memcpy(nonce + 8, data + 6, 8);
    memcpy(auth, data + 14, 8);

    keyCount = nv_record_fp_account_key_count();
    for(int i = 0; i < keyCount; i++)
    {
        nv_record_fp_account_key_get_by_index(i, accKey);
        gfps_encrypt_messasge(accKey, nonce, data, 6, output);
        if(!memcmp(output, auth, 8))
        {
            TRACE(2, "%s find account key index:%d", __func__, i);
            uint8_t btAddr[6];
            app_fp_sass_set_inuse_acckey_by_dev(device_id, accKey);
            if(app_bt_get_device_bdaddr(device_id, btAddr))
            {
                nv_record_fp_update_addr(i, btAddr);
            }
            break;
        }
    }
}

void app_fp_msg_sass_set_custom_data(uint8_t device_id, uint8_t *data)
{
    uint8_t param = data[0];
    SassEvtParam evtParam;

    evtParam.event = SASS_EVT_UPDATE_CUSTOM_DATA;
    evtParam.devId = device_id;
    evtParam.state.cusData = param;
    app_fp_sass_update_state(&evtParam);
}

void app_fp_msg_sass_set_drop_dev(uint8_t device_id, uint8_t *data)
{
    //drop this device
    if(data[0] == 1)
    {
        app_fp_sass_set_drop_dev(device_id);
    }
}

void app_fp_sass_event_handler(uint8_t device_id, uint8_t evt, void *param)
{
    TRACE(3,"%s id:%d evt:0x%0x", __func__, device_id, evt);
#if 0
    ibrt_ctrl_t *p_ibrt_ctrl = app_tws_ibrt_get_bt_ctrl_ctx();
    if(TWS_UI_SLAVE == app_ibrt_if_get_ui_role()&& p_ibrt_ctrl->init_done)
    {
        return;
    }
#endif
    if(evt == FP_MSG_SASS_NTF_INIT_CONN || evt == FP_MSG_SASS_IND_INUSE_ACCOUNT_KEY || evt == FP_MSG_SASS_SEND_CUSTOM_DATA \
        || evt == FP_MSG_SASS_SET_DROP_TGT || evt == FP_MSG_SASS_NTF_CAPBILITY || evt ==FP_MSG_SASS_SET_MULTIPOINT_STATE)
    {
        app_fp_msg_send_message_ack( FP_MSG_GROUP_SASS, evt);
    }

    switch(evt)
    {
        case FP_MSG_SASS_GET_CAPBILITY:
        {  
            app_fp_msg_sass_ntf_capability(device_id);
            break;
        }

        case FP_MSG_SASS_NTF_CAPBILITY:
        {
            app_fp_msg_sass_set_capability(device_id, (uint8_t *)param);
            break;
        }
        case FP_MSG_SASS_SET_SWITCH_PREFERENCE:
        {
            app_fp_msg_sass_set_switch_pref_hdl(device_id, (uint8_t *)param);
            break;
        }
        case FP_MSG_SASS_GET_SWITCH_PREFERENCE:
        {
            app_fp_msg_sass_ntf_switch_pref(device_id);
            break;
        }
        case FP_MSG_SASS_SWITCH_ACTIVE_SOURCE:
        {
            app_fp_msg_sass_switch_src_hdl(device_id, (uint8_t *)param);
            break;
        }
        case FP_MSG_SASS_SWITCH_BACK:
        {
            app_fp_msg_sass_switch_back_hdl(device_id, (uint8_t *)param);
            break;
        }
        case FP_MSG_SASS_GET_CONN_STATUS:
        {
            app_fp_msg_sass_get_conn_hdl(device_id);
            break;
        }
        case FP_MSG_SASS_NTF_INIT_CONN:
        {
            app_fp_msg_sass_set_init_conn(device_id, (uint8_t *)param);
            break;
        }
        case FP_MSG_SASS_IND_INUSE_ACCOUNT_KEY:
        {
            app_fp_msg_sass_ind_inuse_acckey(device_id, (uint8_t *)param);
            break;
        }
        case FP_MSG_SASS_SEND_CUSTOM_DATA:
        {
            app_fp_msg_sass_set_custom_data(device_id, (uint8_t *)param);
            break;
        }
        case FP_MSG_SASS_SET_DROP_TGT:
        {
            break;
        }
        default:
        break;
    }
}
#endif

static void fp_rfcomm_data_handler(uint8_t* ptr, uint16_t len)
{
    FP_MESSAGE_STREAM_T* pMsg = (FP_MESSAGE_STREAM_T *)ptr;
    uint16_t datalen = 0;
    TRACE(2,"fp rfcomm receives msg group %d code %d", 
        pMsg->messageGroup, pMsg->messageCode);

    DUMP8("%02x ",ptr,len);
    switch (pMsg->messageGroup)
    {
        case FP_MSG_GROUP_DEVICE_INFO:
        {
            switch (pMsg->messageCode)
            {
                case FP_MSG_DEVICE_INFO_ACTIVE_COMPONENTS_REQ:
                    app_fp_msg_send_active_components_rsp();
                    break;
                case FP_MSG_DEVICE_INFO_TELL_CAPABILITIES:
                    fp_capabilities.content = pMsg->data[0];
                    TRACE(3,"cap 0x%x isCompanionAppInstalled %d isSilentModeSupported %d",
                        fp_capabilities.content, fp_capabilities.env.isCompanionAppInstalled,
                        fp_capabilities.env.isSilentModeSupported);
                    break;
                default:
                    break;
            }
        }
        /*FALL THROUGH*/
        
        case FP_MSG_GROUP_DEVICE_ACTION:
        {
            switch (pMsg->messageCode)
            {
                case FP_MSG_DEVICE_ACTION_RING:
                    datalen = (pMsg->dataLenHighByte<<8)+pMsg->dataLenLowByte;
                    fp_rfcomm_ring_request_handling(pMsg->data, datalen);
                    break;
                default:
                    break;
            }
            break;
        }
#ifdef SASS_ENABLED
		case FP_MSG_GROUP_SASS:
		{
			TRACE(2,"fp rfcomm receives msg len is %d %d",pMsg->dataLenHighByte, pMsg->dataLenLowByte);
			app_fp_sass_event_handler(0, pMsg->messageCode, pMsg->data);
			break;
		}
#endif

        default:
            break;
    }
}

static void fp_rfcomm_connected_handler(void)
{
    if (!fp_rfcomm_service.isConnected)
	{  
        fp_rfcomm_service.isConnected = true;
        fp_rfcomm_reset_data_accumulator();
#ifdef IBRT
        ibrt_ctrl_t *p_ibrt_ctrl = app_tws_ibrt_get_bt_ctrl_ctx();
        if (IBRT_SLAVE != p_ibrt_ctrl->current_role && p_ibrt_ctrl->init_done)
#endif
        {
            app_fp_msg_send_model_id();
            app_fp_msg_send_updated_ble_addr();
            app_fp_msg_send_battery_levels();
#ifdef SASS_ENABLED
            app_fp_sass_gen_session_nonce(0);
            app_fp_msg_send_session_nonce(0);
            app_fp_msg_sass_get_capability(0);
#endif
        }
#if defined(IBRT) && defined(SASS_ENABLED)
        else
        {
            SassBtInfo *info = app_fp_sass_get_connected_dev(0);
            if(!info)
            {   
                uint8_t btAddr[BTIF_BD_ADDR_SIZE];
                app_bt_get_device_bdaddr(0, btAddr);
                app_fp_sass_connect_handler(0, (bd_addr_t *)btAddr);
            }
            app_fp_sass_sync_info();
        }
#endif
    }
}

static bool fp_rfcomm_callback(RFCOMM_EVENT_E event, 
    uint8_t instanceIndex, uint16_t connHandle,
    uint8_t* pBtAddr, uint8_t* pSentDataBuf, uint16_t sentDataLen)
{
    TRACE(2,"%s,event is %d",__func__,event);
    switch (event)
    {
        case RFCOMM_INCOMING_CONN_REQ:
        {
            TRACE(0,"Connected Indication RFComm device info:");
            TRACE(2,"hci handle is 0x%x service index %d", 
                connHandle, instanceIndex);
            if (pBtAddr)
            {
                TRACE(0,"Bd addr is:");
                DUMP8("%02x ", pBtAddr, 6);     
            }

            fp_rfcomm_connected_handler();
            break;
        }
        case RFCOMM_CONNECTED:
        {
            if (pBtAddr)
            {
              TRACE(0,"Bd addr is:");
              DUMP8("%02x ", pBtAddr, 6);
            }
        
            fp_rfcomm_connected_handler();
            break;
        }
        case RFCOMM_DISCONNECTED:
        {
            TRACE(0,"Disconnected Rfcomm device info:");
            TRACE(0,"Bd addr is:");
            DUMP8("%02x ", pBtAddr, 6);
            TRACE(1,"hci handle is 0x%x", connHandle);

            TRACE(1,"::RFCOMM_DISCONNECTED %d", event);
            
            fp_rfcomm_service.isConnected = false;
            fp_rfcomm_reset_tx_buf();
            break;
        }
        case RFCOMM_TX_DONE:
        {
            TRACE(1,"Rfcomm dataLen %d sent out", sentDataLen);
            fp_rfcomm_free_tx_chunk();
            break;
        }
        default:
        {
            TRACE(1,"Unkown rfcomm event %d", event);
            break;
        }
    }

    return true;
}

bt_status_t app_fp_rfcomm_init(void)
{
    TRACE(1,"%s",__func__);
    bt_status_t stat = BT_STS_SUCCESS;

    if (!fp_rfcomm_service.isRfcommInitialized)
    {
        osMutexId mid;
        mid = osMutexCreate(osMutex(fp_rfcomm_mutex));
        if (!mid) {
            ASSERT(0, "cannot create mutex");
        }
        btif_spp_server_register_already_connected_callback(app_spp_gfps_is_already_connected);

        fp_rfcomm_service.isRfcommInitialized = true;
        
        RFCOMM_CONFIG_T tConfig;
        tConfig.callback = fp_rfcomm_callback;
        tConfig.tx_pkt_cnt = FP_RFCOMM_TX_PKT_CNT;
        tConfig.rfcomm_128bit_uuid = FP_RFCOMM_UUID_128;
        tConfig.rfcomm_ch = RFCOMM_CHANNEL_FP;
        tConfig.app_id = BTIF_APP_SPP_SERVER_FP_RFCOMM_ID;
        tConfig.spp_handle_data_event_func = fp_rfcomm_data_received;
        tConfig.mutexId = mid;
        tConfig.creditMutexId = osMutexCreate(osMutex(fp_rfcomm_credit_mutex));
        int8_t index = app_rfcomm_open(&tConfig);

        if (-1 == index)
        {            
            TRACE(0,"fast pair rfcomm open failed");
            return BT_STS_FAILED;           
        }

        fp_rfcomm_service.isConnected = false;
        fp_rfcomm_service.serviceIndex = index;
    }
    else
    {
        TRACE(0,"already initialized.");
    }

    return stat;
}


#endif
