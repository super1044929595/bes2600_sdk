
#include "xlog.h"
#include "xlog_flash.h"
#ifdef XOS_WQ_SDK_ENABLE



#define IMAGE_TYPE_XLOG_CUSTOM     IMAGE_TYPE_CUS2
#define FLASH_LENGTH_XLOG          FLASH_CUS2_LENGTH



static uint32_t xlog_custom_data_get_base(void)
{
    iot_boot_map_image_t image;
    uint8_t ret;

    ret = iot_boot_map_get_image(&image, IMAGE_TYPE_XLOG_CUSTOM);
    if (ret != RET_OK) {
        return 0;
    }

    return image.code_lma;
}

int xlog_custom_data_get(uint32_t offset,uint8_t **data)
{
    uint32_t base;

    if (FLASH_LENGTH_XLOG == 0) {
        return RET_FAIL;
    }

    base = xlog_custom_data_get_base();
    if (base == 0) {
        return RET_NOT_EXIST;
    }

    *data = (uint8_t *)(FLASH_START + base + offset);

    return 0;
}

int xlog__custom_data_erase(void)
{
    uint32_t base;

    if (FLASH_LENGTH_XLOG == 0) {
        return RET_FAIL;
    }

    base = xlog_custom_data_get_base();
    if (base == 0) {
        return RET_NOT_EXIST;
    }

    return iot_flash_erase_range(base, FLASH_LENGTH_XLOG);
}

int xlog__custom_data_write(uint16_t offset, void *data, uint16_t len)
{
    int ret;
    uint32_t base;

    if (FLASH_LENGTH_XLOG == 0) {
        return RET_FAIL;
    }

    base = xlog_custom_data_get_base();
    if (base == 0) {
        return RET_NOT_EXIST;
    }

    assert(offset + len <= FLASH_LENGTH_XLOG);

    ret = iot_flash_write_without_erase(base + offset, data, len);

    iot_cache_invalidate(IOT_CACHE_SFC_ID, FLASH_START + base, FLASH_LENGTH_XLOG);

    return ret;
}

#endif