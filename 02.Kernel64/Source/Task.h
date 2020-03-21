
#ifndef __TASK_H__
#define __TASK_H__

#include "Types.h"
#include "List.h"


//  5 Registers: SS, RSP, RFLAGS, CS, RIP
// 19 Registers: Other registers saved in the ISR
#define TASK_REGISTERCOUNT	(5 + 19)
#define TASK_REGISTERSIZE	8

#define TASK_GSOFFSET		0
#define TASK_FSOFFSET		1
#define TASK_ESOFFSET		2
#define TASK_DSOFFSET		3
#define TASK_R15OFFSET		4
#define TASK_R14OFFSET		5
#define TASK_R13OFFSET		6
#define TASK_R12OFFSET		7
#define TASK_R11OFFSET		8
#define TASK_R10OFFSET		9
#define TASK_R9OFFSET		10
#define TASK_R8OFFSET		11
#define TASK_RSIOFFSET		12
#define TASK_RDIOFFSET		13
#define TASK_RDXOFFSET		14
#define TASK_RCXOFFSET		15
#define TASK_RBXOFFSET		16
#define TASK_RAXOFFSET		17
#define TASK_RBPOFFSET		18
#define TASK_RIPOFFSET		19
#define TASK_CSOFFSET		20
#define TASK_RFLAGSOFFSET	21
#define TASK_RSPOFFSET		22
#define TASK_SSOFFSET		23


// Address of the Task Pool
#define TASK_TCBPOOLADDRESS		0x800000
#define TASK_MAXCOUNT			1024

// Address of the Stack Pool
#define TASK_STACKPOOLADDRESS	(TASK_TCBPOOLADDRESS + sizeof(TCB) * TASK_MAXCOUNT)
// Size of the stack
#define TASK_STACKSIZE			8192

// Invalid task ID
#define TASK_INVALIDID			0xFFFFFFFFFFFFFFFFL

// Maximum time available for a single task in ms
#define TASK_PROCESSORTIME		5

// Maximum number of lists
#define TASK_MAXREADYLISTCOUNT	5


// Task Flags

// Priority val of the tasks
#define TASK_FLAGS_HIGHEST		0
#define TASK_FLAGS_HIGH			1
#define TASK_FLAGS_MEDIUM		2
#define TASK_FLAGS_LOW			3
#define TASK_FLAGS_LOWEST		4
#define TASK_FLAGS_WAIT			0xFF

#define TASK_FLAGS_ENDTASK		0x8000000000000000
#define TASK_FLAGS_SYSTEM		0x4000000000000000
#define TASK_FLAGS_PROCESS		0x2000000000000000
#define TASK_FLAGS_THREAD		0x1000000000000000
#define TASK_FLAGS_IDLE			0x0800000000000000

#define GETPRIORITY(x)			((x) & 0xFF)
#define SETPRIORITY(x, prior)	((x) = ((x) & 0xFFFFFFFFFFFFFF00) | (prior))
#define GETTCBOFFSET(x)			((x) & 0xFFFFFFFF)

#define GETTCBFROMTHREADLINK(x)	(TCB*)((QWORD)(x) - offsetof(TCB, stThreadLink))


#pragma pack(push, 1)
typedef struct kContextStruct
{
	QWORD vqRegister[TASK_REGISTERCOUNT];
} CONTEXT;

typedef struct kTaskControlBlockStruct
{
	// Link structure including qwID
	LISTLINK stLink;

	QWORD qwFlags;

	void *pvMemoryAddress;
	QWORD qwMemorySize;

	//////////////////////////////////////
	// Member variables for the Thread
	//////////////////////////////////////
	LISTLINK stThreadLink;

	QWORD qwParentProcessID;

	// vqwFPUContext must be 16bytes aligned
	QWORD vqwFPUContext[512 / 8];
	CONTEXT stContext;

	// List of the child threads
	LIST stChildThreadList;


	void *pvStackAddress;
	QWORD qwStackSize;

	BOOL bFPUUsed;

	char vcPadding[11]; // For 16bytes align
} TCB;

typedef struct kTCBPoolManagerStruct
{
	TCB *pstStartAddress;
	int iMaxCount;
	int iUseCount;

	int iAllocatedCount;
} TCBPOOLMANAGER;

typedef struct kSchedulerStruct
{
	// Currently running task
	TCB *pstRunningTask;

	// Time slice
	int iProcessorTime;

	// List of all runnable tasks
	LIST vstReadyList[TASK_MAXREADYLISTCOUNT];
	// List of all gonna-exit tasks
	LIST stWaitList;

	// Record the task-execution-count of all runnable lists
	int viExecuteCount[TASK_MAXREADYLISTCOUNT];

	QWORD qwLastFPUUsedTaskID;

	// Vars to calculate CPU load
	QWORD qwProcessorLoad;
	QWORD qwSpendProcessorTimeInIdleTask;

	QWORD qwTaskSwitchCount;
} SCHEDULER;
#pragma pop


////////////////////////////////////////////////////////////////////////////////
// Task Pool & Task
////////////////////////////////////////////////////////////////////////////////
static void kInitializeTCBPool();
static TCB* kAllocateTCB();
static void kFreeTCB(QWORD qwID);
TCB* kCreateTask(QWORD qwFlags, void *pvMemoryAddress, QWORD qwMemorySize, QWORD qwEntryPointAddress);
static void kSetUpTask(TCB *pstTCB, QWORD qwFlags, QWORD qwEntryPointAddress, void *pvStackAddress, QWORD qwStackSize);

////////////////////////////////////////////////////////////////////////////////
// Scheduler
////////////////////////////////////////////////////////////////////////////////
void kInitializeScheduler();
void kSetRunningTask(TCB *pstTask);
TCB* kGetRunningTask();
static TCB *kGetNextTaskToRun();
static BOOL kAddTaskToReadyList(TCB *pstTask);
static TCB* kRemoveTaskFromReadyList(QWORD qwTaskID);
BOOL kChangePriority(QWORD qwTaskID, BYTE bPriority);
void kSchedule();
BOOL kScheduleInInterrupt();
void kDecreaseProcessorTime();
BOOL kIsProcessorTimeExpired();
BOOL kEndTask(QWORD qwTaskID);
void kExitTask();
int kGetReadyTaskCount();
int kGetTaskCount();
TCB* kGetTCBInTCBPool(int iOffset);
BOOL kIsTaskExist(QWORD qwID);
QWORD kGetProcessorLoad();
static TCB *kGetProcessByThread(TCB *pstThread);

////////////////////////////////////////////////////////////////////////////////
// Idle Tasks
////////////////////////////////////////////////////////////////////////////////
void kIdleTask();
void kHaltProcessorByLoad();

////////////////////////////////////////////////////////////////////////////////
// Idle Tasks
////////////////////////////////////////////////////////////////////////////////
QWORD kGetLastFPUUsedTaskID();
void kSetLastFPUUsedTaskID(QWORD qwTaskID);


#endif /*__TASK_H__*/
