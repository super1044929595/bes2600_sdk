
cur_dir := $(dir $(lastword $(MAKEFILE_LIST)))

obj_s := $(patsubst $(cur_dir)%,%,$(wildcard $(cur_dir)src/*.S))
obj_c := $(patsubst $(cur_dir)%,%,$(wildcard $(cur_dir)src/*.c))
obj_cpp := $(patsubst $(cur_dir)%,%,$(wildcard $(cur_dir)src/*.cpp))

ifeq ($(ANC_ASSIST_ENABLED),1)
obj_c += $(patsubst $(cur_dir)%,%,$(wildcard $(cur_dir)src/assist/*.c))
endif

ifeq ($(ANC_APP),1)
CFLAGS_app_anc.o += -DANC_APP
endif

ifeq ($(ANC_FF_ENABLED),1)
CFLAGS_app_anc.o += -DANC_FF_ENABLED
CFLAGS_app_anc_fade.o += -DANC_FF_ENABLED
endif

ifeq ($(ANC_FB_ENABLED),1)
CFLAGS_app_anc.o += -DANC_FB_ENABLED
CFLAGS_app_anc_fade.o += -DANC_FB_ENABLED
endif

ifeq ($(PSAP_APP),1)
CFLAGS_app_anc.o += -DPSAP_APP
endif

ifeq ($(AUDIO_RESAMPLE),1)
CFLAGS_app_anc.o += -D__AUDIO_RESAMPLE__
endif
ifeq ($(SW_PLAYBACK_RESAMPLE),1)
CFLAGS_app_anc.o += -DSW_PLAYBACK_RESAMPLE
endif
ifeq ($(SW_CAPTURE_RESAMPLE),1)
CFLAGS_app_anc.o += -DSW_CAPTURE_RESAMPLE
endif
ifeq ($(AUDIO_SECTION_SUPPT),1)
CFLAGS_app_anc.o += -D__AUDIO_SECTION_SUPPT__
endif

ifeq ($(ANC_ASSIST_USE_INT_CODEC),1)
CFLAGS_app_anc_assist.o += -DANC_ASSIST_USE_INT_CODEC
endif

ifeq ($(ANC_ASSIST_PILOT_TONE_ALWAYS_ON),1)
CFLAGS_app_anc_assist.o += -DANC_ASSIST_PILOT_TONE_ALWAYS_ON
endif

ifeq ($(ANC_CALIB_WITH_GAIN),1)
CFLAGS_app_anc.o += -DANC_CALIB_WITH_GAIN
endif

ifneq ($(filter 1, $(PC_CMD_UART) $(TOTA_EQ_TUNING) $(USB_EQ_TUNING)),)
CFLAGS_app_anc.o += -DAUDIO_EQ_TUNING
endif

obj-y := $(obj_c:.c=.o) $(obj_s:.S=.o) $(obj_cpp:.cpp=.o)


subdir-ccflags-y += \
	-Iservices/overlay \
	-Iservices/nvrecord \
	-Iservices/resources \
	-Iservices/ibrt_middleware/inc \
	-Iservices/multimedia/audio/process/resample/include \
	-Iservices/multimedia/audio/process/integer_resampling/include \
    -Iservices/multimedia/audio/process/anc/include \
	-Iservices/multimedia/audio/process/psap/include \
    -Iservices/multimedia/speech/inc \
	-Iplatform/drivers/uarthci \
	-Iplatform/drivers/ana \
	-Iplatform/drivers/bt \
	-Iutils/cqueue \
	-Iservices/audioflinger \
	-Iutils/lockcqueue \
	-Iutils/intersyshci \
	-Iinclude/cmsis_dsp \
	-Iapps/anc/inc \
	-Iapps/key \
	-Iapps/main \
	-Iapps/common \
	-Iapps/audioplayers \
	-Iapps/factory \
	-Iapps/voice_detector \
	-Iservices/ble_app \
	-Iservices/bt_app \
	-Iservices/bt_app/a2dp_codecs/include \
	-Iservices/anc/inc \
	-Iservices/multimedia/audio/codec/sbc/src/inc \
	-Iservices/multimedia/audio/codec/sbc/inc \
    -Iservices/nv_section/aud_section \
    -Iservices/nv_section/include   \
    -Iutils/hwtimer_list \
	-Iservices/tota \
	$(BT_IF_INCLUDES) \
	-Iservices/ble_stack/common/api \
	-Iservices/ble_stack/ble_ip \
	-Iinclude/cmsis_dsp \
	-Iservices/audio_dump/include \
	-Iservices/app_ibrt/inc \
	-Iservices/tws_ibrt/inc

ifeq ($(ENABLE_CALCU_CPU_FREQ_LOG),1)
CFLAGS_app_anc_assist.o += -DENABLE_CALCU_CPU_FREQ_LOG
endif

ifeq ($(VOICE_ASSIST_WD_ENABLED),1)
CFLAGS_app_anc_assist.o += -DVOICE_ASSIST_WD_ENABLED
endif

ifeq ($(VOICE_ASSIST_NOISE_ADAPT_ANC_ENABLED),1)
CFLAGS_app_anc_assist.o += -DVOICE_ASSIST_NOISE_ADAPT_ANC_ENABLED
endif
