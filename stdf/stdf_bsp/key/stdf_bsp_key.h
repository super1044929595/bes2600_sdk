/*******************************************************************************
  Filename:       .

  Version         .

  Author          .

  Description:    .


  IMPORTANT:      .

*******************************************************************************/

#ifndef STDF_BSP_KEY_H
#define STDF_BSP_KEY_H

/*******************************************************************************
 * INCLUDES
 */
#include "plat_types.h"

/*******************************************************************************
 * MACROS
 */

/*******************************************************************************
 * TYPEDEFS
 */
// key number/name
typedef enum
{
    STDF_BSP_KEY_NUM_1,
    STDF_BSP_KEY_NUM_MAX
} stdf_bsp_key_num_t;

// key event
typedef enum
{
    STDF_BSP_KEY_EVENT_RELEASE,
    STDF_BSP_KEY_EVENT_PRESS,
    STDF_BSP_KEY_EVENT_SINGLE_CLICK, 
    STDF_BSP_KEY_EVENT_DOUBLE_CLICK,
    STDF_BSP_KEY_EVENT_TRIPLE_CLICK,
    STDF_BSP_KEY_EVENT_LONG_PRESS,
    STDF_BSP_KEY_EVENT_LONG_LONG_PRESS,
    STDF_BSP_KEY_EVENT_CONTINUOUS_PRESS
} stdf_bsp_key_event_t;

/* 1 << stdf_bsp_key_event_t, for exemple if we need double click triple click and long press,event masks are
((1 << STDF_BSP_KEY_EVENT_PRESS)| (1 << STDF_BSP_KEY_EVENT_PRESS) | (1 << STDF_BSP_KEY_EVENT_PRESS)) */
typedef uint32_t stdf_bsp_key_event_mask_t;

// The callback format for key press and release state changed process in 
typedef void (*stdf_bsp_key_event_callback_t)(stdf_bsp_key_num_t key_num, stdf_bsp_key_event_t event);

/*******************************************************************************
 * EXTERNAL VARIABLES
 */

/*******************************************************************************
 * FUNCTIONS
 */
void stdf_bsp_key_init(void);
void stdf_bsp_key_register_event_callback(stdf_bsp_key_num_t key_num, 
                                          stdf_bsp_key_event_callback_t callback, 
                                          stdf_bsp_key_event_mask_t event_mask);
bool stdf_bsp_key_is_pressed(void);

#endif /* STDF_BSP_KEY_H */

/*******************************************************************************
*******************************************************************************/

