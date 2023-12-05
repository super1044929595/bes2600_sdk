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

#ifndef __TRANS_ADAPT_V2_H__
#define __TRANS_ADAPT_V2_H__

#if defined(__cplusplus)
extern "C" {
#endif

void BESHCI_SendData(unsigned char type, unsigned short cmd_conn, unsigned short len, unsigned char *buffer);
int BESHCI_SendBuffer(unsigned char packet_type, unsigned char *packet, int size);
unsigned short hci_h4_receive_msg( const uint8_t *buf, uint32_t size);

#if defined(USE_CBUFF_TO_CACHE_RTXBUFF)
typedef struct
{
	unsigned char type;
	unsigned short conn_handle;
	unsigned short length;
	unsigned char *data;
} hci_cq_element_t;

int hci_rxtx_buff_open(void);
int hci_rx_buff_pop(unsigned char *type, unsigned short *conn_handle, unsigned short *length, unsigned char **data);
int hci_tx_buff_pop(unsigned char *type, unsigned short *conn_handle, unsigned short *length, unsigned char **data);
int hci_tx_buff_push(unsigned char type, unsigned short conn_handle, unsigned short length, unsigned char *data);
int hci_rx_buff_remain_data_len(void);
int hci_tx_buff_remain_data_len(void);
int hci_rx_buff_register_notify_callback(void (*cb)(void));
int hci_tx_buff_register_notify_callback(void (*cb)(void));
int hci_flush_buff(unsigned char hci_cqtype, unsigned char msg_type, unsigned short conn_handle);
#endif

#if defined(__cplusplus)
}
#endif

#endif /* __TRANS_ADAPT_V2_H__ */
