/*******************************************************************************
  Filename:       .

  Version         .

  Author          .

  Description:    .


  IMPORTANT:      .

*******************************************************************************/

/*******************************************************************************
 * INCLUDES
 */
#include "hal_gpio.h"
#include "hal_iomux.h"
#include "hal_timer.h"

#include "stdf_bsp_key.h" 
#include "stdf_define.h"
#include "stdf_os.h"

/*******************************************************************************
 * MACROS
 */
#define STDF_BSP_KEY_MULT_PRESS_TIME        500 // ms
#define STDF_BSP_KEY_LONG_PRESS_TIME        1500
#define STDF_BSP_KEY_LONG_LONG_PRESS_TIME   5000
#define STDF_BSP_KEY_CONTINUOUS_PRESS_TIME  100


#define STDF_BSP_KEY_LOG(str, ...)          STDF_LOG("[BSP][KEY] %s "str, __func__, ##__VA_ARGS__)
#define STDF_BSP_KEY_ASSERT(cond)           STDF_ASSERT(cond)

/*******************************************************************************
 * TYPEDEFS
 */
// key state
typedef enum
{
    STDF_BSP_KEY_STATE_RELEASE,
    STDF_BSP_KEY_STATE_PRESS 
} stdf_bsp_key_state_t;

// 
typedef struct
{   
    uint8_t                       press_count; 
    uint8_t                       release_count;
    uint32_t                      last_press_time;
    uint32_t                      last_release_time;
    stdf_bsp_key_event_callback_t event_callback;
    stdf_bsp_key_event_mask_t     event_mask;
} stdf_bsp_key_data_t;

/*******************************************************************************
* GLOBAL VARIABLES
*/
const struct HAL_IOMUX_PIN_FUNCTION_MAP stdf_bsp_key_int_pinmux = 
{
    HAL_GPIO_PIN_P0_1, 
    HAL_IOMUX_FUNC_AS_GPIO, 
    HAL_IOMUX_PIN_VOLTAGE_VIO, 
    HAL_IOMUX_PIN_PULLUP_ENABLE
};

stdf_bsp_key_data_t  stdf_bsp_key_data[STDF_BSP_KEY_NUM_MAX];

/*******************************************************************************
 * EXTERNAL VARIABLES
 */

/*******************************************************************************
 * FUNCTIONS
 */
static void stdf_bsp_key_irq_handler(enum HAL_GPIO_PIN_T pin);
static void stdf_bsp_key_state_msg_handler(stdf_os_msg_id_t msg_id, void *payload);
static void stdf_bsp_key_event_msg_handler(stdf_os_msg_id_t msg_id, void *payload);

/*******************************************************************************
 * @fn      .
 * @brief   .
 * @param   .
 * @return  .
 * @notice  .
 */
static void stdf_bsp_key_int_set_rise_edge(void)
{
    struct HAL_GPIO_IRQ_CFG_T gpiocfg;
    gpiocfg.irq_enable = 1;
    gpiocfg.irq_debounce = 0;
    gpiocfg.irq_type = HAL_GPIO_IRQ_TYPE_EDGE_SENSITIVE;
    gpiocfg.irq_polarity = HAL_GPIO_IRQ_POLARITY_HIGH_RISING;
    gpiocfg.irq_handler = (HAL_GPIO_PIN_IRQ_HANDLER)stdf_bsp_key_irq_handler;
    hal_gpio_setup_irq((enum HAL_GPIO_PIN_T)stdf_bsp_key_int_pinmux.pin, &gpiocfg);
}

/*******************************************************************************
 * @fn      .
 * @brief   .
 * @param   .
 * @return  .
 * @notice  .
 */
static void stdf_bsp_key_int_set_fall_edge(void)
{
    struct HAL_GPIO_IRQ_CFG_T gpiocfg;
    gpiocfg.irq_enable = 1;
    gpiocfg.irq_debounce = 0;
    gpiocfg.irq_type = HAL_GPIO_IRQ_TYPE_EDGE_SENSITIVE;
    gpiocfg.irq_polarity = HAL_GPIO_IRQ_POLARITY_LOW_FALLING;
    gpiocfg.irq_handler = (HAL_GPIO_PIN_IRQ_HANDLER)stdf_bsp_key_irq_handler;
    hal_gpio_setup_irq((enum HAL_GPIO_PIN_T)stdf_bsp_key_int_pinmux.pin, &gpiocfg);
}

/*******************************************************************************
 * @fn      .
 * @brief   .
 * @param   .
 * @return  .
 * @notice  .
 */
static void stdf_bsp_key_irq_handler(enum HAL_GPIO_PIN_T pin)
{
    bool pressed = stdf_bsp_key_is_pressed();
    
    STDF_BSP_KEY_LOG("pressed %d", pressed);

    if(stdf_bsp_key_is_pressed())
    {
        stdf_bsp_key_int_set_fall_edge();
    }
    else
    {
        stdf_bsp_key_int_set_rise_edge();
    }

    // send the message
    stdf_bsp_key_num_t key_num = STDF_BSP_KEY_NUM_1;
    stdf_bsp_key_state_t state = pressed ? STDF_BSP_KEY_STATE_PRESS : STDF_BSP_KEY_STATE_RELEASE;
    uint16_t msg_id = ((uint8_t)key_num << 8) | (uint8_t)state;
    MessageSend(stdf_bsp_key_state_msg_handler, msg_id, NULL);
}

/*******************************************************************************
 * @fn      .
 * @brief   .
 * @param   .
 * @return  .
 * @notice  .
 */
static void stdf_bsp_key_gpio_init(void)
{
    hal_iomux_init(&stdf_bsp_key_int_pinmux, 1);
    hal_gpio_pin_set_dir(stdf_bsp_key_int_pinmux.pin, HAL_GPIO_DIR_IN, 0);
    
    struct HAL_GPIO_IRQ_CFG_T gpiocfg;
    gpiocfg.irq_enable = 1;
    gpiocfg.irq_debounce = 0;
    gpiocfg.irq_type = HAL_GPIO_IRQ_TYPE_EDGE_SENSITIVE;
    gpiocfg.irq_polarity = HAL_GPIO_IRQ_POLARITY_HIGH_RISING;
    gpiocfg.irq_handler = (HAL_GPIO_PIN_IRQ_HANDLER)stdf_bsp_key_irq_handler;
    hal_gpio_setup_irq((enum HAL_GPIO_PIN_T)stdf_bsp_key_int_pinmux.pin, &gpiocfg);
}

/*******************************************************************************
 * @brief   .
 * @param   .
 * @return  .
 * @notice  .
 */
static void stdf_bsp_key_event_set(stdf_bsp_key_num_t key_num, stdf_bsp_key_event_t event)
{
    uint32_t delay;

    if(!(stdf_bsp_key_data[key_num].event_mask & (1 << event)))
    {
        return;
    }
    
    switch(event)
    {
        case STDF_BSP_KEY_EVENT_PRESS:
        case STDF_BSP_KEY_EVENT_RELEASE:
            delay = 0;
            break;        
        case STDF_BSP_KEY_EVENT_SINGLE_CLICK:
        case STDF_BSP_KEY_EVENT_DOUBLE_CLICK:
        case STDF_BSP_KEY_EVENT_TRIPLE_CLICK:
            delay = STDF_BSP_KEY_MULT_PRESS_TIME;
            break;
        case STDF_BSP_KEY_EVENT_LONG_PRESS:
            delay = STDF_BSP_KEY_LONG_PRESS_TIME;
            break;
        case STDF_BSP_KEY_EVENT_LONG_LONG_PRESS:
            delay = STDF_BSP_KEY_LONG_LONG_PRESS_TIME;
            break;
        case STDF_BSP_KEY_EVENT_CONTINUOUS_PRESS:
            delay = STDF_BSP_KEY_CONTINUOUS_PRESS_TIME;
            break;
        default:
            return;
    }
    
    uint16_t msg = ((uint8_t)key_num << 8) | (uint8_t)event;
    MessageCancelAll(stdf_bsp_key_event_msg_handler, msg);
    MessageSendLater(stdf_bsp_key_event_msg_handler, msg, NULL, delay);
}

/*******************************************************************************
 * @brief   .
 * @param   .
 * @return  .
 * @notice  .
 */
static void stdf_bsp_key_event_reset(stdf_bsp_key_num_t key_num, stdf_bsp_key_event_t event)
{ 
    if(!(stdf_bsp_key_data[key_num].event_mask & (1 << event)))
    {
        return;
    }
    
    uint16_t msg_id = ((uint8_t)key_num << 8) | (uint8_t)event;    
    MessageCancelAll(stdf_bsp_key_event_msg_handler, msg_id); 
}

/*******************************************************************************
 * @brief   .
 * @param   state - FALSE is release and TRUE is press.
 * @return  .
 * @notice  .
 */
void stdf_bsp_key_state_changed(stdf_bsp_key_num_t key_num, stdf_bsp_key_state_t state)
{
    uint8_t  *p_press_count       = &stdf_bsp_key_data[key_num].press_count;
    uint8_t  *p_release_count     = &stdf_bsp_key_data[key_num].release_count;
    uint32_t *p_last_press_time   = &stdf_bsp_key_data[key_num].last_press_time;
    uint32_t *p_last_release_time = &stdf_bsp_key_data[key_num].last_release_time;

    // Get press or release time for key
    uint32_t now = GET_CURRENT_MS();
    STDF_BSP_KEY_LOG("state %d now %d", state, now);

    // handle press, release, long press and long long press event
    if(state == STDF_BSP_KEY_STATE_PRESS)
    {
        stdf_bsp_key_event_set(key_num, STDF_BSP_KEY_EVENT_PRESS);
        stdf_bsp_key_event_set(key_num, STDF_BSP_KEY_EVENT_LONG_PRESS);
        stdf_bsp_key_event_set(key_num, STDF_BSP_KEY_EVENT_LONG_LONG_PRESS);
    }
    else // if(state == STDF_BSP_KEY_STATE_RELEASE)
    {   
        stdf_bsp_key_event_set(key_num, STDF_BSP_KEY_EVENT_RELEASE);
        stdf_bsp_key_event_reset(key_num, STDF_BSP_KEY_EVENT_LONG_PRESS);
        stdf_bsp_key_event_reset(key_num, STDF_BSP_KEY_EVENT_LONG_LONG_PRESS);
        stdf_bsp_key_event_reset(key_num, STDF_BSP_KEY_EVENT_CONTINUOUS_PRESS);
    }

    // handle single, double and trible click event
    if(state == STDF_BSP_KEY_STATE_PRESS)
    {
        if(now - *p_last_release_time >= STDF_BSP_KEY_MULT_PRESS_TIME)
        {
            *p_press_count = 1;
            *p_release_count = 0;
        }
        else
        {
            (*p_press_count)++;
        }
        *p_last_press_time = now;
        stdf_bsp_key_event_reset(key_num, STDF_BSP_KEY_EVENT_SINGLE_CLICK);
        stdf_bsp_key_event_reset(key_num, STDF_BSP_KEY_EVENT_DOUBLE_CLICK);
        stdf_bsp_key_event_reset(key_num, STDF_BSP_KEY_EVENT_TRIPLE_CLICK);
    }
    else // if(state == STDF_BSP_KEY_STATE_RELEASE)
    {
        if(now - *p_last_press_time >= STDF_BSP_KEY_LONG_PRESS_TIME)
        {
            *p_press_count = 0;
            *p_release_count = 0;
        }
        else
        {
            (*p_release_count)++;
        }
        *p_last_release_time = now;

        // handle double and triple press
        if(*p_press_count == 1)
        {
            stdf_bsp_key_event_reset(key_num, STDF_BSP_KEY_EVENT_SINGLE_CLICK);
            stdf_bsp_key_event_reset(key_num, STDF_BSP_KEY_EVENT_DOUBLE_CLICK);
            stdf_bsp_key_event_reset(key_num, STDF_BSP_KEY_EVENT_TRIPLE_CLICK);
            stdf_bsp_key_event_set(key_num, STDF_BSP_KEY_EVENT_SINGLE_CLICK);
        }
        if(*p_press_count == 2)
        {
            stdf_bsp_key_event_reset(key_num, STDF_BSP_KEY_EVENT_SINGLE_CLICK);
            stdf_bsp_key_event_reset(key_num, STDF_BSP_KEY_EVENT_DOUBLE_CLICK);
            stdf_bsp_key_event_reset(key_num, STDF_BSP_KEY_EVENT_TRIPLE_CLICK);
            stdf_bsp_key_event_set(key_num, STDF_BSP_KEY_EVENT_DOUBLE_CLICK);
        }
        else if(*p_press_count == 3)
        {
            stdf_bsp_key_event_reset(key_num, STDF_BSP_KEY_EVENT_SINGLE_CLICK);
            stdf_bsp_key_event_reset(key_num, STDF_BSP_KEY_EVENT_DOUBLE_CLICK);
            stdf_bsp_key_event_reset(key_num, STDF_BSP_KEY_EVENT_TRIPLE_CLICK);
            stdf_bsp_key_event_set(key_num, STDF_BSP_KEY_EVENT_TRIPLE_CLICK);
        }
        else if(*p_press_count > 3)
        {
            stdf_bsp_key_event_reset(key_num, STDF_BSP_KEY_EVENT_TRIPLE_CLICK);
        }
    }
    STDF_BSP_KEY_LOG("key_num %d press_count %d release_count %d", key_num, *p_press_count, *p_release_count);
}

/*******************************************************************************
 * @fn      .
 * @brief   .
 * @param   .
 * @return  .
 * @notice  .
 */ 
static void stdf_bsp_key_state_msg_handler(stdf_os_msg_id_t msg_id, void *payload)
{
    stdf_bsp_key_num_t key_num = (stdf_bsp_key_num_t)((msg_id >> 8) & 0xFF);
    stdf_bsp_key_state_t state = (stdf_bsp_key_state_t)(msg_id & 0xFF);

    switch(state)
    {
        case STDF_BSP_KEY_STATE_PRESS:
            stdf_bsp_key_state_changed(key_num, STDF_BSP_KEY_STATE_PRESS);
            break;
        case STDF_BSP_KEY_STATE_RELEASE:
            stdf_bsp_key_state_changed(key_num, STDF_BSP_KEY_STATE_RELEASE);
            break;
        default:
            STDF_BSP_KEY_ASSERT(false);
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
static void stdf_bsp_key_event_msg_handler(stdf_os_msg_id_t msg_id, void *payload)
{
    stdf_bsp_key_num_t key_num = (stdf_bsp_key_num_t)((msg_id >> 8) & 0xFF);
    stdf_bsp_key_event_t event = (stdf_bsp_key_event_t)(msg_id & 0xFF);
    STDF_BSP_KEY_ASSERT(key_num < STDF_BSP_KEY_NUM_MAX); 
    stdf_bsp_key_event_callback_t callback = stdf_bsp_key_data[key_num].event_callback;
    stdf_bsp_key_event_mask_t key_event_masks = stdf_bsp_key_data[key_num].event_mask;
    //STDF_BSP_KEY_LOG("callback %p key_num %d event %d!", callback, key_num, event);
    
    // check parameter 
    if(callback == NULL || ((key_event_masks & (1 << event)) == 0)) 
    {
        return;
    }
    
    switch(event)
    {
        case STDF_BSP_KEY_EVENT_PRESS:
        case STDF_BSP_KEY_EVENT_RELEASE:
        case STDF_BSP_KEY_EVENT_SINGLE_CLICK: 
        case STDF_BSP_KEY_EVENT_DOUBLE_CLICK:
        case STDF_BSP_KEY_EVENT_TRIPLE_CLICK:
            callback(key_num, event);
            break;
        case STDF_BSP_KEY_EVENT_LONG_PRESS:
            stdf_bsp_key_event_reset(key_num, STDF_BSP_KEY_EVENT_CONTINUOUS_PRESS);
            stdf_bsp_key_event_set(key_num, STDF_BSP_KEY_EVENT_CONTINUOUS_PRESS);
            callback(key_num, event);
            break;
        case STDF_BSP_KEY_EVENT_LONG_LONG_PRESS:
            callback(key_num, event);
            break;
        case STDF_BSP_KEY_EVENT_CONTINUOUS_PRESS:
            stdf_bsp_key_event_reset(key_num, STDF_BSP_KEY_EVENT_CONTINUOUS_PRESS);
            stdf_bsp_key_event_set(key_num, STDF_BSP_KEY_EVENT_CONTINUOUS_PRESS);
            callback(key_num, event);
            break;
        default:
            STDF_BSP_KEY_ASSERT(false);
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
void stdf_bsp_key_init(void)
{ 
    stdf_bsp_key_gpio_init();
}

/*******************************************************************************
 * @fn      .
 * @brief   .
 * @param   .
 * @return  .
 * @notice  .
 */
void stdf_bsp_key_register_event_callback(stdf_bsp_key_num_t key_num, 
                                          stdf_bsp_key_event_callback_t callback, 
                                          stdf_bsp_key_event_mask_t event_mask)

{
    stdf_bsp_key_data[key_num].event_callback = callback;
    stdf_bsp_key_data[key_num].event_mask = event_mask;
}

/*******************************************************************************
 * @fn      .
 * @brief   .
 * @param   .
 * @return  .
 * @notice  .
 */
bool stdf_bsp_key_is_pressed(void)
{
    return (bool)hal_gpio_pin_get_val(stdf_bsp_key_int_pinmux.pin);
}

/*******************************************************************************
*******************************************************************************/

