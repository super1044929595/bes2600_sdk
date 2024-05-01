/*******************************************************************************
  Filename:       .

  Version         V1.0.

  Author          Yuping.Mo.

  Description:    .


  IMPORTANT:      .

*******************************************************************************/

#ifndef __STDF_HAL_PMU_H__
#define __STDF_HAL_PMU_H__

#ifdef __STDF_HAL_PMU_ENABLE__

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
// The standard callback charger plug in/out event
typedef void (*stdf_hal_pmu_callback_t)(bool plug_in);

/*******************************************************************************
 * EXTERNAL VARIABLES
 */

/*******************************************************************************
 * FUNCTIONS
 */
void stdf_hal_pmu_init(void);
void stdf_hal_pmu_register_callback(stdf_hal_pmu_callback_t callback);
bool stdf_hal_pmu_is_plug_in(void);

/*******************************************************************************
*******************************************************************************/

#ifdef __cplusplus
}
#endif

#endif /* __STDF_HAL_PMU_ENABLE__ */

#endif /* __STDF_HAL_PMU_H__ */
