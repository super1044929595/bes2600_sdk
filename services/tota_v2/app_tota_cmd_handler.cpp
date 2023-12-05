/**
 ****************************************************************************************
 *
 * @file app_totac_cmd_handler.c
 *
 * @date 24th April 2018
 *
 * @brief The framework of the tota command handler
 *
 * Copyright (C) 2017
 *
 *
 ****************************************************************************************
 */
#include "string.h"
#include "cmsis_os.h"
#include "hal_trace.h"
#include "hal_timer.h"
#include "apps.h"
#include "stdbool.h"
#include "app_tota.h"
#include "app_tota_cmd_handler.h"
#include "app_tota_cmd_code.h"
#include "app_tota_common.h"
//#include "rwapp_config.h"
#include "app_spp_tota.h"
#ifdef __IAG_BLE_INCLUDE__
#include "app_tota_ble.h"
#endif
#include "tota_stream_data_transfer.h"
#define APP_TOTA_CMD_HANDLER_WAITING_RSP_TIMEOUT_SUPERVISOR_COUNT   8

/**
 * @brief waiting response timeout supervision data structure
 *
 */
typedef struct
{
    uint16_t    entryIndex;     /**< The command waiting for the response */
    uint16_t    msTillTimeout;  /**< run-time timeout left milliseconds */
} APP_TOTA_CMD_WAITING_RSP_SUPERVISOR_T;

/**
 * @brief tota command handling environment
 *
 */
typedef struct
{
    APP_TOTA_CMD_WAITING_RSP_SUPERVISOR_T waitingRspTimeoutInstance[APP_TOTA_CMD_HANDLER_WAITING_RSP_TIMEOUT_SUPERVISOR_COUNT];
    uint32_t    lastSysTicks;
    uint8_t     timeoutSupervisorCount;
    osTimerId   supervisor_timer_id;
    osMutexId   mutex;
} APP_TOTA_CMD_HANDLER_ENV_T;

static APP_TOTA_CMD_HANDLER_ENV_T tota_cmd_handler_env;

osMutexDef(app_tota_cmd_handler_mutex);

static void app_tota_cmd_handler_rsp_supervision_timer_cb(void const *n);
osTimerDef (APP_TOTA_CMD_HANDLER_RSP_SUPERVISION_TIMER, app_tota_cmd_handler_rsp_supervision_timer_cb);

static void app_tota_cmd_handler_remove_waiting_rsp_timeout_supervision(uint16_t entryIndex);



/**
 * @brief Callback function of the waiting response supervisor timer.
 *
 */
static void app_tota_cmd_handler_rsp_supervision_timer_cb(void const *n)
{
    uint32_t entryIndex = tota_cmd_handler_env.waitingRspTimeoutInstance[0].entryIndex;

    app_tota_cmd_handler_remove_waiting_rsp_timeout_supervision(entryIndex);

    // it means time-out happens before the response is received from the peer device,
    // trigger the response handler
    TOTA_COMMAND_PTR_FROM_ENTRY_INDEX(entryIndex)->cmdRspHandler(TOTA_WAITING_RSP_TIMEOUT, NULL, 0);
}

APP_TOTA_CMD_INSTANCE_T* app_tota_cmd_handler_get_entry_pointer_from_cmd_code(APP_TOTA_CMD_CODE_E cmdCode)
{
    for (uint32_t index = 0;
        index < ((uint32_t)__tota_handler_table_end-(uint32_t)__tota_handler_table_start)/sizeof(APP_TOTA_CMD_INSTANCE_T);index++)
    {
        if (TOTA_COMMAND_PTR_FROM_ENTRY_INDEX(index)->cmdCode == cmdCode)
        {
            return TOTA_COMMAND_PTR_FROM_ENTRY_INDEX(index);
        }
    }

    return NULL;
}

uint16_t app_tota_cmd_handler_get_entry_index_from_cmd_code(APP_TOTA_CMD_CODE_E cmdCode)
{

    for (uint32_t index = 0;
        index < ((uint32_t)__tota_handler_table_end-(uint32_t)__tota_handler_table_start)/sizeof(APP_TOTA_CMD_INSTANCE_T);index++)
    {
        if (TOTA_COMMAND_PTR_FROM_ENTRY_INDEX(index)->cmdCode == cmdCode)
        {
            return index;
        }
    }

    return INVALID_TOTA_ENTRY_INDEX;
}

void app_tota_get_cmd_response_handler(APP_TOTA_CMD_CODE_E funcCode, uint8_t* ptrParam, uint32_t paramLen)
{
    // parameter length check
    if (paramLen > sizeof(APP_TOTA_CMD_RSP_T))
    {
        return;
    }

    if (0 == tota_cmd_handler_env.timeoutSupervisorCount)
    {
        return;
    }

    APP_TOTA_CMD_RSP_T* rsp = (APP_TOTA_CMD_RSP_T *)ptrParam;


    uint16_t entryIndex = app_tota_cmd_handler_get_entry_index_from_cmd_code((APP_TOTA_CMD_CODE_E)(rsp->cmdCodeToRsp));
    if (INVALID_TOTA_ENTRY_INDEX == entryIndex)
    {
        return;
    }

    // remove the function code from the time-out supervision chain
    app_tota_cmd_handler_remove_waiting_rsp_timeout_supervision(entryIndex);

    APP_TOTA_CMD_INSTANCE_T* ptCmdInstance = TOTA_COMMAND_PTR_FROM_ENTRY_INDEX(entryIndex);

    // call the response handler
    if (ptCmdInstance->cmdRspHandler)
    {
        ptCmdInstance->cmdRspHandler((APP_TOTA_CMD_RET_STATUS_E)(rsp->cmdRetStatus), rsp->rspData, rsp->rspDataLen);
    }
}



/**
 * @brief Refresh the waiting response supervisor list
 *
 */
static void app_tota_cmd_refresh_supervisor_env(void)
{
    // do nothing if no supervisor was added
    if (tota_cmd_handler_env.timeoutSupervisorCount > 0)
    {
        uint32_t currentTicks = GET_CURRENT_TICKS();
        uint32_t passedTicks;
        if (currentTicks >= tota_cmd_handler_env.lastSysTicks)
        {
            passedTicks = (currentTicks - tota_cmd_handler_env.lastSysTicks);
        }
        else
        {
            passedTicks = (hal_sys_timer_get_max() - tota_cmd_handler_env.lastSysTicks + 1) + currentTicks;
        }

        uint32_t deltaMs = TICKS_TO_MS(passedTicks);

        APP_TOTA_CMD_WAITING_RSP_SUPERVISOR_T* pRspSupervisor = &(tota_cmd_handler_env.waitingRspTimeoutInstance[0]);
        for (uint32_t index = 0;index < tota_cmd_handler_env.timeoutSupervisorCount;index++)
        {
            if (pRspSupervisor[index].msTillTimeout > deltaMs)
            {
                pRspSupervisor[index].msTillTimeout -= deltaMs;
            }
            else
            {
                pRspSupervisor[index].msTillTimeout = 0;
            }
        }
    }

    tota_cmd_handler_env.lastSysTicks = GET_CURRENT_TICKS();
}

/**
 * @brief Remove the time-out supervision of waiting response
 *
 * @param entryIndex    Entry index of the command table
 *
 */
static void app_tota_cmd_handler_remove_waiting_rsp_timeout_supervision(uint16_t entryIndex)
{
    ASSERT(tota_cmd_handler_env.timeoutSupervisorCount > 0,
        "%s The BLE tota command time-out supervisor is already empty!!!", __FUNCTION__);

    osMutexWait(tota_cmd_handler_env.mutex, osWaitForever);

    uint32_t index;
    for (index = 0;index < tota_cmd_handler_env.timeoutSupervisorCount;index++)
    {
        if (tota_cmd_handler_env.waitingRspTimeoutInstance[index].entryIndex == entryIndex)
        {
            memcpy(&(tota_cmd_handler_env.waitingRspTimeoutInstance[index]),
                &(tota_cmd_handler_env.waitingRspTimeoutInstance[index + 1]),
                (tota_cmd_handler_env.timeoutSupervisorCount - index - 1)*sizeof(APP_TOTA_CMD_WAITING_RSP_SUPERVISOR_T));
            break;
        }
    }

    // cannot find it, directly return
    if (index == tota_cmd_handler_env.timeoutSupervisorCount)
    {
        goto exit;
    }

    tota_cmd_handler_env.timeoutSupervisorCount--;

    //TOTA_LOG_DBG(1,"decrease the supervisor timer to %d", tota_cmd_handler_env.timeoutSupervisorCount);

    if (tota_cmd_handler_env.timeoutSupervisorCount > 0)
    {
        // refresh supervisor environment firstly
        app_tota_cmd_refresh_supervisor_env();

        // start timer, the first entry is the most close one
        osTimerStart(tota_cmd_handler_env.supervisor_timer_id,
            tota_cmd_handler_env.waitingRspTimeoutInstance[0].msTillTimeout);
    }
    else
    {
        // no supervisor, directly stop the timer
        osTimerStop(tota_cmd_handler_env.supervisor_timer_id);
    }

exit:
    osMutexRelease(tota_cmd_handler_env.mutex);
}

/**
 * @brief Add the time-out supervision of waiting response
 *
 * @param entryIndex    Index of the command entry
 *
 */
void app_tota_cmd_handler_add_waiting_rsp_timeout_supervision(uint16_t entryIndex)
{
    ASSERT(tota_cmd_handler_env.timeoutSupervisorCount < APP_TOTA_CMD_HANDLER_WAITING_RSP_TIMEOUT_SUPERVISOR_COUNT,
        "%s The tota command response time-out supervisor is full!!!", __FUNCTION__);

    osMutexWait(tota_cmd_handler_env.mutex, osWaitForever);

    // refresh supervisor environment firstly
    app_tota_cmd_refresh_supervisor_env();

    APP_TOTA_CMD_INSTANCE_T* pInstance = TOTA_COMMAND_PTR_FROM_ENTRY_INDEX(entryIndex);

    APP_TOTA_CMD_WAITING_RSP_SUPERVISOR_T   waitingRspTimeoutInstance[APP_TOTA_CMD_HANDLER_WAITING_RSP_TIMEOUT_SUPERVISOR_COUNT];

    uint32_t index = 0, insertedIndex = 0;
    for (index = 0;index < tota_cmd_handler_env.timeoutSupervisorCount;index++)
    {
        uint32_t msTillTimeout = tota_cmd_handler_env.waitingRspTimeoutInstance[index].msTillTimeout;

        // in the order of low to high
        if ((tota_cmd_handler_env.waitingRspTimeoutInstance[index].entryIndex != entryIndex) &&
            (pInstance->timeoutWaitingRspInMs >= msTillTimeout))
        {
            waitingRspTimeoutInstance[insertedIndex++] = tota_cmd_handler_env.waitingRspTimeoutInstance[index];
        }
        else if (pInstance->timeoutWaitingRspInMs < msTillTimeout)
        {
            waitingRspTimeoutInstance[insertedIndex].entryIndex = entryIndex;
            waitingRspTimeoutInstance[insertedIndex].msTillTimeout = pInstance->timeoutWaitingRspInMs;

            insertedIndex++;
        }
    }

    // biggest one? then put it at the end of the list
    if (tota_cmd_handler_env.timeoutSupervisorCount == index)
    {
        waitingRspTimeoutInstance[insertedIndex].entryIndex = entryIndex;
        waitingRspTimeoutInstance[insertedIndex].msTillTimeout = pInstance->timeoutWaitingRspInMs;

        insertedIndex++;
    }

    // copy to the global variable
    memcpy((uint8_t *)&(tota_cmd_handler_env.waitingRspTimeoutInstance), (uint8_t *)&waitingRspTimeoutInstance,
        insertedIndex*sizeof(APP_TOTA_CMD_WAITING_RSP_SUPERVISOR_T));

    tota_cmd_handler_env.timeoutSupervisorCount = insertedIndex;

    //TOTA_LOG_DBG(1,"increase the supervisor timer to %d", tota_cmd_handler_env.timeoutSupervisorCount);


    osMutexRelease(tota_cmd_handler_env.mutex);

    // start timer, the first entry is the most close one
    osTimerStart(tota_cmd_handler_env.supervisor_timer_id, tota_cmd_handler_env.waitingRspTimeoutInstance[0].msTillTimeout);

}

/**
 * @brief Receive the data from the peer device and parse them
 *
 * @param ptrData       Pointer of the received data
 * @param dataLength    Length of the received data
 *
 * @return APP_TOTA_CMD_RET_STATUS_E
 */
APP_TOTA_CMD_RET_STATUS_E app_tota_cmd_received(uint8_t* ptrData, uint32_t dataLength)
{
    TOTA_LOG_DBG(0,"TOTA Receive data:");
    TOTA_LOG_DUMP("0x%02x ", ptrData, dataLength>20?20:dataLength);
    APP_TOTA_CMD_PAYLOAD_T* pPayload = (APP_TOTA_CMD_PAYLOAD_T *)ptrData;

    // check command code
    if (pPayload->cmdCode >= OP_TOTA_COMMAND_COUNT || pPayload->cmdCode < OP_TOTA_STRING)
    {
        TOTA_LOG_DBG(1,"[%s]error TOTA_INVALID_CMD",__func__);
        return TOTA_INVALID_CMD;
    }

    // check parameter length
    if ((pPayload->paramLen + TOTA_DATA_HEAD_SIZE) > (uint16_t)dataLength)
    {
        TOTA_LOG_DBG(3,"[%s]error TOTA_PARAMETER_LENGTH_TOO_SHORT:0x%x > 0x%x ",__func__,pPayload->paramLen, dataLength);
        return TOTA_PARAMETER_LENGTH_TOO_SHORT;
    }

    APP_TOTA_CMD_INSTANCE_T* pInstance = app_tota_cmd_handler_get_entry_pointer_from_cmd_code((APP_TOTA_CMD_CODE_E)(pPayload->cmdCode));

    // execute the command handler
    if(pInstance) {
        pInstance->cmdHandler((APP_TOTA_CMD_CODE_E)(pPayload->cmdCode), pPayload->param, pPayload->paramLen);
    }

    return TOTA_NO_ERROR;
}

/**
 * @brief Initialize the tota command handler framework
 *
 */
void app_tota_cmd_handler_init(void)
{
    memset((uint8_t *)&tota_cmd_handler_env, 0, sizeof(tota_cmd_handler_env));

    tota_cmd_handler_env.supervisor_timer_id =
        osTimerCreate(osTimer(APP_TOTA_CMD_HANDLER_RSP_SUPERVISION_TIMER), osTimerOnce, NULL);

    tota_cmd_handler_env.mutex = osMutexCreate((osMutex(app_tota_cmd_handler_mutex)));
}

TOTA_COMMAND_TO_ADD(OP_TOTA_RESPONSE_TO_CMD, app_tota_get_cmd_response_handler, false, 0, NULL );

