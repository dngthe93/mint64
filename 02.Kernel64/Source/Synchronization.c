
#include "Synchronization.h"
#include "Task.h"
#include "AssemblyUtility.h"
#include "Utility.h"


BOOL kLockForSystemData()
{
	return kSetInterruptFlag(FALSE);
}

void kUnlockForSystemData(BOOL bInterruptFlag)
{
	kSetInterruptFlag(bInterruptFlag);
}

void kInitializeMutex(MUTEX *pstMutex)
{
	pstMutex->bLockFlag = FALSE;
	pstMutex->dwLockCount = 0;
	pstMutex->qwTaskID = TASK_INVALIDID;
}

void kLock(MUTEX *pstMutex)
{
	if (!kTestAndSet(&pstMutex->bLockFlag, 0, 1))
	{
		// Someone already got a mutex and it was me
		if (pstMutex->qwTaskID == kGetRunningTask()->stLink.qwID)
		{
			pstMutex->dwLockCount++;
			return;
		}

		// Wait for the unlock
		while (!kTestAndSet(&pstMutex->bLockFlag, 0, 1))
			kSchedule();
	}

	pstMutex->dwLockCount = 1;
	pstMutex->qwTaskID = kGetRunningTask()->stLink.qwID;
}

void kUnlock(MUTEX *pstMutex)
{
	if (!pstMutex->bLockFlag || pstMutex->qwTaskID != kGetRunningTask()->stLink.qwID)
		return;

	if (pstMutex->dwLockCount > 1)
	{
		pstMutex->dwLockCount--;
		return;
	}

	pstMutex->qwTaskID = TASK_INVALIDID;
	pstMutex->dwLockCount = 0;
	pstMutex->bLockFlag = FALSE;
}
