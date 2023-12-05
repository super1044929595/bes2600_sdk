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
#ifndef __APP_HFP_H__
#define __APP_HFP_H__

#include "bluetooth.h"
#include "btapp.h"

#define HF_VOICE_DISABLE  0
#define HF_VOICE_ENABLE   1

#define HF_SENDBUFF_SIZE (320)
#define HF_SENDBUFF_MEMPOOL_NUM (2)

#define MAX_HFP_VOL    (15)

struct hf_sendbuff_control {
    struct {
        btif_bt_packet_t packet;
        uint8_t buffer[HF_SENDBUFF_SIZE];
    } mempool[HF_SENDBUFF_MEMPOOL_NUM];
    uint8_t index;
};


#ifndef _SCO_BTPCM_CHANNEL_
extern struct hf_sendbuff_control  hf_sendbuff_ctrl;
#endif

#ifdef __cplusplus
extern "C" {
#endif                          /*  */

#if defined(SUPPORT_BATTERY_REPORT) || defined(SUPPORT_HF_INDICATORS)
int app_hfp_battery_report(uint8_t level);
bt_status_t app_hfp_send_at_command(const char *cmd);
#else
static inline int app_hfp_battery_report(uint8_t level) {return 0;}
#endif

uint8_t btapp_hfp_get_call_state(void);

uint8_t btapp_hfp_incoming_calls(void);

bool btapp_hfp_is_call_active(void);

bool btapp_hfp_is_sco_active(void);

void btapp_hfp_report_speak_gain(void);

bool btapp_hfp_mic_need_skip_frame(void);

uint8_t btapp_hfp_need_mute(void);

uint8_t  app_hfp_get_chnl_via_remDev(hf_chan_handle_t * p_hfp_chnl);
bool app_hfp_curr_audio_up(hf_chan_handle_t hfp_chnl);



#ifdef __cplusplus
}
#endif                          /*  */
#endif /*__APP_HFP_H__*/

