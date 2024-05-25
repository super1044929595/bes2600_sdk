#include "xos_intterrupt.h"

#define NR_CPUS 4U 

xos_taskTableHead_t xos_taskTable_Vec[NR_CPUS];
xos_taskTableHead_t xos_taskTable_Hi_Vec[NR_CPUS];

static inline void xos_TasktTableIntterrupt_Schedule(taskintterrupt_t *xos_intter)
{
     //get cpu processor id 
     uint8_t cpu_id=0;
     xos_intter->next=xos_taskTable_Vec[cpu_id].list;
	 //interrupt save 
	 xos_taskTable_Vec[cpu_id].list=xos_intter;

	 // intterrupt handle inq  active
	 
	 //intterupt restore
	 
}

static inline void xos_TasktTableIntterrupt_Handle(void)
{
	taskintterrupt_t *list;

	//get cpu processor id 

	//disbale intterrupt
	uint8_t cpu_id=0;
    list=xos_taskTable_Vec[cpu_id].list;
    xos_taskTable_Vec[cpu_id].list=NULL;
	//enable intterrupt
	while(list!=NULL)
	{

          //try to lock 
          //clear the int bit
          list->func(list->func_parame);		   		   
 		  //unlock     
		  list=list->next;
	}
}

