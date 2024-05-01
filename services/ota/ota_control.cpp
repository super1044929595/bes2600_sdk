#include "hal_iomux.h"
#include "hal_trace.h"
#include "hal_timer.h"
#include "hwtimer_list.h"
#include "hal_bootmode.h"
#include "hal_norflash.h"
#include "hal_cmu.h"
#include "pmu.h"
#include "crc32.h"
#include "cmsis_nvic.h"
#include "string.h"
#include "ota_control.h"
#include "hal_cache.h"
#include "factory_section.h"
#include "app_tws_ibrt.h"
#include "bt_drv_interface.h"
#include "ota_spp.h"
#include "ota_bes.h"
#include "rwapp_config.h"
#include "hal_wdt.h"
#include "norflash_api.h"
#include "nvrecord.h"
#include "factory_section.h"
#if (BLE_APP_OTA)
#include "app_ble_rx_handler.h"
#endif
#include "apps.h"
#include "app_utils.h"
#include "app_bt.h"
#include "hal_aud.h"
#include "btapp.h"
#include "app_tws_ctrl_thread.h"
#include "app_ibrt_ota_cmd.h"
#ifdef __GATT_OVER_BR_EDR__
#include "btgatt_api.h"
#endif

#ifdef IBRT
#include "app_tws_ibrt.h"
#include "app_ibrt_ui.h"
#endif
#include "app.h"
#include "app_ibrt_if.h"

#define DelayMs(a)      hal_sys_timer_delay(MS_TO_TICKS(a))

#define OTA_CONTROL_NORFLASH_BUFFER_LEN    (FLASH_SECTOR_SIZE_IN_BYTES*2)
#define OTA_DATA_PKT_SYNC_MS                1
#define PROMPT_MAX_SIZE                    (0x80000)

#if PUYA_FLASH_ERASE_LIMIT
static uint8_t upsize_segment_cnt = 0;
static uint8_t upsize_erase_cnt = 0;
#define UPSIZE_ELEMENT_LIMIT    4           //No more than UPSIZE_ELEMENT_LIMIT+1 Writes after a Erase
#define UPSIZE_WRITE_FREQ       10          //Write once every UPSIZE_WRITE_FREQ usual upgrade
#endif

extern uint32_t __ota_upgrade_log_start[];
extern OtaContext ota;
uint32_t new_image_copy_flash_offset;

#define LEN_OF_IMAGE_TAIL_TO_FIND_SANITY_CRC    512
static const char* image_info_sanity_crc_key_word = "CRC32_OF_IMAGE=0x";

FLASH_OTA_UPGRADE_LOG_FLASH_T *otaUpgradeLogInFlash = (FLASH_OTA_UPGRADE_LOG_FLASH_T *)__ota_upgrade_log_start;
#define otaUpgradeLog   (*otaUpgradeLogInFlash)
FLASH_OTA_BOOT_INFO_T otaBootInfoInFlash = { NORMAL_BOOT, 0, 0} ;

OTA_IBRT_TWS_CMD_EXECUTED_RESULT_FROM_SLAVE_T receivedResultAlreadyProcessedBySlave = {0xFF, 0, 0, 0};

#define HW_VERSION_LENTH        4    //sizeof(FIRMWARE_REV_INFO_T)
uint8_t peerFwRevInfo[HW_VERSION_LENTH] = {0};
static uint8_t ibrt_tws_mode = 0;

static uint8_t isInBesOtaState = false;
static const bool norflash_api_mode_async = true;
extern "C" void system_get_info(uint8_t *fw_rev_0, uint8_t *fw_rev_1,  uint8_t *fw_rev_2, uint8_t *fw_rev_3);
void ota_status_change(bool status);
OTA_CONTROL_ENV_T ota_control_env __attribute__((aligned(4)));
static bool ota_control_check_image_crc(void);
static uint32_t user_data_nv_flash_offset;
static void app_update_ota_boot_info(FLASH_OTA_BOOT_INFO_T* otaBootInfo);
static void app_update_magic_number_of_app_image(uint32_t newMagicNumber);

static void BesOtaErase(uint32_t flashOffset);
static void BesOtaErasefactory(uint32_t flashOffset);
static void BesOtaProgram(uint32_t flashOffset, uint8_t* ptr, uint32_t len);
static void BesOtaProgramfactory(uint32_t flashOffset, uint8_t* ptr, uint32_t len);
static void BesFlushPendingFlashOp(enum NORFLASH_API_OPRATION_TYPE type);

static const char *typeCode2Str(uint8_t typeCode) {
    #define CASE_M(s) \
        case s: return "["#s"]";

    switch(typeCode) {
    CASE_M(OTA_COMMAND_START)
    CASE_M(OTA_RSP_START)
    CASE_M(OTA_COMMAND_SEGMENT_VERIFY)
    CASE_M(OTA_RSP_SEGMENT_VERIFY)
    CASE_M(OTA_RSP_RESULT)
    CASE_M(OTA_DATA_PACKET)
    CASE_M(OTA_COMMAND_CONFIG_OTA)
    CASE_M(OTA_RSP_CONFIG)
    CASE_M(OTA_COMMAND_GET_OTA_RESULT)
    CASE_M(OTA_READ_FLASH_CONTENT)
    CASE_M(OTA_FLASH_CONTENT_DATA)
    CASE_M(OTA_DATA_ACK)
    CASE_M(OTA_COMMAND_RESUME_VERIFY)
    CASE_M(OTA_RSP_RESUME_VERIFY)
    CASE_M(OTA_COMMAND_GET_VERSION)
    CASE_M(OTA_RSP_VERSION)
    CASE_M(OTA_COMMAND_SIDE_SELECTION)
    CASE_M(OTA_RSP_SIDE_SELECTION)
    CASE_M(OTA_COMMAND_IMAGE_APPLY)
    CASE_M(OTA_RSP_IMAGE_APPLY)
    }

    return "[]";
}

#ifndef __IAG_BLE_INCLUDE__
#define VOICEPATH_COMMON_OTA_BUFF_SIZE 4096

uint8_t* app_voicepath_get_common_ota_databuf(void)
{
    static uint8_t voicepath_common_ota_buf[VOICEPATH_COMMON_OTA_BUFF_SIZE];
    return voicepath_common_ota_buf;
}
#endif

uint8_t app_get_bes_ota_state(void)
{
    return isInBesOtaState;
}

static void Bes_enter_ota_state(void)
{
    if (isInBesOtaState)
    {
        return;
    }
#ifdef IBRT
    ibrt_ctrl_t *p_ibrt_ctrl = app_tws_ibrt_get_bt_ctrl_ctx();
    btdrv_reg_op_set_tws_link_duration(IBRT_TWS_LINK_LARGE_DURATION);
    // btdrv_reg_op_set_private_tws_poll_interval(p_ibrt_ctrl->config.default_private_poll_interval,
    //                                            p_ibrt_ctrl->config.default_private_poll_interval_in_sco);
#endif
    // 1. switch to the highest freq
    app_sysfreq_req(APP_SYSFREQ_USER_OTA, APP_SYSFREQ_208M);

    // 2. exit bt sniff mode
#ifdef IBRT
    app_ibrt_ui_judge_link_policy(OTA_START_TRIGGER, BTIF_BLP_DISABLE_ALL);

    if (app_tws_ibrt_tws_link_connected() && \
        (p_ibrt_ctrl->nv_role == IBRT_MASTER) && \
        p_ibrt_ctrl->p_tws_remote_dev)
    {
        btif_me_stop_sniff(p_ibrt_ctrl->p_tws_remote_dev);
    }
#else
    app_bt_active_mode_set(ACTIVE_MODE_KEEPER_OTA, UPDATE_ACTIVE_MODE_FOR_ALL_LINKS);
#endif

#ifdef __IAG_BLE_INCLUDE__
    // 3. update BLE conn parameter
    if (DATA_PATH_BLE == ota_control_get_datapath_type())
    {
        app_ble_update_conn_param_mode(BLE_CONN_PARAM_MODE_OTA, true);
    }
#endif
    isInBesOtaState = true;
}

void Bes_exit_ota_state(void)
{
    if (!isInBesOtaState)
    {
        return;
    }

#ifdef IBRT
    //app_ibrt_ui_t *p_ibrt_ui = app_ibrt_ui_get_ctx();
    btdrv_reg_op_set_tws_link_duration(IBRT_TWS_LINK_DEFAULT_DURATION);
    // btdrv_reg_op_set_private_tws_poll_interval(p_ibrt_ui->config.long_private_poll_interval,
    //                                            p_ibrt_ui->config.default_private_poll_interval_in_sco);
#endif

    app_sysfreq_req(APP_SYSFREQ_USER_OTA, APP_SYSFREQ_32K);
#ifdef IBRT
    app_ibrt_ui_judge_link_policy(OTA_STOP_TRIGGER, BTIF_BLP_SNIFF_MODE);
#else
    app_bt_active_mode_clear(ACTIVE_MODE_KEEPER_OTA, UPDATE_ACTIVE_MODE_FOR_ALL_LINKS);
#endif

#ifdef __IAG_BLE_INCLUDE__
    app_ble_update_conn_param_mode(BLE_CONN_PARAM_MODE_OTA, false);
    app_ble_update_conn_param_mode(BLE_CONN_PARAM_MODE_OTA_SLOWER, false);
#endif

    isInBesOtaState = false;
}

static void ota_update_userdata_pool(void)
{

    if (ota_control_env.configuration.isToClearUserData)
    {
        nv_record_rebuild();
    }

    if (ota_control_env.configuration.isToRenameBT || ota_control_env.configuration.isToRenameBLE ||
        ota_control_env.configuration.isToUpdateBTAddr || ota_control_env.configuration.isToUpdateBLEAddr)
    {
        factory_section_t* pOrgFactoryData, *pUpdatedFactoryData;
        pOrgFactoryData = (factory_section_t *)(OTA_FLASH_LOGIC_ADDR + ota_control_env.flasehOffsetOfFactoryDataPool);
        memcpy(ota_control_env.dataBufferForBurning, (uint8_t *)pOrgFactoryData,
            FLASH_SECTOR_SIZE_IN_BYTES);
        pUpdatedFactoryData = (factory_section_t *)(ota_control_env.dataBufferForBurning);
        uint32_t nv_record_dev_rev = factory_section_get_version();

        if (1 == nv_record_dev_rev)
        {
            if (ota_control_env.configuration.isToRenameBT)
            {
//                memset(pUpdatedFactoryData->data.device_name, 0, sizeof(pUpdatedFactoryData->data.device_name));
                memcpy(pUpdatedFactoryData->data.device_name, (uint8_t *)(ota_control_env.configuration.newBTName),
                    BES_OTA_NAME_LENGTH);
            }

            if (ota_control_env.configuration.isToUpdateBTAddr)
            {
                memcpy(pUpdatedFactoryData->data.bt_address, (uint8_t *)(ota_control_env.configuration.newBTAddr),
                    BD_ADDR_LEN);
            }

            if (ota_control_env.configuration.isToUpdateBLEAddr)
            {
                memcpy(pUpdatedFactoryData->data.ble_address, (uint8_t *)(ota_control_env.configuration.newBLEAddr),
                    BD_ADDR_LEN);
            }

            pUpdatedFactoryData->head.crc =
                crc32(0,(unsigned char *)(&(pUpdatedFactoryData->head.reserved0)),
                    sizeof(factory_section_t)-2-2-4-(5+63+2+2+2+1+8)*sizeof(int));
        }
        else
        {
            if (ota_control_env.configuration.isToRenameBT) {
//              memset(pUpdatedFactoryData->data.rev2_bt_name, 0,
//                     sizeof(pUpdatedFactoryData->data.rev2_bt_name));
              memcpy(pUpdatedFactoryData->data.rev2_bt_name,
                     (uint8_t *) (ota_control_env.configuration.newBTName),
                     BES_OTA_NAME_LENGTH);
            }

            if (ota_control_env.configuration.isToRenameBLE) {
                memset(pUpdatedFactoryData->data.rev2_ble_name,
                    0, sizeof(pUpdatedFactoryData->data.rev2_ble_name));
                memcpy(pUpdatedFactoryData->data.rev2_ble_name,
                    (uint8_t *)(ota_control_env.configuration.newBLEName),
                    BES_OTA_NAME_LENGTH);
            }

            if (ota_control_env.configuration.isToUpdateBTAddr) {
              memcpy(pUpdatedFactoryData->data.rev2_bt_addr,
                     (uint8_t *) (ota_control_env.configuration.newBTAddr),
                     BD_ADDR_LEN);
            }

            if (ota_control_env.configuration.isToUpdateBLEAddr) {
              memcpy(pUpdatedFactoryData->data.rev2_ble_addr,
                     (uint8_t *) (ota_control_env.configuration.newBLEAddr),
                     BD_ADDR_LEN);
            }

             pUpdatedFactoryData->head.crc =
                crc32(0,(unsigned char *)(&(pUpdatedFactoryData->head.reserved0)),
                    sizeof(factory_section_t)-2-2-4-(5+63+2+2+2+1+8)*sizeof(int));
            pUpdatedFactoryData->data.rev2_crc =
                crc32(0,(unsigned char *)(&(pUpdatedFactoryData->data.rev2_reserved0)),
                pUpdatedFactoryData->data.rev2_data_len);
        }

        BesOtaErasefactory(ota_control_env.flasehOffsetOfFactoryDataPool);
        BesOtaProgramfactory(ota_control_env.flasehOffsetOfFactoryDataPool,
            (uint8_t *)pUpdatedFactoryData, FLASH_SECTOR_SIZE_IN_BYTES);
    }

    BesFlushPendingFlashOp(NORFLASH_API_ALL);
}

/**
 * @brief update MTU, called when the MTU exchange indication is received. The MTU exchange request is sent
 *       right after connection is created.
 *
 * @param mtu    MTU size to update
 *
 */
void ota_control_update_MTU(uint16_t mtu)
{
#if(OTA_OVER_TOTA_ENABLED)
    mtu = (mtu / 16) * 16 - 4;
#endif
    // remove the 3 bytes of overhead
    ota_control_env.dataPacketSize = mtu - 3;
    LOG_DBG("updated data packet size is %d", ota_control_env.dataPacketSize);
}

/**
 * @brief Register the data transmission handler. Called from SPP or BLE layer
 *
 * @param transmit_handle    Handle of the transmission handler
 *
 */
void ota_control_register_transmitter(ota_transmit_data_t transmit_handle)
{
    ota_control_env.transmitHander = transmit_handle;
}

void ota_control_set_datapath_type(uint8_t datapathType)
{
    ota_control_env.dataPathType = datapathType;
}

uint8_t ota_control_get_datapath_type(void)
{
    return ota_control_env.dataPathType;
}
/**
 * @brief Reset the environment of the OTA handling
 *
 */
void ota_control_reset_env(void)
{

    ota_control_env.configuration.startLocationToWriteImage = new_image_copy_flash_offset;
    ota_control_env.offsetInFlashToProgram = new_image_copy_flash_offset;

    ota_control_env.offsetInFlashOfCurrentSegment = ota_control_env.offsetInFlashToProgram;

    ota_control_env.configuration.isToClearUserData = true;
    ota_control_env.configuration.isToRenameBLE = false;
    ota_control_env.configuration.isToRenameBT = false;
    ota_control_env.configuration.isToUpdateBLEAddr = false;
    ota_control_env.configuration.isToUpdateBTAddr = false;
    ota_control_env.configuration.lengthOfFollowingData = 0;
    ota_control_env.AlreadyReceivedConfigurationLength = 0;
    ota_control_env.flasehOffsetOfUserDataPool = user_data_nv_flash_offset;
    ota_control_env.flasehOffsetOfFactoryDataPool = user_data_nv_flash_offset + FLASH_SECTOR_SIZE_IN_BYTES;
    ota_control_env.crc32OfSegment = 0;
    ota_control_env.crc32OfImage = 0;
    ota_control_env.offsetInDataBufferForBurning = 0;
    ota_control_env.alreadyReceivedDataSizeOfImage = 0;
    ota_control_env.offsetOfImageOfCurrentSegment = 0;
    ota_status_change(false);
    ota_control_env.isPendingForReboot = false;

    ota_control_env.leftSizeOfFlashContentToRead = 0;

    if(ota_control_env.resume_at_breakpoint == false)
    {
        ota_control_env.breakPoint = 0;
        ota_control_env.i_log = -1;
    }

    ota_control_env.dataBufferForBurning = app_voicepath_get_common_ota_databuf();
}

/**
 * @brief Send the response to start OTA control packet
 *
 *
 */
static void ota_control_send_start_response(uint8_t user)
{
    OTA_START_RSP_T tRsp =
        {OTA_RSP_START, BES_OTA_START_MAGIC_CODE, OTA_SW_VERSION, OTA_HW_VERSION,
        ota_control_env.dataPacketSize};

    if(ota_control_env.transmitHander != NULL){
        ota_control_env.transmitHander((uint8_t *)&tRsp, sizeof(tRsp));
    }
}

void ibrt_ota_send_start_response(uint8_t user)
{
    ota_control_send_start_response(user);
}

static void ota_control_send_user_response(bool isDone)
{
    OTA_RSP_USER_T tRsp = {OTA_RSP_SET_USER, isDone};
    LOG_DBG("send set user rsp:%0x %d", OTA_RSP_SET_USER,isDone);

    if(ota_control_env.transmitHander != NULL){
        ota_control_env.transmitHander((uint8_t *)&tRsp, sizeof(tRsp));
    }
}

void ibrt_ota_send_user_response(bool isDone)
{
    ota_control_send_user_response(isDone);
}

static void ota_control_send_configuration_response(bool isDone)
{
    OTA_RSP_CONFIG_T tRsp = {OTA_RSP_CONFIG, isDone};

    if(ota_control_env.transmitHander != NULL){
        ota_control_env.transmitHander((uint8_t *)&tRsp, sizeof(tRsp));
    }
}

void ibrt_ota_send_configuration_response(bool isDone)
{
    ota_control_send_configuration_response(isDone);
}

#if DATA_ACK_FOR_SPP_DATAPATH_ENABLED
static void ota_control_send_data_ack_response(void)
{
    uint8_t packeType = OTA_DATA_ACK;
    if(ota_control_env.transmitHander != NULL){
        ota_control_env.transmitHander(&packeType, sizeof(packeType));
    }
}
#endif

static void ota_control_send_version_rsp(void)
{
    uint8_t deviceType;
    OTA_RSP_OTA_VERSION_T tRsp =
    {OTA_RSP_VERSION, BES_OTA_START_MAGIC_CODE, };

    uint8_t fw_rev[4];
    system_get_info(&fw_rev[0], &fw_rev[1], &fw_rev[2], &fw_rev[3]);
    deviceType = 0x00;
    for(int i=0; i<HW_VERSION_LENTH; i++)
    {
        tRsp.leftVersion[i] = fw_rev[i];
        tRsp.rightVersion[i] = 0;
    }
    tRsp.deviceType = deviceType;
    if(ota_control_env.transmitHander != NULL){
        ota_control_env.transmitHander((uint8_t *)&tRsp, sizeof(tRsp));
    }
}

void ibrt_ota_send_version_rsp(void)
{
    ota_control_send_version_rsp();
}

void ota_control_side_selection_rsp(uint8_t success)
{
    OTA_RSP_SELECTION_T tRsp =
        {OTA_RSP_SIDE_SELECTION, success};

    if(ota_control_env.transmitHander != NULL){
        ota_control_env.transmitHander((uint8_t *)&tRsp, sizeof(tRsp));
    }
}

void ota_control_image_apply_rsp(uint8_t success)
{
    OTA_RSP_APPLY_T tRsp =
        {OTA_RSP_IMAGE_APPLY, success};
    if(ota_control_env.transmitHander != NULL){
        ota_control_env.transmitHander((uint8_t *)&tRsp, sizeof(tRsp));
    }
}

/**
 * @brief Send the response to segement verification request
 *
 * @param isPass    false if failed and central will retransmit the former segment.
 *
 */
static void ota_control_send_segment_verification_response(bool isPass)
{
    LOG_DBG("Segment of image's verification pass status is %d (1:pass 0:failed)", isPass);
    OTA_RSP_SEGMENT_VERIFY_T tRsp =
        {OTA_RSP_SEGMENT_VERIFY, isPass};
    if(ota_control_env.transmitHander != NULL){
        ota_control_env.transmitHander((uint8_t *)&tRsp, sizeof(tRsp));
    }
}

void ibrt_ota_send_segment_verification_response(bool isPass)
{
    ota_control_send_segment_verification_response(isPass);
}

/**
 * @brief Send the response to inform central that the OTA has been successful or not
 *
 */
static void ota_control_send_result_response(uint8_t isSuccessful)
{
    OTA_RSP_OTA_RESULT_T tRsp =
        {OTA_RSP_RESULT, isSuccessful};

    ibrt_ctrl_t *p_ibrt_ctrl = app_tws_ibrt_get_bt_ctrl_ctx();

    if(p_ibrt_ctrl->current_role != IBRT_MASTER)
    {
        return;
    }
    if(ota_control_env.transmitHander != NULL){
        ota_control_env.transmitHander((uint8_t *)&tRsp, sizeof(tRsp));
    }
}


void ibrt_ota_send_result_response(uint8_t isSuccessful)
{
    ota_control_send_result_response(isSuccessful);
}

static void BesFlushPendingFlashOp(enum NORFLASH_API_OPRATION_TYPE type)
{
    hal_trace_pause();
    do
    {
        norflash_api_flush();
        if (NORFLASH_API_ALL != type)
        {
            if (0 == norflash_api_get_used_buffer_count(NORFLASH_API_MODULE_ID_OTA, type))
            {
                break;
            }
        }
        else
        {
            if (norflash_api_buffer_is_free(NORFLASH_API_MODULE_ID_OTA))
            {
                break;
            }
        }

        osDelay(10);
    } while(1);

    hal_trace_continue();
}

static void BesOtaErase(uint32_t flashOffset)
{
    LOG_DBG("[%s] flashOffset=0x%x.",__func__, flashOffset);
    uint32_t lock;
    enum NORFLASH_API_RET_T ret;
    bool isFlashVacant = false;
    uint8_t flashContent = 0xff;

    flashOffset &= 0xFFFFFF;

    for(uint32_t checknum = 0 ; checknum < FLASH_SECTOR_SIZE_IN_BYTES ; checknum ++)
    {
        memcpy(&flashContent, (uint8_t *)(OTA_FLASH_LOGIC_ADDR + flashOffset + checknum),
                sizeof(uint8_t));
        if(flashContent != 0xFF)
        {
            isFlashVacant = false;
            break;
        }
        else
        {
            isFlashVacant = true;
        }

    }

    if(isFlashVacant == true)
    {
        LOG_DBG("%s: flash sector is empty, no need erase.",__func__);
        return;
    }

    do
    {
        lock = int_lock_global();
        hal_trace_pause();

        ret = norflash_api_erase(NORFLASH_API_MODULE_ID_OTA, (OTA_FLASH_LOGIC_ADDR + flashOffset),
            FLASH_SECTOR_SIZE_IN_BYTES,
            norflash_api_mode_async);

        hal_trace_continue();
        int_unlock_global(lock);

        if (NORFLASH_API_OK == ret)
        {
            LOG_DBG("%s: norflash_api_erase ok!",__func__);
            break;
        }
        else if(NORFLASH_API_BUFFER_FULL == ret)
        {
            LOG_DBG("Flash async cache overflow! To flush it.");
            BesFlushPendingFlashOp(NORFLASH_API_ERASING);
        }
        else
        {
            ASSERT(0,"GsoundOtaErase: norflash_api_erase failed. ret = %d",ret);
        }
    } while(1);
}

static void BesOtaErasefactory(uint32_t flashOffset)
{
    uint32_t lock;
    enum NORFLASH_API_RET_T ret;

    flashOffset &= 0xFFFFFF;

    do
    {
        lock = int_lock_global();
        hal_trace_pause();

        ret = norflash_api_erase(NORFLASH_API_MODULE_ID_FACTORY, flashOffset, FLASH_SECTOR_SIZE_IN_BYTES,
            norflash_api_mode_async);

        hal_trace_continue();
        int_unlock_global(lock);

        if (NORFLASH_API_OK == ret)
        {
            LOG_DBG("%s: norflash_api_erase ok!",__func__);
            break;
        }
        else if(NORFLASH_API_BUFFER_FULL == ret)
        {
            LOG_DBG("Flash async cache overflow! To flush it.");
            BesFlushPendingFlashOp(NORFLASH_API_ERASING);
        }
        else
        {
            ASSERT(0,"OtaErasefactory: norflash_api_erase failed. ret = %d, addr=%d",ret, flashOffset);
        }
    } while(1);
}

static void BesOtaProgram(uint32_t flashOffset, uint8_t* ptr, uint32_t len)
{
    LOG_DBG("[%s] flashOffset=0x%x, len=0x%x.",__func__, flashOffset, len);
    uint32_t lock;
    enum NORFLASH_API_RET_T ret;

    flashOffset &= 0xFFFFFF;

    do
    {
        lock = int_lock_global();
        hal_trace_pause();

        ret = norflash_api_write(NORFLASH_API_MODULE_ID_OTA,
            (OTA_FLASH_LOGIC_ADDR + flashOffset), ptr, len,norflash_api_mode_async);

        hal_trace_continue();

        int_unlock_global(lock);

        if (NORFLASH_API_OK == ret)
        {
            TRACE(1,"%s: norflash_api_write ok!",__func__);
            break;
        }
        else if (NORFLASH_API_BUFFER_FULL == ret)
        {
            TRACE(0,"Flash async cache overflow! To flush it.");
            BesFlushPendingFlashOp(NORFLASH_API_WRITTING);
        }
        else
        {
            ASSERT(0,"_write_flash: norflash_api_write failed. ret = %d",ret);
        }
    } while(1);
}

static void BesOtaProgramfactory(uint32_t flashOffset, uint8_t* ptr, uint32_t len)
{
    LOG_DBG("[%s] flashOffset=0x%x, len=0x%x.",__func__, flashOffset, len);
    uint32_t lock;
    enum NORFLASH_API_RET_T ret;

    flashOffset &= 0xFFFFFF;

    do
    {
        lock = int_lock_global();
        hal_trace_pause();

        ret = norflash_api_write(NORFLASH_API_MODULE_ID_FACTORY, flashOffset, ptr, len,norflash_api_mode_async);

        hal_trace_continue();

        int_unlock_global(lock);

        if (NORFLASH_API_OK == ret)
        {
            TRACE(1,"%s: norflash_api_write ok!",__func__);
            break;
        }
        else if (NORFLASH_API_BUFFER_FULL == ret)
        {
            TRACE(0,"Flash async cache overflow! To flush it.");
            BesFlushPendingFlashOp(NORFLASH_API_WRITTING);
        }
        else
        {
            ASSERT(0,"_write_flash: norflash_api_write failed. ret = %d",ret);
        }
    } while(1);
}

/**
 * @brief Program the data in the data buffer to flash.
 *
 * @param ptrSource    Pointer of the source data buffer to program.
 * @param lengthToBurn    Length of the data to program.
 * @param offsetInFlashToProgram    Offset in bytes in flash to program
 *
 */
static void ota_control_flush_data_to_flash(uint8_t* ptrSource, uint32_t lengthToBurn, uint32_t offsetInFlashToProgram)
{
    LOG_DBG("flush %d bytes to flash offset 0x%x", lengthToBurn, offsetInFlashToProgram);

    uint32_t preBytes = (FLASH_SECTOR_SIZE_IN_BYTES - (offsetInFlashToProgram%FLASH_SECTOR_SIZE_IN_BYTES))%FLASH_SECTOR_SIZE_IN_BYTES;
    if (lengthToBurn < preBytes)
    {
        preBytes = lengthToBurn;
    }

    uint32_t middleBytes = 0;
    if (lengthToBurn > preBytes)
    {
       middleBytes = ((lengthToBurn - preBytes)/FLASH_SECTOR_SIZE_IN_BYTES*FLASH_SECTOR_SIZE_IN_BYTES);
    }
    uint32_t postBytes = 0;
    if (lengthToBurn > (preBytes + middleBytes))
    {
        postBytes = (offsetInFlashToProgram + lengthToBurn)%FLASH_SECTOR_SIZE_IN_BYTES;
    }

    LOG_DBG("Prebytes is %d middlebytes is %d postbytes is %d", preBytes, middleBytes, postBytes);

    if (preBytes > 0)
    {
        BesOtaProgram(offsetInFlashToProgram, ptrSource, preBytes);

        ptrSource += preBytes;
        offsetInFlashToProgram += preBytes;
    }

    uint32_t sectorIndexInFlash = offsetInFlashToProgram/FLASH_SECTOR_SIZE_IN_BYTES;

    if (middleBytes > 0)
    {
        uint32_t sectorCntToProgram = middleBytes/FLASH_SECTOR_SIZE_IN_BYTES;
        for (uint32_t sector = 0;sector < sectorCntToProgram;sector++)
        {
            BesOtaErase(sectorIndexInFlash*FLASH_SECTOR_SIZE_IN_BYTES);
            BesOtaProgram(sectorIndexInFlash*FLASH_SECTOR_SIZE_IN_BYTES,
                ptrSource + sector*FLASH_SECTOR_SIZE_IN_BYTES, FLASH_SECTOR_SIZE_IN_BYTES);

            sectorIndexInFlash++;
        }

        ptrSource += middleBytes;
    }

    if (postBytes > 0)
    {
        BesOtaErase(sectorIndexInFlash*FLASH_SECTOR_SIZE_IN_BYTES);
        BesOtaProgram(sectorIndexInFlash*FLASH_SECTOR_SIZE_IN_BYTES,
                ptrSource, postBytes);
    }

    BesFlushPendingFlashOp(NORFLASH_API_ALL);
}

/**
 * ota_control_flush_data_to_flash() doesn't erase sector firstly on non sector boundary addresses,
 * so the erase_segment() function is supplemented.
 */
static void erase_segment(uint32_t addr, uint32_t length)
{
    uint32_t bytes = addr & (FLASH_SECTOR_SIZE_IN_BYTES - 1);  // The number of bytes current address minus the previous sector boundary address.
    uint32_t sector_num = (bytes + length)/FLASH_SECTOR_SIZE_IN_BYTES + 1;  //The number of sectors to be erased
    if(bytes)
    {
        //memcpy(ota_control_env.dataBufferForBurning,
            //(uint8_t *)(OTA_FLASH_LOGIC_ADDR|addr), bytes);
        for(uint8_t i = 0; i < sector_num; i++)
        {
            BesOtaErase((addr - bytes + i * FLASH_SECTOR_SIZE_IN_BYTES));
        }
       // BesOtaProgram(addr, ota_control_env.dataBufferForBurning, bytes);
    }
    else
    {
        for(uint8_t i = 0; i < sector_num; i++)
        {
            BesOtaErase((addr + i * FLASH_SECTOR_SIZE_IN_BYTES));
        }
    }

}

/**
 * @brief CRC check on the whole image
 *
 * @return true if the CRC check passes, otherwise, false.
 */

static bool ota_control_check_image_crc(void)
{
    uint32_t verifiedDataSize = 0;
    uint32_t crc32Value = 0;
    uint32_t verifiedBytes = 0;
    uint32_t startFlashAddr = (ota_control_env.dstFlashOffsetForNewImage+OTA_FLASH_LOGIC_ADDR);

    LOG_DBG("%s:start word: 0x%x",__func__, *(uint32_t *)startFlashAddr);

    if (*((uint32_t *)startFlashAddr) != NORMAL_BOOT)
    {
    	LOG_DBG("%s:%d,Wrong Boot = 0x%x.",
        		__func__,__LINE__,ota_control_env.dataBufferForBurning);

        return false;
    }
    else
    {
        uint32_t firstWord = 0xFFFFFFFF;
        crc32Value = crc32(0, (uint8_t *)&firstWord, 4);
        crc32Value = crc32(crc32Value,
            (uint8_t *)(startFlashAddr + 4), ota_control_env.totalImageSize - 4);

        LOG_DBG("Original CRC32 is 0x%x Confirmed CRC32 is 0x%x.", ota_control_env.crc32OfImage, crc32Value);
        if (crc32Value == ota_control_env.crc32OfImage)
        {
            return true;
        }
        else
        {
            LOG_DBG("%s:%d,crc32Value = 0x%x,crc32OfImage = 0x%x.",
                __func__,__LINE__,crc32Value,ota_control_env.crc32OfImage);
            return false;
        }
    }

    while (verifiedDataSize < ota_control_env.totalImageSize)
    {
        if (ota_control_env.totalImageSize - verifiedDataSize > OTA_DATA_BUFFER_SIZE_FOR_BURNING)
        {
            verifiedBytes = OTA_DATA_BUFFER_SIZE_FOR_BURNING;
        }
        else
        {
            verifiedBytes = ota_control_env.totalImageSize - verifiedDataSize;
        }

        memcpy(ota_control_env.dataBufferForBurning,
            (uint8_t *)(OTA_FLASH_LOGIC_ADDR+ota_control_env.dstFlashOffsetForNewImage + verifiedDataSize),
            OTA_DATA_BUFFER_SIZE_FOR_BURNING);

        if (0 == verifiedDataSize)
        {
            if (*(uint32_t *)ota_control_env.dataBufferForBurning != NORMAL_BOOT)
            {
                 LOG_DBG("%s:%d,Wrong Boot = 0x%x.",
                        __func__,__LINE__,ota_control_env.dataBufferForBurning);
                return false;
            }
            else
            {
                *(uint32_t *)ota_control_env.dataBufferForBurning = 0xFFFFFFFF;
            }
        }

        verifiedDataSize += verifiedBytes;

        crc32Value = crc32(crc32Value, (uint8_t *)ota_control_env.dataBufferForBurning,
            verifiedBytes);

   }

    LOG_DBG("Original CRC32 is 0x%x Confirmed CRC32 is 0x%x.", ota_control_env.crc32OfImage, crc32Value);
    if (crc32Value == ota_control_env.crc32OfImage)
    {
        return true;
    }
    else
    {
        return false;
    }
}

void ota_check_and_reboot_to_use_new_image(void)
{
    if (ota.permissionToApply && ota_control_env.isPendingForReboot == true)
    {
        app_start_postponed_reset();
    }
}

bool ota_is_in_progress(void)
{
    return ota_control_env.isOTAInProgress;
}

void ota_status_change(bool status)
{
    ota_control_env.isOTAInProgress = status;
}

static void ota_control_sending_flash_content(OTA_READ_FLASH_CONTENT_REQ_T* pReq)
{
    OTA_READ_FLASH_CONTENT_RSP_T rsp = {OTA_READ_FLASH_CONTENT, true};

    if (pReq->isToStart)
    {
        LOG_DBG("Getreading flash content request start addr 0x%x size %d",
                    pReq->startAddr, pReq->lengthToRead);

        // check the sanity of the request
        if ((pReq->startAddr >= hal_norflash_get_flash_total_size(HAL_FLASH_ID_0)) ||
            ((pReq->startAddr + pReq->lengthToRead) >
            hal_norflash_get_flash_total_size(HAL_FLASH_ID_0)))
        {
            LOG_DBG("%s %d total:%d",__func__,__LINE__,hal_norflash_get_flash_total_size(HAL_FLASH_ID_0));
            rsp.isReadingReqHandledSuccessfully = false;
        }
        else
        {
            ota_control_env.offsetInFlashToRead = pReq->startAddr;
            ota_control_env.leftSizeOfFlashContentToRead = pReq->lengthToRead;
            LOG_DBG("Start sending flash content start addr 0x%x size %d",
                pReq->startAddr, pReq->lengthToRead);
        }
    }
    else
    {
        LOG_DBG("Get stop reading flash content request.");
        ota_control_env.leftSizeOfFlashContentToRead = 0;
    }
    LOG_DBG("%s %d",__func__,__LINE__);
    if(ota_control_env.transmitHander != NULL){
        ota_control_env.transmitHander((uint8_t *)&rsp, sizeof(rsp));
    }
}

static int32_t find_key_word(uint8_t* targetArray, uint32_t targetArrayLen,
    uint8_t* keyWordArray,
    uint32_t keyWordArrayLen)
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

static bool ota_check_image_data_sanity_crc(void) {
  // find the location of the CRC key word string
  uint8_t* ptrOfTheLast4KImage = (uint8_t *)(OTA_FLASH_LOGIC_ADDR+NEW_IMAGE_FLASH_OFFSET+
    ota_control_env.totalImageSize-LEN_OF_IMAGE_TAIL_TO_FIND_SANITY_CRC);
  uint32_t sanityCrc32 = 0;

  int32_t sanity_crc_location = find_key_word(ptrOfTheLast4KImage,
    LEN_OF_IMAGE_TAIL_TO_FIND_SANITY_CRC,
    (uint8_t *)image_info_sanity_crc_key_word,
    strlen(image_info_sanity_crc_key_word));
  if (-1 == sanity_crc_location)
  {
    // if no sanity crc, the image has the old format, just ignore such a check
    return true;
  }

  LOG_DBG("sanity_crc_location is %d", sanity_crc_location);

  uint32_t crc32ImageOffset = sanity_crc_location+ota_control_env.totalImageSize-
    LEN_OF_IMAGE_TAIL_TO_FIND_SANITY_CRC+strlen(image_info_sanity_crc_key_word);
  LOG_DBG("Bytes to generate crc32 is %d", crc32ImageOffset);

  uint8_t* crcString = (uint8_t *)(OTA_FLASH_LOGIC_ADDR+NEW_IMAGE_FLASH_OFFSET+crc32ImageOffset);

  for (uint8_t index = 0;index < 8;index++)
  {
      sanityCrc32 |= (asciiToHex(crcString[index]) << (28-4*index));
  }


//  uint32_t sanityCrc32 = *(uint32_t *)(OTA_FLASH_LOGIC_ADDR+NEW_IMAGE_FLASH_OFFSET+crc32ImageOffset);

  LOG_DBG("sanityCrc32 is 0x%x", sanityCrc32);

  // generate the CRC from image data
  uint32_t calculatedCrc32 = 0;
  calculatedCrc32 = crc32(calculatedCrc32, (uint8_t *)(OTA_FLASH_LOGIC_ADDR+NEW_IMAGE_FLASH_OFFSET),
    crc32ImageOffset);

  LOG_DBG("calculatedCrc32 is 0x%x", calculatedCrc32);

  return (sanityCrc32 == calculatedCrc32);
}


static unsigned int seed = 1;

static void set_rand_seed(unsigned int init)
{
    seed = init;
}

static int get_rand(void)
{
    //Based on Knuth "The Art of Computer Programming"
    seed = seed * 1103515245 + 12345;
    return ( (unsigned int) (seed / 65536) % (32767+1) );
}

void ota_randomCode_log(uint8_t randomCode[])
{
    BesOtaErase((uint32_t)&otaUpgradeLog);
    BesOtaProgram((uint32_t)&otaUpgradeLog, randomCode, sizeof(otaUpgradeLog.randomCode));
}

void ota_update_start_message(void)
{
    BesOtaProgram((uint32_t)&otaUpgradeLog.totalImageSize,
        (uint8_t *)&ota_control_env.totalImageSize, sizeof(ota_control_env.totalImageSize));
    BesOtaProgram((uint32_t)&otaUpgradeLog.crc32OfImage,
        (uint8_t *)&ota_control_env.crc32OfImage, sizeof(ota_control_env.crc32OfImage));
}

void ota_get_start_message(void)
{
    ota_control_env.totalImageSize = otaUpgradeLog.totalImageSize;
    ota_control_env.crc32OfImage = otaUpgradeLog.crc32OfImage;
    LOG_DBG("[%s] totalImageSize = 0x%x, crc32OfImage = 0x%x", __func__, ota_control_env.totalImageSize, ota_control_env.crc32OfImage);
}

void ota_upgradeSize_log(void)
{
#if PUYA_FLASH_ERASE_LIMIT
    if(upsize_segment_cnt < UPSIZE_WRITE_FREQ)
    {
        upsize_segment_cnt++;
        LOG_DBG("PUYA upgradesize return");
        return;
    }
    else if((upsize_segment_cnt == UPSIZE_WRITE_FREQ) && (upsize_erase_cnt < UPSIZE_ELEMENT_LIMIT))
    {
        upsize_segment_cnt = 0;
        upsize_erase_cnt++;
        BesOtaProgram((uint32_t)&otaUpgradeLog.upgradeSize[++ota_control_env.i_log],
                       (uint8_t*)&ota_control_env.alreadyReceivedDataSizeOfImage, 4);
        LOG_DBG("PUYA WRITE {i_log: %d, RecSize: 0x%x, FlashWrSize: 0x%x}", ota_control_env.i_log, \
                ota_control_env.alreadyReceivedDataSizeOfImage, otaUpgradeLog.upgradeSize[ota_control_env.i_log]);
    }
    else if((upsize_segment_cnt == UPSIZE_WRITE_FREQ) && (upsize_erase_cnt == UPSIZE_ELEMENT_LIMIT))
    {
        ota_control_env.i_log = 0;
        upsize_erase_cnt = 0;
        upsize_segment_cnt = 0;
        memcpy(ota_control_env.dataBufferForBurning,
            (uint8_t *)(OTA_FLASH_LOGIC_ADDR|(uint32_t)&otaUpgradeLog),
            OTAUPLOG_HEADSIZE);

        BesOtaErase((uint32_t)&otaUpgradeLog);
        BesOtaProgram((uint32_t)&otaUpgradeLog, ota_control_env.dataBufferForBurning,
            OTAUPLOG_HEADSIZE);
        BesOtaProgram((uint32_t)&otaUpgradeLog.upgradeSize[ota_control_env.i_log],
                       (uint8_t*)&ota_control_env.alreadyReceivedDataSizeOfImage, 4);
        LOG_DBG("PUYA ERASE WRITE {i_log: %d, RecSize: 0x%x, FlashWrSize: 0x%x}", ota_control_env.i_log, \
                ota_control_env.alreadyReceivedDataSizeOfImage, otaUpgradeLog.upgradeSize[ota_control_env.i_log]);

    }
#else
    if(++ota_control_env.i_log >= sizeof(otaUpgradeLog.upgradeSize)/(sizeof(uint32_t)/sizeof(uint8_t)))
    {
        ota_control_env.i_log = 0;

        memcpy(ota_control_env.dataBufferForBurning,
            (uint8_t *)(OTA_FLASH_LOGIC_ADDR|(uint32_t)&otaUpgradeLog),
            OTAUPLOG_HEADSIZE);

        BesOtaErase((uint32_t)&otaUpgradeLog);
        BesOtaProgram((uint32_t)&otaUpgradeLog, ota_control_env.dataBufferForBurning,
            OTAUPLOG_HEADSIZE);

    }

    BesOtaProgram((uint32_t)&otaUpgradeLog.upgradeSize[ota_control_env.i_log],
                       (uint8_t*)&ota_control_env.alreadyReceivedDataSizeOfImage, 4);

    LOG_DBG("{i_log: %d, RecSize: 0x%x, FlashWrSize: 0x%x}", ota_control_env.i_log, ota_control_env.alreadyReceivedDataSizeOfImage, otaUpgradeLog.upgradeSize[ota_control_env.i_log]);
#endif
}

void ota_upgradeLog_destroy(void)
{
    ota_control_env.resume_at_breakpoint = false;
    BesOtaErase((uint32_t)&otaUpgradeLog);
    LOG_DBG("Destroyed upgrade log in flash.");

    BesFlushPendingFlashOp(NORFLASH_API_ERASING);
}

uint32_t get_upgradeSize_log(void)
{
    int32_t *p = (int32_t*)otaUpgradeLog.upgradeSize,
            left = 0, right = sizeof(otaUpgradeLog.upgradeSize)/4 - 1, mid;

    if(p[0] != -1)
    {
        while(left < right)
        {
            mid = (left + right) / 2;
            if(p[mid] == -1)
                right = mid - 1;
            else
                left = mid + 1;
        }
    }
    if(p[left]==-1)
        left--;

    ota_control_env.i_log = left;
    ota_control_env.breakPoint = left!=-1 ? p[left] : 0;
    ota_control_env.resume_at_breakpoint = ota_control_env.breakPoint?true:false;

    LOG_DBG("ota_control_env.i_log: %d", ota_control_env.i_log);
    return ota_control_env.breakPoint;
}

void ota_control_send_resume_response(uint32_t breakPoint, uint8_t randomCode[])
{
    OTA_RSP_RESUME_VERIFY_T tRsp =
        {OTA_RSP_RESUME_VERIFY, breakPoint,};
    for(uint32_t i = 0; i < sizeof(otaUpgradeLog.randomCode); i++)
        tRsp.randomCode[i] = randomCode[i];
    tRsp.crc32 = crc32(0, (uint8_t*)&tRsp.breakPoint, sizeof(tRsp.breakPoint) + sizeof(tRsp.randomCode));

    if(ota_control_env.transmitHander != NULL){
        ota_control_env.transmitHander((uint8_t *)&tRsp, sizeof(tRsp));
    }
}

void ota_update_flash_offset_after_segment_crc(bool flag)
{
    if(flag)
    {
        ota_upgradeSize_log();
        ota_control_env.offsetInFlashOfCurrentSegment = ota_control_env.offsetInFlashToProgram;
        ota_control_env.offsetOfImageOfCurrentSegment = ota_control_env.alreadyReceivedDataSizeOfImage;
    }
    else
    {
        uint32_t startFlashAddr = OTA_FLASH_LOGIC_ADDR +
            ota_control_env.dstFlashOffsetForNewImage + ota_control_env.offsetOfImageOfCurrentSegment;
        uint32_t lengthToDoCrcCheck = ota_control_env.alreadyReceivedDataSizeOfImage -
                                        ota_control_env.offsetOfImageOfCurrentSegment;
        erase_segment(startFlashAddr, lengthToDoCrcCheck);
        // restore the offset
        ota_control_env.offsetInFlashToProgram = ota_control_env.offsetInFlashOfCurrentSegment;
        ota_control_env.alreadyReceivedDataSizeOfImage = ota_control_env.offsetOfImageOfCurrentSegment;
    }
}


#if defined(IBRT)
static void ota_tws_get_master()
{
    ibrt_ctrl_t *p_ibrt_ctrl = app_tws_ibrt_get_bt_ctrl_ctx();
    ibrt_tws_mode = p_ibrt_ctrl->current_role;
}
#else
static void ota_tws_get_master()
{
}
#endif

#if defined(IBRT)
void ibrt_tws_ota_breakPoint_resume(uint32_t breakPoint, uint8_t randomCode[])
{
    OTA_RSP_RESUME_VERIFY_T tRsp =
        {OTA_RSP_RESUME_VERIFY, breakPoint,};
    for(uint32_t i = 0; i < sizeof(otaUpgradeLog.randomCode); i++)
        tRsp.randomCode[i] = randomCode[i];

    tRsp.crc32 = crc32(0, (uint8_t*)&tRsp.breakPoint, sizeof(tRsp.breakPoint) + sizeof(tRsp.randomCode));
    tws_ctrl_send_cmd(IBRT_OTA_TWS_BP_CHECK_CMD, (uint8_t*)&tRsp, sizeof(OTA_RSP_RESUME_VERIFY_T));

}

void tws_breakPoint_confirm(uint32_t breakPoint, uint8_t randomCode[])
{
    ibrt_ctrl_t *p_ibrt_ctrl = app_tws_ibrt_get_bt_ctrl_ctx();
    if(p_ibrt_ctrl->current_role == IBRT_SLAVE)
    {
        ibrt_ota_cmd_type = OTA_RSP_RESUME_VERIFY;
        twsBreakPoint = breakPoint;
        if(receivedResultAlreadyProcessedBySlave.typeCode == OTA_RSP_RESUME_VERIFY)
        {
            app_ibrt_ota_bp_check_cmd_send_handler(receivedResultAlreadyProcessedBySlave.rsp_seq,
                                                receivedResultAlreadyProcessedBySlave.p_buff,
                                                receivedResultAlreadyProcessedBySlave.length);
        }
        else
        {
            LOG_DBG("slave wait for ota info from master");
        }
    }
    else if(p_ibrt_ctrl->current_role == IBRT_MASTER)
    {
        ibrt_tws_ota_breakPoint_resume(breakPoint, randomCode);
        //tws_ctrl_send_cmd(IBRT_OTA_TWS_BP_CHECK_CMD, ibrt_slave_randomCode, sizeof(OTA_RSP_RESUME_VERIFY_T));
    }
}

void ota_tws_send_handle(uint8_t typeCode, uint8_t *buff, uint16_t length, uint32_t cmd_dode)
{
    ibrt_ctrl_t *p_ibrt_ctrl = app_tws_ibrt_get_bt_ctrl_ctx();

    if (p_ibrt_ctrl->current_role == IBRT_MASTER)
    {
        LOG_DBG("master send ota info to slave");
        tws_ctrl_send_cmd(cmd_dode, (uint8_t *)buff, length);
    }
    else if (p_ibrt_ctrl->current_role == IBRT_SLAVE)
    {
        LOG_DBG("slave wait for ota info from master");
        ibrt_ota_cmd_type = typeCode;

        if ((receivedResultAlreadyProcessedBySlave.typeCode == 0xFF)
            || (receivedResultAlreadyProcessedBySlave.typeCode != typeCode))
        {
            LOG_DBG("slave wait for ota info from master!");
        }
        else
        {
            switch (typeCode)
            {
                case OTA_RSP_SEGMENT_VERIFY:
                {
                    app_ibrt_ota_segment_crc_cmd_send_handler(
                            receivedResultAlreadyProcessedBySlave.rsp_seq, \
                            receivedResultAlreadyProcessedBySlave.p_buff, \
                            receivedResultAlreadyProcessedBySlave.length);
                }
                break;
                case OTA_RSP_START:
                {
                    app_ibrt_ota_start_cmd_send_handler(
                            receivedResultAlreadyProcessedBySlave.rsp_seq, \
                            receivedResultAlreadyProcessedBySlave.p_buff, \
                            receivedResultAlreadyProcessedBySlave.length);
                }
                break;
                case OTA_RSP_CONFIG:
                {
                    app_ibrt_ota_config_cmd_send_handler(
                            receivedResultAlreadyProcessedBySlave.rsp_seq, \
                            receivedResultAlreadyProcessedBySlave.p_buff, \
                            receivedResultAlreadyProcessedBySlave.length);
                }
                break;
                case OTA_RSP_RESULT:
                {
                    app_ibrt_ota_image_crc_cmd_send_handler(
                            receivedResultAlreadyProcessedBySlave.rsp_seq, \
                            receivedResultAlreadyProcessedBySlave.p_buff, \
                            receivedResultAlreadyProcessedBySlave.length);
                }
                break;
                case OTA_RSP_VERSION:
                {
                    app_ibrt_ota_get_version_cmd_send_handler(
                            receivedResultAlreadyProcessedBySlave.rsp_seq, \
                            receivedResultAlreadyProcessedBySlave.p_buff, \
                            receivedResultAlreadyProcessedBySlave.length);
                }
                break;
                case OTA_RSP_SIDE_SELECTION:
                {
                    app_ibrt_ota_select_side_cmd_send_handler(
                            receivedResultAlreadyProcessedBySlave.rsp_seq, \
                            receivedResultAlreadyProcessedBySlave.p_buff, \
                            receivedResultAlreadyProcessedBySlave.length);
                }
                break;
                case OTA_RSP_IMAGE_APPLY:
                {
                    app_ibrt_ota_image_overwrite_cmd_send_handler(
                            receivedResultAlreadyProcessedBySlave.rsp_seq, \
                            receivedResultAlreadyProcessedBySlave.p_buff, \
                            receivedResultAlreadyProcessedBySlave.length);
                }
                break;
                default:
                break;
            }
            receivedResultAlreadyProcessedBySlave.typeCode = 0XFF;
        }
    }
}
#endif

void ota_bes_handle_received_data(uint8_t *otaBuf, bool isViaBle,uint16_t dataLenth)
{
    #define MAX_SEG_VERIFY_RETEY    3
    static uint32_t seg_verify_retry = MAX_SEG_VERIFY_RETEY;
    uint8_t typeCode = otaBuf[0];
    ibrt_ctrl_t *p_ibrt_ctrl = app_tws_ibrt_get_bt_ctrl_ctx();

    LOG_DBG("[%s],case = 0x%x%s",__func__, typeCode, typeCode2Str(typeCode));
    switch (typeCode)
    {
        case OTA_DATA_PACKET:
        {
            if ((!ota_is_in_progress()) ||
                ((IBRT_MASTER == p_ibrt_ctrl->current_role) && !app_tws_ibrt_tws_link_connected()))
            {
                ota_control_send_result_response(false);
                return;
            }

            uint8_t* rawDataPtr = &otaBuf[1];

            uint32_t rawDataSize = dataLenth - 1;
            LOG_DBG("Received image data size %d", rawDataSize);
            uint32_t leftDataSize = rawDataSize;
            uint32_t offsetInReceivedRawData = 0;
            do
            {
                uint32_t bytesToCopy;
                // copy to data buffer
                if ((ota_control_env.offsetInDataBufferForBurning + leftDataSize) >
                    OTA_DATA_BUFFER_SIZE_FOR_BURNING)
                {
                    bytesToCopy = OTA_DATA_BUFFER_SIZE_FOR_BURNING - ota_control_env.offsetInDataBufferForBurning;
                }
                else
                {
                    bytesToCopy = leftDataSize;
                }

                leftDataSize -= bytesToCopy;

                memcpy(&ota_control_env.dataBufferForBurning[ota_control_env.offsetInDataBufferForBurning],
                        &rawDataPtr[offsetInReceivedRawData], bytesToCopy);
                offsetInReceivedRawData += bytesToCopy;
                ota_control_env.offsetInDataBufferForBurning += bytesToCopy;
                LOG_DBG("offsetInDataBufferForBurning is %d", ota_control_env.offsetInDataBufferForBurning);
                if (OTA_DATA_BUFFER_SIZE_FOR_BURNING <= ota_control_env.offsetInDataBufferForBurning)
                {
                    LOG_DBG("Program the image to flash.");
                    LOG_DBG(".offsetInFlashToProgram:0x%x  .dstFlashOffsetForNewImage:0x%x  .totalImageSize:%d", ota_control_env.offsetInFlashToProgram, ota_control_env.dstFlashOffsetForNewImage, ota_control_env.totalImageSize);
                    #if (IMAGE_RECV_FLASH_CHECK == 1)
                    if((ota_control_env.offsetInFlashToProgram - ota_control_env.dstFlashOffsetForNewImage > ota_control_env.totalImageSize) ||
                        (ota_control_env.totalImageSize > MAX_IMAGE_SIZE) ||
                        (ota_control_env.offsetInFlashToProgram & (MIN_SEG_ALIGN - 1)))
                    {
                        LOG_DBG("ERROR: IMAGE_RECV_FLASH_CHECK");
                        LOG_DBG(" ota_control_env(.offsetInFlashToProgram - .dstFlashOffsetForNewImage >= .totalImageSize)");
                        LOG_DBG(" or (ota_control_env.totalImageSize > %d)", MAX_IMAGE_SIZE);
                        LOG_DBG(" or .offsetInFlashToProgram isn't segment aligned");
                        LOG_DBG(".offsetInFlashToProgram:0x%x  .dstFlashOffsetForNewImage:0x%x  .totalImageSize:%d", ota_control_env.offsetInFlashToProgram, ota_control_env.dstFlashOffsetForNewImage, ota_control_env.totalImageSize);
                        // ota_upgradeLog_destroy();  // In order to reduce unnecessary erasures and retransmissions we don't imeediately destory the log but reset ota, because a boundary check is performed before flashing and if there is really wrong we'll catch when an image CRC32 check finally.
                        ota_control_send_result_response(OTA_RESULT_ERR_FLASH_OFFSET);
                        return;
                    }
                    #endif

                    ota_control_flush_data_to_flash(ota_control_env.dataBufferForBurning, OTA_DATA_BUFFER_SIZE_FOR_BURNING,
                        ota_control_env.offsetInFlashToProgram);
                    ota_control_env.offsetInFlashToProgram += OTA_DATA_BUFFER_SIZE_FOR_BURNING;
                    ota_control_env.offsetInDataBufferForBurning = 0;
                }
            } while (offsetInReceivedRawData < rawDataSize);

            ota_control_env.alreadyReceivedDataSizeOfImage += rawDataSize;
            LOG_DBG("Image already received %d", ota_control_env.alreadyReceivedDataSizeOfImage);

            #if (IMAGE_RECV_FLASH_CHECK == 1)
            if((ota_control_env.alreadyReceivedDataSizeOfImage > ota_control_env.totalImageSize) ||
                (ota_control_env.totalImageSize > MAX_IMAGE_SIZE))
            {
                LOG_DBG("ERROR: IMAGE_RECV_FLASH_CHECK");
                LOG_DBG(" ota_control_env(.alreadyReceivedDataSizeOfImage > .totalImageSize)");
                LOG_DBG(" or (ota_control_env.totalImageSize > %d)", MAX_IMAGE_SIZE);
                LOG_DBG(".alreadyReceivedDataSizeOfImage:%d  .totalImageSize:%d", ota_control_env.alreadyReceivedDataSizeOfImage, ota_control_env.totalImageSize);
                //ota_upgradeLog_destroy();  // In order to reduce unnecessary erasures and retransmissions we don't imeediately destory the log but reset ota, because a boundary check is performed before flashing and if there is really wrong we'll catch when an image CRC32 check finally.
                ota_control_send_result_response(OTA_RESULT_ERR_RECV_SIZE);
                return;
            }
            #endif

#if DATA_ACK_FOR_SPP_DATAPATH_ENABLED
            if (DATA_PATH_SPP == ota_control_env.dataPathType)
            {
#if defined(OTA_DATA_PKT_SYNC_MS) && defined(IBRT)
                app_send_ota_pkt_sync_ack_response(false);
#else
                ota_control_send_data_ack_response();
#endif
            }
#endif
            break;
        }
        case OTA_COMMAND_SEGMENT_VERIFY:
        {
            OTA_CONTROL_SEGMENT_VERIFY_T* ptVerifyCmd = (OTA_CONTROL_SEGMENT_VERIFY_T *)(otaBuf);
            if(!app_get_bes_ota_state())
            {
                break;
            }
            if((ota_control_env.offsetInDataBufferForBurning == 0) &&
                (ota_control_env.alreadyReceivedDataSizeOfImage == ota_control_env.offsetOfImageOfCurrentSegment))
            {
                //there is no new data, it is app resend 82 cmd
                LOG_DBG("resend 82 cmd, CRC32 of the segement:0x%x, received num:0x%x",
                    ota_control_env.crc32OfSegment,
                    ptVerifyCmd->crc32OfSegment);
                if((BES_OTA_START_MAGIC_CODE == ptVerifyCmd->magicCode) &&
                    (ptVerifyCmd->crc32OfSegment == ota_control_env.crc32OfSegment))
                {
#if defined(IBRT)
                    uint8_t ota_segment_crc = 1;
                    errOtaCode = 1;
                    ota_tws_send_handle(OTA_RSP_SEGMENT_VERIFY, (uint8_t *)&ota_segment_crc, 1, IBRT_OTA_TWS_SEGMENT_CRC_CMD);
#else
                    ota_control_send_segment_verification_response(true);
#endif
                }
                else
                {
#if defined(IBRT)
                    uint8_t ota_segment_crc = 0;
                    ota_tws_send_handle(OTA_RSP_SEGMENT_VERIFY, (uint8_t *)&ota_segment_crc, 1, IBRT_OTA_TWS_SEGMENT_CRC_CMD);
#else
                    ota_control_send_segment_verification_response(false);
#endif
                }
                break;
            }
            #if (IMAGE_RECV_FLASH_CHECK == 1)
            if((ota_control_env.offsetInFlashToProgram - ota_control_env.dstFlashOffsetForNewImage > ota_control_env.totalImageSize) ||
                (ota_control_env.totalImageSize > MAX_IMAGE_SIZE) ||
                (ota_control_env.offsetInFlashToProgram & (MIN_SEG_ALIGN - 1)))
            {
                LOG_DBG("ERROR: IMAGE_RECV_FLASH_CHECK");
                LOG_DBG(" ota_control_env(.offsetInFlashToProgram - .dstFlashOffsetForNewImage >= .totalImageSize)");
                LOG_DBG(" or (ota_control_env.totalImageSize > %d)", MAX_IMAGE_SIZE);
                LOG_DBG(" or .offsetInFlashToProgram isn't segment aligned");
                LOG_DBG(".offsetInFlashToProgram:0x%x  .dstFlashOffsetForNewImage:0x%x  .totalImageSize:%d", ota_control_env.offsetInFlashToProgram, ota_control_env.dstFlashOffsetForNewImage, ota_control_env.totalImageSize);
                //ota_upgradeLog_destroy();  // In order to reduce unnecessary erasures and retransmissions we don't imeediately destory the log but reset ota, because a boundary check is performed before flashing and if there is really wrong we'll catch when an image CRC32 check finally.
#if defined(IBRT)
                errOtaCode = OTA_RESULT_ERR_FLASH_OFFSET;
                ota_tws_send_handle(OTA_RSP_SEGMENT_VERIFY, (uint8_t *)&errOtaCode, 1, IBRT_OTA_TWS_SEGMENT_CRC_CMD);
#else
                ota_control_send_result_response(OTA_RESULT_ERR_FLASH_OFFSET);
#endif
                return;
            }
            #endif
            LOG_DBG(".offsetInFlashToProgram:0x%x  .dstFlashOffsetForNewImage:0x%x  .burning:%d", ota_control_env.offsetInFlashToProgram, ota_control_env.dstFlashOffsetForNewImage, ota_control_env.offsetInDataBufferForBurning);

            ota_control_flush_data_to_flash(ota_control_env.dataBufferForBurning, ota_control_env.offsetInDataBufferForBurning,
                    ota_control_env.offsetInFlashToProgram);
            ota_control_env.offsetInFlashToProgram += ota_control_env.offsetInDataBufferForBurning;
            ota_control_env.offsetInDataBufferForBurning = 0;

            LOG_DBG("Calculate the crc32 of the segment,start addr = 0x%x.",OTA_FLASH_LOGIC_ADDR +
            ota_control_env.dstFlashOffsetForNewImage + ota_control_env.offsetOfImageOfCurrentSegment);

            uint32_t startFlashAddr = OTA_FLASH_LOGIC_ADDR +
            ota_control_env.dstFlashOffsetForNewImage + ota_control_env.offsetOfImageOfCurrentSegment;
            uint32_t lengthToDoCrcCheck = ota_control_env.alreadyReceivedDataSizeOfImage-ota_control_env.offsetOfImageOfCurrentSegment;

#if OTA_NON_CACHE_READ_ISSUE_WORKAROUND
            hal_cache_invalidate(HAL_CACHE_ID_D_CACHE,
                (uint32_t)startFlashAddr,
                lengthToDoCrcCheck);
#endif

            ota_control_env.crc32OfSegment = crc32(0, (uint8_t *)(startFlashAddr), lengthToDoCrcCheck);
            LOG_DBG("CRC32 of the segement is 0x%x,receiced num = 0x%x", ota_control_env.crc32OfSegment, ptVerifyCmd->crc32OfSegment);

            if ((BES_OTA_START_MAGIC_CODE == ptVerifyCmd->magicCode) &&
                (ptVerifyCmd->crc32OfSegment == ota_control_env.crc32OfSegment))
            {
#if defined(IBRT)
                uint8_t ota_segment_crc = 1;
                errOtaCode = 1;
                ota_tws_send_handle(OTA_RSP_SEGMENT_VERIFY, (uint8_t *)&ota_segment_crc, 1, IBRT_OTA_TWS_SEGMENT_CRC_CMD);
#else
                ota_control_send_segment_verification_response(true);
                ota_upgradeSize_log();
                seg_verify_retry = MAX_SEG_VERIFY_RETEY;
                // backup of the information in case the verification of current segment failed
                ota_control_env.offsetInFlashOfCurrentSegment = ota_control_env.offsetInFlashToProgram;
                ota_control_env.offsetOfImageOfCurrentSegment = ota_control_env.alreadyReceivedDataSizeOfImage;
#endif
                ota_status_change(true);
                seg_verify_retry = MAX_SEG_VERIFY_RETEY;
            }
            else
            {
                if(--seg_verify_retry == 0)
                {
                    seg_verify_retry = MAX_SEG_VERIFY_RETEY;

                    LOG_DBG("ERROR: segment verification retry too much!");
                    ota_upgradeLog_destroy();  // Yes, destory it and retransmit the entire image.
#if defined(IBRT)
                    errOtaCode = OTA_RESULT_ERR_SEG_VERIFY;
                    ota_tws_send_handle(OTA_RSP_SEGMENT_VERIFY, (uint8_t *)&errOtaCode, 1, IBRT_OTA_TWS_SEGMENT_CRC_CMD);
#else
                    ota_control_send_result_response(OTA_RESULT_ERR_SEG_VERIFY);
#endif
                    return;
                }
#if defined(IBRT)
                erase_segment(startFlashAddr, lengthToDoCrcCheck);
                uint8_t ota_segment_crc = 0;
                ota_tws_send_handle(OTA_RSP_SEGMENT_VERIFY, (uint8_t *)&ota_segment_crc, 1, IBRT_OTA_TWS_SEGMENT_CRC_CMD);
#else
                // restore the offset
                ota_control_env.offsetInFlashToProgram = ota_control_env.offsetInFlashOfCurrentSegment;
                ota_control_env.alreadyReceivedDataSizeOfImage = ota_control_env.offsetOfImageOfCurrentSegment;
                ota_control_send_segment_verification_response(false);
#endif
            }

            // reset the data buffer
            LOG_DBG("total size is %d already received %d", ota_control_env.totalImageSize,
                ota_control_env.alreadyReceivedDataSizeOfImage);

            break;
        }
        case OTA_COMMAND_START:
        {
            OTA_CONTROL_START_T* ptStart = (OTA_CONTROL_START_T *)(otaBuf);
            if (BES_OTA_START_MAGIC_CODE == ptStart->magicCode)
            {
                LOG_DBG("Receive command start request:");
                ota_control_reset_env();
                ota_control_env.totalImageSize = ptStart->imageSize;
                ota_control_env.crc32OfImage = ptStart->crc32OfImage;
                ota_status_change(true);

                ota_control_env.AlreadyReceivedConfigurationLength = 0;
                seg_verify_retry = MAX_SEG_VERIFY_RETEY;
                TRACE(2,"Image size is 0x%x, crc32 of image is 0x%x",
                    ota_control_env.totalImageSize, ota_control_env.crc32OfImage);
                ota_update_start_message();
#if defined(IBRT)
                ota_tws_send_handle(OTA_RSP_START, (uint8_t*)&isViaBle, 1, IBRT_OTA_TWS_START_OTA_CMD);
#else
                ota_control_send_start_response(isViaBle);
#endif
            }
            break;
        }
        case OTA_COMMAND_RESUME_VERIFY:
        {
            OTA_CONTROL_RESUME_VERIFY_T* ptStart = (OTA_CONTROL_RESUME_VERIFY_T *)(otaBuf);
            ota_tws_get_master();
            if (BES_OTA_START_MAGIC_CODE == ptStart->magicCode)
            {
                uint32_t breakPoint;
                uint8_t *randomCode = NULL;
                uint8_t zeroCode[32] = {0};

                if (ota_control_env.dataBufferForBurning == NULL) {
                    LOG_DBG("%s:randomCode buffer is NULL,reset buffer here!", __func__);
                    ota_control_env.dataBufferForBurning = app_voicepath_get_common_ota_databuf();
                }

                randomCode = ota_control_env.dataBufferForBurning;
                memset(randomCode, 0, sizeof(otaUpgradeLog.randomCode));

                LOG_DBG("Receive command resuming verification:");
                if(ptStart->crc32 != crc32(0, ptStart->randomCode, sizeof(ptStart->randomCode) + sizeof(ptStart->segmentSize)))
                {
                    breakPoint = -1;
                    LOG_DBG("Resuming verification crc32 check fail.");
                    LOG_DBG("Original CRC32: 0x%x, confirmed CRC32:0x%X");
                    goto resume_response;
                }
                LOG_DBG("Resuming verification crc32 check pass.");

                LOG_DBG("Receive segment size 0x%x.", ptStart->segmentSize);

                LOG_DBG("Receive random code:");
                LOG_DUMP("%02x ", ptStart->randomCode, sizeof(otaUpgradeLog.randomCode));

                LOG_DBG("Device's random code:");
                LOG_DUMP("%02x ", otaUpgradeLog.randomCode, sizeof(otaUpgradeLog.randomCode));

                breakPoint = get_upgradeSize_log();
                if(breakPoint & (MIN_SEG_ALIGN - 1))  // Minimum segment alignment.
                {
                    LOG_DBG("Breakpoint:0x%x isn't minimum segment alignment!", breakPoint);
                    ota_upgradeLog_destroy();  // Error in log, we'd better try to retransmit the entire image.
                    #if 1
                    breakPoint = get_upgradeSize_log();
                    #else
                    ota_control_send_result_response(OTA_RESULT_ERR_BREAKPOINT);
                    return;
                    #endif
                }
                if(breakPoint)
                {
                    if(!memcmp(otaUpgradeLog.randomCode, ptStart->randomCode, sizeof(otaUpgradeLog.randomCode)) &&
                        memcmp(ptStart->randomCode, zeroCode, sizeof(zeroCode)))
                    {
                        ota_control_reset_env();
                        ota_status_change(true);
                        ota_get_start_message();
                        LOG_DBG("OTA can resume. Resuming from the breakpoint at: 0x%x.", breakPoint);
                    }
                    else
                    {
                        LOG_DBG("OTA can't resume because the randomCode is inconsistent. [breakPoint: 0x%x]", breakPoint);

                        breakPoint = ota_control_env.breakPoint = ota_control_env.resume_at_breakpoint = 0;
                    }
                }
                if(breakPoint == 0)
                {
                    if(p_ibrt_ctrl->current_role == IBRT_MASTER)
                    {
                        LOG_DBG("OTA resume none. Generate new random code for the new transmisson now. [breakPoint: 0x%x]", breakPoint);
                        for(uint32_t i = 0; i < sizeof(otaUpgradeLog.randomCode); i++)
                        {
                            set_rand_seed(hal_sys_timer_get());
                            randomCode[i] = get_rand();
                            DelayMs(1);
                        }
                        ota_randomCode_log(randomCode);
                        LOG_DBG("New random code:");
                        LOG_DUMP("%02x ", randomCode, sizeof(otaUpgradeLog.randomCode));
                    }
                }

            resume_response:
#if defined(IBRT)
                if(breakPoint == 0)
                {
                    tws_breakPoint_confirm(breakPoint, randomCode);
                }
                else
                {
                    tws_breakPoint_confirm(breakPoint, ptStart->randomCode);
                }
#else
                ota_control_env.AlreadyReceivedConfigurationLength = 0;
                ota_control_send_resume_response(breakPoint, randomCode);
#endif
            }
            break;
        }
        case OTA_COMMAND_CONFIG_OTA:
        {
            OTA_FLOW_CONFIGURATION_T* ptConfig = (OTA_FLOW_CONFIGURATION_T *)&(ota_control_env.configuration);
            Bes_enter_ota_state();
            memcpy((uint8_t *)ptConfig + ota_control_env.AlreadyReceivedConfigurationLength,
                &(otaBuf[1]), dataLenth - 1);

            ota_control_env.AlreadyReceivedConfigurationLength += (dataLenth - 1);
            if ((ptConfig->lengthOfFollowingData + 4) <= ota_control_env.AlreadyReceivedConfigurationLength)
            {
                LOG_DBG("lengthOfFollowingData 0x%x", ptConfig->lengthOfFollowingData);
                LOG_DBG("startLocationToWriteImage 0x%x", ptConfig->startLocationToWriteImage);
                LOG_DBG("isToClearUserData %d", ptConfig->isToClearUserData);
                LOG_DBG("isToRenameBT %d", ptConfig->isToRenameBT);
                LOG_DBG("isToRenameBLE %d", ptConfig->isToRenameBLE);
                LOG_DBG("isToUpdateBTAddr %d", ptConfig->isToUpdateBTAddr);
                LOG_DBG("isToUpdateBLEAddr %d", ptConfig->isToUpdateBLEAddr);
                LOG_DBG("New BT name:");
                LOG_DUMP("0x%02x ", ptConfig->newBTName, BES_OTA_NAME_LENGTH);
                LOG_DBG("New BLE name:");
                LOG_DUMP("0x%02x ", ptConfig->newBLEName, BES_OTA_NAME_LENGTH);
                LOG_DBG("New BT addr:");
                LOG_DUMP("0x%02x ", ptConfig->newBTAddr, BD_ADDR_LEN);
                LOG_DBG("New BLE addr:");
                LOG_DUMP("0x%02x ", ptConfig->newBLEAddr, BD_ADDR_LEN);
                LOG_DBG("crcOfConfiguration 0x%x", ptConfig->crcOfConfiguration);

                if(ota_control_env.configuration.startLocationToWriteImage != ota_control_env.offsetInFlashToProgram)
                {
                    LOG_DBG("Wrong Start Location. Check OTA Flash Offset of Update Image!");
                    //ota_control_send_result_response(OTA_RESULT_ERR_IMAGE_SIZE);
					//ota_control_reset_env();
                    //return;
                }

                // check CRC
                if (ptConfig->crcOfConfiguration == crc32(0, (uint8_t *)ptConfig, sizeof(OTA_FLOW_CONFIGURATION_T) - sizeof(uint32_t)))
                {
                    if(ota_control_env.totalImageSize > MAX_IMAGE_SIZE)
                    {
                        LOG_DBG("ImageSize 0x%x greater than 0x%x! Terminate the upgrade.", ota_control_env.totalImageSize, MAX_IMAGE_SIZE);
                        //ota_control_send_result_response(OTA_RESULT_ERR_IMAGE_SIZE);
                        //uint8_t err_image_size = OTA_RESULT_ERR_IMAGE_SIZE;
#if defined(IBRT)
                        errOtaCode = OTA_RESULT_ERR_IMAGE_SIZE;
                        ota_tws_send_handle(OTA_RSP_CONFIG, (uint8_t *)&errOtaCode, 1, IBRT_OTA_TWS_OTA_CONFIG_CMD);
#else
                        ota_control_send_result_response(OTA_RESULT_ERR_IMAGE_SIZE);
#endif
                        return;
                    }

                    if(ota_control_env.breakPoint > ota_control_env.totalImageSize)
                    {
                        ota_upgradeLog_destroy();  // Error in log, we'd better try to retransmit the entire image.
                        //ota_control_send_result_response(OTA_RESULT_ERR_BREAKPOINT);
#if defined(IBRT)
                        errOtaCode = OTA_RESULT_ERR_BREAKPOINT;
                        ota_tws_send_handle(OTA_RSP_CONFIG, (uint8_t *)&errOtaCode, 1, IBRT_OTA_TWS_OTA_CONFIG_CMD);
#else
                        ota_control_send_result_response(OTA_RESULT_ERR_BREAKPOINT);
#endif
                        return;
                    }
                    //ota_control_env.offsetInFlashToProgram = new_image_copy_flash_offset;
                    ota_control_env.offsetInFlashOfCurrentSegment = ota_control_env.offsetInFlashToProgram;
                    ota_control_env.dstFlashOffsetForNewImage = ota_control_env.offsetInFlashOfCurrentSegment;

                    if(ota_control_env.resume_at_breakpoint == true)
                    {
                        ota_control_env.alreadyReceivedDataSizeOfImage = ota_control_env.breakPoint;
                        ota_control_env.offsetOfImageOfCurrentSegment = ota_control_env.alreadyReceivedDataSizeOfImage;
                        ota_control_env.offsetInFlashOfCurrentSegment = ota_control_env.dstFlashOffsetForNewImage + ota_control_env.offsetOfImageOfCurrentSegment;
                        ota_control_env.offsetInFlashToProgram = ota_control_env.offsetInFlashOfCurrentSegment;
                    }
                    LOG_DBG("OTA config pass.");

                    LOG_DBG("Start writing the received image to flash offset 0x%x", ota_control_env.offsetInFlashToProgram);

                    FLASH_OTA_BOOT_INFO_T otaBootInfo = {NORMAL_BOOT,0,0};
                    app_update_ota_boot_info(&otaBootInfo);
                    LOG_DBG("update NORMAL_BOOT done.");
#if defined(IBRT)
                    bool ota_config = 1;
                    errOtaCode = 1;
                    ota_tws_send_handle(OTA_RSP_CONFIG, (uint8_t *)&ota_config, 1, IBRT_OTA_TWS_OTA_CONFIG_CMD);
#else
                    ota_control_send_configuration_response(true);
#endif
                }
                else
                {
                    LOG_DBG("OTA config failed.");
#if defined(IBRT)
                    bool ota_config = 0;
                    ota_tws_send_handle(OTA_RSP_CONFIG, (uint8_t *)&ota_config, 1, IBRT_OTA_TWS_OTA_CONFIG_CMD);
#else
                    ota_control_send_configuration_response(false);
#endif
                }
            }
            break;
        }
        case OTA_COMMAND_GET_OTA_RESULT:
        {
            // check whether all image data have been received
            if (ota_control_env.alreadyReceivedDataSizeOfImage == ota_control_env.totalImageSize)
            {
                LOG_DBG("The final image programming and crc32 check.");

                // flush the remaining data to flash
                if(0 != ota_control_env.offsetInDataBufferForBurning)
                {
                    ota_control_flush_data_to_flash(ota_control_env.dataBufferForBurning,
                                                    ota_control_env.offsetInDataBufferForBurning,
                                                    ota_control_env.offsetInFlashToProgram);
                }

                bool Ret  =  ota_check_image_data_sanity_crc();
                if(Ret)
                {
                    // update the magic code of the application image
                    app_update_magic_number_of_app_image(NORMAL_BOOT);
                }
                else
                {
                    LOG_DBG("data sanity crc failed.");
#if defined(IBRT)
                    errOtaCode = Ret;
                    ota_tws_send_handle(OTA_RSP_RESULT, (uint8_t *)&Ret, 1, IBRT_OTA_TWS_IMAGE_CRC_CMD);
#else
                    ota_control_send_result_response(Ret);
#endif
                    ota_control_reset_env();
                    return;
                }

                // check the crc32 of the image
                bool ret = ota_control_check_image_crc();
                if (ret)
                {
                    LOG_DBG("Whole image verification pass.");
                    ota_control_env.isPendingForReboot = true;
#if defined(IBRT)
                    errOtaCode = ret;
                    ota_tws_send_handle(OTA_RSP_RESULT, (uint8_t *)&ret, 1, IBRT_OTA_TWS_IMAGE_CRC_CMD);
#else
                    FLASH_OTA_BOOT_INFO_T otaBootInfo = {COPY_NEW_IMAGE, ota_control_env.totalImageSize, ota_control_env.crc32OfImage};
                    app_update_ota_boot_info(&otaBootInfo);
                    ota_update_userdata_pool();
                    ota_control_send_result_response(ret);
                    ota_upgradeLog_destroy();
#endif
                }
                else
                {
                    Bes_exit_ota_state();
                    LOG_DBG("Whole image verification failed.");
#if defined(IBRT)
                    errOtaCode = ret;
                    ota_tws_send_handle(OTA_RSP_RESULT, (uint8_t *)&ret, 1, IBRT_OTA_TWS_IMAGE_CRC_CMD);
#else
                    ota_control_send_result_response(ret);
                    ota_upgradeLog_destroy();
                    ota_control_reset_env();
#endif
                }
            }
            else
            {
#if defined(IBRT)
                bool whole_image_crc = 0;
                ota_tws_send_handle(OTA_RSP_RESULT, (uint8_t *)&whole_image_crc, 1, IBRT_OTA_TWS_IMAGE_CRC_CMD);
#else
                ota_control_send_result_response(false);
#endif
            }
            break;
        }
        case OTA_READ_FLASH_CONTENT:
        {
            ota_control_sending_flash_content((OTA_READ_FLASH_CONTENT_REQ_T*)otaBuf);
            break;
        }
        case OTA_COMMAND_GET_VERSION:
        {
            OTA_GET_VERSION_REQ_T* ptStart = (OTA_GET_VERSION_REQ_T *)(otaBuf);
            hal_norflash_disable_protection(HAL_FLASH_ID_0);
            ota_tws_get_master();
            if (BES_OTA_START_MAGIC_CODE == ptStart->magicCode)
            {
#if defined(IBRT)
                uint8_t fw_rev[4];
                system_get_info(&fw_rev[0], &fw_rev[1], &fw_rev[2], &fw_rev[3]);
                ota_tws_send_handle(OTA_RSP_VERSION, fw_rev, sizeof(fw_rev)/sizeof(fw_rev[0]), IBRT_OTA_TWS_GET_VERSION_CMD);
#else
                ota_control_send_version_rsp();
#endif
            }
            break;
        }
        case OTA_COMMAND_SIDE_SELECTION:
        {
#if defined(IBRT)
            uint8_t selectSide[2] = {0x91 , 0x01};
            ota_tws_send_handle(OTA_RSP_SIDE_SELECTION, selectSide, sizeof(selectSide)/sizeof(selectSide[0]), IBRT_OTA_TWS_SELECT_SIDE_CMD);
#else
            ota_control_side_selection_rsp(true);
#endif
            break;
        }
        case OTA_COMMAND_IMAGE_APPLY:
        {
            Bes_exit_ota_state();
            OTA_IMAGE_APPLY_REQ_T* ptStart = (OTA_IMAGE_APPLY_REQ_T *)(otaBuf);
            if (BES_OTA_START_MAGIC_CODE == ptStart->magicCode && ota_control_env.isPendingForReboot)
            {
                TRACE(0,"ota_control_image_apply_rsp...");
                ota.permissionToApply = 1;
#if defined(IBRT)
                bool overWrite = 1;
                errOtaCode = 1;
                ota_tws_send_handle(OTA_RSP_IMAGE_APPLY, (uint8_t *)&overWrite, 1, IBRT_OTA_TWS_IMAGE_OVERWRITE_CMD);
#else
                ota_control_image_apply_rsp(true);
#endif
            }
            else
            {
                ota.permissionToApply = 0;
#if defined(IBRT)
                bool overWrite = 0;
                ota_tws_send_handle(OTA_RSP_IMAGE_APPLY, (uint8_t *)&overWrite, 1, IBRT_OTA_TWS_IMAGE_OVERWRITE_CMD);
#else
                ota_control_image_apply_rsp(false);
#endif
            }
            break;
        }
        default:
            break;
    }
}

void ota_ibrt_handle_received_data(uint8_t *otaBuf, bool isViaBle, uint16_t len)
{
    TRACE(2,"[%s],case = 0x%x", __func__, otaBuf[0]);
    static uint8_t stop_receive = 0;
    if(otaBuf[0] == OTA_COMMAND_IMAGE_APPLY)
    {
        stop_receive = 1;
    }
    else if(stop_receive == 1)
    {
        stop_receive = 0;
        return;
    }

    tws_ctrl_send_cmd(IBRT_OTA_TWS_IMAGE_BUFF, otaBuf, len);
    ota_bes_handle_received_data(otaBuf, isViaBle, len);
}

static void app_update_magic_number_of_app_image(uint32_t newMagicNumber)
{

    uint32_t startFlashAddr = OTA_FLASH_LOGIC_ADDR +
            ota_control_env.dstFlashOffsetForNewImage;

    memcpy(ota_control_env.dataBufferForBurning, (uint8_t *)startFlashAddr,
        FLASH_SECTOR_SIZE_IN_BYTES);


    *(uint32_t *)ota_control_env.dataBufferForBurning = newMagicNumber;

    BesOtaErase(ota_control_env.dstFlashOffsetForNewImage);
    BesOtaProgram(ota_control_env.dstFlashOffsetForNewImage,
        ota_control_env.dataBufferForBurning, FLASH_SECTOR_SIZE_IN_BYTES);

    BesFlushPendingFlashOp(NORFLASH_API_ALL);
}

uint32_t app_get_magic_number(void)
{
    // Workaround for reboot: controller in standard SPI mode while FLASH in QUAD mode
    // First read will fail when FLASH in QUAD mode, but it will make FLASH roll back to standard SPI mode
    // Second read will succeed

    volatile uint32_t *magic;

    magic = (volatile uint32_t *)&otaBootInfoInFlash;

    // First read (and also flush the controller prefetch buffer)
    *(magic + 0x400);

    return *magic;
}

static void app_update_ota_boot_info(FLASH_OTA_BOOT_INFO_T* otaBootInfo)
{
    BesOtaErase( OTA_INFO_IN_OTA_BOOT_SEC);
    BesOtaProgram(OTA_INFO_IN_OTA_BOOT_SEC, (uint8_t*)otaBootInfo,
        sizeof(FLASH_OTA_BOOT_INFO_T));

    BesFlushPendingFlashOp(NORFLASH_API_ALL);
}

void ota_apply(void)
{
    FLASH_OTA_BOOT_INFO_T otaBootInfo = {COPY_NEW_IMAGE, ota_control_env.totalImageSize, ota_control_env.crc32OfImage};
    app_update_ota_boot_info(&otaBootInfo);
    ota_update_userdata_pool();
    ota_check_and_reboot_to_use_new_image();
}

static void ota_control_opera_callback(void* param)
{
    NORFLASH_API_OPERA_RESULT *opera_result;

    opera_result = (NORFLASH_API_OPERA_RESULT*)param;

    LOG_DBG("%s:type = %d, addr = 0x%x,len = 0x%x,result = %d.",
                __func__,
                opera_result->type,
                opera_result->addr,
                opera_result->len,
                opera_result->result);
}

void ota_flash_init(void)
{
    LOG_DBG("[%s].",__func__);
#ifdef __APP_USER_DATA_NV_FLASH_OFFSET__
    user_data_nv_flash_offset = __APP_USER_DATA_NV_FLASH_OFFSET__;
#else
    user_data_nv_flash_offset = hal_norflash_get_flash_total_size(HAL_FLASH_ID_0) - 2*4096;
#endif

    enum NORFLASH_API_RET_T ret;
    uint32_t block_size = 0;
    uint32_t sector_size = 0;
    uint32_t page_size = 0;

    hal_norflash_get_size(HAL_FLASH_ID_0,NULL,&block_size,&sector_size,&page_size);
    ret = norflash_api_register(NORFLASH_API_MODULE_ID_OTA,
                    HAL_FLASH_ID_0,
                    0x0,
                    (OTA_FLASH_LOGIC_ADDR + user_data_nv_flash_offset), // include ota_upgradeLog  and OTA new_imag_offset
                    block_size,
                    sector_size,
                    page_size,
                    OTA_CONTROL_NORFLASH_BUFFER_LEN,
                    ota_control_opera_callback);
    ASSERT(ret == NORFLASH_API_OK, "ota_control_init: norflash_api register failed,ret = %d.",ret);
    hal_norflash_suspend_check_irq(HAL_FLASH_ID_0,AUDMA_IRQn,true);
    hal_norflash_suspend_check_irq(HAL_FLASH_ID_0,ISDATA_IRQn,true);
    hal_norflash_suspend_check_irq(HAL_FLASH_ID_0,ISDATA1_IRQn,true);
    new_image_copy_flash_offset = NEW_IMAGE_FLASH_OFFSET;
}

void app_send_ota_pkt_sync_ack_response(bool ret)
{
#if defined(OTA_DATA_PKT_SYNC_MS) && defined(IBRT)
    static uint8_t ota_pkt_sync_cache = 0;
    ibrt_ctrl_t *p_ibrt_ctrl = app_tws_ibrt_get_bt_ctrl_ctx();

    if(ret == false)
    {
        if((p_ibrt_ctrl->current_role == IBRT_MASTER) && \
           (ota_pkt_sync_cache == OTA_DATA_ACK))
        {
            TRACE(0,"[OTA]send ota pkt sync response");
            ota_control_send_data_ack_response();
            ota_pkt_sync_cache = 0x00;
            return;
        }

        if(p_ibrt_ctrl->current_role == IBRT_MASTER)
        {
            ibrt_ota_cmd_type = OTA_DATA_ACK;
        }
        else if(p_ibrt_ctrl->current_role == IBRT_SLAVE)
        {
            TRACE(0,"[OTA]send ota pkt sync response");
            tws_ctrl_send_cmd(IBRT_OTA_TWS_ACK_CMD, NULL, 0);
        }
    }
    else
    {
        TRACE(1,"[OTA]ibrt_ota_cmd_type %d",ibrt_ota_cmd_type);
        if(ibrt_ota_cmd_type == OTA_DATA_ACK)
        {
            ota_control_send_data_ack_response();
            ota_pkt_sync_cache = 0x00;
            ibrt_ota_cmd_type = 0;
        }
        else
        {
            /*master do not finish write data into flash*/
            ota_pkt_sync_cache = OTA_DATA_ACK;
        }
   }
#endif
}
