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
#ifndef _SCO_API_H
#define _SCO_API_H
#include "btlib_type.h"

/* notify upper layer */
enum sco_event_enum {
    SCO_OPENED,
    SCO_CLOSED
};

#ifdef __cplusplus
extern "C" {
#endif
    int8 sco_register_link(struct bdaddr_t *bdaddr,
                           void (*sco_notify_callback) (enum sco_event_enum event, void *pdata), void *link);
    int8 sco_open_link(struct bdaddr_t *bdaddr,
                       void (*sco_notify_callback) (enum sco_event_enum event, void *pdata, void *link), void *link);
    int8 sco_close_link(struct bdaddr_t *bdaddr1, uint8 reason, void *link);
    int8 sco_unregister_link(struct bdaddr_t *bdaddr);
    int8 sco_init(void);
    int8 sco_exit(void);
    void sco_conn_opened_ind(struct bdaddr_t *bdaddr_remote);
    void sco_conn_closed_ind(struct bdaddr_t *bdaddr_remote);
#ifdef __cplusplus
    extern "C" {
#endif

#endif                          /**/
