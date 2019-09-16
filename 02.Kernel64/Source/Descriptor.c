
#include "Descriptor.h"
#include "Utility.h"


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


	for (i = 0; i < 100; i++)
		kSetIDTEntry(pstEntry + i, kDummyHandler, 0x08, IDT_FLAGS_IST1, IDT_FLAGS_KERNEL, IDT_TYPE_INTERRUPT);
}

void kPrintString(int iX, int iY, const char *pcString);
void kDummyHandler()
{
	kPrintString(0, 0, "==================================================");
	kPrintString(0, 1, "        Dummy Interrupt Handler Called~!!!        ");
	kPrintString(0, 2, "               Interrupt Occur~!!!                ");
	kPrintString(0, 3, "==================================================");

	while (1);
}
