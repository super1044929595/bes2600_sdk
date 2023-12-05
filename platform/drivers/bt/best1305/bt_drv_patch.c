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
#include "plat_types.h"
#include "hal_i2c.h"
#include "hal_uart.h"
#include "hal_chipid.h"
#include "bt_drv.h"
#include "bt_drv_interface.h"
#include "bt_drv_2500i_internal.h"
#include "string.h"
#include "besbt_string.h"

#define INS_PATCH_MAX_NUM  (64)

#define BTDRV_PATCH_ACT     0x1
#define BTDRV_PATCH_INACT   0x0

typedef struct
{
    uint8_t patch_index;            //patch position
    uint8_t patch_state;            //is patch active
    uint16_t patch_length;          //patch length 0:one instrution replace  other:jump to ram to run more instruction
    uint32_t patch_remap_address;   //patch occured address
    uint32_t patch_remap_value;     //patch replaced instuction
    uint32_t patch_start_address;   //ram patch address for lenth>0
    uint8_t *patch_data;            //ram patch date for length >0

} BTDRV_PATCH_STRUCT;

/*****************************************************************************
 Prototype    : btdrv_ins_patch_write
 Description  : bt driver instruction patch write
 Input        : BTDRV_PATCH_STRUCT *ins_patch_p
 Output       : None
 Return Value :
 Calls        :
 Called By    :

 History      :
 Date         : 2018/12/6
 Author       : bestechnic
 Modification : Created function

*****************************************************************************/
void btdrv_ins_patch_write(BTDRV_PATCH_STRUCT *ins_patch_p)
{
    int sRet = 0;

    BT_PATCH_WR(BTDRV_PATCH_INS_REMAP_ADDR_START + ins_patch_p->patch_index*sizeof(uint32_t), ins_patch_p->patch_remap_value);
    if(ins_patch_p->patch_length != 0)  //have ram patch data
    {
        sRet = memcpy_s((uint8_t *)(uintptr_t)ins_patch_p->patch_start_address,ins_patch_p->patch_length,ins_patch_p->patch_data,ins_patch_p->patch_length);
        if (sRet){
            BT_DRV_TRACE(1, "%s line:%d sRet:%d", __func__, __LINE__, sRet);
        }
    }

    uint32_t remap_addr =  ins_patch_p->patch_remap_address | 1;
    BT_PATCH_WR(BTDRV_PATCH_INS_COMP_ADDR_START + ins_patch_p->patch_index*sizeof(uint32_t), remap_addr);
}
/*****************************************************************************
 Prototype    : btdrv instruction patch init, a

 Description  : btdrv_ins_patch_init
 Input        : void
 Output       : None
 Return Value :
 Calls        :
 Called By    :

 History        :
 Date         : 2018/12/6
 Author       : bestechnic
 Modification : Created function

*****************************************************************************/

extern void btdrv_function_patch_init(void);

void btdrv_ins_patch_init(void)
{
    btdrv_function_patch_init();
}

//////////////////////////////patch enable////////////////////////

void btdrv_patch_en(uint8_t en)
{
    if(en)
    {
        *(volatile uint32_t *)BTDRV_PATCH_EN_REG |= 0x80000000;
    }
    else
    {
        *(volatile uint32_t *)BTDRV_PATCH_EN_REG &= ~0x80000000;
    }
}

void btdrv_ins_patch_disable(uint8_t index)
{
    //disable patch, clear remap address and jump instruction
    BT_PATCH_WR(BTDRV_PATCH_INS_COMP_ADDR_START+index*sizeof(uint32_t), 0);
    BT_PATCH_WR(BTDRV_PATCH_INS_REMAP_ADDR_START + index*sizeof(uint32_t), 0);
}

void btdrv_ins_patch_test_init(void)
{

    btdrv_poweron(BT_POWEROFF);

    btdrv_poweron(BT_POWERON);

    btdrv_hciopen();

    btdrv_config_init();

#ifdef TX_IQ_CAL
    //btdrv_poweron(BT_POWEROFF) will lose some calibration flags. Before this problem is solved, you need to manually write these flag
    BTDIGITAL_REG_SET_FIELD(0xD0350348, 0x1, 31, 0x1);
    bt_dccalib_set_value();
#endif

#ifdef RX_IQ_CAL
    if(!btdrv_freqcalib_mode_get())
    {
        BTDIGITAL_REG_SET_FIELD(0xD03503E0, 0x3, 28, 0);    //enable iq rx cal
    }
#endif

    btdrv_patch_en(0);

    for(uint8_t i=0; i< INS_PATCH_MAX_NUM; i++)
    {
        btdrv_ins_patch_disable(i);
    }
    bt_drv_reg_op_for_test_mode_disable();

    btdrv_function_testmode_patch_init();

    btdrv_patch_en(1);
}
