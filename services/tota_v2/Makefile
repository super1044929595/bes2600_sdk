cur_dir := $(dir $(lastword $(MAKEFILE_LIST)))

obj_s := $(patsubst $(cur_dir)%,%,$(wildcard $(cur_dir)*.s))
obj_c := $(patsubst $(cur_dir)%,%,$(wildcard $(cur_dir)*.c))
obj_cpp := $(patsubst $(cur_dir)%,%,$(wildcard $(cur_dir)*.cpp))

src_obj := $(obj_c:.c=.o) $(obj_s:.S=.o) $(obj_cpp:.cpp=.o)

TOTA_LIB_NAME := libtota

$(TOTA_LIB_NAME)-y := $(src_obj)

obj-y += $(TOTA_LIB_NAME).a

export BUDSODIN2_TOTA ?= 0
ifeq ($(BUDSODIN2_TOTA),1)
subdir-ccflags-y += -DBUDSODIN2_TOTA
endif

ifeq ($(TOTA_EQ_TUNING), 1)
CFLAGS_app_tota_audio_EQ.o += -DTOTA_EQ_TUNING
endif

subdir-ccflags-y += -Iplatform/drivers/ana \
					-Iplatform/hal \
					-Iservices/bluedroid/adapter/src \
					-Iapps/key \
					-Iservices/ibrt_middleware/inc \
					-Iservices/audio_eq \
					-Iservices/fs/fat \
					-Iservices/fs/sd \
					-Iservices/fs/fat/ChaN \
					$(BT_IF_INCLUDES) \
					-Iservices/overlay \
					-Iservices/nvrecord \
					-Iservices/resources \
					-Iservices/multimedia/audio/codec/sbc/inc \
					-Iservices/bt_app/a2dp_codecs/include \
					-Iplatform/drivers/uarthci \
					-Iplatform/drivers/ana \
					-Iplatform/drivers/bt \
					-Iutils/cqueue \
					-Iutils/heap \
					-Iutils/retention_ram \
					-Iservices/audioflinger \
					-Iutils/lockcqueue \
					-Iutils/intersyshci \
                    -Iutils/boot_struct \
					-Iutils/encrypt \
					-Iutils/sha256 \
					-Iapps/main \
					-Iapps/common \
					-Iapps/audioplayers \
					-Iservices/app_ai/inc \
					-Iapps/factory \
					-Iservices/anc_spp_tool \
					-Iservices/ble_app \
					-Iservices/ble_app/app_tota \
					-Iservices/ble_stack/ble_ip \
					-Iservices/ble_stack/hl/api \
					-Iservices/ble_stack/app/api/  \
					-Iservices/ble_stack/common/api/ \
					-Iservices/ble_stack/hl/inc/ \
					-Iservices/ble_stack/ke/api \
					-Iservices/bridge/ \
					-Iservices/ble_stack/hl/src/gap/ \
					-Iservices/ble_stack/hl/src/gap/gapc/ \
					-Iservices/ble_stack/hl/src/gap/gapm/ \
					-Iservices/ble_stack/hl/src/gap/smpc/ \
					-Iservices/ble_stack/ke/src/ \
					-Iservices/ble_stack/hl/src/gatt/attc/ \
					-Iservices/ble_stack/hl/src/gatt/attm/ \
					-Iservices/ble_stack/hl/src/gatt/atts/ \
					-Iservices/ble_stack/hl/src/gatt/gattc/ \
					-Iservices/ble_stack/hl/src/gatt/gattm/ \
					-Iservices/ble_stack/hl/src/gatt/ \
					-Iservices/ble_stack/hl/src/l2c/l2cc/ \
					-Iservices/ble_stack/hl/src/l2c/l2cm/ \
					-Iservices/ble_stack/hci/api/ \
					-Iservices/ble_stack/hci/src/ \
					-Iservices/ble_stack/app/src/  \
					-Iservices/ble_profiles/htp/htpt/api/  \
					-Iservices/ble_profiles/htp/htpt/src/  \
					-Iservices/ble_profiles/htp/	\
					-Iservices/ble_profiles/datapath/datapathps/api/  \
					-Iservices/ble_app/app_main/ \
					-Iservices/ble_app/app_htp/	\
					-Iservices/ble_app/app_datapath/	\
					-Iservices/bt_app \
					-Iservices/ble_app/app_vob \
					-Iservices/multimedia/speech/codec/codecs/adpcm \
					-Iservices/ble_app/app_smartvoice \
					-Iservices/ble_profiles/smartvoice \
					-Iservices/anc_test \
                    -Iservices/tws/inc \
                    -Iservices/tws_ibrt/inc \
					-Iutils/crc32 \
					-Iservices/app_ibrt/inc \
					-Ithirdparty/userapi \
					-Iapps/battery \
					-Iservices/nv_section/factory_section \
					-Iservices/ \
					-Iservices/ibrt_middleware/inc/ \
					-Iservices/audio_process \
					-Iservices/ota/ \
					-Iservices/app_tws/inc \
					-Iapps/anc/inc \
					-Iapps/voice_assist/inc 

