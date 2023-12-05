/*******************************************************************************
  Filename:       .

  Version         V1.0.

  Author          Yuping.Mo.

  Description:    .


  IMPORTANT:      .

*******************************************************************************/

#ifndef __STDF_APP_PHY_H__
#define __STDF_APP_PHY_H__

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
typedef enum
{
    STDF_APP_PHY_STATE_IN_CASE_CLOSE,
    STDF_APP_PHY_STATE_IN_CASE_OPEN,
    STDF_APP_PHY_STATE_OUT_CASE_EAR,
    STDF_APP_PHY_STATE_IN_EAR
} stdf_app_phy_state_t;

/*******************************************************************************
 * EXTERNAL VARIABLES
 */

/*******************************************************************************
 * FUNCTIONS
 */
void stdf_app_phy_event_case_open(void);
void stdf_app_phy_event_case_close(void);
void stdf_app_phy_event_out_case(void);
void stdf_app_phy_event_in_case(void);
void stdf_app_phy_event_in_ear(void);
void stdf_app_phy_event_in_case(void);

void stdf_app_phy_init(void);

/*******************************************************************************
*******************************************************************************/

#ifdef __cplusplus
}
#endif

#endif /* __STDF_APP_PHY_H__ */

