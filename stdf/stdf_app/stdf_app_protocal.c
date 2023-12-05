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
#include "stdf_app.h"
#include "stdf_app_protocal.h" 
#include "stdf_define.h" 
#include "stdf_hal_vbus.h"
#include "stdf_os.h"
#include "stdf_os_config.h"
#include "stdf_sdk_api.h"

#include <string.h>

/*******************************************************************************
 * MACROS
 */
// Maximum data size for response data
#define STDF_APP_PROTOCAL_MAX_BUF_SIZE      (64)
#define STDF_APP_PROTOCAL_MIN_BUF_SIZE      (10)
#define STDF_APP_PROTOCAL_MAX_BUF_DATA_SIZE (64 - 10)
#define STDF_APP_PROTOCAL_MIN_BUF_DATA_SIZE (0)

// Data Format for protocal
#define STDF_APP_PROTOCAL_FRAME_HEAD0       0xAB
#define STDF_APP_PROTOCAL_FRAME_HEAD1       0x1E
#define STDF_APP_PROTOCAL_FRAME_HEAD2       0xAB
#define STDF_APP_PROTOCAL_FRAME_HEAD3_LR    0x01 // data for left or right earbuds
#define STDF_APP_PROTOCAL_FRAME_HEAD3_L     0x02 // data for left earbuds only
#define STDF_APP_PROTOCAL_FRAME_HEAD3_R     0x03 // data for right earbuds only
// command 2 bytes
// data length 2 bytes
// data 0 ~ n bytes
// fcs 2 bytes, reserved, default is 0xCCCC
#define STDF_APP_PROTOCAL_FRAME_FCS_DEFAULT  0xCC


#define STDF_APP_PROTOCAL_LOG(str, ...)     STDF_LOG("[APP][PROT] %s "str, __func__, ##__VA_ARGS__)
#define STDF_APP_PROTOCAL_ASSERT(cond)      STDF_ASSERT(cond)

/*******************************************************************************
 * TYPEDEFS
 */

/*******************************************************************************
* GLOBAL VARIABLES
*/
static uint8_t stdf_app_protocal_response_buf[STDF_APP_PROTOCAL_MAX_BUF_SIZE];

// saved frame last received data, and response data will use it automatically
static uint8_t stdf_app_protocal_frame_head3 = STDF_APP_PROTOCAL_FRAME_HEAD3_LR;
static uint8_t stdf_app_protocal_frame_cmd = STDF_APP_PROTOCAL_CMD_INVALID;

static stdf_app_protocal_callback_t stdf_app_protocal_callback;

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
static void stdf_app_protocal_receive_data_handler(uint8_t *buf, uint16_t length)
{
    stdf_sdk_api_earside_t earside = stdf_sdk_api_get_earside();
    stdf_app_protocal_cmd_t cmd = STDF_APP_PROTOCAL_CMD_INVALID;
    uint16_t data_length = length - STDF_APP_PROTOCAL_MIN_BUF_SIZE;
    
    STDF_APP_PROTOCAL_LOG("<= length %d", length);
    STDF_DUMP8("%02X ", buf, length);

    if(length < STDF_APP_PROTOCAL_MIN_BUF_SIZE || length > STDF_APP_PROTOCAL_MAX_BUF_SIZE)
    {
        STDF_APP_PROTOCAL_LOG("frame length error");
        return;
    }  
    else if(buf[0] != STDF_APP_PROTOCAL_FRAME_HEAD0 ||
            buf[1] != STDF_APP_PROTOCAL_FRAME_HEAD1 ||
            buf[2] != STDF_APP_PROTOCAL_FRAME_HEAD2)
    {
        STDF_APP_PROTOCAL_LOG("head error");
        return;
    }
    // The left earside can process left side command only, and the same as right earside.
    // The unkown earside can process all kind of commands depend on the application layer.
    else if((earside == STDF_SDK_API_EAR_SIDE_LEFT  && buf[3] == STDF_APP_PROTOCAL_FRAME_HEAD3_R) ||
            (earside == STDF_SDK_API_EAR_SIDE_RIGHT && buf[3] == STDF_APP_PROTOCAL_FRAME_HEAD3_L) ||
            (buf[3] < STDF_APP_PROTOCAL_FRAME_HEAD3_LR) || 
            (buf[3] > STDF_APP_PROTOCAL_FRAME_HEAD3_R))
    {
        STDF_APP_PROTOCAL_LOG("not sent to me");
        return;
    }
    else if((buf[6] << 8) + buf[7] != data_length)
    {
        STDF_APP_PROTOCAL_LOG("data length error");
        return;
    }
    else if(buf[length - 2] != STDF_APP_PROTOCAL_FRAME_FCS_DEFAULT && 
            buf[length - 1] != STDF_APP_PROTOCAL_FRAME_FCS_DEFAULT)
    {
        STDF_APP_PROTOCAL_LOG("fcs error");
        return;
    }

    // cmd
    cmd = (buf[4] << 8) + buf[5];
    stdf_app_protocal_frame_head3 = buf[3];
    stdf_app_protocal_frame_cmd = cmd;


    uint8_t *data = (data_length == 0) ? NULL : &buf[8];
    if(stdf_app_protocal_callback)
    {
        stdf_app_protocal_callback(cmd, data, data_length);
    } 
}

/*******************************************************************************
 * @fn      .
 * @brief   .
 * @param   .
 * @return  .
 * @notice  .
 */
static void stdf_app_protocal_msg_handler(stdf_os_msg_id_t msg_id, void *payload)
{
    stdf_hal_vbus_rx_data_t *data = (stdf_hal_vbus_rx_data_t *)payload;
    
    switch(msg_id)
    {
        case STDF_OS_GMSG_ID_VBUS_RECEIVE_DATA:
            stdf_app_protocal_receive_data_handler(data->payload, data->length);
            break;
            
        default:
            STDF_APP_PROTOCAL_ASSERT(false);
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
void stdf_app_protocal_response(uint8_t *data, uint16_t length, stdf_app_protocal_ack_t ack)
{
    uint8_t *response_buf = &stdf_app_protocal_response_buf[0];

    STDF_APP_PROTOCAL_ASSERT(stdf_app_protocal_frame_cmd != STDF_APP_PROTOCAL_CMD_INVALID);
    STDF_APP_PROTOCAL_ASSERT(length < STDF_APP_PROTOCAL_MAX_BUF_DATA_SIZE);
    
    *response_buf++ = STDF_APP_PROTOCAL_FRAME_HEAD0;
    *response_buf++ = STDF_APP_PROTOCAL_FRAME_HEAD1;
    *response_buf++ = STDF_APP_PROTOCAL_FRAME_HEAD2; 
    *response_buf++ = stdf_app_protocal_frame_head3;
    *response_buf++ = (stdf_app_protocal_frame_cmd >> 8) & 0xFF;
    *response_buf++ = stdf_app_protocal_frame_cmd & 0xFF;
    *response_buf++ = ack;
    *response_buf++ = (length >> 8) & 0xFF;
    *response_buf++ = length & 0xFF;
    memcpy(response_buf, data, length);
    response_buf += length;
    *response_buf++ = STDF_APP_PROTOCAL_FRAME_FCS_DEFAULT;
    *response_buf   = STDF_APP_PROTOCAL_FRAME_FCS_DEFAULT;

    length += STDF_APP_PROTOCAL_MIN_BUF_SIZE + 1; // + ack 1 bytes
    STDF_APP_PROTOCAL_LOG("=> length %d", length);
    STDF_DUMP8("%02X ", stdf_app_protocal_response_buf, length);
    stdf_hal_vbus_send(stdf_app_protocal_response_buf, length);
}

/*******************************************************************************
 * @fn      .
 * @brief   .
 * @param   .
 * @return  .
 * @notice  .
 */
void stdf_app_protocal_init(void)
{
    stdf_hal_vbus_register_handler(stdf_app_protocal_msg_handler);
}

/*******************************************************************************
 * @fn      .
 * @brief   .
 * @param   .
 * @return  .
 * @notice  .
 */
void stdf_app_protocal_register_callback(stdf_app_protocal_callback_t callback)
{
    stdf_app_protocal_callback = callback;
}

/*******************************************************************************
*******************************************************************************/


