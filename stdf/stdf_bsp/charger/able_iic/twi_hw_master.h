#ifndef __TWI_HW_MASTER_H
#define __TWI_HW_MASTER_H

#include "hal_gpio.h"
#include "hal_i2c.h"
#include "stdint.h"
#include "stdbool.h"

#ifdef __cplusplus
extern "C" {
#endif

#if defined (HW_IIC_USED)

#ifndef HW_IIC_SPEED
#define HW_IIC_SPEED						400000						//400kbps
#endif

void twi_hw_master_init(void);
bool twi_hw_master_write(uint8_t address, uint8_t *data, uint32_t reg_len, uint32_t value_len);
bool twi_hw_master_read(uint8_t address,uint8_t reg_addr, uint8_t *data, uint16_t rx_size);
#endif

#ifdef __cplusplus
}
#endif

#endif

