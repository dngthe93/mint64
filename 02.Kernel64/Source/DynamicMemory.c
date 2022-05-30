
#include "DynamicMemory.h"
#include "Task.h"
#include "Utility.h"


static DYNAMICMEMORY gs_stDynamicMemory;

void kInitializeDynamicMemory()
{
	QWORD qwDynamicMemorySize;
	int i, j;
	BYTE *pbCurrentBitmapPosition;
	int iBlockCountOfLevel, iMetaBlockCount;

	qwDynamicMemorySize = kCalculateDynamicMemorySize();
	iMetaBlockCount = kCalculateMetaBlockCount(qwDynamicMemorySize);

	gs_stDynamicMemory.iBlockCountOfSmallestBlock =
		(qwDynamicMemorySize / DYNAMICMEMORY_MIN_SIZE) - iMetaBlockCount;

	// Calculate the max level count
	for (i = 0; (gs_stDynamicMemory.iBlockCountOfSmallestBlock >> i) > 0; i++);
	gs_stDynamicMemory.iMaxLevelCount = i;

	gs_stDynamicMemory.pbAllocatedBlockListIndex = (BYTE*)DYNAMICMEMORY_START_ADDRESS;
	for (i = 0; i < gs_stDynamicMemory.iBlockCountOfSmallestBlock; i++)
		gs_stDynamicMemory.pbAllocatedBlockListIndex[i] = 0xFF;



	gs_stDynamicMemory.pstBitmapOfLevel = (BITMAP*)(DYNAMICMEMORY_START_ADDRESS +
		sizeof (BYTE) * gs_stDynamicMemory.iBlockCountOfSmallestBlock);

	pbCurrentBitmapPosition = ((BYTE*)gs_stDynamicMemory.pstBitmapOfLevel) +
		(sizeof(BITMAP) * gs_stDynamicMemory.iMaxLevelCount);

	for (j = 0; j < gs_stDynamicMemory.iMaxLevelCount; j++)
	{
		gs_stDynamicMemory.pstBitmapOfLevel[j].pbBitmap = pbCurrentBitmapPosition;
		gs_stDynamicMemory.pstBitmapOfLevel[j].qwExistBitCount = 0;
		iBlockCountOfLevel = gs_stDynamicMemory.iBlockCountOfSmallestBlock >> j;

		for (i = 0; i < iBlockCountOfLevel / 8; i++)
			*(pbCurrentBitmapPosition++) = 0x00;

		i = iBlockCountOfLevel % 8;
		if (i != 0)
		{
			//*pbCurrentBitmapPosition = 0x00;
			if (i & 1)
			{
				*pbCurrentBitmapPosition = (DYNAMICMEMORY_EXIST << (i - 1));
				gs_stDynamicMemory.pstBitmapOfLevel[j].qwExistBitCount = 1;
			}
			else
				*pbCurrentBitmapPosition = 0x00;

			pbCurrentBitmapPosition++;
		}
	}

	gs_stDynamicMemory.qwStartAddress = DYNAMICMEMORY_START_ADDRESS +
		iMetaBlockCount * DYNAMICMEMORY_MIN_SIZE;
	// < 3GB
//	gs_stDynamicMemory.qwEndAddress = kCalculateDynamicMemorySize() +
//		DYNAMICMEMORY_START_ADDRESS;
	gs_stDynamicMemory.qwEndAddress = qwDynamicMemorySize + DYNAMICMEMORY_START_ADDRESS;
	gs_stDynamicMemory.qwUsedSize = 0;
}

static QWORD kCalculateDynamicMemorySize()
{
	QWORD qwRAMSize;

	// Upper bound: 3GB
	qwRAMSize = (kGetTotalRAMSize() * 1024 * 1024);
	if (qwRAMSize > (QWORD)3 * 1024 * 1024 * 1024)
		qwRAMSize = (QWORD)3 * 1024 * 1024 * 1024;

	return qwRAMSize - DYNAMICMEMORY_START_ADDRESS;
}

static int kCalculateMetaBlockCount(QWORD qwDynamicRAMSize)
{
	long lBlockCountOfSmallestBlock;
	DWORD dwSizeOfAllocatedBlockListIndex;
	DWORD dwSizeOfBitmap;
	long i;


	// round down (1025Bytes -> only one block)
	lBlockCountOfSmallestBlock = qwDynamicRAMSize / DYNAMICMEMORY_MIN_SIZE;

	dwSizeOfAllocatedBlockListIndex = lBlockCountOfSmallestBlock * sizeof(BYTE);

	dwSizeOfBitmap = 0;
	for (i = 0; (lBlockCountOfSmallestBlock >> i) > 0; i++)
	{
		dwSizeOfBitmap += sizeof(BITMAP);
		// round up
		dwSizeOfBitmap += ((lBlockCountOfSmallestBlock >> i) + 7) / 8;
	}

	// round up
	return (dwSizeOfAllocatedBlockListIndex + dwSizeOfBitmap + (DYNAMICMEMORY_MIN_SIZE - 1)) / DYNAMICMEMORY_MIN_SIZE;
}

void* kAllocateMemory(QWORD qwSize)
{
	QWORD qwAlignedSize;
	QWORD qwRelativeAddress;
	long lOffset;
	int iSizeArrayOffset;
	int iIndexOfBlockList;

	qwAlignedSize = kGetBuddyBlockSize(qwSize);
	if (qwAlignedSize == 0)
		return NULL;

	// Not enough memory
	if (gs_stDynamicMemory.qwStartAddress + gs_stDynamicMemory.qwUsedSize + qwAlignedSize >
		gs_stDynamicMemory.qwEndAddress)
		return NULL;

	lOffset = kAllocationBuddyBlock(qwAlignedSize);
	if (lOffset == -1)
		return NULL;

	iIndexOfBlockList = kGetBlockListIndexOfMatchSize(qwAlignedSize);

	qwRelativeAddress = qwAlignedSize * lOffset;
	iSizeArrayOffset = qwRelativeAddress / DYNAMICMEMORY_MIN_SIZE;
	gs_stDynamicMemory.pbAllocatedBlockListIndex[iSizeArrayOffset] = (BYTE)iIndexOfBlockList;
	gs_stDynamicMemory.qwUsedSize += qwAlignedSize;

	return (void*)(qwRelativeAddress + gs_stDynamicMemory.qwStartAddress);
}

static QWORD kGetBuddyBlockSize(QWORD qwSize)
{
	long i;

	for (i = 0; i < gs_stDynamicMemory.iMaxLevelCount; i++)
	{
		if (qwSize <= (DYNAMICMEMORY_MIN_SIZE << i))
			return (DYNAMICMEMORY_MIN_SIZE << i);
	}

	return 0;
}

static int kAllocationBuddyBlock(QWORD qwAlignedSize)
{
	int iBlockListIndex, iFreeOffset;
	int i;
	BOOL bPreviousInterruptFlag;

	iBlockListIndex = kGetBlockListIndexOfMatchSize(qwAlignedSize);
	if (iBlockListIndex == -1)
		return -1;

	bPreviousInterruptFlag = kLockForSystemData();
	{ // C.S.

		for (i = iBlockListIndex; i < gs_stDynamicMemory.iMaxLevelCount; i++)
		{
			iFreeOffset = kFindFreeBlockInBitmap(i);
			if (iFreeOffset != -1)
				break;
		}

		// Failed to find free block -> return -1
		if (iFreeOffset == -1)
		{
			// C.S. end
			kUnlockForSystemData(bPreviousInterruptFlag);
			return -1;
		}

		kSetFlagInBitmap(i, iFreeOffset, DYNAMICMEMORY_EMPTY);

		// Found a free block on the bigger (than the qwAlignedSize) block list
		// We must split the bigger block
		if (i > iBlockListIndex)
		{
			for (i = i - 1; i >= iBlockListIndex; i--)
			{
				// Left one: empty
				kSetFlagInBitmap(i, iFreeOffset * 2, DYNAMICMEMORY_EMPTY);

				// Right one: exist
				kSetFlagInBitmap(i, iFreeOffset * 2 + 1, DYNAMICMEMORY_EXIST);

				// Split the letft one again
				iFreeOffset *= 2;
			}
		}


	} // C.S. end
	kUnlockForSystemData(bPreviousInterruptFlag);

	return iFreeOffset;
}

static int kGetBlockListIndexOfMatchSize(QWORD qwAlignedSize)
{
	int i;

	for (i = 0; i < gs_stDynamicMemory.iMaxLevelCount; i++)
	{
		if (qwAlignedSize <= (DYNAMICMEMORY_MIN_SIZE << i))
			return i;
	}

	return -1;
}

static int kFindFreeBlockInBitmap(int iBlockListIndex)
{
	int i, iMaxCount;
	BYTE *pbBitmap;
	QWORD *pqwBitmap;

	if (gs_stDynamicMemory.pstBitmapOfLevel[iBlockListIndex].qwExistBitCount == 0)
		return -1;

	iMaxCount = gs_stDynamicMemory.iBlockCountOfSmallestBlock >> iBlockListIndex;
	pbBitmap = gs_stDynamicMemory.pstBitmapOfLevel[iBlockListIndex].pbBitmap;
	for (i = 0; i < iMaxCount; )
	{
		//if ((iMaxCount - i) / 64 > 0)
		if ((iMaxCount - i) >= 64)
		{
			pqwBitmap = (QWORD*) &pbBitmap[i / 8];
			if (*pqwBitmap == 0)
			{
				i += 64;
				continue;
			}
		}

		if ((pbBitmap[i / 8] & (DYNAMICMEMORY_EXIST << (i % 8))) != 0)
			return i;

		i++;
	}

	return -1;
}

static void kSetFlagInBitmap(int iBlockListIndex, int iOffset, BYTE bFlag)
{
	BYTE *pbBitmap;

	pbBitmap = gs_stDynamicMemory.pstBitmapOfLevel[iBlockListIndex].pbBitmap;
	if (bFlag == DYNAMICMEMORY_EXIST)
	{
		if ((pbBitmap[iOffset / 8] & (0x01 << (iOffset % 8))) == 0)
		{
			gs_stDynamicMemory.pstBitmapOfLevel[iBlockListIndex].qwExistBitCount++;
			pbBitmap[iOffset / 8] |= (0x01 << (iOffset % 8));
		}
		//pbBitmap[iOffset / 8] |= (0x01 << (iOffset % 8));
	}
	else
	{
		if ((pbBitmap[iOffset / 8] & (0x01 << (iOffset % 8))) != 0)
		{
			gs_stDynamicMemory.pstBitmapOfLevel[iBlockListIndex].qwExistBitCount--;
			pbBitmap[iOffset / 8] &= ~(0x01 << (iOffset % 8));
		}
		//pbBitmap[iOffset / 8] &= ~(0x01 << (iOffset % 8));
	}
}

BOOL kFreeMemory(void *pvAddress)
{
	QWORD qwRelativeAddress;
	int iSizeArrayOffset;
	QWORD qwBlockSize;
	int iBlockListIndex;
	int iBitmapOffset;

	if (pvAddress == NULL)
		return FALSE;

	qwRelativeAddress = ((QWORD)pvAddress) - gs_stDynamicMemory.qwStartAddress;
	iSizeArrayOffset = qwRelativeAddress / DYNAMICMEMORY_MIN_SIZE;

	if (gs_stDynamicMemory.pbAllocatedBlockListIndex[iSizeArrayOffset] == 0xFF)
		return FALSE; // Not allocated

	iBlockListIndex = (int)gs_stDynamicMemory.pbAllocatedBlockListIndex[iSizeArrayOffset];
	gs_stDynamicMemory.pbAllocatedBlockListIndex[iSizeArrayOffset] = 0xFF;

	qwBlockSize = DYNAMICMEMORY_MIN_SIZE << iBlockListIndex;

	iBitmapOffset = qwRelativeAddress / qwBlockSize;
	if (kFreeBuddyBlock(iBlockListIndex, iBitmapOffset))
	{
		gs_stDynamicMemory.qwUsedSize -= qwBLockSize;
		return TRUE;
	}

	return FALSE;
}

static BOOL kFreeBuddyBlock(int iBlockListIndex, int iBlockOffset)
{
	int iBuddyBlockOffset;
	int i;
	BOOL bFlag;
	BOOL bPreviousInterruptFlag;

	bPreviousInterruptFlag = kLockForSystemData();
	{ // C.S.
		for (i = iBlockListIndex; i < gs_stDynamicMemory.iMaxLevelCount; i++)
		{
			kSetFlagInBitmap(i, iBlockOffset, DYNAMICMEMORY_EXIST);

//			if (iBlockOffset & 1)
//				iBuddyBlockOffset = iBlockOffset - 1;
//			else
//				iBuddyBlockOffset = iBlockOffset + 1;
			iBuddyBlockOffset = iBlockOffset ^ 1;

			bFlag = kGetFlagInBitmap(i, iBuddyBlockOffset);
			if (bFlag == DYNAMICMEMORY_EMPTY)
				break; // Buddy block is in use

			kSetFlagInBitmap(i, iBuddyBlockOffset, DYNAMICMEMORY_EMPTY);
			kSetFlagInBitmap(i, iBlockOffset, DYNAMICMEMORY_EMPTY);

			iBlockOffset /= 2;
		}
	} // C.S. end
	kUnlockForSystemData(bPreviousInterruptFlag);

	return TRUE;
}

static BYTE kGetFlagInBitmap(int iBlockListIndex, int iOffset)
{
	BYTE *pbBitmap;

	pbBitmap = gs_stDynamicMemory.pstBitmapOfLevel[iBlockListIndex].pbBitmap;
	return !!(pbBitmap[iOffset / 8] & (0x01 << (iOffset % 8)));

//	if (pbBitmap[iOffset / 8] & (0x01 << (iOffset % 8)))
//		return DYNAMICMEMORY_EXIST;
//
//	return DYNAMICMEMORY_EMPTY;
}

void kGetDynamicMemoryInformation(QWORD *pqwDynamicMemoryStartAddress, QWORD *pqwDynamicMemoryTotalSize,
	QWORD *pqwMetaDataSize, QWORD *pqwUsedMemorySize)
{
	*pqwDynamicMemoryStartAddress = DYNAMICMEMORY_START_ADDRESS;
	*pqwDynamicMemoryTotalSize = kCalculateDynamicMemorySize();
	*pqwMetaDataSize = kCalculateMetaBlockCount(*pqwDynamicMemoryTotalSize) * DYNAMICMEMORY_MIN_SIZE;
	*pqwUsedMemorySize = gs_stDynamicMemory.qwUsedSize;
}

DYNAMICMEMORY* kGetDynamicMemoryManager()
{
	return &gs_stDynamicMemory;
}
