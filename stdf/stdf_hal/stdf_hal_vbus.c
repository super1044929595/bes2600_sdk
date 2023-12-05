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
#include "stdf_hal_vbus.h"
#include "stdf_os.h"

#include <string.h>
#include "cmsis_os.h"
#include "cmsis_nvic.h"
#include "hal_uart.h"
#include "hal_iomux_best1305.h"
#include "hal_iomux.h"
#include "hal_gpio.h"
#include "hal_timer.h"

/*******************************************************************************
 * MACROS
 */
#define STDF_HAL_VBUS_LEN_MIN               (9)
#define STDF_HAL_VBUS_LEN_MAX               (120)
#define STDF_HAL_VBUS_RECEIVE_TIMEOUT_CFG   1
#define STDF_HAL_VBUS_TRANSMITTED_SIGNAL    (1<<0)
#define STDF_HAL_VBUS_RECV_BUF_LEN          (512)
#define STDF_HAL_VBUS_BAUT_RATE             (10000)

#define STDF_HAL_VBUS_LOG(str, ...)         STDF_LOG("[HAL][VBUS] %s "str, __func__, ##__VA_ARGS__)
#define STDF_HAL_VBUS_ASSERT(cond)          STDF_ASSERT(cond)

/*******************************************************************************
 * TYPEDEFS
 */
typedef enum {
  STDF_HAL_VBUS_MODE_INIT,
  STDF_HAL_VBUS_MODE_DEINIT,
  STDF_HAL_VBUS_MODE_RESET,
  STDF_HAL_VBUS_MODE_START,
  STDF_HAL_VBUS_MODE_STOP,
  STDF_HAL_VBUS_MODE_TX,
  STDF_HAL_VBUS_MODE_RX,
  STDF_HAL_VBUS_MODE_NUM,
} stdf_hal_vbus_mode_e;

/*******************************************************************************
* GLOBAL VARIABLES
*/
static const enum HAL_UART_ID_T stdf_hal_vbus_comm = HAL_UART_ID_1;
static bool stdf_hal_vbus_inited = false;
static bool stdf_hal_vbus_is_open = false;
static bool stdf_hal_vbus_rx_dma_is_running = false;

static uint8_t stdf_hal_vbus_recv_buf[STDF_HAL_VBUS_RECV_BUF_LEN];

static const struct HAL_UART_CFG_T stdf_hal_vbus_cfg = {
    HAL_UART_PARITY_NONE,
    HAL_UART_STOP_BITS_1,
    HAL_UART_DATA_BITS_8,
    HAL_UART_FLOW_CONTROL_NONE,
    HAL_UART_FIFO_LEVEL_1_2,
    HAL_UART_FIFO_LEVEL_1_2,
    STDF_HAL_VBUS_BAUT_RATE,
    true,
    true,
    false,
};

stdf_hal_vbus_handler_t stdf_hal_vbus_handler;
stdf_hal_vbus_rx_data_t stdf_hal_vbus_rx_data;

/*******************************************************************************
 * EXTERNAL VARIABLES
 */

/*******************************************************************************
 * FUNCTIONS
 */
static void stdf_hal_vbus_mode_switch(uint8_t vbus_mode);

/*******************************************************************************
 * @fn      .
 * @brief   .
 * @param   .
 * @return  .
 * @notice  .
 */
static void stdf_hal_vbus_rx_dma_handler(uint32_t xfer_size, int dma_error, union HAL_UART_IRQ_T status)
{
    STDF_HAL_VBUS_LOG("xfer_size %d dma_error %d status %d", xfer_size, dma_error, status);
    
    if (dma_error || status.FE|| status.PE || status.BE || status.OE )
    {
        stdf_hal_vbus_mode_switch(STDF_HAL_VBUS_MODE_RESET);
    }
    else
    {
        if (xfer_size < STDF_HAL_VBUS_LEN_MIN || xfer_size > STDF_HAL_VBUS_LEN_MAX )
        {
            stdf_hal_vbus_mode_switch(STDF_HAL_VBUS_MODE_RESET);
        }    
        else if(stdf_hal_vbus_handler)
        {
            stdf_hal_vbus_rx_data_t *data = &stdf_hal_vbus_rx_data;
            data->payload = stdf_hal_vbus_recv_buf;
            data->length = xfer_size;
            MessageSend(stdf_hal_vbus_handler, STDF_OS_GMSG_ID_VBUS_RECEIVE_DATA, data);
        }
        stdf_hal_vbus_mode_switch(STDF_HAL_VBUS_MODE_RX);
    }    
}

/*******************************************************************************
 * @fn      .
 * @brief   .
 * @param   .
 * @return  .
 * @notice  .
 */
static void stdf_hal_vbus_rx_dma_start(void)
{
    //STDF_HAL_VBUS_LOG("");
    
    union HAL_UART_IRQ_T mask;
    uint32_t lock = int_lock();
    
    hal_uart_flush(stdf_hal_vbus_comm, 0);
    mask.reg = 0;
    mask.RT = STDF_HAL_VBUS_RECEIVE_TIMEOUT_CFG;
    hal_uart_dma_recv_mask(stdf_hal_vbus_comm, stdf_hal_vbus_recv_buf, STDF_HAL_VBUS_RECV_BUF_LEN, NULL, NULL,&mask);
    stdf_hal_vbus_rx_dma_is_running = true;
    
    int_unlock(lock);
}

/*******************************************************************************
 * @fn      .
 * @brief   .
 * @param   .
 * @return  .
 * @notice  .
 */
static void stdf_hal_vbus_rx_dma_stop(void)
{
    //STDF_HAL_VBUS_LOG("");
    
    union HAL_UART_IRQ_T mask;
    uint32_t lock = int_lock();
    
    if (stdf_hal_vbus_rx_dma_is_running)
    {        
        mask.reg = 0;
        hal_uart_irq_set_mask(stdf_hal_vbus_comm, mask);
        hal_uart_stop_dma_recv(stdf_hal_vbus_comm);
        stdf_hal_vbus_rx_dma_is_running = false;
    }
    
    int_unlock(lock);
}

/*******************************************************************************
 * @fn      .
 * @brief   .
 * @param   .
 * @return  .
 * @notice  .
 */
static void stdf_hal_vbus_tx_dma_start(void)
{
    STDF_HAL_VBUS_LOG("");

    stdf_hal_vbus_rx_dma_stop();
    hal_iomux_single_wire_uart_tx(stdf_hal_vbus_comm);
}

/*******************************************************************************
 * @fn      .
 * @brief   .
 * @param   .
 * @return  .
 * @notice  .
 */
static void stdf_hal_vbus_tx_dma_handler(uint32_t xfer_size, int dma_error)
{
    //STDF_HAL_VBUS_LOG("");    
}

/*******************************************************************************
 * @fn      .
 * @brief   .
 * @param   .
 * @return  .
 * @notice  .
 */
static void stdf_hal_vbus_tx_handle(uint8_t *data,uint8_t len)
{
    //STDF_HAL_VBUS_LOG("");

    stdf_hal_vbus_mode_switch(STDF_HAL_VBUS_MODE_TX);
    
    hal_uart_dma_send(stdf_hal_vbus_comm, data, len, NULL, NULL);
    
    while (!hal_uart_get_flag(stdf_hal_vbus_comm).TXFE || hal_uart_get_flag(stdf_hal_vbus_comm).BUSY)
    {
        osThreadYield();
    }
    
    stdf_hal_vbus_rx_dma_start();
}

/*******************************************************************************
 * @fn      .
 * @brief   .
 * @param   .
 * @return  .
 * @notice  .
 */
static void stdf_hal_vbus_uart_init(void)
{
    struct HAL_UART_CFG_T comm_stdf_hal_vbus_cfg;
    
    //STDF_HAL_VBUS_LOG("");
    
    if (!stdf_hal_vbus_inited)
    {
        memcpy(&comm_stdf_hal_vbus_cfg, &stdf_hal_vbus_cfg, sizeof(comm_stdf_hal_vbus_cfg));
        hal_uart_open(stdf_hal_vbus_comm, &comm_stdf_hal_vbus_cfg);
        hal_uart_irq_set_dma_handler(stdf_hal_vbus_comm, stdf_hal_vbus_rx_dma_handler, stdf_hal_vbus_tx_dma_handler);
        stdf_hal_vbus_inited = true;
    }
    
    hal_uart_flush(stdf_hal_vbus_comm, 0);
}

/*******************************************************************************
 * @fn      .
 * @brief   .
 * @param   .
 * @return  .
 * @notice  .
 */
static void stdf_hal_vbus_deinit(void)
{
    if (stdf_hal_vbus_inited) 
    {
        stdf_hal_vbus_rx_dma_stop();
        hal_uart_close(stdf_hal_vbus_comm);
        stdf_hal_vbus_inited = false;
    }
}

/*******************************************************************************
 * @fn      .
 * @brief   .
 * @param   .
 * @return  .
 * @notice  .
 */
static void stdf_hal_vbus_reset(void)
{
    uint32_t lock;
    lock = int_lock();
    stdf_hal_vbus_rx_dma_stop();
    hal_iomux_single_wire_uart_rx(stdf_hal_vbus_comm);
    hal_uart_flush(stdf_hal_vbus_comm, 0);
    stdf_hal_vbus_rx_dma_start();
    int_unlock(lock);
}

/*******************************************************************************
 * @fn      .
 * @brief   .
 * @param   .
 * @return  .
 * @notice  .
 */
static void stdf_hal_vbus_mode_switch(uint8_t vbus_mode)
{
    //STDF_HAL_VBUS_LOG("vbus_mode %d", vbus_mode);

    switch(vbus_mode)
    {
        case STDF_HAL_VBUS_MODE_INIT:
            stdf_hal_vbus_init();
            break;

        case STDF_HAL_VBUS_MODE_DEINIT:
            stdf_hal_vbus_deinit();
            break;
        
        case STDF_HAL_VBUS_MODE_RESET:
            stdf_hal_vbus_reset();
            break;

        case STDF_HAL_VBUS_MODE_START:
            break;

        case STDF_HAL_VBUS_MODE_STOP:
            stdf_hal_vbus_rx_dma_stop();
            break;

        case STDF_HAL_VBUS_MODE_TX:
            stdf_hal_vbus_tx_dma_start();
            break;

        case STDF_HAL_VBUS_MODE_RX:
            stdf_hal_vbus_rx_dma_stop();
            stdf_hal_vbus_rx_dma_start();
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
void stdf_hal_vbus_enable(void)
{
    if(stdf_hal_vbus_is_open)
    {
        STDF_HAL_VBUS_LOG("already closed!!!");
        return;
    }
    
    stdf_hal_vbus_is_open = true;
    stdf_hal_vbus_mode_switch(STDF_HAL_VBUS_MODE_RX);
}

/*******************************************************************************
 * @fn      .
 * @brief   .
 * @param   .
 * @return  .
 * @notice  .
 */
void stdf_hal_vbus_disable(void)
{
    if(false == stdf_hal_vbus_is_open)
    {
        STDF_HAL_VBUS_LOG("already opend!!!");
        return;
    }

    stdf_hal_vbus_is_open = false;
    stdf_hal_vbus_mode_switch(STDF_HAL_VBUS_MODE_STOP);
}

/*******************************************************************************
 * @fn      .
 * @brief   .
 * @param   .
 * @return  .
 * @notice  .
 */
void stdf_hal_vbus_send(uint8_t *buf,uint16_t len)
{
    stdf_hal_vbus_tx_handle(buf, len);
}

/*******************************************************************************
 * @fn      .
 * @brief   .
 * @param   .
 * @return  .
 * @notice  .
 */
void stdf_hal_vbus_init(void)
{ 
    stdf_hal_vbus_uart_init();
    hal_iomux_single_wire_uart_rx(stdf_hal_vbus_comm);
    stdf_hal_vbus_enable();
}

/*******************************************************************************
 * @fn      .
 * @brief   .
 * @param   .
 * @return  .
 * @notice  .
 */
void stdf_hal_vbus_register_handler(stdf_hal_vbus_handler_t handler)
{
    stdf_hal_vbus_handler = handler;
}

/*******************************************************************************
*******************************************************************************/

