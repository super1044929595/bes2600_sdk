#ifndef _SP_ECCFGPARA_H
#define _SP_ECCFGPARA_H

#include "SpEcComDef.h"
#ifdef __cplusplus
extern "C" {
#endif
#if (NumOfACMic_AEC >= 1)
typedef struct s_Cfg_SpEc_TxAec {
		int rtTap[NumOfACMic_AEC];		
		float mu;
		float weight;
		float para0;
		float InitIR[NumOfACMic_AEC][AEC_InitCoefs_Taps];
} s_Cfg_SpEc_TxAec;
#endif //#if (NumOfACMic_AEC >= 1)
typedef struct s_Cfg_SpEc_FCBProc {
		int tap;
		float mu;
		float alpha;
		float weight;
		int Flag;
		float para0;
		float para1;
} s_Cfg_SpEc_FCBProc;


//Config
extern float gc_Tx_FdEQ_Tbl_16k[];
extern float gc_Tx_FdEQ_Tbl_8k[];
extern float gc_Rx_FdEQ_Tbl_16k[];
extern float gc_Rx_FdEQ_Tbl_8k[];


#if (NumOfACMic_AEC>=1)
	extern s_Cfg_SpEc_TxAec gc_Para_TxAecInit;
#endif 
	
#if(EC_BCMicEn==1)
	extern float gc_Tx_BcMicEQ_Tbl[][EQ_BcSigUppBin];
//	extern float gc_Ff_Fb_GainRatio;
	extern float gc_NoiseRMSdB_TH0;
	extern float gc_CTRLMIX_SNR_TH1;
	extern float gc_CTRLMIX_SNR_TH0;
	extern int gc_EQ_QValue;
	extern s_Cfg_SpEc_FCBProc gc_FCBInit;

	extern float  gc_StatMinGain;
	extern float gc_EchoWeight;
#endif

#if(EC_ACMicNum==2)

	extern float gc_micD;
	extern s_Cfg_SpEc_FCBProc gc_BlkInit;
	extern s_Cfg_SpEc_FCBProc gc_AncInit;
	/*startAngle:EndAngel:transitionAngleBan*/
	extern float gc_BeamAngleSet[3];
	extern float gc_BF_SpaFilterInit[BF_SpaFilter_InitTap];
#endif 

#if(EC_BCMicEn>=1 || EC_BCGenVpuEn==1) //wzy@20210619-3files-from-qfh-wzq
	extern float gc_NoiseRMSdB_TH0;
	extern float gc_CTRLMIX_SNR_TH1;
	extern float gc_CTRLMIX_SNR_TH0;
	extern float  gc_StatMinGain;
	extern float gc_EchoWeight;
#endif 
	extern float tx_drc_pregain;
    extern float rx_drc_pregain;

#ifdef __cplusplus
}
#endif

#endif
