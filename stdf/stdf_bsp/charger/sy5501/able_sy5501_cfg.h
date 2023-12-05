#ifndef __ABLE_SY5501_CFG_H__
#define __ABLE_SY5501_CFG_H__

#ifdef __cplusplus
extern "C" {
#endif

#define SY5501_VERSION_A    (0)
#define SY5501_VERSION_C    (1)

#define SY5501_VERSION      SY5501_VERSION_C
/********* SY5501 Reg init, according <<SY5501_Ver0.0.6.PDF>> *********/
/* 100us -0x00  1ms -0x04 10ms-0x08  20ms-0x0c */
#define CMD_FREQ                            (CMD_FREQ_1MS) 

#define TRICKLE_CHARGE_MODE_ICC                     (ICC_SEL_THIRTY_PERCENT_ICH)
#define LOW_CHARGE_MODE_ICC                         (ICC_SEL_FIFTY_PERCENT_ICH)
#define QUICKLY_CHARGE_MODE_ICC                     (ICC_SEL_FIFTY_PERCENT_ICH)
#define NEAR_FULL_CHARGE_MODE_ICC                   (ICC_SEL_FIFTY_PERCENT_ICH)

//SY5501_WPEn0_REG <0x72> & SY5501_WPEn1_REG <0x73> init val
#define SY5501_WPEn0_REG_INIT                       (WPEn0_DISA)
#define SY5501_WPEn1_REG_INIT                       (WPEn1_DISA)
//SY5501_IO_CaseChk_SET_REG <0x40> init val
#define SY5501_IO_CaseChk_SET_REG_INIT              (EXIT_SHIPMODE_SEND_PWK_DISA|INBOX_AN_DISA|INBOX_DET_EN|\
                                                     PWK_RST_TRX_PIN_EN|RST_EFF_LVL_SEL_H|INBOX_SEND_RST_DISA|\
                                                     PWK_EFF_LVL_SEL_L|OUTBOX_SEND_PWK_DISA)
//SY5501_RST_Delay_SET_REG <0x41> init val                                                     
#define SY5501_RST_Delay_SET_REG_INIT               (IRQ_WIDTH_8MS|TRK_DISA_VSYS|RST_WIDTH_500MS|RST_DELAY_2S|\
                                                     SHIPMODE_SHUT_VSYS_DELAY_4S)
//SY5501_PWK_SET_REG <0x42> init val                                                     
#define SY5501_PWK_SET_REG_INIT                     (PWK_INTERVAL_100MS|PWK_LONG_2S|PWK_SHORT_EFF_TIM_200MS|\
                                                     PWK_SLONG_EFF_TIM_4S)
//SY5501_VinPullDown_SET_REG <0x43> init val                                                     
#define SY5501_VinPullDown_SET_REG_INIT             (RECG_PULLDOWN_VIN_DISA|INBOX_PULLDOWN_VIN_EN) 
//SY5501_VTK_IBATOC_TRX_SET_REG <0x50> init val
#define SY5501_VTK_IBATOC_TRX_SET_REG_INIT          (TRX_PIN_SEL_OPENDRAIN|TRX_EN_BY_VIN_CMD|BAT_OVER_CURRENT_100MA|CMD_FREQ_1MS|\
                                                     SHIPMODE_SET_VSYS_SHUT|VTRK_SEL_2P7V)
//SY5501_NTC_VIBATF_OD_SET_REG <0x60> init val                                          
#define SY5501_NTC_VIBATF_OD_SET_REG_INIT           (IEND_SEL_FIVE_PERCENT_ICC|BAT_OVER_DISCHARGE_SEL_2P8V|\
                                                     BATF_SEL_4P20V|NTC_SET_OFF)
//SY5501_ITK_VRECG_ICC_SET_REG <0x61> init val                                                 
#define SY5501_ITK_VRECG_ICC_SET_REG_INIT           (ICC_SEL_FIFTY_PERCENT_ICH|JEIT_BATF_SEL_VER_BATF|VRECG_SEL_4P10V|\
                                                     ITRK_SEL_TEN_PERCENT_ICC) 
//SY5501_CHGTO_ODVer_PTVer_REG <0x62> init val
#define SY5501_CHGTO_ODVer_PTVer_REG_INIT           (OCP_NTC_PROTECT_SEL_LOCK|BAT_OVER_DISCHARGE_PROTECT_SEL_LOCK|\
                                                     CONST_CHARGE_TIMEOUT_SEL_3H|CHARGE_TIMEOUT_EN)

#ifdef __cplusplus
}
#endif
#endif /* __MCU_SY5501_INIT_H__ */
