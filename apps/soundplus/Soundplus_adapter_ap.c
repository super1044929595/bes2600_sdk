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

//assume the  SYS_DELAY >= SYS_DELAYNB
#define SYS_DELAY		(0)//(618)
#define SYS_DELAYNB		(0) //(313)
#define ALGN_REFBUFLEN	(SYS_DELAY + SPX_FrmShf_16k)

static bool soundplus_keyReadFlag = false;
static bool soundplus_inited = false;

static void *pBuff_SPeech = NULL;
static short *pRef_align = NULL;
static float *pMic_In = NULL;
static float *pRef_In = NULL;

#if Sndp_Rx_Enable
float gc_Rx_FdEQ_Tbl_16k[SPX_FrmLen_16k / 2 + 1] = {
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
float gc_Rx_FdEQ_Tbl_8k[SPX_FrmLen_8k / 2 + 1] = {
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
#endif

//Multi Mic AEC Config parameters
#if (NumOfACMic_AEC>=1)
s_Cfg_SpEc_TxAec  gc_Para_TxAecInit = {
	//int rtTap[NumOfACMic_AEC];
#if (NumOfACMic_AEC==3)
	{ 16,16,32 },
#endif 
#if(NumOfACMic_AEC == 2)
	{ 16,16},
#endif
#if(NumOfACMic_AEC == 1)
	{32},
#endif 
	//float mu; 
	0.0025f,
	//float weight;
	8.0f,
	//float para0;
	0.001f * SPX_0dBLevel,
	//float InitIR[NumOfACMic_AEC][AEC_InitCoefs_Taps];
	{
#if (NumOfACMic_AEC>=1)
		{ -0.009769431412852f,	0.030019789885092f,	-0.000275295219640f,-0.048215075406187f,
	-0.032622423104622f,	0.023096307576689f,	0.046112660831128f,	0.024357677139471f,
	-0.006741579831287f,	-0.018184717513480f,	-0.011833646820331f,	-0.004227456404502f,
	0.000958492523608f,	0.006414788407070f,	0.007232014020445f,	0.000235268727070f,},
#endif 
#if (NumOfACMic_AEC>=2)
	{ 
		-0.011944749590019f,	0.061485550122565f,	- 0.023964376981538f,	- 0.095661783969621f,
	- 0.026317314755128f,	0.070966600850295f,	0.065862436793061f,	0.006171652119993f,
	- 0.019332676416373f,	- 0.014860259535574f,	- 0.013367132542449f,	- 0.010072258572836f,
	0.003267390312793f,	0.012307143575926f,	0.008584229272519f,	- 0.000359146635318f,
	},
#endif 
#if (NumOfACMic_AEC>=3)
	{ 0.344627190831273f,	0.623373912455214f,	- 0.120677593274072f,	- 0.207476022460700f,
	0.180167325956923f,	0.195521160584488f,	- 0.020505144777988f,	0.003531432438528f,
	0.151856566341553f,	0.129833345805557f,	0.016760663536976f,	0.016333581784144f,
	0.083311882601881f,	0.067997926754794f,	0.002102852185666f,	0.012174515336261f },
#endif 
	},
};
#endif

int soundplus_auth(uint8_t* key, int Key_len)
{
    int ret = 0;

    if (soundplus_keyReadFlag == false)
    {
        ret = sndp_license_auth(key, Key_len);
        soundplus_keyReadFlag = true;
    }

    return ret;
}

int soundplus_auth_status()
{
    return sndp_license_status_get();
}

int soundplus_deal_Aec(float* out, float* out_echos, short* in, short* ref, int buf_len, int ref_len)
{
    int i;
    int numsamples = buf_len/ TOT_ChnNum;
    
	//align the ref and mic in order to erect the system delay
    if (ref_len == SPX_FrmShf_16k)
    {
        memmove(pRef_align, pRef_align + SPX_FrmShf_16k, sizeof(short) * SYS_DELAY);
        memcpy(pRef_align + SYS_DELAY, ref, sizeof(short) * SPX_FrmShf_16k);
    }
    else
    {
        memmove(pRef_align, pRef_align + SPX_FrmShf_8k, sizeof(short) * SYS_DELAYNB);
        memcpy(pRef_align + SYS_DELAYNB, ref, sizeof(short) * SPX_FrmShf_8k);
    }

    /*De-interleave*/
   /* for(i=0; i<numsamples; i++)
	{	
		for(int j=0;j<TOT_ChnNum;j++) pMic_In[numsamples*j + i] = in[TOT_ChnNum*i + j];
	}*/
#ifdef WIN32
    for (i = 0; i < numsamples; i++)
    {
        pMic_In[i] = in[TOT_ChnNum * i + 0];
#if (TOT_ChnNum >= 2) 
        pMic_In[numsamples + i] = in[TOT_ChnNum * i + 1];
#if (TOT_ChnNum >= 3) 
        pMic_In[numsamples * 2 + i] = in[TOT_ChnNum * i + 2];
#endif
#endif

    }
#else
    for (i = 0; i < numsamples; i++)
    {
        pMic_In[i] = in[TOT_ChnNum * i + 2];
#if (TOT_ChnNum >= 2) 
        pMic_In[numsamples + i] = in[TOT_ChnNum * i + 1];
#if (TOT_ChnNum >= 3) 
        pMic_In[numsamples * 2 + i] = in[TOT_ChnNum * i + 0];
#endif
#endif
    }
#endif

    for(i=0; i<ref_len; i++)
    {
        pRef_In[i] = pRef_align[i];
    }

	SndpEC_Tx_AecFrame(out, out_echos, pMic_In, pRef_In);

    return 0;
}

int soundplus_init_ap(int NrwFlag)
{
    if (soundplus_inited) {return -1;}
	// memory init, get number, malloc memory
	int memsize = SndpEc_TxSpxEnh_MemBufSizeInBytes_ap();
	//TRACE("<%s> memsize=%d version=%s built datetime=%s", __func__,memsize, sndp_get_lib_version(), sndp_get_lib_built_datetime());
	pBuff_SPeech = (void *)med_malloc(memsize);
	if(pBuff_SPeech == NULL) {return -1;}
    pMic_In = (float *)med_malloc(SPX_FrmShf_16k * TOT_ChnNum * sizeof(float));
    if(pMic_In == NULL) {return -1;}
    pRef_In = (float *)med_malloc(SPX_FrmShf_16k * sizeof(float));
    if(pRef_In == NULL) {return -1;}
    
    pRef_align = (short *)med_malloc(ALGN_REFBUFLEN * sizeof(short));
    if(pRef_align == NULL) {return -1;}
    
	SndpEc_TxSpxEnh_Init_ap(pBuff_SPeech, NrwFlag);
    
    soundplus_inited = true;

	return 0;
}

int soundplus_deinit_ap(void)
{
	if(pBuff_SPeech)
    {   
	    med_free((void *)pBuff_SPeech);
        pBuff_SPeech = NULL;
    }
    if(pMic_In)
    {   
	    med_free((void *)pMic_In);
        pMic_In = NULL;
    }
    if(pRef_In)
    {   
	    med_free((void *)pRef_In);
        pRef_In = NULL;
    }
    if(pRef_align)
    {   
	    med_free((void *)pRef_align);
        pRef_align = NULL;
    }

    soundplus_inited = false;
	return 0;
}

#if Sndp_WindModule_Enable
float winddetect_alpha = 0.93;
float winddetect_max = 0.7;
static void* pWindMembuf = NULL;
int soundplus_windmodule_init(void)
{
    // memory init, get number, malloc memory
    int memsize = Sndp_SpxEnh_WindModule_MemSize();
    //TRACE(2,"=========================> <%s> memsize = %d ....", __func__, memsize);
    pWindMembuf = med_malloc(memsize);
    if (pWindMembuf == NULL) { return -1; }
    pMic_In = (float *)med_malloc(240 * 2 * sizeof(float));
    if(pMic_In == NULL) {return -1;}

    Sndp_SpxEnh_WindModule_Init(pWindMembuf, memsize);

    return 0;
}

float soundplus_deal_windmodule_Tx(short* buf, int buf_len)
{
    int i;
    float phaseVar_standalone = 0;
    int numsamples = buf_len / 2;

    //TRACE("======================> [%s] takes begin........", __func__);
    //assert((numsamples == 240) || (numsamples == 120));

    for (i = 0; i < numsamples; i++)
    {
        pMic_In[i] = buf[2*i]; //talk
		pMic_In[numsamples + i] = buf[2*i+1]; //ff
    }

    phaseVar_standalone = Sndp_SpxEnh_WindModule_Tx(pMic_In);
    //TRACE(3,"======================> [%d] ", (int)(phaseVar_standalone*10000.0f));
    return phaseVar_standalone;
}

void soundplus_WindModule_deInit(void) {
    if (pWindMembuf)
    {
        med_free((void*)pWindMembuf);
        pWindMembuf = NULL;
    }
    if(pMic_In)
    {   
	    med_free((void *)pMic_In);
        pMic_In = NULL;
    }
}

#endif

#if Sndp_Rx_Enable
float rx_drc_pregain = 2.0f;
int soundplus_deal_Rx(short *inX, int len, int AncFlag)
{
	int i;
	float tmpAbs;
    float din[240];

    for(i=0; i<len; i++)
    {
        din[i] = inX[i];
    }

	SndpEC_Rx_Frame(din, din, len, AncFlag);

	for(i=0; i<len; i++)
	{
		tmpAbs = din[i];
		if (tmpAbs < -32768) tmpAbs = -32768;
		if (tmpAbs > 32767) tmpAbs = 32767;
		inX[i] = (short)tmpAbs;
	}

	return 0;
}
#endif

#if Sndp_Bf_Enable
#define BF_MIC_NUM 2
static void* pBuff_BF = NULL;
static float* pMic_In_BF = NULL;

int soundplus_deal_BF(short *buf, int len)
{
	int i;
	float tmpAbs;
	int numsamples = len/BF_MIC_NUM;

    //assert((numsamples == 240) || (numsamples == 120));
    
    /* Deinterleave and convert data type from short to float. */
	for(i=0; i<numsamples; i++)
	{
        pMic_In_BF[i] = buf[BF_MIC_NUM * i + 0];
        pMic_In_BF[SPX_FrmShf_16k + i] = buf[BF_MIC_NUM * i + 1];
	}

    /* Beamforming */
    Sndp_SpxEnh_BF(pMic_In_BF, numsamples);

    /* Convert data type from float to short. */
    for (i = 0; i < numsamples; i++)
	{
		tmpAbs = pMic_In_BF[i];
        if (tmpAbs < -32768) tmpAbs = -32768;
        else if (tmpAbs > 32767) tmpAbs = 32767;
        buf[i] = (short)tmpAbs;
	}
	
	return 0;
}

int soundplus_bf_init()
{
	// memory init, get number, malloc memory
	int memsize = Sndp_SpxEnh_BFMemSize();

    pBuff_BF = (void*)med_malloc(memsize);
    if (pBuff_BF == NULL) { return -1; }
    pMic_In_BF = (float*)med_malloc(SPX_FrmShf_16k * BF_MIC_NUM * sizeof(float));
    if (pMic_In_BF == NULL) { return -1; }
	
    Sndp_SpxEnh_BFInit(pBuff_BF);
	return 0;
}

int soundplus_bf_deinit(void)
{
    if (pBuff_BF != NULL)
    {   
        med_free((void*)pBuff_BF);
        pBuff_BF = NULL;
    }
    if (pMic_In_BF != NULL)
    {
        med_free((void*)pMic_In_BF);
        pMic_In_BF = NULL;
    }
	return 0;
}
#endif 

