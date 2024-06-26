cur_dir := $(dir $(lastword $(MAKEFILE_LIST)))

obj-y := $(patsubst $(cur_dir)%,%,$(wildcard $(cur_dir)*.c $(cur_dir)*.cpp $(cur_dir)*.S))

obj-y += ../../tests/anc_usb/usb_audio_app.c
obj-y += ../../tests/anc_usb/safe_queue.c
obj-y += ../../tests/anc_usb/memutils.c
obj-y += ../../tests/anc_usb/speech_process.c

ifeq ($(ANC_APP),1)
obj-y += ../../tests/anc_usb/anc_usb_app.c
endif

obj-y := $(obj-y:.c=.o)
obj-y := $(obj-y:.cpp=.o)
obj-y := $(obj-y:.S=.o)


ccflags-y +=  \
	-Iutils/boot_struct \
	-Iutils/crc32 \
	-Iutils/hexdump \
	-Iutils/hwtimer_list \
	-Iplatform/drivers/usb/usb_dev/inc	\
	-Iplatform/drivers/ana \
	-Iservices/multimedia/audio/process/adp/include \
	-Iservices/multimedia/audio/process/anc/include \
	-Iservices/multimedia/audio/process/filters/include \
	-Iservices/multimedia/audio/process/resample/include \
	-Iservices/multimedia/speech/inc \
	-Iservices/audio_process \
	-Iservices/nv_section/aud_section \
	-Iservices/nv_section/include \
	-Itests/anc_usb

CFLAGS_usbaudio_entry.o += -DUSB_AUDIO_APP
CFLAGS_usb_audio_app.o += -DAUDIO_OUTPUT_VOLUME_DEFAULT=$(AUDIO_OUTPUT_VOLUME_DEFAULT)
CFLAGS_adda_loop_app.o += -DAUDIO_OUTPUT_VOLUME_DEFAULT=$(AUDIO_OUTPUT_VOLUME_DEFAULT)

ANC_USB_CFG_FLAGS :=

# ANC option
ifeq ($(ANC_APP),1)
ANC_USB_CFG_FLAGS += -DANC_APP
endif

# USB audio option
ifeq ($(USB_AUDIO_APP),1)
ANC_USB_CFG_FLAGS += -DUSB_AUDIO_APP
endif

ifeq ($(USB_HIGH_SPEED),1)
ANC_USB_CFG_FLAGS += -DUSB_HIGH_SPEED
endif

ifeq ($(AUDIO_RESAMPLE),1)
ANC_USB_CFG_FLAGS += -D__AUDIO_RESAMPLE__
endif

ifeq ($(ADC_CH_SEP_BUFF),1)
ANC_USB_CFG_FLAGS += -DADC_CH_SEP_BUFF
endif

# USB audio configuration
ifeq ($(USB_AUDIO_DYN_CFG),1)

ANC_USB_CFG_FLAGS += -DUSB_AUDIO_DYN_CFG

ifneq ($(AUDIO_RESAMPLE),1)
SW_CAPTURE_RESAMPLE ?= 1
endif

endif

ifeq ($(USB_AUDIO_384K),1)
ANC_USB_CFG_FLAGS += -DUSB_AUDIO_384K
endif
ifeq ($(USB_AUDIO_352_8K),1)
ANC_USB_CFG_FLAGS += -DUSB_AUDIO_352_8K
endif
ifeq ($(USB_AUDIO_192K),1)
ANC_USB_CFG_FLAGS += -DUSB_AUDIO_192K
endif
ifeq ($(USB_AUDIO_176_4K),1)
ANC_USB_CFG_FLAGS += -DUSB_AUDIO_176_4K
endif
ifeq ($(USB_AUDIO_96K),1)
ANC_USB_CFG_FLAGS += -DUSB_AUDIO_96K
endif
ifeq ($(USB_AUDIO_48K),1)
ANC_USB_CFG_FLAGS += -DUSB_AUDIO_48K
endif
ifeq ($(USB_AUDIO_44_1K),1)
ANC_USB_CFG_FLAGS += -DUSB_AUDIO_44_1K
endif
ifeq ($(USB_AUDIO_16K),1)
ANC_USB_CFG_FLAGS += -DUSB_AUDIO_16K
endif

ifeq ($(USB_AUDIO_32BIT),1)
ANC_USB_CFG_FLAGS += -DUSB_AUDIO_32BIT
endif
ifeq ($(USB_AUDIO_24BIT),1)
ANC_USB_CFG_FLAGS += -DUSB_AUDIO_24BIT
endif
ifeq ($(USB_AUDIO_16BIT),1)
ANC_USB_CFG_FLAGS += -DUSB_AUDIO_16BIT
endif

ifeq ($(USB_AUDIO_SEND_32BIT),1)
ANC_USB_CFG_FLAGS += -DUSB_AUDIO_SEND_32BIT
endif
ifeq ($(USB_AUDIO_SEND_24BIT),1)
ANC_USB_CFG_FLAGS += -DUSB_AUDIO_SEND_24BIT
endif
ifeq ($(USB_AUDIO_SEND_16BIT),1)
ANC_USB_CFG_FLAGS += -DUSB_AUDIO_SEND_16BIT
endif

ifneq ($(USB_AUDIO_SEND_CHAN),)
ANC_USB_CFG_FLAGS += -DUSB_AUDIO_SEND_CHAN=$(USB_AUDIO_SEND_CHAN)
endif

ifeq ($(USB_AUDIO_UAC2),1)
ANC_USB_CFG_FLAGS += -DUSB_AUDIO_UAC2
endif

ifeq ($(AUDIO_PLAYBACK_24BIT),1)
ANC_USB_CFG_FLAGS += -DAUDIO_PLAYBACK_24BIT
endif

# DSD configuration
ifeq ($(HW_FIR_DSD_PROCESS),1)
ANC_USB_CFG_FLAGS += -D__HW_FIR_DSD_PROCESS__
endif

# EQ configuration
ifeq ($(HW_FIR_EQ_PROCESS),1)
ANC_USB_CFG_FLAGS += -D__HW_FIR_EQ_PROCESS__
endif

ifeq ($(HW_IIR_EQ_PROCESS),1)
ANC_USB_CFG_FLAGS += -D__HW_IIR_EQ_PROCESS__
endif

ifeq ($(SW_IIR_EQ_PROCESS),1)
ANC_USB_CFG_FLAGS += -D__SW_IIR_EQ_PROCESS__
endif

ifeq ($(HW_DAC_IIR_EQ_PROCESS),1)
ANC_USB_CFG_FLAGS += -D__HW_DAC_IIR_EQ_PROCESS__
endif

ifeq ($(SW_CAPTURE_RESAMPLE),1)
ANC_USB_CFG_FLAGS += -DSW_CAPTURE_RESAMPLE
endif

CFLAGS_usbaudio_entry.o += $(ANC_USB_CFG_FLAGS)
CFLAGS_usb_audio_app.o += $(ANC_USB_CFG_FLAGS)
CFLAGS_anc_usb_app.o += $(ANC_USB_CFG_FLAGS)

ifeq ($(ANC_KEY_DOUBLE_CLICK_ON_OFF),1)
CFLAGS_anc_usb_app.o += -DANC_KEY_DOUBLE_CLICK_ON_OFF
endif

ifeq ($(ANC_FF_ENABLED),1)
CFLAGS_anc_usb_app.o += -DANC_FF_ENABLED
endif

ifeq ($(ANC_FB_ENABLED),1)
CFLAGS_anc_usb_app.o += -DANC_FB_ENABLED
endif

ifeq ($(AUDIO_SECTION_SUPPT),1)
CFLAGS_anc_usb_app.o += -D__AUDIO_SECTION_SUPPT__
endif

ifeq ($(ANC_INIT_OFF),1)
CFLAGS_anc_usb_app.o += -DANC_INIT_OFF
endif

ifeq ($(PC_CMD_UART),1)
CFLAGS_usbaudio_entry.o += -D__PC_CMD_UART__
endif

ifeq ($(DELAY_STREAM_OPEN),1)
CFLAGS_usb_audio_app.o += -DDELAY_STREAM_OPEN
endif

ifeq ($(NOISE_GATING),1)
CFLAGS_usb_audio_app.o += -DNOISE_GATING
endif

ifeq ($(NOISE_REDUCTION),1)
CFLAGS_usb_audio_app.o += -DNOISE_REDUCTION
endif

ifeq ($(ANC_L_R_MISALIGN_WORKAROUND),1)
CFLAGS_usb_audio_app.o += -DANC_L_R_MISALIGN_WORKAROUND
endif

ifeq ($(ANDROID_ACCESSORY_SPEC),1)
CFLAGS_usb_audio_app.o += -DANDROID_ACCESSORY_SPEC
ifeq ($(ANDROID_VOICE_CMD_KEY),1)
CFLAGS_usb_audio_app.o += -DANDROID_VOICE_CMD_KEY
endif
endif

ifeq ($(DUAL_AUX_MIC_MORE_FILTER),1)
CFLAGS_usb_audio_app.o += -DDUAL_AUX_MIC_MORE_FILTER
endif

ifeq ($(FREQ_RESP_EQ),1)
CFLAGS_usb_audio_app.o += -DFREQ_RESP_EQ
endif

ifeq ($(KEEP_SAME_LATENCY),1)
CFLAGS_usb_audio_app.o += -DKEEP_SAME_LATENCY
endif

ifeq ($(USB_AUDIO_PWRKEY_TEST),1)
CFLAGS_usb_audio_app.o += -DUSB_AUDIO_PWRKEY_TEST
endif

ifeq ($(AUDIO_RESAMPLE),1)
# If neither best1000 nor best2000
ifeq ($(filter best1000 best2000,$(CHIP)),)
PLL_TUNE_SAMPLE_RATE ?= 1
endif
ifeq ($(PLL_TUNE_SAMPLE_RATE),1)
CFLAGS_usb_audio_app.o += -DPLL_TUNE_SAMPLE_RATE
endif
endif

ifeq ($(USB_AUDIO_SPEECH),1)
CFLAGS_usbaudio_entry.o += -DUSB_AUDIO_SPEECH
CFLAGS_usb_audio_app.o += -DUSB_AUDIO_SPEECH
endif

ifeq ($(VENDOR_MSG_SUPPT), 1)
CFLAGS_usbaudio_entry.o += -D_VENDOR_MSG_SUPPT_
CFLAGS_usb_audio_app.o += -D_VENDOR_MSG_SUPPT_
CFLAGS_usb_vendor_msg.o += -D_VENDOR_MSG_SUPPT_
endif
