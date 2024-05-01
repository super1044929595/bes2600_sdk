/*******************************************************************************
  Filename:       .

  Version         V1.0.

  Author          Yuping.Mo.

  Description:    .


  IMPORTANT:      .

*******************************************************************************/

#ifndef __STDF_HAL_VBUS_H__
#define __STDF_HAL_VBUS_H__

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
typedef struct
{
    uint8_t  *payload;
    uint16_t  length;    
} stdf_hal_vbus_rx_data_t;

typedef void (*stdf_hal_vbus_handler_t)(stdf_os_msg_id_t msg_id, void *payload);

/*******************************************************************************
 * EXTERNAL VARIABLES
 */

/*******************************************************************************
 * FUNCTIONS
 */
void stdf_hal_vbus_init(void);
void stdf_hal_vbus_send(uint8_t *buf, uint16_t len);
void stdf_hal_vbus_register_handler(stdf_hal_vbus_handler_t handler);

/*******************************************************************************
*******************************************************************************/

#ifdef __cplusplus
}
#endif

#endif /* __STDF_HAL_VBUS_H__ */
