#ifndef _SP_ECCOMDEF_H
#define _SP_ECCOMDEF_H

#ifdef __cplusplus
extern "C" {
#endif

//channel Pre-Define 
#define EC_ACMicNum    (2)   //Number of  Air conduct mic <=2
#define EC_BCMicEn     (0)   //FB mic enable Flag 0/1  
#define EC_BCGenVpuEn  (0)   //WB Gsensor(VPU) enable Flag 0/1
#define TOT_ChnNum     (EC_ACMicNum+EC_BCMicEn+EC_BCGenVpuEn)

//DAC bitLength Pre-Define:support 16bit INPUT
#define SPX_0dBLevel   (32768)

//Fs Pre-Define:support 8k & 16kHz Fs
#define SPX_Fs_16K     (16000) //just support 16k data
#define SPX_Fs_8K      (8000)  //just support 8k data

//Frm shift Pre-Define:support the max Frame Shift and Frame Len
#define SPX_FrmShf_16k (240)
#define SPX_FrmLen_16k (512)
#define SPX_OlaLen_16k (SPX_FrmLen_16k-SPX_FrmShf_16k)

#define SPX_FrmShf_8k  (120)
#define SPX_FrmLen_8k  (256)
#define SPX_OlaLen_8k  (SPX_FrmLen_8k-SPX_FrmShf_8k)

//EPS Pre-Define
#define EPS_MinPwr          (SPX_0dBLevel*SPX_0dBLevel*1.0e-8f)   //-80dB

//AEC Pre-Define 
#define AEC_MaxTaps         (128)
#define AEC_InitCoefs_Taps  (16)
#define NumOfACMic_AEC      TOT_ChnNum

//the number of ANC mode supporting
#define EQ_ForANC_ModeNum   (3)
//the upper bin of FB mic signal 
#define EQ_BcSigUppBin      (80)  // 2500Hz ///(24) //750Hz ///(32) //1000Hz  ///(80)  // 2500Hz

//the upper bin of VPU or gsensor signal
#define EQ_BCGenVpuSigUppBin   32

//PRE_Processing Enable
#define DNN_PrePro_Enable   (1)
#define DNN_PrePro_Gain     (0.3f)  //must be <1.0f and >0.1f

//Dual Mic Beamforming Supporting 
#define BF_SpaFilter_InitTap  (32)
#if ((EC_BCMicEn == 1) && (EC_ACMicNum >= 1))
#define DM_AdaptiveBeamforming_Enable (1)
#else
#define DM_AdaptiveBeamforming_Enable (0)
#endif

#define Sndp_Rx_Enable   (0)
#define Sndp_Bf_Enable   (0)
#define Sndp_WindModule_Enable   (0)

//#define DM_ONLYMixWhenWindNoise  //Always disabled if EC_ACMicNum!=2

//RX Process Support
//#define SP_RX_SUPPORT 1
	
#define C_SOUND    (340)  //m/s
#define MY_PI      (3.1415926535897932f)
#define MY_TWO_PI  (2*MY_PI)

#ifdef __cplusplus
}
#endif

#endif
