#ifndef __DIP_API__H__
#define __DIP_API__H__

#include "bluetooth.h"
#include "sdp_api.h"

#define SRC_BT (1)
#define SRC_USB (2)
#define SRC_BT_SANSUMG (0x0075)
#define SRC_BT_APPLE   (0X004c)
#define SRC_USB_APPLE   (0x05AC)
#define SRC_USB_SONY_MP3 (0X054C)

enum{
    DEVICE_TYPE_IOS,
    DEVICE_TYPE_ANDROID,
};

enum{
    DEVICE_TYPE_ANDROID_NONE = DEVICE_TYPE_ANDROID<<4,
    DEVICE_TYPE_ANDROID_SAMSUNG,
    DEVICE_TYPE_ANDROID_SONY,
};

typedef struct{
    uint16_t spec_id;
    uint16_t vend_id;
    uint16_t prod_id;
    uint16_t prod_ver;
    uint8_t  prim_rec;
    uint16_t vend_id_source;
}btif_dip_manufacture_infor_t;

#ifdef __cplusplus
extern "C" {
#endif

typedef void btif_dip_client_t;
typedef void dip_pnp_info;

typedef void (*DipApiCallBack)(bt_bdaddr_t *_addr, uint16_t device_type,bool target_get);
void dip_need_filer_target_set(uint8_t * ptr);
void dip_need_filter_target_reset(void);

void btif_dip_init(DipApiCallBack callback);
void btif_dip_clear(btif_remote_device_t *bt_dev);
bt_status_t btif_dip_query_for_service(btif_dip_client_t *client_t,btif_remote_device_t *btDevice);
uint16_t btif_dip_check_is_ios_device(btif_remote_device_t *btDevice);
void btif_dip_get_remote_info(btif_remote_device_t *btDevice);
bool btif_dip_get_process_status(btif_remote_device_t *btDevice);
void btif_dip_get_record_vend_id_and_source(bt_bdaddr_t *bdAddr, uint16_t *vend_id, uint16_t *vend_id_source);
bool btif_dip_check_is_ios_by_vend_id(uint16_t vend_id, uint16_t vend_id_source);

#ifdef __cplusplus
}
#endif

#endif
