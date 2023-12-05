
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
#ifndef __PLATFORM_DEPS_H__
#define __PLATFORM_DEPS_H__

#include "bt_sys_cfg.h"
#include "hal_trace.h"

#if defined(__cplusplus)
extern "C" {
#endif

int Plt_HciRxBuffPop(unsigned char *type, unsigned short *conn_handle, unsigned short *length, unsigned char **data);
int Plt_HciTxBuffPop(unsigned char *type, unsigned short *conn_handle, unsigned short *length, unsigned char **data);
int Plt_HciRxBuffRemainDataLen(void);
int Plt_HciTxBuffRemainDataLen(void);
int Plt_HciFlushbuff(unsigned char hci_cqtype, unsigned char msg_type, unsigned short conn_handle);
void Plt_HciSendData(unsigned char type, unsigned short cmd_conn, unsigned short len, unsigned char *buffer);
void Plt_HciSendBuffer(unsigned char type, unsigned char *buff, int len);
void Plt_NotifyScheduler(void);
void Plt_LockHCIBuffer(void);
void Plt_UNLockHCIBuffer(void);
#define Plt_Assert ASSERT

char OS_Init(void);
void OS_LockStack(void);
void OS_UnlockStack(void);
uint8_t OS_LockIsExist(void);
void OS_StopHardware(void);
void OS_ResumeHardware(void);
void OS_NotifyEvm(void);

#if defined(__cplusplus)
}
#endif

#endif /* __PLATFORM_DEPS_H__ */