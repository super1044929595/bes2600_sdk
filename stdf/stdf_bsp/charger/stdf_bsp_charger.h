/*******************************************************************************
  Filename:       .

  Version         V1.0.

  Author          Yuping.Mo.

  Description:    .


  IMPORTANT:      .

*******************************************************************************/

#ifndef __STDF_BSP_CHARGER_H__
#define __STDF_BSP_CHARGER_H__

/*******************************************************************************
 * INCLUDES
 */
#include "plat_types.h"
#include "stdf_os.h"

#ifdef __cplusplus
extern "C" {
#endif

/*******************************************************************************
 * MACROS
 */

/*******************************************************************************
 * TYPEDEFS
 */
typedef void (*stdf_bsp_charger_callback_t)(stdf_os_msg_id_t msg_id, void *payload);

/*******************************************************************************
 * EXTERNAL VARIABLES
 */

/*******************************************************************************
 * FUNCTIONS
 */
bool stdf_bsp_charger_get_plug_in_state(void);
bool stdf_bsp_charger_get_in_case_state(void);

void stdf_bsp_charger_init(void);
void stdf_bsp_charger_register_callback(stdf_bsp_charger_callback_t callback);

/*******************************************************************************
*******************************************************************************/

#ifdef __cplusplus
}
#endif

#endif
