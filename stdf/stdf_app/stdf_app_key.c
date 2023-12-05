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
#include "stdf_bsp_key.h"
#include "stdf_define.h" 
#include "stdf_os.h"
#include "stdf_sdk_api.h"

/*******************************************************************************
 * MACROS
 */
#define STDF_APP_KEY_MFB                    STDF_BSP_KEY_NUM_1
#define STDF_APP_KEY_MFB_MASKS              ((1 << STDF_BSP_KEY_EVENT_SINGLE_CLICK) | \
                                             (1 << STDF_BSP_KEY_EVENT_DOUBLE_CLICK) | \
                                             (1 << STDF_BSP_KEY_EVENT_TRIPLE_CLICK) | \
                                             (1 << STDF_BSP_KEY_EVENT_LONG_PRESS) | \
                                             (1 << STDF_BSP_KEY_EVENT_LONG_LONG_PRESS))

#define STDF_APP_KEY_LOG(str, ...)          STDF_LOG("[APP][KEY] %s "str, __func__, ##__VA_ARGS__)
#define STDF_APP_KEY_ASSERT(cond)           STDF_ASSERT(cond)

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
static void stdf_app_key_event_callback(stdf_bsp_key_num_t key_num, 
                                        stdf_bsp_key_event_t event)
{
    STDF_APP_KEY_LOG("key_num %d event %d", key_num, event);
    
    switch(event)
    {
        case STDF_BSP_KEY_EVENT_SINGLE_CLICK:
            stdf_sdk_api_log_info();
            break;
            
        case STDF_BSP_KEY_EVENT_DOUBLE_CLICK:
            break;
            
        case STDF_BSP_KEY_EVENT_TRIPLE_CLICK:
            break;
            
        case STDF_BSP_KEY_EVENT_LONG_PRESS:
            stdf_sdk_api_reset_enter_tws_pairing();
            break;
            
        case STDF_BSP_KEY_EVENT_LONG_LONG_PRESS:
            break;
        
        default:
            STDF_APP_KEY_ASSERT(false);
            break;
    }
}


/*******************************************************************************
 * @fn      .
 * @brief   .
 * @param   .
 * @return  .
 * @notice  .
 */
void stdf_app_key_init(void)
{
    stdf_bsp_key_register_event_callback(STDF_APP_KEY_MFB,
                                         stdf_app_key_event_callback,
                                         STDF_APP_KEY_MFB_MASKS);
}

/*******************************************************************************
*******************************************************************************/

