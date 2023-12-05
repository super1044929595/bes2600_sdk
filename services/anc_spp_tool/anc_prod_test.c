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
#include "hal_aud.h"
#include "hal_cmu.h"
#include "hal_dma.h"
#include "hal_iomux.h"
#include "hal_sysfreq.h"
#include "hal_trace.h"
#include "aud_section.h"
#include "aud_section_inc.h"
#include "anc_process.h"
#include "pmu.h"
#include "audioflinger.h"
#include "analog.h"
#include "hwtimer_list.h"
#include "tgt_hardware.h"
#include "tool_msg.h"
#include "string.h"
#include "app_tota_cmd_code.h"
#include "app_tota_data_handler.h"
#include "anc_parse_data.h"
//#include "sys_api_programmer.h"
#include "hal_sleep.h"

//#define TR_DEBUG(num,s,...) TRACE(num,s, ##__VA_ARGS__)

enum ANC_CMD_T {
    ANC_CMD_CLOSE = 0,
    ANC_CMD_OPEN = 1,
    ANC_CMD_GET_CFG = 2,
    ANC_CMD_APPLY_CFG = 3,
    ANC_CMD_CFG_SETUP = 4,
    ANC_CMD_CHANNEL_SETUP = 5,
    ANC_CMD_SET_SAMP_RATE = 6,
};
static bool anc_running = false;

//extern enum PROGRAMMER_STATE programmer_state;
extern enum PARSE_STATE parse_state;
extern struct message_t send_msg;
extern struct message_t recv_msg;
extern unsigned char send_seq;

enum AUD_CHANNEL_MAP_T anc_ff_mic_ch_l;
enum AUD_CHANNEL_MAP_T anc_ff_mic_ch_r;
enum AUD_CHANNEL_MAP_T anc_fb_mic_ch_l;
enum AUD_CHANNEL_MAP_T anc_fb_mic_ch_r;

enum PROD_TEST_CMD_T {
    PROD_TEST_CMD_ANC = 0x00000001,
};

enum RUNTIME_CMD_T {
    RUNTIME_CMD_GET_EQ_ADDR = 0x00000001,
    RUNTIME_CMD_FLUSH_AUD_SEC = 0x00000001,
};


static void anc_prod_test_open(void);

static struct_anc_cfg g_anc_config;
//static uint8_t g_anc_config[20];

#ifdef CHIP_BEST1000
static uint8_t pcsuppt_vcodec_set = 0x5;
static uint8_t pcsuppt_vana_set = 0xa;
static uint8_t pcsuppt_tx_gain_set = 0x10;
#else
static uint8_t pcsuppt_anc_type = ANC_NOTYPE;
static enum AUD_SAMPRATE_T anc_sample_rate;
#endif

static struct_anc_cfg *get_anc_config(void)
{
//    return (struct_anc_cfg *)g_anc_config;
    return &g_anc_config;
}

#ifdef PROGRAMMER_ANC_DEBUG
//BEST1000 ANC programmer debug.
static const struct_anc_cfg POSSIBLY_UNUSED wf_cust_auditem = {
    .anc_cfg_ff_l = {
        .gain0 = 0,
        .gain1 = 0x62,
        .len = 500,
        .coef =
        {
            28192
        },
    },
    .anc_cfg_ff_r = {
        .gain0 = 0,
        .gain1 = 0x62,
        .len = 500,
        .coef =
        {
            28192
        },
    }
};

void anc_prod_test_wf_coef(){
    anc_set_cfg(&wf_cust_auditem,ANC_FEEDFORWARD,ANC_GAIN_NO_DELAY);
}
#endif

#ifdef CHIP_BEST1000
static uint32_t anc_cmd_setup()
{
    uint8_t vcodec = pcsuppt_vcodec_set;
    uint8_t vana = pcsuppt_vana_set;
    uint8_t tx_gain = pcsuppt_tx_gain_set;

    /*dcdc vana set*/
    if (pmu_debug_config_ana(vana)) {
        return 1;
    }
    /*vcodec set*/
    if (pmu_debug_config_codec(vcodec)) {
        return 1;
    }
    if(tx_gain > 0x1f)
        return 1;
    analog_aud_set_dac_gain(tx_gain);
    /*anc cmd custom:*/
    // anc_set_gpio();
    return 0;
}
#else
static void best2000_prod_test_anccfg_apply(void)
{
    struct_anc_cfg *anccfg = get_anc_config();
    if(pcsuppt_anc_type & ANC_FEEDFORWARD)
        anc_set_cfg(anccfg,ANC_FEEDFORWARD,ANC_GAIN_NO_DELAY);
    if(pcsuppt_anc_type & ANC_FEEDBACK)
        anc_set_cfg(anccfg,ANC_FEEDBACK,ANC_GAIN_NO_DELAY);
}

static void prod_test_anc_open_internal(void)
{
    enum AUD_SAMPRATE_T play_rate, capture_rate;

    play_rate = anc_sample_rate;
    capture_rate = anc_sample_rate;


    if(pcsuppt_anc_type & ANC_FEEDFORWARD)
    {
        // TR_DEBUG(2,"==============================================func %s,ln %d========================================",__FUNCTION__,__LINE__);
        af_anc_open(ANC_FEEDFORWARD, play_rate, capture_rate, NULL);
        anc_open(ANC_FEEDFORWARD);
        anc_select_coef(play_rate, 0, ANC_FEEDFORWARD, ANC_GAIN_DELAY);
        anc_set_gain(512, 512, ANC_FEEDFORWARD);

        // TR_DEBUG(2,"============================func %s, pcsuppt_anc_type 0x%x=========================",__FUNCTION__, pcsuppt_anc_type);
        //anc_select_coef(play_rate,cur_coef_index,ANC_FEEDFORWARD,ANC_GAIN_DELAY);
    }

    if(pcsuppt_anc_type & ANC_FEEDBACK)
    {
        // TR_DEBUG(2,"==============================================func %s,ln %d========================================",__FUNCTION__,__LINE__);
        af_anc_open(ANC_FEEDBACK, play_rate, capture_rate, NULL);
        anc_open(ANC_FEEDBACK);
        anc_select_coef(play_rate, 0, ANC_FEEDBACK, ANC_GAIN_DELAY);
        anc_set_gain(512, 512, ANC_FEEDBACK);
        
        //anc_select_coef(play_rate,cur_coef_index,ANC_FEEDBACK,ANC_GAIN_DELAY);
    }
}

#if 0
static void prod_test_anc_close_internal(void)
{
    if(pcsuppt_anc_type & ANC_FEEDFORWARD)
    {
        anc_close(ANC_FEEDFORWARD);
        af_anc_close(ANC_FEEDFORWARD);
    }

    if(pcsuppt_anc_type & ANC_FEEDBACK)
    {
        anc_close(ANC_FEEDBACK);
        af_anc_close(ANC_FEEDBACK);
    }
}
#endif

static void best2000_prod_test_anc_open(void)
{
    enum HAL_CMU_FREQ_T freq;

    freq = HAL_CMU_FREQ_52M;
    hal_sysfreq_req(HAL_SYSFREQ_USER_APP_1, freq);
    pmu_anc_config(1);
    prod_test_anc_open_internal();
}
#endif

void anc_set_gpio(enum HAL_GPIO_PIN_T io_pin, bool set_flag)
{
// #define ANC_SET_GPIO_PIN   HAL_IOMUX_PIN_P2_4
    struct HAL_IOMUX_PIN_FUNCTION_MAP pinmux_anc_set_gpio[1] = {
        {io_pin, HAL_IOMUX_FUNC_AS_GPIO, HAL_IOMUX_PIN_VOLTAGE_VIO, HAL_IOMUX_PIN_NOPULL},
    };

    hal_iomux_init(pinmux_anc_set_gpio, ARRAY_SIZE(pinmux_anc_set_gpio));
    hal_gpio_pin_set_dir(io_pin, HAL_GPIO_DIR_OUT, 0);
    if(set_flag)
        hal_gpio_pin_set(io_pin);
    else
        hal_gpio_pin_clr(io_pin);
}

static enum ERR_CODE anc_check_msg_hdr(struct CUST_CMD_PARAM *param)
{
    const struct message_t *msg = param->msg;

    if (msg->hdr.type != TYPE_PROD_TEST) {
        return ERR_TYPE_INVALID;
    }

    if (msg->hdr.len < 4 || msg->hdr.len > 20) {
        TR_DEBUG(1,"PROD_TEST msg length error: %d", msg->hdr.len);
        return ERR_LEN;
    }

    return ERR_NONE;
}


void anc_switch_byspp(bool on)
{
    TR_DEBUG(2,"%s: on=%d", __func__, on);

    if (anc_running==on)
        return;
    else
        anc_running=on;

    if (on) {
        anc_enable();
    } else {
        anc_disable();
    }

#ifdef CFG_HW_ANC_LED_PIN
    if (on) {
        hal_gpio_pin_set(CFG_HW_ANC_LED_PIN);
    } else {
        hal_gpio_pin_clr(CFG_HW_ANC_LED_PIN);
    }
#endif
#ifdef CFG_HW_ANC_LED_PIN2
    if (on) {
        hal_gpio_pin_set(CFG_HW_ANC_LED_PIN2);
    } else {
        hal_gpio_pin_clr(CFG_HW_ANC_LED_PIN2);
    }
#endif
}

int send_data(const unsigned char *buf, size_t len)
{
    //send data by spp
    int ret=0;
    app_anc_tota_send_data(APP_TOTA_VIA_SPP, (uint8_t*)buf,(uint32_t)len);
    return ret;
}

static unsigned char check_sum(const unsigned char *buf, unsigned char len)
{
    int i;
    unsigned char sum = 0;

    for (i = 0; i < len; i++) {
        sum += buf[i];
    }

    return sum;
}


int send_reply(const unsigned char *payload, unsigned int len)
{
    int ret = 0;

    if (len + 1 > sizeof(send_msg.data)) {
        TR_DEBUG(1,"Packet length too long: %u", len);
        return -1;
    }

    send_msg.hdr.type = recv_msg.hdr.type;
    send_msg.hdr.seq = recv_msg.hdr.seq;
    send_msg.hdr.len = len;
    memcpy(&send_msg.data[0], payload, len);
    send_msg.data[len] = ~check_sum((unsigned char *)&send_msg, MSG_TOTAL_LEN(&send_msg) - 1);

    ret = send_data((unsigned char *)&send_msg, MSG_TOTAL_LEN(&send_msg));

    return ret;
}

static enum ERR_CODE anc_cmd_handler(enum ANC_CMD_T cmd, const uint8_t *data, uint32_t len)
{
    unsigned char cret = ERR_NONE;

    switch (cmd) {
        case ANC_CMD_CLOSE:
            if (len != 0) {
                return ERR_LEN;
            }
            anc_switch_byspp(false);
            hal_cpu_wake_unlock(HAL_CPU_WAKE_LOCK_USER_5);
            send_reply(&cret, 1);
            break;
        case ANC_CMD_OPEN: {
            if (len != 0) {
                return ERR_LEN;
            }
            analog_debug_config_anc_calib_mode(true);
            anc_prod_test_open();
            hal_cpu_wake_lock(HAL_CPU_WAKE_LOCK_USER_5);
#ifdef CHIP_BEST1000
            TR_DEBUG(2,"ANC_CMD_OPEN with pcsuppt_vcodec_set = 0x%x, pcsuppt_vana_set = 0x%x", pcsuppt_vcodec_set, pcsuppt_vana_set);
            cret = anc_cmd_setup();
#else
            cret = 0;
#endif
            anc_switch_byspp(true);
            send_reply(&cret, 1);
#ifdef PROGRAMMER_ANC_DEBUG
            //BEST1000 ANC programmer debug.
            anc_prod_test_wf_coef();
            hal_iomux_set_analog_i2c();
#endif
            break;
        }
        case ANC_CMD_GET_CFG: {
                struct_anc_cfg *anccfg_addr = get_anc_config();
                uint32_t addr = (uint32_t)anccfg_addr;

                if (len != 0) {
                    return ERR_LEN;
                }

                TR_DEBUG(1,"send anccfg address 0x%x ------",addr);
                send_reply((unsigned char *)&addr, sizeof(addr));
                break;
            }
        case ANC_CMD_APPLY_CFG: {
                if (len != 0) {
                    return ERR_LEN;
                }
                TR_DEBUG(0,"apply anccfg ------");
#ifdef CHIP_BEST1000
                anc_set_cfg(get_anc_config(),ANC_FEEDFORWARD,ANC_GAIN_NO_DELAY);
#else
                best2000_prod_test_anccfg_apply();
#endif
                send_reply(&cret, 1);
                break;
            }
        case ANC_CMD_CFG_SETUP:
        {
#if 0
#ifdef CHIP_BEST1000
            pcsuppt_vcodec_set = data[0];
            pcsuppt_vana_set = data[1];
            pcsuppt_tx_gain_set = data[2];
            TR_DEBUG(2,"ANC_CMD_CFG_SETUP pcsuppt_vcodec_set = %d, pcsuppt_vana_set = %d", pcsuppt_vcodec_set, pcsuppt_vana_set);
#else
            int ret = 0;
            bool diff;
            bool high_performance_adc;
            bool vcrystal_on;
            uint16_t vcodec;
            pctool_iocfg *iocfg1, *iocfg2;

            pcsuppt_anc_type = data[0];
            vcodec = (data[1] | (data[2] << 8));
            diff = (bool)data[3];
            high_performance_adc = false; //(bool)data[4]; // default 0
            anc_sample_rate = (data[5] | (data[6] << 8) | (data[7] << 16) | (data[8] << 24));
            vcrystal_on = data[9]; //(bool)data[?]; // default 0
            TR_DEBUG(4,"vcodec:%d,diff:%d,anc_sample_rate:%d,vcrystal_on:%d.", vcodec,diff,anc_sample_rate,vcrystal_on);
            ret |= pmu_debug_config_vcrystal(vcrystal_on);
            ret |= anatr_debug_config_audio_output(diff);
            ret |= anatr_debug_config_codec(vcodec);
            ret |= anatr_debug_config_low_power_adc(!high_performance_adc);
            if (anc_sample_rate == AUD_SAMPRATE_50781) {
                hal_cmu_audio_resample_enable();
            } else {
                hal_cmu_audio_resample_disable();
            }
            iocfg1 = (pctool_iocfg *)(&data[10]);
            iocfg2 = (pctool_iocfg *)(&data[12]);
            TR_DEBUG(4,"io cfg:%d    %d    %d    %d",iocfg1->io_pin,iocfg1->set_flag,iocfg2->io_pin,iocfg2->set_flag);
            if(0 != iocfg1->io_pin)
                anc_set_gpio(iocfg1->io_pin,iocfg1->set_flag);
            if(0 != iocfg2->io_pin)
                anc_set_gpio(iocfg2->io_pin,iocfg2->set_flag);
          //  hal_iomux_set_dig_mic_clock_index(data[13]);
#endif
            cret = ret ? ERR_INTERNAL : ERR_NONE;
#else
            if(data[0]==1)
            {
                pcsuppt_anc_type = ANC_FEEDFORWARD;
            }
            else if(data[0]==2)
            {
                pcsuppt_anc_type = ANC_FEEDBACK;
            }
            else if(data[0]==3)
            {
                pcsuppt_anc_type = ANC_FEEDFORWARD|ANC_FEEDBACK;
            }
            else
            {
                pcsuppt_anc_type = ANC_FEEDFORWARD|ANC_FEEDBACK;
            }

            anc_sample_rate = (data[5] | (data[6] << 8) | (data[7] << 16) | (data[8] << 24));
#endif
            send_reply((unsigned char *)&cret, 1);
            break;
        }
        case ANC_CMD_CHANNEL_SETUP:
        {
            const enum AUD_CHANNEL_MAP_T channel_map_arr[16] = {
                                                    AUD_CHANNEL_MAP_CH0,
                                                    AUD_CHANNEL_MAP_CH1,
                                                    AUD_CHANNEL_MAP_CH2,
                                                    AUD_CHANNEL_MAP_CH3,
                                                    AUD_CHANNEL_MAP_CH4,
                                                    AUD_CHANNEL_MAP_CH5,
                                                    AUD_CHANNEL_MAP_CH6,
                                                    AUD_CHANNEL_MAP_CH7,
                                                    AUD_CHANNEL_MAP_DIGMIC_CH0,
                                                    AUD_CHANNEL_MAP_DIGMIC_CH1,
                                                    AUD_CHANNEL_MAP_DIGMIC_CH2,
                                                    AUD_CHANNEL_MAP_DIGMIC_CH3,
                                                    AUD_CHANNEL_MAP_DIGMIC_CH4,
                                                    AUD_CHANNEL_MAP_DIGMIC_CH5,
                                                    AUD_CHANNEL_MAP_DIGMIC_CH6,
                                                    AUD_CHANNEL_MAP_DIGMIC_CH7,
                                                };
            anc_ff_mic_ch_l = channel_map_arr[data[0]];
            anc_ff_mic_ch_r = channel_map_arr[data[1]];
            anc_fb_mic_ch_l = channel_map_arr[data[2]];
            anc_fb_mic_ch_r = channel_map_arr[data[3]];
            TR_DEBUG(4,"anc_ff_mic_ch_l 0x%x,anc_ff_mic_ch_r 0x%x,anc_fb_mic_ch_l 0x%x,anc_fb_mic_ch_r 0x%x",anc_ff_mic_ch_l,
                                                                                                        anc_ff_mic_ch_r,
                                                                                                        anc_fb_mic_ch_l,
                                                                                                        anc_fb_mic_ch_r);
            cret = ERR_NONE;
            send_reply((unsigned char *)&cret, 1);
            break;
        }
        case ANC_CMD_SET_SAMP_RATE:
        {
#if 0
#ifndef CHIP_BEST1000
            bool opened;

            opened = (anc_opened(ANC_FEEDFORWARD) || anc_opened(ANC_FEEDBACK));
            if (opened) {
                anc_switch_byspp(false);
                prod_test_anc_close_internal();
            }
            anc_sample_rate = data[0] | (data[1] << 8) | (data[2] << 16) | (data[3] << 24);
            if (anc_sample_rate == AUD_SAMPRATE_50781) {
                hal_cmu_audio_resample_enable();
            } else {
                hal_cmu_audio_resample_disable();
            }
            if (opened) {
                prod_test_anc_open_internal();
                anc_switch_byspp(true);
            }
#endif
#endif
            cret = ERR_NONE;
            send_reply((unsigned char *)&cret, 1);
            break;
        }
    }

    return ERR_NONE;
}

static enum ERR_CODE anc_handle_data(struct CUST_CMD_PARAM *param)
{
    const struct message_t *msg = param->msg;
    uint32_t cmd;

    if (msg->hdr.type != TYPE_PROD_TEST) {
        return ERR_TYPE_INVALID;
    }

    cmd = msg->data[0] | (msg->data[1] << 8) | (msg->data[2] << 16) | (msg->data[3] << 24);
    if (cmd != PROD_TEST_CMD_ANC) {
        return ERR_TYPE_INVALID;
    }

    if (msg->hdr.len < 5) {
        //TR_DEBUG(1,"PROD_TEST/ANC msg length error: %d", msg->hdr.len);
        return ERR_LEN;
    }

    return anc_cmd_handler(msg->data[4], &msg->data[5], msg->hdr.len - 5);
}

static enum ERR_CODE anc_prod_test(struct CUST_CMD_PARAM *param)
{
    if (param->stage == CUST_CMD_STAGE_HEADER) {
        return anc_check_msg_hdr(param);
    } else if (param->stage == CUST_CMD_STAGE_DATA) {
        return anc_handle_data(param);
    } else {
        return ERR_TYPE_INVALID;
    }
}

static void anc_prod_test_open(void)
{
    #if 0
    //int ret;
    hal_cmu_pll_enable(HAL_CMU_PLL_AUD, HAL_CMU_PLL_USER_SYS);
    hal_cmu_sys_select_pll(HAL_CMU_PLL_AUD);

    hwtimer_init();
    hal_audma_open();
    ret = pmu_open();
    ASSERT(ret == 0, "Failed to open pmu");
    analog_open();
    af_open();
    #endif

    best2000_prod_test_anc_open();

}


//init
void anc_prod_test_init(void)
{
    return;
}

#if 1
void send_sync_cmd_to_tool()
{
    TR_DEBUG(1,"%s",__func__);

    send_msg.hdr.type = TYPE_SYNC;
    send_msg.hdr.seq = send_seq++;
    send_msg.hdr.len = 3;
    send_msg.data[0] = 0x00;
    send_msg.data[1] = COMMAND_PARSER_VERSION & 0xFF;
    send_msg.data[2] = (COMMAND_PARSER_VERSION >> 8) & 0xFF;
    send_msg.data[3] = ~check_sum((unsigned char *)&send_msg, MSG_TOTAL_LEN(&send_msg) - 1);

    send_data((unsigned char *)&send_msg, MSG_TOTAL_LEN(&send_msg));
}
#else
void send_sync_cmd_to_tool()
{
    TR_DEBUG(1,"%s",__func__);
    send_msg.hdr.type = TYPE_SYNC;
    send_msg.hdr.seq = 0x00;
    send_msg.hdr.len = 1;
    send_msg.data[0] = 0x00;
    send_msg.data[1] = ~check_sum((unsigned char *)&send_msg, MSG_TOTAL_LEN(&send_msg) - 1);
    send_msg.data[2] =0xef;
    send_data((unsigned char *)&send_msg, MSG_TOTAL_LEN(&send_msg));
}
#endif


extern int parse_packet(unsigned char **buf, size_t *len);

#if 1
unsigned char *g_buf = NULL;
size_t g_len = 0;

extern unsigned int extra_buf_len;

int anc_handle_received_data(uint8_t* ptrParam, uint32_t paramLen)
{
    int ret=0;
    static size_t rlen = 0;

    if (parse_state == PARSE_EXTRA)
    {
        //set_recv_timeout(recv_extra_timeout);

        ASSERT((rlen + paramLen) <= extra_buf_len, "Pending SPP rx buffer space is not enough !");

        // extera data
        memcpy((unsigned char *)(g_buf + rlen), ptrParam, paramLen);

        rlen += paramLen;
        TR_DEBUG(2,"Receiving data: expect=%u real=%u", g_len, rlen);

        if (rlen > g_len)
        {
            TR_DEBUG(0,"playload over !!");
            rlen = 0;
            ret=-1;
            return ret;
        }
        else if (rlen < g_len)
        {
            return ret;
        }

    }
    else
    {
        //if (programmer_state == PROGRAMMER_NONE && parse_state == PARSE_HEADER) {
        //    set_recv_timeout(default_recv_timeout_idle);
        //} else {
        //    set_recv_timeout(default_recv_timeout_short);
        //}

        ASSERT(g_buf == &recv_msg.hdr.prefix, "Pending SPP rx buffer address is wrong !");

        if (paramLen < sizeof(recv_msg))
        {
            memcpy(g_buf, ptrParam, paramLen);
            g_len = paramLen;
        }
        else
        {
            TR_DEBUG(0,"playload over !!");
            ret=-1;
            return ret;
        }

    }

    ret = parse_packet(&g_buf, &g_len);

    if (ret) {
        TR_DEBUG(0,"Parsing packet failed");
    }
    rlen = 0;

    return ret;
}
#else
int anc_handle_received_data(uint8_t* ptrParam, uint32_t paramLen)
{
    int ret=0;
    uint8_t *tempreceiveddata;
#if 1
    //unsigned char *buf = NULL;
    static size_t len = 0;
    //size_t buf_len, rlen;
    uint32_t receive_len;
    len=paramLen;
    tempreceiveddata=ptrParam;
    receive_len=sizeof(recv_msg.hdr)+sizeof(recv_msg.data);
    //tempreceiveddata=&recv_msg.hdr.prefix;

    if(paramLen<=receive_len)
    {
        if(tempreceiveddata[0]==PREFIX_CHAR)
        {
           memcpy(&recv_msg.hdr.prefix,ptrParam,paramLen);

           DUMP8("0x%02x ", &recv_msg.hdr.prefix,paramLen);
           ret=parse_packet(&tempreceiveddata, &len);
           if(ret)
           {
              TR_DEBUG(0,"Parsing packet failed");
              return ret;
           }
        }
    }
    else
    {
        TR_DEBUG(0,"playload over !!");
        ret=-1;
        return ret;
    }
#endif


#if 0
    //handl handshake
    if(tempreceiveddata[0]==PREFIX_CHAR)
    {
        if(tempreceiveddata[1]==TYPE_SYNC)
        {
            switch (tempreceiveddata[4])
            {
                case 1:
                    TR_DEBUG(0,"reply sync message of anc tool");
                    send_sync_confirm(tempreceiveddata[2]);
                    ret=1;
                break;
                case 2:
                    TR_DEBUG(0,"handshak ok !!");
                    ret=2;
                break;
                default:
                    TR_DEBUG(0,"invalid cmd !");
                    ret=-1;
                break;
            }
        }
        return ret;
    }
    anc_prod_test((struct CUST_CMD_PARAM *)ptrParam);

 #endif
    return ret;
}
#endif

static const CUST_CMD_HANDLER_T CUST_CMD_HDLR_TBL_LOC mod_hldr[] = {
    anc_prod_test,
};


