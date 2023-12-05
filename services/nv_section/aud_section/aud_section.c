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
#include <stdio.h>
#include "hal_trace.h"
#include "aud_section.h"
#include "crc32.h"
#include "tgt_hardware.h"
#include "string.h"
#include "hal_norflash.h"
#include "cmsis.h"

extern uint32_t __aud_start[];

#ifndef ANC_COEF_LIST_NUM
#define ANC_COEF_LIST_NUM                   0
#endif

static uint32_t section_device_length[AUDIO_SECTION_DEVICE_NUM] = {
    AUDIO_SECTION_LENGTH_ANC,
    AUDIO_SECTION_LENGTH_AUDIO,
    AUDIO_SECTION_LENGTH_SPEECH,
};


uint32_t audio_section_get_device_flash_addr(uint32_t device)
{
    return (uint32_t)__aud_start + audio_section_get_device_addr_offset(device);
}

uint32_t audio_section_get_device_addr_offset(uint32_t device)
{
    ASSERT(device < AUDIO_SECTION_DEVICE_NUM, "[%s] device(%d) >= AUDIO_SECTION_DEVICE_NUM", __func__, device);

    uint32_t addr_offset = 0;

    for (uint32_t i=0; i<device; i++)
    {
        addr_offset += section_device_length[i];
    }

    return addr_offset;
}

uint32_t audio_section_get_device_size(uint32_t device)
{
    ASSERT(device < AUDIO_SECTION_DEVICE_NUM, "[%s] device(%d) >= AUDIO_SECTION_DEVICE_NUM", __func__, device);

    return section_device_length[device];
}

int audio_section_store_cfg(uint32_t device, uint8_t *cfg, uint32_t len)
{
    uint32_t addr_start = 0;
    section_head_t  *head = (section_head_t*)cfg;

    head->magic = aud_section_magic;
    head->version = aud_section_struct_version;
    head->crc = crc32(0, cfg + sizeof(section_head_t), len - sizeof(section_head_t));

    addr_start = (uint32_t)__aud_start + audio_section_get_device_addr_offset(device);

#ifdef AUDIO_SECTION_DEBUG
    TRACE(2,"[%s] len = %d", __func__, len);
    TRACE(2,"[%s] addr_start = 0x%x", __func__, addr_start);
    TRACE(2,"[%s] block length = 0x%x", __func__, section_device_length[device]);
#endif

    enum HAL_NORFLASH_RET_T flash_opt_res;
    uint32_t flag = int_lock();
    flash_opt_res = hal_norflash_erase(HAL_FLASH_ID_0, addr_start, len);
    int_unlock(flag);

    if (flash_opt_res)
    {
        TRACE(2,"[%s] ERROR: erase flash res = %d", __func__, flash_opt_res);
        return flash_opt_res;
    }

    flag = int_lock();
    flash_opt_res = hal_norflash_write(HAL_FLASH_ID_0, addr_start, (uint8_t *)cfg, len);
    int_unlock(flag);

    if (flash_opt_res)
    {
        TRACE(2,"[%s] ERROR: write flash res = %d", __func__, flash_opt_res);
        return flash_opt_res;
    }

    return 0;
}

int audio_section_load_cfg(uint32_t device, uint8_t *cfg, uint32_t len)
{
    uint32_t addr_start = 0;
    uint32_t crc = 0;
    section_head_t  *head = NULL;

    addr_start = (uint32_t)__aud_start + audio_section_get_device_addr_offset(device);

#ifdef AUDIO_SECTION_DEBUG
    TRACE(2,"[%s] len = %d", __func__, len);
    TRACE(2,"[%s] addr_start = 0x%x", __func__, addr_start);
    TRACE(2,"[%s] block length = 0x%x", __func__, section_device_length[device]);
#endif
    head = (section_head_t *)addr_start;
    if(head->magic != aud_section_magic)
    {
        TRACE(3,"[%s] WARNING: Different magic number (%x != %x)", __func__, head->magic, aud_section_magic);
        return -1;        
    }
    if(head->version != aud_section_struct_version)
    {
        TRACE(3,"[%s] WARNING: Different version number (%x != %x)", __func__, head->version, aud_section_struct_version);
        return -2;        
    }
    crc = crc32(0, (uint8_t*)(addr_start + sizeof(section_head_t)), len - sizeof(section_head_t));
    if(head->crc != crc)
    {
        TRACE(3,"[%s] WARNING: Different crc (%x != %x)", __func__, head->crc, crc);
        return -2;        
    }

    memcpy(cfg, (uint8_t*)addr_start, len);

    return 0;
}

int anccfg_loadfrom_audsec(const struct_anc_cfg *list[], const struct_anc_cfg *list_44p1k[], uint32_t count)
{
#ifdef PROGRAMMER

    return 1;

#else // !PROGRAMMER

#ifdef CHIP_BEST1000
    ASSERT("[%s] Can not support anc load in this branch!!!", __func__);
#else
    unsigned int re_calc_crc,i;
    const best2000_aud_section *audsec_ptr;

    audsec_ptr = (best2000_aud_section *)__aud_start;
    TRACE(3,"0x%x,0x%x,0x%x",audsec_ptr->sec_head.magic,audsec_ptr->sec_head.version,audsec_ptr->sec_head.crc);
    if (audsec_ptr->sec_head.magic != aud_section_magic) {
        TRACE(0,"Invalid aud section - magic");
        return 1;
    }
    re_calc_crc = crc32(0,(unsigned char *)&(audsec_ptr->sec_body),sizeof(audsec_body)-4);
    if (re_calc_crc != audsec_ptr->sec_head.crc){
        TRACE(0,"crc verify failure, invalid aud section.");
        return 1;
    }
    TRACE(0,"Valid aud section.");
    for(i=0;i<ANC_COEF_LIST_NUM;i++)
        list[i] = (struct_anc_cfg *)&(audsec_ptr->sec_body.anc_config.anc_config_arr[i].anc_cfg[PCTOOL_SAMPLERATE_48X8K]);
    for(i=0;i<ANC_COEF_LIST_NUM;i++)
        list_44p1k[i] = (struct_anc_cfg *)&(audsec_ptr->sec_body.anc_config.anc_config_arr[i].anc_cfg[PCTOOL_SAMPLERATE_44_1X8K]);

#endif

    return 0;

#endif // !PROGRAMMER
}

