
#include "Task.h"
#include "Descriptor.h"
#include "Utility.h"
#include "AssemblyUtility.h"


//static SCHEDULER gs_stScheduler;
SCHEDULER gs_stScheduler;
static TCBPOOLMANAGER gs_stTCBPoolManager;


////////////////////////////////////////////////////////////////////////////////
// Task Pool & Task
////////////////////////////////////////////////////////////////////////////////

void kInitializeTCBPool()
{
	int i;

	kMemSet(&gs_stTCBPoolManager, 0, sizeof(gs_stTCBPoolManager));

	// Initialize TCB Pool & Assign it to the TCBPoolManager
	kMemSet((void*)TASK_TCBPOOLADDRESS, 0, sizeof(TCB) * TASK_MAXCOUNT);
	gs_stTCBPoolManager.pstStartAddress = (TCB*)TASK_TCBPOOLADDRESS;

	for (i = 0; i < TASK_MAXCOUNT; i++)
		gs_stTCBPoolManager.pstStartAddress[i].stLink.qwID = i;

	gs_stTCBPoolManager.iMaxCount = TASK_MAXCOUNT;
	gs_stTCBPoolManager.iAllocatedCount = 1;
}

TCB* kAllocateTCB()
{
	TCB *pstEmptyTCB;
	int i;

	// TCB Pool is full
	if (gs_stTCBPoolManager.iUseCount == gs_stTCBPoolManager.iMaxCount)
		return NULL;

	for (i = 0; i < gs_stTCBPoolManager.iMaxCount; i++)
	{
		if ((gs_stTCBPoolManager.pstStartAddress[i].stLink.qwID >> 32) == 0)
		{
			pstEmptyTCB = gs_stTCBPoolManager.pstStartAddress + i;
			break;
		}
	}

	pstEmptyTCB->stLink.qwID = ((QWORD)gs_stTCBPoolManager.iAllocatedCount << 32) | i;
	gs_stTCBPoolManager.iUseCount++;
	gs_stTCBPoolManager.iAllocatedCount++;
	// iAllocatedCount should never be the zero
	if (gs_stTCBPoolManager.iAllocatedCount == 0)
		gs_stTCBPoolManager.iAllocatedCount = 1;

	return pstEmptyTCB;
}

void kFreeTCB(QWORD qwID)
{
	int i;

	i = qwID & 0xFFFFFFFF;

	kMemSet(&(gs_stTCBPoolManager.pstStartAddress[i].stContext), 0, sizeof(CONTEXT));
	gs_stTCBPoolManager.pstStartAddress[i].stLink.qwID = i;

	gs_stTCBPoolManager.iUseCount--;
}

TCB* kCreateTask(QWORD qwFlags, QWORD qwEntryPointAddress)
{
	TCB *pstTask;
	void *pvStackAddress;

	pstTask = kAllocateTCB();
	if (pstTask == NULL)
		return NULL;

	// Calculate the stack address of the allocated TCB
	pvStackAddress = (void*)(TASK_STACKPOOLADDRESS + (TASK_STACKSIZE * (pstTask->stLink.qwID & 0xFFFFFFFF)));

	// Create a task and push it to the ready list
	kSetUpTask(pstTask, qwFlags, qwEntryPointAddress, pvStackAddress, TASK_STACKSIZE);
	kAddTaskToReadyList(pstTask);

	return pstTask;
}

void kSetUpTask(TCB *pstTCB, QWORD qwFlags, QWORD qwEntryPointAddress, void *pvStackAddress, QWORD qwStackSize)
{
	kMemSet(pstTCB->stContext.vqRegister, 0, sizeof(pstTCB->stContext.vqRegister));

	// Registers related to the stack
	pstTCB->stContext.vqRegister[TASK_RSPOFFSET] = (QWORD)pvStackAddress + qwStackSize;
	pstTCB->stContext.vqRegister[TASK_RBPOFFSET] = (QWORD)pvStackAddress + qwStackSize;

	// Segment selectors
	pstTCB->stContext.vqRegister[TASK_CSOFFSET] = GDT_KERNELCODESEGMENT;
	pstTCB->stContext.vqRegister[TASK_DSOFFSET] = GDT_KERNELDATASEGMENT;
	pstTCB->stContext.vqRegister[TASK_ESOFFSET] = GDT_KERNELDATASEGMENT;
	pstTCB->stContext.vqRegister[TASK_FSOFFSET] = GDT_KERNELDATASEGMENT;
	pstTCB->stContext.vqRegister[TASK_GSOFFSET] = GDT_KERNELDATASEGMENT;
	pstTCB->stContext.vqRegister[TASK_SSOFFSET] = GDT_KERNELDATASEGMENT;

	// RIP
	pstTCB->stContext.vqRegister[TASK_RIPOFFSET] = qwEntryPointAddress;

	// Enable IF flag (bit 9) in the RFLAGS register
	pstTCB->stContext.vqRegister[TASK_RFLAGSOFFSET] |= 0x0200;

	// Other vars
	pstTCB->pvStackAddress = pvStackAddress;
	pstTCB->qwStackSize = qwStackSize;
	pstTCB->qwFlags = qwFlags;
}


////////////////////////////////////////////////////////////////////////////////
// Scheduler
////////////////////////////////////////////////////////////////////////////////

void kInitializeScheduler()
{
	kInitializeTCBPool();

	kInitializeList(&gs_stScheduler.stReadyList);

	gs_stScheduler.pstRunningTask = kAllocateTCB();
}

void kSetRunningTask(TCB *pstTask)
{
	gs_stScheduler.pstRunningTask = pstTask;
}

TCB* kGetRunningTask()
{
	return gs_stScheduler.pstRunningTask;
}

TCB *kGetNextTaskToRun()
{
	if (kGetListCount(&gs_stScheduler.stReadyList) == 0)
		return NULL;

	return (TCB*)kRemoveListFromHeader(&(gs_stScheduler.stReadyList));
}

void kAddTaskToReadyList(TCB *pstTask)
{
	kAddListToTail(&gs_stScheduler.stReadyList, pstTask);
}

void kSchedule()
{
	TCB *pstRunningTask, *pstNextTask;
	BOOL bPreviousFlag;

	// If there is no other runnable task -> just return
	if (kGetListCount(&gs_stScheduler.stReadyList) == 0)
		return;

	// bPreviousFlag is the IF flag of the pstRunningTask
	bPreviousFlag = kSetInterruptFlag(FALSE);

	pstNextTask = kGetNextTaskToRun();
	if (pstNextTask == NULL)
	{
		// Restore the interrupt flag and just return
		kSetInterruptFlag(bPreviousFlag);
		return;
	}

	// Add the running task to the runnable task list
	pstRunningTask = gs_stScheduler.pstRunningTask;
	kAddTaskToReadyList(pstRunningTask);

	// Update the processor time for the new task
	gs_stScheduler.iProcessorTime = TASK_PROCESSORTIME;

	// Set the next task as a running task and do context-switching
	gs_stScheduler.pstRunningTask = pstNextTask;
	kSwitchContext(&pstRunningTask->stContext, &pstNextTask->stContext);

	// Restore the previously saved interrupt flag
	// bPreviousFlag is the IF flag of the pstNextTask
	kSetInterruptFlag(bPreviousFlag);
}

BOOL kScheduleInInterrupt()
{
	TCB *pstRunningTask, *pstNextTask;
	char *pcContextAddress;

	pstNextTask = kGetNextTaskToRun();
	if (pstNextTask == NULL)
		return FALSE;

	pcContextAddress = (char*)IST_STARTADDRESS + IST_SIZE - sizeof(CONTEXT);

	// Save currently running tasks' context from IST and add it to the run-list
	pstRunningTask = gs_stScheduler.pstRunningTask;
	kMemCpy(&pstRunningTask->stContext, pcContextAddress, sizeof(CONTEXT));
	kAddTaskToReadyList(pstRunningTask);

	// Set NextTask as a running task
	// Change the saved context in the IST to the NextTask's context
	gs_stScheduler.pstRunningTask = pstNextTask;
	kMemCpy(pcContextAddress, &pstNextTask->stContext, sizeof(CONTEXT));

	// Update the processor time for the new task
	gs_stScheduler.iProcessorTime = TASK_PROCESSORTIME;
	return TRUE;
}

void kDecreaseProcessorTime()
{
	if (gs_stScheduler.iProcessorTime > 0)
		gs_stScheduler.iProcessorTime--;
}

BOOL kIsProcessorTimeExpired()
{
	return (gs_stScheduler.iProcessorTime <= 0);
}
