#include "heap_api.h"
#include "hal_trace.h"
#include "plat_addr_map.h"
#include "app_tota.h"
#include "custom_allocator.h"

#define SYS_MEM_POOL_RESERVED_SIZE          512

extern uint8_t __mem_pool_start__[];
extern uint8_t __mem_pool_end__[];
extern uint8_t __mem_pool2_start__[];
static uint32_t syspool_size = 0;
static uint32_t syspool_used = 0;
#if defined(CHIP_HAS_CP)||defined(UNIFY_HEAP_ENABLED)
extern uint8_t __cp_mem_pool_start__[];
extern uint8_t __cp_mem_pool_end__[];
extern uint8_t *overlay_addr;
#endif

uint32_t syspool_original_size(void)
{
#if defined(CHIP_HAS_CP)&&defined(UNIFY_HEAP_ENABLED)
    if(overlay_addr == NULL){
        return __cp_mem_pool_end__ - __cp_mem_pool_start__ - SYS_MEM_POOL_RESERVED_SIZE;
    }else{
        return __cp_mem_pool_end__ - overlay_addr - SYS_MEM_POOL_RESERVED_SIZE;
    }
#else
    ASSERT((__mem_pool_start__ + SYS_MEM_POOL_RESERVED_SIZE) < __mem_pool2_start__,
        "%s: mem pool size too small: start=%p end=%p reserved_size=%u",
        __func__, __mem_pool_start__, __mem_pool2_start__, SYS_MEM_POOL_RESERVED_SIZE);
    return __mem_pool2_start__ - __mem_pool_start__ - SYS_MEM_POOL_RESERVED_SIZE;
#endif
}

void syspool_init_specific_size(uint32_t size)
{
#if !defined(UNIFY_HEAP_ENABLED)
    share_pool_init();
#endif
    syspool_size = syspool_original_size();
    if(size<syspool_size)
        syspool_size = size;
    syspool_used = 0;
#ifdef TOTA_v2
    if(is_in_tota_mode())
    {
        tota_mempool_init();
    }
#endif    
#if defined(CHIP_HAS_CP)&&defined(UNIFY_HEAP_ENABLED)
    if(overlay_addr == NULL){
        memset(__cp_mem_pool_start__,0,syspool_size);
        TRACE(2,"syspool_init: %p,0x%x",__cp_mem_pool_start__,syspool_size);
    }else{
        memset(overlay_addr,0,syspool_size);
        TRACE(2,"syspool_init: %p,0x%x",overlay_addr,syspool_size);
    }
#else
    memset(__mem_pool_start__,0,syspool_size);
    TRACE(2,"syspool_init: %p,0x%x", __mem_pool_start__,syspool_size);
#endif
}

void syspool_init(void)
{
    syspool_init_specific_size(syspool_original_size());
}

uint8_t* syspool_start_addr(void)
{
#if defined(CHIP_HAS_CP)&&defined(UNIFY_HEAP_ENABLED)
    if(overlay_addr == NULL){
        return __cp_mem_pool_start__;
    }else{
        return overlay_addr;
    }
#else
    return __mem_pool_start__;
#endif
}

uint32_t syspool_total_size(void)
{
    return syspool_size;
}

int syspool_free_size()
{
    return syspool_size - syspool_used;
}

int syspool_get_buff(uint8_t **buff, uint32_t size)
{
    uint32_t buff_size_free;

    buff_size_free = syspool_free_size();

    if (size % 4){
        size = size + (4 - size % 4);
    }
    ASSERT (size <= buff_size_free, "System pool in shortage! To allocate size %d but free size %d.",
        size, buff_size_free);
#if defined(CHIP_HAS_CP)&&defined(UNIFY_HEAP_ENABLED)
    if(overlay_addr == NULL){
        *buff = __cp_mem_pool_start__ + syspool_used;
    }else{
        *buff = overlay_addr + syspool_used;
    }
#else
    *buff = __mem_pool_start__ + syspool_used;
#endif
    syspool_used += size;
    TRACE(0, "[%s] ptr=%p size=%u free=%u user=%p", __func__, *buff, size, buff_size_free, __builtin_return_address(0));
    return buff_size_free;
}

int syspool_get_available(uint8_t **buff)
{
    uint32_t buff_size_free;
    buff_size_free = syspool_free_size();

    TRACE(2, "[%s] free=%d", __func__, buff_size_free);
    if (buff_size_free < 8)
        return -1;
    if (buff != NULL)
    {
#if defined(CHIP_HAS_CP)&&defined(UNIFY_HEAP_ENABLED)
        if(overlay_addr == NULL){
            *buff = __cp_mem_pool_start__ + syspool_used;
        }else{
            *buff = overlay_addr + syspool_used;
        }
#else
        *buff = __mem_pool_start__ + syspool_used;
#endif
        syspool_used += buff_size_free;
    }
    return buff_size_free;
}

static void *pool_malloc(size_t size)
{
    void *ptr = NULL;
    syspool_get_buff((uint8_t **)&ptr, size);
    ASSERT(ptr, "[%s] no memory", __FUNCTION__);
    return ptr;
}

static void *pool_calloc(size_t nmemb, size_t size)
{
    if (size == 0)
        return NULL;

    void *ptr = pool_malloc(nmemb * size);

    if (ptr)
    {
        memset(ptr, 0 , nmemb * size);
    }

    return ptr;
}

static void pool_free(void *p)
{
}

static custom_allocator allocator = {
    .malloc = pool_malloc,
    .calloc = pool_calloc,
    .free = pool_free,
};

custom_allocator *pool_allocator(void)
{
    return &allocator;
}

#if defined(A2DP_LDAC_ON)
int syspool_force_used_size(uint32_t size)
{
    return syspool_used  = size;
}
#endif
#if !defined(UNIFY_HEAP_ENABLED)
static uint32_t share_pool_size = 0;
void share_pool_init(void)
{
    share_pool_size = __mem_pool_end__ - __mem_pool2_start__;
    if (share_pool_size) {
        memset(__mem_pool2_start__,0,share_pool_size);
    }
    TRACE(3,"%s:%p,0x%x",__FUNCTION__,__mem_pool2_start__,share_pool_size);
}

uint8_t* share_pool_start_addr(void)
{
    return __mem_pool2_start__;
}

int share_pool_free_size(void)
{
    return share_pool_size;
}

#if defined(CHIP_HAS_CP) && (RAMCP_SIZE > 0)

static uint8_t* cp_pool_init_addr = NULL;
static uint32_t cp_pool_size = 0;
static uint32_t cp_pool_used = 0;

void cp_pool_init(void)
{
    if(overlay_addr == NULL)
        cp_pool_init_addr = __cp_mem_pool_start__;
    else
        cp_pool_init_addr = overlay_addr;
    cp_pool_size = __cp_mem_pool_end__ - cp_pool_init_addr;
    cp_pool_used = 0;
    TRACE(5,"[CP] %s start %p end %p size 0x%x %d",__FUNCTION__,
            cp_pool_init_addr,
            __cp_mem_pool_end__,cp_pool_size,cp_pool_size);
    if (cp_pool_size) {
        memset(cp_pool_init_addr, 0, cp_pool_size);
    }
}

int cp_pool_free_size(void)
{
    return cp_pool_size - cp_pool_used;
}

int cp_pool_get_buff(uint8_t **buff, uint32_t size)
{
    uint32_t buff_size_free;

    size = (size + 3) & ~3;
    buff_size_free = cp_pool_free_size();

    if (size <= buff_size_free) {
        *buff = cp_pool_init_addr + cp_pool_used;
        cp_pool_used += size;
    } else {
        *buff = NULL;
    }
    TRACE(4,"[CP] %s get %d used %d free %d",__FUNCTION__,
            size,cp_pool_used,buff_size_free);
    return buff_size_free;
}

uint8_t* cp_pool_start_addr(void)
{
    return cp_pool_init_addr;
}

uint32_t cp_pool_total_size(void)
{
    return cp_pool_size;
}
#endif
#endif
