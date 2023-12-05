/*******************************************************************************
  Filename:       .

  Version         V1.0.

  Author          Yuping.Mo.

  Description:    .


  IMPORTANT:      .

*******************************************************************************/

#ifndef __STDF_OS_CONFIG_H__
#define __STDF_OS_CONFIG_H__

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
#define STDF_OS_MSG_ID_INVALID              0xFFFF
#define STDF_OS_IMSG_ID_BASE                0x0000 // (IMSG)Internal message ID from 0x0000 to 0x7FFF
#define STDF_OS_GMSG_ID_BASE                0x8000 // (GMSG)Global message ID from 0x8000 to 0xFFFE


// GMSG ID for communication
#define STDF_OS_GMSG_ID_VBUS_RECEIVE_DATA   0x8000

#define STDF_OS_GMSG_ID_CHG_PLUG_OUT        0x8010
#define STDF_OS_GMSG_ID_CHG_PLUG_IN         0x8011
#define STDF_OS_GMSG_ID_CASE_PLUG_OUT       0x8012
#define STDF_OS_GMSG_ID_CASE_PLUG_IN        0x8013

#define STDF_OS_GMSG_ID_PHY_IN_CASE_CLOSE   0x8020
#define STDF_OS_GMSG_ID_PHY_IN_CASE_OPEN    0x8021
#define STDF_OS_GMSG_ID_PHY_OUT_CASE_EAR    0x8022
#define STDF_OS_GMSG_ID_PHY_IN_EAR          0x8023

/*******************************************************************************
 * TYPEDEFS
 */
//
typedef  uint16_t stdf_os_msg_id_t;

// The standard function style of the message handler
typedef  void (*stdf_os_handler_t)(stdf_os_msg_id_t msg_id, void *payload);

//
typedef struct
{
    stdf_os_handler_t  handler;             // the handler to recieved the message
    stdf_os_msg_id_t   msg_id;
    void              *payload;             //
}stdf_os_msg_t;


/*******************************************************************************
 * EXTERNAL VARIABLES
 */

/*******************************************************************************
 * FUNCTIONS
 */

/*******************************************************************************
*******************************************************************************/

#ifdef __cplusplus
}
#endif

#endif /* __STDF_OS_CONFIG_H__ */

