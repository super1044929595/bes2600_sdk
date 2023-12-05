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

#ifndef __BASIC_H__
#define __BASIC_H__

#include "btlib_type.h"

void *co_memcpy(void *dst, const void *src, uint32 n);
void *co_memset( void *s, int c, uint32 n);
void delay_ms(int num);

#endif /* __BASIC_H__ */
