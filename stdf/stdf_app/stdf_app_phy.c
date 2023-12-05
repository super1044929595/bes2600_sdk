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
#include "stdf_app_phy.h"
#ifdef __STDF_EXTERNAL_CHAGER_ENABLE__
#include "stdf_bsp_charger.h"
#endif
#include "stdf_define.h"
#include "stdf_os.h"
#include "stdf_sdk_api.h"

/*******************************************************************************
 * MACROS
 */
#define STDF_APP_PHY_INIT_DELAY_TIME        200 // ms

#define STDF_APP_PHY_LOG(str, ...)          STDF_LOG("[APP][PHY] %s "str, __func__, ##__VA_ARGS__)
#define STDF_APP_PHY_ASSERT(cond)           STDF_ASSERT(cond)

/*******************************************************************************
 * TYPEDEFS
 */
typedef enum
{
    STDF_APP_PHY_IMSG_ID_INIT,
    STDF_APP_PHY_IMSG_ID_CASE_OPEN,
    STDF_APP_PHY_IMSG_ID_CASE_CLOSE,
    STDF_APP_PHY_IMSG_ID_OUT_CASE,
    STDF_APP_PHY_IMSG_ID_IN_CASE,
    STDF_APP_PHY_IMSG_ID_OUT_EAR,
    STDF_APP_PHY_IMSG_ID_IN_EAR,
} stdf_app_phy_imsg_id_t;

typedef struct
{
    bool  initialized;
    bool  case_close;
    bool  in_case;
    bool  in_ear;
} stdf_app_phy_data_t;

/*******************************************************************************
* GLOBAL VARIABLES
*/
stdf_app_phy_data_t stdf_app_phy_data;

/*******************************************************************************
 * EXTERNAL VARIABLES
 */

/*******************************************************************************
 * FUNCTIONS
 */
static void stdf_app_phy_msg_handler(stdf_os_msg_id_t msg_id, void *payload);

/*******************************************************************************
 * @brief   .
 */
void stdf_app_phy_event_case_open(void)
{
    STDF_APP_PHY_LOG("");
    MessageSend(stdf_app_phy_msg_handler, STDF_APP_PHY_IMSG_ID_CASE_OPEN, NULL);
}

/*******************************************************************************
 * @brief   .
 */
void stdf_app_phy_event_case_close(void)
{
    STDF_APP_PHY_LOG("");
    MessageSend(stdf_app_phy_msg_handler, STDF_APP_PHY_IMSG_ID_CASE_CLOSE, NULL);
}

/*******************************************************************************
 * @brief   .
 */
void stdf_app_phy_event_out_case(void)
{
    STDF_APP_PHY_LOG("");
    MessageSend(stdf_app_phy_msg_handler, STDF_APP_PHY_IMSG_ID_OUT_CASE, NULL);
}

/*******************************************************************************
 * @brief   .
 */
void stdf_app_phy_event_in_case(void)
{
    STDF_APP_PHY_LOG("");
    MessageSend(stdf_app_phy_msg_handler, STDF_APP_PHY_IMSG_ID_IN_CASE, NULL);
}

/*******************************************************************************
 * @brief   .
 */
void stdf_app_phy_event_out_ear(void)
{
    STDF_APP_PHY_LOG("");
    MessageSend(stdf_app_phy_msg_handler, STDF_APP_PHY_IMSG_ID_OUT_EAR, NULL);
}

/*******************************************************************************
 * @brief   .
 */
void stdf_app_phy_event_in_ear(void)
{
    STDF_APP_PHY_LOG("");
    MessageSend(stdf_app_phy_msg_handler, STDF_APP_PHY_IMSG_ID_IN_EAR, NULL);
}

/*******************************************************************************
 * @brief   .
 */
stdf_app_phy_state_t stdf_app_phy_get_state(void)
{
    stdf_app_phy_state_t state;
    bool  case_close = stdf_app_phy_data.case_close;
    bool  in_case = stdf_app_phy_data.in_case;
    bool  in_ear = stdf_app_phy_data.in_ear;
    
    if(case_close && in_case)
    {
        state = STDF_APP_PHY_STATE_IN_CASE_CLOSE;
    }
    else if(in_case)
    {
        state = STDF_APP_PHY_STATE_IN_CASE_OPEN;
    }
    else if(in_ear)
    {
        state = STDF_APP_PHY_STATE_IN_EAR;
    }
    else
    {
        state = STDF_APP_PHY_STATE_OUT_CASE_EAR;
    }
    return state;
}

/*******************************************************************************
 * @brief   .
 */
static void stdf_app_phy_imsg_init_handler(void)
{
    stdf_app_phy_data.case_close  = stdf_bsp_charger_get_plug_in_state();
    stdf_app_phy_data.in_case     = stdf_bsp_charger_get_in_case_state();
    stdf_app_phy_data.in_ear      = true;
    stdf_app_phy_data.initialized = true;
    stdf_app_phy_state_t phy_state = stdf_app_phy_get_state();
    
    STDF_APP_PHY_LOG("state %d %d %d", stdf_app_phy_data.case_close, 
                     stdf_app_phy_data.in_case, stdf_app_phy_data.in_ear);
    STDF_APP_PHY_LOG("phy_state %d", phy_state);
    
    switch(phy_state)
    {
        case STDF_APP_PHY_STATE_IN_CASE_CLOSE:
            stdf_sdk_api_phy_set_state_in_case_close();
            break;
            
        case STDF_APP_PHY_STATE_IN_CASE_OPEN:
            stdf_sdk_api_phy_set_state_in_case_open();
            break;
            
        case STDF_APP_PHY_STATE_OUT_CASE_EAR:
            stdf_sdk_api_phy_set_state_in_case_open();
            stdf_sdk_api_phy_set_state_out_case_ear();
            break;
            
        case STDF_APP_PHY_STATE_IN_EAR:
            stdf_sdk_api_phy_set_state_in_case_open();
            stdf_sdk_api_phy_set_state_out_case_ear();
            stdf_sdk_api_phy_set_state_in_ear();
            break;
    }
}

/*******************************************************************************
 * @brief   IN_EAR/OUT_CASE_EAR/IN_CASE_OPEN -> CASE_CLOSE.
 */
static void stdf_app_phy_imsg_case_close_handler(void)
{
    stdf_app_phy_state_t old_state = stdf_app_phy_get_state();
    stdf_app_phy_data.case_close = true;
    stdf_app_phy_state_t new_state = stdf_app_phy_get_state();
    stdf_os_handler_t handler = stdf_app_phy_msg_handler;
    
    STDF_APP_PHY_LOG("state %d %d %d", stdf_app_phy_data.case_close, 
                     stdf_app_phy_data.in_case, stdf_app_phy_data.in_ear);
    STDF_APP_PHY_LOG("phy_state %d -> %d", old_state, new_state);

    if(!stdf_app_phy_data.initialized)
    {
        STDF_APP_PHY_LOG("not initialized return!!!");
        return;
    }
    
    if(old_state == STDF_APP_PHY_STATE_IN_EAR)
    {
        if(new_state == STDF_APP_PHY_STATE_IN_CASE_CLOSE)
        {
            stdf_sdk_api_phy_set_state_out_case_ear();
            MessageSend(handler, STDF_OS_GMSG_ID_PHY_OUT_CASE_EAR, NULL);
        }
    }
    if(old_state == STDF_APP_PHY_STATE_IN_EAR || \
       old_state == STDF_APP_PHY_STATE_OUT_CASE_EAR)
    {
        if(new_state == STDF_APP_PHY_STATE_IN_CASE_CLOSE)
        {
            stdf_sdk_api_phy_set_state_in_case_open();
            MessageSend(handler, STDF_OS_GMSG_ID_PHY_IN_CASE_OPEN, NULL);
        }
    }
    if(old_state == STDF_APP_PHY_STATE_IN_EAR || \
       old_state == STDF_APP_PHY_STATE_OUT_CASE_EAR || \
       old_state == STDF_APP_PHY_STATE_IN_CASE_OPEN)
    {
        if(new_state == STDF_APP_PHY_STATE_IN_CASE_CLOSE)
        {
            stdf_sdk_api_phy_set_state_in_case_close();
            MessageSend(handler, STDF_OS_GMSG_ID_PHY_IN_CASE_CLOSE, NULL);
        }
    }
}

/*******************************************************************************
 * @brief   CASE_CLOSE -> IN_EAR/OUT_CASE_EAR/IN_CASE_CLOSE.
 */
static void stdf_app_phy_imsg_case_open_handler(void)
{
    stdf_app_phy_state_t old_state = stdf_app_phy_get_state();
    stdf_app_phy_data.case_close = false;
    stdf_app_phy_state_t new_state = stdf_app_phy_get_state();
    stdf_os_handler_t handler = stdf_app_phy_msg_handler;
    
    STDF_APP_PHY_LOG("state %d %d %d", stdf_app_phy_data.case_close, 
                     stdf_app_phy_data.in_case, stdf_app_phy_data.in_ear);
    STDF_APP_PHY_LOG("phy_state %d -> %d", old_state, new_state);

    if(!stdf_app_phy_data.initialized)
    {
        STDF_APP_PHY_LOG("not initialized return!!!");
        return;
    }
    
    if(old_state == STDF_APP_PHY_STATE_IN_CASE_CLOSE)
    {
        if(new_state == STDF_APP_PHY_STATE_IN_EAR ||
           new_state == STDF_APP_PHY_STATE_OUT_CASE_EAR ||
           new_state == STDF_APP_PHY_STATE_IN_CASE_OPEN)
        {
            stdf_sdk_api_phy_set_state_in_case_open();
            MessageSend(handler, STDF_OS_GMSG_ID_PHY_IN_CASE_OPEN, NULL);
        }
        if(new_state == STDF_APP_PHY_STATE_IN_EAR ||
           new_state == STDF_APP_PHY_STATE_OUT_CASE_EAR)
        {
            stdf_sdk_api_phy_set_state_out_case_ear();
            MessageSend(handler, STDF_OS_GMSG_ID_PHY_OUT_CASE_EAR, NULL);
        }
        if(new_state == STDF_APP_PHY_STATE_IN_EAR)
        {
            stdf_sdk_api_phy_set_state_in_ear();
            MessageSend(handler, STDF_OS_GMSG_ID_PHY_IN_EAR, NULL);
        }
    }
}

/*******************************************************************************
 * @brief   IN_CASE_OPEN -> OUT_CASE_EAR/IN_EAR.
 */
static void stdf_app_phy_imsg_out_case_handler(void)
{
    stdf_app_phy_state_t old_state = stdf_app_phy_get_state();
    stdf_app_phy_data.in_case = false;
    stdf_app_phy_state_t new_state = stdf_app_phy_get_state();
    stdf_os_handler_t handler = stdf_app_phy_msg_handler;
    
    // log related state
    STDF_APP_PHY_LOG("state %d %d %d", stdf_app_phy_data.case_close, 
                     stdf_app_phy_data.in_case, stdf_app_phy_data.in_ear);
    STDF_APP_PHY_LOG("phy_state %d -> %d", old_state, new_state);
    
    if(!stdf_app_phy_data.initialized)
    {
        STDF_APP_PHY_LOG("not initialized return!!!");
        return;
    }
    
    if(old_state == STDF_APP_PHY_STATE_IN_CASE_CLOSE)
    {
        if(new_state == STDF_APP_PHY_STATE_IN_EAR ||
           new_state == STDF_APP_PHY_STATE_OUT_CASE_EAR ||
           new_state == STDF_APP_PHY_STATE_IN_CASE_OPEN)
        {
            stdf_sdk_api_phy_set_state_in_case_open();
            MessageSend(handler, STDF_OS_GMSG_ID_PHY_IN_CASE_OPEN, NULL);
        }
        if(new_state == STDF_APP_PHY_STATE_IN_EAR ||
           new_state == STDF_APP_PHY_STATE_OUT_CASE_EAR)
        {
            stdf_sdk_api_phy_set_state_out_case_ear();
            MessageSend(handler, STDF_OS_GMSG_ID_PHY_OUT_CASE_EAR, NULL);
        }
        if(new_state == STDF_APP_PHY_STATE_IN_EAR)
        {
            stdf_sdk_api_phy_set_state_in_ear();
            MessageSend(handler, STDF_OS_GMSG_ID_PHY_IN_EAR, NULL);
        }
    }
    if(old_state == STDF_APP_PHY_STATE_IN_CASE_OPEN)
    {
        if(new_state == STDF_APP_PHY_STATE_IN_EAR ||
           new_state == STDF_APP_PHY_STATE_OUT_CASE_EAR)
        {
            stdf_sdk_api_phy_set_state_out_case_ear();
            MessageSend(handler, STDF_OS_GMSG_ID_PHY_OUT_CASE_EAR, NULL);
        }
        if(new_state == STDF_APP_PHY_STATE_IN_EAR)
        {
            stdf_sdk_api_phy_set_state_in_ear();
            MessageSend(handler, STDF_OS_GMSG_ID_PHY_IN_EAR, NULL);
        }
    }    
}

/*******************************************************************************
 * @brief   IN_EAR/OUT_CASE_EAR -> IN_CASE_OPEN.
 */
static void stdf_app_phy_imsg_in_case_handler(void)
{
    stdf_app_phy_state_t old_state = stdf_app_phy_get_state();
    stdf_app_phy_data.in_case = true;
    stdf_app_phy_state_t new_state = stdf_app_phy_get_state();
    stdf_os_handler_t handler = stdf_app_phy_msg_handler;
    
    // log related state
    STDF_APP_PHY_LOG("state %d %d %d", stdf_app_phy_data.case_close, 
                     stdf_app_phy_data.in_case, stdf_app_phy_data.in_ear);
    STDF_APP_PHY_LOG("phy_state %d -> %d", old_state, new_state);
    
    if(!stdf_app_phy_data.initialized)
    {
        STDF_APP_PHY_LOG("not initialized return!!!");
        return;
    }
    
    if(old_state == STDF_APP_PHY_STATE_IN_EAR || \
       old_state == STDF_APP_PHY_STATE_OUT_CASE_EAR)
    {
        if(new_state == STDF_APP_PHY_STATE_IN_CASE_OPEN)
        {
            stdf_sdk_api_phy_set_state_out_case_ear();
            MessageSend(handler, STDF_OS_GMSG_ID_PHY_OUT_CASE_EAR, NULL);
        }
    }
    if(old_state == STDF_APP_PHY_STATE_OUT_CASE_EAR)
    {
        if(new_state == STDF_APP_PHY_STATE_IN_CASE_OPEN)
        {
            stdf_sdk_api_phy_set_state_in_case_open();
            MessageSend(handler, STDF_OS_GMSG_ID_PHY_IN_CASE_OPEN, NULL);
        }
    }
}

/*******************************************************************************
 * @brief   IN_EAR -> OUT_CASE_EAR.
 */
static void stdf_app_phy_imsg_out_ear_handler(void)
{
    stdf_app_phy_state_t old_state = stdf_app_phy_get_state();
    stdf_app_phy_data.in_ear = false;
    stdf_app_phy_state_t new_state = stdf_app_phy_get_state();
    stdf_os_handler_t handler = stdf_app_phy_msg_handler;
    
    // log related state
    STDF_APP_PHY_LOG("state %d %d %d", stdf_app_phy_data.case_close, 
                     stdf_app_phy_data.in_case, stdf_app_phy_data.in_ear);
    STDF_APP_PHY_LOG("phy_state %d -> %d", old_state, new_state);
    
    if(!stdf_app_phy_data.initialized)
    {
        STDF_APP_PHY_LOG("not initialized return!!!");
        return;
    }
    
    if(old_state == STDF_APP_PHY_STATE_IN_EAR)
    {
        if(new_state == STDF_APP_PHY_STATE_OUT_CASE_EAR)
        {
            stdf_sdk_api_phy_set_state_out_case_ear();
            MessageSend(handler, STDF_OS_GMSG_ID_PHY_OUT_CASE_EAR, NULL);
        }
    }
}

/*******************************************************************************
 * @brief   OUT_CASE_EAR -> IN_EAR.
 */
static void stdf_app_phy_imsg_in_ear_handler(void)
{
    stdf_app_phy_state_t old_state = stdf_app_phy_get_state();
    stdf_app_phy_data.in_ear = true;
    stdf_app_phy_state_t new_state = stdf_app_phy_get_state();
    stdf_os_handler_t handler = stdf_app_phy_msg_handler;
    
    // log related state
    STDF_APP_PHY_LOG("state %d %d %d", stdf_app_phy_data.case_close, 
                     stdf_app_phy_data.in_case, stdf_app_phy_data.in_ear);
    STDF_APP_PHY_LOG("phy_state %d -> %d", old_state, new_state);
    
    if(!stdf_app_phy_data.initialized)
    {
        STDF_APP_PHY_LOG("not initialized return!!!");
        return;
    }
    
    if(old_state == STDF_APP_PHY_STATE_OUT_CASE_EAR)
    {
        if(new_state == STDF_APP_PHY_STATE_IN_EAR)
        {
            stdf_sdk_api_phy_set_state_in_ear();
            MessageSend(handler, STDF_OS_GMSG_ID_PHY_IN_EAR, NULL);
        }
    }
}

/*******************************************************************************
 * @brief   .
 */
static void stdf_app_phy_gmsg_charger_plug_out_handler(void)
{
    STDF_APP_PHY_LOG("");
}

/*******************************************************************************
 * @brief   .
 */
static void stdf_app_phy_gmsg_charger_plug_in_handler(void)
{
    STDF_APP_PHY_LOG("");
    stdf_app_phy_event_case_close();
}

/*******************************************************************************
 * @brief   .
 */
static void stdf_app_phy_gmsg_case_plug_out_handler(void)
{
    STDF_APP_PHY_LOG("");
    stdf_app_phy_event_out_case();
}

/*******************************************************************************
 * @brief   .
 */
static void stdf_app_phy_gmsg_case_plug_in_handler(void)
{
    STDF_APP_PHY_LOG("");
    stdf_app_phy_event_in_case();
}

/*******************************************************************************
 * @brief   .
 */
static void stdf_app_phy_gmsg_in_case_close_handler(void)
{
    STDF_APP_PHY_LOG("");
}

/*******************************************************************************
 * @brief   .
 */
static void stdf_app_phy_gmsg_in_case_open_handler(void)
{
    STDF_APP_PHY_LOG("");
}

/*******************************************************************************
 * @brief   .
 */
static void stdf_app_phy_gmsg_out_case_ear_handler(void)
{
    STDF_APP_PHY_LOG("");
}

/*******************************************************************************
 * @brief   .
 */
static void stdf_app_phy_gmsg_in_ear_handler(void)
{
    STDF_APP_PHY_LOG("");
}

/*******************************************************************************
 * @brief   .
 */
static void stdf_app_phy_msg_handler(stdf_os_msg_id_t msg_id, void *payload)
{
    switch(msg_id)
    {
        // Internal message handler for phy event
        case STDF_APP_PHY_IMSG_ID_INIT:
            stdf_app_phy_imsg_init_handler();
            break;
            
        case STDF_APP_PHY_IMSG_ID_CASE_CLOSE:
            stdf_app_phy_imsg_case_close_handler();
            break;
        
        case STDF_APP_PHY_IMSG_ID_CASE_OPEN:
            stdf_app_phy_imsg_case_open_handler();
            break;
            
        case STDF_APP_PHY_IMSG_ID_OUT_CASE:
            stdf_app_phy_imsg_out_case_handler();
            break;
        
        case STDF_APP_PHY_IMSG_ID_IN_CASE:
            stdf_app_phy_imsg_in_case_handler();
            break;
        
        case STDF_APP_PHY_IMSG_ID_OUT_EAR:
            stdf_app_phy_imsg_out_ear_handler();
            break;
        
        case STDF_APP_PHY_IMSG_ID_IN_EAR:
            stdf_app_phy_imsg_in_ear_handler();
            break;
            
        // Global message handler for charger
        case STDF_OS_GMSG_ID_CHG_PLUG_OUT:
            stdf_app_phy_gmsg_charger_plug_out_handler();
            break;
            
        case STDF_OS_GMSG_ID_CHG_PLUG_IN:
            stdf_app_phy_gmsg_charger_plug_in_handler();
            break;
            
        case STDF_OS_GMSG_ID_CASE_PLUG_OUT:
            stdf_app_phy_gmsg_case_plug_out_handler();
            break;
            
        case STDF_OS_GMSG_ID_CASE_PLUG_IN:
            stdf_app_phy_gmsg_case_plug_in_handler();
            break;
            
        // Global message handler for phy state changed
        case STDF_OS_GMSG_ID_PHY_IN_CASE_CLOSE:
            stdf_app_phy_gmsg_in_case_close_handler();
            break;
            
        case STDF_OS_GMSG_ID_PHY_IN_CASE_OPEN:
            stdf_app_phy_gmsg_in_case_open_handler();
            break;
        
        case STDF_OS_GMSG_ID_PHY_OUT_CASE_EAR:
            stdf_app_phy_gmsg_out_case_ear_handler();
            break;
        
        case STDF_OS_GMSG_ID_PHY_IN_EAR:
            stdf_app_phy_gmsg_in_ear_handler();
            break;
    }
}

/*******************************************************************************
 * @brief   .
 */
void stdf_app_phy_init(void)
{
#ifdef __STDF_EXTERNAL_CHAGER_ENABLE__
    stdf_bsp_charger_register_callback(stdf_app_phy_msg_handler);
#endif
    
    MessageSendLater(stdf_app_phy_msg_handler, STDF_APP_PHY_IMSG_ID_INIT, NULL, 
                     STDF_APP_PHY_INIT_DELAY_TIME);
}

/*******************************************************************************
*******************************************************************************/


