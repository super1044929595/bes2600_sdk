#ifndef __TWI_SW_MASTER_H
#define __TWI_SW_MASTER_H

#include "stdint.h"
#include "stdbool.h"

#include "able_driver_iic.h"


#ifdef __cplusplus
extern "C" {
#endif

#define TWI_READ_BIT                (0x01)        //!< If this bit is set in the address field, transfer direction is from slave to master.

#define TWI_ISSUE_STOP              ((bool)true)  //!< Parameter for @ref twi_master_transfer
#define TWI_DONT_ISSUE_STOP         ((bool)false) //!< Parameter for @ref twi_master_transfer

#define TWI_SW_SCL_HIGH()           do { hal_gpio_pin_set((enum HAL_GPIO_PIN_T)SW_SCL_PIN); } while(0)    /*!< Pulls SCL line high */
#define TWI_SW_SCL_LOW()            do { hal_gpio_pin_clr((enum HAL_GPIO_PIN_T)SW_SCL_PIN); } while(0)    /*!< Pulls SCL line low  */
#define TWI_SW_SDA_HIGH()           do { hal_gpio_pin_set((enum HAL_GPIO_PIN_T)SW_SDA_PIN); } while(0)    /*!< Pulls SDA line high */
#define TWI_SW_SDA_LOW()            do { hal_gpio_pin_clr((enum HAL_GPIO_PIN_T)SW_SDA_PIN); } while(0)    /*!< Pulls SDA line low  */
#define TWI_SW_SDA_INPUT()          do { hal_gpio_pin_set_dir((enum HAL_GPIO_PIN_T)SW_SDA_PIN, HAL_GPIO_DIR_IN, 1);  } while(0)   /*!< Configures SDA pin as input  */
#define TWI_SW_SCL_INPUT()          do { hal_gpio_pin_set_dir((enum HAL_GPIO_PIN_T)SW_SCL_PIN, HAL_GPIO_DIR_IN, 1);  } while(0)
#define TWI_SW_SDA_OUTPUT()         do { hal_gpio_pin_set_dir((enum HAL_GPIO_PIN_T)SW_SDA_PIN, HAL_GPIO_DIR_OUT,0);  } while(0)   /*!< Configures SDA pin as output */
#define TWI_SW_SCL_OUTPUT()         do { hal_gpio_pin_set_dir((enum HAL_GPIO_PIN_T)SW_SCL_PIN, HAL_GPIO_DIR_OUT,0);  } while(0)   /*!< Configures SCL pin as output */

#define TWI_SW_SDA_READ()           (hal_gpio_pin_get_val((enum HAL_GPIO_PIN_T)SW_SDA_PIN))               /*!< Reads current state of SDA */
#define TWI_SW_SCL_READ()           (hal_gpio_pin_get_val((enum HAL_GPIO_PIN_T)SW_SCL_PIN))               /*!< Reads current state of SCL */


#define TWI_RAM_SHORT_DELAYN() 		__asm("NOP;NOP;NOP;NOP;NOP;NOP;NOP;NOP;NOP;NOP;NOP;NOP;NOP;NOP;NOP;NOP;NOP");\
    __asm("NOP;NOP;NOP;NOP;NOP;NOP;NOP;NOP;NOP;NOP;NOP;NOP;NOP;NOP;NOP;NOP;NOP");\
    __asm("NOP;NOP;NOP;NOP;NOP;NOP;NOP;NOP;NOP;NOP;NOP;NOP;NOP;NOP;NOP;NOP;NOP");\
    __asm("NOP;NOP;NOP;NOP;NOP;NOP;NOP;NOP;NOP;NOP;NOP;NOP;NOP;NOP;NOP;NOP;NOP");\
    __asm("NOP;NOP;NOP;NOP;NOP;NOP;NOP;NOP;NOP;NOP;NOP;NOP;NOP;NOP;NOP;NOP;NOP");\
	__asm("NOP;NOP;NOP;NOP;NOP;NOP;NOP;NOP;NOP;NOP;NOP;NOP;NOP;NOP;NOP;NOP;NOP");

#define TWI_RAM_DELAYN()      		__asm("NOP;NOP;NOP;NOP;NOP;NOP;NOP;NOP;NOP;NOP;NOP;NOP;NOP;NOP;NOP;NOP;NOP");\
    __asm("NOP;NOP;NOP;NOP;NOP;NOP;NOP;NOP;NOP;NOP;NOP;NOP;NOP;NOP;NOP;NOP;NOP");\
    __asm("NOP;NOP;NOP;NOP;NOP;NOP;NOP;NOP;NOP;NOP;NOP;NOP;NOP;NOP;NOP;NOP;NOP");\
    __asm("NOP;NOP;NOP;NOP;NOP;NOP;NOP;NOP;NOP;NOP;NOP;NOP;NOP;NOP;NOP;NOP;NOP");\
    __asm("NOP;NOP;NOP;NOP;NOP;NOP;NOP;NOP;NOP;NOP;NOP;NOP;NOP;NOP;NOP;NOP;NOP");\
    __asm("NOP;NOP;NOP;NOP;NOP;NOP;NOP;NOP;NOP;NOP;NOP;NOP;NOP;NOP;NOP;NOP;NOP");


#define TWI_SW_DELAYN_SHORT()		TWI_RAM_SHORT_DELAYN()
#define TWI_SW_DELAYN()             TWI_RAM_DELAYN()

bool twi_sw_master_init(void);
bool twi_sw_master_transfer(uint8_t address, uint8_t *data, uint16_t data_length, bool issue_stop_condition);
void twi_sw_master_stop(void);
void twi_sw_master_start(void);
bool twi_sw_master_issue_stopcondition(void);

#ifdef __cplusplus
}
#endif

/*lint --flb "Leave library region" */
#endif

