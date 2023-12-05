cur_dir := $(dir $(lastword $(MAKEFILE_LIST)))

obj_c := $(patsubst $(cur_dir)%,%,$(wildcard $(cur_dir)gfps_crypto/*.c))

CRYPTO_LIB_NAME := $libgfps_crypto

ifeq ($(USE_TRACE_ID), 1)
CRYPTO_LIB_NAME := $(CRYPTO_LIB_NAME)_log_compressed
endif

$(CRYPTO_LIB_NAME)-y := $(obj_c:.c=.o)
obj-y := $(CRYPTO_LIB_NAME).a

subdir-ccflags-y += \
	-Igfps_crypto/ \
	-Iutils/encrypt \
	-Iapi/ \
	-Iservices/ble_app/app_gfps 



