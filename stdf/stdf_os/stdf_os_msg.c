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

#include "stdf_define.h"
#include "stdf_os.h"
#include "stdf_os_config.h"


/*******************************************************************************
 * MACROS
 */
#define STDF_OS_MSG_LOG(str, ...)           STDF_LOG("[OS][MSG] %s "str, __func__, ##__VA_ARGS__)
#define STDF_OS_MSG_ASSERT(cond)            STDF_ASSERT(cond)

#define STDF_OS_MSG_THREAD_STACK_SIZE      (1024 * 4)
#define STDF_OS_MSG_MAILBOX_MAX            (30)

/*******************************************************************************
 * TYPEDEFS
 */
//
typedef struct
{
    stdf_os_handler_t  handler;             // the handler to recieved the message
    stdf_os_msg_id_t   msg_id;              //
    void               *payload;            //
} stdf_os_msg_mailbox_t;

/*******************************************************************************
* GLOBAL VARIABLES
*/
//
static void stdf_os_msg_thread(void const *argument);

//
osMailQDef (stdf_os_msg_mailbox, STDF_OS_MSG_MAILBOX_MAX, stdf_os_msg_mailbox_t);
static osMailQId stdf_os_msg_mailbox = NULL;
uint8_t stdf_os_msg_mailbox_cnt;

//
osThreadDef(stdf_os_msg_thread, osPriorityAboveNormal, 1, STDF_OS_MSG_THREAD_STACK_SIZE, "stdf_os_msg_thread");
osThreadId stdf_os_msg_thread_tid;

/*******************************************************************************
 * EXTERNAL VARIABLES
 */

/*******************************************************************************
 * FUNCTIONS
 */


/*******************************************************************************
 * @fn      .
 * @brief   .
 * @param   .
 * @return  .
 * @notice  .
 */
static void stdf_os_msg_mailbox_init(void)
{
    stdf_os_msg_mailbox = osMailCreate(osMailQ(stdf_os_msg_mailbox), NULL);
    STDF_OS_MSG_ASSERT(stdf_os_msg_mailbox != NULL);
}

/*******************************************************************************
 * @fn      .
 * @brief   .
 * @param   .
 * @return  .
 * @notice  .
 */
int stdf_os_msg_mailbox_put(stdf_os_handler_t handler, stdf_os_msg_id_t msg_id, void *payload)
{
    osStatus status;
    stdf_os_msg_mailbox_t *msg_p = (stdf_os_msg_mailbox_t*)osMailAlloc(stdf_os_msg_mailbox, 0);

    if (!msg_p)
    {
        STDF_OS_MSG_LOG("osMailAlloc error dump start");
        for (uint8_t retry = 0; retry < STDF_OS_MSG_MAILBOX_MAX; retry++)
        {
            osEvent evt = osMailGet(stdf_os_msg_mailbox, 0);
            if (evt.status == osEventMail) 
            {
                STDF_OS_MSG_LOG("retry %d handler %p msg_id %d payload %p", 
                                retry,
                                ((stdf_os_msg_mailbox_t *)(evt.value.p))->handler,
                                ((stdf_os_msg_mailbox_t *)(evt.value.p))->msg_id,
                                ((stdf_os_msg_mailbox_t *)(evt.value.p))->payload);
            }
            else
            {                
                STDF_OS_MSG_LOG("retry %d status %d", retry, evt.status); 
                break;
            }
        }
        STDF_OS_MSG_LOG("osMailAlloc error dump end");
    }
    
    STDF_OS_MSG_ASSERT(msg_p != NULL);
    msg_p->handler = handler;
    msg_p->msg_id  = msg_id;
    msg_p->payload = payload;

    status = osMailPut(stdf_os_msg_mailbox, msg_p);
    if (osOK == status)
    {
        stdf_os_msg_mailbox_cnt++;
    }
    return (int)status;
}

/*******************************************************************************
 * @fn      .
 * @brief   .
 * @param   .
 * @return  .
 * @notice  .
 */
static int stdf_os_msg_mailbox_free(stdf_os_msg_mailbox_t* msg_p)
{
    osStatus status;

    status = osMailFree(stdf_os_msg_mailbox, msg_p);
    if (osOK == status)
    {
        stdf_os_msg_mailbox_cnt--;
    }
    return (int)status;
}

/*******************************************************************************
 * @fn      .
 * @brief   .
 * @param   .
 * @return  .
 * @notice  .
 */
static int stdf_os_msg_mailbox_get(stdf_os_msg_mailbox_t** msg_p)
{
    osEvent evt;
    evt = osMailGet(stdf_os_msg_mailbox, osWaitForever);
    if (evt.status == osEventMail)
    {
        *msg_p = (stdf_os_msg_mailbox_t *)evt.value.p;
        return 0;
    }
    return -1;
}

/*******************************************************************************
 * @fn      .
 * @brief   .
 * @param   .
 * @return  .
 * @notice  .
 */
static void stdf_os_msg_thread_init(void)
{
    stdf_os_msg_thread_tid = osThreadCreate(osThread(stdf_os_msg_thread), NULL);
    STDF_OS_MSG_ASSERT(stdf_os_msg_thread_tid != NULL);
}

/*******************************************************************************
 * @fn      .
 * @brief   .
 * @param   .
 * @return  .
 * @notice  .
 */
static void stdf_os_msg_thread(void const *argument)
{
    stdf_os_msg_mailbox_t *msg = NULL;
    
    while(1)
    {
        if (!stdf_os_msg_mailbox_get(&msg))
        {
            if (msg->handler != NULL) 
            {
                msg->handler(msg->msg_id, msg->payload);
            }
            stdf_os_msg_mailbox_free(msg);
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
void stdf_os_msg_init(void)
{
    //STDF_OS_MSG_LOG("");
    
    stdf_os_msg_mailbox_init();
    
    stdf_os_msg_thread_init();
} 

/*******************************************************************************
*******************************************************************************/
