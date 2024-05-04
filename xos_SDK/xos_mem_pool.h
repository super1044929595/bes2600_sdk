#ifndef __XOS_MEM_POOL_H__
#define __XOS_MEM_POOL_H__

#ifdef __cplusplus
extern "C"{
#endif
//----------------------------------------------------------------

typedef struct{

	uint32_t xos_read_cnt;
	uint32_t xos_write_cnt;
	uint8_t  *xos_pdata;
	uint32_t xos_used;
	
	//-------os susport
		// mutex lock
		//write semphore 
		//read semphore 
	//------------------
	
}XOS_Mem_PoolInfo;

extern XOS_Mem_PoolInfo xos_MemPoolInfo;



//----------------------------------------------------------------
#ifdef __cplusplus
}
#endif

#endif
