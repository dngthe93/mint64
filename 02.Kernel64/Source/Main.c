
#include "Types.h"
#include "Keyboard.h"
#include "Descriptor.h"
#include "AssemblyUtility.h"
#include "PIC.h"
#include "Console.h"
#include "ConsoleShell.h"
#include "Utility.h"
#include "PIT.h"


void Main()
{
	int iCursorX, iCursorY;

	// Initialize Console
	kInitializeConsole(0, 10);

	kPrintf("Switch To IA-32e mode success\n");
	kPrintf("IA-32e C Language Kernel Start..............................[Pass]\n");


	kGetCursor(&iCursorX, &iCursorY);
	kPrintf("GDT Initialize And Switch For IA-32e Mode...................[    ]");
	kInitializeGDTTableAndTSS();
	kLoadGDTR(GDTR_STARTADDRESS);
	kSetCursor(61, iCursorY++);
	kPrintf("Pass\n");


	kPrintf("TSS Segment Load............................................[    ]");
	kLoadTR(GDT_TSSSEGMENT);
	kSetCursor(61, iCursorY++);
	kPrintf("Pass\n");


	kPrintf("IDT Initialize..............................................[    ]");
	kInitializeIDTTables();
	kLoadIDTR(IDTR_STARTADDRESS);
	kSetCursor(61, iCursorY++);
	kPrintf("Pass\n");


	kPrintf("Total RAM Size Check........................................[    ]");
	kCheckTotalRAMSize();
	kSetCursor(61, iCursorY++);
	kPrintf("Pass], Size= %d MB\n", kGetTotalRAMSize());

	kPrintf("TCB Pool And Scheduler Initialize...........................[Pass]\n");
	iCursorY++;
	kInitializeScheduler();
	kInitializePIT(MSTOCOUNT(1), 1);

	kPrintf("Keyboard Activate And Queue Initialize......................[    ]");
	if (kInitializeKeyboard())
	{
		kSetCursor(61, iCursorY++);
		kPrintf("Pass\n");
		kChangeKeyboardLED(FALSE, FALSE, FALSE);
	}
	else
	{
		kSetCursor(61, iCursorY++);
		kPrintf("Fail\n");
		while (1);
	}

	kPrintf("PIC Controller And Interrupt Initialize.....................[    ]");
	kInitializePIC();
	kMaskPICInterrupt(0);
	kEnableInterrupt();
	kSetCursor(61, iCursorY++);
	kPrintf("Pass\n");


	// Start the Console Shell
	kStartConsoleShell();
}
