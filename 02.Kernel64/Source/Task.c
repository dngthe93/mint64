
#include "Task.h"
#include "Descriptor.h"
#include "Utility.h"
#include "AssemblyUtility.h"
#include "Console.h"
#include "Synchronization.h"


//static SCHEDULER gs_stScheduler;
SCHEDULER gs_stScheduler;
static TCBPOOLMANAGER gs_stTCBPoolManager;


////////////////////////////////////////////////////////////////////////////////
// Task Pool & Task
////////////////////////////////////////////////////////////////////////////////

static void kInitializeTCBPool()
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

static TCB* kAllocateTCB()
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

static void kFreeTCB(QWORD qwID)
{
	int i;

	i = qwID & 0xFFFFFFFF;

	kMemSet(&(gs_stTCBPoolManager.pstStartAddress[i].stContext), 0, sizeof(CONTEXT));
	gs_stTCBPoolManager.pstStartAddress[i].stLink.qwID = i;

	gs_stTCBPoolManager.iUseCount--;
}

TCB* kCreateTask(QWORD qwFlags, void *pvMemoryAddress, QWORD qwMemorySize, QWORD qwEntryPointAddress)
{
	TCB *pstTask, *pstProcess;
	void *pvStackAddress;
	BOOL bPreviousFlag;

	bPreviousFlag = kLockForSystemData();
	{ // C.S.
		pstTask = kAllocateTCB();
		if (pstTask == NULL)
		{
			// C.S. end
			kUnlockForSystemData(bPreviousFlag);
			return NULL;
		}

		pstProcess = kGetProcessByThread(kGetRunningTask());
		if (pstProcess == NULL)
		{
			kFreeTCB(pstTask->stLink.qwID);

			// C.S. end
			kUnlockForSystemData(bPreviousFlag);
			return NULL;
		}

		if (qwFlags & TASK_FLAGS_THREAD)
		{ // Thread
			pstTask->qwParentProcessID = pstProcess->stLink.qwID;
			pstTask->pvMemoryAddress = pstProcess->pvMemoryAddress;
			pstTask->qwMemorySize = pstProcess->qwMemorySize;

			// Add this thread to the parent process' child-thread-link
			kAddListToTail(&pstProcess->stChildThreadList, &pstTask->stThreadLink);
		}
		else
		{ // Process
			pstTask->qwParentProcessID = pstProcess->stLink.qwID;
			pstTask->pvMemoryAddress = pvMemoryAddress;
			pstTask->qwMemorySize = qwMemorySize;
		}

		// Both of the LISTLINK's qwID must be same
		pstTask->stThreadLink.qwID = pstTask->stLink.qwID;
	}
	// FIXME: The lock should not be released until the end of this function maybe...
	kUnlockForSystemData(bPreviousFlag);

	// Calculate the stack address of the allocated TCB
	pvStackAddress = (void*)(TASK_STACKPOOLADDRESS + (TASK_STACKSIZE * (pstTask->stLink.qwID & 0xFFFFFFFF)));

	// Create a task and push it to the ready list
	kSetUpTask(pstTask, qwFlags, qwEntryPointAddress, pvStackAddress, TASK_STACKSIZE);

	kInitializeList(&pstTask->stChildThreadList);

	bPreviousFlag = kLockForSystemData();
	{ // C.S.
		kAddTaskToReadyList(pstTask);
	}
	kUnlockForSystemData(bPreviousFlag);

	return pstTask;
}

static void kSetUpTask(TCB *pstTCB, QWORD qwFlags, QWORD qwEntryPointAddress, void *pvStackAddress, QWORD qwStackSize)
{
	kMemSet(pstTCB->stContext.vqRegister, 0, sizeof(pstTCB->stContext.vqRegister));

	// Registers related to the stack
	pstTCB->stContext.vqRegister[TASK_RSPOFFSET] = (QWORD)pvStackAddress + qwStackSize - 8;
	pstTCB->stContext.vqRegister[TASK_RBPOFFSET] = (QWORD)pvStackAddress + qwStackSize - 8;
	*(QWORD*)((QWORD)pvStackAddress + qwStackSize - 8) = (QWORD)kExitTask;

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
	TCB *pstTask;
	int i;

	kInitializeTCBPool();

	// Initialize all runnable & wait lists
	for (i = 0; i < TASK_MAXREADYLISTCOUNT; i++)
	{
		kInitializeList(&(gs_stScheduler.vstReadyList[i]));
		gs_stScheduler.viExecuteCount[i] = 0;
	}
	kInitializeList(&gs_stScheduler.stWaitList);

	// Allocate a TCB for the current task and assign the higest priority to it
	// Curren task is the ConsoleShell called from the Main()
	pstTask = kAllocateTCB();
	gs_stScheduler.pstRunningTask = pstTask;
	pstTask->qwFlags = TASK_FLAGS_HIGHEST | TASK_FLAGS_PROCESS | TASK_FLAGS_SYSTEM;
	// Parent process of the initial process is the process itself
	pstTask->qwParentProcessID = pstTask->stLink.qwID;

	// 1 ~ 6 MB area: Kernel's data and code area
	pstTask->pvMemoryAddress = (void*)0x100000;
	pstTask->qwMemorySize = 0x500000;

	// 6 ~ 7 MB area: Kernel's stack area
	pstTask->pvStackAddress = (void*)0x600000;
	pstTask->qwStackSize = 0x100000;

	gs_stScheduler.qwTaskSwitchCount = 0;
	gs_stScheduler.qwProcessorLoad = 0;
}

void kSetRunningTask(TCB *pstTask)
{
	BOOL bPreviousFlag;

	bPreviousFlag = kLockForSystemData();
	{ // C.S.
		gs_stScheduler.pstRunningTask = pstTask;
	}
	kUnlockForSystemData(bPreviousFlag);
}

TCB* kGetRunningTask()
{
	BOOL bPreviousFlag;
	TCB *pstRunningTask;

	bPreviousFlag = kLockForSystemData();
	{ // C.S.
		pstRunningTask = gs_stScheduler.pstRunningTask;
	}
	kUnlockForSystemData(bPreviousFlag);

	return pstRunningTask;
}

static TCB *kGetNextTaskToRun()
{
	TCB *pstTarget = NULL;
	int iTaskCount, i, j;

	// If all tasks of the all ready-list has executed once, the inner for-loop with var 'i'
	//  will just end after clearing  all the execution count of the lists, without
	//  selecting the next task.
	// To prevent that kind of problem, we just simply add an outter for-loop with var 'j'.
	for (j = 0; j < 2; j++)
	{
		for (i = 0; i < TASK_MAXREADYLISTCOUNT; i++)
		{
			iTaskCount = kGetListCount(&(gs_stScheduler.vstReadyList[i]));

			if (gs_stScheduler.viExecuteCount[i] < iTaskCount)
			{
				// Pick the i-th priority list, vstReadyList[i]
				pstTarget = (TCB*)kRemoveListFromHeader(&(gs_stScheduler.vstReadyList[i]));

				// Increase the execution count of the i-th list
				gs_stScheduler.viExecuteCount[i]++;
				break;
			}
			else
			{
				// Go to the next (i+1)-th list,
				//  after clear the execution count of the i-th list
				gs_stScheduler.viExecuteCount[i] = 0;
			}
		}

		// If we found the next task to run, then just simply break
		if (pstTarget)
			break;
	}

	return pstTarget;
}

static BOOL kAddTaskToReadyList(TCB *pstTask)
{
	BYTE bPriority;

	bPriority = GETPRIORITY(pstTask->qwFlags);
	if (bPriority >= TASK_MAXREADYLISTCOUNT)
		return FALSE; // Wrong priority value

	kAddListToTail(&(gs_stScheduler.vstReadyList[bPriority]), pstTask);

	return TRUE;
}

static TCB* kRemoveTaskFromReadyList(QWORD qwTaskID)
{
	TCB *pstTarget;
	BYTE bPriority;

	if (GETTCBOFFSET(qwTaskID) >= TASK_MAXCOUNT)
		return NULL; // Wrong task id

	pstTarget = &(gs_stTCBPoolManager.pstStartAddress[GETTCBOFFSET(qwTaskID)]);
	if (pstTarget->stLink.qwID != qwTaskID)
		return NULL; // Wrong task id

	bPriority = GETPRIORITY(pstTarget->qwFlags);
	pstTarget = kRemoveList(&(gs_stScheduler.vstReadyList[bPriority]), qwTaskID);
	return pstTarget;
}

BOOL kChangePriority(QWORD qwTaskID, BYTE bPriority)
{
	TCB *pstTarget;
	BOOL bPreviousFlag;

	if (bPriority > TASK_MAXREADYLISTCOUNT)
		return FALSE; // Wrong priority value

	bPreviousFlag = kLockForSystemData();
	{ // C.S.
		pstTarget = gs_stScheduler.pstRunningTask;
		if (pstTarget->stLink.qwID == qwTaskID)
		{ // if qwTaskID is the ID of the current-running task

			// Just change the priority and wait.
			// After it uses all of the timeslice of itself,
			//  the scheduler will add it to the appropriate priority-run-list
			SETPRIORITY(pstTarget->qwFlags, bPriority);
		}
		else
		{
			pstTarget = kRemoveTaskFromReadyList(qwTaskID);
			if (pstTarget == NULL)
			{
				// if the target does not exist in the run-lists,
				//  find it on the TCB Pool and set the priority directly
				pstTarget = kGetTCBInTCBPool(GETTCBOFFSET(qwTaskID));
				// FIXME: pstTarget->stLink.qwID != qwTaskID case can be occured,
				//			because GETTCBOFFSET(x) does not check 'x', the input param value
				if (pstTarget != NULL)
					SETPRIORITY(pstTarget->qwFlags, bPriority);
			}
			else
			{ // Found the target in the run-lists

				// Set the priority and add the task to the run-lists
				SETPRIORITY(pstTarget->qwFlags, bPriority);
				kAddTaskToReadyList(pstTarget);
			}
		}
	} // C.S. end
	kUnlockForSystemData(bPreviousFlag);

	return TRUE;
}

void kSchedule()
{
	TCB *pstRunningTask, *pstNextTask;
	BOOL bPreviousFlag;

	// If there is no other runnable task -> just return
	if (kGetReadyTaskCount() < 1)
		return;

	// bPreviousFlag is the IF flag of the pstRunningTask
	bPreviousFlag = kLockForSystemData();
	{ // C.S.

		pstNextTask = kGetNextTaskToRun();
		if (pstNextTask == NULL)
		{
			// Restore the interrupt flag and just return
			// C.S. end
			kUnlockForSystemData(bPreviousFlag);
			return;
		}

		// Change the running task(in gs_stScheduler) as a NextTask
		pstRunningTask = gs_stScheduler.pstRunningTask;
		gs_stScheduler.pstRunningTask = pstNextTask;

		if ((pstRunningTask->qwFlags & TASK_FLAGS_IDLE) == TASK_FLAGS_IDLE)
		{ // If the running task is an IDLE task

			// Increase the TimeInIdleTask variable to calculate CPU load
			gs_stScheduler.qwSpendProcessorTimeInIdleTask +=
				TASK_PROCESSORTIME - gs_stScheduler.iProcessorTime;
		}

		// Update the processor time for the new task
		gs_stScheduler.iProcessorTime = TASK_PROCESSORTIME;

		if (pstRunningTask->qwFlags & TASK_FLAGS_ENDTASK)
		{ // If the running task is marked as a ENDTASK

			// Put it to the wait-list and context switch, w/o saving the current context
			kAddListToTail(&gs_stScheduler.stWaitList, pstRunningTask);
			kSwitchContext(NULL, &(pstNextTask->stContext));
		}
		else
		{
			// Add the running task to the runnable task list and do context-switching
			kAddTaskToReadyList(pstRunningTask);
			kSwitchContext(&pstRunningTask->stContext, &pstNextTask->stContext);
		}
	} // C.S. end
	// Restore the previously saved interrupt flag
	// bPreviousFlag is the IF flag of the pstNextTask
	kUnlockForSystemData(bPreviousFlag);
}

BOOL kScheduleInInterrupt()
{
	TCB *pstRunningTask, *pstNextTask;
	char *pcContextAddress;
	BOOL bPreviousFlag;

	bPreviousFlag = kLockForSystemData();
	{ // C.S.
		pstNextTask = kGetNextTaskToRun();
		if (pstNextTask == NULL)
		{
			// C.S. end
			kUnlockForSystemData(bPreviousFlag);
			return FALSE;
		}

		pcContextAddress = (char*)IST_STARTADDRESS + IST_SIZE - sizeof(CONTEXT);

		pstRunningTask = gs_stScheduler.pstRunningTask;
		gs_stScheduler.pstRunningTask = pstNextTask;

		if ((pstRunningTask->qwFlags & TASK_FLAGS_IDLE) == TASK_FLAGS_IDLE)
		{ // If the running task is an IDLE task

			// Increase the TimeInIdleTask variable to calculate CPU load
			gs_stScheduler.qwSpendProcessorTimeInIdleTask += TASK_PROCESSORTIME;
		}

		// Update the processor time for the new task
		gs_stScheduler.iProcessorTime = TASK_PROCESSORTIME;

		if (pstRunningTask->qwFlags & TASK_FLAGS_ENDTASK)
		{ // If the running task is marked as a ENDTASK

			// Put it to the wait-list
			kAddListToTail(&gs_stScheduler.stWaitList, pstRunningTask);
		}
		else
		{
			// Save currently running tasks' context from IST and add it to the run-list
			kMemCpy(&pstRunningTask->stContext, pcContextAddress, sizeof(CONTEXT));
			kAddTaskToReadyList(pstRunningTask);
		}
	} // C.S. end
	kUnlockForSystemData(bPreviousFlag);

	// Change the saved context in the IST, to the NextTask's context
	kMemCpy(pcContextAddress, &pstNextTask->stContext, sizeof(CONTEXT));

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

BOOL kEndTask(QWORD qwTaskID)
{
	TCB *pstTarget;
	BYTE bPriority;
	BOOL bPreviousFlag;

	bPreviousFlag = kLockForSystemData();
	{ // C.S.

		pstTarget = gs_stScheduler.pstRunningTask;
		if (pstTarget->stLink.qwID == qwTaskID)
		{ // If qwTaskID is running task

			// Set the MSB of the qwFlags (TASK_FLAGS_ENDTASK)
			pstTarget->qwFlags |= TASK_FLAGS_ENDTASK;

			// Set the priority to 0xFF (TASK_FLAGS_WAIT)
			SETPRIORITY(pstTarget->qwFlags, TASK_FLAGS_WAIT);

			// C.S. end
			kUnlockForSystemData(bPreviousFlag);

			// kSchedule() function will put the current task
			//  to the wait-list (by checking the qwFlags value)
			kSchedule();

			// This while-loop below SHOULD NEVER BE EXECUTED
			//  cause the current task has ended & switched
			kPrintf("\n\n***** OOPS!! Something's worng!\n");
			kPrintf("***** It can be happen when all the tasks have been killed");
			while (1);
		}
		else
		{
			// If qwTaskID is not a running-task.
			// It means that, current running task is trying to end the
			//  other task, which has the 'qwTaskID' as it's identifier

			pstTarget = kRemoveTaskFromReadyList(qwTaskID);
			if (pstTarget == NULL)
			{ // qwTaskID does not exists in the ready lists

				pstTarget = kGetTCBInTCBPool(GETTCBOFFSET(qwTaskID));
				if (pstTarget != NULL)
				{
					// FIXME: pstTarget->stLink.qwID != qwTaskID case can be occured,
					//			because GETTCBOFFSET(x) does not check 'x', the input param value

					pstTarget->qwFlags |= TASK_FLAGS_ENDTASK;
					SETPRIORITY(pstTarget->qwFlags, TASK_FLAGS_WAIT);
				}
				// C.S. end
				kUnlockForSystemData(bPreviousFlag);
				return TRUE;
				//return FALSE;
			}

			// If qwTaskID exists in the ready list, then set the flag to
			//  end-state-value, and add the task to the tail of the wait-list
			pstTarget->qwFlags |= TASK_FLAGS_ENDTASK;
			SETPRIORITY(pstTarget->qwFlags, TASK_FLAGS_WAIT);
			kAddListToTail(&(gs_stScheduler.stWaitList), pstTarget);
		}

	} // C.S. end
	kUnlockForSystemData(bPreviousFlag);
	return TRUE;
}

void kExitTask()
{
	// End the current task
	kEndTask(gs_stScheduler.pstRunningTask->stLink.qwID);
}

int kGetReadyTaskCount()
{
	int iTotalCount = 0;
	int i;
	BOOL bPreviousFlag;

	bPreviousFlag = kLockForSystemData();
	{ // C.S.

		for (i = 0; i < TASK_MAXREADYLISTCOUNT; i++)
			iTotalCount += kGetListCount(&(gs_stScheduler.vstReadyList[i]));
	}
	kUnlockForSystemData(bPreviousFlag);

	return iTotalCount;
}

int kGetTaskCount()
{
	int iTotalCount;
	BOOL bPreviousFlag;

	bPreviousFlag = kLockForSystemData();
	{ // C.S.

		// Ready task + Wait task + Running task
		iTotalCount = kGetReadyTaskCount();
		iTotalCount += kGetListCount(&gs_stScheduler.stWaitList) + 1;
	}
	kUnlockForSystemData(bPreviousFlag);

	return iTotalCount;
}

TCB* kGetTCBInTCBPool(int iOffset)
{
	if (0 <= iOffset && iOffset < TASK_MAXCOUNT)
		return &(gs_stTCBPoolManager.pstStartAddress[iOffset]);

	return NULL;
}

BOOL kIsTaskExist(QWORD qwID)
{
	TCB *pstTCB;

	pstTCB = kGetTCBInTCBPool(GETTCBOFFSET(qwID));

	// FIXME: (qwID >> 32) must be checked maybe...
	return (pstTCB && pstTCB->stLink.qwID == qwID);
}

QWORD kGetProcessorLoad()
{
	return gs_stScheduler.qwProcessorLoad;
}

static TCB *kGetProcessByThread(TCB *pstThread)
{
	TCB *pstProcess;

	if (pstThread->qwFlags & TASK_FLAGS_PROCESS)
		return pstThread;

	pstProcess = kGetTCBInTCBPool(GETTCBOFFSET(pstThread->qwParentProcessID));

	if (pstProcess == NULL || pstProcess->stLink.qwID != pstThread->qwParentProcessID)
		return NULL;

	return pstProcess;
}


////////////////////////////////////////////////////////////////////////////////
// Idle Tasks
////////////////////////////////////////////////////////////////////////////////

void kIdleTask()
{
	TCB *pstTask, *pstChildThread, *pstProcess;
	QWORD qwLastMeasureTickCount, qwLastSpendTickInIdleTask;
	QWORD qwCurrentMeasureTickCount, qwCurrentSpendTickInIdleTask;
	BOOL bPreviousFlag;
	int i, iCount;
	QWORD qwTaskID;
	void *pstThreadLink;

	qwLastMeasureTickCount = kGetTickCount();
	qwLastSpendTickInIdleTask = gs_stScheduler.qwSpendProcessorTimeInIdleTask;

	while (1)
	{
		qwCurrentMeasureTickCount = kGetTickCount();
		qwCurrentSpendTickInIdleTask = gs_stScheduler.qwSpendProcessorTimeInIdleTask;

		#define qwDiffOfMeasureTickCount (qwCurrentMeasureTickCount - qwLastMeasureTickCount)
		#define qwDiffOfSpendTickInIdleTask (qwCurrentSpendTickInIdleTask - qwLastSpendTickInIdleTask)
		if (qwDiffOfMeasureTickCount == 0)
		{
			// To avoid the Divide-by-Zero exception
			gs_stScheduler.qwProcessorLoad = 0;
		}
		else
		{
			gs_stScheduler.qwProcessorLoad = 100 - (qwDiffOfSpendTickInIdleTask / qwDiffOfMeasureTickCount) * 100;
		}
		#undef qwDiffOfMeasureTickCount
		#undef qwDiffOfSpendTickInIdleTask

		// Save the current count value
		qwLastMeasureTickCount = qwCurrentMeasureTickCount;
		qwLastSpendTickInIdleTask = qwCurrentSpendTickInIdleTask;


		// Halt according to the processor load
		kHaltProcessorByLoad();


		// Delete all the tasks in the wait list and free the allocated TCB
		if (kGetListCount(&gs_stScheduler.stWaitList) >= 0)
		{
			while (1)
			{
				bPreviousFlag = kLockForSystemData();
				{ // C.S.

					pstTask = kRemoveListFromHeader(&gs_stScheduler.stWaitList);
					if (pstTask == NULL)
					{ // C.S. end
						kUnlockForSystemData(bPreviousFlag);
						break;
					}

					if (pstTask->qwFlags & TASK_FLAGS_PROCESS)
					{
						// Call kEndTask() for all child threads
						pstThreadLink = kGetHeaderFromList(&pstTask->stChildThreadList);
						while (pstThreadLink)
						{
							//pstChildThread = GETTCBFROMTHREADLINK(pstThreadLink);
							//kEndTask(pstChildThread->stLink.qwID);
							kEndTask(((LISTLINK*)pstThreadLink)->qwID);
							pstThreadLink = kGetNextFromList(&pstTask->stChildThreadList, pstThreadLink);
						}



//						// Call kEndTask() for all child threads
//						iCount = kGetListCount(&pstTask->stChildThreadList);
//						for (i = 0; i < iCount; i++)
//						{
//							pstThreadLink = (TCB*)kRemoveListFromHeader(&pstTask->stChildThreadList);
//							if (pstThreadLink == NULL)
//								break;
//
//							pstChildThread = GETTCBFROMTHREADLINK(pstThreadLink);
//							kAddListToTail(&pstTask->stChildThreadList, &pstChildThread->stThreadLink);
//
//							kEndTask(pstChildThread->stLink.qwID);
//						}

						//if (iCount > 0)
						if (kGetListCount(&pstTask->stChildThreadList) > 0)
						{
							// Wait for the child thread to die
							kAddListToTail(&gs_stScheduler.stWaitList, pstTask);

							// C.S. end
							kUnlockForSystemData(bPreviousFlag);
							continue;
						}
						else
						{
							// Time to kill this process
						}
					}
					else if (pstTask->qwFlags & TASK_FLAGS_THREAD)
					{
						// Delete itself from the parent process' child-therad-list
						pstProcess = kGetProcessByThread(pstTask);
						if (pstProcess != NULL)
							kRemoveList(&pstProcess->stChildThreadList, pstTask->stLink.qwID);
					}

					qwTaskID = pstTask->stLink.qwID;
					kFreeTCB(qwTaskID);
				}
				kUnlockForSystemData(bPreviousFlag);

				kPrintf("IDLE: Task ID[0x%q] is completely ended.\n", qwTaskID);
			}
		}

		kSchedule();
	}
}

void kHaltProcessorByLoad()
{
	if (gs_stScheduler.qwProcessorLoad < 40)
	{
		kHlt();
		kHlt();
		kHlt();
	}
	else if (gs_stScheduler.qwProcessorLoad < 80)
	{
		kHlt();
		kHlt();
	}
	else if (gs_stScheduler.qwProcessorLoad < 95)
	{
		kHlt();
	}
}
