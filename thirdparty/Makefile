obj-y := userapi/

ifeq ($(THIRDPARTY_LIB),)
THIRDPARTY_LIB := demo
endif

obj-y += $(foreach lib,$(THIRDPARTY_LIB),$(lib)_lib/)

ifeq ($(USE_LOWLATENCY_LIB),1)
obj-y += lowlatency_lib/
endif

ifeq ($(TILE_DATAPATH_ENABLED), 1)
obj-y += tile
endif

ifeq ($(QIOT_DATAPATH_ENABLE), 1)
obj-y += qiot
endif
