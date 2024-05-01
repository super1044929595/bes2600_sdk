
add_if_exists = $(foreach d,$(1),$(if $(wildcard $(srctree)/$(d)),$(d) ,))

# -------------------------------------------
# Root Option Dependencies
# -------------------------------------------
ifeq ($(BT_ANC),1)
export ANC_APP ?= 1
endif

ifeq ($(BISTO),1)
BLE := 1
export CRASH_REBOOT ?= 1
ifeq ($(CHIP),best1400)
export DUMP_CRASH_LOG ?= 0
else
export DUMP_CRASH_LOG ?= 1
endif
VOICE_DATAPATH_ENABLED ?= 1
export GFPS_ENABLE ?= 1
MIX_MIC_DURING_MUSIC_ENABLED ?= 0

KBUILD_CPPFLAGS += -DCFG_SW_KEY_LPRESS_THRESH_MS=1000

export VOICE_DATAPATH_TYPE ?= gsound
#export TRACE_DUMP2FLASH ?= 1

export FLASH_SUSPEND ?= 1
ifeq ($(FLASH_SUSPEND), 1)
KBUILD_CPPFLAGS += -DFLASH_SUSPEND
endif

KBUILD_CFLAGS += -DGSOUND_ENABLED
KBUILD_CPPFLAGS += -DDEBUG_BLE_DATAPATH=0
KBUILD_CPPFLAGS += -DGSOUND_OTA_ENABLED
export OTA_BASIC := 1
KBUILD_CFLAGS += -DCRC32_OF_IMAGE

#export OPUS_CODEC ?= 1
#ENCODING_ALGORITHM_OPUS                2
#ENCODING_ALGORITHM_SBC                 3
#KBUILD_CPPFLAGS += -DVOB_ENCODING_ALGORITHM=3

# As MIX_MIC_DURING_MUSIC uses the isolated audio stream, if define MIX_MIC_DURING_MUSIC, the isolated audio stream
# must be enabled
KBUILD_CPPFLAGS += -DISOLATED_AUDIO_STREAM_ENABLED=1

ASSERT_SHOW_FILE_FUNC ?= 1
#KBUILD_CPPFLAGS += -DSAVING_AUDIO_DATA_TO_SD_ENABLED=1
endif # BISTO

export AMA_VOICE ?= 0
ifeq ($(AMA_VOICE),1)
BLE := 1
export AI_VOICE ?= 1

KBUILD_CPPFLAGS += -DBTIF_DIP_DEVICE
#KBUILD_CPPFLAGS += -DKEYWORD_WAKEUP_ENABLED
#KBUILD_CPPFLAGS += -DPUSH_AND_HOLD_ENABLED
#KBUILD_CPPFLAGS += -DAI_32KBPS_VOICE
KBUILD_CPPFLAGS += -D__AMA_VOICE__
#KBUILD_CPPFLAGS += -DNO_LOCAL_START_TONE
ifeq ($(CHIP),best1400)
KBUILD_CPPFLAGS += -DVOB_ENCODING_ALGORITHM=3
export OPUS_CODEC ?= 0
else
KBUILD_CPPFLAGS += -DVOB_ENCODING_ALGORITHM=2
export OPUS_CODEC ?= 1
endif
endif

export BLE_ONLY_ENABLED ?= 0
ifeq ($(BLE_ONLY_ENABLED),1)
BLE := 1
KBUILD_CPPFLAGS += -DBLE_ONLY_ENABLED
KBUILD_CPPFLAGS += -DBLE_POWER_LEVEL_0
endif

export DMA_VOICE ?= 0
ifeq ($(DMA_VOICE),1)
export AI_VOICE ?= 1
KBUILD_CPPFLAGS += -D__DMA_VOICE__
KBUILD_CPPFLAGS += -D__BES__
KBUILD_CPPFLAGS += -DBAIDU_DATA_SN_LEN=16
KBUILD_CPPFLAGS += -DFLOW_CONTROL_ON_UPLEVEL=1
KBUILD_CPPFLAGS += -DASSAM_PKT_ON_UPLEVEL=1
KBUILD_CPPFLAGS += -DBAIDU_DATA_RAND_LEN=8
KBUILD_CPPFLAGS += -DCLOSE_BLE_ADV_WHEN_VOICE_CALL=1
KBUILD_CPPFLAGS += -DCLOSE_BLE_ADV_WHEN_SPP_CONNECTED=1
KBUILD_CPPFLAGS += -DBAIDU_RFCOMM_DIRECT_CONN=1
KBUILD_CPPFLAGS += -DBYPASS_SLOW_ADV_MODE=1

KBUILD_CPPFLAGS += -DNVREC_BAIDU_DATA_SECTION=1
KBUILD_CPPFLAGS += -DFM_MIN_FREQ=875 -DFM_MAX_FREQ=1079
KBUILD_CPPFLAGS += -DBAIDU_DATA_DEF_FM_FREQ=893
KBUILD_CPPFLAGS += -DBAIDU_DATA_RAND_LEN=8
KBUILD_CPPFLAGS += -DBAIDU_DATA_SN_LEN=16

#KBUILD_CPPFLAGS += -DKEYWORD_WAKEUP_ENABLED
#KBUILD_CPPFLAGS += -DPUSH_AND_HOLD_ENABLED
#KBUILD_CPPFLAGS += -DAI_32KBPS_VOICE
KBUILD_CPPFLAGS += -DVOB_ENCODING_ALGORITHM=2
export OPUS_CODEC ?= 1
export LIBC_ROM := 0
ifeq ($(LIBC_ROM),1)
$(error LIBC_ROM should be 0 when DMA_VOICE=1)
endif
endif

export SMART_VOICE ?= 0
ifeq ($(SMART_VOICE),1)
export AI_VOICE ?= 1
KBUILD_CPPFLAGS += -D__SMART_VOICE__
#KBUILD_CPPFLAGS += -DKEYWORD_WAKEUP_ENABLED
KBUILD_CPPFLAGS += -DPUSH_AND_HOLD_ENABLED
#KBUILD_CPPFLAGS += -DAI_32KBPS_VOICE
KBUILD_CPPFLAGS += -DVOB_ENCODING_ALGORITHM=2
export OPUS_CODEC ?= 1
#SPEECH_CODEC_CAPTURE_CHANNEL_NUM ?= 2
#KBUILD_CPPFLAGS += -DMCU_HIGH_PERFORMANCE_MODE
#KBUILD_CPPFLAGS += -DSPEECH_CAPTURE_TWO_CHANNEL
endif

export TENCENT_VOICE ?= 0
ifeq ($(TENCENT_VOICE),1)
export AI_VOICE ?= 1
KBUILD_CPPFLAGS += -D__TENCENT_VOICE__
#KBUILD_CPPFLAGS += -DKEYWORD_WAKEUP_ENABLED
KBUILD_CPPFLAGS += -DPUSH_AND_HOLD_ENABLED
#KBUILD_CPPFLAGS += -DAI_32KBPS_VOICE
KBUILD_CPPFLAGS += -DVOB_ENCODING_ALGORITHM=2
export OPUS_CODEC ?= 1
endif

ifeq ($(AI_VOICE),1)
KBUILD_CPPFLAGS += -D__AI_VOICE__
KBUILD_CPPFLAGS += -DNO_ENCODING=0
KBUILD_CPPFLAGS += -DENCODING_ALGORITHM_ADPCM=1
KBUILD_CPPFLAGS += -DENCODING_ALGORITHM_OPUS=2
KBUILD_CPPFLAGS += -DENCODING_ALGORITHM_SBC=3
ifeq ($(USE_KNOWLES),1)
KBUILD_CPPFLAGS += -D__KNOWLES \
                   -DDIG_MIC_WORKAROUND

KBUILD_CPPFLAGS += -DKNOWLES_UART_DATA
KBUILD_CPPFLAGS += -DIDLE_ALEXA_KWD
export THIRDPARTY_LIB ?= knowles_uart
endif
endif

ifeq ($(BISTO),1)
KBUILD_CPPFLAGS += -DIS_GSOUND_BUTTION_HANDLER_WORKAROUND_ENABLED
endif

## Microsoft swift pair
export SWIFT_ENABLE ?= 0
ifeq ($(SWIFT_ENABLE),1)
KBUILD_CPPFLAGS += -DSWIFT_ENABLED
endif

ifeq ($(AI_VOICE),1)
ifeq ($(BISTO),1)
IS_MULTI_AI_ENABLED ?= 1
else
IS_MULTI_AI_ENABLED ?= 0
endif
else
IS_MULTI_AI_ENABLED ?= 0
endif

export QIOT_DATAPATH_ENABLE ?= 0
ifeq ($(QIOT_DATAPATH_ENABLE),1)
KBUILD_CPPFLAGS += -DQIOT_ENABLED
QIOT_CORE_SECTION_SIZE ?= 0x1000
QIOT_OTA_SECTION_SIZE ?= 0x1000
endif
# vvvvvvvvvvvvvvvvvvvvvvvvvvvvv
# TILE feature
# ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
ifeq ($(TILE_DATAPATH_ENABLED),1)
export TILE_DATAPATH_ENABLED
KBUILD_CPPFLAGS += -DTILE_DATAPATH
endif

export CUSTOM_INFORMATION_TILE_ENABLE ?= 0
ifeq ($(CUSTOM_INFORMATION_TILE_ENABLE),1)
KBUILD_CPPFLAGS += -DCUSTOM_INFORMATION_TILE=1
endif # TILE
export OTA_BASIC ?= 0
ifneq ($(filter 1,$(OTA_BASIC) $(BES_OTA_BASIC)),)
export OTA_CODE_OFFSET ?= 0x10000
KBUILD_CPPFLAGS += -D__APP_IMAGE_FLASH_OFFSET__=$(OTA_CODE_OFFSET)
ifeq ($(OTA_BIN_COMPRESSED),1)
export NEW_IMAGE_FLASH_OFFSET ?= 0x130000
else
export NEW_IMAGE_FLASH_OFFSET ?= 0x100000
endif
KBUILD_CPPFLAGS += -DNEW_IMAGE_FLASH_OFFSET=$(NEW_IMAGE_FLASH_OFFSET)
KBUILD_CPPFLAGS += -DFIRMWARE_REV
KBUILD_CPPFLAGS += -DBES_OTA_BASIC
endif

export OTA_OVER_TOTA_ENABLED ?= 0
ifeq ($(OTA_OVER_TOTA_ENABLED),1)
KBUILD_CPPFLAGS += -DOTA_OVER_TOTA_ENABLED
endif	#OTA_OVER_TOTA_ENABLED

# make sure the value of GFPS_ENABLE and GMA_VOICE is confirmed above here
ifneq ($(filter 1,$(GFPS_ENABLE) $(GMA_VOICE) $(TOTA) $(TOTA_v2) $(BLE)),)
core-y += utils/encrypt/
endif

ifneq ($(filter apps/ tests/speech_test/ tests/ota_boot/, $(core-y)),)
BT_APP ?= 1
FULL_APP_PROJECT ?= 1
endif

export NEW_NV_RECORD_ENALBED ?= 1
ifeq ($(NEW_NV_RECORD_ENALBED),1)
KBUILD_CPPFLAGS += -DNEW_NV_RECORD_ENALBED
KBUILD_CPPFLAGS += -Iservices/nv_section/userdata_section
endif

# -------------------------------------------
# CHIP selection
# -------------------------------------------

export CHIP

ifneq (,)
else ifeq ($(CHIP),best1000)
KBUILD_CPPFLAGS += -DCHIP_BEST1000
export CHIP_HAS_FPU := 1
export CHIP_HAS_USB := 1
export CHIP_HAS_USBPHY := 0
export CHIP_HAS_SDMMC := 1
export CHIP_HAS_SDIO := 1
export CHIP_HAS_PSRAM := 1
export CHIP_HAS_SPI := 1
export CHIP_HAS_SPILCD := 1
export CHIP_HAS_SPIPHY := 0
export CHIP_HAS_I2C := 1
export CHIP_HAS_UART := 2
export CHIP_HAS_DMA := 2
export CHIP_HAS_SPDIF := 1
export CHIP_HAS_TRANSQ := 0
export CHIP_HAS_EXT_PMU := 0
export CHIP_HAS_AUDIO_CONST_ROM := 1
export CHIP_FLASH_CTRL_VER := 1
export CHIP_PSRAM_CTRL_VER := 1
export CHIP_SPI_VER := 1
export CHIP_HAS_EC_CODEC_REF := 0
else ifeq ($(CHIP),best1305)
KBUILD_CPPFLAGS += -DCHIP_BEST1305
CPU := m33
SHA256_ROM := 0
export SECURE_BOOT := 0
export CHIP_HAS_FPU := 1
export CHIP_HAS_USB := 0
export CHIP_HAS_USBPHY := 0
export CHIP_HAS_SDMMC := 0
export CHIP_HAS_SDIO := 0
export CHIP_HAS_PSRAM := 0
export CHIP_HAS_SPI := 1
export CHIP_HAS_SPILCD := 0
export CHIP_HAS_SPIPHY := 0
export CHIP_HAS_I2C := 3
export CHIP_HAS_UART := 2
export CHIP_HAS_DMA := 2
export CHIP_HAS_I2S := 1
export CHIP_HAS_TDM := 1
export CHIP_HAS_SPDIF := 0
export CHIP_HAS_TRANSQ := 0
export CHIP_HAS_PSC := 1
export CHIP_HAS_EXT_PMU := 0
export CHIP_HAS_CP := 1
export CHIP_HAS_AUDIO_CONST_ROM := 0
export CHIP_FLASH_CTRL_VER := 4
export CHIP_CACHE_VER := 3
export CHIP_SPI_VER := 4
export CHIP_HAS_EC_CODEC_REF := 1
export CHIP_HAS_SCO_DMA_SNAPSHOT := 1
export CHIP_ROM_UTILS_VER := 1
export CHIP_HAS_ANC_HW_GAIN_SMOOTHING := 0
else ifeq ($(CHIP),best1400)
ifeq ($(CHIP_SUBTYPE),best1402)
SUBTYPE_VALID := 1
KBUILD_CPPFLAGS += -DCHIP_BEST1402
else
KBUILD_CPPFLAGS += -DCHIP_BEST1400
endif
export CHIP_HAS_FPU := 1
export CHIP_HAS_USB := 1
export CHIP_HAS_USBPHY := 0
export CHIP_HAS_SDMMC := 0
export CHIP_HAS_SDIO := 0
export CHIP_HAS_PSRAM := 0
export CHIP_HAS_SPI := 1
export CHIP_HAS_SPILCD := 0
export CHIP_HAS_SPIPHY := 0
export CHIP_HAS_I2C := 1
export CHIP_HAS_UART := 3
export CHIP_HAS_DMA := 1
export CHIP_HAS_SPDIF := 0
export CHIP_HAS_TRANSQ := 0
export CHIP_HAS_EXT_PMU := 0
export CHIP_HAS_AUDIO_CONST_ROM := 0
export CHIP_FLASH_CTRL_VER := 2
export CHIP_SPI_VER := 3
export BTDUMP_ENABLE ?= 1
export CHIP_HAS_EC_CODEC_REF := 1
else ifeq ($(CHIP),best2000)
KBUILD_CPPFLAGS += -DCHIP_BEST2000
export CHIP_HAS_FPU := 1
export CHIP_HAS_USB := 1
export CHIP_HAS_USBPHY := 1
export CHIP_HAS_SDMMC := 1
export CHIP_HAS_SDIO := 1
export CHIP_HAS_PSRAM := 1
export CHIP_HAS_SPI := 1
export CHIP_HAS_SPILCD := 1
export CHIP_HAS_SPIPHY := 1
export CHIP_HAS_I2C := 1
export CHIP_HAS_UART := 3
export CHIP_HAS_DMA := 2
export CHIP_HAS_SPDIF := 2
export CHIP_HAS_TRANSQ := 1
export CHIP_HAS_EXT_PMU := 0
export CHIP_HAS_AUDIO_CONST_ROM := 0
export CHIP_FLASH_CTRL_VER := 1
export CHIP_PSRAM_CTRL_VER := 1
export CHIP_SPI_VER := 1
export CHIP_HAS_EC_CODEC_REF := 0
else ifeq ($(CHIP),best2001)
KBUILD_CPPFLAGS += -DCHIP_BEST2001
ifeq ($(CHIP_SUBSYS),dsp)
KBUILD_CPPFLAGS += -DCHIP_BEST2001_DSP
CPU := a7
else
CPU := m33
export CHIP_HAS_CP := 1
endif
export CHIP_HAS_FPU := 1
export CHIP_HAS_USB := 1
export CHIP_HAS_USBPHY := 1
export CHIP_HAS_SDMMC := 1
export CHIP_HAS_SDIO := 0
export CHIP_HAS_PSRAM := 1
export CHIP_HAS_PSRAMUHS := 1
export CHIP_HAS_SPI := 1
export CHIP_HAS_SPILCD := 1
export CHIP_HAS_SPIPHY := 1
export CHIP_HAS_SPIDPD := 1
export CHIP_HAS_I2C := 2
export CHIP_HAS_UART := 3
export CHIP_HAS_DMA := 2
export CHIP_HAS_SPDIF := 1
export CHIP_HAS_TRANSQ := 2
export CHIP_HAS_EXT_PMU := 0
export CHIP_HAS_AUDIO_CONST_ROM := 0
export CHIP_FLASH_CTRL_VER := 2
export CHIP_PSRAM_CTRL_VER := 2
export CHIP_SPI_VER := 4
export CHIP_CACHE_VER := 2
export CHIP_HAS_EC_CODEC_REF := 1
else ifeq ($(CHIP),best2300)
KBUILD_CPPFLAGS += -DCHIP_BEST2300
export CHIP_HAS_FPU := 1
export CHIP_HAS_USB := 1
export CHIP_HAS_USBPHY := 1
export CHIP_HAS_SDMMC := 1
export CHIP_HAS_SDIO := 0
export CHIP_HAS_PSRAM := 0
export CHIP_HAS_SPI := 1
export CHIP_HAS_SPILCD := 1
export CHIP_HAS_SPIPHY := 1
export CHIP_HAS_I2C := 2
export CHIP_HAS_UART := 3
export CHIP_HAS_DMA := 2
export CHIP_HAS_SPDIF := 1
export CHIP_HAS_TRANSQ := 0
export CHIP_HAS_EXT_PMU := 1
export CHIP_HAS_AUDIO_CONST_ROM := 0
export CHIP_FLASH_CTRL_VER := 2
export CHIP_SPI_VER := 2
export CHIP_HAS_EC_CODEC_REF := 0
else ifeq ($(CHIP),best2300a)
KBUILD_CPPFLAGS += -DCHIP_BEST2300A
CPU := m33
LIBC_ROM := 0
CRC32_ROM := 0
SHA256_ROM := 0
export LIBC_OVERRIDE ?= 1
export CHIP_HAS_FPU := 1
export CHIP_HAS_USB := 1
export CHIP_HAS_USBPHY := 1
export CHIP_HAS_SDMMC := 1
export CHIP_HAS_SDIO := 0
export CHIP_HAS_PSRAM := 0
export CHIP_HAS_SPI := 1
export CHIP_HAS_SPILCD := 1
export CHIP_HAS_SPIPHY := 1
export CHIP_HAS_I2C := 2
export CHIP_HAS_UART := 3
export CHIP_HAS_DMA := 2
export CHIP_HAS_I2S := 2
export CHIP_HAS_TDM := 2
export CHIP_HAS_SPDIF := 1
export CHIP_HAS_TRANSQ := 0
export CHIP_HAS_PSC := 1
export CHIP_HAS_EXT_PMU := 1
export CHIP_HAS_CP := 1
export CHIP_HAS_AUDIO_CONST_ROM := 0
export CHIP_FLASH_CTRL_VER := 3
export CHIP_SPI_VER := 4
export CHIP_CACHE_VER := 2
export CHIP_RAM_BOOT := 1
export CHIP_HAS_EC_CODEC_REF := 1
export CHIP_HAS_SCO_DMA_SNAPSHOT ?= 1
export CHIP_HAS_ANC_GAIN_SMOOTHING ?= 1
else ifeq ($(CHIP),best2300p)
KBUILD_CPPFLAGS += -DCHIP_BEST2300P
export CHIP_HAS_FPU := 1
export CHIP_HAS_USB := 1
export CHIP_HAS_USBPHY := 1
export CHIP_HAS_SDMMC := 1
export CHIP_HAS_SDIO := 0
export CHIP_HAS_PSRAM := 0
export CHIP_HAS_SPI := 1
export CHIP_HAS_SPILCD := 1
export CHIP_HAS_SPIPHY := 1
export CHIP_HAS_I2C := 2
export CHIP_HAS_UART := 3
export CHIP_HAS_DMA := 3
export CHIP_HAS_BCM := 0
export CHIP_HAS_I2S := 2
export CHIP_HAS_SPDIF := 1
export CHIP_HAS_TRANSQ := 0
export CHIP_HAS_EXT_PMU := 1
export CHIP_HAS_CP := 1
export CHIP_HAS_AUDIO_CONST_ROM := 0
export CHIP_FLASH_CTRL_VER := 2
export CHIP_SPI_VER := 3
export CHIP_CACHE_VER := 2
export CHIP_HAS_EC_CODEC_REF := 1
else ifeq ($(CHIP),best3001)
ifeq ($(CHIP_SUBTYPE),best3005)
SUBTYPE_VALID := 1
KBUILD_CPPFLAGS += -DCHIP_BEST3005
export CHIP_CACHE_VER := 2
export CHIP_FLASH_CTRL_VER := 2
else
KBUILD_CPPFLAGS += -DCHIP_BEST3001
export CHIP_FLASH_CTRL_VER := 1
endif
export CHIP_HAS_FPU := 1
export CHIP_HAS_USB := 1
export CHIP_HAS_USBPHY := 1
export CHIP_HAS_SDMMC := 0
export CHIP_HAS_SDIO := 0
export CHIP_HAS_PSRAM := 0
export CHIP_HAS_SPI := 1
export CHIP_HAS_SPILCD := 0
export CHIP_HAS_SPIPHY := 0
export CHIP_HAS_I2C := 1
export CHIP_HAS_UART := 2
export CHIP_HAS_DMA := 1
export CHIP_HAS_SPDIF := 1
export CHIP_HAS_TRANSQ := 0
export CHIP_HAS_EXT_PMU := 0
export CHIP_HAS_AUDIO_CONST_ROM := 0
export CHIP_SPI_VER := 3
export CHIP_HAS_EC_CODEC_REF := 0
else ifeq ($(CHIP),best3003)
KBUILD_CPPFLAGS += -DCHIP_BEST3003
CPU := m33
export CHIP_HAS_FPU := 1
export CHIP_HAS_USB := 1
export CHIP_HAS_USBPHY := 1
export CHIP_HAS_SDMMC := 0
export CHIP_HAS_SDIO := 0
export CHIP_HAS_PSRAM := 0
export CHIP_HAS_SPI := 1
export CHIP_HAS_SPILCD := 0
export CHIP_HAS_SPIPHY := 0
export CHIP_HAS_I2C := 1
export CHIP_HAS_UART := 2
export CHIP_HAS_DMA := 2
export CHIP_HAS_SPDIF := 1
export CHIP_HAS_TRANSQ := 0
export CHIP_HAS_EXT_PMU := 0
export CHIP_HAS_AUDIO_CONST_ROM := 0
export CHIP_CACHE_VER := 2
export CHIP_FLASH_CTRL_VER := 2
export CHIP_SPI_VER := 4
else
$(error Invalid CHIP: $(CHIP))
endif

ifneq ($(CHIP_SUBTYPE),)
ifneq ($(SUBTYPE_VALID),1)
$(error Invalid CHIP_SUBTYPE=$(CHIP_SUBTYPE) for CHIP=$(CHIP))
endif
export CHIP_SUBTYPE
endif

ifeq ($(CPU),)
CPU := m4
endif
export CPU

ifneq ($(filter a%,$(CPU)),)
# Override lds file
LDS_FILE := armca.lds

ifeq ($(GEN_BOOT_SECTION),1)
CPPFLAGS_${LDS_FILE} += -DGEN_BOOT_SECTION
endif

ifeq ($(EXEC_IN_RAM),1)
CPPFLAGS_${LDS_FILE} += -DEXEC_IN_RAM
else ifeq ($(EXEC_IN_PSRAM),1)
CPPFLAGS_${LDS_FILE} += -DEXEC_IN_PSRAM
endif
endif

export FLASH_SIZE ?= 0x100000
ifeq ($(CHIP_HAS_PSRAM),1)
export PSRAM_SIZE ?= 0x400000
endif
ifeq ($(CHIP_HAS_PSRAMUHS),1)
export PSRAMUHS_SIZE ?= 0x800000
endif

KBUILD_CPPFLAGS += -DCHIP_HAS_UART=$(CHIP_HAS_UART)
KBUILD_CPPFLAGS += -DCHIP_HAS_I2C=$(CHIP_HAS_I2C)

ifeq ($(CHIP_HAS_USB),1)
KBUILD_CPPFLAGS += -DCHIP_HAS_USB
endif

ifneq ($(CHIP_HAS_TRANSQ),0)
KBUILD_CPPFLAGS += -DCHIP_HAS_TRANSQ=$(CHIP_HAS_TRANSQ)
endif

ifeq ($(CHIP_HAS_CP),1)
KBUILD_CPPFLAGS += -DCHIP_HAS_CP
endif

ifeq ($(CHIP_HAS_AUDIO_CONST_ROM),1)
KBUILD_CPPFLAGS += -DCHIP_HAS_AUDIO_CONST_ROM
endif

ifeq ($(CHIP_HAS_ANC_HW_GAIN_SMOOTHING),1)
KBUILD_CPPFLAGS += -DANC_HW_GAIN_SMOOTHING
endif

ifeq ($(CORE_SLEEP_POWER_DOWN),1)
KBUILD_CPPFLAGS += -DCORE_SLEEP_POWER_DOWN
endif

ifeq ($(USB_AUDIO_APP),1)
ifneq ($(BTUSB_AUDIO_MODE),1)
NO_OVERLAY ?= 1
endif
endif
export NO_OVERLAY
ifeq ($(NO_OVERLAY),1)
KBUILD_CPPFLAGS +=  -DNO_OVERLAY
endif

ifneq ($(ROM_SIZE),)
KBUILD_CPPFLAGS += -DROM_SIZE=$(ROM_SIZE)
endif

ifneq ($(RAM_SIZE),)
KBUILD_CPPFLAGS += -DRAM_SIZE=$(RAM_SIZE)
endif

# NOTE: This size cannot be changed so that audio section address is fixed.
#       This rule can be removed once audio tool can set audio section address dynamically.
FACTORY_SECTION_SIZE ?= 0x1000

# NOTE: This size cannot be changed so that audio section address is fixed.
#       This rule can be removed once audio tool can set audio section address dynamically.
RESERVED_SECTION_SIZE ?= 0x1000

# depend on length of (ANC + AUDIO + SPEECH) in aud_section.c
ifeq ($(AUDIO_SECTION_ENABLE),1)
KBUILD_CPPFLAGS += -DAUDIO_SECTION_ENABLE
AUD_EQ_SECTION_SIZE ?= 0x6000
AUD_SPEECH_SECTION_SIZE ?= 0x2000
ifeq ($(ANC_APP),1)
AUD_ANC_SECTION_SIZE ?= 0x10000
AUD_SECTION_SIZE ?= 0x18000
else
AUD_ANC_SECTION_SIZE ?= 0
AUD_SECTION_SIZE ?= 0x8000
endif
else
AUD_EQ_SECTION_SIZE ?= 0
AUD_SPEECH_SECTION_SIZE ?= 0
ifeq ($(ANC_APP),1)
AUD_ANC_SECTION_SIZE ?= 0x10000
AUD_SECTION_SIZE ?= 0x10000
else
AUD_ANC_SECTION_SIZE ?= 0
AUD_SECTION_SIZE ?= 0
endif
endif

KBUILD_CPPFLAGS += -DAUD_EQ_SECTION_SIZE=$(AUD_EQ_SECTION_SIZE)
KBUILD_CPPFLAGS += -DAUD_SPEECH_SECTION_SIZE=$(AUD_SPEECH_SECTION_SIZE)
KBUILD_CPPFLAGS += -DAUD_ANC_SECTION_SIZE=$(AUD_ANC_SECTION_SIZE)

USERDATA_SECTION_SIZE ?= 0x1000

CUSTOM_PARAMETER_SECTION_SIZE ?= 0x1000

ifeq ($(DUMP_CRASH_LOG),1)
CRASH_DUMP_SECTION_SIZE ?= 0x4000
KBUILD_CPPFLAGS += -DDUMP_CRASH_ENABLE
else
CRASH_DUMP_SECTION_SIZE ?= 0
endif

export DUMP_NORMAL_LOG ?= 0
ifeq ($(DUMP_NORMAL_LOG),1)
ifeq ($(FLASH_SIZE),0x40000) # 2M bits
LOG_DUMP_SECTION_SIZE ?= 0x4000
endif
ifeq ($(FLASH_SIZE),0x80000) # 4M bits
LOG_DUMP_SECTION_SIZE ?= 0x8000
endif
ifeq ($(FLASH_SIZE),0x100000) # 8M bits
LOG_DUMP_SECTION_SIZE ?= 0x10000
endif
ifeq ($(FLASH_SIZE),0x200000) # 16M bits
LOG_DUMP_SECTION_SIZE ?= 0x8000 #32k
endif
ifeq ($(FLASH_SIZE),0x400000) # 32M bits
LOG_DUMP_SECTION_SIZE ?= 0x200000
endif
ifeq ($(FLASH_SIZE),0x800000) # 64M bits
LOG_DUMP_SECTION_SIZE ?= 0x400000
endif
KBUILD_CPPFLAGS += -DDUMP_LOG_ENABLE
else
LOG_DUMP_SECTION_SIZE ?= 0
endif

export CORE_DUMP_TO_FLASH ?= 0
ifeq ($(CORE_DUMP_TO_FLASH),1)
CORE_DUMP_SECTION_SIZE ?= 0x100000
KBUILD_CPPFLAGS += -DCORE_DUMP_TO_FLASH
else
CORE_DUMP_SECTION_SIZE ?= 0
endif


ifeq ($(OTA_BASIC),1)
OTA_UPGRADE_LOG_SIZE ?= 0x1000
else
OTA_UPGRADE_LOG_SIZE ?= 0
endif

ifeq ($(PROMPT_IN_FLASH),1)
# 200K for prompt package
export PROMPT_IN_FLASH
KBUILD_CPPFLAGS += -DPROMPT_IN_FLASH

PROMPT_SECTION_SIZE ?= 0x28000
else
PROMPT_SECTION_SIZE ?= 0x0
endif

export LDS_SECTION_FLAGS := \
	-DPROMPT_SECTION_SIZE=$(PROMPT_SECTION_SIZE) \
	-DOTA_UPGRADE_LOG_SIZE=$(OTA_UPGRADE_LOG_SIZE) \
	-DLOG_DUMP_SECTION_SIZE=$(LOG_DUMP_SECTION_SIZE) \
	-DCRASH_DUMP_SECTION_SIZE=$(CRASH_DUMP_SECTION_SIZE) \
	-DCORE_DUMP_SECTION_SIZE=$(CORE_DUMP_SECTION_SIZE) \
	-DCUSTOM_PARAMETER_SECTION_SIZE=$(CUSTOM_PARAMETER_SECTION_SIZE) \
	-DAUD_SECTION_SIZE=$(AUD_SECTION_SIZE) \
	-DUSERDATA_SECTION_SIZE=$(USERDATA_SECTION_SIZE) \
	-DFACTORY_SECTION_SIZE=$(FACTORY_SECTION_SIZE) \

ifeq ($(QIOT_DATAPATH_ENABLE),1)
LDS_SECTION_FLAGS += \
	-DQIOT_OTA_SECTION_SIZE=$(QIOT_OTA_SECTION_SIZE) \
	-DQIOT_CORE_SECTION_SIZE=$(QIOT_CORE_SECTION_SIZE) 
endif

CPPFLAGS_${LDS_FILE} += \
	-DLINKER_SCRIPT \
	-DBUILD_INFO_MAGIC=0xBE57341D \
	-DFLASH_SIZE=$(FLASH_SIZE) \
	-Iplatform/hal

CPPFLAGS_${LDS_FILE} += $(LDS_SECTION_FLAGS)

ifneq ($(PSRAM_SIZE),)
CPPFLAGS_${LDS_FILE} +=-DPSRAM_SIZE=$(PSRAM_SIZE)
endif

ifneq ($(PSRAMUHS_SIZE),)
CPPFLAGS_${LDS_FILE} +=-DPSRAMUHS_SIZE=$(PSRAMUHS_SIZE)
endif

ifneq ($(OTA_BOOT_SIZE),)
export OTA_BOOT_SIZE
export OTA_BOOT_OFFSET ?= 0
CPPFLAGS_${LDS_FILE} += -DOTA_BOOT_OFFSET=$(OTA_BOOT_OFFSET) -DOTA_BOOT_SIZE=$(OTA_BOOT_SIZE)
endif

ifneq ($(OTA_CODE_OFFSET),)
export OTA_CODE_OFFSET
CPPFLAGS_${LDS_FILE} += -DOTA_CODE_OFFSET=$(OTA_CODE_OFFSET)
endif

ifneq ($(OTA_REMAP_OFFSET),)
export OTA_REMAP_OFFSET
CPPFLAGS_${LDS_FILE} += -DOTA_REMAP_OFFSET=$(OTA_REMAP_OFFSET)
ifeq ($(OTA_CODE_OFFSET),)
$(error OTA_CODE_OFFSET should be set along with OTA_REMAP_OFFSET)
endif
endif
ifneq ($(FLASH_REGION_SIZE),)
CPPFLAGS_${LDS_FILE} += -DFLASH_REGION_SIZE=$(FLASH_REGION_SIZE)
endif

ifneq ($(SLAVE_BIN_FLASH_OFFSET),)
export SLAVE_BIN_FLASH_OFFSET
CPPFLAGS_${LDS_FILE} += -DSLAVE_BIN_FLASH_OFFSET=$(SLAVE_BIN_FLASH_OFFSET)
endif

ifeq ($(BOOT_CODE_IN_RAM),1)
CPPFLAGS_${LDS_FILE} += -DBOOT_CODE_IN_RAM
endif

ifeq ($(TWS),1)
LARGE_RAM ?= 1
endif
ifeq ($(LARGE_RAM),1)
ifeq ($(DSP_ENABLE), 1)
$(error LARGE_RAM conflicts with DSP_ENABLE)
endif
KBUILD_CPPFLAGS += -DLARGE_RAM
endif

ifneq ($(SECURE_BOOT_VER),)
KBUILD_CPPFLAGS += -DSECURE_BOOT_VER=$(SECURE_BOOT_VER)
endif

ifeq ($(CHIP_HAS_EXT_PMU),1)
export PMU_IRQ_UNIFIED ?= 1
endif

# -------------------------------------------
# Standard C library
# -------------------------------------------
export NUTTX_BUILD ?= 0

export NOSTD
export LIBC_ROM

ifeq ($(NOSTD),1)

ifeq ($(MBED),1)
$(error Invalid configuration: MBED needs standard C library support)
endif
ifeq ($(RTOS),1)
ifneq ($(NO_LIBC),1)
$(error Invalid configuration: RTOS needs standard C library support)
endif
endif

ifneq ($(NO_LIBC),1)
core-y += utils/libc/
KBUILD_CPPFLAGS += -Iutils/libc/inc
endif

SPECS_CFLAGS :=

LIB_LDFLAGS := $(filter-out -lstdc++ -lsupc++ -lm -lc -lgcc -lnosys,$(LIB_LDFLAGS))

KBUILD_CPPFLAGS += -ffreestanding
KBUILD_CPPFLAGS += -nostdinc
CFLAGS_IMAGE += -nostdlib

KBUILD_CPPFLAGS += -DNOSTD

else # NOSTD != 1

ifneq ($(filter 1,$(LIBC_ROM) $(LIBC_OVERRIDE)),)
core-y += utils/libc/
endif

SPECS_CFLAGS := --specs=nano.specs
LIB_LDFLAGS += -lm -lc -lgcc -lnosys

endif # NOSTD != 1

# -------------------------------------------
# RTOS library
# -------------------------------------------

export RTOS

ifeq ($(RTOS),1)

ifeq ($(CPU),m4)
KERNEL ?= RTX
else
KERNEL ?= RTX5
ifeq ($(KERNEL),RTX)
$(error RTX doesn't support $(CPU))
endif
endif

export KERNEL

VALID_KERNEL_LIST := RTX RTX5 FREERTOS

ifeq ($(filter $(VALID_KERNEL_LIST),$(KERNEL)),)
$(error Bad KERNEL=$(KERNEL). Valid values are: $(VALID_KERNEL_LIST))
endif

core-y += rtos/

KBUILD_CPPFLAGS += -DRTOS
KBUILD_CPPFLAGS += -DKERNEL_$(KERNEL)

ifeq ($(KERNEL),RTX)
KBUILD_CPPFLAGS += \
	-Iinclude/rtos/rtx/
KBUILD_CPPFLAGS += -D__RTX_CPU_STATISTICS__=1
#KBUILD_CPPFLAGS += -DTASK_HUNG_CHECK_ENABLED=1
else ifeq ($(KERNEL),RTX5)
OS_IDLESTKSIZE ?= 1024
KBUILD_CPPFLAGS += \
	-Iinclude/rtos/rtx5/
KBUILD_CPPFLAGS += -D__RTX_CPU_STATISTICS__=1
#KBUILD_CPPFLAGS += -DTASK_HUNG_CHECK_ENABLED=1
else #!rtx
ifeq ($(KERNEL),FREERTOS)
KBUILD_CPPFLAGS += \
    -Iinclude/rtos/freertos/
endif #freertos
endif #rtx

ifeq ($(BLE_SECURITY_ENABLED), 1)
KBUILD_CPPFLAGS += -DCFG_APP_SEC
endif

export BLE_PERIPHERAL_ONLY ?= 0
ifeq ($(BLE_PERIPHERAL_ONLY),1)
KBUILD_CPPFLAGS += -D__BLE_PERIPHERAL_ONLY__
endif

export RFCOMM_SDP_UUID_ENABLE ?= 0
ifeq ($(RFCOMM_SDP_UUID_ENABLE),1)
KBUILD_CPPFLAGS += -D__RFCOMM_SDP_UUID_ENABLE__
endif

ifeq ($(TWS),1)
OS_TASKCNT ?= 12
OS_SCHEDULERSTKSIZE ?= 768
OS_IDLESTKSIZE ?= 512
else
OS_TASKCNT ?= 20
OS_SCHEDULERSTKSIZE ?= 512
OS_IDLESTKSIZE ?= 256
endif

ifeq ($(CPU),m33)
OS_CLOCK_NOMINAL ?= 16000
else
OS_CLOCK_NOMINAL ?= 32000
endif
OS_FIFOSZ ?= 24

export OS_TASKCNT
export OS_SCHEDULERSTKSIZE
export OS_IDLESTKSIZE
export OS_CLOCK_NOMINAL
export OS_FIFOSZ

endif

# -------------------------------------------
# MBED library
# -------------------------------------------

export MBED

ifeq ($(MBED),1)

core-y += mbed/

KBUILD_CPPFLAGS += -DMBED

KBUILD_CPPFLAGS += \
	-Imbed/api \
	-Imbed/common \

endif

# -------------------------------------------
# DEBUG functions
# -------------------------------------------

export DEBUG

ifeq ($(CHIP),best1400)
OPT_LEVEL ?= s
endif

ifneq ($(OPT_LEVEL),)
KBUILD_CFLAGS	+= -O$(OPT_LEVEL)
else
KBUILD_CFLAGS	+= -O2
endif

ifeq ($(DEBUG),1)

KBUILD_CPPFLAGS	+= -DDEBUG

STACK_PROTECTOR ?= 1
ifeq ($(STACK_PROTECTOR),1)
KBUILD_CFLAGS  	+= -fstack-protector-strong
endif

else

KBUILD_CPPFLAGS	+= -DNDEBUG

REL_TRACE_ENABLE ?= 1
ifeq ($(REL_TRACE_ENABLE),1)
KBUILD_CPPFLAGS	+= -DREL_TRACE_ENABLE
endif

endif

ifeq ($(MERGE_CONST),1)
KBUILD_CPPFLAGS += -fmerge-constants -fmerge-all-constants
endif

export CORE_DUMP ?= 0
ifeq ($(CORE_DUMP),1)
core-y += utils/crash_catcher/ utils/xyzmodem/
endif

# -------------------------------------------
# SIMU functions
# -------------------------------------------

export SIMU

ifeq ($(SIMU),1)

KBUILD_CPPFLAGS += -DSIMU

endif

# -------------------------------------------
# FPGA functions
# -------------------------------------------

export FPGA

ifeq ($(FPGA),1)

KBUILD_CPPFLAGS += -DFPGA

endif

# -------------------------------------------
# ROM_BUILD functions
# -------------------------------------------

export ROM_BUILD

ifeq ($(ROM_BUILD),1)

KBUILD_CPPFLAGS += -DROM_BUILD

endif

# Limit the length of REVISION_INFO if ROM_BUILD or using rom.lds
ifneq ($(filter 1,$(ROM_BUILD))$(filter rom.lds,$(LDS_FILE)),)
ifeq ($(CHIP),best1000)
REVISION_INFO := x
else
REVISION_INFO := $(GIT_REVISION)
endif
endif

# -------------------------------------------
# PROGRAMMER functions
# -------------------------------------------

export PROGRAMMER

ifeq ($(PROGRAMMER),1)

KBUILD_CPPFLAGS += -DPROGRAMMER

endif

# -------------------------------------------
# ROM_UTILS functions
# -------------------------------------------

export ROM_UTILS_ON ?= 0
ifeq ($(ROM_UTILS_ON),1)
KBUILD_CPPFLAGS += -DROM_UTILS_ON
core-y += utils/rom_utils/
endif

# -------------------------------------------
# Features
# -------------------------------------------

export DEBUG_PORT ?= 1

ifneq ($(filter best1000 best2000,$(CHIP)),)
export AUD_SECTION_STRUCT_VERSION ?= 1
else
export AUD_SECTION_STRUCT_VERSION ?= 2
endif

ifneq ($(AUD_SECTION_STRUCT_VERSION),)
KBUILD_CPPFLAGS += -DAUD_SECTION_STRUCT_VERSION=$(AUD_SECTION_STRUCT_VERSION)
endif

ifneq ($(FLASH_CHIP),)
VALID_FLASH_CHIP_LIST := ALL SIMU \
    GD25LF16E \
	GD25LE255E GD25LE128E GD25LQ64C GD25LQ32C GD25LQ16C GD25LQ80C GD25Q32C GD25Q80C GD25Q40C GD25D20C \
	P25Q256L P25Q128L P25Q64L P25Q32L P25Q16L P25Q80H P25Q40H P25Q21H P25Q32SL P25Q16SL \
	ZB25VQ128B \
	XM25QU128C XM25QH16C XM25QH80B \
	XT25Q08B \
	EN25S80B \
	W25Q128JW W25Q32FW \
	SK25LE032 P25Q128H
export FLASH_CHIP := $(subst $(comma),$(space),$(FLASH_CHIP))
ifneq ($(filter-out $(VALID_FLASH_CHIP_LIST),$(FLASH_CHIP)),)
$(error Invalid FLASH_CHIP: $(filter-out $(VALID_FLASH_CHIP_LIST),$(FLASH_CHIP)))
endif
endif

NV_REC_DEV_VER ?= 2

export NO_SLEEP ?= 0

export FAULT_DUMP ?= 1

export USE_TRACE_ID ?= 0
ifeq ($(USE_TRACE_ID),1)
export TRACE_STR_SECTION ?= 1
endif

export CRASH_BOOT ?= 0

export OSC_26M_X4_AUD2BB ?= 0
ifeq ($(OSC_26M_X4_AUD2BB),1)
export ANA_26M_X4_ENABLE ?= 1
export FLASH_LOW_SPEED ?= 0
endif

export CODEC_POWER_DOWN ?= 1

export AUDIO_CODEC_ASYNC_CLOSE ?= 0

# Enable the workaround for BEST1000 version C & earlier chips
export CODEC_PLAY_BEFORE_CAPTURE ?= 0

export AUDIO_INPUT_CAPLESSMODE ?= 0

export AUDIO_INPUT_LARGEGAIN ?= 0

export AUDIO_INPUT_MONO ?= 0

export AUDIO_OUTPUT_MONO ?= 0

export AUDIO_OUTPUT_VOLUME_DEFAULT ?= 10

KBUILD_CPPFLAGS += -DAUDIO_OUTPUT_VOLUME_DEFAULT=$(AUDIO_OUTPUT_VOLUME_DEFAULT)

export AUDIO_OUTPUT_INVERT_RIGHT_CHANNEL ?= 0

export AUDIO_OUTPUT_CALIB_GAIN_MISSMATCH ?= 0

ifeq ($(USB_AUDIO_APP),1)
export CODEC_HIGH_QUALITY ?= 1
endif

ifeq ($(ANC_APP),1)
export CODEC_HIGH_QUALITY ?= 1
endif

export ANALOG_MAX_DRE_GAIN_VAL ?= 0
ifeq ($(CODEC_HIGH_QUALITY),1)
export ANALOG_MAX_DRE_GAIN_VAL := 0x11
else
export ANALOG_MAX_DRE_GAIN_VAL := 0x11
endif

ifeq ($(DAC_SDM_GAIN_ENABLE),1)
ifneq ($(ANALOG_MAX_DRE_GAIN_VAL),0xf)
$(error Invalid Config! DAC_SDM_GAIN_ENABLE = $(DAC_SDM_GAIN_ENABLE) ANALOG_MAX_DRE_GAIN_VAL =$(ANALOG_MAX_DRE_GAIN_VAL))
endif
endif

ifeq ($(CHIP),best1000)
AUDIO_OUTPUT_DIFF ?= 1
AUDIO_OUTPUT_DC_CALIB ?= $(AUDIO_OUTPUT_DIFF)
AUDIO_OUTPUT_SMALL_GAIN_ATTN ?= 1
export AUDIO_OUTPUT_SW_GAIN ?= 1
ANC_L_R_MISALIGN_WORKAROUND ?= 1
else ifeq ($(CHIP),best2000)
ifeq ($(CODEC_HIGH_QUALITY),1)
export VCODEC_VOLT ?= 2.5V
else
export VCODEC_VOLT ?= 1.6V
endif
AUDIO_OUTPUT_DIFF ?= 0
ifeq ($(VCODEC_VOLT),2.5V)
AUDIO_OUTPUT_DC_CALIB ?= 0
AUDIO_OUTPUT_DC_CALIB_ANA ?= 1
else
AUDIO_OUTPUT_DC_CALIB ?= 1
AUDIO_OUTPUT_DC_CALIB_ANA ?= 0
endif
ifneq ($(AUDIO_OUTPUT_DIFF),1)
# Class-G module still needs improving
#DAC_CLASSG_ENABLE ?= 1
endif
else ifneq ($(filter best3001 best3003 best3005,$(CHIP)),)
export VCODEC_VOLT ?= 2.5V
AUDIO_OUTPUT_DC_CALIB ?= 1
AUDIO_OUTPUT_DC_CALIB_ANA ?= 0
else
AUDIO_OUTPUT_DC_CALIB ?= 0
AUDIO_OUTPUT_DC_CALIB_ANA ?= 1
endif

ifeq ($(AUDIO_OUTPUT_DC_CALIB)-$(AUDIO_OUTPUT_DC_CALIB_ANA),1-1)
$(error AUDIO_OUTPUT_DC_CALIB and AUDIO_OUTPUT_DC_CALIB_ANA cannot be enabled at the same time)
endif

export AUDIO_OUTPUT_DIFF

export AUDIO_OUTPUT_DC_CALIB

export AUDIO_OUTPUT_DC_CALIB_ANA

export AUDIO_OUTPUT_SMALL_GAIN_ATTN

export AUDIO_OUTPUT_SW_GAIN

export ANC_L_R_MISALIGN_WORKAROUND

export DAC_CLASSG_ENABLE

export AF_DEVICE_I2S ?= 0

export AF_ADC_I2S_SYNC ?= 0
ifeq ($(AF_ADC_I2S_SYNC),1)
KBUILD_CPPFLAGS += -DAF_ADC_I2S_SYNC
export AF_DEVICE_I2S = 1
export INT_LOCK_EXCEPTION ?= 1
endif

export BONE_SENSOR_TDM ?= 0
ifeq ($(BONE_SENSOR_TDM),1)
KBUILD_CPPFLAGS += -DBONE_SENSOR_TDM
KBUILD_CPPFLAGS += -DI2C_TASK_MODE
export AF_DEVICE_I2S = 1

ifeq ($(AF_DEVICE_I2S),1)
KBUILD_CPPFLAGS += -DI2S_MCLK_FROM_SPDIF
KBUILD_CPPFLAGS += -DI2S_MCLK_IOMUX_INDEX=13
KBUILD_CPPFLAGS += -DCLKOUT_IOMUX_INDEX=13
endif
endif

ifeq ($(ANC_APP),1)
export ANC_FF_ENABLED ?= 1
ifeq ($(ANC_FB_CHECK),1)
KBUILD_CPPFLAGS += -DANC_FB_CHECK
endif
ifeq ($(ANC_FF_CHECK),1)
KBUILD_CPPFLAGS += -DANC_FF_CHECK
endif
endif

ifeq ($(CHIP),best1400)
export AUDIO_RESAMPLE ?= 1
else
export AUDIO_RESAMPLE ?= 0
endif

ifeq ($(AUDIO_RESAMPLE),1)
ifeq ($(CHIP),best1000)
export SW_PLAYBACK_RESAMPLE ?= 1
export SW_CAPTURE_RESAMPLE ?= 1
export NO_SCO_RESAMPLE ?= 1
endif # CHIP is best1000
ifeq ($(CHIP),best2000)
export SW_CAPTURE_RESAMPLE ?= 1
export SW_SCO_RESAMPLE ?= 1
export NO_SCO_RESAMPLE ?= 0
endif # CHIP is best2000
ifeq ($(BT_ANC),1)
ifeq ($(NO_SCO_RESAMPLE),1)
$(error BT_ANC and NO_SCO_RESAMPLE cannot be enabled at the same time)
endif
endif # BT_ANC
endif # AUDIO_RESAMPLE

export HW_FIR_DSD_PROCESS ?= 0

export HW_FIR_EQ_PROCESS ?= 0

export SW_IIR_EQ_PROCESS ?= 0

export HW_IIR_EQ_PROCESS ?= 0

export HW_DAC_IIR_EQ_PROCESS ?= 0

export AUDIO_DRC ?= 0

export AUDIO_LIMITER ?= 0

ifeq ($(AUDIO_DYNAMIC_BOOST),1)
export AUDIO_DYNAMIC_BOOST
KBUILD_CPPFLAGS += -DAUDIO_DYNAMIC_BOOST
export AUDIO_OUTPUT_SW_GAIN := 1
export AUDIO_OUTPUT_SW_GAIN_BEFORE_DRC := 1
endif

export HW_DC_FILTER_WITH_IIR ?= 0
ifeq ($(HW_DC_FILTER_WITH_IIR),1)
KBUILD_CPPFLAGS += -DHW_DC_FILTER_WITH_IIR
export HW_FILTER_CODEC_IIR ?= 1
endif

ifeq ($(USB_AUDIO_APP),1)
export ANDROID_ACCESSORY_SPEC ?= 1
export FIXED_CODEC_ADC_VOL ?= 0

ifneq ($(BTUSB_AUDIO_MODE),1)
NO_PWRKEY ?= 1
NO_GROUPKEY ?= 1
endif
endif

export NO_PWRKEY

export NO_GROUPKEY

ifneq ($(CHIP),best1000)
ifneq ($(CHIP)-$(TWS),best2000-1)
# For bt
export A2DP_EQ_24BIT ?= 1
# For usb audio
export AUDIO_PLAYBACK_24BIT ?= 1
endif
endif

ifeq ($(CHIP),best1000)

ifeq ($(AUD_PLL_DOUBLE),1)
KBUILD_CPPFLAGS += -DAUD_PLL_DOUBLE
endif

ifeq ($(DUAL_AUX_MIC),1)
ifeq ($(AUDIO_INPUT_MONO),1)
$(error Invalid talk mic configuration)
endif
KBUILD_CPPFLAGS += -D_DUAL_AUX_MIC_
endif

endif # best1000

ifeq ($(CAPTURE_ANC_DATA),1)
KBUILD_CPPFLAGS += -DCAPTURE_ANC_DATA
endif

ifeq ($(AUDIO_ANC_TT_HW),1)
KBUILD_CPPFLAGS += -DAUDIO_ANC_TT_HW
endif

ifeq ($(AUDIO_ANC_FB_MC_HW),1)
KBUILD_CPPFLAGS += -DAUDIO_ANC_FB_MC_HW
endif

ifeq ($(AUDIO_ANC_FB_MC),1)
ifeq ($(AUDIO_RESAMPLE),1)
$(error AUDIO_ANC_FB_MC conflicts with AUDIO_RESAMPLE)
endif
KBUILD_CPPFLAGS += -DAUDIO_ANC_FB_MC
endif

ifeq ($(BT_ANC),1)
KBUILD_CPPFLAGS += -D__BT_ANC__
endif

export ANC_NOISE_TRACKER ?= 0

ifeq ($(IBRT),1)
KBUILD_CPPFLAGS += -DANC_NOISE_TRACKER_CHANNEL_NUM=1
else
KBUILD_CPPFLAGS += -DANC_NOISE_TRACKER_CHANNEL_NUM=2
endif

export BTUSB_AUDIO_MODE ?= 0
ifeq ($(BTUSB_AUDIO_MODE),1)
KBUILD_CPPFLAGS += -DBTUSB_AUDIO_MODE
endif


ifeq ($(AUDIO_DEBUG),1)
export AUDIO_DEBUG
KBUILD_CPPFLAGS += -DAUDIO_DEBUG
KBUILD_CPPFLAGS += -DAUDIO_DEBUG_V0_1_0
# KBUILD_CPPFLAGS += -DSPEECH_RX_PLC_DUMP_DATA
endif


export BT_USB_AUDIO_DUAL_MODE ?= 0
ifeq ($(BT_USB_AUDIO_DUAL_MODE),1)
KBUILD_CPPFLAGS += -DBT_USB_AUDIO_DUAL_MODE
endif

ifeq ($(WATCHER_DOG),1)
KBUILD_CPPFLAGS += -D__WATCHER_DOG_RESET__
endif

ifeq ($(ANC_ASSIST_ENABLED),1)
KBUILD_CPPFLAGS += -DANC_ASSIST_ENABLED
KBUILD_CPPFLAGS += -DANC_ASSIST_USE_PILOT
KBUILD_CPPFLAGS += -DGLOBAL_SRAM_CMSIS_FFT
#KBUILD_CPPFLAGS += -DANC_ASSIST_USE_PILOT
#KBUILD_CPPFLAGS += -DANC_ASSIST_USE_INT_CODEC_PLAYBACK
export SPEECH_TX_24BIT := 1
export AUDIO_OUTPUT_SW_GAIN := 1
ifeq ($(CHIP),best2300p)
export ANC_ASSIST_USE_INT_CODEC := 1
export PLAYBACK_FORCE_48K := 1
export SPEECH_RX_24BIT := 1
export AF_STREAM_ID_0_PLAYBACK_FADEOUT := 1
endif

ifeq ($(CHIP),best1305)
KBUILD_CPPFLAGS += -DASSIST_LOW_RAM_MOD
endif

ifeq ($(VOICE_ASSIST_WD_ENABLED),1)
export VOICE_ASSIST_WD_ENABLED
export ANC_ASSIST_PILOT_TONE_ALWAYS_ON := 1
endif

ifeq ($(VOICE_ASSIST_PILOT_ANC_ENABLED),1)
export VOICE_ASSIST_PILOT_ANC_ENABLED
export ANC_ASSIST_PILOT_TONE_ALWAYS_ON := 1
endif

ifeq ($(VOICE_ASSIST_NOISE_ADAPT_ANC_ENABLED),1)
export VOICE_ASSIST_NOISE_ADAPT_ANC_ENABLED
endif
endif #ANC_ASSIST_ENABLED

export ULTRA_LOW_POWER ?= 0
ifeq ($(ULTRA_LOW_POWER),1)
export FLASH_LOW_SPEED ?= 1
export PSRAM_LOW_SPEED ?= 1
endif

export USB_HIGH_SPEED ?= 0
ifeq ($(CHIP),best2000)
ifeq ($(USB_HIGH_SPEED),1)
export AUDIO_USE_BBPLL ?= 1
endif
ifeq ($(AUDIO_USE_BBPLL),1)
ifeq ($(MCU_HIGH_PERFORMANCE_MODE),1)
$(error MCU_HIGH_PERFORMANCE_MODE conflicts with AUDIO_USE_BBPLL)
endif
else # !AUDIO_USE_BBPLL
ifeq ($(USB_HIGH_SPEED),1)
$(error AUDIO_USE_BBPLL must be used with USB_HIGH_SPEED)
endif
endif # !AUDIO_USE_BBPLL
endif # best2000

ifeq ($(SIMPLE_TASK_SWITCH),1)
KBUILD_CPPFLAGS += -DSIMPLE_TASK_SWITCH
endif

ifeq ($(ASSERT_SHOW_FILE_FUNC),1)
KBUILD_CPPFLAGS += -DASSERT_SHOW_FILE_FUNC
else
ifeq ($(ASSERT_SHOW_FILE),1)
KBUILD_CPPFLAGS += -DASSERT_SHOW_FILE
else
ifeq ($(ASSERT_SHOW_FUNC),1)
KBUILD_CPPFLAGS += -DASSERT_SHOW_FUNC
endif
endif
endif

ifeq ($(CALIB_SLOW_TIMER),1)
KBUILD_CPPFLAGS += -DCALIB_SLOW_TIMER
endif

ifeq ($(INT_LOCK_EXCEPTION),1)
KBUILD_CPPFLAGS += -DINT_LOCK_EXCEPTION
endif

export APP_ANC_TEST ?= 0
ifeq ($(APP_ANC_TEST),1)
KBUILD_CPPFLAGS += -DAPP_ANC_TEST
endif

ifeq ($(APP_TRACE_RX_ENABLE),1)
KBUILD_CPPFLAGS += -DAPP_TRACE_RX_ENABLE
endif

export USE_TRACE_ID ?= 0
ifeq ($(USE_TRACE_ID),1)
export TRACE_STR_SECTION ?= 1
KBUILD_CPPFLAGS += -DUSE_TRACE_ID
endif

ifeq ($(TRACE_STR_SECTION),1)
KBUILD_CPPFLAGS += -DTRACE_STR_SECTION
CPPFLAGS_${LDS_FILE} += -DTRACE_STR_SECTION
endif

USE_THIRDPARTY ?= 0
ifeq ($(USE_THIRDPARTY),1)
KBUILD_CPPFLAGS += -D__THIRDPARTY
core-y += thirdparty/
endif

export PC_CMD_UART ?= 0
ifeq ($(PC_CMD_UART),1)
KBUILD_CPPFLAGS += -D__PC_CMD_UART__
endif

ifeq ($(USB_ANC_MC_EQ_TUNING),1)
KBUILD_CPPFLAGS += -DUSB_ANC_MC_EQ_TUNING -DANC_PROD_TEST
endif

export AUTO_TEST ?= 0
ifeq ($(AUTO_TEST),1)
KBUILD_CFLAGS += -D_AUTO_TEST_
endif

ifeq ($(RB_CODEC),1)
CPPFLAGS_${LDS_FILE} += -DRB_CODEC
endif

ifneq ($(DATA_BUF_START),)
CPPFLAGS_${LDS_FILE} += -DDATA_BUF_START=$(DATA_BUF_START)
endif

ifeq ($(USER_SECURE_BOOT),1)
core-y += utils/user_secure_boot/ \
               utils/system_info/
endif

ifeq ($(MAX_DAC_OUTPUT),-60db)
MAX_DAC_OUTPUT_FLAGS := -DMAX_DAC_OUTPUT_M60DB
else
ifeq ($(MAX_DAC_OUTPUT),3.75mw)
MAX_DAC_OUTPUT_FLAGS := -DMAX_DAC_OUTPUT_3P75MW
else
ifeq ($(MAX_DAC_OUTPUT),5mw)
MAX_DAC_OUTPUT_FLAGS := -DMAX_DAC_OUTPUT_5MW
else
ifeq ($(MAX_DAC_OUTPUT),10mw)
MAX_DAC_OUTPUT_FLAGS := -DMAX_DAC_OUTPUT_10MW
else
ifneq ($(MAX_DAC_OUTPUT),30mw)
ifneq ($(MAX_DAC_OUTPUT),)
$(error Invalid MAX_DAC_OUTPUT value: $(MAX_DAC_OUTPUT) (MUST be one of: -60db 3.75mw 5mw 10mw 30mw))
endif
endif
endif
endif
endif
endif
export MAX_DAC_OUTPUT_FLAGS

# vvvvvvvvvvvvvvvvvvvvvvvvvvvvv
# BT features
# ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
export BT_IF_INCLUDES ?=
export BT_PROFILES_INCLUDES ?=
export ENHANCED_STACK ?= 0

export INTERSYS_NO_THREAD ?= 0

export INTERSYS_DEBUG ?= 0
ifeq ($(INTERSYS_DEBUG),1)
	KBUILD_CPPFLAGS += -DINTERSYS_DEBUG=1
endif

export BT_DEBUG_TPORTS ?= 0
ifneq ($(BT_DEBUG_TPORTS),0)
	KBUILD_CPPFLAGS += -D__BT_DEBUG_TPORTS__
endif

export CONTROLLER_DUMP_ENABLE ?= 0
ifeq ($(CONTROLLER_DUMP_ENABLE),1)
	KBUILD_CPPFLAGS += -DCONTROLLER_DUMP_ENABLE=1
endif

export SNOOP_DATA_EXCHANGE_VIA_BLE ?= 0
ifeq ($(SNOOP_DATA_EXCHANGE_VIA_BLE),1)
	KBUILD_CPPFLAGS += -DSNOOP_DATA_EXCHANGE_VIA_BLE
endif

export SYNC_BT_CTLR_PROFILE ?= 0
ifeq ($(SYNC_BT_CTLR_PROFILE),1)
	KBUILD_CPPFLAGS += -DSYNC_BT_CTLR_PROFILE
endif

export PROFILE_DEBUG ?= 0
ifeq ($(PROFILE_DEBUG),1)
	KBUILD_CPPFLAGS += -DXA_DEBUG=1
endif

ifeq ($(ENHANCED_STACK),1)
BT_IF_INCLUDES += \
	-Iservices/bt_if_enhanced/inc
BT_PROFILES_INCLUDES += \
	-Iservices/bt_profiles_enhanced/inc
else
BT_IF_INCLUDES += \
	-Iservices/bt_if/inc
BT_PROFILES_INCLUDES += \
	-Iservices/bt_profiles/inc \
	-Iservices/bt_profiles/inc/sys
endif

ifeq ($(ENHANCED_STACK),1)
KBUILD_CFLAGS += -DENHANCED_STACK
KBUILD_CPPFLAGS += -DNEW_PROFILE_BLOCKED -DUSE_HCIBUFF_TO_CACHE_RTXBUFF
#KBUILD_CPPFLAGS += -D__A2DP_AVDTP_CP__ -D__A2DP_AVDTP_DR__
#KBUILD_CPPFLAGS += -D__A2DP_AVDTP_DR__
KBUILD_CPPFLAGS += -D__BLE_TX_USE_BT_TX_QUEUE__
endif

ifeq ($(BT_APP),1)
ifneq ($(filter-out 2M 3M,$(BT_RF_PREFER)),)
$(error Invalid BT_RF_PREFER=$(BT_RF_PREFER))
endif
ifneq ($(BT_RF_PREFER),)
RF_PREFER := $(subst .,P,$(BT_RF_PREFER))
KBUILD_CPPFLAGS += -D__$(RF_PREFER)_PACK__
endif

export AUDIO_SCO_BTPCM_CHANNEL ?= 1
ifeq ($(AUDIO_SCO_BTPCM_CHANNEL),1)
KBUILD_CPPFLAGS += -D_SCO_BTPCM_CHANNEL_
endif

export BT_ONE_BRING_TWO ?= 0
ifeq ($(BT_ONE_BRING_TWO),1)
KBUILD_CPPFLAGS += -D__BT_ONE_BRING_TWO__
endif

export BT_USE_COHEAP_ALLOC ?= 1
ifeq ($(BT_USE_COHEAP_ALLOC),1)
KBUILD_CPPFLAGS += -DBT_USE_COHEAP_ALLOC
endif

export BT_DYNAMIC_ALLOC_HCI_RX_BUFF ?= 1
ifeq ($(BT_DYNAMIC_ALLOC_HCI_RX_BUFF),1)
KBUILD_CPPFLAGS += -DBT_DYNAMIC_ALLOC_HCI_RX_BUFF
endif

export A2DP_PLAYER_USE_BT_TRIGGER ?= 1
ifeq ($(A2DP_PLAYER_USE_BT_TRIGGER),1)
KBUILD_CPPFLAGS += -D__A2DP_PLAYER_USE_BT_TRIGGER__
endif

export BT_SELECT_PROF_DEVICE_ID ?= 0
ifeq ($(BT_ONE_BRING_TWO),1)
ifeq ($(BT_SELECT_PROF_DEVICE_ID),1)
KBUILD_CPPFLAGS += -D__BT_SELECT_PROF_DEVICE_ID__
endif
endif

export SBC_FUNC_IN_ROM ?= 0
ifeq ($(SBC_FUNC_IN_ROM),1)

KBUILD_CPPFLAGS += -D__SBC_FUNC_IN_ROM__

ifeq ($(CHIP),best2000)
UNALIGNED_ACCESS ?= 1
KBUILD_CPPFLAGS += -D__SBC_FUNC_IN_ROM_VBEST2000_ONLYSBC__
KBUILD_CPPFLAGS += -D__SBC_FUNC_IN_ROM_VBEST2000__
endif
endif

export HFP_1_6_ENABLE ?= 0
ifeq ($(HFP_1_6_ENABLE),1)
KBUILD_CPPFLAGS += -DHFP_1_6_ENABLE
endif

# Fix codec and vqe sample rate
ifeq ($(SPEECH_BONE_SENSOR),1)
export SPEECH_CODEC_FIXED_SAMPLE_RATE := 16000
export SPEECH_VQE_FIXED_SAMPLE_RATE := 16000
endif

ifeq ($(ANC_ASSIST_ENABLED),1)
export SPEECH_CODEC_FIXED_SAMPLE_RATE := 16000
export SPEECH_VQE_FIXED_SAMPLE_RATE := 16000
endif

ifeq ($(SPEECH_TX_NS5),1)
export SPEECH_CODEC_FIXED_SAMPLE_RATE := 16000
export SPEECH_VQE_FIXED_SAMPLE_RATE := 16000
endif

ifeq ($(SPEECH_TX_NS7),1)
export SPEECH_CODEC_FIXED_SAMPLE_RATE := 16000
export SPEECH_VQE_FIXED_SAMPLE_RATE := 16000
endif

ifeq ($(SPEECH_TX_DTLN),1)
export SPEECH_TX_DTLN
KBUILD_CPPFLAGS += -DSPEECH_TX_DTLN
export HFP_DISABLE_NREC := 1
export NN_LIB ?= 1
export DTLN_37K ?= 1
export DTLN_CP ?= 1
endif

ifeq ($(DTLN_37K),1)
export DTLN_37K
KBUILD_CPPFLAGS += -DDTLN_37K
# KBUILD_CPPFLAGS += -DSPEECH_TX_NS4
endif

ifeq ($(DTLN_CP),1)
export DTLN_CP
KBUILD_CPPFLAGS += -DDTLN_CP
endif

ifeq ($(SPEECH_TX_2MIC_NS7),1)
export SPEECH_CODEC_FIXED_SAMPLE_RATE := 16000
export SPEECH_VQE_FIXED_SAMPLE_RATE := 16000
endif

ifeq ($(AUDIO_ADAPTIVE_IIR_EQ),1)
export AUDIO_ADAPTIVE_IIR_EQ := 1
export HW_DAC_IIR_EQ_PROCESS := 1
endif

ifeq ($(AUDIO_ADAPTIVE_FIR_EQ),1)
export AUDIO_ADAPTIVE_FIR_EQ := 1
export HW_FIR_EQ_PROCESS := 1
endif

export SPEECH_CODEC_FIXED_SAMPLE_RATE ?= 0
ifneq ($(filter 8000 16000 48000,$(SPEECH_CODEC_FIXED_SAMPLE_RATE)),)
KBUILD_CPPFLAGS += -DSPEECH_CODEC_FIXED_SAMPLE_RATE=$(SPEECH_CODEC_FIXED_SAMPLE_RATE)
#export DSP_LIB ?= 1
endif

export SPEECH_VQE_FIXED_SAMPLE_RATE ?= 0
ifneq ($(filter 8000 16000,$(SPEECH_VQE_FIXED_SAMPLE_RATE)),)
#export DSP_LIB ?= 1
endif

export A2DP_AAC_ON ?= 0
ifeq ($(A2DP_AAC_ON),1)
KBUILD_CPPFLAGS += -DA2DP_AAC_ON
endif

export FDKAAC_VERSION ?= 2

ifneq ($(FDKAAC_VERSION),)
KBUILD_CPPFLAGS += -DFDKAAC_VERSION=$(FDKAAC_VERSION)
endif

export A2DP_LHDC_ON ?= 0
ifeq ($(A2DP_LHDC_ON),1)
KBUILD_CPPFLAGS += -DA2DP_LHDC_ON
export A2DP_LHDC_V3 ?= 0
ifeq ($(A2DP_LHDC_V3),1)
KBUILD_CPPFLAGS += -DA2DP_LHDC_V3
endif
core-y += thirdparty/audio_codec_lib/liblhdc-dec/
endif
ifeq ($(USER_SECURE_BOOT),1)
KBUILD_CPPFLAGS += -DUSER_SECURE_BOOT
endif

export A2DP_SCALABLE_ON ?= 0
ifeq ($(A2DP_SCALABLE_ON),1)
KBUILD_CPPFLAGS += -DA2DP_SCALABLE_ON
#KBUILD_CPPFLAGS += -DA2DP_SCALABLE_UHQ_SUPPORT
core-y += thirdparty/audio_codec_lib/scalable/
endif

export A2DP_LDAC_ON ?= 0
ifeq ($(A2DP_LDAC_ON),1)
KBUILD_CPPFLAGS += -DA2DP_LDAC_ON
core-y += thirdparty/audio_codec_lib/ldac/
endif

export A2DP_SBC_PLC_ENABLED ?= 0

export A2DP_CP_ACCEL ?= 0
ifeq ($(A2DP_CP_ACCEL),1)
KBUILD_CPPFLAGS += -DA2DP_CP_ACCEL
endif

export SCO_CP_ACCEL ?= 0
ifeq ($(SCO_CP_ACCEL),1)
KBUILD_CPPFLAGS += -DSCO_CP_ACCEL
# spx fft will share buffer which is not fit for dual cores.
KBUILD_CPPFLAGS += -DUSE_CMSIS_F32_FFT
endif

export SCO_TRACE_CP_ACCEL ?= 0
ifeq ($(SCO_TRACE_CP_ACCEL),1)
KBUILD_CPPFLAGS += -DSCO_TRACE_CP_ACCEL
endif

ifeq ($(BT_XTAL_SYNC),1)
KBUILD_CPPFLAGS += -DBT_XTAL_SYNC
KBUILD_CPPFLAGS += -DBT_XTAL_SYNC_NEW_METHOD
KBUILD_CPPFLAGS += -DFIXED_BIT_OFFSET_TARGET
endif

ifeq ($(BT_XTAL_SYNC_SLOW),1)
KBUILD_CPPFLAGS += -DBT_XTAL_SYNC_SLOW
endif

ifeq ($(BT_XTAL_SYNC_NO_RESET),1)
KBUILD_CPPFLAGS += -DBT_XTAL_SYNC_NO_RESET
endif


ifeq ($(APP_LINEIN_A2DP_SOURCE),1)
KBUILD_CPPFLAGS += -DAPP_LINEIN_A2DP_SOURCE
endif

ifeq ($(HSP_ENABLE),1)
KBUILD_CPPFLAGS += -D__HSP_ENABLE__
endif

ifeq ($(APP_I2S_A2DP_SOURCE),1)
KBUILD_CPPFLAGS += -DAPP_I2S_A2DP_SOURCE
endif

export TX_RX_PCM_MASK ?= 0
ifeq ($(TX_RX_PCM_MASK),1)
KBUILD_CPPFLAGS += -DTX_RX_PCM_MASK
endif

export PCM_PRIVATE_DATA_FLAG ?= 0
ifeq ($(PCM_PRIVATE_DATA_FLAG),1)
KBUILD_CPPFLAGS += -DPCM_PRIVATE_DATA_FLAG
endif

export PCM_FAST_MODE ?= 0
ifeq ($(PCM_FAST_MODE),1)
KBUILD_CPPFLAGS += -DPCM_FAST_MODE
endif

export LOW_DELAY_SCO ?= 0
ifeq ($(LOW_DELAY_SCO),1)
KBUILD_CPPFLAGS += -DLOW_DELAY_SCO
endif

export CVSD_BYPASS ?= 0
ifeq ($(CVSD_BYPASS),1)
KBUILD_CPPFLAGS += -DCVSD_BYPASS
endif

export SCO_DMA_SNAPSHOT ?= 0
ifeq ($(CHIP_HAS_SCO_DMA_SNAPSHOT),1)
export SCO_DMA_SNAPSHOT := 1
KBUILD_CPPFLAGS += -DSCO_DMA_SNAPSHOT
endif

export SCO_OPTIMIZE_FOR_RAM ?= 0
ifeq ($(SCO_OPTIMIZE_FOR_RAM),1)
KBUILD_CPPFLAGS += -DSCO_OPTIMIZE_FOR_RAM
endif

export AAC_TEXT_PARTIAL_IN_FLASH ?= 0
ifeq ($(AAC_TEXT_PARTIAL_IN_FLASH),1)
KBUILD_CPPFLAGS += -DAAC_TEXT_PARTIAL_IN_FLASH
endif

ifeq ($(SUPPORT_BATTERY_REPORT),1)
KBUILD_CPPFLAGS += -DSUPPORT_BATTERY_REPORT
endif

ifeq ($(SUPPORT_HF_INDICATORS),1)
KBUILD_CPPFLAGS += -DSUPPORT_HF_INDICATORS
endif

ifeq ($(SUPPORT_SIRI),1)
KBUILD_CPPFLAGS += -DSUPPORT_SIRI
endif

export BQB_PROFILE_TEST ?= 0
ifeq ($(BQB_PROFILE_TEST),1)
KBUILD_CPPFLAGS += -D__BQB_PROFILE_TEST__
endif

export AUDIO_SPECTRUM ?= 0
ifeq ($(AUDIO_SPECTRUM),1)
KBUILD_CPPFLAGS += -D__AUDIO_SPECTRUM__
endif

export VOICE_PROMPT ?= 0
ifeq ($(VOICE_PROMPT),1)
KBUILD_CPPFLAGS += -DMEDIA_PLAYER_SUPPORT
endif

export INTERACTION ?= 0
ifeq ($(INTERACTION),1)
KBUILD_CPPFLAGS += -D__INTERACTION__
endif

export INTERACTION_FASTPAIR ?= 0
ifeq ($(INTERACTION_FASTPAIR),1)
KBUILD_CPPFLAGS += -D__INTERACTION_FASTPAIR__
KBUILD_CPPFLAGS += -D__INTERACTION_CUSTOMER_AT_COMMAND__
endif

ifeq ($(SUSPEND_ANOTHER_DEV_A2DP_STREAMING_WHEN_CALL_IS_COMING),1)
KBUILD_CPPFLAGS += -DSUSPEND_ANOTHER_DEV_A2DP_STREAMING_WHEN_CALL_IS_COMING
endif

export TWS_PROMPT_SYNC ?= 0
ifeq ($(TWS_PROMPT_SYNC), 1)
export MIX_AUDIO_PROMPT_WITH_A2DP_MEDIA_ENABLED = 1
KBUILD_CPPFLAGS += -DTWS_PROMPT_SYNC
endif

export MIX_AUDIO_PROMPT_WITH_A2DP_MEDIA_ENABLED ?= 1
ifeq ($(MIX_AUDIO_PROMPT_WITH_A2DP_MEDIA_ENABLED), 1)
KBUILD_CPPFLAGS += -DMIX_AUDIO_PROMPT_WITH_A2DP_MEDIA_ENABLED
export RESAMPLE_ANY_SAMPLE_RATE ?= 1
export AUDIO_OUTPUT_SW_GAIN := 1
endif

export MEDIA_PLAY_24BIT ?= 1
export LBRT ?= 0
ifeq ($(LBRT),1)
KBUILD_CPPFLAGS += -DLBRT
endif

ifeq ($(IBRT),1)
KBUILD_CPPFLAGS += -DIBRT
KBUILD_CPPFLAGS += -DIBRT_BLOCKED
KBUILD_CPPFLAGS += -DIBRT_NOT_USE
KBUILD_CPPFLAGS += -D__A2DP_AUDIO_SYNC_FIX_DIFF_NOPID__
endif


export IBRT_TESTMODE ?= 0
ifeq ($(IBRT_TESTMODE),1)
KBUILD_CPPFLAGS += -D__IBRT_IBRT_TESTMODE__
endif

ifeq ($(TWS_SYSTEM_ENABLED),1)
KBUILD_CPPFLAGS += -DTWS_SYSTEM_ENABLED
endif

export BES_AUD ?= 0
ifeq ($(BES_AUD),1)
KBUILD_CPPFLAGS += -DBES_AUD
endif

export IBRT_SEARCH_UI ?= 0
ifeq ($(IBRT_SEARCH_UI),1)
KBUILD_CPPFLAGS += -DIBRT_SEARCH_UI
endif

export BT_AUTO_UI_TEST ?= 0
ifeq ($(BT_AUTO_UI_TEST),1)
	KBUILD_CPPFLAGS += -D__AUTO_UI_TEST__
endif
endif # BT_APP

ifeq ($(TOTA),1)
KBUILD_CPPFLAGS += -DTOTA
ifeq ($(BLE),1)
KBUILD_CPPFLAGS += -DBLE_TOTA_ENABLED
endif
KBUILD_CPPFLAGS += -DSHOW_RSSI
KBUILD_CPPFLAGS += -DTEST_OVER_THE_AIR_ENANBLED
export TEST_OVER_THE_AIR ?= 1
endif

ifeq ($(TOTA_v2),1)
KBUILD_CPPFLAGS += -DTOTA_v2=1
ifeq ($(BLE),1)
KBUILD_CPPFLAGS += -DBLE_TOTA_ENABLED
endif
KBUILD_CPPFLAGS += -DSHOW_RSSI
KBUILD_CPPFLAGS += -DTEST_OVER_THE_AIR_ENANBLED
export TEST_OVER_THE_AIR_v2 ?= 1
endif

export BTIF_DIP_DEVICE ?= 1
ifeq ($(BTIF_DIP_DEVICE),1)
KBUILD_CPPFLAGS += -DBTIF_DIP_DEVICE
endif
# vvvvvvvvvvvvvvvvvvvvvvvvvvvvv
# BLE features
# ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
export BLE ?= 0
ifeq ($(BLE),1)

KBUILD_CPPFLAGS += -D__IAG_BLE_INCLUDE__

export BLE_APP_DATAPATH_SERVER_ENABLED ?= 1
ifeq ($(BLE_APP_DATAPATH_SERVER_ENABLED),1)
KBUILD_CPPFLAGS += -DCFG_APP_DATAPATH_SERVER
endif

IS_USE_BLE_DUAL_CONNECTION ?= 1
ifeq ($(IS_USE_BLE_DUAL_CONNECTION),1)
KBUILD_CPPFLAGS += -DBLE_CONNECTION_MAX=2
else
KBUILD_CPPFLAGS += -DBLE_CONNECTION_MAX=1
endif

ifeq ($(IS_ENABLE_DEUGGING_MODE),1)
KBUILD_CPPFLAGS += -DIS_ENABLE_DEUGGING_MODE
endif

export SMARTVOICE ?= 0
ifeq ($(SMARTVOICE),1)
KBUILD_CPPFLAGS += -D__SMARTVOICE__
endif

IS_MULTI_AI_ENABLED ?= 0
ifeq ($(IS_MULTI_AI_ENABLED),1)
KBUILD_CPPFLAGS += -DIS_MULTI_AI_ENABLED
endif

export VOICE_DATAPATH_ENABLED ?= 0
ifeq ($(VOICE_DATAPATH_ENABLED),1)
KBUILD_CPPFLAGS += -DVOICE_DATAPATH
endif

ifeq ($(MIX_MIC_DURING_MUSIC_ENABLED),1)
KBUILD_CPPFLAGS += -DMIX_MIC_DURING_MUSIC
endif

## Microsoft swift pair
export SWIFT_ENABLE ?= 0
ifeq ($(SWIFT_ENABLE),1)
KBUILD_CPPFLAGS += -DSWIFT_ENABLED
endif # SWIFT


endif # BLE

# vvvvvvvvvvvvvvvvvvvvvvvvvvvvv
# Speech features
# ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
export SPEECH_TX_24BIT ?= 0
ifeq ($(SPEECH_TX_24BIT),1)
KBUILD_CPPFLAGS += -DSPEECH_TX_24BIT
endif

export SPEECH_TX_THIRDPARTY_SNDP ?= 0
ifeq ($(SPEECH_TX_THIRDPARTY_SNDP),1)
# KBUILD_CPPFLAGS += -DSNDP_TX_2MIC_ENABLE
# KBUILD_CPPFLAGS += -DSNDP_TX_2MIC_DUMP_ENABLE
export SPEECH_CODEC_CAPTURE_CHANNEL_NUM = 2
export SPEECH_TX_AEC_CODEC_REF := 1
export SPEECH_PROCESS_FRAME_MS  := 15
KBUILD_CPPFLAGS += -DHFP_DISABLE_NREC

KBUILD_CPPFLAGS += -DSNDP_TX_2_0_0MIC_ENABLE
export SPEECH_TX_THIRDPARTY_2_0_0MIC ?= 1

endif


export SPEECH_TX_DC_FILTER ?= 0
ifeq ($(SPEECH_TX_DC_FILTER),1)
KBUILD_CPPFLAGS += -DSPEECH_TX_DC_FILTER
endif

export SPEECH_TX_MIC_CALIBRATION ?= 0
ifeq ($(SPEECH_TX_MIC_CALIBRATION),1)
KBUILD_CPPFLAGS += -DSPEECH_TX_MIC_CALIBRATION
endif

export SPEECH_TX_MIC_FIR_CALIBRATION ?= 0
ifeq ($(SPEECH_TX_MIC_FIR_CALIBRATION),1)
KBUILD_CPPFLAGS += -DSPEECH_TX_MIC_FIR_CALIBRATION
endif

export SPEECH_TX_AEC_CODEC_REF ?= 0

export SPEECH_TX_AEC ?= 0
ifeq ($(SPEECH_TX_AEC),1)
KBUILD_CPPFLAGS += -DSPEECH_TX_AEC
KBUILD_CPPFLAGS += -DHFP_DISABLE_NREC
ifeq ($(CHIP_HAS_EC_CODEC_REF),1)
export SPEECH_TX_AEC_CODEC_REF := 1
endif
endif

export SPEECH_TX_AEC2 ?= 0
ifeq ($(SPEECH_TX_AEC2),1)
$(error SPEECH_TX_AEC2 is not supported now, use SPEECH_TX_AEC2FLOAT instead)
KBUILD_CPPFLAGS += -DSPEECH_TX_AEC2
KBUILD_CPPFLAGS += -DHFP_DISABLE_NREC
ifeq ($(CHIP_HAS_EC_CODEC_REF),1)
export SPEECH_TX_AEC_CODEC_REF := 1
endif
endif

export SPEECH_TX_AEC3 ?= 0
ifeq ($(SPEECH_TX_AEC3),1)
KBUILD_CPPFLAGS += -DSPEECH_TX_AEC3
KBUILD_CPPFLAGS += -DHFP_DISABLE_NREC
ifeq ($(CHIP_HAS_EC_CODEC_REF),1)
export SPEECH_TX_AEC_CODEC_REF := 1
endif
endif

export SPEECH_TX_AEC2FLOAT ?= 0
ifeq ($(SPEECH_TX_AEC2FLOAT),1)
KBUILD_CPPFLAGS += -DSPEECH_TX_AEC2FLOAT
KBUILD_CPPFLAGS += -DHFP_DISABLE_NREC
export DSP_LIB ?= 1
export NN_LIB ?= 1
ifeq ($(CHIP_HAS_EC_CODEC_REF),1)
export SPEECH_TX_AEC_CODEC_REF := 1
endif
endif

ifeq ($(SCO_DMA_SNAPSHOT),0)
export SPEECH_TX_AEC_CODEC_REF := 0
endif

# disable codec ref when 2300a enable anc
ifeq ($(CHIP), best2300a)
ifeq ($(ANC_APP), 1)
export SPEECH_TX_AEC_CODEC_REF := 0
endif
endif

export SPEECH_TX_NS ?= 0
ifeq ($(SPEECH_TX_NS),1)
KBUILD_CPPFLAGS += -DSPEECH_TX_NS
KBUILD_CPPFLAGS += -DHFP_DISABLE_NREC
endif

export SPEECH_TX_NS2 ?= 0
ifeq ($(SPEECH_TX_NS2),1)
KBUILD_CPPFLAGS += -DSPEECH_TX_NS2
KBUILD_CPPFLAGS += -DHFP_DISABLE_NREC
KBUILD_CPPFLAGS += -DLC_MMSE_FRAME_LENGTH=$(LC_MMSE_FRAME_LENGTH)
endif

export SPEECH_TX_NS2FLOAT ?= 0
ifeq ($(SPEECH_TX_NS2FLOAT),1)
KBUILD_CPPFLAGS += -DSPEECH_TX_NS2FLOAT
KBUILD_CPPFLAGS += -DHFP_DISABLE_NREC
export DSP_LIB ?= 1
endif

export SPEECH_TX_NS3 ?= 0
ifeq ($(SPEECH_TX_NS3),1)
KBUILD_CPPFLAGS += -DSPEECH_TX_NS3
KBUILD_CPPFLAGS += -DHFP_DISABLE_NREC
endif

ifeq ($(SPEECH_TX_NS4),1)
export SPEECH_TX_NS4
KBUILD_CPPFLAGS += -DSPEECH_TX_NS4
export HFP_DISABLE_NREC := 1
endif

ifeq ($(SPEECH_TX_NS5),1)
export SPEECH_TX_NS5
KBUILD_CPPFLAGS += -DSPEECH_TX_NS5
KBUILD_CPPFLAGS += -DHFP_DISABLE_NREC
export SPEECH_CODEC_FIXED_SAMPLE_RATE := 16000
export SPEECH_VQE_FIXED_SAMPLE_RATE := 16000
export SPEECH_TX_AEC_CODEC_REF := 1
endif

ifeq ($(SPEECH_TX_NS7),1)
export SPEECH_TX_NS7
KBUILD_CPPFLAGS += -DSPEECH_TX_NS7
export NN_LIB ?= 1
endif

ifeq ($(SPEECH_TX_WNR),1)
export SPEECH_TX_WNR
KBUILD_CPPFLAGS += -DSPEECH_TX_WNR
export HFP_DISABLE_NREC := 1
endif


export SPEECH_TX_WNR ?= 0
ifeq ($(SPEECH_TX_WNR),1)
KBUILD_CPPFLAGS += -DSPEECH_TX_WNR
KBUILD_CPPFLAGS += -DHFP_DISABLE_NREC
endif

export SPEECH_CS_VAD ?= 0
ifeq ($(SPEECH_CS_VAD),1)
KBUILD_CPPFLAGS += -DSPEECH_CS_VAD
KBUILD_CPPFLAGS += -DHFP_DISABLE_NREC
endif

export SPEECH_CODEC_CAPTURE_CHANNEL_NUM ?= 1

export SPEECH_TX_2MIC_NS ?= 0
ifeq ($(SPEECH_TX_2MIC_NS),1)
KBUILD_CPPFLAGS += -DSPEECH_TX_2MIC_NS
KBUILD_CPPFLAGS += -DHFP_DISABLE_NREC
export SPEECH_CODEC_CAPTURE_CHANNEL_NUM = 2
endif

export SPEECH_TX_2MIC_NS2 ?= 0
ifeq ($(SPEECH_TX_2MIC_NS2),1)
KBUILD_CPPFLAGS += -DSPEECH_TX_2MIC_NS2
KBUILD_CPPFLAGS += -DHFP_DISABLE_NREC
KBUILD_CPPFLAGS += -DCOH_FRAME_LENGTH=$(COH_FRAME_LENGTH)
export SPEECH_CODEC_CAPTURE_CHANNEL_NUM = 2
endif

export SPEECH_TX_2MIC_NS3 ?= 0
ifeq ($(SPEECH_TX_2MIC_NS3),1)
KBUILD_CPPFLAGS += -DSPEECH_TX_2MIC_NS3
KBUILD_CPPFLAGS += -DHFP_DISABLE_NREC
export SPEECH_CODEC_CAPTURE_CHANNEL_NUM = 2
endif

export SPEECH_TX_2MIC_NS4 ?= 0
ifeq ($(SPEECH_TX_2MIC_NS4),1)
KBUILD_CPPFLAGS += -DSPEECH_TX_2MIC_NS4
KBUILD_CPPFLAGS += -DHFP_DISABLE_NREC

ifeq ($(BONE_SENSOR_TDM),1)
# Get 1 channel from sensor
export SPEECH_CODEC_CAPTURE_CHANNEL_NUM = 1
else
export SPEECH_CODEC_CAPTURE_CHANNEL_NUM = 2
endif

endif

export SPEECH_TX_2MIC_NS5 ?= 0
ifeq ($(SPEECH_TX_2MIC_NS5),1)
KBUILD_CPPFLAGS += -DSPEECH_TX_2MIC_NS5
KBUILD_CPPFLAGS += -DHFP_DISABLE_NREC
export SPEECH_CODEC_CAPTURE_CHANNEL_NUM = 2
endif

ifeq ($(SPEECH_TX_2MIC_NS7),1)
export SPEECH_TX_2MIC_NS7
KBUILD_CPPFLAGS += -DSPEECH_TX_2MIC_NS7
export HFP_DISABLE_NREC := 1
KBUILD_CPPFLAGS += -DCOH_FRAME_LENGTH=$(COH_FRAME_LENGTH)
export SPEECH_CODEC_CAPTURE_CHANNEL_NUM = 2
export SPEECH_TX_AEC_CODEC_REF = 1
export NN_LIB = 1
endif

export SPEECH_TX_3MIC_NS ?= 0
ifeq ($(SPEECH_TX_3MIC_NS),1)
KBUILD_CPPFLAGS += -DSPEECH_TX_3MIC_NS
KBUILD_CPPFLAGS += -DHFP_DISABLE_NREC
ifeq ($(BONE_SENSOR_TDM),1)
# Get 1 channel from sensor
export SPEECH_CODEC_CAPTURE_CHANNEL_NUM = 2
else
export SPEECH_CODEC_CAPTURE_CHANNEL_NUM = 3
endif
endif

export SPEECH_TX_3MIC_NS3 ?= 0
ifeq ($(SPEECH_TX_3MIC_NS3),1)
KBUILD_CPPFLAGS += -DSPEECH_TX_3MIC_NS3
KBUILD_CPPFLAGS += -DHFP_DISABLE_NREC
# Get 1 channel from sensor
export SPEECH_CODEC_CAPTURE_CHANNEL_NUM = 3
endif

export SPEECH_TX_THIRDPARTY ?= 0
ifeq ($(SPEECH_TX_THIRDPARTY),1)
KBUILD_CPPFLAGS += -DSPEECH_TX_THIRDPARTY
KBUILD_CPPFLAGS += -DHFP_DISABLE_NREC
# Get 1 channel from sensor
export SPEECH_CODEC_CAPTURE_CHANNEL_NUM = 3
ifeq ($(CHIP_HAS_EC_CODEC_REF),1)
export SPEECH_TX_AEC_CODEC_REF := 1
endif
endif

export SPEECH_TX_NOISE_GATE ?= 0
ifeq ($(SPEECH_TX_NOISE_GATE),1)
KBUILD_CPPFLAGS += -DSPEECH_TX_NOISE_GATE
KBUILD_CPPFLAGS += -DHFP_DISABLE_NREC
endif

export SPEECH_TX_COMPEXP ?= 0
ifeq ($(SPEECH_TX_COMPEXP),1)
KBUILD_CPPFLAGS += -DSPEECH_TX_COMPEXP
KBUILD_CPPFLAGS += -DHFP_DISABLE_NREC
endif

export SPEECH_TX_AGC ?= 0
ifeq ($(SPEECH_TX_AGC),1)
KBUILD_CPPFLAGS += -DSPEECH_TX_AGC
KBUILD_CPPFLAGS += -DHFP_DISABLE_NREC
endif

export SPEECH_TX_EQ ?= 0
ifeq ($(SPEECH_TX_EQ),1)
KBUILD_CPPFLAGS += -DSPEECH_TX_EQ
KBUILD_CPPFLAGS += -DHFP_DISABLE_NREC
export DSP_LIB ?= 1
endif

export SPEECH_TX_POST_GAIN ?= 0
ifeq ($(SPEECH_TX_POST_GAIN),1)
KBUILD_CPPFLAGS += -DSPEECH_TX_POST_GAIN
endif

export SPEECH_RX_24BIT ?= 1

# disable codec ref when enable sidetone
ifeq ($(SPEECH_SIDETONE), 1)
export SPEECH_TX_AEC_CODEC_REF := 0
endif

ifeq ($(SPEECH_RX_COMPEXP),1)
KBUILD_CPPFLAGS += -DSPEECH_RX_COMPEXP
KBUILD_CPPFLAGS += -DHFP_DISABLE_NREC
endif

export SPEECH_RX_NS ?= 0
ifeq ($(SPEECH_RX_NS),1)
KBUILD_CPPFLAGS += -DSPEECH_RX_NS
KBUILD_CPPFLAGS += -DHFP_DISABLE_NREC
endif

export SPEECH_RX_NS2 ?= 0
ifeq ($(SPEECH_RX_NS2),1)
KBUILD_CPPFLAGS += -DSPEECH_RX_NS2
KBUILD_CPPFLAGS += -DHFP_DISABLE_NREC
endif

export SPEECH_RX_NS2FLOAT ?= 0
ifeq ($(SPEECH_RX_NS2FLOAT),1)
KBUILD_CPPFLAGS += -DSPEECH_RX_NS2FLOAT
KBUILD_CPPFLAGS += -DHFP_DISABLE_NREC
export DSP_LIB ?= 1
endif

export SPEECH_RX_NS3 ?= 0
ifeq ($(SPEECH_RX_NS3),1)
KBUILD_CPPFLAGS += -DSPEECH_RX_NS3
KBUILD_CPPFLAGS += -DHFP_DISABLE_NREC
endif

export SPEECH_RX_AGC ?= 0
ifeq ($(SPEECH_RX_AGC),1)
KBUILD_CPPFLAGS += -DSPEECH_RX_AGC
KBUILD_CPPFLAGS += -DHFP_DISABLE_NREC
endif

export SPEECH_RX_EQ ?= 0
ifeq ($(SPEECH_RX_EQ),1)
KBUILD_CPPFLAGS += -DSPEECH_RX_EQ
KBUILD_CPPFLAGS += -DHFP_DISABLE_NREC
export DSP_LIB ?= 1
endif

export SPEECH_RX_POST_GAIN ?= 0
ifeq ($(SPEECH_RX_POST_GAIN),1)
KBUILD_CPPFLAGS += -DSPEECH_RX_POST_GAIN
endif

export SPEECH_PROCESS_FRAME_MS 	?= 16
ifeq ($(SPEECH_CODEC_CAPTURE_CHANNEL_NUM),1)
export SPEECH_PROCESS_FRAME_MS := 15
endif
ifeq ($(SPEECH_TX_NN_NS),1)
export SPEECH_PROCESS_FRAME_MS := 16
endif
ifeq ($(SPEECH_TX_NN_NS2),1)
export SPEECH_PROCESS_FRAME_MS := 16
endif
ifeq ($(SPEECH_TX_NS5),1)
export SPEECH_PROCESS_FRAME_MS := 16
endif
ifeq ($(SPEECH_TX_2MIC_NS2),1)
export SPEECH_PROCESS_FRAME_MS := 15
endif
ifeq ($(SPEECH_TX_2MIC_NS4),1)
export SPEECH_PROCESS_FRAME_MS := 15
endif
ifeq ($(SPEECH_TX_2MIC_NS5),1)
export SPEECH_PROCESS_FRAME_MS := 15
endif
ifeq ($(SPEECH_TX_2MIC_NS7),1)
export SPEECH_PROCESS_FRAME_MS := 16
endif
ifeq ($(SPEECH_TX_THIRDPARTY),1)
export SPEECH_PROCESS_FRAME_MS := 15
endif
ifeq ($(SPEECH_TX_NS5),1)
export SPEECH_PROCESS_FRAME_MS := 16
endif
ifeq ($(SPEECH_TX_DTLN),1)
export SPEECH_PROCESS_FRAME_MS := 16
endif
ifeq ($(SPEECH_TX_NS7),1)
export SPEECH_PROCESS_FRAME_MS := 16
endif
KBUILD_CPPFLAGS += -DSPEECH_PROCESS_FRAME_MS=$(SPEECH_PROCESS_FRAME_MS)

export SPEECH_SCO_FRAME_MS 		?= 15
KBUILD_CPPFLAGS += -DSPEECH_SCO_FRAME_MS=$(SPEECH_SCO_FRAME_MS)

ifeq ($(SPEECH_TX_NS5),1)
export SPEECH_PROCESS_FRAME_MS := 16
endif
ifeq ($(SPEECH_TX_NS7),1)
export SPEECH_PROCESS_FRAME_MS := 16
endif

ifeq ($(SPEECH_SIDETONE),1)
export SPEECH_SIDETONE
KBUILD_CPPFLAGS += -DSPEECH_SIDETONE
ifeq ($(HW_SIDETONE_IIR_PROCESS),1)
ifeq ($(ANC_APP),1)
$(error ANC_APP conflicts with HW_SIDETONE_IIR_PROCESS)
endif
export HW_SIDETONE_IIR_PROCESS
KBUILD_CPPFLAGS += -DHW_SIDETONE_IIR_PROCESS
endif
ifeq ($(CHIP),best2000)
# Disable SCO resample
export SW_SCO_RESAMPLE := 0
export NO_SCO_RESAMPLE := 1
endif
endif

ifeq ($(THIRDPARTY_LIB),aispeech)
export DSP_LIB ?= 1
endif

ifeq ($(OTA_BIN_COMPRESSED),1)
KBUILD_CPPFLAGS += -DOTA_BIN_COMPRESSED
endif

# vvvvvvvvvvvvvvvvvvvvvvvvvvvvv
# Features for full application projects
# ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
ifeq ($(FULL_APP_PROJECT),1)

export BESLIB_INFO := $(GIT_REVISION)

export SPEECH_LIB ?= 1

export FLASH_PROTECTION ?= 1

export APP_TEST_AUDIO ?= 0

export APP_TEST_MODE ?= 0
ifeq ($(APP_TEST_MODE),1)
KBUILD_CPPFLAGS += -DAPP_TEST_MODE
endif

export VOICE_PROMPT ?= 1

export AUDIO_QUEUE_SUPPORT ?= 1

export VOICE_RECOGNITION ?= 0

export FLASH_SUSPEND ?= 1

export ENGINEER_MODE ?= 1
ifeq ($(ENGINEER_MODE),1)
FACTORY_MODE := 1
endif
ifeq ($(FACTORY_MODE),1)
KBUILD_CPPFLAGS += -D__FACTORY_MODE_SUPPORT__
endif

APP_USE_LED_INDICATE_IBRT_STATUS ?= 0
ifeq ($(APP_USE_LED_INDICATE_IBRT_STATUS),1)
KBUILD_CPPFLAGS += -D__APP_USE_LED_INDICATE_IBRT_STATUS__
endif

ifeq ($(HEAR_THRU_PEAK_DET),1)
KBUILD_CPPFLAGS += -D__HEAR_THRU_PEAK_DET__
endif


KBUILD_CPPFLAGS += -DMULTIPOINT_DUAL_SLAVE

endif # FULL_APP_PROJECT

ifeq ($(SPEECH_LIB),1)

export DSP_LIB ?= 1

ifeq ($(USB_AUDIO_APP),1)
ifneq ($(USB_AUDIO_SEND_CHAN),$(SPEECH_CODEC_CAPTURE_CHANNEL_NUM))
$(info )
$(info CAUTION: Change USB_AUDIO_SEND_CHAN($(USB_AUDIO_SEND_CHAN)) to SPEECH_CODEC_CAPTURE_CHANNEL_NUM($(SPEECH_CODEC_CAPTURE_CHANNEL_NUM)))
$(info )
export USB_AUDIO_SEND_CHAN := $(SPEECH_CODEC_CAPTURE_CHANNEL_NUM)
ifneq ($(USB_AUDIO_SEND_CHAN),$(SPEECH_CODEC_CAPTURE_CHANNEL_NUM))
$(error ERROR: Failed to change USB_AUDIO_SEND_CHAN($(USB_AUDIO_SEND_CHAN)))
endif
endif
endif

KBUILD_CPPFLAGS += -DSPEECH_CODEC_CAPTURE_CHANNEL_NUM=$(SPEECH_CODEC_CAPTURE_CHANNEL_NUM)
KBUILD_CPPFLAGS += -DSPEECH_ASR_CAPTURE_CHANNEL_NUM=$(SPEECH_ASR_CAPTURE_CHANNEL_NUM)
endif # SPEECH_LIB
# vvvvvvvvvvvvvvvvvvvvvvvvvvvvv
# GFPS feature
# ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
ifeq ($(GFPS_ENABLE),1)
# SASS feature
export SASS_ENABLE ?= 0
export SUPPORT_REMOTE_COD ?= 0
ifeq ($(SASS_ENABLE),1)
export SUPPORT_REMOTE_COD := 1
KBUILD_CPPFLAGS += -DSASS_ENABLED
endif

#spot feature
export SPOT_ENABLE ?= 0
ifeq ($(SPOT_ENABLE),1)
KBUILD_CPPFLAGS += -DSPOT_ENABLED
endif 

KBUILD_CPPFLAGS += -DGFPS_ENABLED
export BLE_SECURITY_ENABLED := 1

# this macro is used to determain if the resolveable private address is used for BLE
KBUILD_CPPFLAGS += -DBLE_USE_RPA

endif # GFPS
# vvvvvvvvvvvvvvvvvvvvvvvvvvvvv
# Put customized features above
# ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

# vvvvvvvvvvvvvvvvvvvvvvvvvvvvv
# Obsoleted features
# ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
OBSOLETED_FEATURE_LIST := EQ_PROCESS RB_CODEC AUDIO_EQ_PROCESS MEDIA_PLAYER_RBCODEC
USED_OBSOLETED_FEATURE := $(strip $(foreach f,$(OBSOLETED_FEATURE_LIST),$(if $(filter 1,$($f)),$f)))
ifneq ($(USED_OBSOLETED_FEATURE),)
$(error Obsoleted features: $(USED_OBSOLETED_FEATURE))
endif

# -------------------------------------------
# General
# -------------------------------------------

ifneq ($(NO_CONFIG),1)
core-y += config/
endif

ifneq ($(NO_BOOT_STRUCT),1)
core-y += $(call add_if_exists,utils/boot_struct/)
endif

export DEFAULT_CFG_SRC ?= _default_cfg_src_

ifneq ($(wildcard $(srctree)/config/$(T)/tgt_hardware.h $(srctree)/config/$(T)/res/),)
KBUILD_CPPFLAGS += -Iconfig/$(T)
endif
KBUILD_CPPFLAGS += -Iconfig/$(DEFAULT_CFG_SRC)

CPU_CFLAGS := -mthumb
ifeq ($(CPU),a7)
CPU_CFLAGS += -march=armv7-a
else ifeq ($(CPU),m33)
ifeq ($(CPU_NO_DSP),1)
CPU_CFLAGS += -mcpu=cortex-m33+nodsp
else
CPU_CFLAGS += -mcpu=cortex-m33
endif
else
CPU_CFLAGS += -mcpu=cortex-m4
endif

export UNALIGNED_ACCESS ?= 1
ifeq ($(UNALIGNED_ACCESS),1)
KBUILD_CPPFLAGS += -DUNALIGNED_ACCESS
else
CPU_CFLAGS += -mno-unaligned-access
endif

ifeq ($(CHIP_HAS_FPU),1)
ifeq ($(CPU),a7)
CPU_CFLAGS += -mfpu=vfpv3-d16
else ifeq ($(CPU),m33)
CPU_CFLAGS += -mfpu=fpv5-sp-d16
else
CPU_CFLAGS += -mfpu=fpv4-sp-d16
endif
ifeq ($(SOFT_FLOAT_ABI),1)
CPU_CFLAGS += -mfloat-abi=softfp
else
CPU_CFLAGS += -mfloat-abi=hard
endif
else
CPU_CFLAGS += -mfloat-abi=soft
endif

ifneq ($(ALLOW_WARNING),1)
KBUILD_CPPFLAGS += -Werror
endif

ifeq ($(STACK_USAGE),1)
KBUILD_CPPFLAGS += -fstack-usage
endif

ifeq ($(PIE),1)
ifneq ($(NOSTD),1)
$(error PIE can only work when NOSTD=1)
endif
KBUILD_CPPFLAGS += -fPIE -msingle-pic-base
# -pie option will generate .dynamic section
#LDFLAGS += -pie
#LDFLAGS += -z relro -z now
endif

KBUILD_CPPFLAGS += $(CPU_CFLAGS) $(SPECS_CFLAGS)
LINK_CFLAGS += $(CPU_CFLAGS) $(SPECS_CFLAGS)
CFLAGS_IMAGE += $(CPU_CFLAGS) $(SPECS_CFLAGS)

# Save 100+ bytes by filling less alignment holes
# TODO: Array alignment?
#LDFLAGS += --sort-common --sort-section=alignment

ifeq ($(CTYPE_PTR_DEF),1)
LDFLAGS_IMAGE += --defsym __ctype_ptr__=0
endif

ifeq ($(BT_FA_DYNAMIC_SWITCHING),1)
KBUILD_CPPFLAGS += -DBT_FA_DYNAMIC_SWITCHING
endif

ifeq ($(BT_EXT_LNA),1)
KBUILD_CPPFLAGS += -DBT_EXT_LNA
ifeq ($(BT_EXT_LNA_DYNAMIC_SWITCHING),1)
KBUILD_CPPFLAGS += -DBT_EXT_LNA_DYNAMIC_SWITCHING
endif
endif

ifeq ($(POWERKEY_I2C_SWITCH),1)
KBUILD_CPPFLAGS += -DPOWERKEY_I2C_SWITCH
endif

ifeq ($(SIMPLE_TEST_UI),1)
KBUILD_CPPFLAGS += -DSIMPLE_TEST_UI
endif

TX_IQ_CAL ?= 0
ifeq ($(TX_IQ_CAL),1)
KBUILD_CPPFLAGS += -DTX_IQ_CAL
endif

ifeq ($(RX_IQ_CAL),1)
KBUILD_CPPFLAGS += -DRX_IQ_CAL
endif

ifeq ($(USE_LOWLATENCY_LIB),1)
core-y += thirdparty/lowlatency_lib/
endif

USE_PERIPHERAL_THREAD ?=0
ifeq ($(USE_PERIPHERAL_THREAD),1)
KBUILD_CPPFLAGS += -DUSE_PERIPHERAL_THREAD
endif

ifneq ($(RAMCP_SIZE), )
KBUILD_CPPFLAGS += -DRAMCP_SIZE=$(RAMCP_SIZE)
endif

ifneq ($(RAMCPX_SIZE), )
KBUILD_CPPFLAGS += -DRAMCPX_SIZE=$(RAMCPX_SIZE)
endif

ifneq ($(FAST_XRAM_SECTION_SIZE), )
KBUILD_CPPFLAGS += -DFAST_XRAM_SECTION_SIZE=$(FAST_XRAM_SECTION_SIZE)
endif

export AAC_REDUCE_SIZE ?= 1
ifeq ($(AAC_REDUCE_SIZE),1)
KBUILD_CPPFLAGS += -DAAC_REDUCE_SIZE
endif

UNIFY_HEAP_ENABLED ?= 0
ifeq ($(UNIFY_HEAP_ENABLED),1)
KBUILD_CPPFLAGS += -DUNIFY_HEAP_ENABLED
endif

export TWS_RS_BY_BTC ?= 0
ifeq ($(TWS_RS_BY_BTC),1)
KBUILD_CPPFLAGS += -D__BES_FA_MODE__
KBUILD_CPPFLAGS += -DTWS_RS_BY_BTC
endif

$(info ----------common.mk in flash---------------------)
$(info FLASH_REMAP: $(FLASH_REMAP))
$(info NEW_IMAGE_FLASH_OFFSET: $(NEW_IMAGE_FLASH_OFFSET))
$(info OTA_CODE_OFFSET: $(OTA_CODE_OFFSET))
$(info FLASH_SIZE: $(FLASH_SIZE))
$(info -------------------------------)