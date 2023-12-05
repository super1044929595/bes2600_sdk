obj-y := audio_dump/ audioflinger/ audio_process/ hw_dsp/ bt_app/ overlay/ resources/ \
			../utils/crc32/ ../utils/heap/ osif/ norflash_api/

ifeq ($(BONE_SENSOR_TDM),1)
obj-y += bone_sensor/
endif

ifneq ($(filter 1,$(INTERCONNECTION) $(INTERACTION) $(AI_VOICE)),)
obj-y += interconnection/
endif

obj-y += bt_profiles_enhanced/
obj-y += bt_if_enhanced/

ifeq ($(MBED),1)
obj-y += fs/
endif

ifneq ($(MBED)_$(AI_VOICE),0_0)
obj-y +=../utils/kfifo/
endif

ifeq ($(VOICE_DATAPATH_ENABLED),1)
obj-y += voicepath/
obj-y += voicepath/$(VOICE_DATAPATH_TYPE)/
endif

ifeq ($(TILE_DATAPATH_ENABLED),1)
obj-y += ../thirdparty/tile/
endif

ifeq ($(QIOT_DATAPATH_ENABLE),1)
obj-y += ../thirdparty/qiot/
endif

ifeq ($(BLE),1)
obj-y += ble_app/ bridge/ ble_stack/ ../utils/retention_ram/ ble_profiles/
endif

ifeq ($(AUTO_TEST),1)
obj-y += auto_test/
endif

obj-y += nv_section/

ifneq ($(FPGA),1)
obj-y += nvrecord/
endif

ifeq ($(VOICE_RECOGNITION),1)
obj-y += voice_recognition/
endif

ifeq ($(APP_ANC_TEST),1)
obj-y += anc_spp_tool/
endif

ifeq ($(TEST_OVER_THE_AIR),1)
obj-y += tota/
endif

ifeq ($(TEST_OVER_THE_AIR_v2),1)
obj-y += tota_v2/
endif

ifeq ($(BES_OTA_BASIC),1)
obj-y += ota/
endif

ifeq ($(AI_VOICE),1)
obj-y += ai_voice/
endif

ifeq ($(IBRT),1)
obj-y += tws_ibrt/
obj-y += app_ibrt/
obj-y += app_tws/
endif

ifeq ($(RSA_SHA),1)
obj-y +=../utils/sha256/ ../utils/rsa2048/ 
endif

ifeq ($(AES256_FOR_BIN),1)
obj-y +=../utils/aes256/
endif

obj-y += communication/
ifeq ($(A2DP_LHDC_ON),1)
obj-y += lhdc_license/
endif