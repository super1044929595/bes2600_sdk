/***************************************************************************
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
#pragma once

#ifndef __xos_list_H__
#define __xos_list_H__
#if 0
#include <stdbool.h>
#include <stdlib.h>

#ifdef __cplusplus
extern "C" {
#endif

#include "stdint.h"

/* FIFO数据的类型，可以是结构体类型 */
#define qdata_t uint8_t

/* FIFO长度，实际存放的数据=FIFO_SIZE-1 */
#define FIFO_SIZE 6

typedef enum {
    QUEUE_OK,
    QUEUE_FULL,
    QUEUE_EMPTY
}qstatus_t;

typedef struct {
    uint16_t addr_wr;        /* 写地址 */
    uint16_t addr_rd;        /* 读地址 */
    uint16_t length;         /* FIFO长度，实际存放的数据=length-1 */
    qdata_t fifo[FIFO_SIZE];
}queue_t;

qstatus_t queue_reset(queue_t *q);
qstatus_t queue_read(queue_t *q, qdata_t *pdata);
qstatus_t queue_write(queue_t *q, qdata_t data);
int queue_isFull(queue_t *q);
int queue_isEmpty(queue_t *q);
int queue_print(queue_t *q);

typedef void *(*xos_list_mempool_zmalloc)(size_t);
typedef void (*xos_list_mempool_free)(void *);

typedef struct {    
    xos_list_mempool_zmalloc zmalloc;
    xos_list_mempool_free free;
} xos_list_mempool_functions_t;

typedef void (*xos_list_free_cb)(void *data);
typedef bool (*xos_list_iter_cb)(void *data);

typedef struct xos_list_node_t {
  struct xos_list_node_t *next;
  void *data;
} xos_list_node_t;

typedef struct xos_list_t {
  xos_list_node_t *head;
  xos_list_node_t *tail;
  size_t length;
  xos_list_free_cb free_cb;
  xos_list_mempool_functions_t mempool_functions;
} xos_list_t;

struct xos_list_node_t;
typedef struct xos_list_node_t xos_list_node_t;
//struct xos_list_t;
typedef struct xos_list_t xos_list_t;

int xos_list_init(void);

// Lifecycle.
xos_list_t *xos_list_new(xos_list_free_cb callback, xos_list_mempool_zmalloc zmalloc, xos_list_mempool_free free);
void xos_list_free(xos_list_t *xos_list);

// Accessors.
bool xos_list_is_empty(const xos_list_t *xos_list);
size_t xos_list_length(const xos_list_t *xos_list);
void *xos_list_front(const xos_list_t *xos_list);
void *xos_list_back(const xos_list_t *xos_list);

// Mutators.
bool xos_list_insert_after(xos_list_t *xos_list, xos_list_node_t *prev_node, void *data);
bool xos_list_prepend(xos_list_t *xos_list, void *data);
bool xos_list_append(xos_list_t *xos_list, void *data);
bool xos_list_remove(xos_list_t *xos_list, void *data);
void xos_list_clear(xos_list_t *xos_list);

// Iteration.
void xos_list_foreach(const xos_list_t *xos_list, xos_list_iter_cb callback);

xos_list_node_t *xos_list_begin(const xos_list_t *xos_list);
xos_list_node_t *xos_list_end(const xos_list_t *xos_list);
xos_list_node_t *xos_list_next(const xos_list_node_t *node);
void *xos_list_node(const xos_list_node_t *node);

#ifdef __cplusplus
	}
#endif
#endif//if 0
#endif//__FMDEC_H__

