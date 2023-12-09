cur_dir := $(dir $(lastword $(MAKEFILE_LIST)))

obj-y := $(patsubst $(cur_dir)%,%,$(wildcard $(cur_dir)*.c $(cur_dir)*.cpp $(cur_dir)*.S))
obj-y := $(obj-y:.c=.o)
obj-y := $(obj-y:.cpp=.o)
obj-y := $(obj-y:.S=.o)

subdir-ccflags-y += -Iutils/list  \
                    -Iplatform/hal/best1501p \

ccflags-y +=  \
    $(BLUETOOTH_ADAPTER_INCLUDES) \
    $(EPLAYER_INCLUDES) \
    $(ECOMM_INCLUDES) \
    -I$(BLE_GAF_CORE_DIR_PATH)/inc \
    -I$(BLE_AOB_APP_DIR_PATH)/inc \
    -Iservices/audio_bt \
    -Iplatform/drivers/codec \
    -Iservices/audioflinger \
    -Iservices/norflash_api \
    -Iservices/nv_section/factory_section \
    -Iservices/nv_section/log_section \
    -Iservices/overlay \
    -Iservices/osif \
    -Iservices/resources \
    -Iservices/audio_process \
    -Iservices/ux/common/ble_audio_core/inc \
    -Iutils/intersyshci \
    -Iapps/apptester \
    -Iapps/factory \
    -Iapps/ble_audio_test \
    -Iapps/voice_detector \
    -Iservices/ux/common/sound \
    -Ibthost/adapter/inc/adapter_service \
    -Iutils/crc32 \
    -Iutils/list \
    -Iplatform/drivers/bt \
    -Iplatform/drivers/ana \
    -Iapps/audioplayers/rbplay \
    -Itests/anc_usb \
    -Iapps/anc/inc \
    -Iapps/btspeaker \
    -Ithirdparty/userapi \
    -Iplatform/drivers/hx9031_ctrl \
    -Iplatform/drivers/tdk_icm_42679/example_gaf \
    -Iservices/communication \
    -Iutils/cqueue \
    -Iservices/ai_voice/audio \
    -Iservices/ai_voice/manager \
    -Iservices/ai_voice/transport \
    -Iservices/app_ai/inc \
    -Iservices/interconnection/red \
    -Iservices/ux/common/ibrt/ibrt_core/inc \
    -Iservices/interconnection/green \
    -Iservices/bridge/ \
    -Iservices/bt_app/a2dp_codecs/include \
    -Iservices/aob_app/inc \
    -Iservices/aob_bes_ux/inc \
    -Iservices/aob_custom_ux \
    -Ithirdparty/tile \
    -Iservices/ux/common/ibrt/ibrt_middleware/inc  \
    -Iservices/rpc/inc \
    -Iservices/vendor_lc_data \
    -Ithirdparty/tile/tile_target \
    -Iinclude/ \
    -Iapps/btwatch \
    -Iapps/sensorhub \
    -Iapps/dsp_m55 \
	-Irtos/rtx5     \
    -Iplatform/drivers/cp_accel \
    -Iservices/audio_manager \
    -Iservices/ux/product/earbud_tws/inc \
    -Iservices/ux/product/earbud_tws/inc  \
	-Ibthost/service/ble_app_new/inc   \
    -Iapps/key \
	-Iinclude/rtos/rtx \
	-Iapps/main \
	-Iplatform/drivers/bt \
	-Iplatform/hal \
	-Iservices/ai_voice/audio \
	-Iservices/ai_voice/transport \
	-Iservices/ai_voice/manager \
	-Iservices/ai_voice/protocol/ama/ama_stream \
	-Iservices/app_ai/inc \
	-Iservices/ibrt_middleware/inc \
	-Iservices/nv_setion/userdata_section \
	-Iservices/communication \
	-Iservices/nv_section/factory_section \
	$(BLUETOOTH_ADAPTER_INCLUDES) \
	-Iservices/multimedia/audio/codec/sbc/inc \
	-Iservices/multimedia/speech/inc \
	-Iservices/cp_accel \
	-Iservices/voicepath \
	-Iservices/voicepath/gsound/gsound_custom/inc \
	-Iservices/voicepath/gsound/gsound_target \
	-Iservices/voicepath/gsound/gsound_target_api_read_only \
	-Ithirdparty/userapi \
	-Iutils/cqueue \
	-Iservices/norflash_api \
	-Iservices/audio_manager \
	-Iservices/tota \
	-Iapps/lc_customer \
	-Iservices/tota_v2 \
	-Iapps/battery \
	-Ibthost/service/ble_app_new/inc \
	-Iutils/heap \
    -Iplatform/hal/best1501p \

