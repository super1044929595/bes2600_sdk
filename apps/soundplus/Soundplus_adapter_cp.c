#include <assert.h>
#include "SpEcComDef.h"
#include "Soundplus_adapter.h"
#include "SpEcCfgParaSt.h"
#include "sndp_lib.h"

#if defined _MSC_VER || defined __ICCARM__
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#define med_malloc malloc
#define med_free free
typedef unsigned char               bool;
#define true                        1
#define false                       0
#define TRACE printf
#else
#include "heap_api.h"
#include "hal_trace.h"
#endif

static void *pBuff_TX1 = NULL;
static void *pBuff_TX2 = NULL;
static void *pBuff_TX3 = NULL;

//EQ Table Config 
float gc_Tx_FdEQ_Tbl_16k[SPX_FrmLen_16k / 2 + 1] = {
	 0.0f,0.0f,0.0f,0.0f,1.0f,1.0f,1.0f,1.0f,1.0f,1.0f,1.0f,1.0f,1.0f,1.0f,1.0f,1.0f,
	 1.0f,1.0f,1.0f,1.0f,1.0f,1.0f,1.0f,1.0f,1.0f,1.0f,1.0f,1.0f,1.0f,1.0f,1.0f,1.0f,
	 1.0f,1.0f,1.0f,1.0f,1.0f,1.0f,1.0f,1.0f,1.0f,1.0f,1.0f,1.0f,1.0f,1.0f,1.0f,1.0f,
	 1.0f,1.0f,1.0f,1.0f,1.0f,1.0f,1.0f,1.0f,1.0f,1.0f,1.0f,1.0f,1.0f,1.0f,1.0f,1.0f,
	 1.0f,1.0f,1.0f,1.0f,1.0f,1.0f,1.0f,1.0f,1.0f,1.0f,1.0f,1.0f,1.0f,1.0f,1.0f,1.0f,
	 1.0f,1.0f,1.0f,1.0f,1.0f,1.0f,1.0f,1.0f,1.0f,1.0f,1.0f,1.0f,1.0f,1.0f,1.0f,1.0f,
	 1.0f,1.0f,1.0f,1.0f,1.0f,1.0f,1.0f,1.0f,1.0f,1.0f,1.0f,1.0f,1.0f,1.0f,1.0f,1.0f,
	 1.0f,1.0f,1.0f,1.0f,1.0f,1.0f,1.0f,1.0f,1.0f,1.0f,1.0f,1.0f,1.0f,1.0f,1.0f,1.0f,
	 1.0f,1.0f,1.0f,1.0f,1.0f,1.0f,1.0f,1.0f,1.0f,1.0f,1.0f,1.0f,1.0f,1.0f,1.0f,1.0f,
	 1.0f,1.0f,1.0f,1.0f,1.0f,1.0f,1.0f,1.0f,1.0f,1.0f,1.0f,1.0f,1.0f,1.0f,1.0f,1.0f,
	 1.0f,1.0f,1.0f,1.0f,1.0f,1.0f,1.0f,1.0f,1.0f,1.0f,1.0f,1.0f,1.0f,1.0f,1.0f,1.0f,
	 1.0f,1.0f,1.0f,1.0f,1.0f,1.0f,1.0f,1.0f,1.0f,1.0f,1.0f,1.0f,1.0f,1.0f,1.0f,1.0f,
	 1.0f,1.0f,1.0f,1.0f,1.0f,1.0f,1.0f,1.0f,1.0f,1.0f,1.0f,1.0f,1.0f,1.0f,1.0f,1.0f,
	 1.0f,1.0f,1.0f,1.0f,1.0f,1.0f,1.0f,1.0f,1.0f,1.0f,1.0f,1.0f,1.0f,1.0f,1.0f,1.0f,
	 1.0f,1.0f,1.0f,1.0f,1.0f,1.0f,1.0f,1.0f,1.0f,1.0f,1.0f,1.0f,1.0f,1.0f,1.0f,1.0f,
	 1.0f,1.0f,1.0f,1.0f,1.0f,1.0f,1.0f,1.0f,1.0f,1.0f,1.0f,1.0f,1.0f,1.0f,1.0f,1.0f,
	 1.0f
};
float gc_Tx_FdEQ_Tbl_8k[SPX_FrmLen_8k / 2 + 1] = {
	0.0f,0.0f,0.0f,0.0f,1.0f,1.0f,1.0f,1.0f,1.0f,1.0f,1.0f,1.0f,1.0f,1.0f,1.0f,1.0f,
	1.0f,1.0f,1.0f,1.0f,1.0f,1.0f,1.0f,1.0f,1.0f,1.0f,1.0f,1.0f,1.0f,1.0f,1.0f,1.0f,
	1.0f,1.0f,1.0f,1.0f,1.0f,1.0f,1.0f,1.0f,1.0f,1.0f,1.0f,1.0f,1.0f,1.0f,1.0f,1.0f,
	1.0f,1.0f,1.0f,1.0f,1.0f,1.0f,1.0f,1.0f,1.0f,1.0f,1.0f,1.0f,1.0f,1.0f,1.0f,1.0f,
	1.0f,1.0f,1.0f,1.0f,1.0f,1.0f,1.0f,1.0f,1.0f,1.0f,1.0f,1.0f,1.0f,1.0f,1.0f,1.0f,
	1.0f,1.0f,1.0f,1.0f,1.0f,1.0f,1.0f,1.0f,1.0f,1.0f,1.0f,1.0f,1.0f,1.0f,1.0f,1.0f,
	1.0f,1.0f,1.0f,1.0f,1.0f,1.0f,1.0f,1.0f,1.0f,1.0f,1.0f,1.0f,1.0f,1.0f,1.0f,1.0f,
	1.0f,1.0f,1.0f,1.0f,1.0f,1.0f,1.0f,1.0f,1.0f,1.0f,1.0f,1.0f,1.0f,1.0f,1.0f,1.0f,
	1.0f
};

//FB parameters config
#if(EC_BCMicEn>=1)
//EQ Config
int gc_EQ_QValue = 8;
float gc_Tx_BcMicEQ_Tbl[EQ_ForANC_ModeNum][EQ_BcSigUppBin] = {
	{
		2334	,            2328	,            2322	,            2318	,            2314	,            2311	,            2308	,            2306	,            2305	,            2304	,            2303	,            2302	,            2302	,            2302	,            2302	,            2340	,
		2449	,            2619	,            2842	,            3108	,            3410	,            3739	,            4085	,            4440	,            4794	,            5140	,            5469	,            5771	,            6038	,            6260	,            6431	,            6539	,
		6577	,            6576	,            6571	,            6565	,            6557	,            6547	,            6536	,            6525	,            6514	,            6504	,            6494	,            6486	,            6479	,            6475	,            6474	,            6523	,
		6664	,            6882	,            7159	,            7486	,            7856	,            8257	,            8682	,            9121	,            9565	,           10007	,           10441	,           10860	,           11259	,           11636	,           11986	,           12309	,
		12605	,           12933	,           13333	,           13778	,           14244	,           14707	,           15146	,           15540	,           15870	,           16121	,           16279	,           16332	,           16270	,           16086	,           15776	,           15336	,
	},
	{
		2334	,            2328	,            2322	,            2318	,            2314	,            2311	,            2308	,            2306	,            2305	,            2304	,            2303	,            2302	,            2302	,            2302	,            2302	,            2340	,
		2449	,            2619	,            2842	,            3108	,            3410	,            3739	,            4085	,            4440	,            4794	,            5140	,            5469	,            5771	,            6038	,            6260	,            6431	,            6539	,
		6577	,            6576	,            6571	,            6565	,            6557	,            6547	,            6536	,            6525	,            6514	,            6504	,            6494	,            6486	,            6479	,            6475	,            6474	,            6523	,
		6664	,            6882	,            7159	,            7486	,            7856	,            8257	,            8682	,            9121	,            9565	,           10007	,           10441	,           10860	,           11259	,           11636	,           11986	,           12309	,
		12605	,           12933	,           13333	,           13778	,           14244	,           14707	,           15146	,           15540	,           15870	,           16121	,           16279	,           16332	,           16270	,           16086	,           15776	,           15336	,
	},
	{
		2334	,            2328	,            2322	,            2318	,            2314	,            2311	,            2308	,            2306	,            2305	,            2304	,            2303	,            2302	,            2302	,            2302	,            2302	,            2340	,
		2449	,            2619	,            2842	,            3108	,            3410	,            3739	,            4085	,            4440	,            4794	,            5140	,            5469	,            5771	,            6038	,            6260	,            6431	,            6539	,
		6577	,            6576	,            6571	,            6565	,            6557	,            6547	,            6536	,            6525	,            6514	,            6504	,            6494	,            6486	,            6479	,            6475	,            6474	,            6523	,
		6664	,            6882	,            7159	,            7486	,            7856	,            8257	,            8682	,            9121	,            9565	,           10007	,           10441	,           10860	,           11259	,           11636	,           11986	,           12309	,
		12605	,           12933	,           13333	,           13778	,           14244	,           14707	,           15146	,           15540	,           15870	,           16121	,           16279	,           16332	,           16270	,           16086	,           15776	,           15336	,
	},
};


//FCB Config
s_Cfg_SpEc_FCBProc gc_FCBInit = { 128, 0.02f, 0.0f, 32.0f, 1, 1.0f, 1.0f, };

//Mix Config
float gc_NoiseRMSdB_TH0 = 65 + 30;
//float gc_NoiseRMSdB_TH0 = 65 + 20;
float gc_CTRLMIX_SNR_TH1 = 9.0f;
float gc_CTRLMIX_SNR_TH0 = 3.0f;

//FB AEC config
float gc_StatMinGain = 0.17f;//0.17f;//15dB  0.1f//20dB //0.3f//10dB
float gc_EchoWeight = 16.0f;

#endif

//Dual mic paramters config
#if(EC_ACMicNum==2)

float gc_micD = 0.018f;
s_Cfg_SpEc_FCBProc gc_BlkInit = { 64, 0.05f, 0.0f, 4.0f, 1, 1.0f, 0.75f, };
s_Cfg_SpEc_FCBProc gc_AncInit = { 256, 0.1f, 0.5f, 0.5f, 0, 16.0f, 0.0f, };
/*startAngle:EndAngel:transitionAngleBan:Degree*/
float  gc_BeamAngleSet[3] = { 0.0f,60.0f,30.0f };
float gc_BF_SpaFilterInit[BF_SpaFilter_InitTap] = {
	0.42418823f,	0.33681643f,	0.14250951f,	-0.00764578f,	-0.04764903f,	-0.02139775f,	0.08305049f,	-0.08081642f,	0.01529786f,	-0.05091872f,	0.04536784f,	0.01210606f,	0.02588036f,	-0.04528622f,	0.02248913f,	0.00393536f,
    -0.00854540f,	-0.02496080f,	-0.00008572f,	0.00651104f,	-0.01224595f,	-0.00019235f,	0.01029829f,	-0.01863528f,	0.00866312f,	0.00641130f,	0.00367483f,	-0.01187460f,	-0.01929421f,	-0.00752357f,	0.02000479f,	-0.00325514f,
};
#endif 


//float DNN_PrePro_Gain = 0.06f; //must be <1.0f   0.1778f //-15dB //0.1f //-20dB //0.0316f //-30dB //0.01f //-40dB //0.06f //-25dB
float tx_drc_pregain = 1.0f;

static bool soundplus_inited = false;

void soundplus_deal_Tx(short* out, float* in, float* in_echos, int AncFlag, int numsamples)
{
	int i;
    float tmpAbs;

    SndpEC_Tx_MMicSpxEhnFrame(in, in, in_echos, NULL, AncFlag);

	for(i=0; i<numsamples; i++)
	{
		tmpAbs = in[i];
		if (tmpAbs < -32768) tmpAbs = -32768;
		else if (tmpAbs > 32767) tmpAbs = 32767;
        out[i] = (short)tmpAbs;
	}
	return;
}

int soundplus_init_cp(int NrwFlag)
{
    int size1=0, size2=0, size3=0;
    
    if (soundplus_inited) {return -1;}
	// memory init, get number, malloc memory
	SndpEc_TxSpxEnh_MemBufSizeInBytes_cp(&size1, &size2, &size3);
	pBuff_TX1 = (void *)med_malloc(size1);
	if(pBuff_TX1 == NULL) {return -1;}
    pBuff_TX2 = (void *)med_malloc(size2);
	if(pBuff_TX2 == NULL) {return -1;}
    pBuff_TX3 = (void *)med_malloc(size3);
	if(pBuff_TX3 == NULL) {return -1;}

	SndpEc_TxSpxEnh_Init_cp(pBuff_TX1, pBuff_TX2, pBuff_TX3, NrwFlag);
    
    soundplus_inited = true;

	return 0;
}

int soundplus_deinit_cp(void)
{
	if(pBuff_TX1)
    {   
	    med_free((void *)pBuff_TX1);
        pBuff_TX1 = NULL;
    }
    if(pBuff_TX2)
    {   
	    med_free((void *)pBuff_TX2);
        pBuff_TX2 = NULL;
    }
    if(pBuff_TX3)
    {   
	    med_free((void *)pBuff_TX3);
        pBuff_TX3 = NULL;
    }
    
    soundplus_inited = false;
	return 0;
}
