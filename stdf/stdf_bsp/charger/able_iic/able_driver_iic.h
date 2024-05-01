#ifndef __ABLE_DRIVER_IIC_H__
#define __ABLE_DRIVER_IIC_H__

#include "stddef.h"
#include "stdint.h"
#include "stdbool.h"

bool able_iic_master_write(uint8_t address,uint8_t reg_addr, uint8_t data);
bool able_iic_master_read(uint8_t address,uint8_t reg_addr, uint8_t *data);
bool able_iic_master_write_block(uint8_t address,uint8_t reg_addr, uint8_t *data_buffer, uint8_t len);
bool able_iic_master_write_block2(uint8_t address,uint8_t *data_buffer, uint8_t len);
bool able_iic_master_read_block(uint8_t address,uint8_t reg_addr, uint8_t *data, uint16_t rx_size);
bool able_iic_master_write_and_read(uint8_t address,uint8_t* tx_data, uint16_t tx_len, uint8_t *rx_data, uint16_t rx_len);

bool able_iic_master_stop(void);

int able_driver_iic_init(void);

#endif