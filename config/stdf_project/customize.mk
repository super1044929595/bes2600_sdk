## framework define
export __STDF__ ?= 1
ifeq ($(__STDF__),1)
KBUILD_CPPFLAGS += -D__STDF__
endif

## general infomation define
export STDF_SW_VERSION ?= 0.0.0.1
export STDF_HW_VERSION ?= 1.0

## custom falsh section size
STDF_RESERVED_0_SECTION_SIZE ?= 0x1000
STDF_RESERVED_1_SECTION_SIZE ?= 0x1000
KBUILD_CPPFLAGS += -DSTDF_RESERVED_0_SECTION_SIZE=$(STDF_RESERVED_0_SECTION_SIZE)
KBUILD_CPPFLAGS += -DSTDF_RESERVED_1_SECTION_SIZE=$(STDF_RESERVED_1_SECTION_SIZE)

## use function switch
export __STDF_EXTERNAL_CHAGER_ENABLE__          ?= 1





## user function switch and related functions
ifeq ($(__STDF_EXTERNAL_CHAGER_ENABLE__), 1)
    KBUILD_CPPFLAGS += -D__STDF_EXTERNAL_CHAGER_ENABLE__
    KBUILD_CPPFLAGS += -D__STDF_HAL_PMU_ENABLE__
endif

