#include "xOS_App.h"

// UI handle
static void
osState_op_tws_InitDone(xos_handle_state pre,xos_handle_operate operate,xos_handle_state next);
static void
osState_OP_tws_BoxIN_Hanlde(xos_handle_state pre,xos_handle_operate operate,xos_handle_state next);
static void
osState_OP_tws_BoxOut_Hanlde(xos_handle_state pre,xos_handle_operate operate,xos_handle_state next);
static void 
osState_handle_public(xos_handle_state pre,xos_handle_operate operate,xos_handle_state next);

/*+---------------------------------------------------------------------------
	              App User Table Init
  +--------------------------------------------------------------------------*/
  
const xOS_StateInfo appUserTabel[]={
    {XOS_STATE(XOS_APP_STATE_INITING_E  , XOS_APP_STATE_INITDONE_E )      ,XOS_APP_OP_INITING_E      ,osState_op_tws_InitDone},
    {XOS_STATE(XOS_APP_STATE_INITDONE_E , XOS_APP_STATE_BOX_IN_E   )      ,XOS_APP_OP_BOX_IN_E       ,osState_OP_tws_BoxIN_Hanlde},   
    {XOS_STATE(XOS_APP_STATE_BOX_IN_E  ,  XOS_APP_STATE_COVER_IN_E )      ,XOS_APP_OP_BOX_OUT_E      ,osState_OP_Pairing},
};


static void osState_OP_Initing(xos_handle_state pre,xos_handle_operate operate,xos_handle_state next)
{
    xos_state_debug("osState_OP_Initing"); 
	//----------------------------------------------------------

	//----------------------------------------------------------
	os_Handle_StateSwitch(CURRENT_OMNI_DEFAULT,RCU_OP_INIT_DONE);
}
static void osState_op_tws_InitDone(xos_handle_state pre,xos_handle_operate operate,xos_handle_state next)
{


};
	
static void osState_OP_tws_BoxIN_Hanlde(xos_handle_state pre,xos_handle_operate operate,xos_handle_state next)
{
    xos_state_debug("osState_OP_tws_BoxIN_Hanlde");
	
	os_Handle_StateSwitch(CURRENT_OMNI_DEFAULT,RCU_OP_PAIRING);
}

static void osState_OP_tws_BoxOut_Hanlde(xos_handle_state pre,xos_handle_operate operate,xos_handle_state next)
{
    xos_state_debug("osState_OP_tws_BoxOut_Hanlde");

}

/*+---------------------------------------------------------------------------
			      App User Table End
	+--------------------------------------------------------------------------*/


bool User_APP_Init(void)
{
	//led_demo_init();
    os_HanldeTableInit(0,appUserTabel,sizeof(appUserTabel)/sizeof(appUserTabel[0]));

    os_handleTable_PublicHandle_Register(0,osState_handle_public);

	xos_state_debug("ADV timeout duration :[%d]",CFG_ADV0_START_DURATION);	
	xos_state_debug("CFG_GAP_CONN_INT_MIN :[%d],CFG_GAP_CONN_INT_MAX :[%d]",CFG_GAP_CONN_INT_MIN,CFG_GAP_CONN_INT_MAX);
	xos_state_debug("Model Number String :[%s]",APP_DIS_MODEL_NB_STR);
	xos_state_debug("Manufacturer :[%s]",APP_DIS_MANUFACTURER_NAME);

	os_Handle_StateSwitch(CURRENT_OMNI_DEFAULT,RCU_OP_INITING);
	return false;
}

