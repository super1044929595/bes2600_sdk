#ifndef  __XLOG_FLASH_H__
#define  __XLOG_FLASH_H__

#include "xOS_Parame_Configure.h"
#include "xlog.h"
#include "stdint.h"
#ifdef __cplusplus
	extern "C" {
#endif

#ifdef XOS_WQ_SDK_ENABLE
#include "types.h"
#include "string.h"
#include "crc.h"
#include "iot_memory_config.h"
#include "iot_flash.h"
#include "iot_cache.h"
#include "oem.h"
#include "os_mem.h"
#include "string.h"
#include "crc.h"
#include "iot_boot_map.h"
int xlog__custom_data_erase(void);
int xlog__custom_data_write(uint16_t offset, void *data, uint16_t len);
int xlog_custom_data_get(uint32_t offset,uint8_t **data);
#endif



#ifdef __cplusplus
    }  
#endif

#endif
