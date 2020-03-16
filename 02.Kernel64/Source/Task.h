
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


#pragma pack(push, 1)
typedef struct kContextStruct
{
	QWORD vqRegister[TASK_REGISTERCOUNT];
} CONTEXT;

typedef struct kTaskControlBlockStruct
{
	// Link structure including qwID
	LISTLINK stLink;

	CONTEXT stContext;

	QWORD qwFlags;

	void *pvStackAddress;
	QWORD qwStackSize;
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
	LIST stReadyList;
} SCHEDULER;
#pragma pop


////////////////////////////////////////////////////////////////////////////////
// Task Pool & Task
////////////////////////////////////////////////////////////////////////////////
void kInitializeTCBPool();
TCB* kAllocateTCB();
void kFreeTCB(QWORD qwID);
TCB* kCreateTask(QWORD qwFlags, QWORD qwEntryPointAddress);
void kSetUpTask(TCB *pstTCB, QWORD qwFlags, QWORD qwEntryPointAddress, void *pvStackAddress, QWORD qwStackSize);

////////////////////////////////////////////////////////////////////////////////
// Scheduler
////////////////////////////////////////////////////////////////////////////////
void kInitializeScheduler();
void kSetRunningTask(TCB *pstTask);
TCB* kGetRunningTask();
TCB *kGetNExtTaskToRun();
void kAddTaskToReadyList(TCB *pstTask);
void kSchedule();
BOOL kScheduleInInterrupt();
void kDecreaseProcessorTime();
BOOL kIsProcessorTimeExpired();

#endif /*__TASK_H__*/
