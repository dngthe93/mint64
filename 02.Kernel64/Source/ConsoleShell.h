
#ifndef __CONSOLESHELL_H__
#define __CONSOLESHELL_H__

#include "Types.h"

#define CONSOLESHELL_MAXCOMMANDBUFFERCOUNT	300
#define CONSOLESHELL_PROMPTMESSAGE			"MINT64>"

typedef void (*CommandFunction) (const char *pcParameter);


#pragma pack(push, 1)
typedef struct kShellCommandEntryStruct
{
	// Command string
	char *pcCommand;
	// Help string for the command
	char *pcHelp;
	CommandFunction pfFunction;
} SHELLCOMMANDENTRY;

typedef struct kParameterListStruct
{
	const char *pcBuffer;
	int iLength;
	int iCurrentPosition;
} PARAMETERLIST;
#pragma pack(pop)


void kStartConsoleShell();
void kExecuteCommand(const char *pcCommandBuffer);
void kInitializeParameter(PARAMETERLIST *pstList, const char *pcParameter);
int kGetNextParameter(PARAMETERLIST *pstList, char *pcParameter);

static void kHelp(const char *pcParameterBuffer);
static void kCls(const char *pcParameterBuffer);
static void kShowTotalRAMSize(const char *pcParameterBuffer);
static void kStringToDecimalHexTest(const char *pcParameterBuffer);
static void kShutDown(const char *pcParameterBuffer);
static void kSetTimer(const char *pcParameterBuffer);
static void kWaitUsingPIT(const char *pcParameterBuffer);
static void kReadTimeStampCounter(const char *pcParameterBuffer);
static void kMeasureProcessorSpeed(const char *pcParameterBuffer);
static void kShowDateAndTime(const char *pcParameterBuffer);
static void kCreateTestTask(const char *pcParameterBuffer);
static void kChangeTaskPriority(const char *pcParameterBuffer);
static void kShowTaskList(const char *pcParameterBuffer);
static void kKillTask(const char *pcParameterBuffer);
static void kCPULoad(const char *pcParameterBuffer);
static void kTestMutex(const char *pcParameterBuffer);
static void kTestThread(const char *pcParameterBuffer);
static void kShowMatrix(const char *pcParameterBuffer);
static void kTestPIE(const char *pcParameterBuffer);


#endif /*__CONSOLESHELL_H__*/
