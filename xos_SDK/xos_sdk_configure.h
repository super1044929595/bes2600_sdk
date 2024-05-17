#ifndef  __XOS_SDK_CONFIGURE_H_

//#include "xos_systeminfo.h"
#include "xos_timer.h"

#ifdef __cplusplus
	extern "C"{
#endif

#define XOS_BES_SDK_ENABLE

//  --OS SDK--
//              |
//              |---xos time  function
					#define XOS_TIMER_FUNCTION_ENABLE
//              |--- xshell function 
//              |---#define XOS_TWS_OVERDISTANCE_ENABLE
//              |--- xos overdistance function 
				    #define XOS_OVERDIS_FUNCTION_ENABLE

#ifdef __cplusplus
	}
#endif
#endif
