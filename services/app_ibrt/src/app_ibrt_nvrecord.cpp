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
#include <string.h>
#include "app_tws_ibrt_trace.h"
#include "app_ibrt_ui.h"
#include "factory_section.h"
#include "app_ibrt_if.h"
#include "nvrecord.h"
#include "nvrecord_env.h"
#include "app_ibrt_nvrecord.h"
#include "ddbif.h"

/*****************************************************************************

Prototype    : app_ibrt_ui_config_ibrt_info
Description  : app ui init
Input        : void
Output       : None
Return Value :
Calls        :
Called By    :

History        :
Date         : 2019/3/29
Author       : bestechnic
Modification : Created function

*****************************************************************************/
void app_ibrt_nvrecord_config_load(void *config)
{
    struct nvrecord_env_t *nvrecord_env;
    ibrt_config_t *ibrt_config = (ibrt_config_t *)config;
    //factory_section_original_btaddr_get(ibrt_config->local_addr.address);
    nv_record_env_get(&nvrecord_env);
    if(nvrecord_env->ibrt_mode.mode!=IBRT_UNKNOW)
    {
        ibrt_config->nv_role=nvrecord_env->ibrt_mode.mode;
        ibrt_config->peer_addr=nvrecord_env->ibrt_mode.record.bdAddr;
        ibrt_config->local_addr=nvrecord_env->ibrt_mode.record.bdAddr;
        if(nvrecord_env->ibrt_mode.tws_connect_success == 0)
        {
            app_ibrt_ui_clear_tws_connect_success_last();
        }
        else
        {
            app_ibrt_ui_set_tws_connect_success_last();
        }
    }
    else
    {
        ibrt_config->nv_role=IBRT_UNKNOW;
    }
}
/*****************************************************************************

Prototype    : app_ibrt_ui_config_ibrt_info
Description  : app ui init
Input        : void
Output       : None
Return Value :
Calls        :
Called By    :

History        :
Date         : 2019/3/29
Author       : bestechnic
Modification : Created function

*****************************************************************************/
int app_ibrt_nvrecord_find(const bt_bdaddr_t *bd_ddr, nvrec_btdevicerecord **record)
{
    return nv_record_btdevicerecord_find(bd_ddr, record);
}

/*****************************************************************************
 Prototype    : app_ibrt_nvrecord_delete_all_mobile_record
 Description  : app_ibrt_nvrecord_delete_all_mobile_record
 Input        : None
 Output       : None
 Return Value :
 Calls        :
 Called By    :

 History        :
 Date         : 2019/4/25
 Author       : bestechnic
 Modification : Created function

*****************************************************************************/
void app_ibrt_nvrecord_delete_all_mobile_record(void)
{
    btif_device_record_t record = {0};
    int paired_dev_count = nv_record_get_paired_dev_count();

    for (int i = paired_dev_count - 1; i >= 0; --i)
    {
        nv_record_enum_dev_records(i, &record);

        if (MOBILE_LINK == app_tws_ibrt_get_link_type_by_addr(&record.bdAddr))
        {
            nv_record_ddbrec_delete(&record.bdAddr);
        }
    }
    app_ibrt_if_config_keeper_clear();
}

