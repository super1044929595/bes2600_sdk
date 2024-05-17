对于待升级的bin,可以用脚本压缩下，压缩率在60%以上，即原始2M大小的iamge,压缩完成之后会小于1.2M。在完成接收之后，我们会解压。
==》
1. 在编译app iamge的时候加入BES_OTA=1 OTA_BIN_COMPRESSED=1 FLASH_REMAP=0 
2. 压缩待升级的image
./tools/build_compressed_ota.sh out/best1503_ibrt_anc/best1503_ibrt_anc.bin out/xxxxx.bin
3. 对上一步压缩完成的iamge生成crc
python2 tools/generate_crc32_of_image.py out/xxxxx.bin
最终生成out/xxxxx.bin.converted.bin
4. 使用xxxxx.bin.converted.bin + bestechnic app进行ota


下载网址
git@codeup.aliyun.com:6342cef7bd947c7ec67c2931/BES/TWS-2056-04SDK.git

app_bt_count_connected_device 

#define TRACE(str, ...)                 do { printf("%s/" str "\n", __FUNCTION__, __VA_ARGS__); } while (0)

//认证
GFPS_ENABLED 谷歌弹窗

//---------------------API 接口--------------------------------//
app_ibrt_nvrecord_delete_all_mobile_record();    //清除手机连接记录
void app_ibrt_customif_on_access_mode_changed(btif_accessible_mode_t newAccessibleMode)  // 进入可发现可配对模式
app_ibrt_customif_tws_on_acl_state_changed；// tws 链接状态
app_ibrt_customif_tws_on_paring_state_changed  //tws 配对状态
app_bt_count_mobile_link()；获取手机连接数量
app_bt_stream_volumeset（）;音量设置
app_bt_stream_a2dpvolume_reset();
app_bt_stream_hfpvolume_reset();

//按键关键字
CAP_KEY_EVENT_UPSLIDE,
CAP_KEY_EVENT_DOWNSLIDE,
APP_SLIDE_KEY_EVENT_UP2SLIDE,
APP_SLIDE_KEY_EVENT_DOWN2SLIDE,
APP_KEY_EVENT,
double kill,
avrcp_key = ,
VOL,
key_click_handle,


////////////////////////////////////////////////////////////////////////////////////////////////////////
恢复音量
void app_reset_volhandle(void)
{
  default_stream_volume.a2dp_vol=8;
  default_stream_volume.hfp_vol=8;
    
  TRACE(1, "%s jw app_reset_volhandle %x", __func__,bes_bt_me_count_mobile_link());
  struct BT_DEVICE_T* curr_device = app_bt_get_device(app_bt_audio_get_curr_sco_device());  
  btif_hf_report_speaker_volume(curr_device->hf_channel,8); 
  TRACE(2,"\r\n jw update sco %d , hfp :%d",app_bt_audio_get_curr_sco_device(),app_bt_audio_get_curr_hfp_device());
  TRACE(1,"\r\n jw update sco hf_channel %d",*curr_device->hf_channel);

  curr_device = app_bt_get_device(bt_media_current_music_get());
  if (!curr_device)
  {
     TRACE(2, "%s jw invalid sbc id %x", __func__, bt_media_current_music_get());
  }else{
    curr_device = app_bt_get_device(bt_media_current_music_get());
    if (!curr_device)
    {
      TRACE(2, "%s jw invalid sbc id %x", __func__, bt_media_current_music_get());
    }
    app_bt_stream_volumeset(9);
    a2dp_volume_set_local_vol(curr_device->device_id, 9);
    app_audio_control_streaming_volume_down();
  }
}

void  xos_App_MediaDef_ResetHandle(void)
{
    //hfp vol 
  //TRACE(1, "%s jw app_reset_volhandle %x", __func__,bes_bt_me_count_mobile_link());
  //struct BT_DEVICE_T* curr_device = app_bt_get_device(app_bt_audio_get_curr_sco_device());  
  //btif_hf_report_speaker_volume(curr_device->hf_channel,8); 
  //TRACE(2,"\r\n jw update sco %d , hfp :%d",app_bt_audio_get_curr_sco_device(),app_bt_audio_get_curr_hfp_device());
  //TRACE(1,"\r\n jw update sco hf_channel %d",*curr_device->hf_channel);
    //media vol
  
    uint8_t device_id = bt_media_current_music_get();
  struct BT_DEVICE_T* curr_device = NULL;
  TRACE(1,"%s jw get current media id : %d",__func__,device_id);
    if( app_bt_stream_isrun(APP_BT_STREAM_A2DP_SBC)&&(device_id!=0xFF) ){
      app_bt_stream_volumeset(9);
      uint8_t a2dp_local_vol = a2dp_volume_local_get(device_id);
      TRACE(1,"%s jw get current a2dp_local_vol" ,__func__);
      a2dp_volume_set_local_vol(a2dp_local_vol, 9);
      app_audio_control_streaming_volume_down();
    }
}

void xos_App_HfpDef_ResetHandle(void)
{
    default_stream_volume.a2dp_vol=8;
    default_stream_volume.hfp_vol=8;
  
    uint8_t i;
    btif_remote_device_t *remDev = NULL;
    btif_link_mode_t mode = BTIF_BLM_SNIFF_MODE;
    btif_hf_channel_t* chan;
    struct BT_DEVICE_T* curr_device = app_bt_get_device(app_bt_audio_get_curr_sco_device());  

    for(i = 0; i < BT_DEVICE_NUM; i++) {
        remDev = (btif_remote_device_t *)btif_hf_cmgr_get_remote_device(app_bt_get_device(i)->hf_channel);
        if (remDev){
            mode = btif_me_get_current_mode(remDev);
        } else {
            mode = BTIF_BLM_SNIFF_MODE;
        }
        chan = app_bt_get_device(i)->hf_channel;
        TRACE(2,"d%x report hfp volume:%d", i, mode == BTIF_BLM_ACTIVE_MODE);
        if ((btif_get_hf_chan_state(chan) == BT_HFP_CHAN_STATE_OPEN) && (mode == BTIF_BLM_ACTIVE_MODE) &&
             app_bt_audio_get_curr_playing_sco() == i)
  {
            btif_hf_report_speaker_volume(chan, 8);
        }
    }

}

--------------------------------------------------------------------------------------------------------------
操作 nv区

int nvrecord_facreset_defvol(bool op)
{
  if (NULL == nvrecord_extension_p )
  {
    return -1;
  }
    uint32_t lock = nv_record_pre_write_operation();
  //
  nvrecord_extension_p->defvol_resetEnable=1;
  //
    nv_record_update_runtime_userdata();
    nv_record_post_write_operation(lock);
  
    nv_record_flash_flush();

  return 0;
}    

bool nvrecord_facreset_readdefvol(void)
{
 return  nvrecord_extension_p->defvol_resetEnable;
}


#define BT_MAX_TX_PWR_IDX     (3)        //idx0~5
#define BT_INIT_TX_PWR_IDX    (3)
--------------------------------------------------------------------------------------------------------------



//蓝牙状态
app_audio_ctrol:active_bt // 音乐播放状态显示
app_ibrt_customif_ui.cpp
|
|--------耳机role状态----|---app_ibrt_conn_get_ui_role()
|
|
|--------媒体状态 bool bt_media_is_music_media_active(void) 是否播放歌曲
|
|
|
|--------app_ibrt_customif_on_mobile_acl_state_changed  手机蓝牙连接状况
|                                                      |
|                                                      |------断开所有手机连接app_disconnect_all_bt_connections
|                                                      |------清除所有手机连接记录app_ibrt_nvrecord_delete_all_mobile_record
|                                                      |
|
|----系统状态--- |
|               |
|               |-----    app_bt_accessmode_set_req(BTIF_BAM_GENERAL_ACCESSIBLE);
                          app_bt_enter_pairing_mode()

                          if(app_status_indication_get()==APP_STATUS_INDCATION_PAGESCAN){ 
                              app_bt_accessmode_set_req(BTIF_BAM_GENERAL_ACCESSIBLE); //开启蓝牙广播
                          }           
                |------   系统处在什么配对模式 app_bt_get_curr_access_mode()
                |------   p_ibrt_ctrl = app_tws_ibrt_get_bt_ctrl_ctx();
                |------   app_bt_stream_volume_update() 生效声音 调整后刷线下不用重启

|
|
|--------app_ibrt_customif_tws_on_acl_state_changed     tws 连接状态  -------|
                                                                            |
                                                                            ------IBRT_CONN_ACL_CONNECTED    tws连接指示灯
                                                                            ------IBRT_CONN_ACL_DISCONNECTED tws断开超时判断或者               app_ibrt_if_event_entry(APP_UI_EV_TWS_RECONNECT);
|
|
--------手机连接模式  btif_me_set_accessible_mode(BTIF_BAM_GENERAL_ACCESSIBLE,NULL); \

                                                                     进入可发现配对模式|
|
|
|
|
|
|
|---mobile conn---|---app_bt_ibrt_has_mobile_link_connected()   主耳单耳是否连接手机 
|                 |#include "app_tws_ibrt_conn_api.h"
|                 |---app_tws_ibrt_slave_ibrt_link_connected()  从耳手机是否连接
|                 |---app_ibrt_nvrecord_get_latest_mobiles_addr（） 配对记录
|
|
|-------HFP-------|
|                 |
|                 |------- 来电铃声 btif_hf_is_inbandring_enabled() ::HF_EVENT_RING_IND in-band-ring-tone     \
                           手机传过来的铃声（1）,耳机本地音(0) 
|                 |------- 通话状态判断 btapp_hfp_is_sco_active()
|                 |------- ::HF_EVENT_RING_IND in-band-ring-ton 来电铃声
|
|-------ADP-------|
|                 |
|                 |-------媒体状态判断     a2dp_is_music_ongoing()
                  |-------媒体播放状态刷新 app_ibrt_customif_avrcp_callback()
                  |-------调整电影模延时   sbm_update_target_jitter_buf_length(uint16_t targetMs)<--a2dp_aduio_triggered_handler     if (APP_IBRT_MOBILE_LINK_CONNECTED(&curr_device->remote))

|
|------TWS--------|
|                 |------ tws 连接状态 app_tws_ibrt_tws_link_connected()
|                 |         
|                 |-------tws 角色判断 app_ibrt_conn_get_ui_role()
|                 |
|                 |-------tws 配对状态回调 app_ibrt_customif_tws_on_acl_state_changed()----|
|                 |                                                                       |---断链类型 reason  08断链
|                 |
|                 |-------tws 单耳还是双耳判断app_ibrt_if_is_in_freeman_mode()
|
|------GFPS-------|
|                 |------app_tell_battery_info_handler() 电量显示  \
                        *batteryValueCount = 3 2耳机加充电盒的个数 batteryValue[1] 不  显示的写0x7F
                        uint8_t gfps_ble_generate_accountkey_data(uint8_t *outputData)  谷歌弹窗发出的内容


