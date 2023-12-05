
cur_dir := $(dir $(lastword $(MAKEFILE_LIST)))

obj-y := a2dp_decoder.o
obj-y += a2dp_decoder_sbc.o
ifeq ($(A2DP_AAC_ON),1)
obj-y += a2dp_decoder_aac_lc.o
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

ifeq ($(A2DP_SCALABLE_ON),1)
obj-y += a2dp_decoder_scalable.o
SCALABLE_INCLUDES =  \
	-Ithirdparty/audio_codec_lib/scalable
else
SCALABLE_INCLUDES =
endif

ifeq ($(A2DP_LHDC_ON),1)
obj-y += a2dp_decoder_lhdc.o
LHDC_INCLUDES =  \
	-Ithirdparty/audio_codec_lib/liblhdc-dec/inc
else
LHDC_INCLUDES =
endif

ifeq ($(A2DP_LDAC_ON),1)
obj-y += a2dp_decoder_ldac.o
LDAC_INCLUDES =  \
	-Ithirdparty/audio_codec_lib/ldac/inc
else
LDAC_INCLUDES =
endif

ccflags-y := \
	$(AAC_INCLUDES) \
	$(SCALABLE_INCLUDES) \
	$(LHDC_INCLUDES) \
	$(LDAC_INCLUDES) \
	-Iservices/audio_process \
	-Iservices/fs/fat \
	-Iservices/fs/sd \
	-Iservices/resources \
	-Iservices/fs/fat/ChaN \
	-Iservices/bt_app \
	-Iservices/bt_app/a2dp_codecs/include \
	$(BT_IF_INCLUDES) \
	-Iplatform/drivers/uarthci \
	-Iutils/cqueue \
	-Iservices/audio_dump/include \
	-Iservices/multimedia/speech/inc \
	-Iservices/multimedia/rbcodec/inc \
	-Iservices/multimedia/audio/process/eq/include \
	-Iservices/multimedia/audio/process/resample/include \
	-Iservices/multimedia/audio/process/filters/include \
	-Iservices/multimedia/audio/process/sbcplc/include \
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
	-Iplatform/drivers/ana \
	-Iapps/apptester \
	-Iapps/key  \
	-Iservices/multimedia/audio/codec/sbc/inc \
	-Iservices/multimedia/audio/codec/sbc/src/inc \
	-Iplatform/drivers/bt \
	-Iutils/crc32

ifeq ($(A2DP_LHDC_ON),1)
ccflags-y += -Iservices/bt_if_enhanced/lhdc_license
endif

ifeq ($(IBRT),1)
ccflags-y += -Iservices/tws_ibrt/inc
ccflags-y += -Iservices/app_ibrt/inc
endif

ifeq ($(A2DP_CP_ACCEL),1)
obj-y += a2dp_decoder_cp.o
ccflags-y += -Iplatform/drivers/cp_accel
endif

ifeq ($(A2DP_TRACE_CP_ACCEL),1)
ccflags-y += -DA2DP_TRACE_CP_ACCEL
endif

ifeq ($(A2DP_TRACE_DEC_TIME),1)
ccflags-y += -DA2DP_TRACE_DEC_TIME
endif

ifeq ($(A2DP_TRACE_CP_DEC_TIME),1)
ccflags-y += -DA2DP_TRACE_CP_DEC_TIME
endif

ifeq ($(A2DP_SBC_PLC_ENABLED),1)
CFLAGS_a2dp_decoder_sbc.o += -DA2DP_SBC_PLC_ENABLED
endif

ifeq ($(USE_LOWLATENCY_LIB),1)
ccflags-y += -Ithirdparty/lowlatency_lib/inc
endif

ifeq ($(AUDIO_DYNAMIC_BOOST),1)
ccflags-y += -D__AUDIO_DYNAMIC_BOOST__
endif
