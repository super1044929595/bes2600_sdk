#ifndef __OMNI_PLATFORM_COMMON_H
#define __OMNI_PLATFORM_COMMON_H


#include <stdbool.h>
#include <stdint.h>
#include <inttypes.h>
#include <stddef.h>     // standard definition
#include "omni_os_sdk_configure.h"

#ifdef __cplusplus
	extern "C"{
#endif

//os event  members
typedef unsigned char uint8_t; 
typedef uint8_t os_omni_event_t;
typedef void (*os_omni_event_func_t)(os_omni_event_t event_id, const void *ctx);

/**
 * @brief Allocate and configure event
 * @param[in] handler  Event triggered callback (called from main event loop)
 * @param[in] ctx      Context to pass to handler
 * @return             Event ID
 */
os_omni_event_t omni_create_event(os_omni_event_func_t handle,const void *eventcontext);


/**
 * @brief Deallocate event
 * @note Caller must guarantee that set/clear methods are not invoked from
 *       an ISR while this method is running
 * @para
 */
void os_omni_event_free(os_omni_event_t event_id);

/**
 * @brief Trigger event
 * @note Safe to be called from ISR
 * @param[in] event_id  Event ID from sw_event_alloc()
 */

void os_omni_event_set(os_omni_event_t event_id);

/**
 * @brief Clear event
 * @note Safe to be called from ISR
 * @param[in] event_id  Event ID from sw_event_alloc()
 */
void os_omni_event_clear(os_omni_event_t event_id);

/**
 * @brief Get event status
 * @param[in] event_id  Event ID from sw_event_alloc()
 * @return              True when event is valid and triggered
 */
bool os_omni_event_get(os_omni_event_t event_id);


#ifdef __cplusplus
		}
#endif
#endif


