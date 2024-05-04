/*******************************************************************************
  Filename:       .

  Version         V1.0.

  Author          Yuping.Mo.

  Description:    .


  IMPORTANT:      .

*******************************************************************************/

#ifndef __STDF_APP_PROTOCAL_H__
#define __STDF_APP_PROTOCAL_H__

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
    STDF_APP_PROTOCAL_CMD_STATE = 0,
    STDF_APP_PROTOCAL_CMD_CASE_OPEN,
    STDF_APP_PROTOCAL_CMD_CASE_CLOSE,
    STDF_APP_PROTOCAL_CMD_1WIRE_DOWNLOAD,
    STDF_APP_PROTOCAL_CMD_TWS_PAIRING,
    STDF_APP_PROTOCAL_CMD_BT_PAIRING,
    STDF_APP_PROTOCAL_CMD_FREEMAN_PAIRING,
    STDF_APP_PROTOCAL_CMD_RESERVED_1,
    STDF_APP_PROTOCAL_CMD_RESET_FACTORY_SETTINGS,
    STDF_APP_PROTOCAL_CMD_CHARGE_COMPLETE, // 9
    
    STDF_APP_PROTOCAL_CMD_READ_VERSION = 13,
    STDF_APP_PROTOCAL_CMD_READ_BT_ADDR = 14,
    STDF_APP_PROTOCAL_CMD_WRITE_TWS_BDADDR = 15,
    STDF_APP_PROTOCAL_CMD_GOODBYE = 16,
    STDF_APP_PROTOCAL_CMD_ENTER_DUT = 19,
    STDF_APP_PROTOCAL_CMD_POWER_OFF = 40,

    STDF_APP_PROTOCAL_CMD_INVALID
} stdf_app_protocal_cmd_t;

typedef enum
{
    STDF_APP_PROTOCAL_ACK_ERROR,
    STDF_APP_PROTOCAL_ACK_SUCCESS,
} stdf_app_protocal_ack_t;

typedef void (*stdf_app_protocal_callback_t)(stdf_app_protocal_cmd_t cmd, uint8_t *data, uint16_t length);

/*******************************************************************************
 * EXTERNAL VARIABLES
 */

/*******************************************************************************
 * FUNCTIONS
 */
void stdf_app_protocal_init(void);
void stdf_app_protocal_register_callback(stdf_app_protocal_callback_t callback);
void stdf_app_protocal_response(uint8_t *data, uint16_t length, stdf_app_protocal_ack_t ack);

/*******************************************************************************
*******************************************************************************/

#ifdef __cplusplus
}
#endif

#endif /* __STDF_APP_PROTOCAL_H__ */

