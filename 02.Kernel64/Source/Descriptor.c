
#include "Descriptor.h"
#include "Utility.h"
#include "ISR.h"


void kInitializeGDTTableAndTSS()
{
	GDTR *pstGDTR;
	GDTENTRY8 *pstEntry;
	TSSSEGMENT *pstTSS;
	int i;

	pstGDTR = (GDTR*)GDTR_STARTADDRESS;
	pstEntry = (GDTENTRY8*)(GDTR_STARTADDRESS + sizeof(GDTR));

	// (NULL + Kernel Code + Kernel Data) + TSS
	pstGDTR->wLimit = GDT_TABLESIZE - 1;
	pstGDTR->qwBaseAddress = (QWORD)pstEntry;

	pstTSS = (TSSSEGMENT*)((QWORD)pstEntry + GDT_TABLESIZE);

	kSetGDTEntry8(pstEntry + 0, 0, 0, 0, 0, 0);
	kSetGDTEntry8(pstEntry + 1, 0, 0xFFFF, GDT_FLAGS_UPPER_CODE, GDT_FLAGS_LOWER_KERNELCODE, GDT_TYPE_CODE);
	kSetGDTEntry8(pstEntry + 2, 0, 0xFFFF, GDT_FLAGS_UPPER_DATA, GDT_FLAGS_LOWER_KERNELDATA, GDT_TYPE_DATA);

	kSetGDTEntry16((GDTENTRY16*)(pstEntry + 3), (QWORD)pstTSS, sizeof(TSSSEGMENT) - 1, GDT_FLAGS_UPPER_TSS, GDT_FLAGS_LOWER_TSS, GDT_TYPE_TSS);

	kInitializeTSSSegment(pstTSS);
}

void kSetGDTEntry8(GDTENTRY8 *pstEntry, DWORD dwBaseAddress, DWORD dwLimit,
					BYTE bUpperFlags, BYTE bLowerFlags, BYTE bType)
{
	// Set Limit
	pstEntry->wLowerLimit = dwLimit & 0xFFFF;
	pstEntry->bUpperLimitAndUpperFlag = (dwLimit >> 16) & 0x0F;

	// Set BaseAddress
	pstEntry->wLowerBaseAddress = dwBaseAddress & 0xFFFF;
	pstEntry->bUpperBaseAddress1 = (dwBaseAddress >> 16) & 0xFF;
	pstEntry->bUpperBaseAddress2 = (dwBaseAddress >> 24) & 0xFF;

	// Set Type
	pstEntry->bTypeAndLowerFlag = (bType & 0x0F);

	// Set Lower Flag
	pstEntry->bTypeAndLowerFlag |= (bLowerFlags & 0xF0);

	// Set Upper Flag
	pstEntry->bUpperLimitAndUpperFlag |= (bUpperFlags & 0xF0);
}

void kSetGDTEntry16(GDTENTRY16 *pstEntry, QWORD qwBaseAddress, DWORD dwLimit,
					BYTE bUpperFlags, BYTE bLowerFlags, BYTE bType)
{
	// First 8 bytes are same
	kSetGDTEntry8((GDTENTRY8*)pstEntry, (DWORD)qwBaseAddress, dwLimit, bUpperFlags, bLowerFlags, bType);

	pstEntry->dwUpperBaseAddress = (DWORD)(qwBaseAddress >> 32);
	pstEntry->dwReserved = 0;
}

void kInitializeTSSSegment(TSSSEGMENT *pstTSS)
{
	kMemSet(pstTSS, 0, sizeof(TSSSEGMENT));
	pstTSS->qwIST[0] = IST_STARTADDRESS + IST_SIZE;

	// Disable I/O Map
	pstTSS->wIOMapBaseAddress = 0xFFFF;
}

void kSetIDTEntry(IDTENTRY *pstEntry, void *pvHandler, WORD wSelector, BYTE bIST, BYTE bFlags, BYTE bType)
{
	// Set BaseAddress
	pstEntry->wLowerBaseAddress = ((QWORD)pvHandler) & 0xFFFF;
	pstEntry->wMiddleBaseAddress = (((QWORD)pvHandler) >> 16) & 0xFFFF;
	pstEntry->dwUpperBaseAddress = (((QWORD)pvHandler) >> 32); // zero-extension

	// Set Segment Selector
	pstEntry->wSegmentSelector = wSelector;

	// Set IST
	pstEntry->bTypeAndFlags = bIST & 0x3;

	// Set Type and Flags
	pstEntry->bTypeAndFlags = (bType & 0x0F) | (bFlags & 0xF0);

	// Set Reserved
	pstEntry->dwReserved = 0;
}

void kInitializeIDTTables()
{
	IDTR *pstIDTR;
	IDTENTRY *pstEntry;
	int i;

	pstIDTR = (IDTR*)IDTR_STARTADDRESS;

	pstEntry = (IDTENTRY*)(IDTR_STARTADDRESS + sizeof(IDTR));
	pstIDTR->qwBaseAddress = (QWORD)pstEntry;
	pstIDTR->wLimit = IDT_TABLESIZE - 1; // Limit, not size


	kSetIDTEntry(pstEntry + 0, kISRDivideError, 0x08, IDT_FLAGS_IST1, IDT_FLAGS_KERNEL, IDT_TYPE_INTERRUPT);
	kSetIDTEntry(pstEntry + 1, kISRDebug, 0x08, IDT_FLAGS_IST1, IDT_FLAGS_KERNEL, IDT_TYPE_INTERRUPT);
	kSetIDTEntry(pstEntry + 2, kISRNMI, 0x08, IDT_FLAGS_IST1, IDT_FLAGS_KERNEL, IDT_TYPE_INTERRUPT);
	kSetIDTEntry(pstEntry + 3, kISRBreakPoint, 0x08, IDT_FLAGS_IST1, IDT_FLAGS_KERNEL, IDT_TYPE_INTERRUPT);
	kSetIDTEntry(pstEntry + 4, kISROverflow, 0x08, IDT_FLAGS_IST1, IDT_FLAGS_KERNEL, IDT_TYPE_INTERRUPT);
	kSetIDTEntry(pstEntry + 5, kISRBoundRangeExceeded, 0x08, IDT_FLAGS_IST1, IDT_FLAGS_KERNEL, IDT_TYPE_INTERRUPT);
	kSetIDTEntry(pstEntry + 6, kISRInvalidOpcode, 0x08, IDT_FLAGS_IST1, IDT_FLAGS_KERNEL, IDT_TYPE_INTERRUPT);
	kSetIDTEntry(pstEntry + 7, kISRDeviceNotAvailable, 0x08, IDT_FLAGS_IST1, IDT_FLAGS_KERNEL, IDT_TYPE_INTERRUPT);
	kSetIDTEntry(pstEntry + 8, kISRDoubleFault, 0x08, IDT_FLAGS_IST1, IDT_FLAGS_KERNEL, IDT_TYPE_INTERRUPT);
	kSetIDTEntry(pstEntry + 9, kISRCoprocessorSegmentOverrun, 0x08, IDT_FLAGS_IST1, IDT_FLAGS_KERNEL, IDT_TYPE_INTERRUPT);
	kSetIDTEntry(pstEntry + 10, kISRInvalidTSS, 0x08, IDT_FLAGS_IST1, IDT_FLAGS_KERNEL, IDT_TYPE_INTERRUPT);
	kSetIDTEntry(pstEntry + 11, kISRSegmentNotPresent, 0x08, IDT_FLAGS_IST1, IDT_FLAGS_KERNEL, IDT_TYPE_INTERRUPT);
	kSetIDTEntry(pstEntry + 12, kISRStackSegmentFault, 0x08, IDT_FLAGS_IST1, IDT_FLAGS_KERNEL, IDT_TYPE_INTERRUPT);
	kSetIDTEntry(pstEntry + 13, kISRGeneralProtection, 0x08, IDT_FLAGS_IST1, IDT_FLAGS_KERNEL, IDT_TYPE_INTERRUPT);
	kSetIDTEntry(pstEntry + 14, kISRPageFault, 0x08, IDT_FLAGS_IST1, IDT_FLAGS_KERNEL, IDT_TYPE_INTERRUPT);
	kSetIDTEntry(pstEntry + 15, kISR15, 0x08, IDT_FLAGS_IST1, IDT_FLAGS_KERNEL, IDT_TYPE_INTERRUPT);
	kSetIDTEntry(pstEntry + 16, kISRFPUError, 0x08, IDT_FLAGS_IST1, IDT_FLAGS_KERNEL, IDT_TYPE_INTERRUPT);
	kSetIDTEntry(pstEntry + 17, kISRAlignmentCheck, 0x08, IDT_FLAGS_IST1, IDT_FLAGS_KERNEL, IDT_TYPE_INTERRUPT);
	kSetIDTEntry(pstEntry + 18, kISRMachineCheck, 0x08, IDT_FLAGS_IST1, IDT_FLAGS_KERNEL, IDT_TYPE_INTERRUPT);
	kSetIDTEntry(pstEntry + 19, kISRSIMDError, 0x08, IDT_FLAGS_IST1, IDT_FLAGS_KERNEL, IDT_TYPE_INTERRUPT);
	kSetIDTEntry(pstEntry + 20, kISRETCException, 0x08, IDT_FLAGS_IST1, IDT_FLAGS_KERNEL, IDT_TYPE_INTERRUPT);

	for (i = 21; i < 32; i++)
		kSetIDTEntry(pstEntry + i, kISRETCException, 0x08, IDT_FLAGS_IST1, IDT_FLAGS_KERNEL, IDT_TYPE_INTERRUPT);

	kSetIDTEntry(pstEntry + 32, kISRTimer, 0x08, IDT_FLAGS_IST1, IDT_FLAGS_KERNEL, IDT_TYPE_INTERRUPT);
	kSetIDTEntry(pstEntry + 33, kISRKeyboard, 0x08, IDT_FLAGS_IST1, IDT_FLAGS_KERNEL, IDT_TYPE_INTERRUPT);
	kSetIDTEntry(pstEntry + 34, kISRSlavePIC, 0x08, IDT_FLAGS_IST1, IDT_FLAGS_KERNEL, IDT_TYPE_INTERRUPT);
	kSetIDTEntry(pstEntry + 35, kISRSerial2, 0x08, IDT_FLAGS_IST1, IDT_FLAGS_KERNEL, IDT_TYPE_INTERRUPT);
	kSetIDTEntry(pstEntry + 36, kISRSerial1, 0x08, IDT_FLAGS_IST1, IDT_FLAGS_KERNEL, IDT_TYPE_INTERRUPT);
	kSetIDTEntry(pstEntry + 37, kISRParallel2, 0x08, IDT_FLAGS_IST1, IDT_FLAGS_KERNEL, IDT_TYPE_INTERRUPT);
	kSetIDTEntry(pstEntry + 38, kISRFloppy, 0x08, IDT_FLAGS_IST1, IDT_FLAGS_KERNEL, IDT_TYPE_INTERRUPT);
	kSetIDTEntry(pstEntry + 39, kISRParallel1, 0x08, IDT_FLAGS_IST1, IDT_FLAGS_KERNEL, IDT_TYPE_INTERRUPT);
	kSetIDTEntry(pstEntry + 40, kISRRTC, 0x08, IDT_FLAGS_IST1, IDT_FLAGS_KERNEL, IDT_TYPE_INTERRUPT);
	kSetIDTEntry(pstEntry + 41, kISRReserved, 0x08, IDT_FLAGS_IST1, IDT_FLAGS_KERNEL, IDT_TYPE_INTERRUPT);
	kSetIDTEntry(pstEntry + 42, kISRNotUsed1, 0x08, IDT_FLAGS_IST1, IDT_FLAGS_KERNEL, IDT_TYPE_INTERRUPT);
	kSetIDTEntry(pstEntry + 43, kISRNotUsed2, 0x08, IDT_FLAGS_IST1, IDT_FLAGS_KERNEL, IDT_TYPE_INTERRUPT);
	kSetIDTEntry(pstEntry + 44, kISRMouse, 0x08, IDT_FLAGS_IST1, IDT_FLAGS_KERNEL, IDT_TYPE_INTERRUPT);
	kSetIDTEntry(pstEntry + 45, kISRCoprocessor, 0x08, IDT_FLAGS_IST1, IDT_FLAGS_KERNEL, IDT_TYPE_INTERRUPT);
	kSetIDTEntry(pstEntry + 46, kISRHDD1, 0x08, IDT_FLAGS_IST1, IDT_FLAGS_KERNEL, IDT_TYPE_INTERRUPT);
	kSetIDTEntry(pstEntry + 47, kISRHDD2, 0x08, IDT_FLAGS_IST1, IDT_FLAGS_KERNEL, IDT_TYPE_INTERRUPT);

	for (i = 48; i < IDT_MAXENTRYCOUNT; i++)
		kSetIDTEntry(pstEntry + i, kISRETCInterrupt, 0x08, IDT_FLAGS_IST1, IDT_FLAGS_KERNEL, IDT_TYPE_INTERRUPT);
}
