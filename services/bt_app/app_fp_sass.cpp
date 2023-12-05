#ifdef  SASS_ENABLED
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "cmsis.h"
#include "cmsis_os.h"
#include "hal_trace.h"
#include "os_api.h"
#include "bt_if.h"
#include "app_bt.h"
#include "app_bt_func.h"
#ifdef IBRT
#include "app_tws_ibrt.h"
#include "app_ibrt_if.h"
#include "app_tws_ibrt_cmd_handler.h"
#include "app_tws_ctrl_thread.h"
#endif
#include "resources.h"
#include "app_media_player.h"
#include "app_fp_sass.h"
#include "app_ble_mode_switch.h"
#include "app_hfp.h"
#include "app_gfps.h"
#include "app_bt_media_manager.h"
#include "app_fp_rfcomm.h"
#include "../../utils/encrypt/aes.h"
#include "nvrecord_fp_account_key.h"

SassConnInfo sassInfo;
SassAdvInfo sassAdv;

void app_fp_sass_init()
{
    memset((void *)&sassInfo, 0, sizeof(SassConnInfo));
    sassInfo.reconnInfo.evt = 0xFF;
    sassInfo.headState = SASS_HEAD_STATE_ON;
    sassInfo.connAvail = SASS_CONN_AVAILABLE;
#ifdef IBRT_V2_MULTIPOINT
    sassInfo.isMulti = true;
#endif
    for(int i= 0; i < BT_DEVICE_NUM; i++)
    {
        sassInfo.connInfo[i].connId = 0xFF;
        sassInfo.connInfo[i].devType = SASS_DEV_TYPE_INVALID;
    }
    INIT_LIST_HEAD(&sassInfo.hDevHead);

    //for (int i = 0; i < HISTORY_DEV_NUM; ++i) {
    //    colist_addto_head(&(sassInfo.historyDev[i].node), &sassInfo.hDevHead);
    //}

    memset((void *)&sassAdv, 0, sizeof(SassAdvInfo));
    sassAdv.lenType = (3 << 4) + SASS_CONN_STATE_TYPE;
    SET_SASS_STATE(sassAdv.state, HEAD_ON, 1);
    SET_SASS_STATE(sassAdv.state, CONN_AVAILABLE, 1);

    TRACE(1, "%s", __func__);
}

void app_fp_sass_sync_info()
{
    TRACE(1, "%s", __func__);

    tws_ctrl_send_cmd(APP_TWS_CMD_SEND_SASS_INFO, NULL, 0);
}

void app_fp_sass_get_sync_info(uint8_t *buf, uint32_t *len)
{
    SassSyncInfo *info = (SassSyncInfo *)buf;
    memcpy((uint8_t *)info->connInfo, (uint8_t *)sassInfo.connInfo, sizeof(SassBtInfo) * BT_DEVICE_NUM);
    info->activeId = sassInfo.activeId;
    *len = sizeof(SassSyncInfo);
}

void app_fp_sass_set_sync_info(uint8_t *buf, uint32_t len)
{
    SassSyncInfo *info = (SassSyncInfo *)buf;
    for(int i= 0; i < BT_DEVICE_NUM; i++)
    {
        if(info->connInfo[i].connId != 0xFF)
        {
            for(int j= 0; j < BT_DEVICE_NUM; j++)
            {
                if(!memcmp((void *)sassInfo.connInfo[j].btAddr.addr, \
                    (void *)info->connInfo[i].btAddr.addr, sizeof(bd_addr_t)))
                {
                    uint8_t tempId = sassInfo.connInfo[j].connId;
                    memcpy((uint8_t *)&sassInfo.connInfo[j], (uint8_t *)&info->connInfo[i], sizeof(SassBtInfo));
                    sassInfo.connInfo[j].connId = tempId;
                    break;
                }
            }
        }
    }

    sassInfo.activeId = info->activeId;
}

void app_fp_sass_gen_session_nonce(uint8_t device_id)
{
    for(int i= 0; i < BT_DEVICE_NUM; i++)
    {
        if(sassInfo.connInfo[i].connId == device_id)
        {
            for(int n = 0; n < SESSION_NOUNCE_NUM; n++)
            {
                 sassInfo.connInfo[i].session[n] = (uint8_t)rand();
            }
            TRACE(1, "sass dev %d session nounce is:", device_id);
            DUMP8("%02x ", sassInfo.connInfo[i].session, 8);
            break;
        }
    }
}

bool app_fp_sass_get_session_nonce(uint8_t device_id, uint8_t *session)
{
    bool ret = false;
    uint8_t zero[SESSION_NOUNCE_NUM] = {0};
    for(int i= 0; i < BT_DEVICE_NUM; i++)
    {
        if((sassInfo.connInfo[i].connId == device_id) && (memcmp(sassInfo.connInfo[i].session, zero, SESSION_NOUNCE_NUM)))
        {
            memcpy(session, sassInfo.connInfo[i].session, SESSION_NOUNCE_NUM);
            ret = true;
            break;
        }
    }
    TRACE(1, "get sass dev %d session is:", device_id);
    DUMP8("0x%2x ", session, SESSION_NOUNCE_NUM);

    return ret;
}

SassBtInfo *app_fp_sass_get_free_handler()
{
    SassBtInfo *handler = NULL;
    for(int i= 0; i < BT_DEVICE_NUM; i++)
    {
        if(sassInfo.connInfo[i].connId == 0xFF)
        {
            handler = &(sassInfo.connInfo[i]);
            break;
        }
    }
    return handler;
}

SassBtInfo *app_fp_sass_get_connected_dev(uint8_t id)
{
    SassBtInfo *handler = NULL;
    for(int i= 0; i < BT_DEVICE_NUM; i++)
    {
        if(sassInfo.connInfo[i].connId == id)
        {
            handler = &(sassInfo.connInfo[i]);
            break;
        }
    }
    return handler;
}

SassBtInfo *app_fp_sass_get_other_connected_dev(uint8_t id)
{
    SassBtInfo *info = NULL;
    for(int i= 0; i < BT_DEVICE_NUM; i++)
    {
        if((sassInfo.connInfo[i].connId != id) && (sassInfo.connInfo[i].connId != 0xFF))
        {
            info = (SassBtInfo *)&(sassInfo.connInfo[i]);
            break;
        }
    }
    return info;
}

void app_fp_sass_remove_dev_handler(bd_addr_t *addr)
{
    for(int i= 0; i < BT_DEVICE_NUM; i++)
    {
        if(!memcmp(sassInfo.connInfo[i].btAddr.addr, (uint8_t *)addr, sizeof(bd_addr_t)))
        {
            memset((void *)&(sassInfo.connInfo[i]), 0, sizeof(SassBtInfo));
            sassInfo.connInfo[i].connId = 0xFF;
            sassInfo.connNum--;
            break;
        }
    }

    if(!memcmp(sassInfo.reconnInfo.reconnAddr.addr, (uint8_t *)addr, sizeof(bd_addr_t)))
    {
        app_fp_sass_reset_reconn_info();
    }

    if(sassInfo.connNum == 0) {
         sassInfo.connAvail = SASS_CONN_AVAILABLE;
         sassInfo.connState = SASS_STATE_NO_CONNECTION;
         sassInfo.focusMode = SASS_CONN_NO_FOCUS;
    }
}

void app_fp_sass_get_adv_data(uint8_t *buf, uint8_t *len)
{
    *len =  (sassAdv.lenType >> 4) + 1;
    memcpy(buf, (uint8_t *)&sassAdv, *len);
    TRACE(1, "sass adv data, len:%d", *len);
    DUMP8("%02x ", buf, *len);
}

void app_fp_sass_encrypt_adv_data(uint8_t *FArray, uint8_t sizeOfFilter, uint8_t *inUseKey, 
                                               uint8_t *outputData, uint8_t *dataLen)
{
    uint8_t sassBuf[4] = {0};//{0x35, 0x85, 0x38, 0x09};
    uint8_t outBuf[16 + 1] = {0};
    uint8_t iv[16] = {0}; //{0x8c, 0xa9, 0X0c, 0x08, 0x1c, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
    uint8_t sassLen = 0;
    uint8_t tempLen = *dataLen;
    app_fp_sass_get_adv_data(sassBuf, &sassLen);
    memcpy(iv, FArray, sizeOfFilter);
    AES128_CTR_encrypt_buffer(sassBuf, sassLen, inUseKey, iv, outBuf + 1);
    TRACE(0, "encrypt connection state is:");
    DUMP8("%2x ", outBuf + 1, sassLen);
    outBuf[0] = (sassLen << 4) + APP_GFPS_RANDOM_RESOLVABLE_DATA_TYPE;
    memcpy(outputData + tempLen, outBuf, sassLen + 1);
    tempLen += (sassLen + 1);
    *dataLen = tempLen;
}

void app_fp_sass_update_adv_data()
{
    uint8_t tempDev = 0;
    for(int i = 0; i < sassInfo.connNum; i++)
    {
        if((tempDev & SASS_DEV_TYPE_PHONEA) && \
            (sassInfo.connInfo[i].devType == SASS_DEV_TYPE_PHONEA))
        {
            tempDev |= SASS_DEV_TYPE_PHONEB;
        }
        else if(sassInfo.connInfo[i].devType != SASS_DEV_TYPE_INVALID)
        {
            tempDev |= sassInfo.connInfo[i].devType;
        }else
        {
        }
    }
    sassAdv.devBitMap = tempDev;

    sassAdv.state = 0;
    //SET_SASS_STATE(sassAdv.state, HEAD_ON, sassInfo.headState);
    SET_SASS_STATE(sassAdv.state, HEAD_ON, 1);
    SET_SASS_STATE(sassAdv.state, CONN_AVAILABLE, sassInfo.connAvail);
    SET_SASS_STATE(sassAdv.state, FOCUS_MODE, sassInfo.focusMode);
    SET_SASS_STATE(sassAdv.state, AUTO_RECONN, sassInfo.autoReconn);
    SET_SASS_CONN_STATE(sassAdv.state, CONN_STATE, sassInfo.connState);
    DUMP8("%02x ", (uint8_t *)&sassAdv, sizeof(SassAdvInfo));
}

void app_fp_sass_set_sass_mode(uint8_t device_id, uint16_t sassVer)
{
    for(int i = 0; i < BT_DEVICE_NUM; i++)
    {
        if(sassInfo.connInfo[i].connId == device_id)
        {
            sassInfo.connInfo[i].sassVer = sassVer;
            break;
        }
   }
}

void app_fp_sass_set_multi_status(uint8_t device_id, bool isMulti)
{
    SassEvtParam evtParam;
    SASS_CONN_AVAIL_E availstate;
    uint8_t num = app_bt_get_num_of_connected_dev();
    uint8_t total = isMulti ? BT_DEVICE_NUM : 1;
    TRACE(3, "%s num:%d, total:%d", __func__, num, total);
    sassInfo.isMulti = isMulti;
    evtParam.devId = device_id;
    evtParam.event = SASS_EVT_UPDATE_MULTI_STATE;
    if(num >= total) {
        availstate = SASS_CONN_NONE_AVAILABLE;
    }else {
        availstate = SASS_CONN_AVAILABLE;
    }
    evtParam.state.connAvail = availstate;
    app_fp_sass_update_state(&evtParam);
}

bool app_fp_sass_get_multi_status()
{
    TRACE(1, "sass is in multi-point state ? %d", sassInfo.isMulti);
    return sassInfo.isMulti;
}

bool app_fp_sass_is_any_dev_connected()
{
    bool ret = false;
    for(int i = 0; i < BT_DEVICE_NUM; i++)
    {
        if(sassInfo.connInfo[i].connId != 0xFF) {
            ret = true;
            break;
        }
    }
    return ret;
}

bool app_fp_sass_is_sass_dev(uint8_t device_id)
{
    bool ret = false;
    for(int i = 0; i < BT_DEVICE_NUM; i++)
    {
        if(sassInfo.connInfo[i].connId == device_id) {
            if(sassInfo.connInfo[i].sassVer) {
                ret = true;
            }
            break;
        }
    }
    return ret;
}

bool app_fp_sass_is_there_sass_dev()
{
    bool ret = false;
    for(int i = 0; i < BT_DEVICE_NUM; i++)
    {
        if(sassInfo.connInfo[i].connId != 0xFF && sassInfo.connInfo[i].sassVer) {
            ret = true;
            break;
        }
    }
    TRACE(1, "there is a sass dev ? %d", ret);
    return ret;
}

void app_fp_sass_set_custom_data(uint8_t data)
{
    sassAdv.cusData = data;
}

void app_fp_sass_get_cap(uint8_t *buf, uint32_t *len)
{
    uint16_t sass_ver = SASS_VERSION;
    uint16_t capbility = 0;

    capbility |= SASS_STATE_ON_BIT;
#ifdef IBRT_V2_MULTIPOINT
    capbility |= SASS_MULTIPOINT_BIT;
#endif
    if(app_fp_sass_get_multi_status())
    {
        capbility |= SASS_MULTIPOINT_ON_BIT;
    }
    capbility |= SASS_ON_HEAD_BIT;
    if(app_fp_sass_get_head_state())
    {
        capbility |= SASS_ON_HEAD_ON_BIT;
    }
    *len = NTF_CAP_LEN;
    buf[0] = (sass_ver >> 8) & 0xFF;
    buf[1] = (sass_ver & 0xFF);
    buf[2] = (capbility >> 8) & 0xFF;
    buf[3] = (capbility & 0xFF);
}

void app_fp_sass_set_switch_pref(uint8_t pref)
{
    sassInfo.preference = pref;
}

uint8_t app_fp_sass_get_switch_pref()
{
    return sassInfo.preference;
}

void app_fp_sass_set_active_dev(uint8_t dev_id)
{
    if(sassInfo.activeId != dev_id)
    {
        sassInfo.idSwitched = true;
        sassInfo.activeId = dev_id;
    }
}

uint8_t app_fp_sass_get_active_dev()
{
    return sassInfo.activeId;
}

void app_fp_sass_set_reconnecting_dev(bd_addr_t *addr, uint8_t evt)
{
    memcpy(sassInfo.reconnInfo.reconnAddr.addr, addr->addr, sizeof(bd_addr_t));
    sassInfo.reconnInfo.evt = evt;
}

bool app_fp_sass_is_reconn_dev(bd_addr_t *addr)
{
    bool ret = false;
    if(!memcmp(sassInfo.reconnInfo.reconnAddr.addr, addr->addr, sizeof(bd_addr_t)))
    {
        ret = true;
    }
    TRACE(1, "sass is reconnect dev ? %d", ret);
    return ret;
}

bool app_fp_sass_is_need_resume(bd_addr_t *addr)
{
    bool ret = false;
    if(!memcmp(sassInfo.reconnInfo.reconnAddr.addr, addr->addr, sizeof(bd_addr_t)) && \
        (sassInfo.reconnInfo.evt == SASS_EVT_SWITCH_BACK_AND_RESUME))
    {
        ret = true;
    }
    //TRACE(1, "sass is need resume a2dp ? %d", ret);
    return ret;
}

bool app_fp_sass_is_profile_connected(bd_addr_t *addr)
{
    bool ret = false;
    for(int i= 0; i < BT_DEVICE_NUM; i++)
    {     
        if(!memcmp(sassInfo.connInfo[i].btAddr.addr, addr->addr, sizeof(bd_addr_t)))
        {
            if((sassInfo.connInfo[i].audState & PROFILE_STATE(A2DP, CONNECTION)) && \
                (sassInfo.connInfo[i].audState & PROFILE_STATE(HFP, CONNECTION)) && \
                (sassInfo.connInfo[i].audState & PROFILE_STATE(AVRCP, CONNECTION)))
            {
                ret = true;
            }
            break;
        }
    }

    TRACE(1, "sass is need resume a2dp ? %d", ret);
    return ret;
}

void app_fp_sass_reset_reconn_info()
{
    memset((uint8_t *)&(sassInfo.reconnInfo), 0xFF, sizeof(sassInfo.reconnInfo));
    TRACE(0, "reset sass reconn info");
    return;
}

#if 0
void app_fp_sass_update_last_dev(bd_addr_t *addr)
{
    bool found = false;
    struct list_node *node = colist_get_head(&sassInfo.hDevHead);
    struct list_node *list = &sassInfo.hDevHead;
    bd_addr_t *tempAddr = NULL;
    while(node && (node != list))
    {
        tempAddr = NODE_BD_ADDR(node);
        TRACE(0, "node dev addr is:");
        DUMP8("%02x ", tempAddr, 6);
        if(!memcmp(tempAddr, addr, sizeof(bd_addr_t)))
        {
            found = true;
            colist_moveto_head(node, list);
            break;
        }
        node = node->next;
    }

    if(!found)
    {
        for(int i = 0; i < HISTORY_DEV_NUM; i++)
        {
            if(!colist_is_node_on_list(&(sassInfo.historyDev[i].node), list))
            {
                memcpy(sassInfo.historyDev[i].addr.addr, addr->addr, sizeof(bd_addr_t));
                colist_addto_head(&(sassInfo.historyDev[i].node), list);
                break;
            }

            if(i == HISTORY_DEV_NUM)
            {
                node = colist_get_head(list);
                tempAddr = NODE_BD_ADDR(node);
                memcpy(tempAddr->addr, addr->addr, sizeof(bd_addr_t));
                colist_moveto_head(node, list);
            }
        }
    }
}

bd_addr_t *app_fp_sass_get_last_dev(bd_addr_t *currAddr)
{
    bd_addr_t *addr = NULL;
    struct list_node *list = &sassInfo.hDevHead;
    struct list_node *head = colist_get_head(list);
    while(head && (head != list))
    {
        addr = NODE_BD_ADDR(head);
        if(!memcmp(currAddr->addr, addr->addr, sizeof(bd_addr_t)))
        {
            head = head->next;
            addr = NULL;
        }else {
            addr = NODE_BD_ADDR(head);
            break;
        }
    }

    return addr;
}
#else
void app_fp_sass_update_last_dev(bd_addr_t *addr)
{
    if(addr)
    {
        memcpy(sassInfo.lastDev.addr, addr->addr, sizeof(bd_addr_t));
    }
}

void app_fp_sass_clear_last_dev()
{
    memset(sassInfo.lastDev.addr, 0, sizeof(bd_addr_t));
}

void app_fp_sass_get_last_dev(bd_addr_t *lastAddr)
{
    memcpy(lastAddr->addr, sassInfo.lastDev.addr, sizeof(bd_addr_t));
}

#endif

#ifdef __BT_ONE_BRING_TWO__
uint8_t app_fp_sass_switch_src_evt_hdl(uint8_t device_id, uint8_t evt)
{
    uint8_t reason = SASS_STATUS_OK;
    uint8_t awayId, switchId, currentId, otherId = 0xFF;
    SassBtInfo *info = app_fp_sass_get_other_connected_dev(device_id);
    if(info)
    {
        otherId = info->connId;
    }

    if(evt & SASS_SWITCH_TO_CURR_DEV_BIT)
    {
        switchId = device_id;
        awayId = otherId;
    }
    else
    {
        switchId = otherId;
        awayId = device_id;
    }

    TRACE(3, "sass switch src to %d from %d, other:%d", switchId, awayId, otherId);
    currentId = app_bt_audio_get_curr_a2dp_device();
    app_fp_sass_set_active_dev(switchId);
    if(currentId == switchId) {
        return SASS_STATUS_REDUNTANT;
    }else {
        //app_audio_manager_switch_a2dp((enum BT_DEVICE_ID_T)device_id);
        //app_audio_manager_set_active_sco_num((enum BT_DEVICE_ID_T)device_id);

        if(!app_bt_audio_switch_streaming_a2dp())
        {
            app_bt_audio_stop_a2dp_playing(currentId);
            app_bt_switch_to_prompt_a2dp_mode();
        }
        app_bt_ibrt_audio_pause_a2dp_stream(currentId);
    }

    if(evt & SASS_RESUME_ON_SWITCH_DEV_BIT)
    {
        app_bt_resume_music_player(switchId);
    }

    if(evt & SASS_REJECT_SCO_ON_AWAY_DEV_BIT)
    {
        app_bt_hf_set_reject_dev(awayId);
    }

    if(evt & SASS_DISCONN_ON_AWAY_DEV_BIT)
    {
        SassBtInfo *info = app_fp_sass_get_connected_dev(awayId);
        app_fp_sass_update_last_dev(&(info->btAddr));
        app_bt_disconnect_link_by_id(awayId);
    }
    return reason;
}
#endif

uint8_t app_fp_sass_switch_back_hdl(uint8_t device_id, uint8_t evt)
{
    uint8_t isMulti;
    bd_addr_t currAddr, lastAddr;

    if((evt != SASS_EVT_SWITCH_BACK) && (evt != SASS_EVT_SWITCH_BACK_AND_RESUME))
    {
        return  SASS_STATUS_FAIL;
    }

    app_bt_get_device_bdaddr(device_id, currAddr.addr);
    app_fp_sass_get_last_dev(&lastAddr);
    isMulti = app_fp_sass_get_multi_status();

    if(!isMulti)
    {
        app_ibrt_if_disconnet_moblie_device((bt_bdaddr_t *)&currAddr);
    }

    if(memcmp(lastAddr.addr, currAddr.addr, sizeof(bd_addr_t)))
    {
        TRACE(0, "sass switch back to dev:");
        DUMP8("%02x ", lastAddr.addr, 6);
        app_ibrt_if_reconnect_moblie_device((bt_bdaddr_t *)&lastAddr);

        if(evt == SASS_EVT_SWITCH_BACK_AND_RESUME)
        {
            if(app_fp_sass_is_profile_connected(&lastAddr)) {
                uint8_t lastId = app_bt_get_devId_from_addr(lastAddr.addr);
                app_bt_resume_music_player(lastId);
            } else {
                app_fp_sass_set_reconnecting_dev(&lastAddr, evt);
            }
        }
    }
    else
    {
        TRACE(0, "sass switch back hdl disconnect itself");
        app_ibrt_if_disconnet_moblie_device((bt_bdaddr_t *)&currAddr);
    }
    return SASS_STATUS_OK;
}

SASS_CONN_STATE_E app_fp_sass_get_conn_state()
{
    return sassInfo.connState;
}

void app_fp_sass_set_conn_state(SASS_CONN_STATE_E state)
{
    TRACE(1, "set sass conn state:0x%0x", state);

    sassInfo.connState = state;
}

SASS_HEAD_STATE_E app_fp_sass_get_head_state()
{
    return sassInfo.headState;
}

void app_fp_sass_set_head_state(SASS_HEAD_STATE_E headstate)
{
    sassInfo.headState = headstate;
}

void app_fp_sass_set_conn_available(SASS_CONN_AVAIL_E available)
{
    sassInfo.connAvail= available;
}

void app_fp_sass_set_focus_mode(SASS_FOCUS_MODE_E focus)
{
    sassInfo.focusMode = focus;
}

void app_fp_sass_set_auto_reconn(SASS_AUTO_RECONN_E focus)
{
    sassInfo.autoReconn = focus;
}

void app_fp_sass_set_init_conn(uint8_t device_id, bool bySass)
{
    SassBtInfo *info = app_fp_sass_get_connected_dev(device_id);
    if(info)
    {
        info->initbySass = bySass;
    }
}

void app_fp_sass_set_inuse_acckey_by_dev(uint8_t device_id, uint8_t *accKey)
{
    memcpy(sassInfo.inuseKey, accKey, FP_ACCOUNT_KEY_SIZE);
    SassBtInfo *sInfo = app_fp_sass_get_connected_dev(device_id);
    if(sInfo)
    {
        memcpy(sInfo->accKey, accKey, FP_ACCOUNT_KEY_SIZE);
        sInfo->updated = true;
    }
    TRACE(1, "sass dev %d inuse acckey is:", device_id);
    DUMP8("0x%2x ", accKey, FP_ACCOUNT_KEY_SIZE);
    //sassInfo.inuseKey[0] = SASS_IN_USE_ACCOUNT_KEY;
}

void app_fp_sass_set_inuse_acckey(uint8_t *accKey)
{
    memcpy(sassInfo.inuseKey, accKey, FP_ACCOUNT_KEY_SIZE);
}

void app_fp_sass_get_inuse_acckey(uint8_t *accKey)
{
    memcpy(accKey, sassInfo.inuseKey, FP_ACCOUNT_KEY_SIZE);
}

bool app_fp_sass_get_inuse_acckey_by_id(uint8_t device_id, uint8_t *accKey)
{
    bool ret = false;
    uint8_t key[FP_ACCOUNT_KEY_SIZE] = {0};
    SassBtInfo *sInfo = app_fp_sass_get_connected_dev(device_id);
    if(sInfo && memcmp(sInfo->accKey, key, FP_ACCOUNT_KEY_SIZE))
    {
        memcpy(accKey, sInfo->accKey, FP_ACCOUNT_KEY_SIZE);
        ret = true;
    }
    TRACE(1, "get sass dev %d inuse acckey is:", device_id);
    DUMP8("0x%2x ", accKey, FP_ACCOUNT_KEY_SIZE);
    return ret;
}

void app_fp_sass_set_drop_dev(uint8_t device_id)
{
    SassBtInfo *info = app_fp_sass_get_connected_dev(device_id);
    memcpy(sassInfo.dropDevAddr.addr, info->btAddr.addr, sizeof(bd_addr_t));
}

SASS_DEV_TYPE_E app_fp_sass_get_dev_type_by_cod(uint8_t *cod)
{
    SASS_DEV_TYPE_E type;
    uint32_t devCod = cod[2] + (cod[1] << 8) + (cod[0] << 16);
    if(devCod & COD_TYPE_LAPTOP) {
        type = SASS_DEV_TYPE_LAPTOP;
    }else if(devCod & COD_TYPE_TABLET) {
        type = SASS_DEV_TYPE_TABLET;
    }else if(devCod & COD_TYPE_TV) {
        type = SASS_DEV_TYPE_TV;
    }else if(devCod & COD_TYPE_PHONE) {
        type = SASS_DEV_TYPE_PHONEA;
    }else {
        type = SASS_DEV_TYPE_PHONEA;//SASS_DEV_TYPE_INVALID;
    }
    TRACE(2, "%s type: 0x%0x", __func__, type);
    return type;
}

bool app_fp_sass_is_id_switched()
{
    return sassInfo.idSwitched;
}

void app_fp_sass_reset_id_switched()
{
    sassInfo.idSwitched = false;
}

SASS_CONN_STATE_E app_fp_sass_get_state(uint8_t devId, SASS_CONN_STATE_E entry)
{
    SASS_CONN_STATE_E state;
    SassBtInfo *otherInfo = app_fp_sass_get_other_connected_dev(devId);
    TRACE(3, "activeId:%d, devId:%d, entry:%d", sassInfo.activeId, devId, entry);
    if(otherInfo)
    {
        if(sassInfo.activeId == otherInfo->connId)
        {
            state = otherInfo->state;
        }
        else if(sassInfo.activeId == devId){
            state = entry;
        }
        else if(otherInfo->state > SASS_STATE_NO_AUDIO || entry > SASS_STATE_NO_AUDIO)
        {
            if(otherInfo->state > entry)
            {
                state = otherInfo->state;
                sassInfo.activeId = otherInfo->connId;
            }else {
                state = entry;
                sassInfo.activeId = devId;
            }
        }
        else
        {
            state = (otherInfo->state > entry) ? otherInfo->state : entry;
        }
    }
    else
    {
        state = entry;
    }

    return state;
}

SASS_CONN_STATE_E app_fp_sass_update_conn_state(uint8_t devId)
{
    SASS_CONN_STATE_E connState = SASS_STATE_NO_CONNECTION;
    SassBtInfo *info = app_fp_sass_get_connected_dev(devId);
    if(info)
    {
        if(info->audState & PROFILE_STATE(HFP, AUDIO)) {
            info->state = SASS_STATE_HFP;
        }else if(info->audState & PROFILE_STATE(AVRCP, AUDIO)) {
            info->state = SASS_STATE_A2DP_WITH_AVRCP;
        }else if(info->audState& PROFILE_STATE(A2DP, AUDIO)) {
            info->state = SASS_STATE_ONLY_A2DP;
        }else if((info->audState & PROFILE_STATE(HFP, CONNECTION)) || \
            (info->audState & PROFILE_STATE(A2DP, CONNECTION))) {
            info->state = SASS_STATE_NO_DATA;
        }else {
            info->state = SASS_STATE_NO_CONNECTION;
        }
    }else {
        info->state = SASS_STATE_NO_CONNECTION;
    }
    connState = app_fp_sass_get_state(devId, info->state);
    TRACE(2, "update profile state:0x%0x, info->state:%d", connState, info->state);
    return connState;
}

void app_fp_sass_profile_event_handler(SASS_PROFILE_ID_E pro, uint8_t devId, uint8_t event, uint8_t *param)
{
    bd_addr_t addr;
    bool needUpdate = true;
    bool needResume = false;
    SASS_CONN_STATE_E tempState;
    SASS_CONN_STATE_E updateState;
    uint8_t pState = 0;
    uint8_t pBit = 0;

    SassBtInfo *sInfo = app_fp_sass_get_connected_dev(devId);
    if(!sInfo)
    {
        return;
    }

    app_bt_get_device_bdaddr(devId, addr.addr);
    needResume = app_fp_sass_is_need_resume(&addr);

    TRACE(5,"%s id:%d event:%d pro:%d avtiveId:%d", __func__, devId, event,pro,sassInfo.activeId);

    if(pro == SASS_PROFILE_A2DP)
    {
        pBit = SASS_PROFILE_BIT_A2DP;
        switch(event)
        {
            case BTIF_A2DP_EVENT_STREAM_OPEN:
                pState = UPDATE_PROFILE_STATE(CONNECTION, 1);
                break;

            case BTIF_A2DP_EVENT_STREAM_CLOSED:
                pState = UPDATE_PROFILE_STATE(CONNECTION, 0);			
#ifdef __BT_ONE_BRING_TWO__
                app_bt_switch_to_multi_a2dp_quick_switch_play_mode();
#endif
                break;

            case BTIF_A2DP_EVENT_STREAM_STARTED:
            case BTIF_A2DP_EVENT_STREAM_STARTED_MOCK:
                pState = UPDATE_PROFILE_STATE(AUDIO, 1);
                app_fp_sass_set_active_dev(devId);
#ifdef __BT_ONE_BRING_TWO__
				SassBtInfo *otherInfo = NULL;
                app_bt_switch_to_multi_a2dp_quick_switch_play_mode();
                otherInfo = app_fp_sass_get_other_connected_dev(devId);
                if(otherInfo && (app_bt_is_a2dp_streaming(otherInfo->connId)))
                {
                    app_bt_ibrt_audio_pause_a2dp_stream(otherInfo->connId);
                }
#endif
                break;

            case BTIF_A2DP_EVENT_STREAM_SUSPENDED:
                pState = UPDATE_PROFILE_STATE(AUDIO, 0);
                break;
            default:
                needUpdate = false;
                break;
        }
    }
    else if(pro == SASS_PROFILE_HFP)
    {
        pBit = SASS_PROFILE_BIT_HFP;
        switch(event)
        {
            case BTIF_HF_EVENT_AUDIO_CONNECTED:
                pState = UPDATE_PROFILE_STATE(AUDIO, 1);
                app_fp_sass_set_active_dev(devId);
                break;

            case BTIF_HF_EVENT_AUDIO_DISCONNECTED:
                pState = UPDATE_PROFILE_STATE(AUDIO, 0);
                break;

            case BTIF_HF_EVENT_SERVICE_DISCONNECTED:
                pState = UPDATE_PROFILE_STATE(CONNECTION, 0);
                break;

            case BTIF_HF_EVENT_SERVICE_CONNECTED:
                pState = UPDATE_PROFILE_STATE(CONNECTION, 1);
                break;

            default:
            needUpdate = false;
            break;
        }
    }
    else if(pro == SASS_PROFILE_AVRCP)
    {
        pBit = SASS_PROFILE_BIT_AVRCP;
        switch(event)
        {
            case BTIF_AVCTP_CONNECT_EVENT:
                pState = UPDATE_PROFILE_STATE(CONNECTION, 1);
                break;

            case BTIF_AVCTP_DISCONNECT_EVENT:
                pState = UPDATE_PROFILE_STATE(CONNECTION, 0);
                break;

            case BTIF_AVRCP_EVENT_ADV_NOTIFY:
                if(*param == BTIF_AVRCP_MEDIA_PLAYING)
                {
                    pState = UPDATE_PROFILE_STATE(AUDIO, 1);
                }else if(*param == BTIF_AVRCP_MEDIA_PAUSED || *param == BTIF_AVRCP_MEDIA_STOPPED)
                {
                    if(sInfo->audState & PROFILE_STATE(A2DP, AUDIO))
                    {
                         pState = UPDATE_PROFILE_STATE(AUDIO, 0);
                        sInfo->audState = UPDATE_ALL_STATE(sInfo->audState, pState, SASS_PROFILE_BIT_A2DP);
                    }
                    pState = UPDATE_PROFILE_STATE(AUDIO, 0);
                }else {
                    needUpdate = false;
                }
                break;

            default:
                needUpdate = false;
                break;
        }
    }
    else
    {
        needUpdate = false;
        TRACE(1,"%s sass profile update error", __func__);
    }

    if(needUpdate)
    {
        sInfo->audState = UPDATE_ALL_STATE(sInfo->audState, pState, pBit);

        TRACE(2,"%s sass audState: 0x%0x", __func__, sInfo->audState);
        tempState = app_fp_sass_get_conn_state();
        updateState = app_fp_sass_update_conn_state(devId);

        if(tempState != updateState)
        {
            SassEvtParam evtParam;
            evtParam.devId = devId;
            evtParam.event = SASS_EVT_UPDATE_CONN_STATE;
            evtParam.state.connState = updateState;
            app_fp_sass_update_state(&evtParam);
        }
    }

    if(needResume && (sInfo->audState & PROFILE_STATE(A2DP, CONNECTION)) && \
        (sInfo->audState & PROFILE_STATE(AVRCP, CONNECTION)))
    {
        app_bt_resume_music_player(devId);
        sassInfo.reconnInfo.evt = 0xFF;
    }
}

void app_fp_sass_get_acckey_from_nv(uint8_t *addr, uint8_t *key)
{
    nv_record_fp_get_key_by_addr(addr, key);
}

void app_fp_sass_add_dev_handler(uint8_t devId, bd_addr_t *addr)
{
    uint8_t cod[3];
    uint8_t connNum = 0;
    SassBtInfo *btHdl;

    if(sassInfo.isMulti) {
        connNum = BT_DEVICE_NUM;
    }else {
        connNum = 1;
    }
#if 0
#ifndef IBRT_V2_MULTIPOINT
    SassBtInfo *connHdl = NULL;
    for(int i = 0; i < BT_DEVICE_NUM; i++)
    {
        connHdl = &(sassInfo.connInfo[i]);
        if((connHdl->connId != 0xFF) && (connHdl->connId !=devId))
        {
            break;
        }
    }
    if(connHdl)
    {
        app_bt_disconnect_link_by_id(connHdl->connId);
    }
#endif
#endif
    app_bt_get_remote_cod_by_addr((bt_bdaddr_t *)addr, cod);
    btHdl = app_fp_sass_get_free_handler();
    TRACE(4,"%s cod: 0x%0x %0x %0x", __func__, cod[0], cod[1], cod[2]);
    if(btHdl)
    {
        memcpy(btHdl->btAddr.addr, addr, sizeof(bt_bdaddr_t));  
        app_fp_sass_get_acckey_from_nv((uint8_t *)addr, btHdl->accKey);
        btHdl->devType = app_fp_sass_get_dev_type_by_cod(cod);
        btHdl->connId = devId;
    }
    sassInfo.connNum++;
    TRACE(2,"%s connNum: %d", __func__, sassInfo.connNum);

    if(sassInfo.connNum == 1) {
        sassInfo.connState = SASS_STATE_NO_DATA;
        sassInfo.focusMode = SASS_CONN_NO_FOCUS;
    }

    if(sassInfo.connNum < connNum){
        sassInfo.connAvail = SASS_CONN_AVAILABLE;
    }else{
        sassInfo.connAvail = SASS_CONN_NONE_AVAILABLE;
    }

    if(!memcmp(sassInfo.reconnInfo.reconnAddr.addr, addr, sizeof(bd_addr_t)))
    {
        sassInfo.autoReconn = SASS_AUTO_RECONNECTED;
        if(sassInfo.reconnInfo.evt != SASS_EVT_SWITCH_BACK_AND_RESUME)
        {
            sassInfo.reconnInfo.evt = 0xFF;
        }
    }
}

void app_fp_sass_del_dev_handler(bd_addr_t *addr, uint8_t reason)
{
    app_fp_sass_remove_dev_handler(addr);
    if (reason == BTIF_BEC_MAX_CONNECTIONS) {
        app_fp_sass_update_last_dev(addr);
    }else if (!app_fp_sass_is_any_dev_connected()) {
        app_fp_sass_clear_last_dev();
    }else {
        TRACE(1,"%s don't update last dev", __func__);
    }
    app_bt_accessmode_set(BTIF_BAM_CONNECTABLE_ONLY);
    TRACE(2,"%s connNum: %d", __func__, sassInfo.connNum);
}

void app_fp_sass_connect_handler(uint8_t device_id, bd_addr_t *addr)
{
    SassEvtParam evtParam;
    evtParam.event = SASS_EVT_ADD_DEV;
    evtParam.devId = device_id;
    memcpy(evtParam.addr.addr, addr, sizeof(bd_addr_t)); 
    app_fp_sass_update_state(&evtParam);
}

void app_fp_sass_disconnect_handler(uint8_t device_id, bd_addr_t *addr, uint8_t errCode)
{
    SassEvtParam evtParam;
    evtParam.event = SASS_EVT_DEL_DEV;
    evtParam.devId = device_id;
    evtParam.reason = errCode;
    memcpy(evtParam.addr.addr, addr, sizeof(bd_addr_t)); 
    app_fp_sass_update_state(&evtParam);
}

void app_fp_sass_update_state(SassEvtParam *evtParam)
{
    uint8_t devId = evtParam->devId;
    uint8_t oldAdv[4], newAdv[4];
    uint8_t advLen = 0;

    app_fp_sass_get_adv_data(oldAdv, &advLen);

    TRACE(2, "sass update state evt:%d, devId:%d", evtParam->event, devId);
    switch(evtParam->event)
    {
        case SASS_EVT_ADD_DEV:
        {       
            app_fp_sass_add_dev_handler(devId, &(evtParam->addr));      
            break;
        }

        case SASS_EVT_DEL_DEV:
        {
            app_fp_sass_del_dev_handler(&(evtParam->addr), evtParam->reason);
            break;
        }

        case SASS_EVT_UPDATE_CONN_STATE:
        {
            app_fp_sass_set_conn_state(evtParam->state.connState);
            break;
        }

        case SASS_EVT_UPDATE_HEAD_STATE:
        {
            SASS_HEAD_STATE_E headstate = evtParam->state.headState;
            app_fp_sass_set_head_state(headstate);
            break;
        }

        case SASS_EVT_UPDATE_FOCUS_STATE:
        {
            SASS_FOCUS_MODE_E focusstate = evtParam->state.focusMode;
            app_fp_sass_set_focus_mode(focusstate);
            break;
        }

        case SASS_EVT_UPDATE_RECONN_STATE:
        {
            SASS_AUTO_RECONN_E reconnstate = evtParam->state.autoReconn;
            app_fp_sass_set_auto_reconn(reconnstate);
            break;
        }

        case SASS_EVT_UPDATE_CUSTOM_DATA:
        {
            app_fp_sass_set_custom_data(evtParam->state.cusData);
            break;
        }

        case SASS_EVT_UPDATE_MULTI_STATE:
        {
            app_fp_sass_set_conn_available(evtParam->state.connAvail);
            break;
        }

        default:
        break;
    }

    app_fp_sass_update_adv_data();
    app_fp_sass_get_adv_data(newAdv, &advLen);
    if(memcmp(newAdv, oldAdv, advLen))
    {
        bool ntfSwitch = false;
        SASS_REASON_E reason = SASS_REASON_UNSPECIFIED;
        SASS_CONN_STATE_E state;
        SassBtInfo *info = NULL;

        app_ble_refresh_adv_state(BLE_FASTPAIR_NORMAL_ADVERTISING_INTERVAL);

        //info = app_fp_sass_get_connected_dev(devId);
        //if(info && info->updated)
        {
            app_fp_msg_sass_ntf_conn_status(devId);
        }

        if(evtParam->event == SASS_EVT_UPDATE_CONN_STATE && app_fp_sass_is_id_switched())
        {
            ntfSwitch = true;
            state = evtParam->state.connState;
            if(state == SASS_STATE_ONLY_A2DP || state == SASS_STATE_A2DP_WITH_AVRCP){
                reason = SASS_REASON_A2DP;
            }else if(state == SASS_STATE_HFP) {
                reason = SASS_REASON_HFP;
            }else {
                reason = SASS_REASON_UNSPECIFIED;
            }
            app_fp_msg_sass_ntf_switch_evt(devId, reason);
        }

        info  = app_fp_sass_get_other_connected_dev(devId);
        if(info && (info->connId != 0xFF))
        {
            uint8_t acckeyA[FP_ACCOUNT_KEY_SIZE];
            uint8_t acckeyB[FP_ACCOUNT_KEY_SIZE];
            app_fp_sass_get_inuse_acckey_by_id(devId, acckeyA);
            memcpy(acckeyB, info->accKey, FP_ACCOUNT_KEY_SIZE);
            if(!memcmp(acckeyA, acckeyB, FP_ACCOUNT_KEY_SIZE) && info->updated)
            {
                app_fp_msg_sass_ntf_conn_status(info->connId);
            }

            if(ntfSwitch)
            {
                app_fp_msg_sass_ntf_switch_evt(info->connId, reason);
            }
        }

        app_fp_sass_reset_id_switched();
    }
}

#endif
