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
#if defined(USE_PERIPHERAL_THREAD)
#include <string.h>
#include "cmsis_os.h"
#include "app_tws_ibrt_trace.h"
#include "bluetooth.h"
#include "hci_api.h"
#include "me_api.h"
#include "app_tws_ibrt.h"
#include "app_tws_besaud.h"
#include "app_vendor_cmd_evt.h"
#include "tws_role_switch.h"
#include "l2cap_api.h"
#include "rfcomm_api.h"
#include "conmgr_api.h"
#include "bt_if.h"
#include "app_ibrt_if.h"
#include "app_tws_ibrt_cmd_handler.h"
#include "app_tws_ibrt.h"
#include "bt_drv_interface.h"
#include "bt_drv_reg_op.h"
#include "app_ibrt_peripheral_manager.h"
#include "app_ibrt_ui.h"
#if defined(IBRT)
osThreadId app_ibrt_peripheral_tid;
static void app_ibrt_peripheral_thread(void const *argument);
osThreadDef(app_ibrt_peripheral_thread, osPriorityHigh,1, 2048,"app_ibrt_peripheral_thread");

#define TWS_PERIPHERAL_DEVICE_MAILBOX_MAX (10)
osMailQDef (app_ibrt_peripheral_mailbox, TWS_PERIPHERAL_DEVICE_MAILBOX_MAX, TWS_PD_MSG_BLOCK);
static osMailQId app_ibrt_peripheral_mailbox = NULL;
static uint8_t app_ibrt_peripheral_mailbox_cnt = 0;

static int app_ibrt_peripheral_mailbox_init(void)
{
    app_ibrt_peripheral_mailbox = osMailCreate(osMailQ(app_ibrt_peripheral_mailbox), NULL);
    if (app_ibrt_peripheral_mailbox == NULL)  {
        TRACE(0,"Failed to Create app_ibrt_peripheral_mailbox\n");
        return -1;
    }
    app_ibrt_peripheral_mailbox_cnt = 0;
    return 0;
}

int app_ibrt_peripheral_mailbox_put(TWS_PD_MSG_BLOCK* msg_src)
{
    if(!msg_src){
        TRACE(0,"msg_src is a null pointer in app_ibrt_peripheral_mailbox_put!");
        return -1;
    }
    if(app_ibrt_peripheral_tid==NULL)
    {
        TRACE(0,"app_ibrt_peripheral_thread not ready!  can't use it's mailbox");
        return -1;
    }
    osStatus status;
    TWS_PD_MSG_BLOCK *msg_p = NULL;
    msg_p = (TWS_PD_MSG_BLOCK*)osMailAlloc(app_ibrt_peripheral_mailbox, 0);
    ASSERT(msg_p, "osMailAlloc error");
    msg_p->msg_body.message_id = msg_src->msg_body.message_id;
    msg_p->msg_body.message_ptr = msg_src->msg_body.message_ptr;
    msg_p->msg_body.message_Param0 = msg_src->msg_body.message_Param0;
    msg_p->msg_body.message_Param1 = msg_src->msg_body.message_Param1;
    msg_p->msg_body.message_Param2 = msg_src->msg_body.message_Param2;

    status = osMailPut(app_ibrt_peripheral_mailbox, msg_p);
    if (osOK == status)
        app_ibrt_peripheral_mailbox_cnt++;
    return (int)status;
}

int app_ibrt_peripheral_mailbox_free(TWS_PD_MSG_BLOCK* msg_p)
{
    if(!msg_p){
        TRACE(0,"msg_p is a null pointer in app_ibrt_peripheral_mailbox_free!");
        return -1;
    }
    osStatus status;

    status = osMailFree(app_ibrt_peripheral_mailbox, msg_p);
    if (osOK == status)
        app_ibrt_peripheral_mailbox_cnt--;

    return (int)status;
}

int app_ibrt_peripheral_mailbox_get(TWS_PD_MSG_BLOCK** msg_p)
{
    if(!msg_p){
        TRACE(0,"msg_p is a null pointer in app_ibrt_peripheral_mailbox_get!");
        return -1;
    }

    osEvent evt;
    evt = osMailGet(app_ibrt_peripheral_mailbox, osWaitForever);
    if (evt.status == osEventMail) {
        *msg_p = (TWS_PD_MSG_BLOCK *)evt.value.p;
        return 0;
    }
    return -1;
}

/*****************************************************************************
 Prototype    : app_ibrt_peripheral_thread
 Description  : peripheral manager thread
 Input        : void const *argument
 Output       : None
 Return Value :
 Calls        :
 Called By    :

 History        :
 Date         : 2019/4/18
 Author       : bestechnic
 Modification : Created function

*****************************************************************************/
void app_ibrt_peripheral_thread(void const *argument)
{
    while(1){
        TWS_PD_MSG_BLOCK *msg_p = NULL;
        if ((!app_ibrt_peripheral_mailbox_get(&msg_p))&&(!argument)) {
            switch(msg_p->msg_body.message_id){
                case 0:
                    break;
                case 1:
                    break;
                case 0xff: {// ibrt test
                    char ibrt_cmd[20] = {0};
                    memcpy(ibrt_cmd+0, &msg_p->msg_body.message_Param0, 4);
                    memcpy(ibrt_cmd+4, &msg_p->msg_body.message_Param1, 4);
                    memcpy(ibrt_cmd+8, &msg_p->msg_body.message_Param2, 4);
                    TRACE(1,"ibrt_ui_log: %s\n", ibrt_cmd);
                    app_ibrt_ui_test_cmd_handler((unsigned char*)ibrt_cmd, strlen(ibrt_cmd)+1);
                    }
                    break;
                default:
                    break;
            }
            app_ibrt_peripheral_mailbox_free(msg_p);
        }
    }
}

extern "C" void app_ibrt_peripheral_perform_test(const char* ibrt_cmd)
{
    TWS_PD_MSG_BLOCK msg;
    msg.msg_body.message_id = 0xff;
    memcpy(&msg.msg_body.message_Param0, ibrt_cmd+0, 4);
    memcpy(&msg.msg_body.message_Param1, ibrt_cmd+4, 4);
    memcpy(&msg.msg_body.message_Param2, ibrt_cmd+8, 4);
    app_ibrt_peripheral_mailbox_put(&msg);
}

void app_ibrt_peripheral_thread_init(void)
{
    if (app_ibrt_peripheral_mailbox_init())
        return;

    app_ibrt_peripheral_tid = osThreadCreate(osThread(app_ibrt_peripheral_thread), NULL);
    if (app_ibrt_peripheral_tid == NULL)  {
        TRACE(0,"Failed to Create app_ibrt_peripheral_thread\n");
        return;
    }

    return;
}
#endif
#endif

