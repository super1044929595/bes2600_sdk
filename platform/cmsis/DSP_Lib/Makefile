cur_dir := $(dir $(lastword $(MAKEFILE_LIST)))

obj-y := CommonTables/
obj-y += BasicMathFunctions/
obj-y += FilteringFunctions/
obj-y += TransformFunctions/
obj-y += SupportFunctions/
obj-y += StatisticsFunctions/
obj-y += FastMathFunctions/
obj-y += ComplexMathFunctions/

subdir-ccflags-y := -Iinclude/cmsis_dsp/
subdir-ccflags-y += -DARM_MATH_LOOPUNROLL

ifeq ($(SCO_CP_ACCEL),1)
cmsis_dsp_lib-y := $(obj-y)
obj-y := cmsis_dsp_lib.o
endif

