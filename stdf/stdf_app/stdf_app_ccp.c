/*******************************************************************************
  Filename:       .

  Version         V1.0.

  Author          Yuping.Mo.

  Description:    CCP(The Charging Case Protocal).


  IMPORTANT:      .

*******************************************************************************/

/*******************************************************************************
 * INCLUDES
 */
#include "stdf_app_ccp.h"
#include "stdf_app_phy.h"
#include "stdf_app_protocal.h"
#include "stdf_define.h" 
#include "stdf_sdk_api.h"

/*******************************************************************************
 * MACROS
 */
#define STDF_APP_CCP_LOG(str, ...)          STDF_LOG("[CCP] %s "str, __func__, ##__VA_ARGS__)
#define STDF_APP_CCP_ASSERT(cond)           STDF_ASSERT(cond)

/*******************************************************************************
 * TYPEDEFS
 */
typedef void (*stdf_app_ccp_callback_t)(uint8_t *data, uint16_t length);

typedef struct
{
    stdf_app_protocal_cmd_t cmd;
    stdf_app_ccp_callback_t callback;
} stdf_app_ccp_config_t;

/*******************************************************************************
* GLOBAL VARIABLES
*/

/*******************************************************************************
 * EXTERNAL VARIABLES
 */

/*******************************************************************************
 * FUNCTIONS
 */

/*******************************************************************************
 * @brief   .
 */
static void stdf_app_ccp_state(uint8_t *data, uint16_t length)
{
    // data[0] - 右耳是否在盒, 01:入盒 00:出盒
    // data[1] - 左耳是否在盒，01:入盒 00:出盒
    // data[2] - bit7: 1-充电 0-放电, 	bit0~bit6-盒子电量百分比
    // data[3] - 盒子是否关闭，01:开盖 00:关盖
    // data[4] - 盒子版本号

    // response[0] - 01:tws连接； 00:未连接
    // response[1] - 01:配对模式；00:未配对模式 
    // response[2] - 00:主耳； 01:从耳； FF:未知
    // response[3] - 蓝牙地址和
    // response[4] - 耳机电量百分比
    // response[5] - 手机已连接；00:未连接   
    uint8_t response[6] = {0};
    response[0] = stdf_sdk_api_is_tws_connected();
    response[1] = stdf_sdk_api_is_bt_pairing();
    response[2] = stdf_sdk_api_is_role_unkouwn() ? 0xFF : (stdf_sdk_api_is_role_master() ? 0x00 : 0x01);
    response[3] = 0xFF;
    response[4] = 100;
    response[5] = stdf_sdk_api_is_bt_connected();
    stdf_app_protocal_response(response, 6, STDF_APP_PROTOCAL_ACK_SUCCESS);
}

/*******************************************************************************
 * @brief   .
 */
static void stdf_app_ccp_case_open(uint8_t *data, uint16_t length)
{
    STDF_APP_CCP_LOG("");
    stdf_app_phy_event_case_open();
    stdf_app_protocal_response(NULL, 0, STDF_APP_PROTOCAL_ACK_SUCCESS);
}

/*******************************************************************************
 * @brief   .
 */
static void stdf_app_ccp_case_close(uint8_t *data, uint16_t length)
{
    STDF_APP_CCP_LOG("");
    stdf_app_phy_event_case_close();
    stdf_app_protocal_response(NULL, 0, STDF_APP_PROTOCAL_ACK_SUCCESS);
}

/*******************************************************************************
 * @brief   .
 */
static void stdf_app_ccp_1wire_download(uint8_t *data, uint16_t length)
{
    STDF_APP_CCP_LOG("");
    stdf_app_protocal_response(NULL, 0, STDF_APP_PROTOCAL_ACK_SUCCESS);
    stdf_sdk_api_sys_reset_enter_1wire_download();
}

/*******************************************************************************
 * @brief   .
 */
static void stdf_app_ccp_tws_pairing(uint8_t *data, uint16_t length)
{
    STDF_APP_CCP_LOG("");
    stdf_app_protocal_response(NULL, 0, STDF_APP_PROTOCAL_ACK_SUCCESS);
    
    // Wait the response sent complete.
    stdf_sdk_api_sys_delay_ms(20);
    stdf_sdk_api_reset_enter_tws_pairing();
}

/*******************************************************************************
 * @brief   .
 */
static void stdf_app_ccp_bt_pairing(uint8_t *data, uint16_t length)
{
    STDF_APP_CCP_LOG("");
    stdf_app_protocal_response(NULL, 0, STDF_APP_PROTOCAL_ACK_SUCCESS);
}

/*******************************************************************************
 * @brief   .
 */
static void stdf_app_ccp_freeman_pairing(uint8_t *data, uint16_t length)
{
    STDF_APP_CCP_LOG("");
    stdf_app_protocal_response(NULL, 0, STDF_APP_PROTOCAL_ACK_SUCCESS);
    
    // Wait the response sent complete.
    stdf_sdk_api_sys_delay_ms(20);
    stdf_sdk_api_reset_enter_freeman_pairing();
}

/*******************************************************************************
 * @brief   .
 */
static void stdf_app_ccp_reset_factory_settings(uint8_t *data, uint16_t length)
{
    STDF_APP_CCP_LOG("");
    stdf_app_protocal_response(NULL, 0, STDF_APP_PROTOCAL_ACK_SUCCESS);
    
    // Wait the response sent complete.
    stdf_sdk_api_sys_delay_ms(20);
    stdf_sdk_api_sys_reset();
}

/*******************************************************************************
 * @brief   .
 */
static void stdf_app_ccp_read_bt_addr(uint8_t *data, uint16_t length)
{
    uint8_t *response = stdf_sdk_api_read_bt_addr();
    STDF_APP_CCP_LOG("");    
    stdf_sdk_api_enter_tws_pairing();
    stdf_app_protocal_response(response, 6, STDF_APP_PROTOCAL_ACK_SUCCESS);
}

/*******************************************************************************
 * @brief   .
 */
static void stdf_app_ccp_write_tws_addr(uint8_t *data, uint16_t length)
{
    STDF_APP_CCP_LOG("");
    stdf_app_protocal_response(NULL, 0, STDF_APP_PROTOCAL_ACK_SUCCESS);
}

/*******************************************************************************
 * @brief   .
 */
static void stdf_app_ccp_power_off(uint8_t *data, uint16_t length)
{
    STDF_APP_CCP_LOG("");
    stdf_app_protocal_response(NULL, 0, STDF_APP_PROTOCAL_ACK_SUCCESS);
    
    // Wait the response sent complete.
    stdf_sdk_api_sys_delay_ms(20);
    stdf_sdk_api_sys_power_off();
}

/*******************************************************************************
 * @fn      .
 * @brief   .
 * @param   .
 * @return  .
 * @notice  .
 */
stdf_app_ccp_config_t stdf_app_ccp_config_table[] = 
{
    {STDF_APP_PROTOCAL_CMD_STATE,                  stdf_app_ccp_state},
    {STDF_APP_PROTOCAL_CMD_CASE_OPEN,              stdf_app_ccp_case_open},
    {STDF_APP_PROTOCAL_CMD_CASE_CLOSE,             stdf_app_ccp_case_close},
    {STDF_APP_PROTOCAL_CMD_1WIRE_DOWNLOAD,         stdf_app_ccp_1wire_download},
    {STDF_APP_PROTOCAL_CMD_TWS_PAIRING,            stdf_app_ccp_tws_pairing},
    {STDF_APP_PROTOCAL_CMD_BT_PAIRING,             stdf_app_ccp_bt_pairing},
    {STDF_APP_PROTOCAL_CMD_FREEMAN_PAIRING,        stdf_app_ccp_freeman_pairing},
    {STDF_APP_PROTOCAL_CMD_RESET_FACTORY_SETTINGS, stdf_app_ccp_reset_factory_settings},
    {STDF_APP_PROTOCAL_CMD_READ_BT_ADDR,           stdf_app_ccp_read_bt_addr},
    {STDF_APP_PROTOCAL_CMD_WRITE_TWS_BDADDR,       stdf_app_ccp_write_tws_addr},
    {STDF_APP_PROTOCAL_CMD_POWER_OFF,              stdf_app_ccp_power_off},
};

/*******************************************************************************
 * @fn      .
 * @brief   .
 * @param   .
 * @return  .
 * @notice  .
 */
static void stdf_app_ccp_protocal_callback(stdf_app_protocal_cmd_t cmd, uint8_t *data, uint16_t length)
{
    uint16_t table_length = sizeof(stdf_app_ccp_config_table) / sizeof(stdf_app_ccp_config_t);

    for(uint8_t index = 0; index <  table_length; index++)
    {
        if(cmd == stdf_app_ccp_config_table[index].cmd)
        {
            stdf_app_ccp_config_table[index].callback(data, length);
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
void stdf_app_ccp_init(void)
{
    stdf_app_protocal_register_callback(stdf_app_ccp_protocal_callback);
}

/*******************************************************************************
*******************************************************************************/


