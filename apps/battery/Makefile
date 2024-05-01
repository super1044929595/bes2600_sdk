cur_dir := $(dir $(lastword $(MAKEFILE_LIST)))

obj-y := $(patsubst $(cur_dir)%,%,$(wildcard $(cur_dir)*.c $(cur_dir)*.cpp $(cur_dir)*.S))
obj-y := $(obj-y:.c=.o)
obj-y := $(obj-y:.cpp=.o)
obj-y := $(obj-y:.S=.o)

subdir-ccflags-y += -Iplatform/drivers/ana \
                    $(BT_IF_INCLUDES) \
                    -Iservices/ble_app/app_main \
                    -Iservices/app_ibrt/inc \
                    -Iservices/tws_ibrt/inc

ifeq ($(VOICE_PROMPT),1)
CFLAGS_app_battery.o += -DMEDIA_PLAYER_SUPPORT
endif

ifeq ($(__STDF__),1)
subdir-ccflags-y += -Istdf/stdf_sdk
endif
