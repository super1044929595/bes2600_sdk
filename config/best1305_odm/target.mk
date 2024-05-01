CHIP        ?= best1305

DEBUG       ?= 1

FPGA        ?= 0

RTOS        ?= 1

#LIBC_ROM    ?= 1

export LIBC_OVERRIDE := 1

#KERNEL      ?= RTX5

export USER_SECURE_BOOT	?= 0
# enable:1
# disable:0

WATCHER_DOG ?= 1

DEBUG_PORT  ?= 1
# 0: usb
# 1: uart0
# 2: uart1

FLASH_CHIP	?= ALL
# GD25Q80C
# GD25Q32C
# ALL

export CHG_OUT_PWRON ?= 0

#If you want to use the single-wire communication function,
#please open this Macro, otherwise the compilation error will be reported
export PMU_IRQ_UNIFIED ?= 1

export NEW_SWAGC_MODE ?= 1

export BLE_NEW_SWAGC_MODE ?= 0

export BT_UART_LOG_P16 ?= 0

export BT_UART_LOG ?= 0

export NO_LPU_26M ?= 0

export BT_SYSTEM_52M ?= 0

export BES_FA_MODE ?= 0

export LL_MONITOR ?= 0

export SOFTBIT_EN ?= 0

export ACL_DATA_CRC_TEST ?= 0

export FORCE_SIGNALINGMODE ?= 0

export FORCE_NOSIGNALINGMODE ?= 0

export FORCE_SCO_MAX_RETX ?= 0

export BT_FA_ECC ?= 0

ifeq ($(BT_FA_ECC),1)
export BT_FA_SCO_ECC ?= 1
export BT_FA_ACL_ECC ?= 1
endif

export BT_ECC_CONFIG_BLK ?= 2

export BT_FAST_LOCK_ENABLE ?= 0

export IBRT_TESTMODE ?= 0

export CONTROLLER_DUMP_ENABLE ?= 1

export CONTROLLER_MEM_LOG_ENABLE ?= 0

export INTERSYS_DEBUG ?= 1

export PROFILE_DEBUG ?= 0

export BTDUMP_ENABLE ?= 0

export BT_DEBUG_TPORTS ?= 0

TPORTS_KEY_COEXIST ?= 0

export PATCH_SYMBOL_DIR ?= $(srctree)/platform/drivers/bt/best1305

export LD_USE_PATCH_SYMBOL ?= -Wl,--just-symbols=$(PATCH_SYMBOL_DIR)/patch_symbol_parsed.txt -Wl,--just-symbols=$(PATCH_SYMBOL_DIR)/patch_symbol_parsed_testmode.txt

AUDIO_OUTPUT_MONO ?= 0

AUDIO_OUTPUT_DIFF ?= 0

HW_FIR_EQ_PROCESS ?= 0

SW_IIR_EQ_PROCESS ?= 0

HW_DAC_IIR_EQ_PROCESS ?= 1

HW_IIR_EQ_PROCESS ?= 0

HW_DC_FILTER_WITH_IIR ?= 0

AUDIO_DRC ?= 0

AUDIO_LIMITER ?= 0

AUDIO_DYNAMIC_BOOST ?= 0

PC_CMD_UART ?= 0

AUDIO_SECTION_ENABLE ?= 0

AUDIO_RESAMPLE ?= 1

RESAMPLE_ANY_SAMPLE_RATE ?= 1

export OSC_26M_X4_AUD2BB ?= 0
export FLASH_LOW_SPEED ?= 1
export FLASH_ULTRA_LOW_SPEED ?= 0
export FLASH_DTR ?= 0

AUDIO_OUTPUT_VOLUME_DEFAULT ?= 12
# range:1~16

AUDIO_INPUT_CAPLESSMODE ?= 0

AUDIO_INPUT_LARGEGAIN ?= 0

AUDIO_CODEC_ASYNC_CLOSE ?= 0

AUDIO_SCO_BTPCM_CHANNEL ?= 1

export A2DP_CP_ACCEL ?= 1
export SCO_CP_ACCEL ?= 1

export SCO_TRACE_CP_ACCEL ?= 0

# For TWS SCO DMA snapshot and low delay
export PCM_FAST_MODE ?= 1

export PCM_PRIVATE_DATA_FLAG ?= 0

export CVSD_BYPASS ?= 1

export LOW_DELAY_SCO ?= 0

export SPEECH_TX_24BIT ?= 0

export SCO_OPTIMIZE_FOR_RAM ?=1

export SPEECH_ECHO_NLP_ONLY ?= 1

SPEECH_TX_THIRDPARTY_SNDP ?= 0

SPEECH_TX_DC_FILTER ?= 0

SPEECH_TX_AEC2FLOAT ?= 0

#1 mic
SPEECH_TX_NS5 ?= 0

SPEECH_TX_NS7 ?= 0

SPEECH_TX_DTLN ?= 0

#2 mic
SPEECH_TX_2MIC_NS7 ?= 0

SPEECH_TX_COMPEXP ?= 0

SPEECH_TX_EQ ?= 0

SPEECH_TX_POST_GAIN ?= 0

SPEECH_RX_COMPEXP ?= 0

SPEECH_RX_NS2FLOAT ?= 1

SPEECH_RX_EQ ?= 1

SPEECH_RX_POST_GAIN ?= 0

LARGE_RAM ?= 1

HSP_ENABLE ?= 0

HFP_1_6_ENABLE ?= 1

MSBC_PLC_ENABLE ?= 1

MSBC_PLC_ENCODER ?= 1

MSBC_16K_SAMPLE_RATE ?= 1

VOICE_TX_AEC ?= 0

APP_NOISE_ESTIMATION ?= 0

SBC_FUNC_IN_ROM ?= 0

ROM_UTILS_ON ?= 1

VOICE_PROMPT ?= 1

BLE ?= 1
export BLE_PERIPHERAL_ONLY ?= 0
export GFPS_ENABLE ?= 1

TOTA ?= 0

TOTA_v2 ?= 0

ifeq ($(TOTA_v2),1)
export TOTA := 0
endif

export BLE_APP_DATAPATH_SERVER_ENABLED ?= 0

export GATT_OVER_BR_EDR ?= 0

export BES_OTA_BASIC ?= 0

#export OTA_CODE_OFFSET ?= 0x18000

TILE_DATAPATH_ENABLED ?= 0

QIOT_DATAPATH_ENABLE ?= 0

CUSTOM_INFORMATION_TILE_ENABLE ?= 0

INTERACTION ?= 0

INTERACTION_FASTPAIR ?= 0

BT_ONE_BRING_TWO ?= 0

DSD_SUPPORT ?= 0

A2DP_EQ_24BIT ?= 1

A2DP_AAC_ON ?= 1

A2DP_SCALABLE_ON ?= 0

A2DP_LHDC_ON ?= 0

ifeq ($(A2DP_LHDC_ON),1)
A2DP_LHDC_V3 ?= 0
endif

A2DP_LDAC_ON ?= 0

export TX_RX_PCM_MASK ?= 0

A2DP_SCALABLE_ON ?= 0

FACTORY_MODE ?= 1

ENGINEER_MODE ?= 1

ULTRA_LOW_POWER	?= 1

DAC_CLASSG_ENABLE ?= 1

SPEECH_SIDETONE ?= 0

HW_SIDETONE_IIR_PROCESS ?= 0

NO_SLEEP ?= 0

CORE_DUMP ?= 1

CORE_DUMP_TO_FLASH ?= 0

ENHANCED_STACK ?= 1

CALIB_SLOW_TIMER ?= 1

export SYNC_BT_CTLR_PROFILE ?= 0

export A2DP_AVDTP_CP ?= 0

export IBRT ?= 1

export IBRT_SEARCH_UI ?= 0

export BES_AUD ?= 1

export POWER_MODE   ?= DIG_DCDC

export BT_RF_PREFER ?= 2M

export SPEECH_CODEC ?= 1

export MIX_AUDIO_PROMPT_WITH_A2DP_MEDIA_ENABLED ?= 1

export TWS_PROMPT_SYNC ?= 1

export IOS_MFI ?= 0

export FLASH_SIZE ?= 0x200000
export FLASH_SUSPEND ?= 1

export HOST_GEN_ECDH_KEY ?= 0

export USE_TRACE_ID ?= 0

export PROMPT_IN_FLASH ?= 0

export USE_POWER_KEY_RESET ?= 0

export SINGLE_WIRE_DOWNLOAD ?= 0

ifeq ($(DSD_SUPPORT),1)
export BTUSB_AUDIO_MODE     ?= 1
export AUDIO_INPUT_MONO     ?= 1
export USB_ISO              ?= 1
export USB_AUDIO_DYN_CFG    ?= 1
export DELAY_STREAM_OPEN    ?= 0
export KEEP_SAME_LATENCY    ?= 1
export HW_FIR_DSD_PROCESS   ?= 1
ifeq ($(HW_FIR_DSD_PROCESS),1)
ifeq ($(CHIP),best2300)
export HW_FIR_DSD_BUF_MID_ADDR  ?= 0x200A0000
export DATA_BUF_START           ?= 0x20040000
endif
endif
export USB_AUDIO_UAC2 ?= 1
export USB_HIGH_SPEED ?= 1
KBUILD_CPPFLAGS += \
    -DHW_FIR_DSD_BUF_MID_ADDR=$(HW_FIR_DSD_BUF_MID_ADDR) \
    -DDATA_BUF_START=$(DATA_BUF_START)
endif

USE_THIRDPARTY ?= 0
export USE_KNOWLES ?= 0

export LAURENT_ALGORITHM ?= 1

export TX_IQ_CAL ?= 1

export RX_IQ_CAL ?= 1

export BT_XTAL_SYNC ?= 1

export BT_XTAL_SYNC_SLOW ?= 0

export BT_XTAL_SYNC_NO_RESET ?= 0

export BTADDR_FOR_DEBUG ?= 1

export POWERKEY_I2C_SWITCH ?=0

AUTO_TEST ?= 0

BES_AUTOMATE_TEST ?= 0

export DUMP_NORMAL_LOG ?= 0

SUPPORT_BATTERY_REPORT ?= 1

SUPPORT_HF_INDICATORS ?= 0

SUPPORT_SIRI ?= 1

export TOTA_EQ_TUNING ?= 0

BES_AUDIO_DEV_Main_Board_9v0 ?= 0

APP_USE_LED_INDICATE_IBRT_STATUS ?= 0

export BT_FA_DYNAMIC_SWITCHING ?=0

export BT_EXT_LNA ?=0
export BT_EXT_LNA_DYNAMIC_SWITCHING ?=0
export BT_EXT_PA ?=0

export UNIFY_HEAP_ENABLED ?=1
ifeq ($(UNIFY_HEAP_ENABLED),1)
export AUDIO_BUFFER_SIZE ?= 100*1024
export AAC_MTU_LIMITER ?= 50
export SBC_MTU_LIMITER ?= 300
else
export AUDIO_BUFFER_SIZE ?= 50*1024
export AAC_MTU_LIMITER ?= 34
export SBC_MTU_LIMITER ?= 150
endif
export TRACE_BUF_SIZE ?= 4*1024
export TRACE_BAUD_RATE ?= 1152000

export MEM_POOL_SIZE ?= 0
export OS_DYNAMIC_MEM_SIZE ?= 0x5800
export OS_TIMER_THREAD_STACK_SIZE ?= 0x800
export OPT_LEVEL ?= s
KBUILD_CPPFLAGS += -DAUDIO_BUFFER_SIZE=$(AUDIO_BUFFER_SIZE)
KBUILD_CPPFLAGS += -DAAC_MTU_LIMITER=$(AAC_MTU_LIMITER)
KBUILD_CPPFLAGS += -DSBC_MTU_LIMITER=$(SBC_MTU_LIMITER)
KBUILD_CPPFLAGS += -DMEM_POOL_SIZE=$(MEM_POOL_SIZE)

init-y :=
core-y := platform/ services/ apps/ utils/cqueue/ utils/list/ services/multimedia/ utils/intersyshci/ utils/sha256/

KBUILD_CPPFLAGS += \
    -Iplatform/cmsis/inc \
    -Iservices/audioflinger \
    -Iplatform/hal

KBUILD_CPPFLAGS += \
    -DAPP_AUDIO_BUFFER_SIZE=$(AUDIO_BUFFER_SIZE) \
    -DCHARGER_PLUGINOUT_RESET=0
ifeq ($(BES_AUDIO_DEV_Main_Board_9v0),1)
KBUILD_CPPFLAGS += -DBES_AUDIO_DEV_Main_Board_9v0
endif

ifeq ($(TPORTS_KEY_COEXIST),1)
KBUILD_CPPFLAGS += -DTPORTS_KEY_COEXIST
endif

#-DIBRT_LINK_LOWLAYER_MONITOR

#-D_AUTO_SWITCH_POWER_MODE__
#-D__APP_KEY_FN_STYLE_A__
#-D__APP_KEY_FN_STYLE_B__
#-D__EARPHONE_STAY_BOTH_SCAN__
#-D__POWERKEY_CTRL_ONOFF_ONLY__
#-DAUDIO_LINEIN

export AUDIO_OUTPUT_FACTORY_CALIB ?= 0

export AUDIO_OUTPUT_DC_AUTO_CALIB ?= 1
ifeq ($(AUDIO_OUTPUT_DC_AUTO_CALIB), 1)
export AUDIO_OUTPUT_DC_CALIB := 1
export AUDIO_OUTPUT_DC_CALIB_ANA := 0
export AUDIO_OUTPUT_SET_LARGE_ANA_DC ?= 1
export AUDIO_OUTPUT_DC_CALIB_RST_ANA_ADC ?= 1
export DAC_DRE_ENABLE ?= 1
export CODEC_DAC_DC_NV_DATA ?= 1
export CODEC_DAC_DC_DYN_BUF ?= 1
endif

ifeq ($(CURRENT_TEST),1)
INTSRAM_RUN ?= 1
endif
ifeq ($(INTSRAM_RUN),1)
LDS_FILE := best1000_intsram.lds
else
LDS_FILE := best1000.lds
endif

export OTA_SUPPORT_SLAVE_BIN ?= 0
ifeq ($(OTA_SUPPORT_SLAVE_BIN),1)
export SLAVE_BIN_FLASH_OFFSET ?= 0x100000
export SLAVE_BIN_TARGET_NAME ?= anc_usb
endif

ifeq ($(GATT_OVER_BR_EDR),1)
export GATT_OVER_BR_EDR ?= 1
KBUILD_CPPFLAGS += -D__GATT_OVER_BR_EDR__
endif

ifeq ($(ANC_ASSIST_UNUSED_ON_PHONE_CALL),1)
KBUILD_CPPFLAGS += -DANC_ASSIST_UNUSED_ON_PHONE_CALL
endif
ifeq ($(INTERACTION),1)
export OTA_BASIC := 1
endif

ifeq ($(PROMPT_IN_FLASH),1)
KBUILD_CPPFLAGS += -DPROMPT_IN_FLASH
endif

ifeq ($(HOST_GEN_ECDH_KEY),1)
KBUILD_CPPFLAGS += -D__HOST_GEN_ECDH_KEY__
endif

ifeq ($(AUDIO_OUTPUT_DC_AUTO_CALIB),1)
KBUILD_CPPFLAGS += -DAUDIO_OUTPUT_DC_AUTO_CALIB
endif

ifeq ($(CODEC_DAC_DC_NV_DATA), 1)
KBUILD_CPPFLAGS += -DCODEC_DAC_DC_NV_DATA
endif

export USE_LOWLATENCY_LIB ?= 0
ifeq ($(USE_LOWLATENCY_LIB),1)
KBUILD_CPPFLAGS += -DUSE_LOWLATENCY_LIB
endif

ifeq ($(USE_POWER_KEY_RESET),1)
KBUILD_CPPFLAGS += -DUSE_POWER_KEY_RESET
endif

ifeq ($(SINGLE_WIRE_DOWNLOAD),1)
KBUILD_CPPFLAGS += -DSINGLE_WIRE_DOWNLOAD
endif

#### ANC DEFINE START ######
export ANC_APP ?= 0
ifeq ($(ANC_APP),1)
#### ANC CONFIG ######
export ANC_FF_ENABLED	    ?= 1
export ANC_FB_ENABLED	    ?= 1
export ANC_ASSIST_ENABLED   ?= 0
export AUDIO_ANC_FB_MC      ?= 0
export AUDIO_ANC_FB_MC_HW   ?= 1
export AUDIO_ANC_FB_ADJ_MC  ?= 0
export AUDIO_SECTION_SUPPT  ?= 1
export AUD_SECTION_STRUCT_VERSION ?= 2
##### ANC DEFINE END ######

export PSAP_APP  ?= 0

export ANC_LOAD_DEFAULT_COEF_L_R ?= 1
# 1: load left coef
# 0: load right coef
export DEBUG_ANC_BY_PHONE ?= 0

APP_ANC_TEST ?= 0
ifeq ($(APP_ANC_TEST),1)
TOTA := 1
endif

ifeq ($(ANC_APP),1)
KBUILD_CPPFLAGS += \
    -DANC_APP \
    -D__BT_ANC_KEY__\
    -D__APP_KEY_FN_STYLE_A__ 
endif
endif
LIB_LDFLAGS += -lstdc++ -lsupc++

#CFLAGS_IMAGE += -u _printf_float -u _scanf_float

#LDFLAGS_IMAGE += --wrap main

export RAMCP_SIZE ?= 0x26000
export RAMCPX_SIZE ?= 0x8800

ifeq ($(SPEECH_TX_DTLN),1)
core-y += thirdparty/tflite/
endif
