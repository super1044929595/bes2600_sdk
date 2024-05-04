#include "tgt_hardware.h"
#include "twi_hw_master.h"
#include "hal_gpio.h"
#include "hal_timer.h"
//#include "debug_cfg.h"

#if(TWI_DEBUG_ENABLE)
#define TWI_TRACE_CRITICAL(s,...)           
#define TWI_ERROR(s,...)                    
#define TWI_TRACE(s,...)                    
#define TWI_DEBUG(s,...)                    
#else
#define TWI_TRACE_CRITICAL(s,...)           
#define TWI_ERROR(s,...)                    
#define TWI_TRACE(s,...)
#define TWI_DEBUG(s,...)
#endif

#if defined (HW_IIC_USED)

void twi_hw_master_init(void)
{
    struct HAL_I2C_CONFIG_T hal_i2c_cfg;

    hal_iomux_set_i2c_num_select((uint8_t)HAL_I2C_ID_NUM_SELECTED);

    hal_i2c_cfg.mode = HAL_I2C_API_MODE_SELECT;
    hal_i2c_cfg.use_dma  = 0;
    hal_i2c_cfg.use_sync = 1;
    hal_i2c_cfg.speed = HW_IIC_SPEED;
    hal_i2c_cfg.as_master = 1;
    hal_i2c_open(HAL_I2C_ID_NUM_SELECTED, &hal_i2c_cfg);
}

bool twi_hw_master_write(uint8_t address, uint8_t *data, uint32_t reg_len, uint32_t value_len)
{
    bool ret = true;
 
    ret = !hal_i2c_send(HAL_I2C_ID_NUM_SELECTED, address, (uint8_t *)data, reg_len, value_len, 0,NULL);

    return ret;
}

bool twi_hw_master_read(uint8_t address,uint8_t reg_addr, uint8_t *data, uint16_t rx_size)
{
    bool ret = true;

    ret = !hal_i2c_send(HAL_I2C_ID_NUM_SELECTED, address, (uint8_t *)&reg_addr, 1, 0, 0,NULL);

    if (ret == false)
    {
        return false;
    }

       return !hal_i2c_recv(HAL_I2C_ID_NUM_SELECTED, address, (uint8_t *)data, 0, rx_size, HAL_I2C_STOP_AFTER_WRITE, 0,NULL);
}

#endif
