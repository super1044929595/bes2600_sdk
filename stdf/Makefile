obj-y := stdf_main/ stdf_app/ stdf_bsp/ stdf_hal/ stdf_os/ stdf_sdk/

obj-y += stdf_bsp/key/

#sdk include
subdir-ccflags-y += \
    -Iapps/key \
    -Iplatform/drivers/bt \
    -Iplatform/drivers/ana \
    -Iplatform/hal \
    -Iplatform/hal/best1305 \
    -Iservices/app_tws/inc \
    -Iservices/bt_if_enhanced/inc \
    -Iservices/app_ibrt/inc \
    -Iservices/multimedia/audio/codec/sbc/inc \
    -Iservices/nv_section/factory_section \
    -Iservices/tws_ibrt/inc \
    -Iutils/cqueue \
    -Iutils/heap


#stdf include
subdir-ccflags-y += \
    -Iconfig/$(T) \
    -Istdf/stdf_main \
    -Istdf/stdf_app \
    -Istdf/stdf_bsp \
    -Istdf/stdf_bsp/key \
    -Istdf/stdf_hal \
    -Istdf/stdf_os \
    -Istdf/stdf_sdk

ifeq ($(__STDF_EXTERNAL_CHAGER_ENABLE__),1)
obj-y += stdf_bsp/charger/ stdf_bsp/charger/sy5501/ stdf_bsp/charger/able_iic/

subdir-ccflags-y += \
    -Istdf/stdf_bsp/charger \
    -Istdf/stdf_bsp/charger/able_iic \
    -Istdf/stdf_bsp/charger/sy5501
endif