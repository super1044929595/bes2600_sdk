#ifndef __APP_GFPS_SASS_H__
#define __APP_GFPS_SASS_H__

#include "bt_co_list.h"
#include "btapp.h"
#include "gap.h"

#define COD_SERVICE_CLASS_AUDIO           (1 << 5)
#define COD_SERVICE_CLASS_TELEPHONY       (1 << 6)

#define COD_MAJOR_CLASS_COMPUTER          (1 << 0)
#define COD_MAJOR_CLASS_PHONE             (1 << 1)
#define COD_MAJOR_CLASS_AUD_VID           (1 << 2)
#define COD_MAJOR_CLASS_IMAGING           (3 << 1)

#define COD_MINOR_CLASS_LAPTOP            (0xc)
#define COD_MINOR_CLASS_TABLET            (0x1c)
#define COD_MINOR_CLASS_DISPLAY_SPK       (0x3c)

#define COD_TYPE_LAPTOP  (COD_SERVICE_CLASS_AUDIO | COD_MAJOR_CLASS_COMPUTER | COD_MINOR_CLASS_LAPTOP)
#define COD_TYPE_TABLET  (COD_SERVICE_CLASS_AUDIO | COD_MAJOR_CLASS_COMPUTER | COD_MINOR_CLASS_TABLET)
#define COD_TYPE_PHONE   (COD_SERVICE_CLASS_TELEPHONY | COD_MAJOR_CLASS_PHONE)
#define COD_TYPE_TV      (COD_SERVICE_CLASS_AUDIO | COD_MAJOR_CLASS_AUD_VID | COD_MINOR_CLASS_DISPLAY_SPK)

#define SASS_CONN_STATE_TYPE (0x05)
#define SASS_VERSION         (0x0101)
#define NTF_CAP_LEN          (4)

/**
 * @brief sass capability state
 *
 */
#define SASS_STATE_ON_BIT                 (1 << 15)
//1: if the device supports multipoint and it can be switched between on and off
//0: otherwise (does not support multipoint or multipoint is always on)
#define SASS_MULTIPOINT_BIT               (1 << 14)
//1: if multipoint on; 0: otherwise
#define SASS_MULTIPOINT_ON_BIT            (1 << 13)
//1: support on-head detection; 0: otherwise
#define SASS_ON_HEAD_BIT                  (1 << 12)
//1: on-head detection is turned on; 0: do not support on-head detection or is disabled
#define SASS_ON_HEAD_ON_BIT               (1 << 11)

/**
 * @brief sass switch preference, new profile request VS current active profile
 *
 */
#define SASS_A2DP_VS_A2DP_BIT             (1 << 7)  //default be 0
#define SASS_HFP_VS_HFP_BIT               (1 << 6)  //default be 0
#define SASS_A2DP_VS_HFP_BIT              (1 << 5)  //default be 0
#define SASS_HFP_VS_A2DP_BIT              (1 << 4)  //default be 1

/**
 * @brief sass switch audio source event type
 *
 */
#define SASS_SWITCH_TO_CURR_DEV_BIT       (1 << 7)
#define SASS_RESUME_ON_SWITCH_DEV_BIT     (1 << 6)
#define SASS_REJECT_SCO_ON_AWAY_DEV_BIT   (1 << 5)
#define SASS_DISCONN_ON_AWAY_DEV_BIT      (1 << 4)

#define HISTORY_DEV_NUM                   (2)
#define SASS_ACCOUNT_KEY_SIZE             (16)
#define SESSION_NOUNCE_NUM                (8)

/**
 * @brief The head state
 *
 */
#define SASS_STATE_BIT_HEAD_ON          (7)
#define SASS_STATE_BIT_CONN_AVAILABLE   (6)
#define SASS_STATE_BIT_FOCUS_MODE       (5)
#define SASS_STATE_BIT_AUTO_RECONN      (4)
#define SASS_STATE_BIT_CONN_STATE       (0xF)

#define SET_SASS_STATE(S, B, P)       (S = ((S & ~(1 << SASS_STATE_BIT_##B)) | (P << SASS_STATE_BIT_##B)))
#define SET_SASS_CONN_STATE(S, B, P)  (S = ((S & ~(SASS_STATE_BIT_##B)) | P))


/**
 * @brief The first byte of account key
 *
 */
#define SASS_NOT_IN_USE_ACCOUNT_KEY         (0x04)
#define SASS_RECENTLY_USED_ACCOUNT_KEY      (0x05)
#define SASS_IN_USE_ACCOUNT_KEY             (0x06)

#define NODE_BD_ADDR(n) ((bd_addr_t *)((uint8_t *)n + sizeof(struct list_node)))

typedef uint8_t SASS_PROFILE_STATUS_E;
#define SASS_PROFILE_STATE_BIT_CONNECTION       (0) //bit0, 1: connected; 0: disconnected
#define SASS_PROFILE_STATE_BIT_AUDIO            (1) //bit1, 1: audio playing; 0: paused or stopped

#define SASS_PROFILE_BIT_A2DP             (0)
#define SASS_PROFILE_BIT_AVRCP            (4)
#define SASS_PROFILE_BIT_HFP              (8)

#define UPDATE_ALL_STATE(n, s, b)    ((n & ~(0xF << b)) | (s << b))
#define UPDATE_PROFILE_STATE(s, v)   (v << SASS_PROFILE_STATE_BIT_##s)
#define PROFILE_STATE(p, s)          ((1 << SASS_PROFILE_STATE_BIT_##s) << SASS_PROFILE_BIT_##p)

/**
 * @brief switch back event type
 *
 */
typedef enum
{
    SASS_EVT_SWITCH_BACK = 1,
    SASS_EVT_SWITCH_BACK_AND_RESUME,
    SASS_EVT_INVALID = 0xFFFF,
} SASS_BACK_EVT_E;

/**
 * @brief switch reason
 *
 */
typedef enum
{
    SASS_REASON_UNSPECIFIED = 0,
    SASS_REASON_A2DP,
    SASS_REASON_HFP,
} SASS_REASON_E;

/**
 * @brief switch device
 *
 */
typedef enum
{
    SASS_DEV_THIS_DEVICE = 0x01,
    SASS_DEV_ANOTHER,
} SASS_SWITCH_DEV_E;

/**
 * @brief device type of the connected seeker
 *
 */
typedef enum
{
    SASS_DEV_TYPE_LAPTOP = 1 << 7,
    SASS_DEV_TYPE_PHONEA = 1 << 6,
    SASS_DEV_TYPE_PHONEB = 1 << 5,
    SASS_DEV_TYPE_TABLET = 1 << 4,
    SASS_DEV_TYPE_TV     = 1 << 3,
    SASS_DEV_TYPE_INVALID = 0xFFFF,
} SASS_DEV_TYPE_E;

/**
 * @brief The connection or audio state of the provider
 *
 */
typedef enum
{
    SASS_STATE_NO_CONNECTION = 0,
    SASS_STATE_PAGING,
    SASS_STATE_NO_DATA,
    SASS_STATE_NO_AUDIO,
    SASS_STATE_ONLY_A2DP,
    SASS_STATE_A2DP_WITH_AVRCP,
    SASS_STATE_HFP,
    SASS_STATE_LE_AUD_MEDIA_CTRL,
    SASS_STATE_LE_AUD_MEDIA_WITHOUT_CTRL,
    SASS_STATE_LE_AUD_CALL,
    SASS_STATE_LE_AUD_BIS,
    SASS_STATE_DISABLE_SWITCH = 0xF,
} SASS_CONN_STATE_E;

typedef enum
{
    SASS_HEAD_STATE_OFF = 0,
    SASS_HEAD_STATE_ON,
} SASS_HEAD_STATE_E;

typedef enum
{
    SASS_CONN_NONE_AVAILABLE = 0,
    SASS_CONN_AVAILABLE,
} SASS_CONN_AVAIL_E;

typedef enum
{
    SASS_CONN_NO_FOCUS = 0,
    SASS_CONN_FOCUS_MODE,

} SASS_FOCUS_MODE_E;

typedef enum
{
    SASS_MANTUAL_CONNECTED = 0,
    SASS_AUTO_RECONNECTED,

} SASS_AUTO_RECONN_E;

/**
 * @brief return status
 *
 */
typedef enum
{
    SASS_STATUS_OK = 0,
    SASS_STATUS_REDUNTANT,
    SASS_STATUS_FAIL,
} SASS_STATUS_E;

typedef enum
{
    SASS_PROFILE_A2DP = 1,
    SASS_PROFILE_AVRCP,
    SASS_PROFILE_HFP,
    SASS_PROFILE_INVALID = 0xFF,
} SASS_PROFILE_ID_E;

typedef struct {
    uint8_t                  connId;
    SASS_CONN_STATE_E        state;
    uint16_t                 audState;//bit0~3: a2dp; bit4~7:avrcp; bit8~11:hfp; 
    uint16_t                 sassVer;
    SASS_DEV_TYPE_E          devType;
    bool                     initbySass; //connection is triggeed by sass or not
    bd_addr_t                 btAddr;
    uint8_t                  accKey[SASS_ACCOUNT_KEY_SIZE];
    uint8_t                  session[SESSION_NOUNCE_NUM];
	bool                     updated;
} SassBtInfo;

typedef struct
{
    struct list_node    node;
    bd_addr_t            addr;
} SassDev;

typedef struct
{
    bd_addr_t                    reconnAddr;
    uint8_t                     evt;
} SassReconnectInfo;

typedef struct {
    SassBtInfo             connInfo[BT_DEVICE_NUM];
    uint8_t                preference;
    uint8_t                connNum;
    uint8_t                activeId;
    bool                   idSwitched;
    bool                   isMulti;
    struct list_node       hDevHead;
    SassDev                historyDev[HISTORY_DEV_NUM];
    bd_addr_t              lastDev;
    bd_addr_t              dropDevAddr;
    SassReconnectInfo      reconnInfo;
    SASS_HEAD_STATE_E      headState;
    SASS_CONN_AVAIL_E      connAvail;
    SASS_FOCUS_MODE_E      focusMode;
    SASS_AUTO_RECONN_E     autoReconn;
    SASS_CONN_STATE_E      connState;
    uint8_t                inuseKey[SASS_ACCOUNT_KEY_SIZE];
} SassConnInfo;

typedef struct {
    SassBtInfo             connInfo[BT_DEVICE_NUM];
    uint8_t                activeId;
} SassSyncInfo;

/**
 * @brief The event for update adv data
 *
 */
typedef enum
{
    SASS_EVT_ADD_DEV = 0,
    SASS_EVT_DEL_DEV,
    SASS_EVT_UPDATE_CONN_STATE,
    SASS_EVT_UPDATE_HEAD_STATE,
    SASS_EVT_UPDATE_FOCUS_STATE,
    SASS_EVT_UPDATE_RECONN_STATE,
    SASS_EVT_UPDATE_CUSTOM_DATA,
    SASS_EVT_UPDATE_MULTI_STATE,
} SASS_UPDATE_EVT_E;

typedef struct {
    SASS_UPDATE_EVT_E event;
    uint8_t   devId;
    uint8_t   reason;
    bd_addr_t  addr;
    union
    {
        SASS_CONN_AVAIL_E       connAvail;
        SASS_CONN_STATE_E       connState;
        SASS_HEAD_STATE_E       headState;
        SASS_FOCUS_MODE_E       focusMode;
        SASS_AUTO_RECONN_E      autoReconn;
        uint8_t                 cusData;
    } state;
} SassEvtParam;

typedef struct {
/*0bLLLLTTTT: L = length of connection status in bytes, type = 0b0101*/
    uint8_t lenType;
/* Connection state:0bHAFRSSSS  
*  H = on head detection; A = connection availability;
*  F = focus mode; R = auto-connected; S = connection state
*/
    uint8_t state;
/* custom data is sent from the Seeker of current active streaming to the provider via message stream.
*  0 if the current active streaming is not from Seeker.
*/
    uint8_t cusData;
/* 1: device is connected to the provider,
*  0: otherwise
*/
    uint8_t devBitMap;
} SassAdvInfo;

#ifdef __cplusplus
extern "C" {
#endif

void app_fp_sass_init();
void app_fp_sass_get_adv_data(uint8_t *buf, uint8_t *len);
void app_fp_sass_set_custom_data(uint8_t data);
void app_fp_sass_get_cap(uint8_t *buf, uint32_t *len);
void app_fp_sass_set_switch_pref(uint8_t pref);
uint8_t app_fp_sass_get_switch_pref();
void app_fp_sass_set_active_dev(uint8_t dev_id);
uint8_t app_fp_sass_get_active_dev();
void app_fp_sass_set_last_dev(bd_addr_t *addr);
bd_addr_t *app_fp_sass_get_last_dev();
uint8_t app_fp_sass_switch_src_evt_hdl(uint8_t device_id, uint8_t evt);
uint8_t app_fp_sass_switch_back_hdl(uint8_t device_id, uint8_t evt);
void app_fp_sass_set_conn_state(SASS_CONN_STATE_E state);
SASS_HEAD_STATE_E app_fp_sass_get_head_state();
void app_fp_sass_set_head_state(SASS_HEAD_STATE_E headstate);
void app_fp_sass_set_conn_available(SASS_CONN_AVAIL_E available);
void app_fp_sass_set_focus_mode(SASS_FOCUS_MODE_E focus);
void app_fp_sass_set_auto_reconn(SASS_AUTO_RECONN_E focus);
void app_fp_sass_set_init_conn(uint8_t device_id, bool bySass);
void app_fp_sass_set_inuse_acckey_by_dev(uint8_t device_id, uint8_t *accKey);
void app_fp_sass_set_inuse_acckey(uint8_t *accKey);
void app_fp_sass_get_inuse_acckey(uint8_t *accKey);
bool app_fp_sass_get_inuse_acckey_by_id(uint8_t device_id, uint8_t *accKey);
void app_fp_sass_set_drop_dev(uint8_t device_id);
void app_fp_sass_connect_handler(uint8_t device_id, bd_addr_t *addr);
void app_fp_sass_disconnect_handler(uint8_t device_id, bd_addr_t *addr, uint8_t errCode);
void app_fp_sass_update_state(SassEvtParam *evtParam);
bool app_fp_sass_is_need_resume(bd_addr_t *addr);
void app_fp_sass_reset_reconn_info();
void app_fp_sass_profile_event_handler(SASS_PROFILE_ID_E pro, uint8_t devId, uint8_t event, uint8_t *param);
SassBtInfo *app_fp_sass_get_other_connected_dev(uint8_t id);
void app_fp_sass_gen_session_nonce(uint8_t device_id);
bool app_fp_sass_get_session_nonce(uint8_t device_id, uint8_t *session);
bool app_fp_sass_is_sass_dev(uint8_t device_id);
bool app_fp_sass_is_there_sass_dev();
void app_fp_sass_set_sass_mode(uint8_t device_id, uint16_t sassVer);
void app_fp_sass_encrypt_adv_data(uint8_t *FArray, uint8_t sizeOfFilter, uint8_t *inUseKey, 
                                               uint8_t *outputData, uint8_t *dataLen);
void app_fp_sass_sync_info();
void app_fp_sass_get_sync_info(uint8_t *buf, uint32_t *len);
void app_fp_sass_set_sync_info(uint8_t *buf, uint32_t len);
SassBtInfo *app_fp_sass_get_connected_dev(uint8_t id);
void app_fp_sass_set_multi_status(uint8_t device_id, bool isMulti);
bool app_fp_sass_get_multi_status();

#ifdef __cplusplus
}
#endif

#endif  // __APP_GFPS_SASS_H__
