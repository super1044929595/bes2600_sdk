/********************************************************************************************************************
*add by jw on 2021-2-8
*Function:This file called by user when debug the breadpoint on the SDK
*
*
*
*
*
*********************************************************************************************************************/
#include "xos_typedef.h"
#include "xos_mem.h"


#define XOS_DEBUG_MEM_ENABLE
#ifdef  XOS_DEBUG_MEM_ENABLE 
#define xos_mem_debug(format,...)     TRACE(5,"[xos mem %d] %s "format "\n",__LINE__,__func__,##__VA_ARGS__);
#else
#define xos_mem_debug(format,...) 
#endif


/* Allocate the memory for the heap. */
static uint8_t jwHeap[JWZOOMSIZE];

//typedef  unsigned int  size_t;

typedef struct JW_BLOCK_LINK
{
	struct JW_BLOCK_LINK* pxNextFreeBlock;
	size_t xBlockSize;
} jwBlockLink_t;

size_t  jwMemBytesRemain = JWZOOMSIZE;

static void prvInsertBlockIntoFreeList(jwBlockLink_t* pxBlockToInsert);

static void prvHeapInit(void);

static const size_t jwHeapSturctSize = (size_t)((sizeof(jwBlockLink_t) + ((size_t)(portBYTE_ALIGNMENT - 1))) & ~((size_t)portBYTE_ALIGNMENT_MASK));

static jwBlockLink_t xStart, * pxEnd = NULL;

static size_t xFreeBytesRemaining = (size_t)0;
static size_t xMinimumEverFreeBytesRemaining = 0U;

static size_t xBlockAllocatedBit = 0;

#define heapMINIMUM_BLOCK_SIZE	( ( size_t ) ( jwHeapSturctSize << 1 ) )

#define heapBITS_PER_BYTE		( ( size_t ) 4 )

void jwAssert(void)
{
	//xos_mem_debug("jwAssert !");  cancel this handle
	//do nothing 
}

/*************************************************************************
*Function:  JW_MemAlloc
*Parameters:
*Return:None
*Note:It called by user when malloc the amount of data
*Date:2021-2-26
*************************************************************************/
void* JW_MemAlloc(size_t jwWantSize)
{
	jwBlockLink_t* pxBlock, * pxPreviousBlock, * pxNewBlockLink;
	void* pvReturn = NULL;

	//osKernelSuspend();
	{

		if (pxEnd == NULL)
		{
			prvHeapInit();
		}
		else
		{
			//APP_UI_TRACE(0,"HJW----->Mem Malloc prvHeapInit ERROR !");
		}

		if ((jwWantSize & xBlockAllocatedBit) == 0)
		{

			if (jwWantSize > 0)
			{
				jwWantSize += jwHeapSturctSize;

				/* Ensure that blocks are always aligned to the required number
				of bytes. */
				if ((jwWantSize & portBYTE_ALIGNMENT_MASK) != 0x00)
				{
					/* Byte alignment required. */
					jwWantSize += (portBYTE_ALIGNMENT - (jwWantSize & portBYTE_ALIGNMENT_MASK));
					//configASSERT( ( xWantedSize & portBYTE_ALIGNMENT_MASK ) == 0 );
				}
				else
				{
					//APP_UI_TRACE(0,"HJW----->JW_MemAlloc Alligment Error !");
				}
			}
			else
			{
				//APP_UI_TRACE(0,"HJW----->JW_MemAlloc xWantedSize is free !");
			}

			if ((jwWantSize > 0) && (jwWantSize <= xFreeBytesRemaining))
			{

				pxPreviousBlock = &xStart;
				pxBlock = xStart.pxNextFreeBlock;
				while ((pxBlock->xBlockSize < jwWantSize) && (pxBlock->pxNextFreeBlock != NULL))
				{
					pxPreviousBlock = pxBlock;
					pxBlock = pxBlock->pxNextFreeBlock;
				}


				if (pxBlock != pxEnd)
				{

					pvReturn = (void*)(((uint8_t*)pxPreviousBlock->pxNextFreeBlock) + jwHeapSturctSize);


					pxPreviousBlock->pxNextFreeBlock = pxBlock->pxNextFreeBlock;

					if ((pxBlock->xBlockSize - jwWantSize) > heapMINIMUM_BLOCK_SIZE)
					{

						pxNewBlockLink = (jwBlockLink_t*)(((uint8_t*)pxBlock) + jwWantSize);
						//configASSERT( ( ( ( size_t ) pxNewBlockLink ) & portBYTE_ALIGNMENT_MASK ) == 0 );

						pxNewBlockLink->xBlockSize = pxBlock->xBlockSize - jwWantSize;
						pxBlock->xBlockSize = jwWantSize;

						prvInsertBlockIntoFreeList(pxNewBlockLink);
					}
					else
					{
						//APP_UI_TRACE(0,"HJW----->JW_MemAlloc xWantedSize is free !");
					}

					xFreeBytesRemaining -= pxBlock->xBlockSize;

					JW_MEMLeftSize();

					//printf("\r\n xFreeBytesRemaining sub is %4d", xFreeBytesRemaining);

					if (xFreeBytesRemaining < xMinimumEverFreeBytesRemaining)
					{
						xMinimumEverFreeBytesRemaining = xFreeBytesRemaining;
					}
					else
					{
						jwAssert();
					}


					pxBlock->xBlockSize |= xBlockAllocatedBit;
					pxBlock->pxNextFreeBlock = NULL;
				}
				else
				{
					//APP_UI_TRACE(0,"HJW----->JW_MemAlloc xWantedSize is free 1!");
				}
			}
			else
			{
				//APP_UI_TRACE(0,"HJW----->JW_MemAlloc xWantedSize is free 2!");
			}
		}
		else
		{
			jwAssert();
		}

		//traceMALLOC( pvReturn, xWantedSize );
	}
	//osKernelResume(1);

#if( configUSE_MALLOC_FAILED_HOOK == 1 )
	{
		if (pvReturn == NULL)
		{
			extern void vApplicationMallocFailedHook(void);
			vApplicationMallocFailedHook();
		}
		else
		{
			mtCOVERAGE_TEST_MARKER();
		}
	}
#endif
	xos_mem_debug("  HJW----->MEM Successful");
	//configASSERT( ( ( ( size_t ) pvReturn ) & ( size_t ) portBYTE_ALIGNMENT_MASK ) == 0 );
	return pvReturn;
}
/*-----------------------------------------------------------*/

void JW_MemFree(void* pv)
{
	uint8_t* puc = (uint8_t*)pv;
	jwBlockLink_t* pxLink;

	if (pv != NULL)
	{
		puc -= jwHeapSturctSize;

		pxLink = (jwBlockLink_t*)puc;

		//configASSERT( ( pxLink->xBlockSize & xBlockAllocatedBit ) != 0 );
		//configASSERT( pxLink->pxNextFreeBlock == NULL );

		if ((pxLink->xBlockSize & xBlockAllocatedBit) != 0)
		{
			if (pxLink->pxNextFreeBlock == NULL)
			{
				pxLink->xBlockSize &= ~xBlockAllocatedBit;

				//osKernelSuspend();
				{
					xFreeBytesRemaining += pxLink->xBlockSize;
					//traceFREE( pv, pxLink->xBlockSize );
					prvInsertBlockIntoFreeList(((jwBlockLink_t*)pxLink));

					JW_MEMLeftSize();

					//printf("\r\n xFreeBytesRemaining add is %2d", xFreeBytesRemaining);
				}
				// osKernelResume(1);
			}
			else
			{
				jwAssert();
			}
		}
		else
		{
			jwAssert();
		}
	}
	xos_mem_debug(" free successful!");
}

size_t xPortGetFreeHeapSize(void)
{
	return xFreeBytesRemaining;
}

size_t xPortGetMinimumEverFreeHeapSize(void)
{
	return xMinimumEverFreeBytesRemaining;
}

void vPortInitialiseBlocks(void)
{
	/* This just exists to keep the linker quiet. */
}

static void prvHeapInit(void)
{
	jwBlockLink_t* pxFirstFreeBlock;
	uint8_t* pucAlignedHeap;
	size_t uxAddress;
	size_t xTotalHeapSize = JWZOOMSIZE;

	/* Ensure the heap starts on a correctly aligned boundary. */
	uxAddress = (size_t)jwHeap;

	if ((uxAddress & portBYTE_ALIGNMENT_MASK) != 0)
	{
		uxAddress += (portBYTE_ALIGNMENT - 1);
		uxAddress &= ~((size_t)portBYTE_ALIGNMENT_MASK);
		xTotalHeapSize -= uxAddress - (size_t)jwHeap;
	}

	pucAlignedHeap = (uint8_t*)uxAddress;

	xStart.pxNextFreeBlock = (struct JW_BLOCK_LINK*)pucAlignedHeap;
	xStart.xBlockSize = (size_t)0;

	uxAddress = ((size_t)pucAlignedHeap) + xTotalHeapSize;
	uxAddress -= jwHeapSturctSize;
	uxAddress &= ~((size_t)portBYTE_ALIGNMENT_MASK);
	pxEnd = (jwBlockLink_t*)uxAddress;
	pxEnd->xBlockSize = 0;
	pxEnd->pxNextFreeBlock = NULL;

	pxFirstFreeBlock = (jwBlockLink_t*)pucAlignedHeap;
	pxFirstFreeBlock->xBlockSize = uxAddress - (size_t)pxFirstFreeBlock;
	pxFirstFreeBlock->pxNextFreeBlock = pxEnd;

	xMinimumEverFreeBytesRemaining = pxFirstFreeBlock->xBlockSize;
	xFreeBytesRemaining = pxFirstFreeBlock->xBlockSize;
	//printf("\r\n xFreeBytesRemaining is %2d", xFreeBytesRemaining);
	xBlockAllocatedBit = ((size_t)1) << ((sizeof(size_t) * heapBITS_PER_BYTE) - 1);

	xos_mem_debug("prvHeapInit!");
}
/*-----------------------------------------------------------*/

static void prvInsertBlockIntoFreeList(jwBlockLink_t* pxBlockToInsert)
{
	jwBlockLink_t* pxIterator;
	uint8_t* puc;

	/* Iterate through the list until a block is found that has a higher address
	than the block being inserted. */
	for (pxIterator = &xStart; pxIterator->pxNextFreeBlock < pxBlockToInsert; pxIterator = pxIterator->pxNextFreeBlock)
	{
		/* Nothing to do here, just iterate to the right position. */
	}

	/* Do the block being inserted, and the block it is being inserted after
	make a contiguous block of memory? */
	puc = (uint8_t*)pxIterator;
	if ((puc + pxIterator->xBlockSize) == (uint8_t*)pxBlockToInsert)
	{
		pxIterator->xBlockSize += pxBlockToInsert->xBlockSize;
		pxBlockToInsert = pxIterator;
	}
	else
	{
		jwAssert();
	}

	/* Do the block being inserted, and the block it is being inserted before
	make a contiguous block of memory? */
	puc = (uint8_t*)pxBlockToInsert;
	if ((puc + pxBlockToInsert->xBlockSize) == (uint8_t*)pxIterator->pxNextFreeBlock)
	{
		if (pxIterator->pxNextFreeBlock != pxEnd)
		{
			/* Form one big block from the two blocks. */
			pxBlockToInsert->xBlockSize += pxIterator->pxNextFreeBlock->xBlockSize;
			pxBlockToInsert->pxNextFreeBlock = pxIterator->pxNextFreeBlock->pxNextFreeBlock;
		}
		else
		{
			pxBlockToInsert->pxNextFreeBlock = pxEnd;
		}
	}
	else
	{
		pxBlockToInsert->pxNextFreeBlock = pxIterator->pxNextFreeBlock;
	}

	/* If the block being inserted plugged a gab, so was merged with the block
	before and the block after, then it's pxNextFreeBlock pointer will have
	already been set, and should not be set here as that would make it point
	to itself. */
	if (pxIterator != pxBlockToInsert)
	{
		pxIterator->pxNextFreeBlock = pxBlockToInsert;
	}
	else
	{
		jwAssert();
	}
}

size_t JW_MEMLeftSize(void)
{
	jwMemBytesRemain = xFreeBytesRemaining;
#if defined  JW_MEM_PRINTFENABLE
	xos_mem_debug("\r\n xFreeBytesRemaining  is %2d", jwMemBytesRemain);
#endif
	return jwMemBytesRemain;
}

void JW_MEM_TEST(void)
{
	uint8_t* prc = NULL;
	uint8_t* prc1 = NULL;
	uint8_t* prc2 = NULL;
	for (int i = 0; i < 100; i++) {
		prc = (uint8_t*)JW_MemAlloc(128);
		prc1 = (uint8_t*)JW_MemAlloc(128);
		prc2 = (uint8_t*)JW_MemAlloc(128);
		sprintf((char*)prc,"%s","tesassjahgsag");
		JW_MemFree(prc);
		JW_MemFree(prc2);
		JW_MemFree(prc1);		
	}
}





