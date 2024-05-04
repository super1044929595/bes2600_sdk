#include "multi_heap.h"
#include "heap_api.h"
#include "string.h"
#include "hal_trace.h"

// TODO: multi heap and block
#define MED_HEAP_BLOCK_MAX_NUM  (3)
static int g_block_index = 0;
static int g_cp_block_index = 0;
static int g_switch_cp = 0;
// NOTE: Can use g_med_heap_begin_addr and g_med_heap_size to optimize free speed
// static void *g_med_heap_begin_addr[MED_HEAP_BLOCK_MAX_NUM];
// static size_t g_med_heap_size[MED_HEAP_BLOCK_MAX_NUM];

static heap_handle_t g_med_heap[MED_HEAP_BLOCK_MAX_NUM];
static heap_handle_t g_med_cp_heap[MED_HEAP_BLOCK_MAX_NUM];

void heap_memory_info(heap_handle_t heap, size_t *total, size_t *used, size_t *max_used)
{
    multi_heap_info_t info;
    heap_get_info(heap, &info);

    if (total != NULL)
        *total = info.total_bytes;

    if (used  != NULL)
        *used = info.total_allocated_bytes;

    if (max_used != NULL)
        *max_used = info.total_bytes - info.minimum_free_bytes;
}

static int med_help_get_index(void *ptr)
{
    int index = 0;
    int diff_addr = 0;
    multi_heap_info_t info;

    for (index = 0; index < g_block_index; index++)
    {
        heap_get_info(g_med_heap[index], &info);
        diff_addr = (char *)ptr - (char *)g_med_heap[index];

#ifdef HEAP_API_DEBUG
        TRACE(3,"[%s] index = %d, diff_addr = %d", __func__, index, diff_addr);
#endif

        if ((diff_addr > 0) && (diff_addr < info.total_bytes))
        {
            break;
        }
    }

    ASSERT(index < g_block_index, "[%s] Can not find ptr = %p", __func__, ptr);

    return index;
}

static int med_help_get_cp_index(void *ptr)
{
    int index = 0;
    int diff_addr = 0;
    multi_heap_info_t info;

    for (index = 0; index < g_cp_block_index; index++)
    {
        heap_get_info(g_med_cp_heap[index], &info);
        diff_addr = (char *)ptr - (char *)g_med_cp_heap[index];

#ifdef HEAP_API_DEBUG
        TRACE(3,"[%s] index = %d, diff_addr = %d", __func__, index, diff_addr);
#endif

        if ((diff_addr > 0) && (diff_addr < info.total_bytes))
        {
            break;
        }
    }

    ASSERT(index < g_cp_block_index, "[%s] Can not find ptr = %p", __func__, ptr);

    return index;
}

static void med_heap_reset(void)
{
    g_block_index = 0;
    g_cp_block_index = 0;
    g_switch_cp = 0;

    for (int i = 0; i < MED_HEAP_BLOCK_MAX_NUM; i++)
    {
        // g_med_heap_begin_addr[i] = NULL;
        // g_med_heap_size[i] = NULL;
        g_med_heap[i] = NULL;
        g_med_cp_heap[i] = NULL;
    }
}

void med_heap_set_cp(int switch_cp)
{
#if defined(UNIFY_HEAP_ENABLED)
    switch_cp = 0;
#endif
    TRACE(2,"[%s] switch_cp: %d", __func__, switch_cp);
    g_switch_cp = switch_cp;
}

void med_heap_add_block(void *begin_addr, size_t size)
{
    if (g_switch_cp)
    {
        TRACE(4,"[%s] g_cp_block_index = %d, begin_addr = %p, size = %d", __func__, g_cp_block_index, begin_addr, size);
        ASSERT(g_cp_block_index < MED_HEAP_BLOCK_MAX_NUM, "[%s] g_cp_block_index(%d) >= MED_HEAP_BLOCK_MAX_NUM", __func__, g_cp_block_index);
    }
    else
    {
        TRACE(4,"[%s] g_block_index = %d, begin_addr = %p, size = %d", __func__, g_block_index, begin_addr, size);
        ASSERT(g_block_index < MED_HEAP_BLOCK_MAX_NUM, "[%s] g_block_index(%d) >= MED_HEAP_BLOCK_MAX_NUM", __func__, g_block_index);
    }

    memset(begin_addr, 0, size);
    if (g_switch_cp)
    {
        g_med_cp_heap[g_cp_block_index] = heap_register(begin_addr, size);
        g_cp_block_index++;
    }
    else
    {
        g_med_heap[g_block_index] = heap_register(begin_addr, size);
        // g_med_heap_begin_addr[g_block_index] = begin_addr;
        // g_med_heap_size[g_block_index] = size;

        g_block_index++;
    }
}

void med_heap_init(void *begin_addr, size_t size)
{
    med_heap_reset();
    med_heap_add_block(begin_addr, size);
}

void *med_malloc(size_t size)
{
    int index = 0;
    void *ptr = NULL;

    if (size == 0)
        return NULL;

    if (g_switch_cp)
    {
        for (index = 0; index < g_cp_block_index; index++)
        {
            if (multi_heap_free_size(g_med_cp_heap[index]) >= size)
            {
                break;
            }
        }

        ASSERT(index < g_cp_block_index, "[%s] index = %d, g_block_index = %d. Can not malloc any RAM", __func__, index, g_cp_block_index);
        ptr = heap_malloc(g_med_cp_heap[index], size);
    }else{
        for (index = 0; index < g_block_index; index++)
        {
            if (multi_heap_free_size(g_med_heap[index]) >= size)
            {
                break;
            }
        }

        ASSERT(index < g_block_index, "[%s] index = %d, g_block_index = %d. Can not malloc any RAM", __func__, index, g_block_index);
        ptr = heap_malloc(g_med_heap[index], size);
    } 
    ASSERT(ptr != NULL, "[%s]: no memory, needed size %d", __FUNCTION__, size);
    
#ifdef HEAP_API_DEBUG
    TRACE(3,"[%s] ptr = %x, index = %d, size = %d", __func__,ptr, index, size);
#endif
    return ptr;
}

void med_free(void *p)
{
    if (p)
    {
        if (g_switch_cp)
        {
            int index = med_help_get_cp_index(p);
            heap_free(g_med_cp_heap[index], p);
        }
        else
        {
            int index = med_help_get_index(p);
            heap_free(g_med_heap[index], p);
        }

        p = NULL;
    }
}

void *med_calloc(size_t nmemb, size_t size)
{
    if (size == 0)
        return NULL;

    void *ptr = med_malloc(nmemb * size);

    if (ptr)
    {
        memset(ptr, 0 , nmemb * size);
    }

    return ptr;
}

void *med_realloc(void *ptr, size_t size)
{
    // TODO: Do not support multi blocks
    // TODO: Do not support cp
    void *newptr = heap_realloc(g_med_heap[0],ptr,size);

    ASSERT(newptr != NULL, "[%s]: no memory, needed size %d", __FUNCTION__, size);

    return newptr;
}

void med_memory_info(size_t *total,
                    size_t *used,
                    size_t *max_used)
{
    size_t _total = 0;
    size_t _used = 0;
    size_t _max_used = 0;

    for (int i = 0; i < g_block_index; i++)
    {
        heap_memory_info(g_med_heap[i], &_total, &_used, &_max_used);
        *total += _total;
        *used += _used;
        *max_used += _max_used;

#ifdef HEAP_API_DEBUG
        TRACE(4,"[%s] %d: g_med_heap = %p, size = %d", __func__, i, g_med_heap[i], _total);
#endif
    }

    for (int i = 0; i < g_cp_block_index; i++)
    {
        heap_memory_info(g_med_cp_heap[i], &_total, &_used, &_max_used);
        *total += _total;
        *used += _used;
        *max_used += _max_used;

#ifdef HEAP_API_DEBUG
        TRACE(4,"[%s] %d: g_med_cp_heap = %p, size = %d", __func__, i, g_med_cp_heap[i], _total);
#endif
    }
}

custom_allocator med_allocator = {
    .malloc = med_malloc,
    .calloc = med_calloc,
    .free = med_free,
};

custom_allocator *default_allocator(void)
{
    return &med_allocator;
}
