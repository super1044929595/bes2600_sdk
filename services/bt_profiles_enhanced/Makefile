
cur_dir := $(dir $(lastword $(MAKEFILE_LIST)))

obj_s := $(patsubst $(cur_dir)%,%,$(wildcard $(cur_dir)protocols/*/*.s))
obj_c := $(patsubst $(cur_dir)%,%,$(wildcard $(cur_dir)protocols/*/*.c))
obj_cpp := $(patsubst $(cur_dir)%,%,$(wildcard $(cur_dir)protocols/*/*.cpp))

ifeq ($(AUTO_TEST),1)
obj_s += $(patsubst $(cur_dir)%,%,$(wildcard $(cur_dir)protocols/test/*/*.s))
obj_c += $(patsubst $(cur_dir)%,%,$(wildcard $(cur_dir)protocols/test/*/*.c))
obj_cpp += $(patsubst $(cur_dir)%,%,$(wildcard $(cur_dir)protocols/test/*/*.cpp))
endif

obj_s += $(patsubst $(cur_dir)%,%,$(wildcard $(cur_dir)common/*.s))
obj_c += $(patsubst $(cur_dir)%,%,$(wildcard $(cur_dir)common/*.c))
obj_cpp += $(patsubst $(cur_dir)%,%,$(wildcard $(cur_dir)common/*.cpp))

obj_s += $(patsubst $(cur_dir)%,%,$(wildcard $(cur_dir)besport/*.s))
obj_c += $(patsubst $(cur_dir)%,%,$(wildcard $(cur_dir)besport/*.c))
obj_cpp += $(patsubst $(cur_dir)%,%,$(wildcard $(cur_dir)besport/*.cpp))

obj_s += $(patsubst $(cur_dir)%,%,$(wildcard $(cur_dir)hci/*.s))
obj_c += $(patsubst $(cur_dir)%,%,$(wildcard $(cur_dir)hci/*.c))
obj_cpp += $(patsubst $(cur_dir)%,%,$(wildcard $(cur_dir)hci/*.cpp))

obj_c += $(patsubst $(cur_dir)%,%,$(wildcard $(cur_dir)main/bt_schedule.c))

obj_c += $(patsubst $(cur_dir)%,%,$(wildcard $(cur_dir)applications/hshf/hshf_app.c))
obj_c += $(patsubst $(cur_dir)%,%,$(wildcard $(cur_dir)applications/hshf/bt_schedule.c))

BTPROFILE_LIB_NAME := $(CHIP)_libbt_profiles

ifeq ($(HFP_1_6_ENABLE), 1)
BTPROFILE_LIB_NAME := $(BTPROFILE_LIB_NAME)_sbc_enc
endif

ifeq ($(ROM_UTILS_ON),1)
BTPROFILE_LIB_NAME := $(BTPROFILE_LIB_NAME)_romaac
endif

ifeq ($(BLE),1)
BTPROFILE_LIB_NAME := $(BTPROFILE_LIB_NAME)_ble
endif

ifeq ($(BT_RF_PREFER), 2M)
BTPROFILE_LIB_NAME := $(BTPROFILE_LIB_NAME)_2m
endif

ifeq ($(BT_RF_PREFER), 3M)
BTPROFILE_LIB_NAME := $(BTPROFILE_LIB_NAME)_3m
endif

ifeq ($(GATT_OVER_BR_EDR),1)
BTPROFILE_LIB_NAME := $(BTPROFILE_LIB_NAME)_gatt_over_br_edr
endif

ifeq ($(USE_TRACE_ID), 1)
BTPROFILE_LIB_NAME := $(BTPROFILE_LIB_NAME)_log_compressed
endif

ifeq ($(BES_OTA_MODE),1)
BTPROFILE_LIB_NAME := $(BTPROFILE_LIB_NAME)_ota
endif

$(BTPROFILE_LIB_NAME)-y := $(obj_c:.c=.o) $(obj_s:.S=.o) $(obj_cpp:.cpp=.o)

-include $(cur_dir)lib_proc/Makefile

obj-y += $(BTPROFILE_LIB_NAME).a

subdir-ccflags-y += \
    -O2 \
    -Iservices/osif/ \
    -Iservices/auto_test \
    $(BT_IF_INCLUDES) \
    $(BT_PROFILES_INCLUDES) \
    -Iservices/bridge/ \
    -Iservices/ble_stack/ke/api/ \
    -Iservices/ble_stack/ble_ip/ \
    -Iservices/ble_stack/common/api/ \
    -Iservices/ble_stack/hl/api/ \
    -Iplatform/drivers/uarthci \
    -Iplatform/drivers/ana \
    -Iplatform/drivers/bt \
    -Iutils/cqueue \
    -Iutils/heap \
    -Iutils/intersyshci \
    -Iservices/bt_app \
    -Iservices/audioflinger \
    -Iservices/nvrecord \
    -Iservices/resources \
    -Iservices/tws_ibrt/inc \
    -Iutils/lockcqueue \
    -Iapps/key \
	-Iservices/multimedia/audio/codec/sbc/inc \
	-Iservices/multimedia/audio/codec/sbc/src/inc \
    -Iapps/audioplayers

