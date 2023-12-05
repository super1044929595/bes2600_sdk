
#include "pmu.h"
#include "hal_gpio.h"
#include "hal_iomux.h"
#include "stdio.h"
#include "cmsis_os.h"
#include "string.h"
#include "hal_timer.h"
#include "hal_trace.h"

#include "mcu_sy5501.h"
#include "able_sy5501_cfg.h"
//#include "tgt_hardware_customer.h"

#include "able_driver_iic.h"

#define ABLE_SY5501_IRQ_PIN    HAL_GPIO_PIN_P0_0
#define ABLE_VDD_1_7V_PIN      HAL_GPIO_PIN_NUM

#define ABLE_MCU_SY5501_DEBUG
#ifdef ABLE_MCU_SY5501_DEBUG
#define ABLE_MCU_SY5501_LOG(str, ...)  TRACE(1, "[SY5501]"str, ##__VA_ARGS__)
#else
#define ABLE_MCU_SY5501_LOG(str, ...)
#endif

bool is_sy5501_init_success = false;


static bool sy5501_twi_master_write(uint8_t address,uint8_t reg_addr, uint8_t data);
static bool sy5501_twi_master_read(uint8_t address,uint8_t reg_addr, uint8_t *data);

static HAL_GPIO_PIN_IRQ_HANDLER sy5501_irq_callback = NULL;

const uint8_t sy5501_list[][2] = {
#if 0
    //{0x72, 0x97},
    //{0x73, 0x39},
    {0x40, 0x5a},
    {0x41, 0x73},
    {0x42, 0x00},
    {0x43, 0x01},
    {0x50, 0xA0|CMD_FREQ},//charge max 200ma
    {0x60, 0x1B}, //charge max 4.4V
    {0x61, 0x80},
    {0x62, 0x0b},
#else
    //{SY5501_WPEn0_REG,              SY5501_WPEn0_REG_INIT},
    //{SY5501_WPEn1_REG,              SY5501_WPEn1_REG_INIT},
    {SY5501_IO_CaseChk_SET_REG,     SY5501_IO_CaseChk_SET_REG_INIT},
    {SY5501_RST_Delay_SET_REG,      SY5501_RST_Delay_SET_REG_INIT},
    {SY5501_PWK_SET_REG,            SY5501_PWK_SET_REG_INIT},
    {SY5501_VinPullDown_SET_REG,    SY5501_VinPullDown_SET_REG_INIT},
    {SY5501_VTK_IBATOC_TRX_SET_REG, SY5501_VTK_IBATOC_TRX_SET_REG_INIT|CMD_FREQ},//charge max 200ma
    {SY5501_NTC_VIBATF_OD_SET_REG,  SY5501_NTC_VIBATF_OD_SET_REG_INIT}, //charge max 4.4V
    {SY5501_ITK_VRECG_ICC_SET_REG,  SY5501_ITK_VRECG_ICC_SET_REG_INIT},
    {SY5501_CHGTO_ODVer_PTVer_REG,  SY5501_CHGTO_ODVer_PTVer_REG_INIT},
#endif
};

void sy5501_reg_printf(void)
{
#if 1 //debug:out sy5501 register status
    uint8_t reg_data = 0;

	ABLE_MCU_SY5501_LOG("%s ", __func__);

    sy5501_twi_master_read(SY5501_ADDR, 0x10, &reg_data);
    sy5501_twi_master_read(SY5501_ADDR, 0x11, &reg_data);
    sy5501_twi_master_read(SY5501_ADDR, 0x20, &reg_data);
    sy5501_twi_master_read(SY5501_ADDR, 0x40, &reg_data);
    sy5501_twi_master_read(SY5501_ADDR, 0x41, &reg_data);
    sy5501_twi_master_read(SY5501_ADDR, 0x42, &reg_data);
    sy5501_twi_master_read(SY5501_ADDR, 0x43, &reg_data);
    sy5501_twi_master_read(SY5501_ADDR, 0x50, &reg_data);
    sy5501_twi_master_read(SY5501_ADDR, 0x60, &reg_data);
    sy5501_twi_master_read(SY5501_ADDR, 0x61, &reg_data);
    sy5501_twi_master_read(SY5501_ADDR, 0x62, &reg_data);
#endif
    return;
}


/**
 * @FUNC: sy5501_unlock_wpen.
 * @NOTE: write sy5501 0x50 0x60 0x61 0x62 Reg must unlock sy5501 wpen.
 * @RETURN: false:ERR; true:SUCCESS.      
 */ 
bool sy5501_unlock_wpen(void)
{
    bool ret = true;
    ret &= sy5501_twi_master_write(SY5501_ADDR, SY5501_WPEn0_REG, WPEn0_DISA);
    ret &= sy5501_twi_master_write(SY5501_ADDR, SY5501_WPEn1_REG, WPEn1_DISA);
    return ret;
}

/**
 * @FUNC: sy5501_lock_wpen.
 * @NOTE: write sy5501 0x50 0x60 0x61 0x62 Reg finish. lock sy5501 wpen.
 * @RETURN: false:ERR; true:SUCCESS.       
 */ 

bool sy5501_lock_wpen(void)
{
    bool ret = true;
    ret &= sy5501_twi_master_write(SY5501_ADDR, SY5501_WPEn0_REG, WPEn0_EN);
    ret &= sy5501_twi_master_write(SY5501_ADDR, SY5501_WPEn1_REG, WPEn1_EN);
    return ret;
}

/**
 * @FUNC: sy5501_read_st_case
 * @RETURN: INBOX_STATUS \ OUTBOX_STATUS
 */
uint8_t sy5501_read_st_case(void)
{
	//if sy5501 init fail, awayls return inbox.
    uint8_t reg_data = INBOX_STATUS;

	if(is_sy5501_init_success) {
	    sy5501_twi_master_read(SY5501_ADDR, SY5501_STATE0_REG, &reg_data); 
	    reg_data &= ST_CASE_BIT_MARK;
	}
    return reg_data;
}


/**
 * @FUNC: sy5501_read_state0_reg
 * @RETURN: 0: SUCCESS.  !0 :ERR 
*/           
uint8_t sy5501_read_state0_reg(void)
{
    uint8_t reg_data = 0;
	if(is_sy5501_init_success) {
    	sy5501_twi_master_read(SY5501_ADDR, SY5501_STATE0_REG, &reg_data); 
	}
    return reg_data;	
}


/**
 * @FUNC: sy5501_read_state2_reg
 * @RETURN: 0: SUCCESS.  !0 :ERR 
*/           
uint8_t sy5501_read_state2_reg(void)
{
    uint8_t reg_data = 0;
	if(is_sy5501_init_success) {
    	sy5501_twi_master_read(SY5501_ADDR, SY5501_STATE2_REG, &reg_data); 
	}
    return reg_data;	
}

/**
 * @FUNC: sy5501_reset_state2_reg
 * @RETURN: 0: SUCCESS.  !0 :ERR 
*/           
void sy5501_write_state2_reg(uint8_t data)
{
	if(is_sy5501_init_success) {
    	sy5501_twi_master_write(SY5501_ADDR, SY5501_STATE2_REG, data); 
	}	
}

/**
 * @FUNC: sy5501_write_i2c_cmd_reg
 * @PARAM: 
 * @RETURN: false: ERR  true :SUCCESS 
*/           
bool sy5501_write_i2c_cmd_reg(uint8_t write_data)
{
	bool ret = false;
	if(is_sy5501_init_success) {
		ret = sy5501_twi_master_write(SY5501_ADDR, SY5501_I2C_CMD_REG, write_data);
	}
	return ret;
}

bool sy5501_read_vin_ok(void)
{
	bool ret = false;
	uint8_t read_data = 0 ;
	if(is_sy5501_init_success) {
		if(sy5501_twi_master_read(SY5501_ADDR, SY5501_STATE1_REG, &read_data)) {
			ret = read_data & VIN_BETWEEN_VINOVP_AND_VINUVLO;
		}
	}
	ABLE_MCU_SY5501_LOG("[%s] Vin_ok:%d", __func__, ret);
	return ret;
}


/***************** customer function ***************************/
void sy5501_enter_shipmode_before_shutdown(void)
{
	bool ret = false;
	if(is_sy5501_init_success) {
		ret = sy5501_twi_master_write(SY5501_ADDR, SY5501_NTC_VIBATF_OD_SET_REG, SY5501_NTC_VIBATF_OD_SET_REG_INIT);
	    ret &= sy5501_twi_master_write(SY5501_ADDR, SY5501_I2C_CMD_REG,EXIT_TRX_MODE);
	    ret &= sy5501_twi_master_write(SY5501_ADDR, SY5501_STATE2_REG, 0xFF);
	    ret &= sy5501_twi_master_write(SY5501_ADDR, SY5501_I2C_CMD_REG, TURNON_SHIP_MODE);
		
	}
	ABLE_MCU_SY5501_LOG("[%s] %d", __func__, ret);
}

void sy5501_set_vin_push_pull_mode(void)
{
 	
}


void sy5501_set_vin_open_drain_mode(void)
{
	bool nRet =false;
	if(is_sy5501_init_success) {
	    nRet = sy5501_twi_master_write(SY5501_ADDR, SY5501_VTK_IBATOC_TRX_SET_REG, \
	                (TRX_PIN_SEL_OPENDRAIN|BAT_OVER_CURRENT_200MA|CMD_FREQ)); //0xA0|CMD_FREQ);
	}
    ABLE_MCU_SY5501_LOG("<%s> ,%u", __func__,nRet);
}

/* 
  Note!!! before enter ota /single download ,must set the reg following:
   1.set TRX  push_pull mode (reg:0x50)
   2. disable inbox /outbox detect (reg��0x40)
*/
void sy5501_enter_single_line_download(void)
{
	bool nRet = false;
    uint8_t data,data2;

	if(is_sy5501_init_success) {
	    nRet = sy5501_twi_master_write(SY5501_ADDR, SY5501_VTK_IBATOC_TRX_SET_REG, \
	                (TRX_PIN_SEL_OPENDRAIN|BAT_OVER_CURRENT_200MA|CMD_FREQ)); //0xA0|CMD_FREQ);
	    nRet &= sy5501_twi_master_write(SY5501_ADDR, SY5501_IO_CaseChk_SET_REG, \
	                (INBOX_AN_DISA|INBOX_DET_DISA|PWK_RST_TRX_PIN_EN|RST_EFF_LVL_SEL_H|PWK_EFF_LVL_SEL_H));//0x7A);
	    
	    nRet = sy5501_twi_master_read(SY5501_ADDR, SY5501_VTK_IBATOC_TRX_SET_REG, &data);
		nRet = sy5501_twi_master_read(SY5501_ADDR, SY5501_IO_CaseChk_SET_REG, &data2);
	}
    ABLE_MCU_SY5501_LOG("<%s> =%u,0x50=0x%02x,0x40=0x%02x", __func__,nRet,data,data2);
}

bool sy5501_set_charger_current(uint8_t charge_current) 
{
	uint8_t reg_data;
	bool ret = false;
	
	if(charge_current < ICC_SEL_TEN_PERCENT_ICH || charge_current > ICC_SEL_THREE_HUNDRED_PERCENT_ICH) {
		return false;
	}

	if(is_sy5501_init_success) {
		ret = sy5501_twi_master_read(SY5501_ADDR, SY5501_ITK_VRECG_ICC_SET_REG, &reg_data); 
		if(ret) {
	    	reg_data &= (~VER_ICC_BIT_MARK);
	    	reg_data |= (charge_current & VER_ICC_BIT_MARK);  
			sy5501_twi_master_write(SY5501_ADDR, SY5501_ITK_VRECG_ICC_SET_REG, reg_data);
		}
	}
	ABLE_MCU_SY5501_LOG("%s %d", __func__, ret);
	return ret;
	
}

void sy5501_set_vfloat_4p20v(void)
{
	bool ret = false;
	uint8_t reg_data;

	if(is_sy5501_init_success) {
		ret = sy5501_twi_master_read(SY5501_ADDR, SY5501_NTC_VIBATF_OD_SET_REG, &reg_data);
		if(ret) {
			reg_data &= (~VER_BATF_BIT_MARK);
			reg_data |= (BATF_SEL_4P20V);
			ret = sy5501_twi_master_write(SY5501_ADDR, SY5501_NTC_VIBATF_OD_SET_REG, reg_data);
		}
	}
	ABLE_MCU_SY5501_LOG("[%s] %d", __func__, ret);
}

void sy5501_set_vfloat_4p35v(void)
{
	bool ret = false;
	uint8_t reg_data;

	if(is_sy5501_init_success) {
		ret = sy5501_twi_master_read(SY5501_ADDR, SY5501_NTC_VIBATF_OD_SET_REG, &reg_data);

		if(ret) {
			reg_data &= (~VER_BATF_BIT_MARK);
			reg_data |= (BATF_SEL_4P35V);
			ret = sy5501_twi_master_write(SY5501_ADDR, SY5501_NTC_VIBATF_OD_SET_REG, reg_data);
		}
	}
	ABLE_MCU_SY5501_LOG("[%s] %d", __func__, ret);
}


static bool sy5501_reg_init(void)
{
	uint8_t i;
	bool bret = true;
	
	uint8_t reg_addr,write_data,read_data;

	uint8_t list_len = sizeof(sy5501_list)/sizeof(sy5501_list[0]);
	
	sy5501_unlock_wpen();

	osDelay(1);
	
	for (i = 0; i < list_len; i++) {
		reg_addr = sy5501_list[i][0];
		write_data = sy5501_list[i][1];

		bret &= sy5501_twi_master_write(SY5501_ADDR, reg_addr, write_data);

		if(reg_addr !=  SY5501_WPEn0_REG && reg_addr != SY5501_WPEn1_REG) {
			//check again
			sy5501_twi_master_read(SY5501_ADDR, reg_addr, &read_data);

			if(read_data != write_data) {
				//write again
				bret &= sy5501_twi_master_write(SY5501_ADDR, reg_addr, write_data);
			}
		}
		
	}
	return bret;
}


/**************   customer  interface  *************/

const struct HAL_IOMUX_PIN_FUNCTION_MAP sy5501_irq_pinmux[1] = {
    {ABLE_SY5501_IRQ_PIN, HAL_IOMUX_FUNC_AS_GPIO, HAL_IOMUX_PIN_VOLTAGE_VIO, HAL_IOMUX_PIN_PULLUP_ENABLE}
};

static bool sy5501_twi_master_write(uint8_t address,uint8_t reg_addr, uint8_t data)
{
    int retry_cnt = 0;
    bool ret;

	do {
		ret = able_iic_master_write(address, reg_addr, data);
		ABLE_MCU_SY5501_LOG("W REG:%02X,DATA:%02X %s", reg_addr, data, (ret)?"":"FAIL"); 
	}while(retry_cnt++ < 10 && !ret);

	return ret;
}

static bool sy5501_twi_master_read(uint8_t address,uint8_t reg_addr, uint8_t *data)
{
	int retry_cnt = 0;
    bool ret;

	do {
		ret = able_iic_master_read(address, reg_addr, data);
		ABLE_MCU_SY5501_LOG("R REG:%02X,DATA:%02X %s", reg_addr, *data, (ret)?"":"FAIL"); 
	}while(retry_cnt++ < 10 && !ret);

	return ret;
}

static void sy5501_irq_handler(enum HAL_GPIO_PIN_T pin)
{
	//ABLE_MCU_SY5501_LOG("%s", __func__);

	if(sy5501_irq_callback != NULL) {
		sy5501_irq_callback(pin);
	}
}

const struct HAL_IOMUX_PIN_FUNCTION_MAP vdd_1_7V_pinmux[1] = {
    {HAL_GPIO_PIN_P0_4, HAL_IOMUX_FUNC_AS_GPIO, HAL_IOMUX_PIN_VOLTAGE_VIO, HAL_IOMUX_PIN_NOPULL}
};
static void sy5501_int_pin_init(void)
{
    hal_iomux_init(vdd_1_7V_pinmux, 1);
    hal_gpio_pin_set_dir((enum HAL_GPIO_PIN_T)vdd_1_7V_pinmux[0].pin, HAL_GPIO_DIR_OUT, 1);
    hal_gpio_pin_set((enum HAL_GPIO_PIN_T)vdd_1_7V_pinmux[0].pin);

	hal_iomux_init(sy5501_irq_pinmux, 1);
	hal_gpio_pin_set_dir((enum HAL_GPIO_PIN_T)sy5501_irq_pinmux[0].pin, HAL_GPIO_DIR_IN, 0);
}

static void sy5501_int_irq_handler(void)
{
	struct HAL_GPIO_IRQ_CFG_T gpiocfg;
	gpiocfg.irq_enable = 1;
	gpiocfg.irq_debounce = 0;
	gpiocfg.irq_type = HAL_GPIO_IRQ_TYPE_EDGE_SENSITIVE;
	gpiocfg.irq_polarity = HAL_GPIO_IRQ_POLARITY_LOW_FALLING;
	gpiocfg.irq_handler = (HAL_GPIO_PIN_IRQ_HANDLER)sy5501_irq_handler;
	hal_gpio_setup_irq((enum HAL_GPIO_PIN_T)sy5501_irq_pinmux[0].pin, &gpiocfg);
}

static bool sy5501_hardware_check(void)
{
	bool result = false;
	bool iic_read_ret;
	uint8_t read_data;

	uint8_t test_cnt = 0,  int_pin_low_cnt = 0;

	uint8_t retry_cnt = 0;

	do {
		test_cnt = 0;
		int_pin_low_cnt = 0;
	
		//set float input
		hal_iomux_set_io_pull_select(sy5501_irq_pinmux[0].pin, HAL_IOMUX_PIN_NOPULL);
		
		//check IRQ pin
		hal_gpio_pin_clr(ABLE_VDD_1_7V_PIN);   //disable vdd
		do{
			if(test_cnt == 15) {
				hal_gpio_pin_set(ABLE_VDD_1_7V_PIN);
				osDelay(5);
			}
			if(hal_gpio_pin_get_val(sy5501_irq_pinmux[0].pin) == 0) {
				int_pin_low_cnt++;
			}
		}while(test_cnt++ < 50);	
		ABLE_MCU_SY5501_LOG("irq_pin_low_cnt %d/%d", int_pin_low_cnt, test_cnt);

		//set uppull input
		hal_iomux_set_io_pull_select(sy5501_irq_pinmux[0].pin, HAL_IOMUX_PIN_PULLUP_ENABLE);

		//check SCL\SDA, I2C communication
		iic_read_ret = sy5501_twi_master_read(SY5501_ADDR, SY5501_STATE0_REG, &read_data);
		ABLE_MCU_SY5501_LOG("iic read result %d", iic_read_ret);

		if(int_pin_low_cnt > 3 && int_pin_low_cnt < test_cnt && iic_read_ret) {
			result = true;
		}

		retry_cnt++;
	}while(!result && retry_cnt < 10);
	
	ABLE_MCU_SY5501_LOG("%s %d", __func__, result);	

	return result;
}

bool sy5501_get_init_status(void)
{
	return is_sy5501_init_success;
}

void sy5501_init(uint32_t irq_handler)
{
	sy5501_int_pin_init();

	if(sy5501_hardware_check()) {
			
		sy5501_irq_callback = (HAL_GPIO_PIN_IRQ_HANDLER)irq_handler;

		sy5501_reg_init();
		sy5501_int_irq_handler();

		is_sy5501_init_success = true;
	} else {
		is_sy5501_init_success = false;
	}
	ABLE_MCU_SY5501_LOG("%s %d", __func__, is_sy5501_init_success);
}

