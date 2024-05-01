
cur_dir := $(dir $(lastword $(MAKEFILE_LIST)))

obj_s := $(patsubst $(cur_dir)%,%,$(wildcard $(cur_dir)*.s))
obj_c := $(patsubst $(cur_dir)%,%,$(wildcard $(cur_dir)*.c))

obj_cpp := $(patsubst $(cur_dir)%,%,$(wildcard $(cur_dir)*.cpp))
ifeq ($(VOICE_DATAPATH_TYPE),gsound)
obj_cpp += $(patsubst $(cur_dir)%,%,$(wildcard $(cur_dir)/app_voice/app_$(VOICE_DATAPATH_TYPE)/*.cpp))
endif

obj-y := $(obj_c:.c=.o) $(obj_s:.S=.o) $(obj_cpp:.cpp=.o)

obj-y += a2dp_codecs/

ifeq ($(A2DP_AAC_ON),1)
ifeq ($(FDKAAC_VERSION),2)
ifeq ($(AAC_REDUCE_SIZE),1)
AAC_INCLUDES =  \
	-Iservices/multimedia/audio/codec/fdkaac_codec_2_0_1_reduce/libAACdec/include \
	-Iservices/multimedia/audio/codec/fdkaac_codec_2_0_1_reduce/libAACenc/include \
	-Iservices/multimedia/audio/codec/fdkaac_codec_2_0_1_reduce/libFDK/include \
	-Iservices/multimedia/audio/codec/fdkaac_codec_2_0_1_reduce/libMpegTPDec/include \
	-Iservices/multimedia/audio/codec/fdkaac_codec_2_0_1_reduce/libMpegTPEnc/include \
	-Iservices/multimedia/audio/codec/fdkaac_codec_2_0_1_reduce/libPCMutils/include \
	-Iservices/multimedia/audio/codec/fdkaac_codec_2_0_1_reduce/libSBRdec/include \
	-Iservices/multimedia/audio/codec/fdkaac_codec_2_0_1_reduce/libSBRenc/include \
	-Iservices/multimedia/audio/codec/fdkaac_codec_2_0_1_reduce/libSYS/include
else
AAC_INCLUDES =  \
	-Iservices/multimedia/audio/codec/fdkaac_codec_2_0_1/libAACdec/include \
	-Iservices/multimedia/audio/codec/fdkaac_codec_2_0_1/libAACenc/include \
	-Iservices/multimedia/audio/codec/fdkaac_codec_2_0_1/libFDK/include \
	-Iservices/multimedia/audio/codec/fdkaac_codec_2_0_1/libMpegTPDec/include \
	-Iservices/multimedia/audio/codec/fdkaac_codec_2_0_1/libMpegTPEnc/include \
	-Iservices/multimedia/audio/codec/fdkaac_codec_2_0_1/libPCMutils/include \
	-Iservices/multimedia/audio/codec/fdkaac_codec_2_0_1/libSBRdec/include \
	-Iservices/multimedia/audio/codec/fdkaac_codec_2_0_1/libSBRenc/include \
	-Iservices/multimedia/audio/codec/fdkaac_codec_2_0_1/libSYS/include
endif
else
AAC_INCLUDES =  \
	-Iservices/multimedia/audio/codec/fdkaac_codec/libAACdec/include \
	-Iservices/multimedia/audio/codec/fdkaac_codec/libAACenc/include \
	-Iservices/multimedia/audio/codec/fdkaac_codec/libFDK/include \
	-Iservices/multimedia/audio/codec/fdkaac_codec/libMpegTPDec/include \
	-Iservices/multimedia/audio/codec/fdkaac_codec/libMpegTPEnc/include \
	-Iservices/multimedia/audio/codec/fdkaac_codec/libPCMutils/include \
	-Iservices/multimedia/audio/codec/fdkaac_codec/libSBRdec/include \
	-Iservices/multimedia/audio/codec/fdkaac_codec/libSBRenc/include \
	-Iservices/multimedia/audio/codec/fdkaac_codec/libSYS/include
endif
else
AAC_INCLUDES =
endif

ccflags-y += \
	$(AAC_INCLUDES) \
	-Iservices/osif \
	-Iservices/audio_process \
	-Iservices/hw_dsp/inc \
	-Iservices/fs/fat \
	-Iservices/fs/sd \
	-Iservices/fs/fat/ChaN \
	$(BT_IF_INCLUDES) \
    $(BT_PROFILES_INCLUDES) \
	-Iservices/multimedia/speech/inc \
	-Iservices/bone_sensor/lis25ba \
	-Iservices/overlay \
	-Iservices/nvrecord \
	-Iservices/resources \
	-Iservices/multimedia/rbcodec \
	-Iservices/multimedia/audio/process/resample/include \
	-Iservices/multimedia/audio/process/filters/include \
	-Iservices/multimedia/audio/process/drc/include \
	-Iservices/multimedia/audio/process/anc/include\
	-Iservices/multimedia/audio/process/sidetone/include\
	-Iservices/multimedia/audio/process/integer_resampling/include \
	-Iservices/nv_section/aud_section \
	-Iservices/nv_section/userdata_section \
  	-Iservices/nv_section/include   \
	-Iservices/voicepath/$(VOICE_DATAPATH_TYPE) \
	-Iservices/voicepath/gsound \
	-Iplatform/drivers/uarthci \
	-Iplatform/drivers/ana \
	-Iplatform/cmsis \
	-Iplatform/drivers/bt \
	-Iplatform/drivers/codec \
	-Iutils/cqueue \
	-Iutils/heap \
	-Iutils/crc16 \
	-Iservices/audioflinger \
	-Iutils/lockcqueue \
	-Iutils/intersyshci \
	-Iapps/anc/inc \
	-Iapps/key \
	-Iapps/main \
	-Iapps/common \
	-Iapps/audioplayers \
	-Iapps/audioplayers/a2dp_decoder \
	-Iapps/anc/src/assist \
	-Iapps/battery \
	-Iapps/common \
	-Iapps/factory \
	-Iservices/app_ibrt/inc \
	-Iapps/voice_assist/inc \
	-Iservices/ble_app \
	-Iutils/hwtimer_list \
	-Iservices/tws_ibrt/inc \
	-Iservices/ble_stack/ble_ip \
	-Iservices/ble_stack/hl/api \
	-Iservices/ble_stack/app/api/  \
	-Iservices/ble_stack/common/api/ \
	-Iservices/ble_stack/hl/inc/ \
	-Iservices/ble_stack/ke/api \
	-Iservices/voicepath \
	-Ithirdparty/userapi \
	-Iservices/ble_app/app_gfps \
	-Iservices/ble_app/app_main \
	-Ithirdparty/audio_codec_lib/liblhdc-dec/inc \
	-Iservices/ai_voice/manager \
	-Iservices/ai_voice/audio \
	-Iservices/ai_voice/transport \
	-Iservices/app_ai/inc \
	-Iservices/interconnection/green \
	-Iservices/interconnection/umm_malloc \
	-Iservices/bt_app \
	-Iservices/multimedia/audio/codec/sbc/inc \
	-Iservices/multimedia/audio/codec/sbc/src/inc \
	-Iservices/bt_app/a2dp_codecs/include \
    -Iservices/bt_if_enhanced/inc \
    -Iservices/app_tws/inc \
	-Iservices/ble_profiles/gfps/api \
	-Ithirdparty/tile/tile_target  \
	-Ithirdparty/qiot/qiot_target  \

ifeq ($(GATT_OVER_BR_EDR),1)
ccflags-y += -Iservices/ble_stack/hl/src/l2c/l2cm
endif

ifeq ($(ANC_NOISE_TRACKER),1)
ccflags-y += \
	-Ithirdparty/noise_tracker_lib \
	-Ithirdparty/userapi/noise_tracker_app
endif

ifeq ($(BTUSB_AUDIO_MODE),1)
ccflags-y += -Iapps/btusbaudio
endif

ifeq ($(BT_USB_AUDIO_DUAL_MODE),1)
ccflags-y += $(BT_PROFILES_INCLUDES)
endif

ifeq ($(INTERACTION),1)
ccflags-y += $(BT_PROFILES_INCLUDES)
endif

CFLAGS_app_bt_stream.o += -DAUDIO_OUTPUT_VOLUME_DEFAULT=$(AUDIO_OUTPUT_VOLUME_DEFAULT)

ifeq ($(A2DP_LDAC_ON),1)
ccflags-y += -Ithirdparty/audio_codec_lib/ldac/inc 
endif

ifeq ($(RB_CODEC),1)
ccflags-y += -DRB_CODEC
endif

ifeq ($(TEST_OVER_THE_AIR),1)
ccflags-y += -Iservices/tota
endif

ifeq ($(TEST_OVER_THE_AIR_v2),1)
ccflags-y += -Iservices/tota_v2
endif

ifeq ($(IBRT),1)
ccflags-y += -Iservices/ibrt/inc
endif


ifeq ($(AUDIO_QUEUE_SUPPORT),1)
CFLAGS_app_bt_media_manager.o += -D__AUDIO_QUEUE_SUPPORT__
endif

CFLAGS_besmain.o += -DBUILD_HOSTNAME=$(BUILD_HOSTNAME) -DBUILD_USERNAME=$(BUILD_USERNAME)

ifeq ($(SPEECH_TX_AEC),1)
CFLAGS_app_bt_stream.o += -DSPEECH_TX_AEC
CFLAGS_app_hfp.o += -DSPEECH_TX_AEC
endif

ifeq ($(SPEECH_TX_NS),1)
CFLAGS_app_bt_stream.o += -DSPEECH_TX_NS
CFLAGS_app_hfp.o += -DSPEECH_TX_NS
endif

ifeq ($(AUDIO_INPUT_MONO),1)
CFLAGS_app_bt_stream.o += -D__AUDIO_INPUT_MONO_MODE__
endif

ifeq ($(AUDIO_OUTPUT_MONO),1)
CFLAGS_app_bt_stream.o += -D__AUDIO_OUTPUT_MONO_MODE__
endif
ifeq ($(AUDIO_ADAPTIVE_IIR_EQ),1)
CFLAGS_app_bt_stream.o += -DAUDIO_ADAPTIVE_IIR_EQ
CFLAGS_app_bt_stream.o += -DAUDIO_ADAPTIVE_EQ
endif

ifeq ($(AUDIO_ADAPTIVE_FIR_EQ),1)
CFLAGS_app_bt_stream.o += -DAUDIO_ADAPTIVE_FIR_EQ
CFLAGS_app_bt_stream.o += -DAUDIO_ADAPTIVE_EQ
endif

ifeq ($(AUDIO_OUTPUT_SW_GAIN),1)
CFLAGS_app_bt_stream.o += -DAUDIO_OUTPUT_SW_GAIN
endif

ifeq ($(AUDIO_OUTPUT_SW_GAIN_BEFORE_DRC),1)
CFLAGS_app_bt_stream.o += -DAUDIO_OUTPUT_SW_GAIN_BEFORE_DRC
endif

ifeq ($(AUDIO_RESAMPLE),1)
CFLAGS_app_bt_stream.o += -D__AUDIO_RESAMPLE__
endif
ifeq ($(SW_PLAYBACK_RESAMPLE),1)
CFLAGS_app_bt_stream.o += -DSW_PLAYBACK_RESAMPLE
endif
ifeq ($(SW_CAPTURE_RESAMPLE),1)
CFLAGS_app_bt_stream.o += -DSW_CAPTURE_RESAMPLE
endif
ifeq ($(SW_SCO_RESAMPLE),1)
CFLAGS_app_bt_stream.o += -DSW_SCO_RESAMPLE
endif
ifeq ($(NO_SCO_RESAMPLE),1)
CFLAGS_app_bt_stream.o += -DNO_SCO_RESAMPLE
endif
ifeq ($(RESAMPLE_ANY_SAMPLE_RATE),1)
CFLAGS_app_bt_stream.o += -DRESAMPLE_ANY_SAMPLE_RATE
endif

ifeq ($(BT_XTAL_SYNC),1)
CFLAGS_app_bt_stream.o += -DBT_XTAL_SYNC
endif

ifeq ($(AUDIO_SPECTRUM),1)
CFLAGS_app_bt_stream.o += -D__AUDIO_SPECTRUM__
endif

ifeq ($(HW_FIR_EQ_PROCESS),1)
CFLAGS_app_bt_stream.o += -D__HW_FIR_EQ_PROCESS__
endif

ifeq ($(HW_IIR_EQ_PROCESS),1)
CFLAGS_app_bt_stream.o += -D__HW_IIR_EQ_PROCESS__
endif

ifeq ($(SW_IIR_EQ_PROCESS),1)
CFLAGS_app_bt_stream.o += -D__SW_IIR_EQ_PROCESS__
endif

ifeq ($(HW_DAC_IIR_EQ_PROCESS),1)
CFLAGS_app_bt_stream.o += -D__HW_DAC_IIR_EQ_PROCESS__
endif

ifeq ($(ANC_APP),1)
CFLAGS_app_bt_stream.o += -DANC_APP
CFLAGS_app_media_player.o += -DANC_APP
endif

ifeq ($(ANC_ADPT_ENABLED),1)
CFLAGS_app_bt_stream.o += -DANC_ADPT_ENABLED
endif

MUSIC_DELAY_CONTROL ?= 1

ifeq ($(MUSIC_DELAY_CONTROL),1)
CFLAGS_app_bt_stream.o += -DMUSIC_DELAY_CONTROL
endif

ifeq ($(A2DP_EQ_24BIT),1)
CFLAGS_app_bt_stream.o += -DA2DP_EQ_24BIT
CFLAGS_app_ring_merge.o += -DA2DP_EQ_24BIT
endif

ifeq ($(APP_MUSIC_26M),1)
CFLAGS_app_bt_stream.o += -DAPP_MUSIC_26M
endif

ifeq ($(AUDIO_DRC),1)
ccflags-y += -D__AUDIO_DRC__
endif

ifeq ($(AUDIO_DRC2),1)
ccflags-y += -D__AUDIO_DRC2__
endif

ifeq ($(SPEECH_TX_AEC_CODEC_REF),1)
CFLAGS_app_bt_stream.o += -DSPEECH_TX_AEC_CODEC_REF
endif

ifeq ($(SPEECH_RX_24BIT),1)
CFLAGS_app_bt_stream.o += -DSPEECH_RX_24BIT
endif

ifeq ($(ANC_NOISE_TRACKER),1)
CFLAGS_app_bt_stream.o += -DANC_NOISE_TRACKER
endif

ifeq ($(SNIFF_MODE_CHECK),1)
CFLAGS_app_a2dp.o += -D__SNIFF_MODE_CHECK__
endif

ifeq ($(HFP_DISABLE_NREC),1)
CFLAGS_app_hfp.o += -DHFP_DISABLE_NREC
endif

ifeq ($(MEDIA_PLAY_24BIT),1)
CFLAGS_app_media_player.o += -DMEDIA_PLAY_24BIT
endif

ifneq ($(filter 8000 16000,$(SPEECH_VQE_FIXED_SAMPLE_RATE)),)
CFLAGS_app_bt_stream.o += -DSPEECH_VQE_FIXED_SAMPLE_RATE=$(SPEECH_VQE_FIXED_SAMPLE_RATE)
endif

ifeq ($(PLAYBACK_FORCE_48K),1)
CFLAGS_app_bt_stream.o += -DPLAYBACK_FORCE_48K
endif

ifeq ($(ANC_ASSIST_USE_INT_CODEC),1)
CFLAGS_app_bt_stream.o += -DANC_ASSIST_USE_INT_CODEC
CFLAGS_app_media_player.o += -DANC_ASSIST_USE_INT_CODEC
endif

ifeq ($(AUDIO_REVERB),1)
CFLAGS_app_bt_stream.o += -D__AUDIO_REVERB__
endif

ifeq ($(USE_LOWLATENCY_LIB),1)
ccflags-y += -Ithirdparty/lowlatency_lib/inc
endif

ifeq ($(BT_NO_SLEEP),1)
ccflags-y += -D__BT_NO_SLEEP__
endif
