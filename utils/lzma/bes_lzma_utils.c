#ifdef OTA_BIN_COMPRESSED
#include "hal_trace.h"
#include "hal_timer.h"
#include "hal_bootmode.h"
#include "hal_norflash.h"
#include "hal_cmu.h"
#include "hal_wdt.h"
#include "pmu.h"
#include "tool_msg.h"
#include "crc16.h"
#include "crc32.h"
#include "cmsis_nvic.h"
#include "string.h"
#include "ota_control.h"
#include "hal_cache.h"
#define LOG_TAG "[OTA_CONTROL] "

#include "heap_api.h"
#include "bes_lzma_api.h"

#ifndef FLASH_SECTOR_SIZE_IN_BYTES
#define FLASH_SECTOR_SIZE_IN_BYTES 4096
#endif

#define OTA_DATA_BUFFER_SIZE_FOR_BURNING FLASH_SECTOR_SIZE_IN_BYTES

#ifdef USE_MULTI_FLASH
#define SRC_FLASH_ID             HAL_FLASH_ID_1
#define DST_FLASH_ID             HAL_FLASH_ID_0
#define SRC_FLASH_LOGIC_ADDR     OTA_FLASH1_LOGIC_ADDR
#define DST_FLASH_LOGIC_ADDR     OTA_FLASH_LOGIC_ADDR
#else
#define SRC_FLASH_ID             HAL_FLASH_ID_0
#define DST_FLASH_ID             HAL_FLASH_ID_0
#define SRC_FLASH_LOGIC_ADDR     OTA_FLASH_LOGIC_ADDR
#define DST_FLASH_LOGIC_ADDR     OTA_FLASH_LOGIC_ADDR
#endif

extern FLASH_OTA_BOOT_INFO_T otaBootInfoInFlash;

static uint32_t decompressed[128*1024/4];

#define SANITY_CRC_CHECK
#ifdef SANITY_CRC_CHECK
static const char* image_info_sanity_crc_key_word = "CRC32_OF_IMAGE=0x";

static uint8_t asciiToHex(uint8_t asciiCode)
{
    if ((asciiCode >= '0') && (asciiCode <= '9'))
    {
        return asciiCode - '0';
    }
    else if ((asciiCode >= 'a') && (asciiCode <= 'f'))
    {
        return asciiCode - 'a' + 10;
    }
    else if ((asciiCode >= 'A') && (asciiCode <= 'F'))
    {
        return asciiCode - 'A' + 10;
    }
    else
    {
        return 0xff;
    }
}

static int32_t find_key_word(uint8_t *targetArray, uint32_t targetArrayLen,
                             uint8_t *keyWordArray, uint32_t keyWordArrayLen)
{
    if ((keyWordArrayLen > 0) && (targetArrayLen >= keyWordArrayLen))
    {
        uint32_t index = 0, targetIndex = 0;
        for (targetIndex = 0;targetIndex < targetArrayLen;targetIndex++)
        {
            for (index = 0;index < keyWordArrayLen;index++)
            {
                if (targetArray[targetIndex + index] != keyWordArray[index])
                {
                    break;
                }
            }

            if (index == keyWordArrayLen)
            {
                return targetIndex;
            }
        }

        return -1;
    }
    else
    {
        return -1;
    }
}
#endif

static __inline uint32_t __bswap32(uint32_t __x)
{
    return (__x>>24) | (__x>>8&0xff00) | (__x<<8&0xff0000) | (__x<<24);
}

static inline unsigned long crc_update(unsigned long crc, const unsigned char *buf, unsigned int len)
{
#ifdef __CRC16__
    return crc16ccitt(crc, (uint8_t *)buf, 0, len);
#else
    return crc32(crc, (uint8_t *)buf, len);
#endif
}

static void update_magic_number(uint32_t newMagicNumber)
{
    uint32_t lock;

    uint32_t addr = (uint32_t)&otaBootInfoInFlash;

    lock = int_lock();
    pmu_flash_write_config();
    hal_norflash_erase(HAL_FLASH_ID_0, addr, FLASH_SECTOR_SIZE_IN_BYTES);
    hal_norflash_write(HAL_FLASH_ID_0, addr, (uint8_t*)(&newMagicNumber), 4);
    pmu_flash_read_config();
    int_unlock(lock);
}

void ota_copy_compressed_image(uint32_t srcFlashOffset, uint32_t dstFlashOffset)
{
    FLASH_OTA_BOOT_INFO_T otaBootInfo;
    memcpy((uint8_t *)&otaBootInfo, (uint8_t *)&otaBootInfoInFlash, sizeof(FLASH_OTA_BOOT_INFO_T));

    TRACE(0,"ota boot info imageSize is: %d, imageCrc=0x%x",otaBootInfo.imageSize,otaBootInfo.imageCrc);
    DUMP8("0x%02x ", (uint8_t *)&otaBootInfoInFlash, sizeof(FLASH_OTA_BOOT_INFO_T));

    struct boot_hdr_t bakup_hdr;
    uint32_t readOffset = 0;
    uint32_t wrtieOffset = 0;
    uint8_t crc_mark_str[16];
#ifdef __CRC16__
    uint32_t crcValue = 0xFFFF;
#else
    uint32_t crcValue = 0;
#endif
#ifdef SANITY_CRC_CHECK
    uint32_t sanityCrc32 = 0;
    uint32_t crcValue_dcom = 0;
#endif
    uint32_t lock;

#ifdef __WATCHER_DOG_RESET__
    uint32_t copiedDataSizeSinceLastWdPing = 0;
    uint32_t erasedDataSizeSinceLastWdPing = 0;
#endif
    int ret;
    syspool_init();
    bes_lzma_dec_init(syspool_start_addr(), syspool_total_size());
    uint32_t magic_number = 0xffffffff;
    memcpy((void*)&magic_number,(const uint8_t*)SRC_FLASH_LOGIC_ADDR + srcFlashOffset + readOffset,4);
    readOffset += 4;
    if (magic_number != NORMAL_BOOT)
    {
        TRACE(0,"Wrong magic number =0x%x",magic_number);
        return;
    }
    TRACE(0,"magic number =0x%x",magic_number);
    magic_number = 0xFFFFFFFF;
    crcValue = crc_update(crcValue, (const unsigned char*)&magic_number, 4);
    while (readOffset < otaBootInfo.imageSize)
    {
        TRACE(0,"readOffset=%d,wrtieOffset=%d,srcFlashOffset=0x%x,dstFlashOffset=0x%x",readOffset,wrtieOffset,srcFlashOffset,dstFlashOffset);
        lock = int_lock();
        pmu_flash_write_config();
        uint32_t next_size = 0;
        uint32_t output_size = 128*1024;

        memcpy((void*)&next_size,(const uint8_t *)SRC_FLASH_LOGIC_ADDR + srcFlashOffset + readOffset,4);
        readOffset += 4;
        crcValue = crc_update(crcValue, (const unsigned char*)&next_size, 4);

        TRACE(0,"readOffset=%d,next_size=0x%x,swap=0x%x",readOffset,next_size,__bswap32(next_size));
        next_size = __bswap32(next_size);
        void *srcbuf = bes_lzma_malloc(next_size);
        memcpy((void*)srcbuf,(const uint8_t*)SRC_FLASH_LOGIC_ADDR + srcFlashOffset + readOffset,next_size);
        readOffset += next_size;
        crcValue = crc_update(crcValue, (const unsigned char*)srcbuf, next_size);
        memset(decompressed,0,output_size);
        ret = bes_lzma_dec_buf((uint8_t *)srcbuf,next_size,(uint8_t *)decompressed,&output_size);
        TRACE(0,"bes_lzma_dec_buf ret=%d",ret);
        bes_lzma_free(srcbuf);

        if (0 == wrtieOffset)
        {
            memcpy(&bakup_hdr,(const void *)decompressed,sizeof(bakup_hdr));
        }

        hal_norflash_erase(DST_FLASH_ID, dstFlashOffset + wrtieOffset,
            output_size);
        hal_norflash_write(DST_FLASH_ID, dstFlashOffset + wrtieOffset,
            (const uint8_t *)decompressed, output_size);

#if OTA_NON_CACHE_READ_ISSUE_WORKAROUND
        hal_cache_invalidate(HAL_CACHE_ID_D_CACHE,
            DST_FLASH_LOGIC_ADDR + dstFlashOffset + wrtieOffset,
            output_size);
#endif

        pmu_flash_read_config();

        int_unlock(lock);
        wrtieOffset += output_size;

    //TRACE(1,"#WDT TIME LEFT %d", hal_wdt_get_timeleft(HAL_WDT_ID_0));
    #ifdef __WATCHER_DOG_RESET__
        copiedDataSizeSinceLastWdPing += wrtieOffset;
        if (copiedDataSizeSinceLastWdPing > 20*OTA_DATA_BUFFER_SIZE_FOR_BURNING)
        {
            hal_wdt_ping(HAL_WDT_ID_0);
            //TRACE(1,"WDT TIME LEFT %d", hal_wdt_get_timeleft(HAL_WDT_ID_0));
            #ifndef CHIP_BEST2000
                pmu_wdt_feed();
            #endif
            copiedDataSizeSinceLastWdPing = 0;
        }
    #endif
        memcpy((void*)&crc_mark_str,(const uint8_t *)SRC_FLASH_LOGIC_ADDR + srcFlashOffset + readOffset,16);
        if (strncmp((const char *)crc_mark_str,"CRC32_OF_IMAGE=",15) == 0)
        {
#ifdef SANITY_CRC_CHECK
            int32_t sanity_crc_location = find_key_word((uint8_t *)decompressed,
                                                        output_size,
                                                        (uint8_t *)image_info_sanity_crc_key_word,
                                                        strlen(image_info_sanity_crc_key_word));

            TRACE(2,"sanity_crc_location is %d,output_size=%d", sanity_crc_location,output_size);

            uint8_t *crcString = (uint8_t *)((uint8_t *)decompressed + sanity_crc_location + strlen(image_info_sanity_crc_key_word));
            for (uint8_t index = 0; index < 8; index++)
            {
                sanityCrc32 |= (asciiToHex(crcString[index]) << (28 - 4 * index));
            }
            crcValue_dcom = crc_update(crcValue_dcom, (const unsigned char*)decompressed, sanity_crc_location+ strlen(image_info_sanity_crc_key_word));
            TRACE(2,"sanityCrc32 is 0x%x, crcValue_dcom is 0x%x", sanityCrc32,crcValue_dcom);
#endif
            break;
        }
#ifdef SANITY_CRC_CHECK
        else
        {
            crcValue_dcom = crc_update(crcValue_dcom, (const unsigned char*)decompressed, output_size);
        }
#endif
    }

    void *srcbuf = bes_lzma_malloc(otaBootInfo.imageSize-readOffset);
    if (srcbuf)
    {
        memset(srcbuf,0,otaBootInfo.imageSize-readOffset);
        TRACE(0,"readOffset=%d,remain=%d",readOffset,otaBootInfo.imageSize-readOffset);
        memcpy((void*)srcbuf,(const uint8_t*)SRC_FLASH_LOGIC_ADDR + srcFlashOffset + readOffset,26);
        crcValue = crc_update(crcValue, (const unsigned char*)srcbuf, otaBootInfo.imageSize-readOffset);
        bes_lzma_free(srcbuf);
    }
    TRACE(2,"Original CRC32 is 0x%x Confirmed CRC32 is 0x%x.", otaBootInfo.imageCrc, crcValue);
#ifdef SANITY_CRC_CHECK
    if (crcValue == otaBootInfo.imageCrc && (sanityCrc32 == crcValue_dcom || 0 == sanityCrc32))
#else
    if (crcValue == otaBootInfo.imageCrc)
#endif
    {
        lock = int_lock();
        pmu_flash_write_config();
        memcpy(decompressed,(const uint8_t*)SRC_FLASH_LOGIC_ADDR + dstFlashOffset,FLASH_SECTOR_SIZE_IN_BYTES);
        bakup_hdr.magic = NORMAL_BOOT;
        memcpy(decompressed,(const void *)&bakup_hdr,sizeof(bakup_hdr));
        hal_norflash_erase(DST_FLASH_ID, dstFlashOffset, FLASH_SECTOR_SIZE_IN_BYTES);
        hal_norflash_write(DST_FLASH_ID, dstFlashOffset, (uint8_t*)(decompressed), FLASH_SECTOR_SIZE_IN_BYTES);
        pmu_flash_read_config();
        int_unlock(lock);

        uint32_t erasedDataSize = 0;
        while (erasedDataSize < otaBootInfo.imageSize)
        {
            lock = int_lock();
            pmu_flash_write_config();

            hal_norflash_erase(SRC_FLASH_ID, (SRC_FLASH_LOGIC_ADDR + srcFlashOffset + erasedDataSize),
                    OTA_DATA_BUFFER_SIZE_FOR_BURNING);

            pmu_flash_read_config();
            int_unlock(lock);
            erasedDataSize += OTA_DATA_BUFFER_SIZE_FOR_BURNING;

    #ifdef __WATCHER_DOG_RESET__
            erasedDataSizeSinceLastWdPing += erasedDataSize;
            if (erasedDataSizeSinceLastWdPing > 20*OTA_DATA_BUFFER_SIZE_FOR_BURNING)
            {
                hal_wdt_ping(HAL_WDT_ID_0);
                #ifndef CHIP_BEST2000
                    pmu_wdt_feed();
                #endif
                erasedDataSizeSinceLastWdPing = 0;
            }
    #endif
        }
        update_magic_number(NORMAL_BOOT);
    }

    if (crcValue == otaBootInfo.imageCrc)
    {
        hal_sw_bootmode_set(HAL_SW_BOOTMODE_REBOOT);
        pmu_reboot();
        while (1) {}
    }
}
#endif

