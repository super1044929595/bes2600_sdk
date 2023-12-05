cur_dir := $(dir $(lastword $(MAKEFILE_LIST)))

obj_s += $(patsubst $(cur_dir)%,%,$(wildcard $(cur_dir)*.S))
obj_c += $(patsubst $(cur_dir)%,%,$(wildcard $(cur_dir)*.c))
obj_cpp += $(patsubst $(cur_dir)%,%,$(wildcard $(cur_dir)*.cpp))

PROGRAMMER_INFLASH_LIB_NAME := libprogrammer_inflash

ifeq ($(OTA_BIN_COMPRESSED),1)
PROGRAMMER_INFLASH_LIB_NAME := $(PROGRAMMER_INFLASH_LIB_NAME)_compressed
endif

ifeq ($(SINGLE_WIRE_DOWNLOAD),1)
PROGRAMMER_INFLASH_LIB_NAME := $(PROGRAMMER_INFLASH_LIB_NAME)_single_wire_download
endif

ifeq ($(DIRECT_TO_SINGLE_DLD),1)
PROGRAMMER_INFLASH_LIB_NAME := $(PROGRAMMER_INFLASH_LIB_NAME)_direct_single_dld
endif

$(PROGRAMMER_INFLASH_LIB_NAME)-y := $(obj_c:.c=.o) $(obj_s:.S=.o) $(obj_cpp:.cpp=.o)
obj-y += $(PROGRAMMER_INFLASH_LIB_NAME).a

ccflags-y := \
	-Iutils/boot_struct

ifeq ($(SINGLE_WIRE_DOWNLOAD),1)
CFLAGS_programmer_inflash_main.o += -DSINGLE_WIRE_DOWNLOAD
endif

ifeq ($(DIRECT_TO_SINGLE_DLD),1)
CFLAGS_programmer_inflash_main.o += -DDIRECT_TO_SINGLE_DLD
endif
