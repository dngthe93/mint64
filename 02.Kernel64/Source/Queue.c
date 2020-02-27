
#include "Queue.h"
#include "Utility.h"


void kInitializeQueue(QUEUE *pstQueue, void *pvQueueBuffer, int iMaxDataCount, int iDataSize)
{
	pstQueue->iMaxDataCount = iMaxDataCount;
	pstQueue->iDataSize = iDataSize;
	pstQueue->pvQueueArray = pvQueueBuffer;

	pstQueue->iPutIndex = 0;
	pstQueue->iGetIndex = 0;
	pstQueue->bLastOperationPut = FALSE;
}

BOOL kIsQueueFull(const QUEUE *pstQueue)
{
	return pstQueue->iGetIndex == pstQueue->iPutIndex &&
		pstQueue->bLastOperationPut;
}

BOOL kIsQueueEmpty(const QUEUE *pstQueue)
{
	return pstQueue->iGetIndex == pstQueue->iPutIndex &&
		!pstQueue->bLastOperationPut;
}

BOOL kPutQueue(QUEUE *pstQ, const void *pvData)
{
	if (kIsQueueFull(pstQ))
		return FALSE;

	register char *dst = (char*)pstQ->pvQueueArray + (pstQ->iDataSize * pstQ->iPutIndex);
	kMemCpy(dst, pvData, pstQ->iDataSize);

	pstQ->iPutIndex = (pstQ->iPutIndex + 1) % pstQ->iMaxDataCount;
	pstQ->bLastOperationPut = TRUE;
	return TRUE;
}

BOOL kGetQueue(QUEUE *pstQ, void *pvData)
{
	if (kIsQueueEmpty(pstQ))
		return FALSE;

	register char *src = (char*)pstQ->pvQueueArray + (pstQ->iDataSize * pstQ->iGetIndex);
	kMemCpy(pvData, src, pstQ->iDataSize);

	pstQ->iGetIndex = (pstQ->iGetIndex + 1) % pstQ->iMaxDataCount;
	pstQ->bLastOperationPut = FALSE;
	return TRUE;
}
