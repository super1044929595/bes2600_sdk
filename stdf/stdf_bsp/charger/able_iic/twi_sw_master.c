/* Copyright (c) 2009 Nordic Semiconductor. All Rights Reserved.
 *
 * The information contained herein is property of Nordic Semiconductor ASA.
 * Terms and conditions of usage are described in detail in NORDIC
 * SEMICONDUCTOR STANDARD SOFTWARE LICENSE AGREEMENT.
 *
 * Licensees are granted free, non-transferable use of the information. NO
 * WARRANTY of ANY KIND is provided. This heading must NOT be removed from
 * the file.
 *
 */

#include "twi_sw_master.h"
#include "hal_gpio.h"
#include "hal_timer.h"
//#include "debug_cfg.h"

//#include "tgt_hardware_customer.h"

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


#define RETRY_COUNTER	50

#define SW_SCL_PIN      HAL_GPIO_PIN_P1_2
#define SW_SDA_PIN      HAL_GPIO_PIN_P1_3

static const struct HAL_IOMUX_PIN_FUNCTION_MAP pinmux_iic[] =
{
    {HAL_GPIO_PIN_P1_3, HAL_IOMUX_FUNC_AS_GPIO, HAL_IOMUX_PIN_VOLTAGE_VIO, HAL_IOMUX_PIN_PULLUP_ENABLE},
    {HAL_GPIO_PIN_P1_2, HAL_IOMUX_FUNC_AS_GPIO, HAL_IOMUX_PIN_VOLTAGE_VIO, HAL_IOMUX_PIN_PULLUP_ENABLE},
};

static bool twi_sw_master_clear_bus(void);
static bool twi_sw_master_issue_startcondition(void);
static bool twi_sw_master_clock_byte(uint_fast8_t databyte);
static bool twi_sw_master_clock_byte_in(uint8_t *databyte, bool ack);
static bool twi_sw_master_wait_while_scl_low(void);


bool twi_sw_master_init(void)
{
    TWI_DEBUG("%s",__func__);
#if 1
    hal_iomux_init((struct HAL_IOMUX_PIN_FUNCTION_MAP *)pinmux_iic, sizeof(pinmux_iic) / sizeof(struct HAL_IOMUX_PIN_FUNCTION_MAP));
#endif

    // Configure SCL as output
    TWI_SW_SCL_OUTPUT();
    TWI_SW_SCL_HIGH();

    // Configure SDA as output
    TWI_SW_SDA_OUTPUT();
    TWI_SW_SDA_HIGH();

    static uint8_t rw_switch = 4;
    if(rw_switch > 0)
    {
    	rw_switch--;
	    if(rw_switch == 2)
		    rw_switch = 0;
    }
    return twi_sw_master_clear_bus();
}

bool twi_sw_master_transfer(uint8_t address, uint8_t *data, uint16_t data_length, bool issue_stop_condition)
{
    bool transfer_succeeded = true;

    transfer_succeeded &= twi_sw_master_issue_startcondition();
#if 0
    TWI_DEBUG("startcondition transfer_succeeded = %d", transfer_succeeded);
#endif
    transfer_succeeded &= twi_sw_master_clock_byte(address);

    TWI_SW_DELAYN_SHORT();
    if (address & TWI_READ_BIT)
    {
        /* Transfer direction is from Slave to Master */
        while (data_length-- && transfer_succeeded)
        {
            // To indicate to slave that we've finished transferring last data byte
            // we need to NACK the last transfer.
            if (data_length == 0)
            {
                transfer_succeeded &= twi_sw_master_clock_byte_in(data, (bool)false);
            }
            else
            {
                transfer_succeeded &= twi_sw_master_clock_byte_in(data, (bool)true);
            }
            data++;
        }
    }
    else
    {
        /* Transfer direction is from Master to Slave */
        while (data_length-- && transfer_succeeded)
        {
            transfer_succeeded &= twi_sw_master_clock_byte(*data);
            data++;
        }
    }

#ifndef   STOPCONDITION_EXT
    if (issue_stop_condition || !transfer_succeeded)
    {
        transfer_succeeded &= twi_sw_master_issue_stopcondition();
    }
#endif

   
    return transfer_succeeded;
}

/**
 * Detects stuck slaves and tries to clear the bus.
 *
 * @return
 * @retval false Bus is stuck.
 * @retval true Bus is clear.
 */
static bool twi_sw_master_clear_bus(void)
{
    bool bus_clear;
    TWI_SW_SCL_OUTPUT();
    TWI_SW_SDA_OUTPUT();
    TWI_SW_SDA_HIGH();
    TWI_SW_SCL_HIGH();
    TWI_SW_DELAYN();

    if (TWI_SW_SDA_READ() == 1 && TWI_SW_SCL_READ() == 1)
    {
        bus_clear = true;
    }
    else if (TWI_SW_SCL_READ() == 1)
    {
        bus_clear = false;

        // Clock max 18 pulses worst case scenario(9 for master to send the rest of command and 9 for slave to respond) to SCL line and wait for SDA come high
        for (uint_fast8_t i = 18; i--;)
        {
            TWI_SW_SCL_LOW();
            TWI_SW_DELAYN();
            TWI_SW_SCL_HIGH();
            TWI_SW_DELAYN();

            if (TWI_SW_SDA_READ() == 1)
            {
                bus_clear = true;
                break;
            }
        }
    }
    else
    {
        bus_clear = false;
    }

//    TWI_DEBUG("iic bus_clear = %d", bus_clear);
    return bus_clear;
}

/**
 * Issues TWI START condition to the bus.
 *
 * START condition is signaled by pulling SDA low while SCL is high. After this function SCL and SDA will be low.
 *
 * @return
 * @retval false Timeout detected
 * @retval true Clocking succeeded
 */
static bool twi_sw_master_issue_startcondition(void)
{
	// Make sure both SDA and SCL are high before pulling SDA low.
	TWI_SW_SDA_INPUT();
	TWI_SW_SCL_INPUT();

    for (uint_fast8_t i = RETRY_COUNTER; 0 != i; i--)
    {
    	if( TWI_SW_SDA_READ() && TWI_SW_SCL_READ() )
    		break;
    	TWI_SW_DELAYN();
    }

	if (!twi_sw_master_wait_while_scl_low())
	{
		return false;
	}

	TWI_SW_SDA_OUTPUT();
	TWI_SW_SDA_LOW();
	TWI_SW_DELAYN();

	// Other module function expect SCL to be low
	TWI_SW_SCL_OUTPUT();
	TWI_SW_SCL_LOW();
	TWI_SW_DELAYN();

	return true;
}

/**
 * Issues TWI STOP condition to the bus.
 *
 * STOP condition is signaled by pulling SDA high while SCL is high. After this function SDA and SCL will be high.
 *
 * @return
 * @retval false Timeout detected
 * @retval true Clocking succeeded
 */
bool twi_sw_master_issue_stopcondition(void)
{

	TWI_SW_SDA_OUTPUT();

	TWI_SW_DELAYN();

	TWI_SW_SCL_INPUT();

    for (uint_fast8_t i = RETRY_COUNTER; 0 != i; i--)
    {
    	if( TWI_SW_SCL_READ() )
    		break;
    	TWI_SW_DELAYN();
    }
	TWI_SW_SDA_INPUT();
	TWI_SW_DELAYN();

	return true;
}

/**
 * Clocks one data byte out and reads slave acknowledgement.
 *
 * Can handle clock stretching.
 * After calling this function SCL is low and SDA low/high depending on the
 * value of LSB of the data byte.
 * SCL is expected to be output and low when entering this function.
 *
 * @param databyte Data byte to clock out.
 * @return
 * @retval true Slave acknowledged byte.
 * @retval false Timeout or slave didn't acknowledge byte.
 */

static bool twi_sw_master_clock_byte(uint_fast8_t databyte)
{
    bool transfer_succeeded = true;

    // Make sure SDA is an output

    // MSB first
    for (uint_fast8_t i = 0x80; i != 0; i >>= 1)
    {
		
    	TWI_SW_SCL_OUTPUT();
        TWI_SW_DELAYN();
        if (databyte & i)
        {
            TWI_SW_SDA_INPUT();
        }
        else
        {
			TWI_SW_SDA_OUTPUT();
        }

        if (!twi_sw_master_wait_while_scl_low())
        {
            transfer_succeeded = false; // Timeout
            break;
        }
    }
	

#if 1
    TWI_DEBUG("\n\r transfer_succeeded = %d ,%d", __LINE__,transfer_succeeded);
#else
    TWI_SW_DELAYN();
#endif
    // Finish last data bit by pulling SCL low
    
    TWI_SW_SCL_OUTPUT();
    TWI_SW_DELAYN();

	TWI_SW_SDA_INPUT();

    TWI_SW_DELAYN();


    // Configure TWI_SDA pin as input for receiving the ACK bit

    // Give some time for the slave to load the ACK bit on the line

    // Pull SCL high and wait a moment for SDA line to settle
    // Make sure slave is not stretching the clock
    transfer_succeeded &= twi_sw_master_wait_while_scl_low();
  
#if 1
    TWI_DEBUG("transfer_succeeded = %d ,%d", __LINE__,transfer_succeeded);
#else
	TWI_SW_DELAYN();
#endif
    // Read ACK/NACK. NACK == 1, ACK == 0
    transfer_succeeded &= !(TWI_SW_SDA_READ());
  
    TWI_SW_SCL_OUTPUT();
    // Finish ACK/NACK bit clock cycle and give slave a moment to release control
    // of the SDA line
    TWI_SW_DELAYN();

    // Configure TWI_SDA pin as output as other module functions expect that

    return transfer_succeeded;
}

/**
 * Clocks one data byte in and sends ACK/NACK bit.
 *
 * Can handle clock stretching.
 * SCL is expected to be output and low when entering this function.
 * After calling this function, SCL is high and SDA low/high depending if ACK/NACK was sent.
 *
 * @param databyte Data byte to clock out.
 * @param ack If true, send ACK. Otherwise send NACK.
 * @return
 * @retval true Byte read succesfully
 * @retval false Timeout detected
 */
static bool twi_sw_master_clock_byte_in(uint8_t *databyte, bool ack)
{
    uint_fast8_t byte_read = 0;
    bool transfer_succeeded = true;

    // Make sure SDA is an input
    TWI_SW_SDA_INPUT();

    // SCL state is guaranteed to be high here

    // MSB first
    for (uint_fast8_t i = 0x80; i != 0; i >>= 1)
    {
        if (!twi_sw_master_wait_while_scl_low())
        {
            transfer_succeeded = false;
            break;
        }

        if (TWI_SW_SDA_READ())
        {
            byte_read |= i;
        }
        else
        {
            // No need to do anything
        }

        TWI_SW_SCL_OUTPUT();
        TWI_SW_DELAYN();
    }

    // Make sure SDA is an output before we exit the function
    TWI_SW_SDA_OUTPUT();

    *databyte = (uint8_t)byte_read;

    // Send ACK bit

    // SDA high == NACK, SDA low == ACK
    if (ack)
    {
        TWI_SW_SDA_OUTPUT();
    }
    else
    {
        TWI_SW_SDA_INPUT();
    }

    // Let SDA line settle for a moment
    TWI_SW_DELAYN();

    // Drive SCL high to start ACK/NACK bit transfer
    // Wait until SCL is high, or timeout occurs
    if (!twi_sw_master_wait_while_scl_low())
    {
        transfer_succeeded = false; // Timeout
    }

    // Finish ACK/NACK bit clock cycle and give slave a moment to react
    TWI_SW_SCL_OUTPUT();
    TWI_SW_DELAYN();

    return transfer_succeeded;
}

/**
 * Pulls SCL high and waits until it is high or timeout occurs.
 *
 * SCL is expected to be output before entering this function.
 * @note If TWI_MASTER_TIMEOUT_COUNTER_LOAD_VALUE is set to zero, timeout functionality is not compiled in.
 * @return
 * @retval true SCL is now high.
 * @retval false Timeout occurred and SCL is still low.
 */
static bool twi_sw_master_wait_while_scl_low(void)
{
    uint32_t volatile timeout_counter = 1000; //TWI_MASTER_TIMEOUT_COUNTER_LOAD_VALUE;

	TWI_SW_SCL_INPUT();
	TWI_SW_DELAYN_SHORT();

	while (TWI_SW_SCL_READ() == 0)
	{
		// If SCL is low, one of the slaves is busy and we must wait
		if (timeout_counter-- == 0)
		{
			// If timeout_detected, return false
			return false;
		}
	}

	return true;
}

