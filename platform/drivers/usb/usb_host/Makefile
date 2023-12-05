cur_dir := $(dir $(lastword $(MAKEFILE_LIST)))

src_obj := $(patsubst $(cur_dir)%,%,$(wildcard $(cur_dir)*.c $(cur_dir)*.cpp $(cur_dir)*.S))
src_obj := $(src_obj:.c=.o)
src_obj := $(src_obj:.cpp=.o)
src_obj := $(src_obj:.S=.o)

USB_HOST_LIB_NAME := $libusbhost

ifeq ($(USE_TRACE_ID), 1)
USB_HOST_LIB_NAME := $(USB_HOST_LIB_NAME)_log_compressed
endif

$(USB_HOST_LIB_NAME)-y := $(src_obj)

obj-y := $(USB_HOST_LIB_NAME).a

ccflags-y := -Iplatform-Iplatform/drivers/usb/usb_host/inc -Iplatform/drivers/ana

ifeq ($(USB_WAKEUP_OPEN),1)
CFLAGS_USBHost.o += -DUSB_WAKEUP_OPEN
endif

ifeq ($(USB_WAKEUP_RESET),1)
CFLAGS_USBHost.o += -DUSB_WAKEUP_RESET
endif
