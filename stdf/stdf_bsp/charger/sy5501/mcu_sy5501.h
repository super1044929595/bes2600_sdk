#ifndef __MCU_SY5501_H__
#define __MCU_SY5501_H__

#ifdef __cplusplus
extern "C" {
#endif
#include <stdint.h>


/************************** SY5501 chip addr ****************************/
#define SY5501_ADDR                                (0X16)

/********* SY5501 Reg define, according <<SY5501_Ver0.0.7.PDF>> *********/
#define SY5501_STATE0_REG                           (0x10)
/* macro for SY5501_STATE0_REG Reg and st_reg_cfg_ok Bit, only Read */
//whether SY5501_IO_CaseChk_SET_REG Reg reg_cfg_ok Bit set PWK_RST_TRX_PIN_EN flag.
#define ST_REG_CFG_OK_BIT_MARK                      (0x40)

#define PWK_RST_TRX_IS_EN_FLG                       (0x1<<6)

#define PWK_RST_TRX_IS_DISA_FLG                     (0x0<<6)
/* macro for SY5501_STATE0_REG Reg and st_init_ok Bit, only Read */
//whether reading fuse statu when sy5501 poer-on.
#define ST_INIT_OK_BIT_MARK                         (0x20)
#define READ_FUSE_OK                                (0x1<<5)
#define FUSE_NO_READ                                (0x0<<5)
/* macro for SY5501_STATE0_REG Reg and st_case_lowbat Bit, only Read */
//box feed status.outbox will clr 0.
#define ST_CASE_LOWBAT_BIT_MARK                     (0x10)
#define BOX_IS_FEED_FLG                             (0x1<<4)
#define BOX_NO_FEED_FLG                             (0x0<<4)
/* macro for SY5501_STATE0_REG Reg and st_case Bit, only Read */
//INBOX or OUTBOX.
#define ST_CASE_BIT_MARK                            (0x08)
#define INBOX_STATUS                                (0x1<<3)
#define OUTBOX_STATUS                               (0x0<<3)

/* macro for SY5501_STATE0_REG Reg and st_chip[2:0] Bit, only Read */
//SY5501 MODE.
#define ST_CHIP_BIT_MARK                            (0x07)
#define SY5501_IS_STANDBY_MODE                      (0x0<<0)
#define SY5501_IS_SHIP_MODE                         (0x1<<0)
#define SY5501_IS_TRX_MODE                          (0x2<<0)
#define SY5501_IS_UNCHARGE_MODE                     (0x3<<0)
#define SY5501_IS_TRICKLE_MODE                      (0x4<<0)
#define SY5501_IS_CONST_CURRENT_MODE                (0x5<<0)
#define SY5501_IS_CONST_VOLTAGE_MODE                (0x6<<0)
#define SY5501_IS_FULL_CHARGE_MODE                  (0x7<<0)


/* macro for SY5501_STATE1_REG Reg */
#define SY5501_STATE1_REG                           (0x11) 
/* macro for SY5501_STATE1_REG Reg and st_overtime Bit, only Read */
//whether charge is overtime.
#define ST_OVERTIME_BIT_MARK                        (0x80)
#define CHARGE_IS_OVERTIME                          (0x1<<7)
#define CHARGE_NO_OVERTIME                          (0x0<<7)
/* macro for SY5501_STATE1_REG Reg and st_vin_h1v Bit, only Read */
//whether VIN > 1V.
#define ST_VIN_H1V_BIT_MARK                         (0x40)
#define VIN_IS_GREATER_1V                           (0x1<<6)
#define VIN_IS_LESS_1V                              (0x0<<6)
/* macro for SY5501_STATE1_REG Reg and st_ntc_sdb[1:0] Bit, only Read */
//ntc temperature in uncharge.
#define ST_NTC_SDB_BIT_MARK                         (0X30)
#define UNCHARGE_NTC_TEMP_IS_LESS_MINUS10           (0x0<<4)
#define UNCHARGE_NTC_TEMP_BETWEEN_MINUS10_AND_60    (0x1<<4)
#define UNCHARGE_NTC_TEMP_IS_GREATER_60             (0x2<<4)
#define UNCHARGE_NTC_TEMP_IS_RSV                    (0x3<<4)
/* macro for SY5501_STATE1_REG Reg and st_ntc_cg[2:0] Bit, only Read */
//ntc temperature in charge.
#define ST_NTC_CG_BIT_MARK                          (0x0e)
#define CHARGE_NTC_TEMP_IS_LESS_MINUS10             (0x0<<1)
#define CHARGE_NTC_TEMP_BETWEEN_MINUS10_AND_0       (0x1<<1)
#define CHARGE_NTC_TEMP_BETWEEN_0_AND_10            (0x2<<1)
#define CHARGE_NTC_TEMP_BETWEEN_10_AND_20           (0x3<<1)
#define CHARGE_NTC_TEMP_BETWEEN_20_AND_45           (0x4<<1)
#define CHARGE_NTC_TEMP_BETWEEN_45_AND_60           (0x5<<1)
#define CHARGE_NTC_TEMP_IS_GREATER_60               (0x6<<1)
#define CHARGE_NTC_TEMP_IS_RSV                      (0x7<<1)
/* macro for SY5501_STATE1_REG Reg and st_vinok Bit, only Read */
//VIN status: >VINovp, <VINuvlo, normal.
#define ST_VINOK_BIT_MARK                           (0x01)
#define VIN_BETWEEN_VINOVP_AND_VINUVLO              (0x1<<0)
#define VIN_GREATER_VINOVP_OR_LESS_VINUVLO          (0x0<<0)


/* macro for SY5501_STATE2_REG Reg */
#define SY5501_STATE2_REG                           (0x20)
/* macro for SY5501_STATE2_REG Reg and st_vccocp Bit, Write 1 clr */
//VCC OCP status. read 1: VCC is over current protection.
#define ST_VCCOCP_BIT_MARK                          (0x80)
#define VCC_IN_OCP_STATUS                           (0x1<<7) 
#define VCC_NO_OCP_STATUS                           (0x0<<7) 
/* macro for SY5501_STATE2_REG Reg and st_otp Bit, Write 1 clr */
//sy5501 chip temperature is too high status (>110¡æ). read 1: chip temp is too high.
#define ST_OTP_BIT_MARK                             (0x40)
#define IN_OVER_TEMP_STATUS                         (0x1<<6)
#define NO_OVER_TEMP_STATUS                         (0x0<<6)
/* macro for SY5501_STATE2_REG Reg and st_vinovp Bit, Write 1 clr */
//VIN OVP status. read 1: VIN is over voltage protection.
#define ST_OVINOVP_BIT_MARK                         (0x20)
#define VIN_IN_OVP_STATUS                           (0x1<<5) 
#define VIN_NO_OVP_STATUS                           (0x0<<5) 
/* macro for SY5501_STATE2_REG Reg and st_ich_short Bit, Write 1 clr */
//ICH short circuit status. read 1: ICH is short circuit.
#define ST_ICH_SHORT_BIT_MARK                       (0x10)
#define ICH_IN_SHORT_CIRCUIT_STATUS                 (0x1<<4)
#define ICH_NO_SHORT_CIRCUIT_STATUS                 (0x0<<4)
/* macro for SY5501_STATE2_REG Reg and st_vsys_short Bit, Write 1 clr */
//VSYS short circuit status. read 1: VSYS is short circuit.
#define ST_VSYS_SHORT_BIT_MARK                      (0x08)
#define VSYS_IN_SHORT_CIRCUIT_STATUS                (0x1<<3)
#define VSYS_NO_SHORT_CIRCUIT_STATUS                (0x0<<3)
/* macro for SY5501_STATE2_REG Reg and st_bat_od Bit, Write 1 clr */
//VBAT OD status. read 1: VBAT is over discharge.
#define ST_BAT_OD_BIT_MARK                          (0x04)
#define VBAT_IN_OVER_DISCHARGE_STATUS               (0x1<<2)
#define VBAT_NO_OVER_DISCHARGE_STATUS               (0x0<<2)
/* macro for SY5501_STATE2_REG Reg and st_bat_oc Bit, Write 1 clr */
//VSYS OCP status. read 1: VSYS is over current protection.
#define ST_BAT_OC_BIT_MARK                          (0x02)
#define VSYS_IN_OCP_STATUS                          (0x1<<1) 
#define VSYS_NO_OCP_STATUS                          (0x0<<1) 
/* macro for SY5501_STATE2_REG Reg and st_batovp Bit, Write 1 clr */
//VBAT OVP status. read 1: VBAT is over voltage protection.
#define ST_BATOVP_BIT_MARK                          (0x01)
#define VBAT_IN_OVP_STATUS                          (0x1<<0) 
#define VBAT_NO_OVP_STATUS                          (0x0<<0)


#define SY5501_IO_CaseChk_SET_REG                   (0x40)
/* macro for SY5501_IO_CaseChk_SET_REG Reg and ver_pwk_shipm Bit */
//whether send PWK when exit SHIP mode.
#define VER_PWK_SHIPM_BIT_MARK                      (0x80)
#define EXIT_SHIPMODE_SEND_PWK_DISA                 (0x0<<7)
#define EXIT_SHIPMODE_SEND_PWK_EN                   (0x1<<7)
/* macro for SY5501_IO_CaseChk_SET_REG Reg and ver_case_an Bit */
//whether support inbox infeed detection.
#define VER_CASE_AN_BIT_MARK                        (0x40)
#define INBOX_AN_EN                                 (0X0<<6) 
#define INBOX_AN_DISA                               (0X1<<6) 
/* macro for SY5501_IO_CaseChk_SET_REG Reg and ver_dis_casedet Bit */
//whether support inbox and outbox dectection.
#define VER_DIS_CASEDET_BIT_MARK                    (0x20)
#define INBOX_DET_EN                                (0x0<<5)
#define INBOX_DET_DISA                              (0x1<<5)
/* macro for SY5501_IO_CaseChk_SET_REG Reg and reg_cfg_ok Bit */
//enable sy5501 PWK RST TRX pin.
#define VER_REG_CFG_OK_BIT_MARK                     (0x10)
#define PWK_RST_TRX_PIN_EN                          (0x1<<4)
#define PWK_RST_TRX_PIN_DISA                        (0x0<<4)
/* macro for SY5501_IO_CaseChk_SET_REG Reg and ver_rst_level Bit */
//sy5501 RST effective level seting.
#define VER_RST_LEVEL_BIT_MART                      (0x08)
#define RST_EFF_LVL_SEL_H                           (0x1<<3)
#define RST_EFF_LVL_SEL_L                           (0x0<<3)
/* macro for SY5501_IO_CaseChk_SET_REG Reg and ver_rst_func Bit */
//whether send RST when inbox.
#define VER_RST_FUNC_BIT_MARK                       (0x04)
#define INBOX_SEND_RST_EN                           (0x1<<2)
#define INBOX_SEND_RST_DISA                         (0x0<<2)
/* macro for SY5501_IO_CaseChk_SET_REG Reg and ver_pwk_level Bit */
//sy5501 PWK effective level seting.
#define VER_PWK_LEVEL_BIT_MARK                      (0x02)
#define PWK_EFF_LVL_SEL_H                           (0x1<<1)
#define PWK_EFF_LVL_SEL_L                           (0x0<<1)
/* macro for SY5501_IO_CaseChk_SET_REG Reg and ver_pwk_func Bit */
//whether send PWK when outbox.
#define VER_PWK_FUNC_BIT_MARK                       (0x01)
#define OUTBOX_SEND_PWK_EN                          (0x1<<0)
#define OUTBOX_SEND_PWK_DISA                        (0x0<<0)


#define SY5501_RST_Delay_SET_REG                    (0x41)
/* macro for SY5501_RST_Delay_SET_REG Reg and ver_irq_width Bit */
//IRQ effectivet time width seting.
#define VER_IRQ_WIDTH_BIT_MARK                      (0x80)
#define IRQ_WIDTH_8MS                               (0x0<<7)
#define IRQ_WIDTH_2S                                (0x1<<7)
/* macro for SY5501_RST_Delay_SET_REG Reg and ver_trk_disvsys Bit */
//whether open VSYS when trickle charge.
#define VER_TRK_DISVSYS_BIT_MARK                    (0x40)
#define TRK_EN_VSYS                                 (0x0<<6)
#define TRK_DISA_VSYS                               (0x1<<6)
/* macro for SY5501_RST_Delay_SET_REG Reg and ver_rst_width[1:0] Bit */
//RST effectivet time width seting.
#define VER_RST_WIDTH_BIT_MARK                      (0x30)
#define RST_WIDTH_50MS                              (0x0<<4)
#define RST_WIDTH_100MS                             (0x1<<4)
#define RST_WIDTH_200MS                             (0x2<<4)
#define RST_WIDTH_500MS                             (0x3<<4)
/* macro for SY5501_RST_Delay_SET_REG Reg and ver_rst_delay[2:0] Bit */ 
//seting for send RST delay time when inbox.
#define VER_RST_DELAY_BIT_MARK                      (0x0e)
#define RST_DELAY_1S                                (0x0<<1)
#define RST_DELAY_2S                                (0x1<<1)
#define RST_DELAY_4S                                (0x2<<1)
#define RST_DELAY_8S                                (0x3<<1)
#define RST_DELAY_10S                               (0x4<<1)
#define RST_DELAY_12S                               (0x5<<1)
#define RST_DELAY_16S                               (0x6<<1)
#define RST_DELAY_20S                               (0x7<<1)
/* macro for SY5501_RST_Delay_SET_REG Reg and ver_shipm_delay Bit */ 
//seting for shut off VSYS delay time when into shipmode.
#define VER_SHIPMODE_DELAY_BIT_MARK                 (0x01)
#define SHIPMODE_SHUT_VSYS_DELAY_4S                 (0x1<<0)
#define SHIPMODE_SHUT_VSYS_NO_DELAY                 (0x0<<0)


#define SY5501_PWK_SET_REG                          (0x42)
/* macro for SY5501_PWK_SET_REG Reg and ver_pwk_interval[1:0] Bit */ 
//set interval between the two pulses when double click.
#define VER_PWK_INTERVAL_BIT_MARK                   (0xc0)
#define PWK_INTERVAL_100MS                          (0x0<<6)
#define PWK_INTERVAL_200MS                          (0x1<<6)
#define PWK_INTERVAL_300MS                          (0x2<<6)
#define PWK_INTERVAL_400MS                          (0x3<<6)
/* macro for SY5501_PWK_SET_REG Reg and ver_pwk_long[1:0] Bit */ 
//PWK long press time length.
#define VER_PWK_LONG_BIT_MARK                       (0x30)
#define PWK_LONG_2S                                 (0x0<<4)
#define PWK_LONG_3S                                 (0x1<<4)
#define PWK_LONG_4S                                 (0x2<<4)
#define PWK_LONG_5S                                 (0x3<<4)
/* macro for SY5501_PWK_SET_REG Reg and ver_pwk_short Bit */
//PWK click, double click, three click effective time.
#define VER_PWK_SHORT_BIT_MARK                      (0x08)
#define PWK_SHORT_EFF_TIM_200MS                     (0x0<<3)
#define PWK_SHORT_EFF_TIM_500MS                     (0x1<<3)
/* macro for SY5501_PWK_SET_REG Reg and ver_pwk_slong[2:0] Bit */  
//PWK long long press time length.
#define VER_PWK_SLONG_BIT_MARK                      (0x07)
#define PWK_SLONG_EFF_TIM_4S                        (0x0<<0)
#define PWK_SLONG_EFF_TIM_6S                        (0x1<<0)
#define PWK_SLONG_EFF_TIM_8S                        (0x2<<0)
#define PWK_SLONG_EFF_TIM_10S                       (0x3<<0)
#define PWK_SLONG_EFF_TIM_12S                       (0x4<<0)
#define PWK_SLONG_EFF_TIM_14S                       (0x5<<0)
#define PWK_SLONG_EFF_TIM_16S                       (0x6<<0)
#define PWK_SLONG_EFF_TIM_20S                       (0x7<<0)


#define SY5501_VinPullDown_SET_REG                  (0x43)
/* macro for SY5501_VinPullDown_SET_REG Reg and ver_recg_vinpd Bit */
//whether pull down VIN when request recharge.
#define VER_RECG_VINPD_BIT_MARK                     (0x02)
#define RECG_PULLDOWN_VIN_EN                        (0x1<<1)
#define RECG_PULLDOWN_VIN_DISA                      (0x0<<1)
/* macro for SY5501_VinPullDown_SET_REG Reg and ver_casein_vinpd Bit */
//whether pull down VIN when inbox.
#define VER_CASEIN_VINPD_BIT_MARK                   (0x01)
#define INBOX_PULLDOWN_VIN_EN                       (0x1<<0)
#define INBOX_PULLDOWN_VIN_DISA                     (0x0<<0)


#define SY5501_I2C_CMD_REG                          (0x44)
/* macro for SY5501_I2C_CMD_REG Reg and i2c_cmd[3:0] Bit */
#define CMD_BIT_MARK                                (0x0F)
//i2c cmd.
#define SY_COMM_CHARGE_NONE                         (0X0)
#define EN_CHRG                                     (0x1)
#define DISA_CHRG                                   (0x2)
#define TURNON_SHIP_MODE                            (0x3)
#define RESTART                                     (0x4)
#define EXIT_TRX_MODE                               (0x5)
#define CLR_RST                                     (0x6)
#define CLR_IRQ                                     (0x7)


#define SY5501_VTK_IBATOC_TRX_SET_REG               (0x50)
/* macro for SY5501_VTK_IBATOC_TRX_SET_REG Reg and ver_trx_od Bit, set WPEn0_DISA & WPEn1_DISA to disa Write protection */
//set VIN open drain or push pull.
#define VER_TRX_OD_BIT_MARK                         (0x80)
#define TRX_PIN_SEL_OPENDRAIN                       (0x1<<7)
#define TRX_PIN_SEL_PUSHPULL                        (0x0<<7)
/* macro for SY5501_VTK_IBATOC_TRX_SET_REG Reg and ver_trx_en Bi, set WPEn0_DISA & WPEn1_DISA to disa Write protection */
//set the way of into TRX: auto or VIN_CMD.
#define VER_TRX_EN_BIT_MARK                         (0x40)
#define TRX_EN_AUTO                                 (0x1<<6)
#define TRX_EN_BY_VIN_CMD                           (0x0<<6)
/* macro for SY5501_VTK_IBATOC_TRX_SET_REG Reg and ver_bat_oc[1:0] Bit, set WPEn0_DISA & WPEn1_DISA to disa Write protection */   
//set battery over current val.
#define VER_BAT_OC_BIT_MARK                         (0x30)
#define BAT_OVER_CURRENT_100MA                      (0x0<<4)
#define BAT_OVER_CURRENT_50MA                       (0x1<<4) 
#define BAT_OVER_CURRENT_200MA                      (0x2<<4)
#define BAT_OVER_CURRENT_150MA                      (0x3<<4)
/* macro for SY5501_VTK_IBATOC_TRX_SET_REG Reg and ver_cmd_freq[1:0] Bit, set WPEn0_DISA & WPEn1_DISA to disa Write protection */ 
//set the "T" time length of VIN_CMD, "T" precision:-5% ~ +5%. 
//VIN_CMD = "S code" + "sync code" + 8bit cmd + "stop code".
//"S code" = [n(n>1)"T" L lvl] + [n(n>1)"T" H lvl].
//"sync code" = [1"T" H lvl] + [16"T" L lvl].
//cmd data: 0 = [1"T" H lvl] + [3"T" L lvl], 1 = [3"T" H lvl] + [1"T" L lvl].
//"stop code" = [8"T" H lvl].
#define VER_CMD_FREQ_BIT_MARK                       (0x0c)
#define CMD_FREQ_100US                              (0x0<<2)
#define CMD_FREQ_1MS                                (0x1<<2)
#define CMD_FREQ_10MS                               (0x2<<2)
#define CMD_FREQ_20MS                               (0x3<<2)
/* macro for SY5501_VTK_IBATOC_TRX_SET_REG Reg and ver_vsys Bit, set WPEn0_DISA & WPEn1_DISA to disa Write protection */
//set VSYS status in ship mode: shut off VSYS or open drain.
#define VER_VSYS_BIT_MARK                           (0x02)
#define SHIPMODE_SET_VSYS_SHUT                      (0x0<<1)
#define SHIPMODE_SET_VSYS_OPENDRAIN                 (0x1<<1)
/* macro for SY5501_VTK_IBATOC_TRX_SET_REG Reg and ver_vtrk Bit, set WPEn0_DISA & WPEn1_DISA to disa Write protection */
//set the voltage threshold of trickle charge to const charge.
#define VER_VTRK_BIT_MARK                           (0x01)
#define VTRK_SEL_2P7V                               (0x0<<0)
#define VTRK_SEL_2P9V                               (0x1<<0)


#define SY5501_NTC_VIBATF_OD_SET_REG                (0x60)
/* macro for SY5501_NTC_VIBATF_OD_SET_REG Reg and ver_iend Bit, set WPEn0_DISA & WPEn1_DISA to disa Write protection */
//set end charge current.
#define VER_IEND_BIT_MARK                           (0x20)
#define IEND_SEL_TEN_PERCENT_ICC                    (0x0<<5)
#define IEND_SEL_FIVE_PERCENT_ICC                   (0x1<<5)
/* macro for SY5501_NTC_VIBATF_OD_SET_REG Reg and ver_bat_od Bit, set WPEn0_DISA & WPEn1_DISA to disa Write protection */
//set the voltage threshold of VBAT over discharge.
#define VER_BAT_OD_BIT_MARK                         (0x10)
#define BAT_OVER_DISCHARGE_SEL_2P5V                 (0x0<<4)
#define BAT_OVER_DISCHARGE_SEL_2P8V                 (0x1<<4)
/* macro for SY5501_NTC_VIBATF_OD_SET_REG Reg and ver_batf[1:0] Bit, set WPEn0_DISA & WPEn1_DISA to disa Write protection */ 
//set the voltage threshold of float charge.
#define VER_BATF_BIT_MARK                           (0x0c)
#define BATF_SEL_4P20V                              (0x0<<2)
#define BATF_SEL_4P35V                              (0x1<<2)
#define BATF_SEL_4P40V                              (0x2<<2)
#define BATF_SEL_4P45V                              (0x3<<2)
/* macro for SY5501_NTC_VIBATF_OD_SET_REG Reg and ver_ntc[1:0] Bit, set WPEn0_DISA & WPEn1_DISA to disa Write protection */ 
//set ntc protection mode: JEITA, charge or discharge ntc protection, whether output temp data.
#define VER_NTC_BIT_MARK                            (0x03)
#define NTC_SET_JEITA_ONLY_CHARGE                   (0x0<<0)
#define NTC_SET_JEITA_CHARGE_DISCHARGE              (0x1<<0)
#define NTC_SET_ONLY_OUTPUT_TEMP_DATA               (0x2<<0)
#define NTC_SET_OFF                                 (0x3<<0)

        
#define SY5501_ITK_VRECG_ICC_SET_REG                (0x61) 
/* macro for SY5501_ITK_VRECG_ICC_SET_REG Reg and ver_icc[2:0] Bit, set WPEn0_DISA & WPEn1_DISA to disa Write protection */
//set the current value of const charge.
#define VER_ICC_BIT_MARK                            (0xe0)
#define ICC_SEL_TEN_PERCENT_ICH                     (0x0<<5)
#define ICC_SEL_TWENTY_PERCENT_ICH                  (0X1<<5)
#define ICC_SEL_THIRTY_PERCENT_ICH                  (0x2<<5)
#define ICC_SEL_FIFTY_PERCENT_ICH                   (0x3<<5)
#define ICC_SEL_ONE_HUNDRED_PERCENT_ICH             (0x4<<5)
#define ICC_SEL_ONE_HUNDRED_AND_FIFTY_PERCENT_ICH   (0x5<<5)
#define ICC_SEL_TWO_HUNDRED_PERCENT_ICH             (0x6<<5)
#define ICC_SEL_THREE_HUNDRED_PERCENT_ICH           (0x7<<5)
/* macro for SY5501_ITK_VRECG_ICC_SET_REG Reg and ver_jeit_batf Bit, set WPEn0_DISA & WPEn1_DISA to disa Write protection */
/* set ntc float charge voltage.
   @ JEIT_BATF_SEL_VER_BATF:
     normal temp: voltage = SY5501_NTC_VIBATF_OD_SET_REG Reg ver_batf[1:0] Bit seting(BATF_SEL_xx). 
     temp > 45Â°: if (SY5501_NTC_VIBATF_OD_SET_REG Reg ver_ntc[1:0] Bit seting == JEITA) voltage = 4.05V.
   @ JEIT_BATF_SEL_4P05V: = 4.05V. */
#define VER_JEIT_BATF_BIT_MARK                      (0x10)
#define JEIT_BATF_SEL_VER_BATF                      (0x0<<4)
#define JEIT_BATF_SEL_4P05V                         (0x1<<4) //4.05V in any temp.
/* macro for SY5501_ITK_VRECG_ICC_SET_REG Reg and ver_vrecg[2:0] Bit, set WPEn0_DISA & WPEn1_DISA to disa Write protection */
//set the voltage threshold of trigger recharge requst.
#define VER_VRECG_BIT_MARK                          (0x0e)
#define VRECG_SEL_4P10V                             (0x0<<1)
#define VRECG_SEL_4P00V                             (0x1<<1)
#define VRECG_SEL_3P90V                             (0x2<<1)
#define VRECG_SEL_3P80V                             (0x3<<1)
#define VRECG_SEL_3P70V                             (0x4<<1)
#define VRECG_SEL_3P55V                             (0x5<<1)
#define VRECG_SEL_4P30V                             (0x6<<1)
#define VRECG_SEL_4P20V                             (0x7<<1)
/* macro for SY5501_ITK_VRECG_ICC_SET_REG Reg and ver_itrk Bit, set WPEn0_DISA & WPEn1_DISA to disa Write protection */
//set the current value of trickle charge.
#define VER_ITRK_BIT_MARK                           (0x01)
#define ITRK_SEL_TEN_PERCENT_ICC                    (0x0<<0) 
#define ITRK_SEL_FIVE_PERCENT_ICC                   (0x1<<0)


#define SY5501_CHGTO_ODVer_PTVer_REG                (0x62)
/* macro for SY5501_CHGTO_ODVer_PTVer_REG Reg and ver_TRXTimeOut Bit, set WPEn0_DISA & WPEn1_DISA to disa Write protection */
//whether pullup VIN 16ms when TRX timeout.
#define VER_TRXTIMEOUT_BIT_MARK                     (0x80)
#define TRX_TIMEOUT_PULLUP_VIN_16MS                 (0x1<<7)
#define TRX_DISA_TIMEOUT                            (0x0<<7)
/* macro for SY5501_CHGTO_ODVer_PTVer_REG Reg and ver_TRXCaseInDet Bit, set WPEn0_DISA & WPEn1_DISA to disa Write protection */
//inbox detection en or disa in TRX mode.
#define VER_TRXCASEINDET_BIT_MARK                   (0x40)
#define EN_INBOX_DET_IN_TRX_MODE                    (0x1<<6)
#define DISA_INBOX_DET_IN_TRX_MODE                  (0x0<<6)
/* macro for SY5501_CHGTO_ODVer_PTVer_REG Reg and ver_UnChgNTCDet Bit, set WPEn0_DISA & WPEn1_DISA to disa Write protection */
//whether updata ntc temp in uncharge, if updata will trigger IRQ.
#define VER_UNCHGNTCDET_BIT_MARK                    (0x20)
#define UPDATA_NTC_TEMP_IN_UNCHARGE_EN              (0x1<<5)
#define UPDATA_NTC_TEMP_IN_UNCHARGE_DISA            (0x0<<5)
/* macro for SY5501_CHGTO_ODVer_PTVer_REG Reg and ver_watchdog Bit, set WPEn0_DISA & WPEn1_DISA to disa Write protection */
//if (SY5501_IO_CaseChk_SET_REG Reg and ver_rst_func Bit seting == INBOX_SEND_RST_EN), ver_watchdog Bit Write 1 and then 0,
//the cnt of ver_rst_delay[2:0] will clear.
#define VER_WATCHDOG_BIT_MARK                       (0x10)
#define VER_WATCHDOG_BIT_WRITE_1                    (0x1<<4)
#define VER_WATCHDOG_BIT_WRITE_0                    (0x0<<4)
/* macro for SY5501_CHGTO_ODVer_PTVer_REG Reg and ver_prt Bit, set WPEn0_DISA & WPEn1_DISA to disa Write protection */
//set OCP NTC protection mode:
#define VER_PRT_BIT_MARK                            (0x08)
#define OCP_NTC_PROTECT_SEL_UNLOCK                  (0x0<<3) //unlock: 2s hiccups
#define OCP_NTC_PROTECT_SEL_LOCK                    (0x1<<3) //lock: input VIN to clear lock.
/* macro for SY5501_CHGTO_ODVer_PTVer_REG Reg and ver_prt_od Bit, set WPEn0_DISA & WPEn1_DISA to disa Write protection */ 
//set battery over discharge protection mode: whether auto restore.
#define VER_PRT_OD_BIT_MARK                         (0x04)
#define BAT_OVER_DISCHARGE_PROTECT_SEL_UNLOCK       (0x0<<2) //unlock: auto restore.
#define BAT_OVER_DISCHARGE_PROTECT_SEL_LOCK         (0x1<<2) //lock: input VIN to clear lock
/* macro for SY5501_CHGTO_ODVer_PTVer_REG Reg and ver_timer Bit, set WPEn0_DISA & WPEn1_DISA to disa Write protection */ 
//set the constant current charging timeout.
#define VER_TIMER_BIT_MARK                          (0x02)
#define CONST_CHARGE_TIMEOUT_SEL_3H                 (0x0<<1)
#define CONST_CHARGE_TIMEOUT_SEL_6H                 (0x1<<1)
/* macro for SY5501_CHGTO_ODVer_PTVer_REG Reg and ver_timer_en Bit, set WPEn0_DISA & WPEn1_DISA to disa Write protection */
//set charge timeout protection enable.
#define VER_TIMER_EN_BIT_MARK                       (0x01)
#define CHARGE_TIMEOUT_EN                           (0x1<<0) //trickle charge timeout = 1H; const charge timeout = SY5501_CHGTO_ODVer_PTVer_REG Reg  ver_timer Bit seting(CONST_CHARGE_TIMEOUT_SEL_xx)
#define CHARGE_TIMEOUT_DISA                         (0x0<<0)

  
/* macro for SY5501_WPEn0_REG & SY5501_WPEn1_REG Reg */
/* Write SY5501_VTK_IBATOC_TRX_SET_REG, SY5501_NTC_VIBATF_OD_SET_REG, SY5501_ITK_VRECG_ICC_SET_REG,
    SY5501_CHGTO_ODVer_PTVer_REG must set WPEn0_DISA & WPEn1_DISA !!!!!!! */
#define SY5501_WPEn0_REG                            (0x72)
#define WPEn0_EN                                    (0x00)
#define WPEn0_DISA                                  (0x97)
#define SY5501_WPEn1_REG                            (0x73)
#define WPEn1_EN                                    (0x00)
#define WPEn1_DISA                                  (0x39)
//STOP


//FUNTION Start
void sy5501_init(uint32_t irq_handler);
bool sy5501_get_init_status(void);

uint8_t sy5501_read_st_case(void);
uint8_t sy5501_read_state0_reg(void);

uint8_t sy5501_read_state2_reg(void);
void sy5501_write_state2_reg(uint8_t data);


bool sy5501_write_i2c_cmd_reg(uint8_t write_data);
bool sy5501_set_charger_current(uint8_t charge_current);

bool sy5501_read_vin_ok(void);

void sy5501_reg_printf(void);


void sy5501_enter_shipmode_before_shutdown(void);
void sy5501_enter_single_line_download(void);


#ifdef __cplusplus
}
#endif
#endif /* __MCU_SY5501_H__ */
