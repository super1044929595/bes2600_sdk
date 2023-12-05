#include "omni_platform_common.h"
#include "sw_event.h"



os_omni_event_t omni_create_event(os_omni_event_func_t handle,const void *eventcontext)
{
#if OS_OMNI_ATMOSTIC_EANBLE
	return sw_event_alloc(handle,eventcontext);
#endif
}



void os_omni_event_free(os_omni_event_t event_id)
{
#if OS_OMNI_ATMOSTIC_EANBLE
	sw_event_free(event_id);
#endif
}



void os_omni_event_set(os_omni_event_t event_id)
{
#if OS_OMNI_ATMOSTIC_EANBLE
	sw_event_set(event_id);
#endif
}


void os_omni_event_clear(os_omni_event_t event_id)
{
#if OS_OMNI_ATMOSTIC_EANBLE
	sw_event_clear(event_id);
#endif
}


bool os_omni_event_get(os_omni_event_t event_id)
{
#if OS_OMNI_ATMOSTIC_EANBLE
	return sw_event_get(event_id);
#endif
}

