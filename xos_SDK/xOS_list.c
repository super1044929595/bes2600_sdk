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
 #if 0
#include "stdio.h"
#include "cmsis_os.h"
#include "hal_trace.h"
#include "string.h"
#include "xos_list.h"
#include "stdio.h"

/* FIFO复位 */
qstatus_t queue_reset(queue_t *q)
{
    int i = 0;

    q->addr_wr = 0;
    q->addr_rd = 0;
    q->length = FIFO_SIZE;
    for(i = 0; i < q->length; i++)
        q->fifo[i] = 0;

    return QUEUE_OK;
}

/* FIFO写入数据 */
qstatus_t queue_write(queue_t *q, qdata_t data)
{
    if(queue_isFull(q))
    {
        printf("Write failed(%d), queue is full\n", data);
        return QUEUE_FULL;
    }

    q->fifo[q->addr_wr] = data;
    q->addr_wr = (q->addr_wr + 1) % q->length;
    printf("write success: %02d\n", data);
    queue_print(q);

    return QUEUE_OK;
}

/* FIFO读出数据 */
qstatus_t queue_read(queue_t *q, qdata_t *pdata)
{
    if(queue_isEmpty(q))
    {
        printf("Read failed, queue is empty\n");
        return QUEUE_EMPTY;
    }

    *pdata = q->fifo[q->addr_rd];
    q->addr_rd = (q->addr_rd + 1) % q->length;

    printf("read success: %02d\n", *pdata);
    queue_print(q);

    return QUEUE_OK;
}

/* FIFO是否为空 */
int queue_isEmpty(queue_t *q)
{
    return (q->addr_wr == q->addr_rd);
}

/* FIFO是否为满 */
int queue_isFull(queue_t *q)
{
    return ((q->addr_wr + 1) % q->length == q->addr_rd);
}

/* FIFO内数据的个数 */
int queue_count(queue_t *q)
{
    if(q->addr_rd <= q->addr_wr)
        return (q->addr_wr - q->addr_rd);
    //addr_rd > addr_wr;
    return (q->length + q->addr_wr - q->addr_rd);
}

/* 打印当前FIFO内的数据和读写指针的位置 */
int queue_print(queue_t *q)
{
    int i = 0;
    int j = 0;

    for(i = 0; i < q->addr_rd; i++)
        printf("     ");

    printf("rd=%d", q->addr_rd);
    printf("\n");

    for(i = 0; i < q->length; i++)
    {
        if(q->addr_wr > q->addr_rd)
        {
            if(i >= q->addr_rd && i < q->addr_wr)
                printf("[%02d] ", q->fifo[i]);
            else
                printf("[  ] ");
        }
        else//addr_rd > addr_wr
        {
            if(i < q->addr_wr || i >= q->addr_rd)
                printf("[%02d] ", q->fifo[i]);
            else
                printf("[  ] ");
        }
    }
    printf("------count = %d\n", queue_count(q));

    for(i = 0; i < q->addr_wr; i++)
        printf("     ");

    printf("wr=%d", q->addr_wr);
    printf("\n");

    return QUEUE_OK;
}

osPoolId   xos_list_t_mempool = NULL;
osPoolDef (xos_list_t_mempool, 3, xos_list_t);
osPoolId   xos_list_node_t_mempool = NULL;
osPoolDef (xos_list_node_t_mempool, 50, xos_list_node_t);

static xos_list_node_t *xos_list_free_node_(xos_list_t *xos_list, xos_list_node_t *node);

int xos_list_init(void)
{
    if (xos_list_t_mempool == NULL)
        xos_list_t_mempool = osPoolCreate(osPool(xos_list_t_mempool));

    if (xos_list_node_t_mempool == NULL)
        xos_list_node_t_mempool = osPoolCreate(osPool(xos_list_node_t_mempool));

    return 0;
}

inline static xos_list_node_t *malloc_xos_list_node (void)
{
    xos_list_node_t *node = (xos_list_node_t *)osPoolCAlloc (xos_list_node_t_mempool);
    ASSERT(node,"[%s] failed",__func__);
    return node;
}

inline static void free_xos_list_node(xos_list_node_t *node)
{
    osPoolFree (xos_list_node_t_mempool, node);
}

inline static xos_list_t *malloc_xos_list(void)
{
    xos_list_t *xos_list = (xos_list_t *)osPoolCAlloc (xos_list_t_mempool);
    ASSERT(xos_list,"[%s] failed",__func__);
    return xos_list;
}

inline static void free_xos_list(xos_list_t *xos_list)
{
    osPoolFree (xos_list_t_mempool, xos_list);
}


// Returns a new, empty xos_list. Returns NULL if not enough memory could be allocated
// for the xos_list structure. The returned xos_list must be freed with |xos_list_free|. The
// |callback| specifies a function to be called whenever a xos_list element is removed
// from the xos_list. It can be used to release resources held by the xos_list element, e.g.
// memory or file descriptor. |callback| may be NULL if no cleanup is necessary on
// element removal.
xos_list_t *xos_list_new(xos_list_free_cb callback, xos_list_mempool_zmalloc zmalloc, xos_list_mempool_free free) {
  xos_list_t *xos_list = NULL;
  if (zmalloc){
    xos_list = (xos_list_t *)zmalloc(sizeof(xos_list_t));    
    if (xos_list){
      xos_list->mempool_functions.zmalloc = zmalloc;
      xos_list->mempool_functions.free = free;
      xos_list->free_cb = callback;
    }
  }else{
    xos_list = (xos_list_t *)malloc_xos_list();    
    if (xos_list){
      xos_list->free_cb = callback;
    }
  }
  return xos_list;
}

// Frees the xos_list. This function accepts NULL as an argument, in which case it
// behaves like a no-op.
void xos_list_free(xos_list_t *xos_list) {
  if (xos_list != NULL)
    xos_list_clear(xos_list);
  if (xos_list->mempool_functions.free)
    xos_list->mempool_functions.free(xos_list);
  else
    free_xos_list(xos_list);
}

// Returns true if the xos_list is empty (has no elements), false otherwise.
// Note that a NULL xos_list is not the same as an empty xos_list. This function
// does not accept a NULL xos_list.
bool xos_list_is_empty(const xos_list_t *xos_list) {
  ASSERT(xos_list != NULL,"%s",__func__);
  return (xos_list->length == 0);
}

// Returns the length of the xos_list. This function does not accept a NULL xos_list.
size_t xos_list_length(const xos_list_t *xos_list) {
  ASSERT(xos_list != NULL,"%s",__func__);
  return xos_list->length;
}

// Returns the first element in the xos_list without removing it. |xos_list| may not
// be NULL or empty.
void *xos_list_front(const xos_list_t *xos_list) {
  ASSERT(xos_list != NULL,"%s",__func__);
//  ASSERT(!xos_list_is_empty(xos_list),"%s",__func__);
  if(xos_list_is_empty(xos_list)){
     TRACE(0,"WARNING !!!!! %s return NULL !",__func__);
     return NULL;
  }else{
     return xos_list->head->data;
  }
}

// Returns the last element in the xos_list without removing it. |xos_list| may not
// be NULL or empty.
void *xos_list_back(const xos_list_t *xos_list) {
  ASSERT(xos_list != NULL,"%s",__func__);
  ASSERT(!xos_list_is_empty(xos_list),"%s",__func__);

  return xos_list->tail->data;
}

bool xos_list_insert_after(xos_list_t *xos_list, xos_list_node_t *prev_node, void *data) {
  xos_list_node_t *node;
  ASSERT(xos_list != NULL,"%s",__func__);
  ASSERT(prev_node != NULL,"%s",__func__);
  ASSERT(data != NULL,"%s",__func__);

  if (xos_list->mempool_functions.zmalloc)
    node = (xos_list_node_t *)xos_list->mempool_functions.zmalloc(sizeof(xos_list_node_t));
  else
    node = (xos_list_node_t *)malloc_xos_list_node();

  if (!node)
    return false;

  node->next = prev_node->next;
  node->data = data;
  prev_node->next = node;
  if (xos_list->tail == prev_node)
    xos_list->tail = node;
  ++xos_list->length;
  return true;
}

#include "stdio.h"
// Inserts |data| at the beginning of |xos_list|. Neither |data| nor |xos_list| may be NULL.
// This function does not make a copy of |data| so the pointer must remain valid
// at least until the element is removed from the xos_list or the xos_list is freed.
// Returns true if |data| could be inserted, false otherwise (e.g. out of memory).
bool xos_list_prepend(xos_list_t *xos_list, void *data) {
  xos_list_node_t *node;
  ASSERT(xos_list != NULL,"%s",__func__);
  ASSERT(data != NULL,"%s",__func__);

  if (xos_list->mempool_functions.zmalloc)
    node = (xos_list_node_t *)xos_list->mempool_functions.zmalloc(sizeof(xos_list_node_t));
  else
    node = (xos_list_node_t *)malloc_xos_list_node();

  if (!node)
    return false;
  node->next = xos_list->head;
  node->data = data;
  xos_list->head = node;
  if (xos_list->tail == NULL)
    xos_list->tail = xos_list->head;
  ++xos_list->length;
  return true;
}

// Inserts |data| at the end of |xos_list|. Neither |data| nor |xos_list| may be NULL.
// This function does not make a copy of |data| so the pointer must remain valid
// at least until the element is removed from the xos_list or the xos_list is freed.
// Returns true if |data| could be inserted, false otherwise (e.g. out of memory).
bool xos_list_append(xos_list_t *xos_list, void *data) {
  xos_list_node_t *node;
  ASSERT(xos_list != NULL,"%s",__func__);
  ASSERT(data != NULL,"%s",__func__);

  if (xos_list->mempool_functions.zmalloc)
    node = (xos_list_node_t *)xos_list->mempool_functions.zmalloc(sizeof(xos_list_node_t));
  else
    node = (xos_list_node_t *)malloc_xos_list_node();

  if (!node)
    return false;
  node->next = NULL;
  node->data = data;
  if (xos_list->tail == NULL) {
    xos_list->head = node;
    xos_list->tail = node;
  } else {
    xos_list->tail->next = node;
    xos_list->tail = node;
  }
  ++xos_list->length;
  return true;
}

// Removes |data| from the xos_list. Neither |xos_list| nor |data| may be NULL. If |data|
// is inserted multiple times in the xos_list, this function will only remove the first
// instance. If a free function was specified in |xos_list_new|, it will be called back
// with |data|. This function returns true if |data| was found in the xos_list and removed,
// false otherwise.
bool xos_list_remove(xos_list_t *xos_list, void *data) {
  xos_list_node_t *prev, *node;
  ASSERT(xos_list != NULL,"%s",__func__);
  ASSERT(data != NULL,"%s",__func__);

  if (xos_list_is_empty(xos_list))
    return false;

  if (xos_list->head->data == data) {
    xos_list_node_t *next = xos_list_free_node_(xos_list, xos_list->head);
    if (xos_list->tail == xos_list->head)
      xos_list->tail = next;
    xos_list->head = next;
    return true;
  }

  for (prev = xos_list->head, node = xos_list->head->next; node; prev = node, node = node->next)
    if (node->data == data) {
      prev->next = xos_list_free_node_(xos_list, node);
      if (xos_list->tail == node)
        xos_list->tail = prev;
      return true;
    }

  return false;
}

// Removes all elements in the xos_list. Calling this function will return the xos_list to the
// same state it was in after |xos_list_new|. |xos_list| may not be NULL.
void xos_list_clear(xos_list_t *xos_list) {
  xos_list_node_t *node;
  ASSERT(xos_list != NULL,"%s",__func__);
  for (node = xos_list->head; node; )
    node = xos_list_free_node_(xos_list, node);
  xos_list->head = NULL;
  xos_list->tail = NULL;
  xos_list->length = 0;
}

// Iterates through the entire |xos_list| and calls |callback| for each data element.
// If the xos_list is empty, |callback| will never be called. It is safe to mutate the
// xos_list inside the callback. If an element is added before the node being visited,
// there will be no callback for the newly-inserted node. Neither |xos_list| nor
// |callback| may be NULL.
void xos_list_foreach(const xos_list_t *xos_list, xos_list_iter_cb callback) {
  xos_list_node_t *node;
  ASSERT(xos_list != NULL,"%s",__func__);
  ASSERT(callback != NULL,"%s",__func__);

  for (node = xos_list->head; node; ) {
    xos_list_node_t *next = node->next;
    callback(node->data);
    node = next;
  }
}

// Returns an iterator to the first element in |xos_list|. |xos_list| may not be NULL.
// The returned iterator is valid as long as it does not equal the value returned
// by |xos_list_end|.
xos_list_node_t *xos_list_begin(const xos_list_t *xos_list) {
  ASSERT(xos_list != NULL,"%s",__func__);
  return xos_list->head;
}

// Returns an iterator that points past the end of the xos_list. In other words,
// this function returns the value of an invalid iterator for the given xos_list.
// When an iterator has the same value as what's returned by this function, you
// may no longer call |xos_list_next| with the iterator. |xos_list| may not be NULL.
xos_list_node_t *xos_list_end(const xos_list_t *xos_list) {
  ASSERT(xos_list != NULL,"%s",__func__);
  return NULL;
}

// Given a valid iterator |node|, this function returns the next value for the
// iterator. If the returned value equals the value returned by |xos_list_end|, the
// iterator has reached the end of the xos_list and may no longer be used for any
// purpose.
xos_list_node_t *xos_list_next(const xos_list_node_t *node) {
  ASSERT(node != NULL,"%s",__func__);
  return node->next;
}

// Returns the value stored at the location pointed to by the iterator |node|.
// |node| must not equal the value returned by |xos_list_end|.
void *xos_list_node(const xos_list_node_t *node) {
  ASSERT(node != NULL,"%s",__func__);
  return node->data;
}

static xos_list_node_t *xos_list_free_node_(xos_list_t *xos_list, xos_list_node_t *node) {
  xos_list_node_t *next;
  ASSERT(xos_list != NULL,"%s",__func__);
  ASSERT(node != NULL,"%s",__func__);

  next = node->next;

  if (xos_list->free_cb)
    xos_list->free_cb(node->data);
  
  if (xos_list->mempool_functions.free)
    xos_list->mempool_functions.free(node);
  else
    free_xos_list_node(node);

  --xos_list->length;

  return next;
}
#endif
