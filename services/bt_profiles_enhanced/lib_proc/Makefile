cur_makefile := $(lastword $(MAKEFILE_LIST))

$(cur_makefile): ;

ifeq ($(GEN_LIB),1)

ifeq ($(GEN_LIB_CUSTOM_DEMO),1)

archive-custom += $(BTPROFILE_LIB_NAME).a

$(srctree)/$(src)/lib/$(BTPROFILE_LIB_NAME).a: $(obj)/$(BTPROFILE_LIB_NAME)_ROUND3
	@$(call create_dir,$(dir $(@)))
	$(call if_changed,use_lib_file)

$(obj)/$(BTPROFILE_LIB_NAME)_ROUND3: $(obj)/$(BTPROFILE_LIB_NAME)_ROUND2 FORCE
	$(call if_changed,use_lib_file)

$(obj)/$(BTPROFILE_LIB_NAME)_ROUND2: $(obj)/$(BTPROFILE_LIB_NAME)_ROUND1 FORCE
	$(call if_changed,use_lib_file)

$(obj)/$(BTPROFILE_LIB_NAME)_ROUND1: $(obj)/$(BTPROFILE_LIB_NAME)_ROUND0 FORCE
	$(call if_changed,use_lib_file)

$(obj)/$(BTPROFILE_LIB_NAME)_ROUND0: $(obj)/$(BTPROFILE_LIB_NAME).a FORCE
	$(call if_changed,use_lib_file)

targets += $(BTPROFILE_LIB_NAME)_ROUND0 $(BTPROFILE_LIB_NAME)_ROUND1 $(BTPROFILE_LIB_NAME)_ROUND2 $(BTPROFILE_LIB_NAME)_ROUND3

endif

endif

