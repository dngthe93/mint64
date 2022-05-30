
#ifndef __DYNAMICMEMORY_H__
#define __DYNAMICMEMORY_H__

#include "Types.h"


// First-met 1MB-aligned address, after the Stack Pool
#define DYNAMICMEMORY_START_ADDRESS	((TASK_STACKPOOLADDRESS + \
	(TASK_STACKSIZE * TASK_MAXCOUNT) + 0xfffff) & 0xfffffffffff00000)


#define DYNAMICMEMORY_MIN_SIZE	(1 * 1024)

#define DYNAMICMEMORY_EXIST		0x01
#define DYNAMICMEMORY_EMPTY		0x02


typedef struct kBitmapStruct
{
	BYTE *pbBitmap;
	QWORD qwExistBitCount;
} BITMAP;

typedef struct kDynamicMemoryManagerStruct
{
	int iMaxLevelCount;
	int iBlockCountOfSmallestBlock;
	QWORD qwUsedSize;

	QWORD qwStartAddress;
	QWORD qwEndAddress;

	BYTE *pbAllocatedBlockListIndex;
	BITMAP *pstBitmapOfLevel;
} DYNAMICMEMORY;

void kInitializeDynamicMemory();
static QWORD kCalculateDynamicMemorySize();
static int kCalculateMetaBlockCount(QWORD qwDynamicRAMSize);
void* kAllocateMemory(QWORD qwSize);
static QWORD kGetBuddyBlockSize(QWORD qwSize);
static int kAllocationBuddyBlock(QWORD qwAlignedSize);
static int kGetBlockListIndexOfMatchSize(QWORD qwAlignedSize);
static int kFindFreeBlockInBitmap(int iBlockListIndex);
static void kSetFlagInBitmap(int iBlockListIndex, int iOffset, BYTE bFlag);
BOOL kFreeMemory(void *pvAddress);
static BOOL kFreeBuddyBlock(int iBlockListIndex, int iBlockOffset);
static BYTE kGetFlagInBitmap(int iBlockListIndex, int iOffset);
void kGetDynamicMemoryInformation(QWORD *pqwDynamicMemoryStartAddress, QWORD *pqwDynamicMemoryTotalSize,;
	QWORD *pqwMetaDataSize, QWORD *pqwUsedMemorySize);
DYNAMICMEMORY* kGetDynamicMemoryManager();

#endif /*__DYNAMICMEMORY_H__*/
