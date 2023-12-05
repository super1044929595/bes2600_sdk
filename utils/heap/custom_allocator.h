#ifndef CUSTOM_ALLOCATOR
#define CUSTOM_ALLOCATOR

#include "stddef.h"

typedef struct
{
    void* (*malloc)(size_t);
    void* (*calloc)(size_t, size_t);
    void (*free)(void*);
} custom_allocator;

#ifdef __cplusplus
extern "C" {
#endif

custom_allocator *default_allocator(void);

custom_allocator *pool_allocator(void);

#ifdef __cplusplus
}
#endif

#endif
