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
#include "stdint.h"
#include "stdbool.h"
#include "plat_types.h"
#include "string.h"
#include "stdio.h"
#include "crc32.h"
#include "hexdump.h"
#include "tool_msg.h"
#include "hal_norflash.h"
#include "hal_trace.h"
#include "hal_timer.h"
#include "anc_parse_data.h"
#include "hal_bootmode.h"
//#include "sys_api_programmer.h"
//#include "flash_programmer.h"
#include "hal_cmu.h"
#include "cmsis.h"
#include "pmu.h"
#include "heap_api.h"

//======================================================================================================

#define FLASH_PROGRAMMER_VERSION        0x0100

#define SLIDE_WIN_NUM                   2

#define SPP_BULK_READ_STEP_SIZE         600



static int send_burn_data_reply(enum ERR_CODE code, unsigned short sec_seq, unsigned char seq);
int free_data_buf(void);
int data_buf_in_use(void);
extern int send_reply(const unsigned char *payload, unsigned int len);
extern int send_data(const unsigned char *buf, size_t len);

typedef enum ERR_CODE (*CUST_CMD_HANDLER_T)(struct CUST_CMD_PARAM *param);


enum PROGRAMMER_STATE programmer_state;

enum PARSE_STATE parse_state;
struct message_t send_msg = { { PREFIX_CHAR, }, };
struct message_t recv_msg;
unsigned char send_seq = 0;


unsigned int extra_buf_len;
unsigned int recv_extra_timeout;

static unsigned int burn_addr;
static unsigned int burn_len;
static unsigned int sector_size;
static unsigned int sector_cnt;
static unsigned int last_sector_len;
static unsigned int cur_sector_seq;

/* databuf is a two-dimensional array, size: SLIDE_WIN_NUM*ANC_PARSE_DATA_BUFF_SIZE */
static uint8_t *data_buf[SLIDE_WIN_NUM];
static bool anc_data_buff_inited_flag = false;

static enum DATA_BUF_STATE data_buf_state[SLIDE_WIN_NUM];
static enum ERR_CODE data_buf_errcode[SLIDE_WIN_NUM];
static unsigned int cur_data_buf_idx;

static const unsigned int size_mask = SECTOR_SIZE_4K;

#if 1
#define default_recv_timeout_short      (MS_TO_TICKS(500))
#define default_recv_timeout_idle       (TIMEOUT_INFINITE)
#define default_recv_timeout_4k_data    (MS_TO_TICKS(500))
#define default_send_timeout            (MS_TO_TICKS(500))
#else
const unsigned int default_recv_timeout_short = MS_TO_TICKS(500);
const unsigned int default_recv_timeout_idle = TIMEOUT_INFINITE; //MS_TO_TICKS(10 * 60 * 1000);
const unsigned int default_recv_timeout_4k_data = MS_TO_TICKS(500);
const unsigned int default_send_timeout = MS_TO_TICKS(500);
#endif

void anc_data_buff_init(void)
{
    if (false == anc_data_buff_inited_flag)
    {
        syspool_init();
        for (int i = 0; i < SLIDE_WIN_NUM; i++)
        {
            syspool_get_buff(&data_buf[i], ANC_PARSE_DATA_BUFF_SIZE);
        }
        anc_data_buff_inited_flag = true;
    }
}
void anc_data_buff_deinit(void)
{
    anc_data_buff_inited_flag = false;
    memset(data_buf, 0, sizeof(data_buf));
}

int debug_read_enabled(void)
{
    return !!(hal_sw_bootmode_get() & HAL_SW_BOOTMODE_READ_ENABLED);
}

int debug_write_enabled(void)
{
    return !!(hal_sw_bootmode_get() & HAL_SW_BOOTMODE_WRITE_ENABLED);
}



int get_sector_info(unsigned int addr, unsigned int *sector_addr, unsigned int *sector_len)
{
    int ret;

    ret = hal_norflash_get_boundary(HAL_FLASH_ID_0, addr, NULL, (uint32_t *)sector_addr);
    if (ret) {
        return ret;
    }

    ret = hal_norflash_get_size(HAL_FLASH_ID_0, NULL, NULL, (uint32_t *)sector_len, NULL);

    return ret;
}

int erase_sector(unsigned int sector_addr, unsigned int sector_len)
{
    uint32_t lock;
    uint8_t ret;

    lock = int_lock_global();
    pmu_flash_write_config();

    ret = hal_norflash_erase(HAL_FLASH_ID_0, sector_addr, sector_len);

    pmu_flash_read_config();
    int_unlock_global(lock);

    return ret;
}

#if 0
int erase_chip(void)
{
    return hal_norflash_erase_chip(HAL_FLASH_ID_0);
}
#endif

int burn_data(unsigned int addr, const unsigned char *data, unsigned int len)
{
    uint32_t lock;
    uint8_t ret;

    lock = int_lock_global();
    pmu_flash_write_config();

    ret = hal_norflash_write(HAL_FLASH_ID_0, addr, data, len);

    pmu_flash_read_config();
    int_unlock_global(lock);

    return ret;
}

int verify_flash_data(unsigned int addr, const unsigned char *data, unsigned int len)
{
    const unsigned char *fdata;
    const unsigned char *mdata;
    int i;

    fdata = (unsigned char *)addr;
    mdata = data;
    for (i = 0; i < len; i++) {
        if (*fdata++ != *mdata++) {
            --fdata;
            --mdata;
            TR_DEBUG(4,"****** Verify flash data failed: 0x%02X @ %p != 0x%02X @ %p", *fdata, fdata, *mdata, mdata);
            return *fdata - *mdata;
        }
    }

    return 0;
}

int wait_data_buf_finished(void)
{
    while (free_data_buf()) {
        //task_yield();
        hal_sys_timer_delay(10);
    }

    return 0;
}

int wait_all_data_buf_finished(void)
{
    while (data_buf_in_use()) {
        while (free_data_buf() == 0);
        //task_yield();
        hal_sys_timer_delay(10);
    }

    return 0;
}

static unsigned int count_set_bits(unsigned int i)
{
    i = i - ((i >> 1) & 0x55555555);
    i = (i & 0x33333333) + ((i >> 2) & 0x33333333);
    return (((i + (i >> 4)) & 0x0F0F0F0F) * 0x01010101) >> 24;
}

static void reset_data_buf_state(void)
{
    unsigned int i;

    cur_data_buf_idx = 0;
    data_buf_state[0] = DATA_BUF_RECV;
    for (i = 1; i < SLIDE_WIN_NUM; i++) {
        data_buf_state[i] = DATA_BUF_FREE;
    }
}

static int get_free_data_buf(void)
{
    unsigned int i;
    unsigned int index;

    while (1) {
        index = cur_data_buf_idx + 1;
        if (index >= SLIDE_WIN_NUM) {
            index = 0;
        }
        for (i = 0; i < SLIDE_WIN_NUM; i++) {
            if (data_buf_state[index] == DATA_BUF_FREE) {
                data_buf_state[index] = DATA_BUF_RECV;
                cur_data_buf_idx = index;
                return 0;
            }

            if (++index >= SLIDE_WIN_NUM) {
                index = 0;
            }
        }

        if (wait_data_buf_finished()) {
            break;
        }
    }

    return 1;
}

#ifndef _WIN32
int get_burn_data_buf(unsigned int *rindex)
{
    unsigned int i;
    unsigned int index;

    index = cur_data_buf_idx + 1;
    if (index >= SLIDE_WIN_NUM) {
        index = 0;
    }
    for (i = 0; i < SLIDE_WIN_NUM; i++) {
        if (data_buf_state[index] == DATA_BUF_BURN) {
            *rindex = index;
            return 0;
        }

        if (++index >= SLIDE_WIN_NUM) {
            index = 0;
        }
    }

    return 1;
}

static void finish_data_buf(unsigned int index, enum ERR_CODE code)
{
    data_buf_errcode[index] = code;
    data_buf_state[index] = DATA_BUF_DONE;
}

static enum ERR_CODE handle_write_cmd(unsigned int addr, unsigned int len, unsigned char *wdata);

int handle_data_buf(unsigned int index)
{
    unsigned int addr;
    unsigned int len;
    unsigned int sec_seq;
    //unsigned int mcrc;
    int ret = 0;

    if (index >= SLIDE_WIN_NUM) {
        return 1;
    }

    TR_DEBUG(2,"### FLASH_TASK: %s index=%u ###", __FUNCTION__, index);

    len = data_buf[index][4] | (data_buf[index][5] << 8) |
        (data_buf[index][6] << 16) | (data_buf[index][7] << 24);
    //mcrc = data_buf[index][8] | (data_buf[index][9] << 8) |
    //    (data_buf[index][10] << 16) | (data_buf[index][11] << 24);
    sec_seq = data_buf[index][12] | (data_buf[index][13] << 8);
    addr = burn_addr + sec_seq * sector_size;

    TR_DEBUG(3,"### FLASH_TASK: sec_seq=%u addr=0x%08X len=%u ###", sec_seq, addr, len);

    if (programmer_state == PROGRAMMER_ERASE_BURN_START) {
        ret = erase_sector(addr, len);
        if (ret) {
            TR_DEBUG(2,"### FLASH_TASK: ERASE_DATA failed: addr=0x%08X len=%u ###", addr, len);
            finish_data_buf(index, ERR_ERASE_FLSH);
            return 0;
        }
        TR_DEBUG(0,"### FLASH_TASK: Erase done ###");
    }

    if (programmer_state == PROGRAMMER_BULK_WRITE_START) {
        ret = handle_write_cmd(addr, len, &data_buf[index][BURN_DATA_MSG_OVERHEAD]);
    }
    else {
        ret = burn_data(addr, &data_buf[index][BURN_DATA_MSG_OVERHEAD], len);
    }

    if (ret) {
        TR_DEBUG(2,"### FLASH_TASK: BURN_DATA failed: addr=0x%08X len=%u ###", addr, len);
        finish_data_buf(index, ERR_BURN_FLSH);
        return 0;
    }
    TR_DEBUG(0,"### FLASH_TASK: Burn done ###");

    ret = verify_flash_data(addr, &data_buf[index][BURN_DATA_MSG_OVERHEAD], len);
    if (ret) {
        TR_DEBUG(0,"### FLASH_TASK: VERIFY_DATA failed");
        finish_data_buf(index, ERR_VERIFY_FLSH);
        return 0;
    }
    TR_DEBUG(0,"### FLASH_TASK: Verify done ###");

    finish_data_buf(index, ERR_BURN_OK);

    return 0;
}

int free_data_buf(void)
{
    unsigned int i;
    unsigned int index;
    unsigned int sec_seq;

    index = cur_data_buf_idx + 1;
    if (index >= SLIDE_WIN_NUM) {
        index = 0;
    }

    // Free one data buffer once
    for (i = 0; i < SLIDE_WIN_NUM; i++) {
        if (data_buf_state[index] == DATA_BUF_DONE) {
            sec_seq = data_buf[index][12] | (data_buf[index][13] << 8);
            send_burn_data_reply(data_buf_errcode[index], sec_seq, data_buf[index][2]);
            data_buf_state[index] = DATA_BUF_FREE;
            return 0;
        }

        if (++index >= SLIDE_WIN_NUM) {
            index = 0;
        }
    }

    return 1;
}

int data_buf_in_use(void)
{
    unsigned int index;

    for (index = 0; index < SLIDE_WIN_NUM; index++) {
        if (data_buf_state[index] != DATA_BUF_FREE) {
            return 1;
        }
    }

    return 0;
}
#endif


static void burn_data_buf(int index)
{
    data_buf_state[index] = DATA_BUF_BURN;

#ifdef _WIN32
    write_flash_data(&data_buf[index][BURN_DATA_MSG_OVERHEAD],
        data_buf[index][4] | (data_buf[index][5] << 8) |
        (data_buf[index][6] << 16) | (data_buf[index][7] << 24));

    send_burn_data_reply(ERR_BURN_OK,
        data_buf[index][12] | (data_buf[index][13] << 8),
        data_buf[index][2]);
    data_buf_state[index] = DATA_BUF_FREE;
#else

    handle_data_buf(index);

#endif
}


static unsigned char check_sum(unsigned char *buf, unsigned char len)
{
    int i;
    unsigned char sum = 0;

    for (i = 0; i < len; i++) {
        sum += buf[i];
    }

    return sum;
}

#if 0
static int send_sector_size_msg(void)
{
    int ret;

    send_msg.hdr.type = TYPE_SECTOR_SIZE;
    send_msg.hdr.seq = send_seq++;
    send_msg.hdr.len = 6;
    send_msg.data[0] = FLASH_PROGRAMMER_VERSION & 0xFF;
    send_msg.data[1] = (FLASH_PROGRAMMER_VERSION >> 8) & 0xFF;
    send_msg.data[2] = size_mask & 0xFF;
    send_msg.data[3] = (size_mask >> 8) & 0xFF;
    send_msg.data[4] = (size_mask >> 16) & 0xFF;
    send_msg.data[5] = (size_mask >> 24) & 0xFF;
    send_msg.data[6] = ~check_sum((unsigned char *)&send_msg, MSG_TOTAL_LEN(&send_msg) - 1);

    ret = send_data((unsigned char *)&send_msg, MSG_TOTAL_LEN(&send_msg));

    return ret;
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
#endif

static int send_burn_data_reply(enum ERR_CODE code, unsigned short sec_seq, unsigned char seq)
{
    int ret = 0;

    if (programmer_state == PROGRAMMER_BULK_WRITE_START) {
        send_msg.hdr.type = TYPE_BULK_WRITE_DATA;
    } else if (programmer_state == PROGRAMMER_ERASE_BURN_START) {
        send_msg.hdr.type = TYPE_ERASE_BURN_DATA;
    } else {
        return 1;
    }

    send_msg.hdr.seq = seq;
    send_msg.hdr.len = 3;
    send_msg.data[0] = code;
    send_msg.data[1] = sec_seq & 0xFF;
    send_msg.data[2] = (sec_seq >> 8) & 0xFF;
    send_msg.data[3] = ~check_sum((unsigned char *)&send_msg, MSG_TOTAL_LEN(&send_msg) - 1);

    ret = send_data((unsigned char *)&send_msg, MSG_TOTAL_LEN(&send_msg));

    return ret;
}

static void reset_parse_state(unsigned char **buf, size_t *len)
{
    parse_state = PARSE_HEADER;
    memset(&recv_msg.hdr, 0, sizeof(recv_msg.hdr));

    *buf = (unsigned char *)&recv_msg.hdr;
    *len = sizeof(recv_msg.hdr);
}

void reset_programmer_state(unsigned char **buf, size_t *len)
{
    programmer_state = PROGRAMMER_NONE;
    reset_parse_state(buf, len);
    reset_data_buf_state();
}


enum ERR_CODE handle_cust_cmd(struct CUST_CMD_PARAM *param)
{
    enum ERR_CODE errcode = ERR_TYPE_INVALID;

    extern uint32_t __cust_cmd_hldr_tbl_start[];
    extern uint32_t __cust_cmd_hldr_tbl_end[];

    CUST_CMD_HANDLER_T *start = (CUST_CMD_HANDLER_T *)__cust_cmd_hldr_tbl_start;
    CUST_CMD_HANDLER_T *end = (CUST_CMD_HANDLER_T *)__cust_cmd_hldr_tbl_end;

    while (errcode == ERR_TYPE_INVALID && start < end) {
        errcode = (*start)(param);
        start++;
    }

    return errcode;
}


static enum ERR_CODE check_msg_hdr(void)
{
    enum ERR_CODE errcode = ERR_NONE;
    struct CUST_CMD_PARAM param;
    TR_DEBUG(0,"check_msg_hdr");

    switch (recv_msg.hdr.type) {
        case TYPE_SYS:
            if (recv_msg.hdr.len != 1 && recv_msg.hdr.len != 5) {
                //TR_DEBUG(1,"SYS msg length error: %u", recv_msg.hdr.len);
                errcode = ERR_LEN;
            }
            break;
        case TYPE_SYNC:
            TR_DEBUG(0,"get TYPE_SYNC");
            if (recv_msg.hdr.len != 1 && recv_msg.hdr.len != 5) {
                //TR_DEBUG(1,"SYS msg length error: %u", recv_msg.hdr.len);
                errcode = ERR_LEN;
            }
            break;
        case TYPE_READ:
            if (recv_msg.hdr.len != 5) {
                //TR_DEBUG(1,"READ msg length error: %u", recv_msg.hdr.len);
                errcode = ERR_LEN;
            }
            break;
        case TYPE_WRITE:
        case TYPE_EXT_WRITE:
            //if (recv_msg.hdr.len <= 4 || recv_msg.hdr.len > 20) {
            if (recv_msg.hdr.len <= 4) {
                //TR_DEBUG(1,"WRITE msg length error: %u", recv_msg.hdr.len);
                errcode = ERR_LEN;
            }
            break;
        case TYPE_BULK_READ:
            if (recv_msg.hdr.len != 8) {
                //TR_DEBUG(1,"BULK_READ msg length error: %u", recv_msg.hdr.len);
                errcode = ERR_LEN;
            }
            break;
        case TYPE_BULK_WRITE_START:
        case TYPE_ERASE_BURN_START:
            if (recv_msg.hdr.len != 12) {
                //TR_DEBUG(2,"BURN_START 0x%x msg length error: %u", recv_msg.hdr.type, recv_msg.hdr.len);
                errcode = ERR_LEN;
            }
            break;
        case TYPE_BULK_WRITE_DATA:
        case TYPE_ERASE_BURN_DATA:
            // BURN_DATA msgs are sent in extra msgs
            errcode = ERR_BURN_INFO_MISSING;
            break;
        case TYPE_FLASH_CMD:
            if (recv_msg.hdr.len != 1 && (recv_msg.hdr.len <= 5 || recv_msg.hdr.len > 21)) {
                //TR_DEBUG(1,"TYPE_FLASH_CMD msg length error: %u", recv_msg.hdr.len);
                errcode = ERR_LEN;
            }
            break;
        case TYPE_GET_SECTOR_INFO:
            if (recv_msg.hdr.len != 4) {
                //TR_DEBUG(1,"GET_SECTOR_INFO msg length error: %u", recv_msg.hdr.len);
                errcode = ERR_LEN;
            }
            break;
        default:
            param.stage = CUST_CMD_STAGE_HEADER;
            param.msg = &recv_msg;
            errcode = handle_cust_cmd(&param);
            if (errcode == ERR_TYPE_INVALID) {
                TR_DEBUG(1,"Invalid message type: 0x%02x", recv_msg.hdr.type);
            }
            break;
    }

    if (errcode == ERR_NONE && recv_msg.hdr.len + 1 > sizeof(recv_msg.data)) {
        errcode = ERR_LEN;
    }

    return errcode;
}

extern int system_shutdown(void);

static enum ERR_CODE handle_sys_cmd(enum SYS_CMD_TYPE cmd, unsigned char *param, unsigned int len)
{
    unsigned char cret[5];
    //unsigned int bootmode;

    cret[0] = ERR_NONE;

    if (cmd == SYS_CMD_SET_BOOTMODE || cmd == SYS_CMD_CLR_BOOTMODE) {
        if (len != 4) {
            TR_DEBUG(2,"Invalid SYS CMD len %u for cmd: 0x%x", len, cmd);
            return ERR_DATA_LEN;
        }
    } else {
        if (len != 0) {
            TR_DEBUG(2,"Invalid SYS CMD len %u for cmd: 0x%x", len, cmd);
            return ERR_DATA_LEN;
        }
    }

    switch (cmd) {
        case SYS_CMD_REBOOT: {
            TR_DEBUG(0,"--- Reboot---");
            send_reply(cret, 1);
            //system_reboot();
            break;
        }
        case SYS_CMD_SHUTDOWN: {
            TR_DEBUG(0,"--- Shutdown ---");
            send_reply(cret, 1);
            system_shutdown();
            break;
        }
        case SYS_CMD_FLASH_BOOT: {
            TR_DEBUG(0,"--- Flash boot ---");
            send_reply(cret, 1);
            //system_flash_boot();
            break;
        }
        case SYS_CMD_SET_BOOTMODE: {
            TR_DEBUG(0,"--- Set bootmode ---");
            //memcpy(&bootmode, param, 4);
            //system_set_bootmode(bootmode);
            send_reply(cret, 1);
            break;
        }
        case SYS_CMD_CLR_BOOTMODE: {
            TR_DEBUG(0,"--- Clear bootmode ---");
           // memcpy(&bootmode, param, 4);
            //system_clear_bootmode(bootmode);
            send_reply(cret, 1);
            break;
        }
        case SYS_CMD_GET_BOOTMODE: {
            TR_DEBUG(0,"--- Get bootmode ---");
           // bootmode = system_get_bootmode();
            //memcpy(&cret[1], &bootmode, 4);
            send_reply(cret, 5);
            break;
        }
        default: {
            TR_DEBUG(1,"Invalid command: 0x%x", recv_msg.data[0]);
            return ERR_SYS_CMD;
        }
    }

    return ERR_NONE;
}

static enum ERR_CODE handle_read_cmd(unsigned int addr, unsigned int len)
{
    union {
            unsigned int data[4];
            unsigned char buf[19];
        } d;
    int i;
#ifndef _WIN32
    int cnt;
    unsigned short *p;
#endif

    TR_DEBUG(2,"[READ] addr=0x%08X len=%u", addr, len);

    if (debug_read_enabled() == 0) {
        TR_DEBUG(0,"[READ] No access right");
        return ERR_ACCESS_RIGHT;
    }
    if (len > 16) {
        TR_DEBUG(1,"[READ] Length error: %u", len);
        return ERR_DATA_LEN;
    }

#ifdef _WIN32
    for (i = 0; i < 4; i++) {
        d.data[i] = (0x11 + i) | ((0x22 + i) << 8) | ((0x33 + i) << 16) | ((0x44 + i ) << 24);
    }
#else
    // Handle half-word and word register reading
    if ((len & 0x03) == 0 && (addr & 0x03) == 0) {
        cnt = len / 4;
        for (i = 0; i < cnt; i++) {
            d.data[i] = *((unsigned int *)addr + i);
        }
    } else if ((len & 0x01) == 0 && (addr & 0x01) == 0) {
        cnt = len / 2;
        p = (unsigned short *)&d.data[0];
        for (i = 0; i < cnt; i++) {
            p[i] = *((unsigned short *)addr + i);
        }
    } else {
        memcpy(&d.data[0], (unsigned char *)addr, len);
    }
#endif

    memmove(&d.buf[1], &d.buf[0], len);
    d.buf[0] = ERR_NONE;
    send_reply((unsigned char *)&d.buf[0], 1 + len);

    return ERR_NONE;
}

static enum ERR_CODE handle_write_cmd(unsigned int addr, unsigned int len, unsigned char *wdata)
{
    unsigned int data;
#ifdef _WIN32
    unsigned char d[16];
#else
    int i;
    int cnt;
#endif

    TR_DEBUG(2,"[WRITE] addr=0x%08X len=%u", addr, len);

    if (debug_write_enabled() == 0) {
        TR_DEBUG(0,"[WRITE] No access right");
        return ERR_ACCESS_RIGHT;
    }

    if (recv_msg.hdr.type == TYPE_WRITE && len > 16) {
        TR_DEBUG(1,"[WRITE] Length error: %u", len);
        return ERR_DATA_LEN;
    }

#ifdef _WIN32
    memcpy(d, wdata, len);
    dump_buffer(d, len);
#else
    // Handle half-word and word register writing
    if ((len & 0x03) == 0 && (addr & 0x03) == 0) {
        cnt = len / 4;
        for (i = 0; i < cnt; i++) {
            data = wdata[4 * i] | (wdata[4 * i + 1] << 8) | (wdata[4 * i + 2] << 16) | (wdata[4 * i + 3] << 24);
            *((unsigned int *)addr + i) = data;
        }
    } else if ((len & 0x01) == 0 && (addr & 0x01) == 0) {
        cnt = len / 2;
        for (i = 0; i < cnt; i++) {
            data = wdata[2 * i] | (wdata[2 * i + 1] << 8);
            memcpy(&data, wdata + 2 * i, 2);
            *((unsigned short *)addr + i) = (unsigned short)data;
        }
    } else {
        memcpy((unsigned char *)addr, wdata, len);
    }
#endif

    if (recv_msg.hdr.type == TYPE_WRITE || recv_msg.hdr.type == TYPE_EXT_WRITE) {
        data = ERR_NONE;
        send_reply((unsigned char *)&data, 1);
    }

    return ERR_NONE;
}


#if 0
static enum ERR_CODE handle_bulk_read_cmd(unsigned int addr, unsigned int len)
{
    int ret;
    unsigned int sent;
    unsigned char cret[1];

    TR_DEBUG(2,"[BULK_READ] addr=0x%08X len=%u", addr, len);

    if (debug_read_enabled() == 0) {
        TR_DEBUG(0,"[READ] No access right");
        return ERR_ACCESS_RIGHT;
    }

    cret[0] = ERR_NONE;
    send_reply(cret, 1);

    ret = 0;
#ifdef _WIN32
    while (ret == 0 && len > 0) {
        sent = (len > sizeof(data_buf[0])) ? sizeof(data_buf[0]) : len;
        ret = send_data(&data_buf[0][0], sent);
        len -= sent;
    }
#else
    while (ret == 0 && len > 0) {
        sent = (len > BULK_READ_STEP_SIZE) ? BULK_READ_STEP_SIZE : len;
        ret = send_data((unsigned char *)addr, sent);
        addr += sent;
        len -= sent;
    }
#endif

    if (ret) {
        TR_DEBUG(1,"[BULK_READ] Failed to send data: %d", ret);
        // Just let the peer timeout
    }

    return ERR_NONE;
}
#else
static unsigned int bulk_addr;
static unsigned int bulk_len;

static enum ERR_CODE handle_bulk_read_cmd(unsigned int addr, unsigned int len)
{
    int ret;
    unsigned int sent;
    unsigned char cret[1];

    TR_DEBUG(2,"[BULK_READ] addr=0x%08X len=%u", addr, len);

    if (debug_read_enabled() == 0) {
        TR_DEBUG(0,"[READ] No access right");
        return ERR_ACCESS_RIGHT;
    }

    cret[0] = ERR_NONE;
    send_reply(cret, 1);

    ret = 0;
#ifdef _WIN32
    while (ret == 0 && len > 0) {
        sent = (len > sizeof(data_buf[0])) ? sizeof(data_buf[0]) : len;
        ret = send_data(&data_buf[0][0], sent);
        len -= sent;
    }
#else
	if (ret == 0 && len > 0)
	{
		bulk_addr = addr;
		bulk_len = len;
		sent = (bulk_len > SPP_BULK_READ_STEP_SIZE) ? SPP_BULK_READ_STEP_SIZE : bulk_len;

		ret = send_data((unsigned char *)bulk_addr, sent);
		TR_DEBUG(2,"[send data] addr=0x%08X sent=%u", bulk_addr, sent);

		bulk_len -=  sent;
		bulk_addr += sent;
	}
#endif

    if (ret) {
        TR_DEBUG(1,"[BULK_READ] Failed to send data: %d", ret);
        // Just let the peer timeout
    }

    return ERR_NONE;
}


void bulk_read_done(void)
{
    unsigned int sent;

	if (bulk_len > 0)
	{
	    sent = (bulk_len > SPP_BULK_READ_STEP_SIZE) ? SPP_BULK_READ_STEP_SIZE : bulk_len;
		send_data((unsigned char *)bulk_addr, sent);
		TR_DEBUG(2,"[send data] addr=0x%08X sent=%u", bulk_addr, sent);

		bulk_len -=  sent;
		bulk_addr += sent;
	}
}
#endif

static enum ERR_CODE handle_flash_cmd(enum FLASH_CMD_TYPE cmd, unsigned char *param, unsigned int len)
{
    int ret = 0;
    unsigned char cret = ERR_NONE;

    switch (cmd) {
        case FLASH_CMD_ERASE_SECTOR: {
            unsigned int addr;
            unsigned int size;

            TR_DEBUG(0,"- FLASH_CMD_ERASE_SECTOR -");

            if (len != 8) {
                TR_DEBUG(1,"Invalid ERASE_SECTOR cmd param len: %u", len);
                return ERR_LEN;
            }

            addr = param[0] | (param[1] << 8) | (param[2] << 16) | (param[3] << 24);
            size = param[4] | (param[5] << 8) | (param[6] << 16) | (param[7] << 24);
            TR_DEBUG(2,"addr=0x%08X size=%u", addr, size);

            ret = erase_sector(addr, size);
            if (ret) {
                TR_DEBUG(2,"ERASE_SECTOR failed: addr=0x%08X size=%u", addr, size);
                return ERR_ERASE_FLSH;
            }
            send_reply(&cret, 1);
            break;
        }
        case FLASH_CMD_BURN_DATA: {
            unsigned int addr;

            TR_DEBUG(0,"- FLASH_CMD_BURN_DATA -");

            if (len <= 4 || len > 20) {
                TR_DEBUG(1,"Invalid BURN_DATA cmd param len: %u", len);
                return ERR_LEN;
            }

            addr = param[0] | (param[1] << 8) | (param[2] << 16) | (param[3] << 24);
            TR_DEBUG(2,"addr=0x%08X len=%u", addr, len - 4);

            ret = burn_data(addr, &param[4], len - 4);
            if (ret) {
                TR_DEBUG(0,"BURN_DATA failed");
                return ERR_BURN_FLSH;
            }

            ret = verify_flash_data(addr, &param[4], len - 4);
            if (ret) {
                TR_DEBUG(0,"BURN_DATA verify failed");
                return ERR_VERIFY_FLSH;
            }
            send_reply(&cret, 1);
            break;
        }
        case FLASH_CMD_ERASE_CHIP: {
            TR_DEBUG(0,"- FLASH_CMD_ERASE_CHIP -");

            if (len != 0) {
                TR_DEBUG(1,"Invalid ERASE_CHIP cmd param len: %u", len);
                return ERR_LEN;
            }
            //ret = erase_chip();
            if (ret) {
                TR_DEBUG(0,"ERASE_CHIP failed");
                return ERR_ERASE_FLSH;
            }
            send_reply(&cret, 1);
            break;
        }
        default:
            TR_DEBUG(1,"Unsupported flash cmd: 0x%x", cmd);
            return ERR_FLASH_CMD;
    }

    return ERR_NONE;
}

static enum ERR_CODE handle_sector_info_cmd(unsigned int addr)
{
    unsigned int sector_addr;
    unsigned int sector_len;
    unsigned char buf[9];
    int ret;

    ret = get_sector_info(addr, &sector_addr, &sector_len);
    if (ret) {
        return ERR_DATA_ADDR;
    }

    TR_DEBUG(3,"addr=0x%08X sector_addr=0x%08X sector_len=%u", addr, sector_addr, sector_len);

    buf[0] = ERR_NONE;
    memcpy(&buf[1], &sector_addr, 4);
    memcpy(&buf[5], &sector_len, 4);

    send_reply(buf, 9);

    return ERR_NONE;
}


static int send_sync_confirm(void)
{
    int ret;
    send_msg.hdr.type = TYPE_SYNC;
    send_msg.hdr.seq = recv_msg.hdr.seq;
    send_msg.hdr.len = 3;
    send_msg.data[0] = 0x02;
    send_msg.data[1] = COMMAND_PARSER_VERSION & 0xFF;
    send_msg.data[2] = (COMMAND_PARSER_VERSION >> 8) & 0xFF;
    send_msg.data[3] = ~check_sum((unsigned char *)&send_msg, MSG_TOTAL_LEN(&send_msg) - 1);
    ret = send_data((unsigned char *)&send_msg, MSG_TOTAL_LEN(&send_msg));
    return ret;
}


int send_sync_flag=0;
int get_send_sync_flag(void)
{
    return send_sync_flag;
}
static enum ERR_CODE handle_data(unsigned char **buf, size_t *len, int *extra)
{
    unsigned char cret = ERR_NONE;
    enum ERR_CODE errcode;
    struct CUST_CMD_PARAM param;

    *extra = 0;

    // Checksum
    uint8_t chksum = check_sum((unsigned char *)&recv_msg, MSG_TOTAL_LEN(&recv_msg));
    if ( chksum != 0xFF) {
        TR_DEBUG(1,"Checksum error: %d", chksum);
        return ERR_CHECKSUM;
    }

    switch (recv_msg.hdr.type) {
        case TYPE_SYS: {
            TR_DEBUG(0,"------ SYS CMD ------");
            errcode = handle_sys_cmd((enum SYS_CMD_TYPE)recv_msg.data[0], &recv_msg.data[1], recv_msg.hdr.len - 1);
            if (errcode != ERR_NONE) {
                return errcode;
            }
            break;
        }
        case TYPE_SYNC: {
            TR_DEBUG(0,"------ SYNC CMD ------");
            switch (recv_msg.data[0])
            {
                case 1:
                    TR_DEBUG(0,"reply sync message of anc tool");
                    send_sync_flag=1;
                    send_sync_confirm();
                    errcode = ERR_NONE;
                    //ret=1;
                break;
                case 2:
                    send_sync_flag=0;
                    TR_DEBUG(0,"handshak ok !!");
                    errcode = ERR_NONE;
                    //ret=2;
                break;
                default:
                    TR_DEBUG(0,"invalid cmd !");
                    // ret=-1;
                    errcode=ERR_TYPE_INVALID;
                break;
           }
            if (errcode != ERR_NONE) {
                return errcode;
            }
            break;
        }
        case TYPE_READ: {
            unsigned int addr;
            unsigned int len;

            TR_DEBUG(0,"------ READ CMD ------");

            addr = recv_msg.data[0] | (recv_msg.data[1] << 8) | (recv_msg.data[2] << 16) | (recv_msg.data[3] << 24);
            len = recv_msg.data[4];

            errcode = handle_read_cmd(addr, len);
            if (errcode != ERR_NONE) {
                return errcode;
            }
            break;
        }
        case TYPE_WRITE:
        case TYPE_EXT_WRITE:
        {
            unsigned int addr;
            unsigned int len;
            unsigned char *wdata;

            if (recv_msg.hdr.type == TYPE_EXT_WRITE)
            {
                TR_DEBUG(0,"------ EXT WRITE CMD ------");
            }
            else{
                TR_DEBUG(0,"------ WRITE CMD ------");
            }

            addr = recv_msg.data[0] | (recv_msg.data[1] << 8) | (recv_msg.data[2] << 16) | (recv_msg.data[3] << 24);
            len = recv_msg.hdr.len - 4;
            wdata = &recv_msg.data[4];

            errcode = handle_write_cmd(addr, len, wdata);
            if (errcode != ERR_NONE) {
                return errcode;
            }
            break;
        }
        case TYPE_BULK_READ: {
            unsigned int addr;
            unsigned int len;

            TR_DEBUG(0,"------ BULK READ CMD ------");

            addr = recv_msg.data[0] | (recv_msg.data[1] << 8) | (recv_msg.data[2] << 16) | (recv_msg.data[3] << 24);
            len = recv_msg.data[4] | (recv_msg.data[5] << 8) | (recv_msg.data[6] << 16) | (recv_msg.data[7] << 24);

            errcode = handle_bulk_read_cmd(addr, len);
            if (errcode != ERR_NONE) {
                return errcode;
            }
            break;
        }
        case TYPE_BULK_WRITE_START:
        case TYPE_ERASE_BURN_START: {
            if (recv_msg.hdr.type == TYPE_BULK_WRITE_START) {
                TR_DEBUG(0,"------ BULK_WRITE_START ------");
            }
            else if (recv_msg.hdr.type == TYPE_ERASE_BURN_START) {
                TR_DEBUG(0,"------ ERASE_BURN_START ------");
            } else {
                TR_DEBUG(0,"------ BURN_START ------");
            }

            burn_addr = recv_msg.data[0] | (recv_msg.data[1] << 8) | (recv_msg.data[2] << 16) | (recv_msg.data[3] << 24);
            burn_len = recv_msg.data[4] | (recv_msg.data[5] << 8) | (recv_msg.data[6] << 16) | (recv_msg.data[7] << 24);
            sector_size = recv_msg.data[8] | (recv_msg.data[9] << 8) | (recv_msg.data[10] << 16) | (recv_msg.data[11] << 24);

            TR_DEBUG(3,"burn_addr=0x%08X burn_len=%u sector_size=%u", burn_addr, burn_len, sector_size);

            if ((size_mask & sector_size) == 0 || count_set_bits(sector_size) != 1) {
                TR_DEBUG(2,"Unsupported sector_size=0x%08X mask=0x%08X", sector_size, size_mask);
                return ERR_SECTOR_SIZE;
            }

            sector_cnt = burn_len / sector_size;
            last_sector_len = burn_len % sector_size;
            if (last_sector_len) {
                sector_cnt++;
            } else {
                last_sector_len = sector_size;
            }

            if (sector_cnt > 0xFFFF) {
                TR_DEBUG(1,"Sector seq overflow: %u", sector_cnt);
                return ERR_SECTOR_SEQ_OVERFLOW;
            }

            send_reply(&cret, 1);

            if (burn_len == 0) {
                TR_DEBUG(0,"Burn length = 0");
                break;
            }

            if (recv_msg.hdr.type == TYPE_BULK_WRITE_START) {
                programmer_state = PROGRAMMER_BULK_WRITE_START;
            } else {
                programmer_state = PROGRAMMER_ERASE_BURN_START;
            }

            reset_data_buf_state();

            *extra = 1;
            *buf = &data_buf[0][0];
            *len = BURN_DATA_MSG_OVERHEAD + ((sector_cnt == 1) ? last_sector_len : sector_size);
            //extra_buf_len = sizeof(data_buf[0]);
            extra_buf_len = ANC_PARSE_DATA_BUFF_SIZE;
            recv_extra_timeout = default_recv_timeout_short + (*len + 4096 - 1) / 4096 * default_recv_timeout_4k_data;;
            break;
        }
        case TYPE_FLASH_CMD: {

            TR_DEBUG(0,"------ FLASH CMD ------");

            errcode = handle_flash_cmd((enum FLASH_CMD_TYPE)recv_msg.data[0], &recv_msg.data[1], recv_msg.hdr.len - 1);

            TR_DEBUG(1,"BURN CMD ret=%d", errcode);

            if (errcode != ERR_NONE) {
                return errcode;
            }
            break;
        }
        case TYPE_GET_SECTOR_INFO: {
            unsigned int addr;

            TR_DEBUG(0,"------ GET SECTOR INFO ------");

            addr = recv_msg.data[0] | (recv_msg.data[1] << 8) | (recv_msg.data[2] << 16) | (recv_msg.data[3] << 24);

            errcode = handle_sector_info_cmd(addr);
            if (errcode != ERR_NONE) {
                return errcode;
            }
            break;
        }
        default:
            param.stage = CUST_CMD_STAGE_DATA;
            param.msg = &recv_msg;
            param.extra = 0;
            errcode = handle_cust_cmd(&param);
            if (errcode == ERR_NONE && param.extra) {
                *extra = 1;
                *buf = param.buf;
                *len = param.expect;
                extra_buf_len = param.size;
                recv_extra_timeout = param.timeout;
            }
            return errcode;
    }

    return ERR_NONE;
}

static enum ERR_CODE check_extra_burn_data_msg(unsigned char *data)
{
    struct message_t *msg = (struct message_t *)data;
    enum ERR_CODE errcode = ERR_NONE;
    enum MSG_TYPE type;
    const char *str_burn_data;

    if (programmer_state == PROGRAMMER_BULK_WRITE_START) {
        type = TYPE_BULK_WRITE_DATA;
        str_burn_data = "BULK_WRITE_DATA";
    } else if (programmer_state == PROGRAMMER_ERASE_BURN_START) {
        type = TYPE_ERASE_BURN_DATA;
        str_burn_data = "ERASE_BURN_DATA";
    } else {
        return ERR_INTERNAL;
    }

    if (msg->hdr.prefix != PREFIX_CHAR) {
        TR_DEBUG(1,"Invalid prefix char: 0x%x", msg->hdr.prefix);
        errcode = ERR_SYNC_WORD;
    } else if (msg->hdr.type != type) {
        TR_DEBUG(2,"Invalid %s msg type: 0x%x", str_burn_data, msg->hdr.type);
        errcode = ERR_TYPE_INVALID;
    } else if (msg->hdr.len != 11) {
        TR_DEBUG(2,"%s msg length error: %u", str_burn_data, msg->hdr.len);
        errcode = ERR_LEN;
    } else if (check_sum(data, BURN_DATA_MSG_OVERHEAD) != 0xFF) {
        TR_DEBUG(1,"%s msg checksum error", str_burn_data);
        errcode = ERR_CHECKSUM;
    }

    return errcode;
}

static enum ERR_CODE handle_extra(unsigned char **buf, size_t *len, int *extra)
{
    enum ERR_CODE errcode = ERR_NONE;
    int ret;
    struct CUST_CMD_PARAM param;

    *extra = 0;

    switch (recv_msg.hdr.type) {
        case TYPE_BULK_WRITE_START:
        case TYPE_ERASE_BURN_START:  {
            unsigned int dlen;
            unsigned int mcrc;
            unsigned int crc;
            const char *str_burn_data;

            if (programmer_state == PROGRAMMER_BULK_WRITE_START) {
                str_burn_data = "BULK_WRITE_DATA";
            } else if (programmer_state == PROGRAMMER_ERASE_BURN_START) {
                str_burn_data = "ERASE_BURN_DATA";
            } else {
                TR_DEBUG(1,"Invalid programmer state when receiving burn data: %d", programmer_state);
                return ERR_INTERNAL;
            }

            TR_DEBUG(1,"------ %s ------", str_burn_data);

            errcode = check_extra_burn_data_msg((unsigned char *)&data_buf[cur_data_buf_idx][0]);
            if (errcode != ERR_NONE) {
                return errcode;
            }

            dlen = data_buf[cur_data_buf_idx][4] | (data_buf[cur_data_buf_idx][5] << 8) |
                (data_buf[cur_data_buf_idx][6] << 16) | (data_buf[cur_data_buf_idx][7] << 24);
            mcrc = data_buf[cur_data_buf_idx][8] | (data_buf[cur_data_buf_idx][9] << 8) |
                (data_buf[cur_data_buf_idx][10] << 16) | (data_buf[cur_data_buf_idx][11] << 24);
            cur_sector_seq = data_buf[cur_data_buf_idx][12] | (data_buf[cur_data_buf_idx][13] << 8);
            TR_DEBUG(4,"%s: sec_seq=%u dlen=%u cur_data_buf_idx=%u", str_burn_data, cur_sector_seq, dlen, cur_data_buf_idx);

            if (cur_sector_seq >= sector_cnt) {
                TR_DEBUG(3,"%s: Bad sector seq: sec_seq=%u sector_cnt=%u", str_burn_data, cur_sector_seq, sector_cnt);
                send_burn_data_reply(ERR_SECTOR_SEQ, cur_sector_seq, data_buf[cur_data_buf_idx][2]);
                return ERR_NONE;
            }

            if (((cur_sector_seq + 1) == sector_cnt && dlen != last_sector_len) ||
                    ((cur_sector_seq + 1) != sector_cnt && dlen != sector_size)) {
                TR_DEBUG(3,"%s: Bad sector len: sec_seq=%u dlen=%u", str_burn_data, cur_sector_seq, dlen);
                send_burn_data_reply(ERR_SECTOR_DATA_LEN, cur_sector_seq, data_buf[cur_data_buf_idx][2]);
                return ERR_NONE;
            }

            crc = crc32(0, (unsigned char *)&data_buf[cur_data_buf_idx][BURN_DATA_MSG_OVERHEAD], dlen);
            if (crc != mcrc) {
                TR_DEBUG(1,"%s: Bad CRC", str_burn_data);
                send_burn_data_reply(ERR_SECTOR_DATA_CRC, cur_sector_seq, data_buf[cur_data_buf_idx][2]);
                return ERR_NONE;
            }

            burn_data_buf(cur_data_buf_idx);

            if (cur_sector_seq + 1 == sector_cnt) {
                ret = wait_all_data_buf_finished();
                if (ret) {
                    TR_DEBUG(1,"%s: Waiting all data buffer free failed", str_burn_data);
                    return ERR_INTERNAL;
                }
                // Reset state
                programmer_state = PROGRAMMER_NONE;
            } else {
                ret = get_free_data_buf();
                if (ret) {
                    TR_DEBUG(1,"%s: Getting free data buffer failed", str_burn_data);
                    return ERR_INTERNAL;
                }

                TR_DEBUG(2,"%s: Recv next buffer. cur_data_buf_idx=%u", str_burn_data, cur_data_buf_idx);

                *extra = 1;
                *buf = (unsigned char *)&data_buf[cur_data_buf_idx][0];
                *len = BURN_DATA_MSG_OVERHEAD + ((cur_sector_seq + 2 == sector_cnt) ? last_sector_len : sector_size);
                //extra_buf_len = sizeof(data_buf[cur_data_buf_idx]);
                extra_buf_len = ANC_PARSE_DATA_BUFF_SIZE;
                recv_extra_timeout = default_recv_timeout_short + (*len + 4096 - 1) / 4096 * default_recv_timeout_4k_data;;
            }
            break;
        }
        default:
            param.stage = CUST_CMD_STAGE_EXTRA;
            param.msg = &recv_msg;
            param.extra = 0;
            errcode = handle_cust_cmd(&param);
            if (errcode == ERR_NONE && param.extra) {
                *extra = 1;
                *buf = param.buf;
                *len = param.expect;
                extra_buf_len = param.size;
                recv_extra_timeout = param.timeout;
            }
            return errcode;
    }
    return ERR_NONE;
}

int parse_packet(unsigned char **buf, size_t *len)
{
    enum ERR_CODE errcode;
    int rlen = *len;
    unsigned char *data;
    int i;
    int extra;
    unsigned char cret;
    bool isNeedContinue;
    isNeedContinue=true;
    TR_DEBUG(2,"rlen:%d,sizeof(recv_msg.hdr):%d",rlen,sizeof(recv_msg.hdr));
    while(1)
    {
        switch (parse_state) {
            case PARSE_HEADER:
                ASSERT(rlen > 0 && rlen <= (sizeof(recv_msg.hdr)+sizeof(recv_msg.data)), "Invalid rlen!");
                TR_DEBUG(1,"###prefix:%d",recv_msg.hdr.prefix);
                if (recv_msg.hdr.prefix != PREFIX_CHAR) {
                    data = (unsigned char *)&recv_msg.hdr.prefix;
                    for (i = 1; i < rlen; i++) {
                        if (data[i] == PREFIX_CHAR) {
                            memmove(&recv_msg.hdr.prefix, &data[i], rlen - i);
                            break;
                        }
                    }
                    //*buf = &data[rlen - i];
                    //*len = sizeof(recv_msg.hdr) + i - rlen;
                    if (i == rlen)
                    {
                        TR_DEBUG(0,"recv_msg.hdr.prefix != PREFIX_CHAR !");
                        goto _err;
                    }
                    else {
                        rlen -= i;
                    }
                } else {
                    errcode = check_msg_hdr();
                    if (errcode != ERR_NONE) {
                        goto _err;
                    }
                    parse_state = PARSE_DATA;
                    //*buf = &recv_msg.data[0];
                    //*len = recv_msg.hdr.len + 1;
                }
                break;
            case PARSE_DATA:
            case PARSE_EXTRA:
                if (parse_state == PARSE_DATA) {
                    errcode = handle_data(buf, len, &extra);
                } else {
                    errcode = handle_extra(buf, len, &extra);
                }
                if (errcode != ERR_NONE) {
                    goto _err;
                }
                else
                {
                    isNeedContinue=false;
                }

                if (extra) {
                    parse_state = PARSE_EXTRA;
                } else {
                    // Receive next message
                    reset_parse_state(buf, len);
                }
                break;
            default:
                TR_DEBUG(1,"Invalid parse_state: %d", parse_state);
                isNeedContinue=false;
                break;
        }
        if(false==isNeedContinue)
        {
            break;
        }
    }

    return 0;

_err:
    //cancel_input();
    TR_DEBUG(1,"get errcode:%d",errcode);
    cret = (unsigned char)errcode;
    send_reply(&cret, 1);
    // Receive new message
    reset_parse_state(buf, len);

    return 1;
}


