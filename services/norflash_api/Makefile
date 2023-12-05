cur_dir := $(dir $(lastword $(MAKEFILE_LIST)))

obj-y := $(patsubst $(cur_dir)%,%,$(wildcard $(cur_dir)*.c $(cur_dir)*.cpp $(cur_dir)*.S))
obj-y := $(obj-y:.c=.o)
obj-y := $(obj-y:.cpp=.o)
obj-y := $(obj-y:.S=.o)

ccflags-y += \
    -Iplatform/hal \
    -Iplatform/drivers/ana \
    -Iplatform/drivers/norflash \
    -Iutils/heap

ifeq ($(FLASH_SUSPEND),1)
CFLAGS_norflash_api.o += -DFLASH_SUSPEND
endif

ifeq ($(PUYA_FLASH_ERASE_PAGE_ENABLE),1)
CFLAGS_norflash_api.o += -DPUYA_FLASH_ERASE_PAGE_ENABLE
endif

ifeq ($(USB_AUDIO_APP),1)
ifneq (,$(filter $(BTUSB_AUDIO_MODE) $(BT_USB_AUDIO_DUAL_MODE), 1))
export FLASH_API_SIMPLE ?= 1
endif
endif

ifeq ($(FLASH_API_SIMPLE),1)
CFLAGS_norflash_api.o += -DFLASH_API_SIMPLE
else
ifeq ($(FLASH_API_HIGHPERFORMANCE),1)
CFLAGS_norflash_api.o += -DFLASH_API_HIGHPERFORMANCE
else
CFLAGS_norflash_api.o += -DFLASH_API_NORMAL
endif
endif

