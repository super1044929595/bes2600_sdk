/*******************************************************************************
  Filename:       .

  Version         .

  Author          .

  Description:    .


  IMPORTANT:      .

*******************************************************************************/

#ifndef __STDF_OS_DELAY_MSG_H__
#define __STDF_OS_DELAY_MSG_H__


/*******************************************************************************
 * INCLUDES
 */
#include "plat_types.h"
#include "stdf_os_config.h"

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
// APIs, ALL the APIs should be called after stdf_os_init()
void     stdf_os_delay_msg_send(stdf_os_handler_t handler, stdf_os_msg_id_t msg_id, void *payload);
void     stdf_os_delay_msg_send_later(stdf_os_handler_t handler, stdf_os_msg_id_t msg_id, void *payload, uint32_t delay);
uint16_t stdf_os_delay_msg_get_count(stdf_os_handler_t handler, stdf_os_msg_id_t msg_id);
bool     stdf_os_delay_msg_cancel_first(stdf_os_handler_t handler, stdf_os_msg_id_t msg_id);
uint16_t stdf_os_delay_msg_cancel_all(stdf_os_handler_t handler, stdf_os_msg_id_t msg_id);

// Framework
void     stdf_os_delay_msg_init(void);

/*******************************************************************************
*******************************************************************************/

#ifdef __cplusplus
}
#endif

#endif /* __STDF_OS_DELAY_MSG_H__ */