#ifndef _SNDP_LIB_H
#define _SNDP_LIB_H

#include "stdint.h"

#ifdef __cplusplus
extern "C" {
#endif

//-----------------------------------------------------------------------------

// sndp alg interface define---------------------------------------------------


/***************************************************************
	FunctionName:	sndp_license_auth
	Purpose:		soundplus algorithm authorization
	Parameter:		[IN] license_key: license byte array
					[IN] Key_len: byte array length
					
	Return:			return 1 if sucess,else return error code
****************************************************************/
int sndp_license_auth(uint8_t* license_key, int Key_len);

/***************************************************************
    FunctionName:   sndp_license_status_get
    Purpose:        get soundplus algorithm license status
    Parameter:
                    [IN] void
    Return:         return 1 if Auth sucess,else return error code
****************************************************************/
int sndp_license_status_get(void);

/******************************************************************************
    FunctionName:   sndp_get_lib_version
    Purpose:        get soundplus algorithm lib version
    Parameter:      
                    void
            
    Return:         unsigned char *:  point to version string
******************************************************************************/
unsigned char *sndp_get_lib_version(void);

/******************************************************************************
    FunctionName:   sndp_get_lib_built_datetime
    Purpose:        get soundplus algorithm lib built_datetime
    Parameter:      
                    void
            
    Return:         char *:  point to built datetime string
******************************************************************************/
unsigned char *sndp_get_lib_built_datetime(void);

int SndpEc_TxSpxEnh_MemBufSizeInBytes_ap(void);
void SndpEc_TxSpxEnh_Init_ap(void* membuf, int NrwFlag);
int SndpEc_TxSpxEnh_MemBufSizeInBytes_cp(int *size1, int *size2, int *size3);
void SndpEc_TxSpxEnh_Init_cp(void *membuf1, void *membuf2, void *membuf3, int NrwFlag);

//void Sndp_BTDM_SpxEnhInit_angles(void *membuf, int membufsizeinbytes,
//                                 int angle1_in_degree, int angle2_in_degree);
//        assert(angle1_in_degree>=30 && angle2_in_degree<=90);
//        assert(angle1_in_degree+5 <= angle2_in_degree);
/*
* inX        : two-channel interleaved input
* outY       : one-channel output
* numsamples : number of samples in buffer (MUST BE 240)
*/
void SndpEC_Rx_Frame(float* outY, float* inX, int numsamples, int AncFlag);
void SndpEC_Tx_AecFrame(float* out, float* out_echos, float* pMic_In, float* pRef_In);
void SndpEC_Tx_MMicSpxEhnFrame(float* out, float* AecedX, float* echos, float* pMic_In, int AncFlag);


/******************************************************************************
    FunctionName:   Sndp_SpxEnh_BFMemSize
    Purpose:        calculate BF soundplus algorithm required memory size
    Parameter:      
                    void
            
    Return:         int: memory size in bytes
******************************************************************************/
int Sndp_SpxEnh_BFMemSize(void);

/******************************************************************************
    FunctionName:   Sndp_SpxEnh_BFInit
    Purpose:        BF soundplus algorithm init
    Parameter:      
					void *membuf : the memory giving to alg
    Return:         void
******************************************************************************/
void Sndp_SpxEnh_BFInit(void *membuf);

/******************************************************************************
	FunctionName:	Sndp_SpxEnh_BF
	Purpose:		BF soundplus algorithm process main-function
	Parameter:		
					float *inOut : two-channel de-interleaved input data
					               when process finish one-channel output data
					int numsamples : frame len (NOTE : MUST BE 240)
	Return: 		void
******************************************************************************/
void Sndp_SpxEnh_BF(float *inOut, int numsamples);

#if 1//defined(SNDP_TX_3MIC_TESTWIND)
int Sndp_SpxEnh_WindModule_MemSize(void);

void Sndp_SpxEnh_WindModule_Init(void* membuf, int memsize);

float Sndp_SpxEnh_WindModule_Tx(float* inX);
#endif

#ifdef __cplusplus
}
#endif

#endif // _SNDP_LIB_H
