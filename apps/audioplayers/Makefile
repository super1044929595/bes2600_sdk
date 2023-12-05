
cur_dir := $(dir $(lastword $(MAKEFILE_LIST)))

obj_s := $(patsubst $(cur_dir)%,%,$(wildcard $(cur_dir)*.s))
obj_c := $(patsubst $(cur_dir)%,%,$(wildcard $(cur_dir)*.c))
obj_cpp := $(patsubst $(cur_dir)%,%,$(wildcard $(cur_dir)*.cpp))

obj_cpp := $(filter-out a2dpplay.cpp,$(obj_cpp))
obj_cpp += a2dp_decoder/
bt_sco_chain_obj = bt_sco_chain.c bt_sco_chain_cp.c bt_sco_chain_thirdparty.c bt_sco_chain_cp_thirdparty.c bt_sco_chain_reframe.c
bt_sco_chain_obj += bt_sco_chain_thirdparty_alango.c
bt_sco_chain_obj += bt_sco_chain_thirdparty_soundplus.c bt_sco_chain_cp_soundplus.c
obj_c := $(filter-out $(bt_sco_chain_obj),$(obj_c))

ifeq ($(SPEECH_TX_THIRDPARTY_ALANGO),1)
obj_c += bt_sco_chain_thirdparty_alango.c
else ifeq ($(SPEECH_TX_THIRDPARTY_SNDP),1)
obj_c += bt_sco_chain_thirdparty_soundplus.c
obj_c += bt_sco_chain_cp_soundplus.c
else ifeq ($(SPEECH_TX_THIRDPARTY),1)
obj_c += bt_sco_chain_thirdparty.c
obj_c += bt_sco_chain_cp.c
# obj_c += bt_sco_chain_cp_thirdparty.c
# obj_c += bt_sco_chain_reframe.c
else
obj_c += bt_sco_chain.c
obj_c += bt_sco_chain_cp.c
endif

ifeq ($(SCO_ADD_CUSTOMER_CODEC),0)
obj_c := $(filter-out bt_sco_codec_xxxx.c,$(obj_c))
endif
ifeq ($(SCO_DMA_SNAPSHOT),1)
obj_cpp := $(filter-out voicebtpcmplay.cpp,$(obj_cpp))
else
obj_cpp := $(filter-out voicebtpcmplay_sco_dma_snapshot.cpp,$(obj_cpp))
endif

ifeq ($(RB_CODEC),1)
obj_cpp += $(patsubst $(cur_dir)%,%,$(wildcard $(cur_dir)rbplay/*.cpp))
endif

obj-y := $(obj_c:.c=.o) $(obj_s:.S=.o) $(obj_cpp:.cpp=.o)

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

ccflags-y := \
	$(AAC_INCLUDES) \
	-Iservices/audio_process \
	-Iservices/resources \
	-Iservices/bt_app \
	$(BT_IF_INCLUDES) \
	-Iplatform/drivers/uarthci \
	-Iutils/cqueue \
	-Iservices/audio_dump/include \
	-Iservices/multimedia/speech/inc \
	-Iservices/multimedia/rbcodec/inc \
	-Iservices/multimedia/audio/process/eq/include \
	-Iservices/multimedia/audio/process/resample/include \
	-Iservices/multimedia/audio/process/filters/include \
	-Iservices/multimedia/fm/inc \
	-Iservices/nv_section/aud_section \
	-Iservices/nv_section/include \
	-Iservices/overlay \
	-Iservices/norflash_api \
	-Iservices/nvrecord \
	-Iservices/nv_section/log_section \
	-Iapps/main \
	-Iapps/audioplayers/rbplay/ \
	-Iapps/audioplayers/a2dp_decoder \
	-Iutils/list \
	-Iutils/heap \
	-Irtos/rtos \
	-Iplatform/drivers/ana \
	-Ithirdparty/audio_codec_lib/scalable/ \
	-Ithirdparty/audio_codec_lib/liblhdc-dec/inc \
	-Iapps/apptester \
	-Iapps/key  \
	-Iplatform/drivers/bt \
	-Iapps/anc/inc \
	-Iservices/multimedia/audio/codec/sbc/inc \
	-Iservices/multimedia/audio/codec/sbc/src/inc \
	-Iservices/bt_app/a2dp_codecs/include \
	-Iinclude/cmsis_dsp \
	-Iinclude/cmsis_nn

CFLAGS_app_audio.o += -Wno-array-bounds

ifeq ($(IBRT),1)
ccflags-y += -Iservices/tws_ibrt/inc
ccflags-y += -Iservices/app_ibrt/inc
endif

ifeq ($(A2DP_LDAC_ON),1)
ccflags-y += -Ithirdparty/audio_codec_lib/ldac/inc
endif

ifeq ($(A2DP_CP_ACCEL),1)
ccflags-y += -Iplatform/drivers/cp_accel
else ifeq ($(SCO_CP_ACCEL),1)
ccflags-y += -Iplatform/drivers/cp_accel
endif

ifeq ($(APP_TEST_AUDIO),1)
CFLAGS_app_audio.o += -DAPP_TEST_AUDIO
endif

ifeq ($(AUDIO_RESAMPLE),1)
CFLAGS_a2dpplay.o += -D__AUDIO_RESAMPLE__
CFLAGS_voicebtpcmplay.o += -D__AUDIO_RESAMPLE__
CFLAGS_voicebtpcmplay_sco_dma_snapshot.o += -D__AUDIO_RESAMPLE__
CFLAGS_bt_sco_chain.o += -D__AUDIO_RESAMPLE__
endif

ifeq ($(SW_PLAYBACK_RESAMPLE),1)
CFLAGS_a2dpplay.o += -DSW_PLAYBACK_RESAMPLE
endif

ifeq ($(RESAMPLE_ANY_SAMPLE_RATE),1)
CFLAGS_a2dpplay.o += -DRESAMPLE_ANY_SAMPLE_RATE
endif

ifeq ($(SW_SCO_RESAMPLE),1)
CFLAGS_voicebtpcmplay.o += -DSW_SCO_RESAMPLE
CFLAGS_voicebtpcmplay_sco_dma_snapshot.o += -DSW_SCO_RESAMPLE
endif

ifeq ($(SPEECH_TX_THIRDPARTY_ALANGO),1)
CFLAGS_bt_sco_chain_thirdparty_alango.o += -Ithirdparty/alango_lib/include
endif

ifeq ($(SPEECH_TX_THIRDPARTY_SNDP),1)
CFLAGS_bt_sco_chain_thirdparty_soundplus.o += -Ithirdparty/$(THIRDPARTY_LIB)_lib/sp_ec_mv03
CFLAGS_bt_sco_chain_cp_soundplus.o += -Ithirdparty/$(THIRDPARTY_LIB)_lib/sp_ec_mv03
endif

ifeq ($(VOICE_PROMPT),1)
CFLAGS_app_audio.o += -DMEDIA_PLAYER_SUPPORT
endif

ifeq ($(AUDIO_QUEUE_SUPPORT),1)
CFLAGS_app_audio.o += -D__AUDIO_QUEUE_SUPPORT__
endif

ifeq ($(ANC_APP),1)
CFLAGS_app_audio.o += -DANC_APP
endif

ifeq ($(A2DP_EQ_24BIT),1)
CFLAGS_app_audio.o += -DA2DP_EQ_24BIT
CFLAGS_a2dpplay.o += -DA2DP_EQ_24BIT
endif

ifeq ($(A2DP_TRACE_CP_ACCEL),1)
CFLAGS_a2dpplay.o += -DA2DP_TRACE_CP_ACCEL
endif

ifeq ($(A2DP_TRACE_DEC_TIME),1)
CFLAGS_a2dpplay.o += -DA2DP_TRACE_DEC_TIME
endif

ifeq ($(A2DP_TRACE_CP_DEC_TIME),1)
CFLAGS_a2dpplay.o += -DA2DP_TRACE_CP_DEC_TIME
endif

ifeq ($(SPEECH_TX_AEC_CODEC_REF),1)
CFLAGS_voicebtpcmplay.o += -DSPEECH_TX_AEC_CODEC_REF
CFLAGS_voicebtpcmplay_sco_dma_snapshot.o += -DSPEECH_TX_AEC_CODEC_REF
CFLAGS_bt_sco_chain_cfg_default.o += -DSPEECH_TX_AEC_CODEC_REF
endif

ifeq ($(SPEECH_RX_24BIT),1)
CFLAGS_bt_sco_chain.o += -DSPEECH_RX_24BIT
CFLAGS_bt_sco_chain_thirdparty.o += -DSPEECH_RX_24BIT
CFLAGS_voicebtpcmplay.o += -DSPEECH_RX_24BIT
CFLAGS_voicebtpcmplay_sco_dma_snapshot.o += -DSPEECH_RX_24BIT
endif

ifeq ($(SPEECH_RX_COMPEXP),1)
CFLAGS_bt_sco_chain.o += -DSPEECH_RX_COMPEXP
CFLAGS_bt_sco_chain_thirdparty.o += -DSPEECH_RX_COMPEXP
CFLAGS_bt_sco_chain_cfg_default.o += -DSPEECH_RX_COMPEXP
CFLAGS_bt_sco_chain_tuning.o += -DSPEECH_RX_COMPEXP
endif

ifeq ($(SPEECH_RX_HW_EQ),1)
CFLAGS_bt_sco_chain.o += -DSPEECH_RX_HW_EQ
CFLAGS_bt_sco_chain_thirdparty.o += -DSPEECH_RX_HW_EQ
CFLAGS_bt_sco_chain_cfg_default.o += -DSPEECH_RX_HW_EQ
CFLAGS_bt_sco_chain_tuning.o += -DSPEECH_RX_HW_EQ
endif

ifeq ($(SPEECH_RX_DAC_EQ),1)
CFLAGS_bt_sco_chain.o += -DSPEECH_RX_DAC_EQ
CFLAGS_bt_sco_chain_thirdparty.o += -DSPEECH_RX_DAC_EQ
CFLAGS_bt_sco_chain_cfg_default.o += -DSPEECH_RX_DAC_EQ
CFLAGS_bt_sco_chain_tuning.o += -DSPEECH_RX_DAC_EQ
endif

ifeq ($(SPEECH_TX_2MIC_SWAP_CHANNELS),1)
CFLAGS_bt_sco_chain.o += -DSPEECH_TX_2MIC_SWAP_CHANNELS
endif

ifeq ($(AI_VOICE),1)
ccflags-y += -Iservices/ai_voice/audio
endif
