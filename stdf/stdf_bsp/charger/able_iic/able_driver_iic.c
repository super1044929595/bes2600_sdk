#include "stdio.h"
#include "string.h"

#include "able_driver_iic.h"
#include "twi_sw_master.h"

int able_driver_iic_init(void)
{
#if defined (HW_IIC_USED)
	twi_hw_master_init();
#else
	twi_sw_master_init();
#endif
	return 0;
}

bool able_iic_master_stop(void)
{
#if defined (HW_IIC_USED)
	return true;
#else
    return twi_sw_master_issue_stopcondition();
#endif
}

bool able_iic_master_write(uint8_t address,uint8_t reg_addr, uint8_t data)
{
	uint8_t I2c_Write_Buffer[2] = {0};
	bool bret = false;

  	I2c_Write_Buffer[0] = reg_addr;
  	I2c_Write_Buffer[1] = data;

#if defined (HW_IIC_USED)
    bret = twi_hw_master_write(address, I2c_Write_Buffer, 1, 1);
#else
    bret = twi_sw_master_transfer(address << 1, I2c_Write_Buffer, 2, TWI_ISSUE_STOP);
#endif
//       TWI_DEBUG_DEBUG("master write is %d", bret);
	return bret;
}

bool able_iic_master_read(uint8_t address,uint8_t reg_addr, uint8_t *data)
{
	bool bret = false;
//	TWI_DEBUG_DEBUG("%s",__func__);

#if defined (HW_IIC_USED)
   	bret = twi_hw_master_read(address,reg_addr, data, 1);
#else
	bret = twi_sw_master_transfer(address << 1 , &reg_addr, 1, TWI_DONT_ISSUE_STOP);

    if (bret){
        bret = twi_sw_master_transfer((address << 1) + 1, data, 1, TWI_ISSUE_STOP);
    }  else {
    	able_iic_master_stop();
    }
#endif

//    TWI_DEBUG_DEBUG("master read is %d", bret);
    return bret;
}

bool able_iic_master_write_block(uint8_t address,uint8_t reg_addr, uint8_t *data_buffer, uint8_t len)
{	
    uint8_t I2c_Write_Buffer[256] = {0};
	bool bret = false;

	if(len > 255) {
		return false;
	}

  	I2c_Write_Buffer[0] = reg_addr;
  	memcpy(I2c_Write_Buffer + 1,data_buffer,len);

#if defined (HW_IIC_USED)
    bret = twi_hw_master_write(address, I2c_Write_Buffer, 1, len);
#else
    bret = twi_sw_master_transfer(address << 1, I2c_Write_Buffer, 1 + len, TWI_ISSUE_STOP);
#endif

	return bret;
}

bool able_iic_master_write_block2(uint8_t address,uint8_t *data_buffer, uint8_t len)
{	
	bool bret = false;
#if defined (HW_IIC_USED)
    bret = twi_hw_master_write(address, data_buffer, 1, len-1);
#else
    bret = twi_sw_master_transfer(address << 1, data_buffer, len, TWI_ISSUE_STOP);
#endif

	return bret;
}


bool able_iic_master_read_block(uint8_t address,uint8_t reg_addr, uint8_t *data, uint16_t rx_size)
{
	bool bret = false;

#if defined (HW_IIC_USED)
	bret = twi_hw_master_read(address,reg_addr, data, rx_size);
#else
    bret = twi_sw_master_transfer(address << 1, &reg_addr, 1, TWI_DONT_ISSUE_STOP);

    if (bret) {
        bret = twi_sw_master_transfer((address << 1) + 1, data, rx_size, TWI_ISSUE_STOP);
    } else {
		   able_iic_master_stop();
	}
#endif

	return bret;
}

bool able_iic_master_write_and_read(uint8_t address,uint8_t* tx_data, uint16_t tx_len, uint8_t *rx_data, uint16_t rx_len)
{
	bool bret = false;
#if defined (HW_IIC_USED)
    //Todo 

#else
    bret = twi_sw_master_transfer(address << 1, tx_data, tx_len, TWI_DONT_ISSUE_STOP);
    if (bret) {
        bret = twi_sw_master_transfer((address << 1) + 1, rx_data, rx_len, TWI_ISSUE_STOP);
    } else {
    	able_iic_master_stop();
    }
#endif
	return bret;
}

