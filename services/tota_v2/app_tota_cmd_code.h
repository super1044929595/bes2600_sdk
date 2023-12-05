/***************************************************************************
 *
 * Copyright 2015-2019 BES.
 * All rights reserved. All unpublished rights reserved.
 *
 * No part of this work may be used or reproduced in any form or by any
 * means, or stored in a database or retrieval system, without prior written
 * permission of BES.
 *
 * Use of this work is governed by a license granted by BES.
 * This work contains confidential and proprietary information of
 * BES. which is protected by copyright, trade secret,
 * trademark and other intellectual property rights.
 *
 ****************************************************************************/
#ifndef __APP_TOTA_CMD_CODE_H__
#define __APP_TOTA_CMD_CODE_H__

#include "hal_trace.h"

#ifdef __cplusplus
extern "C" {
#endif

#define LOG_DEBUG_TAG "[TOTA_CONTROL] "

#define TOTA_CONTROL_DEBUG

#ifdef TOTA_CONTROL_DEBUG
#define TOTA_LOG_DBG(num,str,...)   TRACE(num,LOG_DEBUG_TAG"" str, ##__VA_ARGS__)             // DEBUG OUTPUT
#define TOTA_LOG_MSG(num,str,...)   TRACE(num,LOG_DEBUG_TAG"" str, ##__VA_ARGS__)             // MESSAGE OUTPUT
#define TOTA_LOG_ERR(num,str,...)   TRACE(num,LOG_DEBUG_TAG"err:""" str, ##__VA_ARGS__)       // ERROR OUTPUT

#define TOTA_LOG_FUNC_LINE()        TRACE(2,LOG_DEBUG_TAG"%s:%d\n", __FUNCTION__, __LINE__)
#define TOTA_LOG_FUNC_IN()          TRACE(1,LOG_DEBUG_TAG"%s ++++\n", __FUNCTION__)
#define TOTA_LOG_FUNC_OUT()         TRACE(1,LOG_DEBUG_TAG"%s ----\n", __FUNCTION__)

#define TOTA_LOG_DUMP               DUMP8
#else
#define TOTA_LOG_DBG(str,...)
#define TOTA_LOG_MSG(num,str,...)   TRACE(num,LOG_DEBUG_TAG"" str, ##__VA_ARGS__)
#define TOTA_LOG_ERR(num,str,...)   TRACE(num,LOG_DEBUG_TAG"err:""" str, ##__VA_ARGS__)

#define TOTA_LOG_FUNC_LINE()
#define TOTA_LOG_FUNC_IN()
#define TOTA_LOG_FUNC_OUT()

#define TOTA_LOG_DUMP
#endif

extern uint32_t __tota_handler_table_start[];
extern uint32_t __tota_handler_table_end[];

#define INVALID_TOTA_ENTRY_INDEX        0xFFFF
#define TOTA_CMD_CODE_SIZE              sizeof(uint16_t)

/**
 * @brief Type of the tota transmission path.
 *
 */
typedef enum
{
    APP_TOTA_VIA_SPP = 0,
    APP_TOTA_VIA_NOTIFICATION,
    APP_TOTA_VIA_INDICATION,
    APP_TOTA_GEN_VIA_SPP,
    APP_TOTA_TRANSMISSION_PATH_COUNT,

    APP_TOTA_PATH_IDLE = 0xff
} APP_TOTA_TRANSMISSION_PATH_E;

/**
 * @brief The command code
 *
 */
typedef enum
{
    OP_TOTA_NONE                = 0x0000,
    /* basic cmd */
    OP_TOTA_STRING              = 0x1000,
    OP_TOTA_CONN_INITIATE       = 0x1001,
    OP_TOTA_CONN_RESPONSE       = 0x1002,
    OP_TOTA_CONN_CONFIRM        = 0x1003,
    OP_TOTA_DATA_ENCODE         = 0x1004,

    /* response cmd */
    OP_TOTA_RESPONSE_TO_CMD     = 0x6000, /**< the payload is: OP_TOTA_RESPONSE_TO_CMD + paramLen + BLE_TOTA_CMD_RSP_T */
    OP_TOTA_SPP_DATA_ACK        = 0x6001,

    /* test cmd: test ok */
    OP_TOTA_TEST_CMD            = 0x6100,
    OP_TOTA_ECHO_TEST_CMD       = 0x6101,
    OP_TOTA_DEMO_CMD            = 0x6102,

    /* flash cmd: test ok */
    OP_TOTA_WRITE_FLASH_CMD     = 0x6200,
    OP_TOTA_ERASE_FLASH_CMD     = 0x6201,
    OP_TOTA_READ_FLASH_CMD      = 0x6202,
    OP_TOTA_GET_SECINFO_EQ      = 0x6203,
    OP_TOTA_GET_SECINFO_FAC     = 0x6204,
    OP_TOTA_GET_SECINFO_ANC     = 0x6205, 

    /* general info cmd: test ok */
    OP_TOTA_GENERAL_INFO_CMD    = 0x6300,
    OP_TOTA_EXCHANGE_MTU_CMD    = 0x6301,
    OP_TOTA_VOLUME_PLUS_CMD     = 0x6302,
    OP_TOTA_VOLUME_DEC_CMD      = 0x6303,
    OP_TOTA_VOLUME_SET_CMD      = 0x6304,
    OP_TOTA_VOLUME_GET_CMD      = 0x6305,
    OP_TOTA_GET_RSSI_CMD        = 0x6306,
    OP_TOTA_GET_DUMP_INFO_CMD   = 0x6307,
    OP_TOTA_SET_CUSTOMER_CMD    = 0x6308,
    OP_TOTA_GET_CUSTOMER_CMD    = 0x6309,
    OP_TOTA_CRC_CHECK_CMD       = 0x6310,

    /* audio dump and mic cmd */
    OP_TOTA_AUDIO_DUMP_START    = 0x6400,
    OP_TOTA_AUDIO_DUMP_STOP     = 0x6401,
    OP_TOTA_AUDIO_DUMP_CONTROL  = 0x6402,
    OP_TOTA_MIC_TEST_ON         = 0x6403,
    OP_TOTA_MIC_TEST_OFF        = 0x6404,
    OP_TOTA_MIC_SWITCH          = 0x6405,
    OP_TOTA_MIC_FULL_DUT_TEST_CLOSE = 0x6406,

    /* for big data transfer */
    OP_TOTA_DATA_CTL            = 0x6500,
    OP_TOTA_DATA                = 0x6501,

    /* EQ info cmd: test ok */
    OP_TOTA_MERIDIAN_EFFECT_CMD = 0x6600,
    OP_TOTA_EQ_SELECT_CMD       = 0x6601,
    OP_TOTA_EQ_SET_CMD          = 0x6602,
    OP_TOTA_EQ_GET_CMD          = 0x6603,
    OP_TOTA_EQ_SET_DATA         = 0x6604,

    /* ANC cmd */
    OP_TOTA_ANC_CONTROL_INIT_CMD    = 0x6800,
    OP_TOTA_ANC_CONTROL_DATA_CMD    = 0x6801,
    OP_TOTA_ANC_CONTROL_DEINIT_CMD  = 0x6802,
    OP_TOTA_ANC_CONTROL_ADDR_CMD    = 0x6803,

    /* custom cmd */
    // TODO:
    OP_TOTA_FACTORY_RESET           = 0x8000,
    OP_TOTA_CLEAR_PAIRING_INFO      = 0x8001,
    OP_TOTA_SHUTDOWM                = 0x8002,
    OP_TOTA_REBOOT                  = 0x8003,
    OP_TOTA_IR                      = 0x8004,

    OP_TOTA_OTA                     = 0x9000,

    // TODO:?
    OP_TOTA_RAW_DATA_SET_CMD = 0x9100,

    /* commond count */
    OP_TOTA_COMMAND_COUNT,
    /* to mark that it's a data packet */
    OP_TOTA_STREAM_DATA = 0xFFFF,
} APP_TOTA_CMD_CODE_E;

/**
 * @brief The tota command handling return status
 *
 */
typedef enum
{
    TOTA_NO_ERROR = 0,
    TOTA_INVALID_CMD = 1,
    TOTA_INVALID_DATA_PACKET = 2,
    TOTA_PARAM_LEN_OUT_OF_RANGE = 3,
    TOTA_PARAMETER_LENGTH_TOO_SHORT = 4,
    TOTA_PARAM_LEN_TOO_SHORT = 5,
    TOTA_CMD_HANDLING_FAILED = 6,
    TOTA_WAITING_RSP_TIMEOUT = 7,
    TOTA_DATA_XFER_ALREADY_STARTED = 8,
    TOTA_DATA_XFER_NOT_STARTED_YET = 9,
    TOTA_DATA_SEGMENT_CRC_CHECK_FAILED = 10,
    TOTA_WHOLE_DATA_CRC_CHECK_FAILED = 11,
    TOTA_DATA_XFER_LEN_NOT_MATCHED = 12,
    TOTA_WRITE_FLASH_CRC_CHECK_FAILED = 13,
    TOTA_MIC_SWITCH_FAILED,
    TOTA_EQ_GET_FAILED,
    TOTA_RECEIVE_STREAM_TIMEOUT,
     TOTA_LEAK_DETECTED_FAILED,
    // TO ADD: new return status
} APP_TOTA_CMD_RET_STATUS_E;

/**
 * @brief Format of the tota command handler function, called when the command is received
 *
 * @param cmdCode	Custom command code
 * @param ptrParam 	Pointer of the received parameter
 * @param paramLen 	Length of the recevied parameter
 * 
 */
typedef void (*app_tota_cmd_handler_t)(APP_TOTA_CMD_CODE_E cmdCode, uint8_t* ptrParam, uint32_t paramLen);

/**
 * @brief Format of the tota command response handler function, 
 *	called when the response to formerly sent command is received
 *
 * @param retStatus	Handling return status of the command
 * @param ptrParam 	Pointer of the received parameter
 * @param paramLen 	Length of the recevied parameter
 * 
 */
typedef void (*app_tota_cmd_response_handler_t)(APP_TOTA_CMD_RET_STATUS_E retStatus, uint8_t* ptrParam, uint32_t paramLen);

/**
 * @brief Smart voice command definition data structure
 *
 */
typedef struct
{
    uint32_t                cmdCode;
    app_tota_cmd_handler_t  cmdHandler;             /**< command handler function */
    uint16_t                isNeedResponse;         /**< true if needs the response from the peer device */
    uint16_t                timeoutWaitingRspInMs;  /**< time-out of waiting for response in milli-seconds */
    app_tota_cmd_response_handler_t	cmdRspHandler;  /**< command response handler function */
} APP_TOTA_CMD_INSTANCE_T;


#define TOTA_COMMAND_TO_ADD(cmdCode, cmdHandler, isNeedResponse, timeoutWaitingRspInMs, cmdRspHandler)	\
	static const APP_TOTA_CMD_INSTANCE_T cmdCode##_entry __attribute__((used, section(".tota_handler_table"))) = 	\
		{(cmdCode), (cmdHandler), (isNeedResponse), (timeoutWaitingRspInMs), (cmdRspHandler)};

#define TOTA_COMMAND_PTR_FROM_ENTRY_INDEX(index)	\
	((APP_TOTA_CMD_INSTANCE_T *)((uint32_t)__tota_handler_table_start + (index)*sizeof(APP_TOTA_CMD_INSTANCE_T)))

#ifdef __cplusplus
	}
#endif


#endif // #ifndef __APP_TOTA_CMD_CODE_H__

