cur_dir := $(dir $(lastword $(MAKEFILE_LIST)))

obj_s += $(patsubst $(cur_dir)%,%,$(wildcard $(cur_dir)/*/*.S))
obj_c += $(patsubst $(cur_dir)%,%,$(wildcard $(cur_dir)/*/*.c))
obj_cpp += $(patsubst $(cur_dir)%,%,$(wildcard $(cur_dir)/*/*.cpp))

PROGRAMMER_LIB_NAME := libprogrammer

ifeq ($(OTA_BIN_COMPRESSED),1)
PROGRAMMER_LIB_NAME := $(PROGRAMMER_LIB_NAME)_compressed
endif

ifeq ($(SINGLE_WIRE_DOWNLOAD),1)
PROGRAMMER_LIB_NAME := $(PROGRAMMER_LIB_NAME)_single_wire_download
endif

ifeq ($(DIRECT_TO_SINGLE_DLD),1)
PROGRAMMER_LIB_NAME := $(PROGRAMMER_LIB_NAME)_direct_single_dld
endif

$(PROGRAMMER_LIB_NAME)-y := $(obj_c:.c=.o) $(obj_s:.S=.o) $(obj_cpp:.cpp=.o)

obj-y += $(PROGRAMMER_LIB_NAME).a

obj-y += ../../utils/hexdump/ ../../utils/hwtimer_list/ ../../utils/crc32/

obj-y += ../../platform/drivers/norflash/ ../../platform/drivers/ana/

ifeq ($(CHIP_HAS_USB),1)
obj-y += ../../platform/drivers/usb/usb_dev/
CFLAGS_sys_api_programmer.o += -Iplatform/drivers/usb/usb_dev/inc
endif

#ccflags-y += -I$(obj)/../../utils/hexdump -I$(obj)/../../utils/boot_struct -I$(obj)/../../utils/rom_utils
ccflags-y += -Itests/programmer/inc -Iutils/hexdump -Iutils/hwtimer_list -Iutils/boot_struct -Iutils/rom_utils -Iutils/crc32

CFLAGS_sys_api_programmer.o += -Iplatform/drivers/ana

ifneq ($(DEBUG_PORT),)
CFLAGS_sys_api_programmer.o += -DDEBUG_PORT=$(DEBUG_PORT)
endif

ifeq ($(FLASH_UNIQUE_ID),1)
CFLAGS_flash_programmer.o += -DFLASH_UNIQUE_ID
CFLAGS_sys_api_programmer.o += -DFLASH_UNIQUE_ID
endif

ifeq ($(FLASH_SECURITY_REGISTER),1)
CFLAGS_flash_programmer.o += -DFLASH_SECURITY_REGISTER
CFLAGS_sys_api_programmer.o += -DFLASH_SECURITY_REGISTER
endif

ifeq ($(FLASH_REMAP),1)
CFLAGS_flash_programmer.o += -DFLASH_REMAP
CFLAGS_sys_api_programmer.o += -DFLASH_REMAP
endif

ifeq ($(FLASH_DTR),1)
CFLAGS_sys_api_programmer.o += -DFLASH_DTR
endif

ifeq ($(PROGRAMMER_LOAD_SIMU),1)
CFLAGS_sys_api_programmer.o += -DPROGRAMMER_LOAD_SIMU
endif

ifeq ($(PROGRAMMER_ERROR_ASSERT),1)
CFLAGS_sys_api_programmer.o += -DPROGRAMMER_ERROR_ASSERT
endif

ifeq ($(JTAG_ENABLE),1)
CFLAGS_sys_api_programmer.o += -DJTAG_ENABLE
endif

ifeq ($(NO_UART_IOMUX),1)
CFLAGS_sys_api_programmer.o += -DNO_UART_IOMUX
endif

ifeq ($(FORCE_ANALOG_I2C),1)
CFLAGS_sys_api_programmer.o += -DFORCE_ANALOG_I2C
endif

ifeq ($(SINGLE_WIRE_DOWNLOAD),1)
CFLAGS_flash_programmer.o += -DSINGLE_WIRE_DOWNLOAD
CFLAGS_sys_api_programmer.o += -DSINGLE_WIRE_DOWNLOAD -DSINGLE_WIRE_DOWNLOAD_UART=$(SINGLE_WIRE_DOWNLOAD_UART)
endif

ifeq ($(DIRECT_TO_SINGLE_DLD),1)
CFLAGS_flash_programmer.o += -DDIRECT_TO_SINGLE_DLD
CFLAGS_sys_api_programmer.o += -DDIRECT_TO_SINGLE_DLD
endif
