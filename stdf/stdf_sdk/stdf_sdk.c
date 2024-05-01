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
#include "stdf_sdk.h"
#include "stdf_sdk_api.h"

/*******************************************************************************
 * MACROS
 */
#define STDF_SDK_LOG(str, ...)              STDF_LOG("[SDK] %s "str, __func__, ##__VA_ARGS__)
#define STDF_SDK_ASSERT(cond)               STDF_ASSERT(cond)

/*******************************************************************************
 * TYPEDEFS
 */

/*******************************************************************************
* GLOBAL VARIABLES
*/

/*******************************************************************************
 * EXTERNAL VARIABLES
 */

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
void stdf_sdk_init(void)
{
    // init easide by bluetooth address, gpio or flash value
    stdf_sdk_api_init_earside();
    
    // what the power reason before power on
    stdf_sdk_api_power_reason_init();

#ifdef __STDF_HAL_PMU_ENABLE__
#else
    // set the default phy state for power on
    stdf_sdk_api_phy_init_state();
#endif
}

/*******************************************************************************
*******************************************************************************/
