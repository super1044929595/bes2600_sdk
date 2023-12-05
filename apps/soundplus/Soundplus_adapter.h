#ifndef __SOUNDPLUS_ADAPTER_H__
#define __SOUNDPLUS_ADAPTER_H__
#include "stdint.h"
#ifndef WIN32
#include "hal_cmu.h"
#endif

#ifdef __cplusplus
extern "C" {
#endif
/***************************************************************
    FunctionName:   sndp_auth_get_bt_address
    Purpose:        support soundplus algorithm authorize
                    call this function in factory_section.c
    Parameter:
                    version: the version of nvrec

    Return:         void
****************************************************************/
void sndp_auth_get_bt_address(uint8_t version);
/***************************************************************
    FunctionName:   sndp_auth_get_norflash_uuid
    Purpose:        support soundplus algorithm authorize
                    call this function in norflash_drv.c
                    like other function norflash_read_reg_ex
    Parameter:
                    cmd:
                    param:
                    param_len:
                    val:
                    len:
    Return:         return 0 if read sucessed
****************************************************************/
int sndp_auth_get_norflash_uuid(enum HAL_FLASH_ID_T id, uint8_t cmd, uint8_t* param, uint32_t param_len, uint8_t* val, uint32_t len);
/***************************************************************
    FunctionName:   soundplus_auth
    Purpose:        soundplus algorithm authorize
    Parameter:
                    Key:
                    Key_len:
    Return:         return 1 if auth sucessed,else return error code
****************************************************************/
int soundplus_auth(uint8_t* key, int Key_len);
/***************************************************************
    FunctionName:   soundplus_auth_status
    Purpose:        get soundplus algorithm license state
    Parameter:
                    
    Return:         return 1 if auth sucessed,else return error code
****************************************************************/
int soundplus_auth_status();

/***************************************************************
    FunctionName:   soundplus_deal_Aec
    Purpose:        soundplus algorithm Tx AEC process
    Parameter:
                    out : mic pcm data out.
                    out_echos : estimated echos pcm data out.
                    in : mic pcm data in.
                    ref : ref pcm data.
                    buf_len : mic pcm numsamples,
                          set 240*mic_num when BTIF_HF_SCO_CODEC_MSBC,
                          set 120*mic_num when BTIF_HF_SCO_CODEC_CVSD
                    ref_len : ref pcm numsamples,
                          set 240 when BTIF_HF_SCO_CODEC_MSBC,
                          set 120 when BTIF_HF_SCO_CODEC_CVSD
                    AncFlag : 0 - anc off
                              1 - anc on weak
                              2 - anc on 
                              3 - anc on strong
                              4 - Transparency
    Return:         return 0 if sucess,else return other
****************************************************************/
int soundplus_deal_Aec(float* out, float* out_echos, short* in, short* ref, int buf_len, int ref_len);
/***************************************************************
    FunctionName:   soundplus_deal_Tx
    Purpose:        soundplus algorithm Tx ENC process
    Parameter:
                    out : mic pcm data out.
                    in : mic pcm data in.
                    in_echos : estimated echos pcm data in.
                    fb : fb mic pcm data in.
                    numsamples : must be 240
                    AncFlag : 0 - anc off
                              1 - anc on
                              2 - talk through
                    winddegree: wind noise probability
                    snr: snr
    Return:         void
****************************************************************/

void soundplus_deal_Tx(short* out, float* in, float* in_echos, int AncFlag, int numsamples);
//void soundplus_deal_Tx(short* out, float* in, float* fb, int numsamples, int AncFlag, float * winddegree, float *snr);

/***************************************************************
    FunctionName:   soundplus_deal_Rx
    Purpose:        soundplus algorithm Rx process
    Parameter:      
                    buf : bt Rx pcm data.
                    len : Rx pcm data numsamples,
                          set 240 when BTIF_HF_SCO_CODEC_MSBC,
                          set 120 when BTIF_HF_SCO_CODEC_CVSD
                    AncFlag : 0: anc off; 1:anc on
    Return:         return 0 if sucess,else return other
****************************************************************/

int soundplus_deal_Rx(short *inX, int len, int AncFlag);

/***************************************************************
    FunctionName:   soundplus_init
    Purpose:        soundplus algorithm init
    Parameter:      
                    NrwFlag : narrowband flag. 
                              set 1 when BTIF_HF_SCO_CODEC_CVSD, 
                              set 0 when BTIF_HF_SCO_CODEC_MSBC
    Return:         return 0 if sucess,else return other
****************************************************************/
int soundplus_init_ap(int NrwFlag);
int soundplus_init_cp(int NrwFlag);
/***************************************************************
    FunctionName:   soundplus_deinit
    Purpose:        soundplus algorithm deinit
    Parameter:      
                    void
    Return:         return 0 if sucess,else return other
****************************************************************/
int soundplus_deinit_ap(void);
int soundplus_deinit_cp(void);

/***************************************************************
    FunctionName:   soundplus_deal_BF
    Purpose:        Do beamforming
    Parameter:      
                    buf : interleaved pcm data (in/out)
                    len : numsamples(240) * numchannels(2)
    Return:         return 0
****************************************************************/

int soundplus_deal_BF(short *buf, int len);


/***************************************************************
    FunctionName:   soundplus_bf_init
    Purpose:        Allocate memory to BF and initialize BF Module
    Parameter:      
    Return:         return 0 if success else -1.
****************************************************************/
int soundplus_bf_init();


/***************************************************************
    FunctionName:   soundplus_bf_deinit
    Purpose:        Free memory allocated to BF Module
    Parameter:
    Return:         return 0
****************************************************************/
int soundplus_bf_deinit(void);



int soundplus_windmodule_init(void);

void soundplus_WindModule_deInit(void);

float soundplus_deal_windmodule_Tx(short* buf, int buf_len);



#ifdef __cplusplus
}
#endif

#endif
