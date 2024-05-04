#include "xos_mem_pool.h"
#include "xos_typedef.h"

//debug -------------------------------------------------
#define XOS_MEM_POOL_DEBUG_ENABLE
#ifdef  XOS_MEM_POOL_DEBUG_ENABLE 
#define xos_mempool_debug(format,...)     TRACE(5,"[ xos mem pool %d ] %s "format "\n",__LINE__,__func__,##__VA_ARGS__);
#else
#define xos_mempool_debug(format,...) 
#endif

//public members


//prvate membes 
#define XOS_SUSPORT_MEMPOOL_MAX  1024U
static XOS_Mem_PoolInfo xos_MemPoolInfo;


bool xos_MemPoolInit(void)
{
	xos_MemPoolInfo.xos_data_cnt=0;
	xos_MemPoolInfo.xos_read_cnt=0;
	xos_MemPoolInfo.xos_write_cnt=0;
	return XOS_FALSE;
}

bool xos_MemPoolIsFull(void)
{
	if( (++xos_MemPoolInfo.xos_write_cnt)==xos_MemPoolInfo.xos_read_cnt){
		return XOS_TRUE
	}else{
		return XOS_FALSE;
	}
}

bool xos_MemPoolIsEmpty(void)
{
	if( xos_MemPoolInfo.xos_write_cnt==xos_MemPoolInfo.xos_read_cnt ){
		return XOS_TRUE;
	}else{
		return XOS_FALSE;
	}
}

bool xos_MemPoolWrite(uint8_t*pdata,uint32_t len)
{
    uint32_t len_offset=XOS_SUSPORT_MEMPOOL_MAX-xos_MemPoolInfo.xos_write_cnt;
	
	if( (pdata==NULL) ||(len<=0) )	return XOS_TRUE;

	if(xos_MemPoolIsFull())         return XOS_TRUE;

	if( len<=len_offset ){
		memcpy((char*)xos_MemPoolInfo.xos_pdata[xos_MemPoolInfo.xos_write_cnt],(char*)pdata,len);		
		len_offset= (uint32_t)((xos_MemPoolInfo.xos_write_cnt+len) % XOS_SUSPORT_MEMPOOL_MAX);
	}else{
        while( len%XOS_SUSPORT_MEMPOOL_MAX  ){ 

        }
	}


	
	xos_MemPoolInfo.xos_data_cnt+=len;

	return XOS_TRUE;
	
}


bool xos_MemPoolRead(void)
{

	return XOS_FALSE;
}
