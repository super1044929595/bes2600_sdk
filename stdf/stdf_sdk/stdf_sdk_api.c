/*******************************************************************************
  Filename:       .

  Version         V1.0.

  Author          Yuping.Mo.

  Description:    .


  IMPORTANT:      .

*******************************************************************************/

/*******************************************************************************
 * INCLUDES
 */
#include "stdf_define.h"
#include "stdf_os.h"
#include "stdf_sdk_api.h"

#include "app_ibrt_if.h"
#include "app_ibrt_ui.h"
#include "app_tws_besaud.h"
#include "app_tws_ibrt.h"
#include "app_tws_if.h"
#include "cmsis_os.h"
#include "factory_section.h"
#include "hal_bootmode.h"
#include "hal_cmu.h"
#include "heap_api.h"
#include "pmu.h"
#include "nvrecord_bt.h"

/*******************************************************************************
 * MACROS
 */
// Only one define can be selected
//#define STDF_SDK_API_EARSIDE_USE_GPIO
#define STDF_SDK_API_EARSIDE_USE_BT_ADDR
//STDF_SDK_API_EARSIDE_USE_FLASH


#define STDF_SDK_API_LOG(str, ...)          STDF_LOG("[SDK][API] %s "str, __func__, ##__VA_ARGS__)
#define STDF_SDK_API_ASSERT(cond)           STDF_ASSERT(cond)

/*******************************************************************************
 * TYPEDEFS
 */

/*******************************************************************************
* GLOBAL VARIABLES
*/
stdf_sdk_api_power_reason_t stdf_sdk_api_power_reason = STDF_SDK_API_POWER_REASON_UNKOWN;

/*******************************************************************************
 * EXTERNAL VARIABLES
 */

/*******************************************************************************
 * FUNCTIONS
 */

/* -----------------------------------------------------------------------------
 *                                   System
 * ---------------------------------------------------------------------------*/

/*******************************************************************************
 * @brief   .
 */
void stdf_sdk_api_sys_delay_ms(uint32_t ms)
{
    osDelay(ms);
}

/*******************************************************************************
 * @brief   .
 */
void stdf_sdk_api_sys_power_off(void)
{
    pmu_shutdown();
}

/*******************************************************************************
 * @brief   .
 */
void stdf_sdk_api_sys_reset(void)
{
    hal_cmu_sys_reboot();
}

/*******************************************************************************
 * @brief   .
 */
void stdf_sdk_api_sys_reset_enter_1wire_download(void)
{
    hal_sw_bootmode_clear(HAL_SW_BOOTMODE_REBOOT);
    hal_sw_bootmode_set(HAL_SW_BOOTMODE_SINGLE_LINE_DOWNLOAD);
    stdf_sdk_api_sys_reset();
}

/* -----------------------------------------------------------------------------
 *                                   power on/off
 * ---------------------------------------------------------------------------*/

void stdf_sdk_api_power_reason_init(void)
{
    uint32_t bootmode = hal_sw_bootmode_get();
    stdf_sdk_api_power_reason_t power_reason;
    
    if (bootmode & STDF_SDK_API_BOOTMODE_TWS_PAIRING)
    {
        hal_sw_bootmode_clear(STDF_SDK_API_BOOTMODE_TWS_PAIRING);
        power_reason = STDF_SDK_API_POWER_REASON_TWS_PAIRING;
    }
    else if (bootmode & STDF_SDK_API_BOOTMODE_FREEMAN_PAIRING)
    {
        hal_sw_bootmode_clear(STDF_SDK_API_BOOTMODE_FREEMAN_PAIRING);
        power_reason = STDF_SDK_API_POWER_REASON_FREEMAN_PAIRING;
    }
    else
    {
        power_reason = STDF_SDK_API_POWER_REASON_NORMAL;
    }

    STDF_SDK_API_LOG("bootmode %x power_reason %d", bootmode, power_reason);

    stdf_sdk_api_power_reason_set(power_reason);
}

/*******************************************************************************
 * @brief   .
 */
stdf_sdk_api_power_reason_t stdf_sdk_api_power_reason_get(void)
{
    return stdf_sdk_api_power_reason;
}

/*******************************************************************************
 * @brief   .
 */
void stdf_sdk_api_power_reason_set(stdf_sdk_api_power_reason_t reason)
{
    if(reason != stdf_sdk_api_power_reason)
    {
        STDF_SDK_API_LOG("reason %d -> %d", stdf_sdk_api_power_reason, reason);
        stdf_sdk_api_power_reason = reason;
    }
}

/*******************************************************************************
 * @brief   .
 */
void stdf_sdk_api_power_reason_check(void)
{
    STDF_SDK_API_LOG("reason %d", stdf_sdk_api_power_reason);
    
    switch(stdf_sdk_api_power_reason)
    {
        case STDF_SDK_API_POWER_REASON_NORMAL:
            break;
        
        case STDF_SDK_API_POWER_REASON_TWS_PAIRING:
            stdf_sdk_api_enter_tws_pairing();
            break;
        
        case STDF_SDK_API_POWER_REASON_FREEMAN_PAIRING:
            stdf_sdk_api_enter_freeman_pairing();
            break;

        default:
            break;
    }
}

/* -----------------------------------------------------------------------------
 *                                   PHY
 * ---------------------------------------------------------------------------*/

/*******************************************************************************
 * @brief   .
 */
static void stdf_sdk_api_phy_init_msg_handler(stdf_os_msg_id_t msg_id, void *payload)
{
    enum PMU_CHARGER_STATUS_T status = pmu_charger_get_status();

    STDF_SDK_API_LOG("status %d", status);
    
    if(status != PMU_CHARGER_PLUGIN)
    {
        stdf_sdk_api_phy_set_state_in_case_open();
    }
}

/*******************************************************************************
 * @brief   .
 */
void stdf_sdk_api_phy_init_state(void)
{
    MessageSendLater(stdf_sdk_api_phy_init_msg_handler, 0x00, NULL, 1000);
}

/*******************************************************************************
 * @brief   .
 */
bool stdf_sdk_api_phy_is_state_in_case_close(bool local)
{
    if(local)
    {
        return (app_ibrt_ui_get_ctx()->box_state == IBRT_IN_BOX_CLOSED);
    }
    else
    {
        return (app_ibrt_ui_get_ctx()->peer_tws_info.box_state == IBRT_IN_BOX_CLOSED);
    }
}

/*******************************************************************************
 * @brief   .
 */
bool stdf_sdk_api_phy_is_state_in_case_open(bool local)
{
    if(local)
    {
        return (app_ibrt_ui_get_ctx()->box_state == IBRT_IN_BOX_OPEN);
    }
    else
    {
        return (app_ibrt_ui_get_ctx()->peer_tws_info.box_state == IBRT_IN_BOX_OPEN);
    }
}

/*******************************************************************************
 * @brief   .
 */
bool stdf_sdk_api_phy_is_state_out_case_ear(bool local)
{
    if(local)
    {
        return (app_ibrt_ui_get_ctx()->box_state == IBRT_OUT_BOX);
    }
    else
    {
        return (app_ibrt_ui_get_ctx()->peer_tws_info.box_state == IBRT_OUT_BOX);
    }
}

/*******************************************************************************
 * @brief   .
 */
bool stdf_sdk_api_phy_is_state_in_ear(bool local)
{
    if(local)
    {
        return (app_ibrt_ui_get_ctx()->box_state == IBRT_OUT_BOX_WEARED);
    }
    else
    {
        return (app_ibrt_ui_get_ctx()->peer_tws_info.box_state == IBRT_OUT_BOX_WEARED);
    }
}

/*******************************************************************************
 * @brief   .
 */
void stdf_sdk_api_phy_set_state_in_case_close(void)
{
    ibrt_box_state box_state = app_ibrt_ui_get_ctx()->box_state;
    ibrt_box_state box_state_future = app_ibrt_ui_get_ctx()->box_state_future;

    STDF_SDK_API_LOG("box_state (%d %d) -> %d", box_state, box_state_future, IBRT_IN_BOX_CLOSED);
    if(box_state == IBRT_BOX_UNKNOWN)
    {
        STDF_SDK_API_LOG("init!!!");
        app_ibrt_ui_event_entry(IBRT_CLOSE_BOX_EVENT);
        stdf_sdk_api_power_reason_set(STDF_SDK_API_POWER_REASON_NORMAL);
    }
    else if((box_state_future == IBRT_IN_BOX_OPEN) || 
        ((box_state_future != IBRT_IN_BOX_OPEN) && (box_state == IBRT_IN_BOX_OPEN)))
    {
        app_ibrt_ui_event_entry(IBRT_CLOSE_BOX_EVENT); 
        stdf_sdk_api_power_reason_set(STDF_SDK_API_POWER_REASON_NORMAL);
    }
    else
    {
        STDF_SDK_API_LOG("fail!!!");
    }
}

/*******************************************************************************
 * @brief   .
 */
void stdf_sdk_api_phy_set_state_in_case_open(void)
{
    ibrt_box_state box_state = app_ibrt_ui_get_ctx()->box_state;
    ibrt_box_state box_state_future = app_ibrt_ui_get_ctx()->box_state_future;

    STDF_SDK_API_LOG("box_state (%d %d) -> %d", box_state, box_state_future, IBRT_IN_BOX_OPEN);
    if(box_state == IBRT_BOX_UNKNOWN)
    {
        STDF_SDK_API_LOG("init!!!");
        app_ibrt_ui_event_entry(IBRT_OPEN_BOX_EVENT);    
    }     
    else if((box_state_future == IBRT_IN_BOX_CLOSED) || 
        ((box_state_future != IBRT_IN_BOX_CLOSED) && (box_state == IBRT_IN_BOX_CLOSED)))
    {
        if(stdf_sdk_api_power_reason_get() == STDF_SDK_API_POWER_REASON_NORMAL)
        {
            app_ibrt_ui_event_entry(IBRT_OPEN_BOX_EVENT);
        }
        else
        {
            stdf_sdk_api_power_reason_check();
        }
    }
    else if((box_state_future == IBRT_OUT_BOX) || 
        ((box_state_future != IBRT_OUT_BOX) && (box_state == IBRT_OUT_BOX)))
    {
        app_ibrt_ui_event_entry(IBRT_PUT_IN_EVENT);
    }
    else
    {
        STDF_SDK_API_LOG("fail!!!");
    }
}

/*******************************************************************************
 * @brief   .
 */
void stdf_sdk_api_phy_set_state_out_case_ear(void)
{
    ibrt_box_state box_state = app_ibrt_ui_get_ctx()->box_state;
    ibrt_box_state box_state_future = app_ibrt_ui_get_ctx()->box_state_future;

    STDF_SDK_API_LOG("box_state (%d %d) -> %d", box_state, box_state_future, IBRT_OUT_BOX);
    if(box_state == IBRT_BOX_UNKNOWN)
    {
        STDF_SDK_API_LOG("init!!!");
        app_ibrt_ui_event_entry(IBRT_FETCH_OUT_EVENT);   
    }    
    else if((box_state_future == IBRT_IN_BOX_OPEN) || 
        ((box_state_future == IBRT_BOX_UNKNOWN) && (box_state == IBRT_IN_BOX_OPEN)))
    {
        app_ibrt_ui_event_entry(IBRT_FETCH_OUT_EVENT);    
    }
    else if((box_state_future == IBRT_OUT_BOX_WEARED) || 
        ((box_state_future != IBRT_OUT_BOX_WEARED) && (box_state == IBRT_OUT_BOX_WEARED)))
    {
        app_ibrt_ui_event_entry(IBRT_WEAR_DOWN_EVENT);
    }
    else
    {
        STDF_SDK_API_LOG("fail!!!");
    }    
}

/*******************************************************************************
 * @brief   .
 */
void stdf_sdk_api_phy_set_state_in_ear(void)
{
    ibrt_box_state box_state = app_ibrt_ui_get_ctx()->box_state;
    ibrt_box_state box_state_future = app_ibrt_ui_get_ctx()->box_state_future;

    STDF_SDK_API_LOG("box_state (%d %d) -> %d", box_state, box_state_future, IBRT_OUT_BOX_WEARED);
    if(box_state == IBRT_BOX_UNKNOWN)
    {
        STDF_SDK_API_LOG("init!!!");
        app_ibrt_ui_event_entry(IBRT_WEAR_UP_EVENT);    
    }
    else if((box_state_future == IBRT_OUT_BOX) || 
        ((box_state_future != IBRT_OUT_BOX) && (box_state == IBRT_OUT_BOX)))
    {
        app_ibrt_ui_event_entry(IBRT_WEAR_UP_EVENT);
    }
    else
    {
        STDF_SDK_API_LOG("fail!!!");
    }
}

/* -----------------------------------------------------------------------------
 *                                   BT
 * ---------------------------------------------------------------------------*/

/*******************************************************************************
 * @brief   .
 */
void stdf_sdk_api_enter_tws_pairing(void)
{
    STDF_SDK_API_LOG("");
    
#ifdef IBRT_SEARCH_UI
    if(app_tws_is_left_side())
    {
        app_ibrt_enter_limited_mode();
    }
    else if(app_tws_is_right_side())
    {
        app_ibrt_enter_limited_mode();
        app_start_tws_serching_direactly();
    }
    else
    {
        STDF_SDK_API_LOG("unknown earside exit tws pairing!!!");
    }
#endif
}

/*******************************************************************************
 * @brief   .
 */
void stdf_sdk_api_reset_enter_tws_pairing(void)
{
    STDF_SDK_API_LOG("");
    hal_sw_bootmode_set(STDF_SDK_API_BOOTMODE_TWS_PAIRING);
    stdf_sdk_api_sys_reset();
}

/*******************************************************************************
 * @brief   .
 */
void stdf_sdk_api_enter_freeman_pairing(void)
{
    STDF_SDK_API_LOG("");
    app_ibrt_if_enter_freeman_pairing();
}

/*******************************************************************************
 * @brief   .
 */
void stdf_sdk_api_reset_enter_freeman_pairing(void)
{
    STDF_SDK_API_LOG("");
    hal_sw_bootmode_set(STDF_SDK_API_BOOTMODE_FREEMAN_PAIRING);
    stdf_sdk_api_sys_reset();
}

/*******************************************************************************
 * @brief   .
 */
bool stdf_sdk_api_is_tws_pairing(void)
{
	ibrt_ctrl_t *p_ibrt_ctrl = app_tws_ibrt_get_bt_ctrl_ctx();
    return (p_ibrt_ctrl->access_mode == BTIF_BAM_LIMITED_ACCESSIBLE);
}

/*******************************************************************************
 * @brief   .
 */
bool stdf_sdk_api_is_bt_pairing(void)
{
	ibrt_ctrl_t *p_ibrt_ctrl = app_tws_ibrt_get_bt_ctrl_ctx();
    return (p_ibrt_ctrl->access_mode == BTIF_BAM_GENERAL_ACCESSIBLE);
}

/*******************************************************************************
 * @brief   .
 */
bool stdf_sdk_api_is_tws_connected(void)
{
    return tws_besaud_is_connected();
	//return app_tws_ibrt_tws_link_connected();
}

/*******************************************************************************
 * @brief   .
 */
bool stdf_sdk_api_is_bt_connected(void)
{
    return app_tws_ibrt_mobile_link_connected();
}

/*******************************************************************************
 * @brief   .
 */
bool stdf_sdk_api_is_role_unkouwn(void)
{
    ibrt_ctrl_t *p_ibrt_ctrl = app_tws_ibrt_get_bt_ctrl_ctx();
    return (p_ibrt_ctrl->current_role == IBRT_UNKNOW);
}

/*******************************************************************************
 * @brief   .
 */
bool stdf_sdk_api_is_role_master(void)
{
    ibrt_ctrl_t *p_ibrt_ctrl = app_tws_ibrt_get_bt_ctrl_ctx();
    return (p_ibrt_ctrl->current_role == IBRT_MASTER);
}

/*******************************************************************************
 * @brief   .
 */
uint8_t* stdf_sdk_api_read_bt_addr(void)
{
    uint8_t *addr = factory_section_get_bt_address();
    STDF_SDK_API_ASSERT(addr != NULL);
    return addr;
}

/*******************************************************************************
 * @brief   .
 */
void stdf_sdk_api_delate_tws_record(void)
{
    btif_device_record_t record = {0};
    int paired_dev_count = nv_record_get_paired_dev_count();

    for (int i = paired_dev_count - 1; i >= 0; --i)
    {
        nv_record_enum_dev_records(i, &record);

        if (TWS_LINK == app_tws_ibrt_get_link_type_by_addr(&record.bdAddr))
        {
            nv_record_ddbrec_delete(&record.bdAddr);
        }
    }
    app_ibrt_if_config_keeper_clear();
}

/*******************************************************************************
 * @brief   .
 */
void stdf_sdk_api_delate_all_bt_record(void)
{
    btif_device_record_t record = {0};
    int paired_dev_count = nv_record_get_paired_dev_count();

    for (int i = paired_dev_count - 1; i >= 0; --i)
    {
        nv_record_enum_dev_records(i, &record);

        if (MOBILE_LINK == app_tws_ibrt_get_link_type_by_addr(&record.bdAddr))
        {
            nv_record_ddbrec_delete(&record.bdAddr);
        }
    }
    app_ibrt_if_config_keeper_clear();
}

/* -----------------------------------------------------------------------------
 *                                   misc
 * ---------------------------------------------------------------------------*/

/*******************************************************************************
 * @fn      .
 * @brief   .
 * @param   .
 * @return  .
 * @notice  .
 */
void stdf_sdk_api_init_earside(void)
{
    stdf_sdk_api_earside_t earside_old = stdf_sdk_api_get_earside();
    
    // GPIO level is low means right side while high means left side
#if defined(STDF_SDK_API_EARSIDE_USE_GPIO)

    // BT addr last bit is Odd means right side while even means left side.
#elif defined(STDF_SDK_API_EARSIDE_USE_BT_ADDR)
    uint8_t *addr = stdf_sdk_api_read_bt_addr();
    app_tws_set_side_from_addr(addr);
    
    // Read the ear side flag form flash that the factory ATE whited.
#elif defined(STDF_SDK_API_EARSIDE_USE_FLASH)
    
#else
#error Please select a way to deside ear side!!!
#endif
    
    stdf_sdk_api_earside_t earside_new = stdf_sdk_api_get_earside();
    
    STDF_SDK_API_LOG("earside %d -> %d", earside_old, earside_new);
}

/*******************************************************************************
 * @fn      .
 * @brief   .
 * @param   .
 * @return  .
 * @notice  .
 */
stdf_sdk_api_earside_t stdf_sdk_api_get_earside(void)
{
    return !app_tws_is_unknown_side() + app_tws_is_right_side();
}

/*******************************************************************************
 * @fn      .
 * @brief   .
 * @param   .
 * @return  .
 * @notice  .
 */
void stdf_sdk_api_log_info(void)
{
    size_t total = 8, used = 8, max_used = 8;
    med_memory_info(&total, &used, &max_used);
    STDF_SDK_API_LOG("total %d used %d max_used %d", total, used, max_used);
}

/*******************************************************************************
*******************************************************************************/
