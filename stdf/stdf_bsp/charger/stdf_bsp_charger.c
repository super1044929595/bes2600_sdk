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
#include "stdf_bsp.h"
#include "stdf_bsp_charger.h"
#include "stdf_hal_pmu.h"
#include "stdf_os.h"

#include "able_driver_iic.h"
#include "mcu_sy5501.h"

#include "hal_iomux.h"

/*******************************************************************************
 * MACROS
 */
#define STDF_BSP_CHG_LOG(str, ...)          STDF_LOG("[BSP][CHG] %s "str, __func__, ##__VA_ARGS__)
#define STDF_BSP_CHG_ASSERT(cond)           STDF_ASSERT(cond)

/*******************************************************************************
 * TYPEDEFS
 */
typedef enum
{
    STDF_BSP_CHG_IMSG_CHARGER_PLUG_OUT,
    STDF_BSP_CHG_IMSG_CHARGER_PLUG_IN,
    STDF_BSP_CHG_IMSG_CHARGER_OUT_CASE,
    STDF_BSP_CHG_IMSG_CHARGER_IN_CASE,
    STDF_BSP_CHG_IMSG_SY5501_EVENT,
} stdf_bsp_charger_imsg_t;


/*******************************************************************************
* GLOBAL VARIABLES
*/
bool stdf_bsp_charger_plug_in;
bool stdf_bsp_charger_in_case;
stdf_bsp_charger_callback_t stdf_bsp_charger_callback;

/*******************************************************************************
 * EXTERNAL VARIABLES
 */

/*******************************************************************************
 * FUNCTIONS
 */
static void stdf_bsp_charger_msg_handler(stdf_os_msg_id_t msg_id, void *payload);

/*******************************************************************************
 * @brief   .
 */
bool stdf_bsp_charger_get_plug_in_state(void)
{
    return stdf_bsp_charger_plug_in;
}

/*******************************************************************************
 * @brief   .
 */
static void stdf_bsp_charger_set_plug_in_state(bool plug_in)
{
    STDF_BSP_CHG_LOG("plug_in %d -> %d", stdf_bsp_charger_plug_in, plug_in);
    stdf_bsp_charger_plug_in = plug_in;
}

/*******************************************************************************
 * @brief   .
 */
bool stdf_bsp_charger_get_in_case_state(void)
{
    return stdf_bsp_charger_in_case;
}

/*******************************************************************************
 * @brief   .
 */
static void stdf_bsp_charger_set_in_case_state(bool in_case)
{
    STDF_BSP_CHG_LOG("in_case %d -> %d", stdf_bsp_charger_in_case, in_case);
    stdf_bsp_charger_in_case = in_case;
}

/*******************************************************************************
 * @brief   .
 */
static void stdf_bsp_charger_pmu_callback(bool plug_in)
{
    stdf_os_msg_id_t msg_id = plug_in ? STDF_BSP_CHG_IMSG_CHARGER_PLUG_IN : \
                                        STDF_BSP_CHG_IMSG_CHARGER_PLUG_OUT;
    MessageSend(stdf_bsp_charger_msg_handler, msg_id, NULL);
    stdf_bsp_charger_set_plug_in_state(plug_in);
}

/*******************************************************************************
 * @brief   .
 */
static void stdf_bsp_charger_sy5501_irq_callback(enum HAL_GPIO_PIN_T pin)
{
    STDF_BSP_CHG_LOG("");
    MessageSend(stdf_bsp_charger_msg_handler, STDF_BSP_CHG_IMSG_SY5501_EVENT, NULL);

}

/*******************************************************************************
 * @brief   .
 */
static void stdf_bsp_charger_plug_out_handler(void)
{
    STDF_BSP_CHG_LOG("");
    stdf_bsp_charger_set_plug_in_state(false);
    if(stdf_bsp_charger_callback)
    {
        MessageSend(stdf_bsp_charger_callback, STDF_OS_GMSG_ID_CHG_PLUG_OUT, NULL);
    }
}

/*******************************************************************************
 * @brief   .
 */
static void stdf_bsp_charger_plug_in_handler(void)
{
    STDF_BSP_CHG_LOG("");
    stdf_bsp_charger_set_plug_in_state(true);
    if(stdf_bsp_charger_callback)
    {
        MessageSend(stdf_bsp_charger_callback, STDF_OS_GMSG_ID_CHG_PLUG_IN, NULL);
    }
}

/*******************************************************************************
 * @brief   .
 */
static void stdf_bsp_charger_out_case_handler(void)
{
    STDF_BSP_CHG_LOG("");
    stdf_bsp_charger_set_in_case_state(false);
    if(stdf_bsp_charger_callback)
    {
        MessageSend(stdf_bsp_charger_callback, STDF_OS_GMSG_ID_CASE_PLUG_OUT, NULL);
    }
}

/*******************************************************************************
 * @brief   .
 */
static void stdf_bsp_charger_in_case_handler(void)
{
    STDF_BSP_CHG_LOG("");
    stdf_bsp_charger_set_in_case_state(true);
    if(stdf_bsp_charger_callback)
    {
        MessageSend(stdf_bsp_charger_callback, STDF_OS_GMSG_ID_CASE_PLUG_IN, NULL);
    }
}

/*******************************************************************************
 * @brief   .
 */
static void stdf_bsp_charger_sy5501_event_handler(void)
{
	uint8_t state0_reg = sy5501_read_state0_reg();
	bool in_case_state = (bool)(state0_reg & ST_CASE_BIT_MARK);

    STDF_BSP_CHG_LOG("state0_reg %x in_case_state %d", state0_reg, in_case_state);
	if((state0_reg & ST_CHIP_BIT_MARK) == SY5501_IS_FULL_CHARGE_MODE)
    {
		STDF_BSP_CHG_LOG("full charged");
	}
    
    if(in_case_state != stdf_bsp_charger_get_in_case_state())
    {
        stdf_os_msg_id_t msg_id = in_case_state ? STDF_BSP_CHG_IMSG_CHARGER_IN_CASE : \
                                                  STDF_BSP_CHG_IMSG_CHARGER_OUT_CASE;
        MessageSend(stdf_bsp_charger_msg_handler, msg_id, NULL);
    }
}

/*******************************************************************************
 * @brief   .
 */
static void stdf_bsp_charger_msg_handler(stdf_os_msg_id_t msg_id, void *payload)
{
    switch(msg_id)
    {
        case STDF_BSP_CHG_IMSG_CHARGER_PLUG_OUT:
            stdf_bsp_charger_plug_out_handler();
            break;
        
        case STDF_BSP_CHG_IMSG_CHARGER_PLUG_IN:
            stdf_bsp_charger_plug_in_handler();
            break;
        
        case STDF_BSP_CHG_IMSG_CHARGER_OUT_CASE:
            stdf_bsp_charger_out_case_handler();
            break;
        
        case STDF_BSP_CHG_IMSG_CHARGER_IN_CASE:
            stdf_bsp_charger_in_case_handler();
            break;
        
        case STDF_BSP_CHG_IMSG_SY5501_EVENT:
            stdf_bsp_charger_sy5501_event_handler();
            break;
        
        default:
            break;
    }
}

/*******************************************************************************
 * @brief   .
 */
void stdf_bsp_charger_init(void)
{
    able_driver_iic_init();
    
    stdf_hal_pmu_register_callback(stdf_bsp_charger_pmu_callback);
    bool plug_in_state = stdf_hal_pmu_is_plug_in();
    stdf_bsp_charger_set_plug_in_state(plug_in_state);
    
    sy5501_init((uint32_t)stdf_bsp_charger_sy5501_irq_callback);
    bool in_case_state = (sy5501_read_st_case() == INBOX_STATUS);
    stdf_bsp_charger_set_in_case_state(in_case_state);
}

/*******************************************************************************
 * @brief   .
 */
void stdf_bsp_charger_register_callback(stdf_bsp_charger_callback_t callback)
{
    stdf_bsp_charger_callback = callback;
}

/*******************************************************************************
*******************************************************************************/

