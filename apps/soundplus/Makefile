cur_dir := $(dir $(lastword $(MAKEFILE_LIST)))

obj-y := $(patsubst $(cur_dir)%,%,$(wildcard $(cur_dir)*.c $(cur_dir)*.cpp $(cur_dir)*.S))
obj-y := $(obj-y:.c=.o)
obj-y := $(obj-y:.S=.o)
obj-y := $(obj-y:.cpp=.o)

obj-y += 2500iu_sp_ec_framework_2.1_v3_iu_bes2500_2_0_0_e2fc7b46.a

ccflags-y := \
    -Iplatform/cmsis/inc  \
	-Iplatform/hal \
	-Iservices/bt_app \
	-Iutils/list \
	-Iutils/heap \
	-Iservices/bt_profiles/inc

	
	
