/*******************************************************************************
  Filename:       .

  Version         V1.0.

  Author          Yuping.Mo.

  Description:    .


  IMPORTANT:      .

*******************************************************************************/

#ifdef __STDF_HAL_PMU_ENABLE__

/*******************************************************************************
 * INCLUDES
 */
#include "stdf_define.h" 
#include "stdf_hal_pmu.h"
#include "stdf_os.h"

#include "hal_gpio.h"
#include "hal_iomux.h"
#include "pmu.h"

/*******************************************************************************
 * MACROS
 */
#define STDF_HAL_PMU_PLUG_IN_DEBOUNCE_TIME  200
#define STDF_HAL_PMU_PLUG_OUT_DEBOUNCE_TIME 200

#define STDF_HAL_PMU_LOG(str, ...)          STDF_LOG("[HAL][PMU] %s "str, __func__, ##__VA_ARGS__)
#define STDF_HAL_PMU_ASSERT(cond)           STDF_ASSERT(cond)

/*******************************************************************************
 * TYPEDEFS
 */
typedef enum
{
    STDF_HAL_PMU_IMSG_PLUG_IN,
    STDF_HAL_PMU_IMSG_PLUG_OUT,
    STDF_HAL_PMU_IMSG_PLUG_IN_DEBOUNCED,
    STDF_HAL_PMU_IMSG_PLUG_OUT_DEBOUNCED
} stdf_hal_pmu_imsg_t;

//
typedef struct
{
    bool                    plug_in;
    bool                    plug_in_debounced; 
    stdf_hal_pmu_callback_t callback;
} stdf_hal_pmu_data_t;

/*******************************************************************************
* GLOBAL VARIABLES
*/
static stdf_hal_pmu_data_t stdf_hal_pmu_data;

/*******************************************************************************
 * EXTERNAL VARIABLES
 */


/*******************************************************************************
 * FUNCTIONS
 */
static void stdf_hal_pmu_msg_handler(stdf_os_msg_id_t msg_id, void *payload);

/*******************************************************************************
 * @fn      .
 * @brief   .
 * @param   .
 * @return  .
 * @notice  .
 */
static void stdf_hal_pmu_plug_in_handler(void)
{
    STDF_HAL_PMU_LOG("");

    stdf_hal_pmu_data.plug_in = true;
    MessageCancelAll(stdf_hal_pmu_msg_handler, STDF_HAL_PMU_IMSG_PLUG_IN_DEBOUNCED);
    MessageCancelAll(stdf_hal_pmu_msg_handler, STDF_HAL_PMU_IMSG_PLUG_OUT_DEBOUNCED);
    MessageSendLater(stdf_hal_pmu_msg_handler, STDF_HAL_PMU_IMSG_PLUG_IN_DEBOUNCED, 
                     NULL, STDF_HAL_PMU_PLUG_IN_DEBOUNCE_TIME);
}

/*******************************************************************************
 * @fn      .
 * @brief   .
 * @param   .
 * @return  .
 * @notice  .
 */
static void stdf_hal_pmu_plug_out_handler(void)
{
    STDF_HAL_PMU_LOG("");
    
    stdf_hal_pmu_data.plug_in = false;
    MessageCancelAll(stdf_hal_pmu_msg_handler, STDF_HAL_PMU_IMSG_PLUG_IN_DEBOUNCED);
    MessageCancelAll(stdf_hal_pmu_msg_handler, STDF_HAL_PMU_IMSG_PLUG_OUT_DEBOUNCED);
    MessageSendLater(stdf_hal_pmu_msg_handler, STDF_HAL_PMU_IMSG_PLUG_OUT_DEBOUNCED, 
                     NULL, STDF_HAL_PMU_PLUG_OUT_DEBOUNCE_TIME);
}

/*******************************************************************************
 * @fn      .
 * @brief   .
 * @param   .
 * @return  .
 * @notice  .
 */
static void stdf_hal_pmu_plug_in_debounced_handler(void)
{
    stdf_hal_pmu_callback_t callback = stdf_hal_pmu_data.callback;
    
    if(stdf_hal_pmu_data.plug_in && !stdf_hal_pmu_data.plug_in_debounced)
    {
        stdf_hal_pmu_data.plug_in_debounced = true;
        if(callback)
        {   
            callback(true);    
        }
    }
}

/*******************************************************************************
 * @fn      .
 * @brief   .
 * @param   .
 * @return  .
 * @notice  .
 */
static void stdf_hal_pmu_plug_out_debounced_handler(void)
{
    stdf_hal_pmu_callback_t callback = stdf_hal_pmu_data.callback;
    
    if(!stdf_hal_pmu_data.plug_in && stdf_hal_pmu_data.plug_in_debounced)
    {
        stdf_hal_pmu_data.plug_in_debounced = false;
        if(callback)
        {
            callback(false);
        }
    }
}

/*******************************************************************************
 * @fn      .
 * @brief   .
 * @param   .
 * @return  .
 * @notice  .
 */
static void stdf_hal_pmu_msg_handler(stdf_os_msg_id_t msg_id, void *payload)
{
    switch(msg_id)
    {
        case STDF_HAL_PMU_IMSG_PLUG_IN:
            stdf_hal_pmu_plug_in_handler();
            break;            
        case STDF_HAL_PMU_IMSG_PLUG_OUT:
            stdf_hal_pmu_plug_out_handler();
            break;

        case STDF_HAL_PMU_IMSG_PLUG_IN_DEBOUNCED:
            stdf_hal_pmu_plug_in_debounced_handler();
        
        case STDF_HAL_PMU_IMSG_PLUG_OUT_DEBOUNCED:
            stdf_hal_pmu_plug_out_debounced_handler();
            break;
        
        default:
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
static void stdf_hal_pmu_irq_handler(enum PMU_CHARGER_STATUS_T status)
{
    if(status == PMU_CHARGER_PLUGIN)
    {
        MessageSend(stdf_hal_pmu_msg_handler, STDF_HAL_PMU_IMSG_PLUG_IN, NULL);
    }
    else if(status == PMU_CHARGER_PLUGOUT)
    {
        MessageSend(stdf_hal_pmu_msg_handler, STDF_HAL_PMU_IMSG_PLUG_OUT, NULL);
    }
    else
    {
        STDF_HAL_PMU_ASSERT(false);
    }
}

/*******************************************************************************
 * @fn      .
 * @brief   .
 * @param   .
 * @return  .
 * @notice  .
 */
void stdf_hal_pmu_init(void)
{
    pmu_charger_init();
    pmu_charger_set_irq_handler(stdf_hal_pmu_irq_handler);

    // init state
    enum PMU_CHARGER_STATUS_T status = pmu_charger_get_status();
    STDF_HAL_PMU_LOG("status %d", status);
    if(status == PMU_CHARGER_PLUGIN)
    {
        stdf_hal_pmu_data.plug_in = true;
        stdf_hal_pmu_data.plug_in_debounced = true;
    }
    else
    {
        stdf_hal_pmu_data.plug_in = false;
        stdf_hal_pmu_data.plug_in_debounced = false;
    }
}

/*******************************************************************************
 * @fn      .
 * @brief   .
 * @param   .
 * @return  .
 * @notice  .
 */
void stdf_hal_pmu_register_callback(stdf_hal_pmu_callback_t callback)
{
    stdf_hal_pmu_data.callback = callback;
}

/*******************************************************************************
 * @fn      .
 * @brief   .
 * @param   .
 * @return  .
 * @notice  .
 */
bool stdf_hal_pmu_is_plug_in(void)
{
    return stdf_hal_pmu_data.plug_in_debounced;
}

/*******************************************************************************
*******************************************************************************/

#endif /* __STDF_HAL_PMU_ENABLE__ */
