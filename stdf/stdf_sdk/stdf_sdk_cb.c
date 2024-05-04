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
#include "stdf_sdk_api.h"
#include "stdf_sdk_cb.h"

/*******************************************************************************
 * MACROS
 */
#define STDF_SDK_CB_LOG(str, ...)           STDF_LOG("[SDK][CB] %s "str, __func__, ##__VA_ARGS__)
#define STDF_SDK_CB_ASSERT(cond)            STDF_ASSERT(cond)

/*******************************************************************************
 * TYPEDEFS
 */

/*******************************************************************************
* GLOBAL VARIABLES
*/

/*******************************************************************************
 * EXTERNAL VARIABLES
 */
 
/* -----------------------------------------------------------------------------
 *                                   PHY
 * ---------------------------------------------------------------------------*/

/*******************************************************************************
 * @brief   .
 */
void stdf_sdk_cb_5v_plug_in(void)
{
    STDF_SDK_CB_LOG("");
    stdf_sdk_api_phy_set_state_in_case_close();
}

/*******************************************************************************
 * @brief   .
 */
void stdf_sdk_cb_5v_plug_out(void)
{
    STDF_SDK_CB_LOG("");
}

/*******************************************************************************
 * @brief   .
 */
void stdf_sdk_cb_charging(void)
{
    STDF_SDK_CB_LOG("");
}

/*******************************************************************************
 * @brief   .
 */
void stdf_sdk_cb_discharging(void)
{
    STDF_SDK_CB_LOG("");
}

/* -----------------------------------------------------------------------------
 *                                   BT
 * ---------------------------------------------------------------------------*/
 
/*******************************************************************************
 * FUNCTIONS
 */

/*******************************************************************************
 * @fn      .
 * @brief   .
 * @param   .
 * @return  .
 * @notice  .
 */
void stdf_sdk_cb_pairing_enter(uint8_t trigger_type)
{
    STDF_SDK_CB_LOG("trigger_type %d", trigger_type);
}

/*******************************************************************************
 * @fn      .
 * @brief   .
 * @param   .
 * @return  .
 * @notice  .
 */
void stdf_sdk_cb_pairing_exit(uint8_t trigger_type)
{
    STDF_SDK_CB_LOG("trigger_type %d", trigger_type);
}

/*******************************************************************************
 * @fn      .
 * @brief   .
 * @param   .
 * @return  .
 * @notice  .
 */
void stdf_sdk_cb_tws_connecting(void)
{
    STDF_SDK_CB_LOG("");
}

/*******************************************************************************
 * @fn      .
 * @brief   .
 * @param   .
 * @return  .
 * @notice  .
 */
void stdf_sdk_cb_tws_connected(uint8_t *addr)
{
    STDF_SDK_CB_LOG("addr %02x %02x %02x %02x %02x", \
        addr[0], addr[1], addr[2], addr[3], addr[4], addr[5]);
}

/*******************************************************************************
 * @fn      .
 * @brief   .
 * @param   .
 * @return  .
 * @notice  .
 */
void stdf_sdk_cb_bt_connecting(void)
{
    STDF_SDK_CB_LOG("");
}

/*******************************************************************************
 * @fn      .
 * @brief   .
 * @param   .
 * @return  .
 * @notice  .
 */
void stdf_sdk_cb_tws_disconnected(void)
{
    STDF_SDK_CB_LOG("");
}

/*******************************************************************************
 * @fn      .
 * @brief   .
 * @param   .
 * @return  .
 * @notice  .
 */
void stdf_sdk_cb_bt_connected(uint8_t *addr)
{
    STDF_SDK_CB_LOG("addr %02x %02x %02x %02x %02x %02x", \
        addr[0], addr[1], addr[2], addr[3], addr[4], addr[5]);
}

/*******************************************************************************
 * @fn      .
 * @brief   .
 * @param   .
 * @return  .
 * @notice  .
 */
void stdf_sdk_cb_bt_disconnected(void)
{
    STDF_SDK_CB_LOG("");
}

/*******************************************************************************
 * @fn      .
 * @brief   .
 * @param   .
 * @return  .
 * @notice  .
 */
void stdf_sdk_cb_bt_linkloss(void)
{
    STDF_SDK_CB_LOG("");
}
    
/*******************************************************************************
 * @fn      .
 * @brief   .
 * @param   .
 * @return  .
 * @notice  .
 */
void stdf_sdk_cb_global_handler_ind(uint8_t link_type, uint8_t evt_type, uint8_t status)
{
    STDF_SDK_CB_LOG("link_type %d evt_type %d status %d", link_type, evt_type, status);
}

    
/*******************************************************************************
 * @fn      .
 * @brief   .
 * @param   .
 * @return  .
 * @notice  .
 */
void stdf_sdk_cb_profile_state_change_ind(uint32_t profile,uint8_t connected)
{
    STDF_SDK_CB_LOG("profile %d connected %d", profile, connected);
}

/*******************************************************************************
*******************************************************************************/
