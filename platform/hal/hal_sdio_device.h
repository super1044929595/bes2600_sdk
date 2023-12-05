/***************************************************************************
 *
 * Copyright 2015-2021 BES.
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
#ifndef __HAL_SDIO_DEVICE_H__
#define __HAL_SDIO_DEVICE_H__

#ifdef __cplusplus
extern "C" {
#endif

#include "plat_types.h"

enum HAL_SDIO_DEVICE_STATUS {
    HAL_SDIO_DEVICE_NOT_RDY = 0,
    HAL_SDIO_DEVICE_RDY,
    HAL_SDIO_DEVICE_IDLE,
    HAL_SDIO_DEVICE_BUSY,
    HAL_SDIO_DEVICE_TX,
    HAL_SDIO_DEVICE_RX,
};

struct HAL_SDIO_DEVICE_CB_T {
    void (*hal_sdio_device_enum_done)(enum HAL_SDIO_DEVICE_STATUS status);
    void (*hal_sdio_device_rxtx_start)(enum HAL_SDIO_DEVICE_STATUS status);
    void (*hal_sdio_device_send_done)(const uint8_t *buf, uint32_t buf_len);
    void (*hal_sdio_device_recv_done)(uint8_t *buf, uint32_t buf_len);
    void (*hal_sdio_device_int_from_host)(void);
};

//Public use
void hal_sdio_device_open(void);
void hal_sdio_device_close(void);
enum HAL_SDIO_DEVICE_STATUS hal_sdio_device_send(const uint8_t *buf, uint32_t buf_len);
void hal_sdio_device_recv(uint8_t *buf, uint32_t buf_len, uint8_t buf_cnt);
void hal_sdio_device_register_callback(struct HAL_SDIO_DEVICE_CB_T *callback);

//Firmware download dedicated
void hal_sdio_device_download_ready(void);
void hal_sdio_device_rx_success(void);
void hal_sdio_device_rx_error(void);
void hal_sdio_device_fws_already_run(void);
void hal_sdio_device_fws_stop_run(void);

//Under normal circumstances, do not call the following two functions
void hal_sdio_device_change_tplmanid_manf(uint16_t new_manf_id);
void hal_sdio_device_change_tplmanid_card(uint16_t new_card_id);

#ifdef __cplusplus
}
#endif

#endif /* __HAL_SDIO_DEVICE_H__ */

