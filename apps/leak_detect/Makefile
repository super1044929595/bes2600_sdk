cur_dir := $(dir $(lastword $(MAKEFILE_LIST)))

obj-y := $(patsubst $(cur_dir)%,%,$(wildcard $(cur_dir)*.c $(cur_dir)*.cpp $(cur_dir)*.S))
obj-y := $(obj-y:.c=.o)
obj-y := $(obj-y:.cpp=.o)
obj-y := $(obj-y:.S=.o)

ccflags-y += \
	-Iservices/fs/fat \
	-Iservices/fs/sd \
	-Iservices/fs/fat/ChaN \
	-Iservices/overlay \
	-Iservices/nvrecord \
	-Iservices/resources \
	-Iservices/multimedia/audio/process/resample/include \
    -Iservices/multimedia/audio/process/anc/include \
    -Iservices/multimedia/speech/inc \
    -Iservices/audio_dump/include \
	-Iplatform/drivers/uarthci \
	-Iplatform/drivers/ana \
	-Iplatform/drivers/bt \
	-Iutils/cqueue \
	-Iservices/audioflinger \
	-Iutils/lockcqueue \
	-Iutils/intersyshci \
	-Iapps/anc/inc \
	-Iapps/key \
	-Iapps/main \
	-Iapps/common \
	-Iapps/audioplayers \
	-Iapps/factory \
	-Iservices/ble_app \
	-Iservices/bt_app \
	-Iservices/anc/inc \
	-Iinclude/cmsis_dsp \
	-Iservices/multimedia/audio/codec/sbc/inc \
    -Iservices/multimedia/speech/src/process/leak_detect \
    -Iservices/nv_section/aud_section \
    -Iservices/nv_section/include   \
    -Iutils/hwtimer_list \
	-Iservices/tws_ibrt/inc \
	-Iservices/tota \
	$(BT_IF_INCLUDES) \
	-Iservices/app_ibrt/inc \
	-Iservices/ble_stack/common/api \
	-Iservices/ble_stack/ble_ip
