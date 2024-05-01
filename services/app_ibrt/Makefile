
cur_dir := $(dir $(lastword $(MAKEFILE_LIST)))

obj_s := $(patsubst $(cur_dir)%,%,$(wildcard $(cur_dir)src/*.s))
obj_c := $(patsubst $(cur_dir)%,%,$(wildcard $(cur_dir)src/*.c))

obj_cpp := $(patsubst $(cur_dir)%,%,$(wildcard $(cur_dir)src/*.cpp))

src_obj := $(obj_c:.c=.o) $(obj_s:.S=.o) $(obj_cpp:.cpp=.o)

IBRT_LIB_NAME := libtws_ibrt_ui

ifeq ($(USE_TRACE_ID), 1)
IBRT_LIB_NAME := $(IBRT_LIB_NAME)_log_compressed
endif

$(IBRT_LIB_NAME)-y := $(src_obj)

obj-y :=  $(IBRT_LIB_NAME).a

subdir-ccflags-y += \
	-Iservices/osif \
	-Iservices/fs/fat \
	-Iservices/fs/sd \
	-Iservices/fs/fat/ChaN \
	-Iservices/overlay \
	-Iservices/nvrecord \
	-Iservices/resources \
	-Iservices/multimedia/audio/process/resample/include \
	$(BT_IF_INCLUDES) \
	-Iservices/nv_section/factory_section \
	-Iplatform/drivers/uarthci \
	-Iplatform/drivers/ana \
	-Iplatform/drivers/bt \
	-Iutils/cqueue \
	-Iutils/heap \
	-Iservices/audioflinger \
	-Iutils/lockcqueue \
	-Iutils/intersyshci \
	-Iapps/key \
	-Iapps/main \
	-Iapps/common \
	-Iapps/audioplayers \
	-Iapps/audioplayers/a2dp_decoder \
	-Iapps/anc/inc \
	-Iapps/factory \
	-Iservices/ble_app \
	-Iservices/ble_app/app_gfps \
	-Iservices/interconnection/green \
	-Iutils/hwtimer_list \
	-Ithirdparty/audio_codec_lib/liblhdc-dec/inc \
	$(BT_PROFILES_INCLUDES) \
	-Iservices/ble_stack/common/api \
	-Iservices/ble_app/app_tws	\
	-Iservices/nv_section/log_section \
	-Iservices/ai_voice/manager \
	-Iapps/battery/ \
	-Iutils/crc16 \
	-Iutils/crc32 \
	-Iutils/list \
	-Iservices/multimedia/audio/codec/sbc/inc \
	-Iservices/multimedia/audio/codec/sbc/src/inc \
	-Iservices/bt_app/a2dp_codecs/include \
	-Iservices/ble_app/app_main \
	-Iservices/app_tws/inc \
	-Iservices/ota \
	-Iservices/tota \
	-Iservices/tota_v2 \
	-Iservices/voicepath \
	-Iservices/ble_stack/ble_ip \
	-Iservices/ble_stack/hl/api \
	-Iservices/ble_stack/app/api/ \
	-Iservices/ble_stack/common/api/ \
	-Iservices/ble_stack/hl/inc/ \
	-Iservices/ble_stack/ke/api \
	-Iservices/norflash_api \
	-Iapps/anc/inc \
	-Iservices/ble_app/app_gfps \
	-Iapps/audioplayers

ifeq ($(QIOT_DATAPATH_ENABLE),1)
subdir-ccflags-y += \
	-Ithirdparty/qiot \
	-Ithirdparty/qiot/qiot_explorer_ble/internal_inc
endif

ifeq ($(IBRT),1)
subdir-ccflags-y += \
	-Iservices/tws_ibrt/inc \
	-Iservices/app_ibrt/inc \
	-Iservices/bt_app
endif

ifeq ($(USE_LOWLATENCY_LIB),1)
subdir-ccflags-y += -Ithirdparty/lowlatency_lib/inc
endif

ifeq ($(ANC_APP),1)
CFLAGS_app_ibrt_ui_test.o += -DANC_APP
endif
ifeq ($(TWS_SWITCH_ACCORDING_NOISE),1)
CFLAGS_app_ibrt_customif_ui.o += -DTWS_SWITCH_ACCORDING_NOISE
endif

ifeq ($(__STDF__),1)
subdir-ccflags-y += \
	-Istdf/stdf_sdk
endif

