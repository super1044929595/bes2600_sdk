CHIP		?= best1305

DEBUG		?= 0

NOSTD		?= 1

LIBC_ROM	?= 1

PROGRAMMER	:= 1

FAULT_DUMP	?= 0

WATCHER_DOG ?= 1

init-y		:=
core-y		:= tests/programmer/ platform/cmsis/ platform/hal/
core-y      += tests/programmer_inflash/

ifeq ($(OTA_BIN_COMPRESSED),1)
core-y		+= utils/lzma/
core-y		+= utils/heap/

KBUILD_CPPFLAGS += -Iutils/lzma/
KBUILD_CPPFLAGS += -Iutils/heap/

export OTA_REBOOT_FLASH_REMAP := 0
endif

LDS_FILE	:= programmer_inflash.lds

export CRC32_ROM ?= 1

KBUILD_CPPFLAGS += -Iplatform/cmsis/inc -Iplatform/hal

KBUILD_CPPFLAGS += -DPROGRAMMER_INFLASH \

KBUILD_CFLAGS +=

LIB_LDFLAGS +=

CFLAGS_IMAGE +=

LDFLAGS_IMAGE +=

# KBUILD_CPPFLAGS += -DNEW_IMAGE_FLASH_OFFSET=0x100000 \
#                    -D__APP_IMAGE_FLASH_OFFSET__=0x10000

KBUILD_CPPFLAGS += -DDOWNLOAD_UART_BANDRATE=921600
                #    -DAPP_ENTRY_ADDRESS=0x3c010000 

#export SINGLE_WIRE_DOWNLOAD ?= 1
#KBUILD_CPPFLAGS += -DUNCONDITIONAL_ENTER_SINGLE_WIRE_DOWNLOAD

export FLASH_UNIQUE_ID ?= 1
export TRACE_BAUD_RATE := 10*115200

