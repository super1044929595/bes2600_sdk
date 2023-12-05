
cur_dir := $(dir $(lastword $(MAKEFILE_LIST)))

obj_s := $(patsubst $(cur_dir)%,%,$(wildcard $(cur_dir)src/*.s))
obj_c := $(patsubst $(cur_dir)%,%,$(wildcard $(cur_dir)src/*.c))

obj_cpp := $(patsubst $(cur_dir)%,%,$(wildcard $(cur_dir)src/*.cpp))

src_obj := $(obj_c:.c=.o) $(obj_s:.S=.o) $(obj_cpp:.cpp=.o)

IBRT_LIB_NAME := libtws_ibrt

ifeq ($(ENHANCED_STACK),1)
IBRT_LIB_NAME := $(IBRT_LIB_NAME)_enhanced_stack
endif

ifeq ($(A2DP_AAC_ON), 1)
IBRT_LIB_NAME := $(IBRT_LIB_NAME)_aac
endif

ifeq ($(BLE), 1)
IBRT_LIB_NAME := $(IBRT_LIB_NAME)_ble
endif

ifeq ($(ANC_APP),1)
IBRT_LIB_NAME := $(IBRT_LIB_NAME)_ANC
endif

ifeq ($(GATT_OVER_BR_EDR),1)
IBRT_LIB_NAME := $(IBRT_LIB_NAME)_gatt_over_br_edr
endif

ifeq ($(USE_TRACE_ID), 1)
IBRT_LIB_NAME := $(IBRT_LIB_NAME)_log_compressed
endif

$(IBRT_LIB_NAME)-y := $(src_obj)

obj-y :=  $(IBRT_LIB_NAME).a

subdir-ccflags-y += \
	-Iservices/fs/fat \
	-Iservices/fs/sd \
	-Iservices/fs/fat/ChaN \
	-Iservices/overlay \
	-Iservices/nvrecord \
	-Iservices/resources \
	-Iservices/multimedia/audio/process/resample/include \
	$(BT_IF_INCLUDES) \
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
	-Iapps/battery \
	-Iapps/common \
	-Iapps/audioplayers \
	-Iapps/audioplayers/a2dp_decoder \
	-Iapps/factory \
	-Iservices/ble_stack/ble_ip \
	-Iservices/ble_stack/common/api \
	-Iservices/ble_stack/hl/api \
	-Iservices/ble_stack/hl/inc \
	-Iservices/ble_stack/ke/api \
	-Iservices/ble_app \
	-Iservices/ble_app/app_datapath \
	-Iservices/ble_app/app_tws	\
	-Iservices/ble_app/app_main \
	$(BT_PROFILES_INCLUDES) \
	-Iservices/nv_section/factory_section \
	-Iservices/nv_section/log_section \
	-Iservices/app_ibrt/inc \
	-Iservices/bt_app/a2dp_codecs/include \
	-Ithirdparty/audio_codec_lib/liblhdc-dec/inc \
	-Iservices/multimedia/audio/codec/sbc/inc \
	-Iservices/multimedia/audio/codec/sbc/src/inc \
	-Iutils/hwtimer_list \
	-Iutils/list \
    -Iservices/app_tws/inc \
    -Iapps/anc/inc \
	-Iservices/tws_ibrt/inc \
	-Iservices/app_ibrt/inc \
	-Iservices/tota_v2 \
    -Iservices/ble_app/app_gfps
	
ifeq ($(IBRT),1)
subdir-ccflags-y += \
	-Iservices/tws_ibrt/inc \
	-Iservices/bt_app
endif

BT_ERROR_HANDLE_SUPPORT ?= 0
ifeq ($(BT_ERROR_HANDLE_SUPPORT),1)
subdir-ccflags-y += -DBT_ERROR_HANDLE_SUPPORT
endif
