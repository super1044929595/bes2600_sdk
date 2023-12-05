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
#ifndef __BT_COMMON_H__
#define __BT_COMMON_H__

#include "btlib_type.h"
#include "btlib.h"

#define bdaddr_equal(addr1, addr2) \
            (co_memcmp((const void *)(addr1),(const void *)(addr2),6) == \
                        0 ? TRUE : FALSE)

#define bdaddr_set(dest, src) \
    do { \
        memcpy((void *)(dest),(void *)(src),6); \
    } while (0);

static inline void bdaddr_cpy(struct bdaddr_t *dst, const struct bdaddr_t *src) {
    co_memcpy(dst, src, sizeof(struct bdaddr_t));
}

void print_bdaddr(const struct bdaddr_t *bdaddr);
int ba2str(const struct bdaddr_t *bdaddr, char *str);
int sprintf(char *buf, const char *fmt, ...);

#endif /* __BT_COMMON_H__ */