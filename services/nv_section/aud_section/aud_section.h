/***************************************************************************
 *
 * Copyright 2015-2019 BES.
 * All rights reserved. All unpublished rights reserved.
 *
 * No part of this work may be used or reproduced in any form or by any
 * means, or stored in a database or retrieval system, without prior written
 * permission of BES.
 *
 * Use of this work is governed by a license granted by BES.
 * This work contains confidential and proprietary information of
 * BES. which is protected by copyright, trade secret,
 * trademark and other intellectual property rights.
 *
 ****************************************************************************/
#ifndef __aud_section_h__
#define __aud_section_h__

#include "section_def.h"

#ifdef __cplusplus
extern "C" {
#endif

#define aud_section_debug
#ifdef aud_section_debug
#define aud_trace     TRACE
#else
#define aud_trace(...)
#endif
#include "hal_aud.h"
#define audsec_tag  "audsec_tag"


#define aud_section_magic               0xdad1
#define aud_section_struct_version      1

#define AUD_COEF_LEN        (500)
#if (AUD_SECTION_STRUCT_VERSION == 1)
#define AUD_IIR_NUM         (6)
#elif (AUD_SECTION_STRUCT_VERSION == 2)
#define AUD_IIR_NUM         (8)
#else
#error "Can not support this version!!!"
#endif

typedef struct _anc_rir_coefs {
    int32_t coef_b[3];
	int32_t coef_a[3];
} anc_iir_coefs;

typedef struct _aud_item {
    int32_t     total_gain;
	
	uint16_t   iir_bypass_flag;
	uint16_t   iir_counter;
    anc_iir_coefs   iir_coef[AUD_IIR_NUM];    
    
#if (AUD_SECTION_STRUCT_VERSION == 1)
    uint16_t    fir_bypass_flag;
    uint16_t    fir_len;
    int16_t     fir_coef[AUD_COEF_LEN];
    int8_t      pos_tab[16];
#elif (AUD_SECTION_STRUCT_VERSION == 2)
    int32_t     reserved_for_drc[32];
#endif
    int16_t     reserved1;
    int8_t      dac_gain_offset; // in qdb (quater of dB)
    int8_t      adc_gain_offset; // in qdb (quater of dB)
} aud_item;

typedef struct {
	aud_item anc_cfg_ff_l;
  	aud_item anc_cfg_ff_r;
   	aud_item anc_cfg_fb_l;
  	aud_item anc_cfg_fb_r;
#if (AUD_SECTION_STRUCT_VERSION == 2)	    
   	aud_item anc_cfg_tt_l;
  	aud_item anc_cfg_tt_r;
   	aud_item anc_cfg_mc_l;
  	aud_item anc_cfg_mc_r;
#endif

} struct_anc_cfg;
typedef struct _dehowling_config
{
       uint32_t dehowling_delay;
	aud_item dehowling_l;
}dehowling_config;

typedef struct _struct_spkcalib_cfg
{
	aud_item spkcalib_cfg_l;
}struct_spkcalib_cfg;

enum ANC_INDEX {
    ANC_INDEX_0 = 0,
    ANC_INDEX_1,
    ANC_INDEX_2,
    ANC_INDEX_3,
    ANC_INDEX_TOTAL,
};

typedef struct {
    unsigned char anc_ver[16];
    unsigned char batch_info[16];
    unsigned char serial[16];
} anc_ident;

typedef struct _best2000_anc_iir_coefs {
    int32_t     coef_b[3];
    int32_t     coef_a[3];
} best2000_anc_iir_coefs;

typedef struct _best2000_aud_item {
    int32_t     total_gain;
    uint16_t    iir_bypass_flag;
    uint16_t    iir_counter;
    best2000_anc_iir_coefs  iir_coef[AUD_IIR_NUM];
#if (AUD_SECTION_STRUCT_VERSION == 1)
    uint16_t    fir_bypass_flag;
    uint16_t    fir_len;
    int16_t     fir_coef[AUD_COEF_LEN];
    int8_t      pos_tab[16];
#elif (AUD_SECTION_STRUCT_VERSION == 2)
    int32_t     reserved_for_drc[32];
#endif
    int16_t     reserved1;
    int8_t      dac_gain_offset; // in qdb (quater of dB)
    int8_t      adc_gain_offset; // in qdb (quater of dB)
} best2000_aud_item;

enum auditem_side_enum_t {
    PCTOOL_AUDITEM_SIDE_LEFT,
    PCTOOL_AUDITEM_SIDE_RIGHT,
    PCTOOL_AUDITEM_SIDE_COUNT
};

typedef struct {
    best2000_aud_item   anccfg[PCTOOL_AUDITEM_SIDE_COUNT];
} pctool_anccfg_sruct;

enum auditem_anc_type_enum_t {
    PCTOOL_AUDITEM_ANC_TYPE_FF,
    PCTOOL_AUDITEM_ANC_TYPE_FB,
#if (AUD_SECTION_STRUCT_VERSION == 2)	        
    PCTOOL_AUDITEM_ANC_TYPE_TT,
    PCTOOL_AUDITEM_ANC_TYPE_MC,    
#endif    
    PCTOOL_AUDITEM_ANC_TYPE_COUNT
};

typedef struct {
    pctool_anccfg_sruct     anc_type_cfg[PCTOOL_AUDITEM_ANC_TYPE_COUNT];
} pctool_anccfg_struct;

enum auditem_sample_enum_t {
    PCTOOL_SAMPLERATE_44_1X8K,
    PCTOOL_SAMPLERATE_48X8K,
    PCTOOL_SAMPLERATE_50_7X8K = PCTOOL_SAMPLERATE_48X8K,
    PCTOOL_AUDITEM_SAMPLERATE_COUNT
};

typedef struct {
    pctool_anccfg_struct    anc_cfg[PCTOOL_AUDITEM_SAMPLERATE_COUNT];
} best2000_struct_anc_cfg;

#define BEST2000_ANC_APPMODE_COUNT      4

typedef struct {
    best2000_struct_anc_cfg anc_config_arr[BEST2000_ANC_APPMODE_COUNT];
} best2000_anc_config_t;

#define BEST2000_AUDSEC_RESERVED_LEN (0x10000 - sizeof(section_head_t) - sizeof(anc_ident) - sizeof(best2000_anc_config_t))

typedef struct {
    anc_ident               ancIdent;
    best2000_anc_config_t   anc_config;
    unsigned char           reserved[BEST2000_AUDSEC_RESERVED_LEN];
} audsec_body;

typedef struct {
    section_head_t  sec_head;
    audsec_body     sec_body;
} best2000_aud_section;

typedef struct {
    uint8_t     io_pin;
    uint8_t     set_flag;
} pctool_iocfg;
/*
typedef struct{
    uint8_t digmic_ck_iomux_pin;
    uint8_t digmic_d0_iomux_pin;
    uint8_t digmic_d1_iomux_pin;
    uint8_t digmic_d2_iomux_pin;
    uint8_t digmic_phase;
}digital_mic_cfg;
*/

// Add audio and speech support
#define AUDIO_SECTION_DEBUG

// Device
#define AUDIO_SECTION_DEVICE_ANC            (0)
#define AUDIO_SECTION_DEVICE_AUDIO          (1)
#define AUDIO_SECTION_DEVICE_SPEECH         (2)
#define AUDIO_SECTION_DEVICE_NUM            (3)

// If add device, need add length to section_device_length
#define AUDIO_SECTION_LENGTH_ANC            AUD_ANC_SECTION_SIZE
#define AUDIO_SECTION_LENGTH_AUDIO          AUD_EQ_SECTION_SIZE
#define AUDIO_SECTION_LENGTH_SPEECH         AUD_SPEECH_SECTION_SIZE

uint32_t audio_section_get_device_addr_offset(uint32_t device);
uint32_t audio_section_get_device_flash_addr(uint32_t device);
uint32_t audio_section_get_device_size(uint32_t device);
int audio_section_store_cfg(uint32_t device, uint8_t *cfg, uint32_t len);
int audio_section_load_cfg(uint32_t device, uint8_t *cfg, uint32_t len);
int audio_section_check(uint32_t device, uint32_t totalLen);

#ifdef __cplusplus
}
#endif

#endif
