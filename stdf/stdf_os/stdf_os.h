/*******************************************************************************
  Filename:       .

  Version         V1.0.

  Author          Yuping.Mo.

  Description:    .


  IMPORTANT:      .

*******************************************************************************/

#ifndef __STDF_OS_H__
#define __STDF_OS_H__

/*******************************************************************************
 * INCLUDES
 */
#include "plat_types.h"

#include "stdf_os.h" 
#include "stdf_os_config.h"
#include "stdf_os_delay_msg.h"
#include "stdf_os_mem.h"
#include "stdf_os_msg.h"

#ifdef __cplusplus
extern "C" {
#endif

/*******************************************************************************
 * MACROS
 */
#define MessageSend(handler, id, payload)   stdf_os_msg_mailbox_put(handler, id, payload)
#define MessageSendLater(handler, id, payload, delay) \
                                            do { \
                                                if(delay) \
                                                { \
                                                    stdf_os_delay_msg_send_later(handler, id, payload, delay); \
                                                } \
                                                else \
                                                { \
                                                    stdf_os_msg_mailbox_put(handler, id, payload); \
                                                } \
                                            } while(0)
#define MessageGetCount(handler, id)        stdf_os_delay_msg_get_count(handler, id)

#define MessageCancelAll(handler, id)       stdf_os_delay_msg_cancel_all(handler, id)

/*******************************************************************************
 * TYPEDEFS
 */

/*******************************************************************************
 * EXTERNAL VARIABLES
 */

/*******************************************************************************
 * FUNCTIONS
 */
void stdf_os_init(void);

/*******************************************************************************
*******************************************************************************/

#ifdef __cplusplus
}
#endif

#endif /* __STDF_OS_H__ */

