cur_dir := $(dir $(lastword $(MAKEFILE_LIST)))

obj-y := $(patsubst $(cur_dir)%,%,$(wildcard $(cur_dir)*.c $(cur_dir)*.cpp $(cur_dir)*.S))
obj-y := $(obj-y:.c=.o)
obj-y := $(obj-y:.cpp=.o)
obj-y := $(obj-y:.S=.o)

obj-y += $(CHIP)/

subdir-ccflags-y += \
    -Iinclude/watchdog \
    -Iplatform/drivers/bt \
    -Iplatform/drivers/ana \
    -Iservices/nvrecord \
    -Iutils/cqueue \
    -Iutils/heap \
	-Iutils/intersyshci \
    -Iservices/nv_section/userdata_section

ifeq ($(BT_XTAL_SYNC),1)
subdir-ccflags-y += -DBT_XTAL_SYNC
endif

export BT_DRV_UART_BRIDGE_FLAGS :=
ifneq ($(DEBUG_PORT),)
BT_DRV_UART_BRIDGE_FLAGS += -DDEBUG_PORT=$(DEBUG_PORT)
endif

export BT_DRV_FLAGS :=
ifeq ($(NO_SLEEP),1)
BT_DRV_FLAGS += -DNO_SLEEP
endif

export BT_DRV_CONFIG_FLAGS :=
ifeq ($(INTERSYS_NO_THREAD),1)
BT_DRV_CONFIG_FLAGS += -D_INTERSYS_NO_THREAD_
endif

VCO_TEST_TOOL ?= 1
ifeq ($(VCO_TEST_TOOL),1)
subdir-ccflags-y += -DVCO_TEST_TOOL
endif

ifeq ($(LAURENT_ALGORITHM),1)
subdir-ccflags-y += -DLAURENT_ALGORITHM
endif

ifeq ($(FORCE_SCO_MAX_RETX),1)
subdir-ccflags-y += -D__FORCE_SCO_MAX_RETX__
endif

ifeq ($(CONTROLLER_DUMP_ENABLE),1)
subdir-ccflags-y += -DCONTROLLER_DUMP_ENABLE=1
endif

ifeq ($(BT_UART_LOG),1)
subdir-ccflags-y += -DBT_UART_LOG
endif

ifeq ($(BT_UART_LOG_P16),1)
subdir-ccflags-y += -DBT_UART_LOG_P16
endif

ifeq ($(LL_MONITOR),1)
subdir-ccflags-y += -DLL_MONITOR=1
endif

ifeq ($(SOFTBIT_EN),1)
subdir-ccflags-y += -DSOFTBIT_EN=1
endif

ifeq ($(ACL_DATA_CRC_TEST),1)
subdir-ccflags-y += -DACL_DATA_CRC_TEST=1
endif

ifeq ($(FA_RX_GAIN_CTRL),1)
subdir-ccflags-y += -D__FA_RX_GAIN_CTRL__
endif

ifeq ($(BT_SYSTEM_52M),1)
subdir-ccflags-y += -DBT_SYSTEM_52M
endif

ifeq ($(BT_FA_ECC),1)
subdir-ccflags-y += -D__FASTACK_ECC_ENABLE__
endif

ifeq ($(BT_FA_SCO_ECC),1)
subdir-ccflags-y += -D__FASTACK_SCO_ECC_ENABLE__
endif

ifeq ($(BT_FA_ACL_ECC),1)
subdir-ccflags-y += -D__FASTACK_ACL_ECC_ENABLE__
endif

ifeq ($(BT_FAST_LOCK_ENABLE),1)
subdir-ccflags-y += -DBT_FAST_LOCK_ENABLE
endif


ifdef BT_ECC_CONFIG_BLK
ifneq ($(BT_ECC_CONFIG_BLK),0)
subdir-ccflags-y += -D__FASTACK_ECC_CONFIG_BLOCK__=$(BT_ECC_CONFIG_BLK)
endif
endif

ifdef BT_BID_ECC_EN
ifneq ($(BT_BID_ECC_EN),0)
subdir-ccflags-y += -D__BT_BID_ECC_EN__=$(BT_BID_ECC_EN)
endif
endif

ifeq ($(BES_FA_MODE),1)
subdir-ccflags-y += -D__BES_FA_MODE__
endif

ifeq ($(HW_AGC),1)
subdir-ccflags-y += -D__HW_AGC__
endif

ifeq ($(BT_LOG_POWEROFF),1)
subdir-ccflags-y += -DBT_LOG_POWEROFF
endif

ifeq ($(BT_INFO_CHECKER),1)
subdir-ccflags-y += -DBT_INFO_CHECKER
endif

ifeq ($(NEW_SWAGC_MODE),1)
subdir-ccflags-y += -D__NEW_SWAGC_MODE__
endif

ifeq ($(BLE_NEW_SWAGC_MODE),1)
subdir-ccflags-y += -D__BLE_NEW_SWAGC_MODE__
endif

ifeq ($(BT_EXT_LNA),1)
KBUILD_CPPFLAGS += -DBT_EXT_LNA
endif

ifeq ($(BT_EXT_PA),1)
KBUILD_CPPFLAGS += -DBT_EXT_PA
endif

ifdef BT_DEBUG_TPORTS
ifneq ($(BT_DEBUG_TPORTS),0)
subdir-ccflags-y += -DTPORT_TYPE=$(BT_DEBUG_TPORTS)
endif
endif

ifeq ($(CONTROLLER_DUMP_ENABLE),1)
	KBUILD_CPPFLAGS += -DCONTROLLER_DUMP_ENABLE=1
endif

ifeq ($(CONTROLLER_MEM_LOG_ENABLE),1)
	KBUILD_CPPFLAGS += -DCONTROLLER_MEM_LOG_ENABLE=1
endif

ifeq ($(AFH_ASSESS),1)
KBUILD_CPPFLAGS += -D__AFH_ASSESS__
endif

ifeq ($(BT_RAMRUN),1)
$(obj)/bt_ramrun_bin.o : $(cur_dir)$(CHIP)/$(CHIP)_ramrun_fw.bin
AFLAGS_bt_ramrun_bin.o += -DBT_RAMRUN_BIN_NAME=$(cur_dir)$(CHIP)/$(CHIP)_ramrun_fw.bin
endif

ifeq ($(HIGH_EFFICIENCY_TX_PWR_CTRL),1)
ccflags-y += -DHIGH_EFFICIENCY_TX_PWR_CTRL
endif

ccflags-y += $(BT_IF_INCLUDES)
