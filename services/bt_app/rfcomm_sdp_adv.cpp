#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "cmsis.h"
#include "cmsis_os.h"
#include "hal_trace.h"
#include "os_api.h"
#include "bt_if.h"
#include "app_bt.h"
#include "sdp.h"
#include "sdp_api.h"
#ifdef __RFCOMM_SDP_UUID_ENABLE__

/* 128 bit UUID in Big Endian df21fe2c-2515-4fdb-8886-f12c4d67927c */
static const uint8_t ADV_RFCOMM_UUID_128[16] = {
    0xA5, 0xA5, 0x00, 0x5B, 0x02, 0x00, 0x23 ,0x9B, 0xE1, 0x11 , 0x02, 0xD1, 0x03, 0xeb, 0x86, 0x84};

static const U8 rfcommClassId[] = {
    SDP_ATTRIB_HEADER_8BIT(17),        /* Data Element Sequence, 17 bytes */
    SDP_UUID_128BIT(ADV_RFCOMM_UUID_128),    /* 128 bit UUID in Big Endian */
};

static const U8 RfcommProtoDescList[] = {
    SDP_ATTRIB_HEADER_8BIT(5),  /* Data element sequence, 12 bytes */

    /* Each element of the list is a Protocol descriptor which is a
    * data element sequence. The first element is L2CAP which only
    * has a UUID element.
    */
    SDP_ATTRIB_HEADER_8BIT(3),   /* Data element sequence for L2CAP, 3
                                * bytes
                                */

    SDP_UUID_16BIT(PROT_L2CAP)   /* Uuid16 L2CAP */

    /* Next protocol descriptor in the list is RFCOMM. It contains two
    * elements which are the UUID and the channel. Ultimately this
    * channel will need to filled in with value returned by RFCOMM.
    */

    /* Data element sequence for RFCOMM, 5 bytes */
    
    //SDP_ATTRIB_HEADER_8BIT(5),


    //SDP_UUID_16BIT(PROT_RFCOMM), /* Uuid16 RFCOMM */

   
    /* Uint8 RFCOMM channel number - value can vary */
    //SDP_UINT_8BIT(0)
};

/* SPP attributes.
 *
 * This is a ROM template for the RAM structure used to register the
 * SPP SDP record.
 */
struct sdp_server_record_attr rfcommAttributes[] = {
    SDP_DEF_ATTRIBUTE(SERV_ATTRID_PROTOCOL_DESC_LIST, RfcommProtoDescList),

    SDP_DEF_ATTRIBUTE(SERV_ATTRID_SERVICE_CLASS_ID_LIST, rfcommClassId),
};

struct sdp_server_record rfcomm_uuid_sdp;

void _rfcomm_sdp_adv_init(void)
{
    static bool initialized = false;
    if(!initialized)
    {
        initialized = true;
        rfcomm_uuid_sdp.record_handle = 0x0;
        sdp_init_server_record(rfcommAttributes, sizeof(rfcommAttributes), &rfcomm_uuid_sdp);
        sdp_server_add_global_record(&rfcomm_uuid_sdp); 
    }
}

#endif