/*******************************************************************************
  Filename:       .

  Version         V1.0.

  Author          Yuping.Mo.

  Description:    .


  IMPORTANT:      .

*******************************************************************************/

#ifndef __STDF_SDK_CB_H__
#define __STDF_SDK_CB_H__

/*******************************************************************************
 * INCLUDES
 */
#include "plat_types.h"

#ifdef __cplusplus
extern "C" {
#endif

/*******************************************************************************
 * MACROS
 */

/*******************************************************************************
 * TYPEDEFS
 */

/*******************************************************************************
 * EXTERNAL VARIABLES
 */

/*******************************************************************************
 * FUNCTIONS
 */

/* -----------------------------------------------------------------------------
 *                                   PHY
 * ---------------------------------------------------------------------------*/

void stdf_sdk_cb_5v_plug_in(void);
void stdf_sdk_cb_5v_plug_out(void);
void stdf_sdk_cb_charging(void);
void stdf_sdk_cb_discharging(void);

/* -----------------------------------------------------------------------------
 *                                   BT
 * ---------------------------------------------------------------------------*/

void stdf_sdk_cb_pairing_enter(uint8_t trigger_type);
void stdf_sdk_cb_pairing_exit(uint8_t trigger_type);

void stdf_sdk_cb_tws_connecting(void);
void stdf_sdk_cb_tws_connected(uint8_t *addr);
void stdf_sdk_cb_tws_disconnected(void);

void stdf_sdk_cb_bt_connecting(void);
void stdf_sdk_cb_bt_connected(uint8_t *addr);
void stdf_sdk_cb_bt_disconnected(void);
void stdf_sdk_cb_bt_linkloss(void);

void stdf_sdk_cb_global_handler_ind(uint8_t link_type, uint8_t evt_type, uint8_t status);
void stdf_sdk_cb_profile_state_change_ind(uint32_t profile,uint8_t connected);

/*******************************************************************************
*******************************************************************************/

#ifdef __cplusplus
}
#endif

#endif /* __STDF_SDK_CB_H__ */
