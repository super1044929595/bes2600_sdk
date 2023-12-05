#include "xOS_State.h"
#if OMNI_USER_MAIN_ENABLE




#define XOS_STATEDEBUG_ENABLE
#ifdef XOS_STATEDEBUG_ENABLE 
#define xos_state_debug(format,...)    printf("omni app[%s:%s (%d)]:" format "\n" ,__FILE__,__FUNCTION__,__LINE__,##__VA_ARGS__);
#else
#define xos_state_debug(format,...) 
#endif

#define STATE_TABLE_NUMBERS   3U
#define CURRENT_OMNI_DEFAULT  0U





xOS_StateInfo osStateTable[STATE_TABLE_NUMBERS];

#define XOS_STATE(state,op)  ((struct _xOS_StateInfo*)NULL),(0),(xos_handle_state)state,(xos_handle_state)op

static os_Public_Handle _xPublichandle=NULL; 





static void os_HanldeTableInit(uint8_t index, const xOS_StateInfo *info,uint8_t table_size)
{
   
    osStateTable[index].xTable=info;
    
    osStateTable[index].xTablecnt=table_size;

	osStateTable[index].prestate=0;
	
	osStateTable[index].newstate=0;

	osStateTable[index].operate=RCU_OP_INITING;
	
    xos_state_debug("\r\n");
    
}

static void os_handleTable_PublicHandle_Register(uint8_t index,xos_handle handle )
{
    osStateTable[index].handle=handle;
}

static void os_Handle_CurrentState_Set(uint8_t index,xos_handle_state state ,xos_handle_operate operateId)
{
   
    osStateTable[index].prestate=osStateTable[index].newstate;
    
    osStateTable[index].newstate=state;
    
    osStateTable[index].operate =operateId;
    
    if(osStateTable[index].handle){
        
        //printf("\r\n hanle the public handle on CurrentState ");
        
        osStateTable[index].handle(osStateTable[index].newstate,osStateTable[index].operate,state);
        
    }
   
    printf("                            os_Handle_CurrentState_Set pre state:[%d], new state:[%d] ,operateId:[%d] \n ", osStateTable[index].prestate,state,operateId);
    
}


static void os_Handle_StateSwitch(uint8_t index,xos_handle_operate operateId)
{
    #ifdef XOS_STATEDEBUG_ENABLE
    xos_state_debug("switch[op id:%d],newstate[%d]",operateId,osStateTable[index].newstate);
	#endif
	
    for(uint8_t i=0;i<osStateTable[index].xTablecnt;i++){

	
#ifdef XOS_STATEDEBUG_ENABLE
   		xos_state_debug("switch com [operate id:%d],prestate[%d]",osStateTable[index].xTable[i].operate,osStateTable[index].xTable[i].prestate );
#endif

        if( osStateTable[index].xTable[i].operate == operateId && osStateTable[index].xTable[i].prestate == osStateTable[index].newstate ){

		
#ifdef XOS_STATEDEBUG_ENABLE
            xos_state_debug("sys pre:%d ,new:%d" ,osStateTable[index].prestate ,osStateTable[index].newstate);

		    xos_state_debug("usr pre:%d ,new:%d" ,osStateTable[index].xTable[i].prestate ,osStateTable[index].xTable[i].newstate);
            
            xos_state_debug("[ index:%d ][total :%d]------pre :%d , new :%d  operateId:%d \n",i,osStateTable[index].xTablecnt, osStateTable[index].newstate,osStateTable[index].xTable[i].newstate,operateId);
#endif

            os_Handle_CurrentState_Set(index,osStateTable[index].xTable[i].newstate,operateId);
            
            if( osStateTable[index].xTable[i].handle ){
                
                osStateTable[index].xTable[i].handle(osStateTable[index].xTable[i].prestate,operateId,osStateTable[index].xTable[i].newstate);
                
            }
            
            break;
            
         }
        
    }
    
}

void osState_handle_public(xos_handle_state pre,xos_handle_operate operate,xos_handle_state next)
{

}

void os_Public_Handle_Register(os_Public_Handle _cb)
{
	_xPublichandle=_cb;
}




#endif

