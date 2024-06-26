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

/*******************************************************************************
 * MACROS
 */
#define STDF_OS_LOG(str, ...)               STDF_LOG("[OS] %s "str, __func__, ##__VA_ARGS__)
#define STDF_OS_ASSERT(cond)                STDF_ASSERT(cond)

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
void stdf_os_init(void)
{
    //STDF_OS_LOG("");

    stdf_os_mem_init();

    stdf_os_msg_init();

    stdf_os_delay_msg_init();   
}

/*******************************************************************************
*******************************************************************************/

