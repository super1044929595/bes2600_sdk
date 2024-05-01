#ifndef _L2CC_PDU_INT_H_
#define _L2CC_PDU_INT_H_

/**
 ****************************************************************************************
 * @addtogroup L2CC_PDU_INT L2Cap Controller (internal)
 * @ingroup L2CC
 * @brief This module is in charge to pack or unpack L2CAP packets
 *
 * @{
 ****************************************************************************************
 */

/*
 * INCLUDE FILES
 ****************************************************************************************
 */

#include "rwip_config.h"

#if (BLE_L2CC)

#include "l2cc_pdu.h"

/*
 * MACROS
 ****************************************************************************************
 */


/*
 * DEFINES
 ****************************************************************************************
 */

/*
 * FUNCTION DECLARATIONS
 ****************************************************************************************
 */

/**
 ****************************************************************************************
 * Pack PDU from firmware readable data to a L2CAP packet using generic method.
 * (no segmentation)
 *
 * @param[in]      p_pdu       Pointer to the unpacked PDU structure
 * @param[in|out]  p_offset    Pointer to address where the next part of PDU will be read
 * @param[in|out]  p_length    Pointer to the length of payload that has been packed
 * @param[in|out]  p_buffer    Pointer to address where the next part of PDU will be written
 * @param[in|out]  pb_flag     Packet Boundary flags
 *
 * @return GAP_ERR_NO_ERROR if packing succeed, else another error code.
 ****************************************************************************************
 */
#ifdef __GATT_OVER_BR_EDR__
    uint8_t l2cc_pdu_pack(uint8_t conidx, struct l2cc_pdu *p_pdu, uint16_t *p_offset, uint16_t *p_length, uint8_t *p_buffer, uint8_t *pb_flag);
#else
    uint8_t l2cc_pdu_pack(struct l2cc_pdu *p_pdu, uint16_t *p_offset, uint16_t *p_length, uint8_t *p_buffer, uint8_t *pb_flag);
#endif

/**
 ****************************************************************************************
 * Unpack L2Cap PDU in a generic format that can be used by firmware using generic method.
 * (no reassembly)
 *
 * @param[in]      p_pdu       Pointer to the unpacked PDU structure
 * @param[in|out]  p_offset    Pointer to address where the next part of PDU will be written
 * @param[in|out]  p_rem_len   Pointer to the remaining length value of PDU to receive
 * @param[in|out]  p_buffer    Pointer to address where the next part of PDU will be read
 * @param[in]      pkt_length  Length of the received packet
 * @param[in]      pb_flag     Packet Boundary flags
 *
 * @return GAP_ERR_NO_ERROR if packing succeed, else another error code.
 ****************************************************************************************
 */
uint8_t l2cc_pdu_unpack(struct l2cc_pdu *p_pdu, uint16_t *p_offset, uint16_t *p_rem_len,
                        const uint8_t *p_buffer, uint16_t pkt_length, uint8_t pb_flag);



#if (RW_DEBUG)
/**
 ****************************************************************************************
 * Pack DBG PDU (any kind of PDU generated by Host) from firmware readable data to a L2CAP
 * packet (no segmentation)
 *
 * @param[in]      conidx      Connection Index
 * @param[in]      pdu         Pointer to the Debug PDU to send
 * @param[in|out]  p_length    Pointer to the length of payload that has been packed
 * @param[in|out]  buffer      Pointer to address where the next part of PDU will be written
 * @param[in|out]  offset      Pointer to address where the next part of PDU will be read
 * @param[in|out]  pb_flag     Packet Boundary flags
 *
 * @return GAP_ERR_NO_ERROR if packing succeed, else another error code.
 ****************************************************************************************
 */
uint8_t l2cc_dbg_pdu_pack(struct l2cc_dbg_pdu *pdu, uint16_t *length, uint8_t *buffer, uint16_t* offset,uint8_t *pb_flag);


/**
 ****************************************************************************************
 * Unpack L2Cap PDU in a generic format that can be used by firmware for debugging
 * (no reassembly)
 *
 * @param[in|out]  sdu         Pointer to the unpacked SDU structure
 * @param[in]      buffer      Pointer to address where the next part of PDU will be read
 * @param[in]      length      Length of the received packet
 * @param[in|out]  offset      Pointer to address where the next part of PDU will be written
 * @param[in]      pb_flag     Packet Boundary flags
 *
 * @return GAP_ERR_NO_ERROR if packing succeed, else another error code.
 ****************************************************************************************
 */
uint8_t l2cc_dbg_pdu_unpack(struct l2cc_dbg_pdu *pdu, uint8_t *buffer, uint16_t length, uint16_t* offset, uint8_t pb_flag);
#endif // (RW_DEBUG)

#if (BLE_LECB)
/**
 ****************************************************************************************
 * Pack LE Credit Based channel PDU from firmware readable data to a L2CAP packet
 *
 * @param[in]      conidx      Connection Index
 * @param[in]      pdu         Pointer to the SDU to send
 * @param[out]     length      Pointer to the length of payload that has been packed
 * @param[in|out]  buffer      Pointer to address where the next part of PDU will be written
 * @param[in|out]  offset      Pointer to address where the next part of PDU will be read
 * @param[in|out]  pdu_len     PDU length to transmitt
 * @param[in]      pb_flag     Packet Boundary flags
 *
 * @return GAP_ERR_NO_ERROR if packing succeed, else another error code.
 ****************************************************************************************
 */
uint8_t l2cc_lecb_pdu_pack(uint8_t conidx, struct l2cc_sdu *sdu, uint16_t *length, uint8_t *buffer, uint16_t* offset,
                           uint16_t pdu_len, uint8_t pb_flag);


/**
 ****************************************************************************************
 * Unpack L2Cap LECB SDU in a generic format that can be used by firmware
 *
 * @param[in|out] sdu            Pointer to the unpacked SDU structure
 * @param[in]     buffer         Pointer to address where the next part of PDU will be read
 * @param[in]     length         Length of the received packet
 * @param[in|out] offset         Pointer to address where the next part of PDU will be written
 * @param[in|out] pdu_remain_len Pointer to the data length still expected on segment
 * @param[in]     pb_flag     Packet Boundary flags
 *
 * @return GAP_ERR_NO_ERROR if packing succeed, else another error code.
 ****************************************************************************************
 */
uint8_t l2cc_lecb_pdu_unpack(struct l2cc_sdu *sdu, uint8_t *buffer, uint16_t length, uint16_t* offset,
         uint16_t* pdu_remain_len, uint8_t pb_flag);

#endif //(BLE_LECB)

/**
 ****************************************************************************************
 * @brief Check the PDU packet header channel ID, Length, in order to:
 *  - verify if no error detected
 *  - allocate or retrieve the RX messge for expected destination task
 *
 * @param[in]  conidx   Connection Index
 * @param[in]  buffer   RX buffer that contains first PDU fragment
 *
 * @return GAP_ERR_NO_ERROR if header sanity check succeed, else an error code to ignore
 *         reception of PDU
 ****************************************************************************************
 */
uint8_t l2cc_pdu_header_check(uint8_t conidx, uint8_t* buffer);


/// @} L2CC_PDU_INT

#endif //(BLE_L2CC)

#endif /* _L2CC_PDU_INT_H_ */
