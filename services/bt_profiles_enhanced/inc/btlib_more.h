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


#ifndef __BT_LIB_MORE_H__
#define __BT_LIB_MORE_H__

#include "cobuf.h"
#include "co_printf.h"
#include "btlib_type.h"     
#include "cmsis_os.h"
#include "cmsis.h"

#define OS_CRITICAL_METHOD      0
#define OS_ENTER_CRITICAL()     uint32_t os_lock = int_lock()
#define OS_EXIT_CRITICAL()      int_unlock(os_lock)
#define OSTimeDly(a)            osDelay((a)*2)

#endif /* __BT_LIB_MORE_H__ */
