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
#include "cmsis_os.h"
#include "hal_timer.h"
#include "hal_trace.h"

#include "stdf_define.h"
#include "stdf_os_config.h"
#include "stdf_os_msg.h"
#include "stdf_os_delay_msg.h"

/*******************************************************************************
 * MACROS
 */
#define STDF_OS_DELAY_MSG_FOREVER           0xFFFFFFFF // forever
#define STDF_OS_DELAY_MSG_IMMEDIATELY       0 // immediately

// Maximum number of pendding message
#define STDF_OS_DELAY_MSG_MAX_NUM           20 

#define STDF_OS_DELAY_MSG_ENTER_CRITICAL()  STDF_ASSERT(osMutexWait(stdf_os_delay_msg_mutex_id, 500) == osOK)
#define STDF_OS_DELAY_MSG_EXIT_CRITICAL()   osMutexRelease(stdf_os_delay_msg_mutex_id) 

#define STDF_OS_DELAY_MSG_LOG(str, ...)     STDF_LOG("[OS][DMSG] %s "str, __func__, ##__VA_ARGS__)
#define STDF_OS_DELAY_MSG_ASSERT(cond)      STDF_ASSERT(cond)

/*******************************************************************************
 * TYPEDEFS
 */
//
typedef struct
{
    bool               used;            // is used or not
    bool               latest;          // the latest time in the delay table, need to run
    stdf_os_handler_t  handler;         // the handler to recieved the message
    stdf_os_msg_id_t   msg_id;
    void              *payload;         //
    uint32_t           run_time;        // The absolute time to call the handler
} stdf_os_delay_msg_data_t;

/*******************************************************************************
* GLOBAL VARIABLES
*/
//
stdf_os_delay_msg_data_t stdf_os_delay_msg_data[STDF_OS_DELAY_MSG_MAX_NUM];

static osTimerId stdf_os_delay_msg_timer = NULL;

osMutexId stdf_os_delay_msg_mutex_id = NULL;
osMutexDef(STDF_OS_DELAY_MSG_MUTEX);

/*******************************************************************************
 * EXTERNAL VARIABLES
 */

/*******************************************************************************
 * FUNCTIONS
 */

/* -----------------------------------------------------------------------------
 *                              static functions
 * ---------------------------------------------------------------------------*/

/*******************************************************************************
 * @fn      .
 * @brief   .
 * @param   .
 * @return  .
 * @notice  .
 */
static void stdf_os_delay_msg_deinit(uint8_t index)
{
    //STDF_OS_DELAY_MSG_LOG("index %d", index);
    stdf_os_delay_msg_data[index].used     = false;
    stdf_os_delay_msg_data[index].latest   = false;
    stdf_os_delay_msg_data[index].handler  = NULL;
    stdf_os_delay_msg_data[index].msg_id   = STDF_OS_MSG_ID_INVALID;
    stdf_os_delay_msg_data[index].payload  = NULL;
    stdf_os_delay_msg_data[index].run_time = STDF_OS_DELAY_MSG_FOREVER;
}

/*******************************************************************************
 * @fn      .
 * @brief   .
 * @param   .
 * @return  .
 * @notice  .
 */
static uint8_t stdf_os_delay_msg_get_latest(void)
{
    uint8_t index;

    for(index = 0; index < STDF_OS_DELAY_MSG_MAX_NUM; index++)
    {
        if(stdf_os_delay_msg_data[index].used && stdf_os_delay_msg_data[index].latest)
        {
            return index;
        }
    }
    return STDF_OS_DELAY_MSG_MAX_NUM;
}

/*******************************************************************************
 * @fn      .
 * @brief   .
 * @param   .
 * @return  .
 * @notice  .
 */
static void stdf_os_delay_msg_set_latest(uint8_t index, bool enable)
{   
    //STDF_OS_DELAY_MSG_LOG("index %d", index);
    if(index  < STDF_OS_DELAY_MSG_MAX_NUM)
    {
        stdf_os_delay_msg_data[index].latest = enable;
    }
}

/*******************************************************************************
 * @fn      Find the latest timer message and start or restart timer.
 * @brief   .
 * @param   .
 * @return  .
 * @notice  .
 */
static  void stdf_os_delay_msg_timer_start(void)
{
    uint32_t current_ms = GET_CURRENT_MS();
    uint8_t index;
    uint32_t min_run_time = STDF_OS_DELAY_MSG_FOREVER;
    uint8_t min_run_time_index = STDF_OS_DELAY_MSG_MAX_NUM;

    // find the min run time
    for(index = 0; index < STDF_OS_DELAY_MSG_MAX_NUM; index++)
    {
        if(stdf_os_delay_msg_data[index].used)
        {
            if(stdf_os_delay_msg_data[index].run_time < min_run_time)
            {
                min_run_time = stdf_os_delay_msg_data[index].run_time;
                min_run_time_index = index;
            }
        }
    }

    if(min_run_time != STDF_OS_DELAY_MSG_FOREVER && 
       min_run_time_index < STDF_OS_DELAY_MSG_MAX_NUM)
    {   
        //stdf_os_handler_t handler = stdf_os_delay_msg_data[min_run_time_index].handler;
        //stdf_os_msg_id_t msg_id = stdf_os_delay_msg_data[min_run_time_index].msg_id;
        //void *payload = stdf_os_delay_msg_data[min_run_time_index].payload;
        uint32_t run_time = stdf_os_delay_msg_data[min_run_time_index].run_time;
    
        //STDF_OS_DELAY_MSG_LOG("func %p msg_id %d payload %p, run_time %d current_ms %d",
        //                      handler, msg_id, payload, run_time, current_ms);
        
        // check if the time is too late to call handle
        if(run_time <= current_ms)
        {
            run_time = current_ms + 1;
        }     

        // set latest message
        uint8_t old_latest = stdf_os_delay_msg_get_latest();
        if(old_latest != min_run_time_index)
        {
            stdf_os_delay_msg_set_latest(old_latest, false);
        }
        stdf_os_delay_msg_set_latest(min_run_time_index, true);

        // start or restart timer
        if(osTimerIsRunning(stdf_os_delay_msg_timer)) 
        {
            osTimerStop(stdf_os_delay_msg_timer);
        }
        osTimerStart(stdf_os_delay_msg_timer, run_time - current_ms);
    }
    else
    {
        //STDF_OS_DELAY_MSG_LOG("message is empty");
    }
}

/*******************************************************************************
 * @fn      .
 * @brief   .
 * @param   .
 * @return  .
 * @notice  .
 */
static bool stdf_os_delay_msg_timer_is_run(void)
{
    return (bool)osTimerIsRunning(stdf_os_delay_msg_timer);
}

/*******************************************************************************
 * @fn      .
 * @brief   .
 * @param   .
 * @return  .
 * @notice  .
 */
static void stdf_os_delay_msg_timer_stop(void)
{
    //STDF_OS_DELAY_MSG_LOG("");
    if(osTimerIsRunning(stdf_os_delay_msg_timer)) 
    {
        osTimerStop(stdf_os_delay_msg_timer);
    }
}

/*******************************************************************************
 * @fn      .
 * @brief   .
 * @param   .
 * @return  .
 * @notice  .
 */
static void stdf_os_delay_msg_timer_timeout(void)
{
    stdf_os_handler_t handler;
    stdf_os_msg_id_t msg_id;
    void *payload;

    // call handle and remove all handle data from the latest from message table
    STDF_OS_DELAY_MSG_ENTER_CRITICAL(); 
    uint8_t latest_index = stdf_os_delay_msg_get_latest();  
    STDF_OS_DELAY_MSG_ASSERT(latest_index < STDF_OS_DELAY_MSG_MAX_NUM);
    handler = stdf_os_delay_msg_data[latest_index].handler;
    msg_id = stdf_os_delay_msg_data[latest_index].msg_id;
    payload = stdf_os_delay_msg_data[latest_index].payload;
    STDF_OS_DELAY_MSG_LOG("normal call handler %p msg_id %d payload %p", handler, msg_id, payload);
    stdf_os_delay_msg_set_latest(latest_index, false);
    stdf_os_delay_msg_deinit(latest_index);
    STDF_OS_DELAY_MSG_EXIT_CRITICAL();
    if(handler != NULL)
    {
        stdf_os_msg_mailbox_put(handler, msg_id, payload);
    }

    // check if other handle need to call
    uint8_t index;
    do 
    {
        STDF_OS_DELAY_MSG_ENTER_CRITICAL();
        uint32_t current_ms = GET_CURRENT_MS();
        for(index = 0; index < STDF_OS_DELAY_MSG_MAX_NUM; index++)
        {        
            if(stdf_os_delay_msg_data[index].used && 
               stdf_os_delay_msg_data[index].run_time <= current_ms)
            {
                handler = stdf_os_delay_msg_data[index].handler;
                msg_id = stdf_os_delay_msg_data[index].msg_id;
                payload = stdf_os_delay_msg_data[index].payload;
                STDF_OS_DELAY_MSG_LOG("fast call handler %p msg_id %d payload %p", handler, msg_id, payload);
                stdf_os_delay_msg_set_latest(index, false);
                stdf_os_delay_msg_deinit(index);           
            }
        }
        STDF_OS_DELAY_MSG_EXIT_CRITICAL();
        
        if(index < STDF_OS_DELAY_MSG_MAX_NUM)
        {            
            if(handler != NULL)
            {
                stdf_os_msg_mailbox_put(handler, msg_id, payload);
            }
        }
    }         
    while(index < STDF_OS_DELAY_MSG_MAX_NUM);
   
    // restart the timer
    STDF_OS_DELAY_MSG_ENTER_CRITICAL();
    if(!stdf_os_delay_msg_timer_is_run())
    {
        stdf_os_delay_msg_timer_start();
    }
    STDF_OS_DELAY_MSG_EXIT_CRITICAL();
}

/*******************************************************************************
 * @fn      .
 * @brief   .
 * @param   .
 * @return  .
 * @notice  .
 */
static void stdf_os_delay_msg_timer_handler(const void *param)
{
    stdf_os_delay_msg_timer_timeout(); 
}
osTimerDef (STDF_OS_DELAY_MSG_TIMER, stdf_os_delay_msg_timer_handler);

/*******************************************************************************
 * @fn      .
 * @brief   .
 * @param   .
 * @return  .
 * @notice  .
 */
static void stdf_os_delay_msg_add(stdf_os_handler_t handler, 
                                  stdf_os_msg_id_t msg_id, 
                                  void *payload, 
                                  uint32_t delay)
{
    uint8_t index;
    
    for(index = 0; index < STDF_OS_DELAY_MSG_MAX_NUM; index++)
    {
        if(stdf_os_delay_msg_data[index].used == false)
        {
            break;
        }
    }

    if(index < STDF_OS_DELAY_MSG_MAX_NUM)
    {
        STDF_OS_DELAY_MSG_LOG("success, index %d handler %p msg_id %d delay %u", 
                              index, handler, msg_id, delay);
        
        stdf_os_delay_msg_timer_stop();
        uint32_t current_ms = GET_CURRENT_MS();
        stdf_os_delay_msg_data[index].used     = true;      
        stdf_os_delay_msg_data[index].handler  = handler;
        stdf_os_delay_msg_data[index].msg_id   = msg_id;
        stdf_os_delay_msg_data[index].payload  = payload;
        stdf_os_delay_msg_data[index].run_time = current_ms + delay;
        stdf_os_delay_msg_timer_start();
    }
    else
    {
        STDF_OS_DELAY_MSG_LOG("failed, index %d handler %p msg_id %d delay %u", 
                              index, handler, msg_id, delay);
    }
}

/*******************************************************************************
 * @fn      .
 * @brief   .
 * @param   .
 * @return  .
 * @notice  .
 */
static bool stdf_os_delay_msg_delate_first(stdf_os_handler_t handler, 
                                           stdf_os_msg_id_t msg_id)
{
    uint8_t index;
    uint32_t min_run_time = STDF_OS_DELAY_MSG_FOREVER;
    uint8_t min_run_time_index = STDF_OS_DELAY_MSG_MAX_NUM;

    // find the latest of with the same handler and msg_id
    for(index = 0; index < STDF_OS_DELAY_MSG_MAX_NUM; index++)
    {
        if(stdf_os_delay_msg_data[index].used && 
           stdf_os_delay_msg_data[index].handler == handler &&
           stdf_os_delay_msg_data[index].msg_id == msg_id)
        {
            if(stdf_os_delay_msg_data[index].run_time < min_run_time)
            {
                min_run_time = stdf_os_delay_msg_data[index].run_time;
                min_run_time_index = index;
            }
        }
    } 

    // delate it and restart timer
    if(min_run_time_index < STDF_OS_DELAY_MSG_MAX_NUM)
    {
        STDF_OS_DELAY_MSG_LOG("success, index %d handler %p msg_id %d delay %u", 
                              min_run_time_index, handler, msg_id);
        
        if(stdf_os_delay_msg_get_latest() == min_run_time_index)
        {
            //STDF_OS_DELAY_MSG_LOG("delated is the latest");
            stdf_os_delay_msg_timer_stop();
            stdf_os_delay_msg_deinit(min_run_time_index);
            stdf_os_delay_msg_timer_start();
        }
        else
        {
            //STDF_OS_DELAY_MSG_LOG("delated is not the latest");
            stdf_os_delay_msg_deinit(min_run_time_index);
        }
        return true;
    }
    else
    {
        //STDF_OS_DELAY_MSG_LOG("failed not find");
        return false;
    }
}

/* -----------------------------------------------------------------------------
 *                                   APIs
 * ---------------------------------------------------------------------------*/

/*******************************************************************************
 * @fn      .
 * @brief   .
 * @param   .
 * @return  .
 * @notice  .
 */
void stdf_os_delay_msg_send(stdf_os_handler_t handler, 
                            stdf_os_msg_id_t msg_id, 
                            void *payload)
{
    STDF_OS_DELAY_MSG_ENTER_CRITICAL();
    stdf_os_delay_msg_add(handler, msg_id, payload, STDF_OS_DELAY_MSG_IMMEDIATELY);
    STDF_OS_DELAY_MSG_EXIT_CRITICAL();    
}

/*******************************************************************************
 * @fn      .
 * @brief   .
 * @param   .
 * @return  .
 * @notice  .
 */
void stdf_os_delay_msg_send_later(stdf_os_handler_t handler, 
                                  stdf_os_msg_id_t msg_id, 
                                  void *payload, 
                                  uint32_t delay)
{
    STDF_OS_DELAY_MSG_ENTER_CRITICAL();
    stdf_os_delay_msg_add(handler, msg_id, payload, delay);
    STDF_OS_DELAY_MSG_EXIT_CRITICAL(); 
}

/*******************************************************************************
 * @fn      .
 * @brief   .
 * @param   .
 * @return  .
 * @notice  .
 */
uint16_t stdf_os_delay_msg_get_count(stdf_os_handler_t handler, 
                                     stdf_os_msg_id_t msg_id)
{
    uint16_t count = 0; 

    STDF_OS_DELAY_MSG_ENTER_CRITICAL();
    for(uint8_t index = 0; index < STDF_OS_DELAY_MSG_MAX_NUM; index++)
    {
        if(stdf_os_delay_msg_data[index].used && 
           stdf_os_delay_msg_data[index].handler == handler && 
           stdf_os_delay_msg_data[index].msg_id == msg_id)
        {
            count++;
        }
    }
    STDF_OS_DELAY_MSG_EXIT_CRITICAL();    
    return count;
}

/*******************************************************************************
 * @fn      .
 * @brief   .
 * @param   .
 * @return  .
 * @notice  .
 */
bool stdf_os_delay_msg_cancel_first(stdf_os_handler_t handler, 
                                    stdf_os_msg_id_t msg_id)
{
    bool result;
    
    STDF_OS_DELAY_MSG_ENTER_CRITICAL();
    result = stdf_os_delay_msg_delate_first(handler, msg_id);
    STDF_OS_DELAY_MSG_EXIT_CRITICAL();

    return result;
}

/*******************************************************************************
 * @fn      .
 * @brief   .
 * @param   .
 * @return  .
 * @notice  .
 */
uint16_t stdf_os_delay_msg_cancel_all(stdf_os_handler_t handler, 
                                      stdf_os_msg_id_t msg_id)
{
    uint16_t count = 0;
    
    STDF_OS_DELAY_MSG_ENTER_CRITICAL();    
    while(stdf_os_delay_msg_delate_first(handler, msg_id))
    {
        count++;
    }
    STDF_OS_DELAY_MSG_EXIT_CRITICAL();
    
    return count;
}

/* -----------------------------------------------------------------------------
 *                                   Framwork
 * ---------------------------------------------------------------------------*/

/*******************************************************************************
 * @fn      .
 * @brief   .
 * @param   .
 * @return  .
 * @notice  .
 */
void stdf_os_delay_msg_init(void)
{
    //STDF_OS_DELAY_MSG_LOG("");

    // deinit all data
    for(uint8_t index = 0; index < STDF_OS_DELAY_MSG_MAX_NUM; index++) 
    {
        stdf_os_delay_msg_data[index].used     = false;
        stdf_os_delay_msg_data[index].latest   = false;
        stdf_os_delay_msg_data[index].handler  = NULL;
        stdf_os_delay_msg_data[index].msg_id   = STDF_OS_MSG_ID_INVALID;
        stdf_os_delay_msg_data[index].payload  = NULL;
        stdf_os_delay_msg_data[index].run_time = STDF_OS_DELAY_MSG_FOREVER;
    }

    // create timer
    stdf_os_delay_msg_timer = osTimerCreate(osTimer(STDF_OS_DELAY_MSG_TIMER), osTimerOnce, NULL);
    STDF_OS_DELAY_MSG_ASSERT(stdf_os_delay_msg_timer != NULL);

    // create mutex
    stdf_os_delay_msg_mutex_id = osMutexCreate((osMutex(STDF_OS_DELAY_MSG_MUTEX)));
    STDF_OS_DELAY_MSG_ASSERT(stdf_os_delay_msg_mutex_id != NULL);
}

/*******************************************************************************
*******************************************************************************/