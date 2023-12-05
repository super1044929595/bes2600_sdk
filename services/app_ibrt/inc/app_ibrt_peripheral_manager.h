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
#ifndef __APP_IBRT_PERIPHERAL_MANAGER_H__
#define __APP_IBRT_PERIPHERAL_MANAGER_H__
#ifdef  USE_PERIPHERAL_THREAD
typedef struct {
    uint32_t message_id;
    uint32_t message_ptr;
    uint32_t message_Param0;
    uint32_t message_Param1;
    uint32_t message_Param2;
} TWS_PD_MSG_BODY;

typedef struct {
    TWS_PD_MSG_BODY msg_body;
} TWS_PD_MSG_BLOCK;

int app_ibrt_peripheral_mailbox_put(TWS_PD_MSG_BLOCK* msg_src);
int app_ibrt_peripheral_mailbox_free(TWS_PD_MSG_BLOCK* msg_p);
int app_ibrt_peripheral_mailbox_get(TWS_PD_MSG_BLOCK** msg_p);
void app_ibrt_peripheral_thread_init(void);
#endif
#endif

