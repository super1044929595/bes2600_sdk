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
#if defined(NEW_NV_RECORD_ENALBED)
//#include "cmsis_os.h"
#include <stdbool.h>
#include <assert.h>
#include <string.h>
#include "nvrecord_extension.h"
#include "nvrecord_fp_account_key.h"
#include "hal_trace.h"

#ifdef GFPS_ENABLED 

static NV_FP_ACCOUNT_KEY_RECORD_T *nvrecord_fp_account_key_p = NULL;

void nvrecord_rebuild_fp_account_key(NV_FP_ACCOUNT_KEY_RECORD_T* pFpAccountKey)
{
    memset((uint8_t *)pFpAccountKey, 0, sizeof(NV_FP_ACCOUNT_KEY_RECORD_T));
    pFpAccountKey->key_count = 0;
}

NV_FP_ACCOUNT_KEY_RECORD_T* nv_record_get_fp_data_structure_info(void)
{
    return nvrecord_fp_account_key_p;
}

void nv_record_update_fp_data_structure(NV_FP_ACCOUNT_KEY_RECORD_T* pFpData)
{
    if (!memcmp((uint8_t *)pFpData, (uint8_t *)nvrecord_fp_account_key_p, 
        sizeof(NV_FP_ACCOUNT_KEY_RECORD_T)))
    {
        // the updated fp data is the same as the local content, do nothing
    }
    else
    {
        uint32_t lock = nv_record_pre_write_operation();
        TRACE(0,"Fast pair non-volatile data needs to be updated to aligned with peer device.");
        *nvrecord_fp_account_key_p = *pFpData;
        nv_record_post_write_operation(lock);
        nv_record_update_runtime_userdata();
    }
}

void nv_record_fp_account_key_init(void)
{
    if (NULL == nvrecord_fp_account_key_p)
    {
        nvrecord_fp_account_key_p = &(nvrecord_extension_p->fp_account_key_rec);
    }
}

void nv_record_fp_account_key_add(NV_FP_ACCOUNT_KEY_ENTRY_T* param_rec)
{
    TRACE(0,"Add account key:");
    DUMP8("0x%02x ", param_rec->key, sizeof(NV_FP_ACCOUNT_KEY_ENTRY_T));
    uint32_t lock = nv_record_pre_write_operation();
    if (FP_ACCOUNT_KEY_RECORD_NUM == nvrecord_fp_account_key_p->key_count)
    {
       
#ifdef SPOT_ENABLED
         // discard the earliest key
        memmove((uint8_t *)&(nvrecord_fp_account_key_p->accountKey[1]),
            (uint8_t *)&(nvrecord_fp_account_key_p->accountKey[2]),
            (FP_ACCOUNT_KEY_RECORD_NUM - 2)*sizeof(NV_FP_ACCOUNT_KEY_ENTRY_T));
#else
        // discard the earliest key
        memmove((uint8_t *)&(nvrecord_fp_account_key_p->accountKey[0]),
            (uint8_t *)&(nvrecord_fp_account_key_p->accountKey[1]),
            (FP_ACCOUNT_KEY_RECORD_NUM - 1)*sizeof(NV_FP_ACCOUNT_KEY_ENTRY_T));
#endif
        nvrecord_fp_account_key_p->key_count--;
    }

    memcpy((uint8_t *)&(nvrecord_fp_account_key_p->accountKey[nvrecord_fp_account_key_p->key_count]), 
            param_rec,
            sizeof(NV_FP_ACCOUNT_KEY_ENTRY_T));
    
    nvrecord_fp_account_key_p->key_count++;
    nv_record_post_write_operation(lock);
    nv_record_update_runtime_userdata();
    nv_record_execute_async_flush();
}

void nv_record_fp_update_addr(uint8_t i, uint8_t* addr)
{
    TRACE(1,"%s", __func__);
    uint32_t lock = nv_record_pre_write_operation();
    if (nvrecord_fp_account_key_p->key_count < (i + 1))
    {
        return;
    }

    memcpy((uint8_t *)&(nvrecord_fp_account_key_p->accountKey[i].addr), addr, BTIF_BD_ADDR_SIZE);
    nv_record_post_write_operation(lock);
    nv_record_update_runtime_userdata();
}

void nv_record_fp_get_key_by_addr(uint8_t* addr, uint8_t* key)
{
    uint8_t count = nvrecord_fp_account_key_p->key_count;
    uint32_t lock = nv_record_pre_write_operation();
    for(uint8_t i = 0; i < count; i++)
    {
        if(!memcmp(nvrecord_fp_account_key_p->accountKey[i].addr, addr, BTIF_BD_ADDR_SIZE))
        {
            memcpy(key, nvrecord_fp_account_key_p->accountKey[i].key, FP_ACCOUNT_KEY_SIZE);
            break;
        }
    }
    nv_record_post_write_operation(lock);
}

void nv_record_fp_account_info_reset(void) 
{
    uint32_t lock = nv_record_pre_write_operation();
    uint8_t size = nv_record_fp_account_key_count();
    for (uint8_t i = 0 ; i < size ; i++){
    memset((uint8_t *)&(nvrecord_fp_account_key_p->accountKey[i]), 0,
           sizeof(NV_FP_ACCOUNT_KEY_ENTRY_T));
    }
    nvrecord_fp_account_key_p->key_count = 0;
    nv_record_post_write_operation(lock);
    nv_record_fp_update_name(NULL, 0);
}

void nv_record_fp_account_key_delete(void)
{
    nv_record_fp_account_info_reset();
}

bool nv_record_fp_account_key_get_by_index(uint8_t index, uint8_t* outputKey)
{
    if (nvrecord_fp_account_key_p->key_count < (index + 1))
    {
        return false;
    }
    
    memcpy(outputKey, (uint8_t *)&(nvrecord_fp_account_key_p->accountKey[index].key),
        FP_ACCOUNT_KEY_SIZE);

    TRACE(1,"Get account key %d as:", index);
    DUMP8("0x%02x ", outputKey, FP_ACCOUNT_KEY_SIZE);
    
    return true;
}

uint8_t nv_record_fp_account_key_count(void)
{
    if (NULL == nvrecord_fp_account_key_p)
    {
        nvrecord_fp_account_key_p = &(nvrecord_extension_p->fp_account_key_rec);
    }
    ASSERT(nvrecord_fp_account_key_p->key_count <= FP_ACCOUNT_KEY_RECORD_NUM, "fp account exceed");
    return nvrecord_fp_account_key_p->key_count;
}

void nv_record_fp_update_name(uint8_t* ptrName, uint32_t nameLen)
{
    TRACE(1,"update name len %d", nameLen);
    uint32_t lock = nv_record_pre_write_operation();
    
    memset(nvrecord_fp_account_key_p->name, 0, FP_MAX_NAME_LEN);
    if (nameLen > 0)
    {
        DUMP8("0x%02x ", ptrName, nameLen);
        memcpy(nvrecord_fp_account_key_p->name, ptrName, nameLen);
    }
    nvrecord_fp_account_key_p->nameLen = nameLen;

    nv_record_post_write_operation(lock);
    nv_record_update_runtime_userdata();
}

uint8_t* nv_record_fp_get_name_ptr(uint32_t* ptrNameLen)
{
    *ptrNameLen = nvrecord_fp_account_key_p->nameLen;
    return nvrecord_fp_account_key_p->name;
}

void nv_record_fp_update_all(uint8_t *info)
{
    ASSERT(info, "null pointer received in [%s]", __func__);

    if (memcmp(nvrecord_fp_account_key_p, info, sizeof(NV_FP_ACCOUNT_KEY_RECORD_T)))
    {
        uint32_t lock = nv_record_pre_write_operation();
        memcpy(nvrecord_fp_account_key_p, info, sizeof(NV_FP_ACCOUNT_KEY_RECORD_T));
        nv_record_extension_update();
        TRACE(1,"received fp count num:%d", nvrecord_fp_account_key_p->key_count);
        nv_record_post_write_operation(lock);
    }
}

#ifdef SPOT_ENABLED
void nv_record_fp_update_eph_identity_key(uint8_t* ptrEph_identity_key)
{
    uint32_t lock = nv_record_pre_write_operation();

    memset(nvrecord_fp_account_key_p->eph_identity_key, 0, FP_EPH_IDENTITY_KEY_LEN);
    memcpy(nvrecord_fp_account_key_p->eph_identity_key, ptrEph_identity_key, FP_EPH_IDENTITY_KEY_LEN);
    
    nv_record_post_write_operation(lock);
    nv_record_update_runtime_userdata();
}

uint8_t* nv_record_fp_get_eph_identity_key(void)
{
    return nvrecord_fp_account_key_p->eph_identity_key;
}

void nv_record_fp_reset_eph_identity_key(void)
{
    uint32_t lock = nv_record_pre_write_operation();

    memset(nvrecord_fp_account_key_p->eph_identity_key, 0, FP_EPH_IDENTITY_KEY_LEN);
    
    nv_record_post_write_operation(lock);
    nv_record_update_runtime_userdata();
}

void nv_record_fp_update_spot_adv_data(uint8_t* spot_adv_data)
{
    uint32_t lock = nv_record_pre_write_operation();

    memset(nvrecord_fp_account_key_p->spot_adv_data, 0, FP_SPOT_ADV_DATA_LEN);
    memcpy(nvrecord_fp_account_key_p->spot_adv_data, spot_adv_data, FP_SPOT_ADV_DATA_LEN);
    
    nv_record_post_write_operation(lock);
    nv_record_update_runtime_userdata();
}

uint8_t* nv_record_fp_get_spot_adv_data(void)
{
    return nvrecord_fp_account_key_p->spot_adv_data;
}

void nv_record_fp_reset_spot_adv_data(void)
{
    uint32_t lock = nv_record_pre_write_operation();

    memset(nvrecord_fp_account_key_p->spot_adv_data, 0, FP_SPOT_ADV_DATA_LEN);
    
    nv_record_post_write_operation(lock);
    nv_record_update_runtime_userdata();
}

void nv_record_fp_update_spot_adv_eanbled(bool spot_adv_enable)
{
    uint32_t lock = nv_record_pre_write_operation();

    nvrecord_fp_account_key_p->spot_adv_enabled = spot_adv_enable;

    nv_record_post_write_operation(lock);
    nv_record_update_runtime_userdata();
}
bool nv_record_fp_get_spot_adv_enable_value(void)
{
    TRACE(1,"nv_record_fp_get_spot_adv_enable_value,%d",nvrecord_fp_account_key_p->spot_adv_enabled);
    return nvrecord_fp_account_key_p->spot_adv_enabled;
}
void nv_record_fp_reset_spot_adv_enable_value(void)
{
    uint32_t lock = nv_record_pre_write_operation();

    nvrecord_fp_account_key_p->spot_adv_enabled = false;
    nv_record_post_write_operation(lock);
    nv_record_update_runtime_userdata();
}

#endif


#endif

#endif // #if defined(NEW_NV_RECORD_ENALBED)
