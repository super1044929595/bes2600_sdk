
#ifndef  __xOS_STATE_H
#define  __xOS_STATE_H

#ifdef __cplusplus
	extern "C"{
#endif
#include "stdint.h"
#ifdef XOS_RTX_OSSDK_ENABLE
#include "hal_trace.h"
#include "app_utils.h"
#include "app_thread.h"
#include "hal_timer.h"
#include "cmsis_os.h"
#include "app_thread.h"
#include "app_utils.h"
#endif

 
#define OMNI_USER_MAIN_ENABLE   0

typedef uint8_t xos_handle_state;

typedef uint8_t xos_handle_operate;

typedef void(*xos_handle)(xos_handle_state pre,xos_handle_operate operate,xos_handle_state next);

typedef struct  _xOS_StateInfo{
    
    const struct _xOS_StateInfo* xTable;
    
    uint8_t            xTablecnt;
    
    xos_handle_state   prestate;
    
    xos_handle_state   newstate;
    
    xos_handle_operate operate;
    
    xos_handle         handle;
    
}/*__attribute__((packed, aligned(4))) */xOS_StateInfo;

typedef enum{
	RUN_IDLE,
	RUN_BLE_ADVERSTING,
	RUN_BLE_CONNETCTED,
}RCU_RUN_STATUS_ENUM;

typedef struct{

	RCU_RUN_STATUS_ENUM current_mode;
	
}RCU_SystemInfo;


/// RCU RUN states
typedef enum {
    /// System booted.
    RCU_S_BOOTED=0,
    /// Initialing.
    RCU_S_INITING,
    /// Initialized.
    RCU_S_IDLE,
    /// Pairing.
    RCU_S_PAIRING,
    /// Reconnecting.
    RCU_S_RECONNING,
    /// Connected but no profile ready.
    RCU_S_CONNECTED,
    /// HID is ready
    RCU_S_HID_READY,
    /// Disconnecting
    RCU_S_DISCONNING,
    /// Under RF testing mode.
    RCU_S_RF_TEST,
    /// Dummy state for marking.
    RCU_S_NUM
} RCU_Run_state_t;

/// MMI operations for MMI state transition
typedef enum {
    /// Started initialing.
    RCU_OP_INITING=0,
    /// Initialized.
    RCU_OP_INIT_DONE,
    /// Started reconnecting.
    RCU_OP_RECONNING,
    /// Started pairing.
    RCU_OP_PAIRING,
    /// Reconnection failed.
    RCU_OP_RECONN_FAIL,
    /// Pairing failed.
    RCU_OP_PAIR_FAIL,
    /// Reconnection timeout.
    RCU_OP_RECONN_TOUT,
    /// Pairing timeout.
    RCU_OP_PAIR_TOUT,
    /// Pairing success.
    RCU_OP_PAIR_SUCCESS,
   /// Connected.
    RCU_OP_CONNECTED,
    /// Disconnected.
    RCU_OP_DISCONNED,
    /// HOGP became ready.
    RCU_OP_HID_READY,
    /// HOGP became unready.
    RCU_OP_HID_UNREADY,
    /// Started disconnecting.
    RCU_OP_DISCONNING,
    /// ADV became stopped.
    RCU_OP_ADV_STOPPED,
    /// for number counter in code.
    RCU_OP_NUM,
} RCU_op_t;

#define XOS_STATE(state,op)  ((struct _xOS_StateInfo*)NULL),(0),(xos_handle_state)state,(xos_handle_state)op

//public
//typedef void os_Public_Handle(uint8_t index,xos_handle_operate operateId);

void os_HanldeTableInit(uint8_t index, const xOS_StateInfo *info,uint8_t table_size);
void os_handleTable_PublicHandle_Register(uint8_t index,xos_handle handle );
void os_Handle_CurrentState_Set(uint8_t index,xos_handle_state state ,xos_handle_operate operateId);
void os_Handle_StateSwitch(uint8_t index,xos_handle_operate operateId);
//void os_Public_Handle_Register(os_Public_Handle _cb);
void os_Handle_CurrentState_JumpeSet(uint8_t index, xos_handle_state state, xos_handle_operate operateId);

#ifdef __cplusplus
}
#endif

#endif
