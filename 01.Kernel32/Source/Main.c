
#include "Types.h"
#include "Page.h"
#include "ModeSwitch.h"

void kPrintString(int iX, int iY, const char *pcString);
BOOL kInitializeKernel64Area();
BOOL kIsMemoryEnough();


void Main()
{
	char vcVendorString[13] = { 0 };
	DWORD dwTmp;

	kPrintString(0, 3, "C Language Kernel Start.....................................[Pass]");


	kPrintString(0, 4, "Minimum Memory Size Check...................................[    ]");
	if (!kIsMemoryEnough())
	{
		kPrintString(61, 4, "Fail");
		kPrintString(0, 5, "Not Enough Memory~! MINT64 OS requires over 64MB");
		goto InfiniteLoop;
	}
	else
		kPrintString(61, 4, "Pass");


	kPrintString(0, 5, "IA-32e Kernel Area Initialize...............................[    ]");
	if (!kInitializeKernel64Area())
	{
		kPrintString(61, 5, "Fail");
		kPrintString(0, 6, "IA-32e Kernel Area Initialization Failed");
		goto InfiniteLoop;
	}
	else
		kPrintString(61, 5, "Pass");

	
	kPrintString(0, 6, "IA-32e Page Table Initialize................................[    ]");
	kInitializePageTables();
	kPrintString(61, 6, "Pass");


	kReadCPUID(0x00, &dwTmp, (DWORD*)vcVendorString, (DWORD*)(vcVendorString + 8), (DWORD*)(vcVendorString + 4));
	kPrintString(0, 7, "Processor Vendor String.....................................[            ]");
	kPrintString(61, 7, vcVendorString);


	kReadCPUID(0x80000001, &dwTmp, &dwTmp, &dwTmp, &dwTmp); // We only need EDX, the last parameter, here
	kPrintString(0, 8, "64bit Mode Support Check....................................[    ]");
	if (!( dwTmp & (1 << 29) ))
	{
		kPrintString(61, 8, "Fail");
		kPrintString(0, 9, "This processor does not support 64bit mode");
		goto InfiniteLoop;
	}
	else
		kPrintString(61, 8, "Pass");


	kPrintString(0, 9, "Switch To IA-32e mode");
	//kSwitchAndExecute64bitKernel();


InfiniteLoop:
	while (1);
}

void kPrintString(int iX, int iY, const char *pcString)
{
	CHARACTER *pstScreen = (CHARACTER*)0xb8000;
	int i = 0;

	pstScreen += (iY * 80) + iX;
//	for (i = 0; pcString[i]; i++)
//		pstScreen[i].bCharactor = pcString[i];

	while (pcString[i])
		pstScreen[i].bCharactor = pcString[i], i++;
}

BOOL kInitializeKernel64Area()
{
	DWORD *pdwCurrentAddress;

	pdwCurrentAddress = (DWORD*) (1024 * 1024);

	while ((DWORD)pdwCurrentAddress < 6 * 1024 * 1024)
	{
		*pdwCurrentAddress = 0;

		if (*pdwCurrentAddress)
			return FALSE;

		pdwCurrentAddress++;
	}

	return TRUE;
}

BOOL kIsMemoryEnough()
{
	DWORD *pdwCurrentAddress;

	pdwCurrentAddress = (DWORD*) (1024 * 1024);

	while ((DWORD)pdwCurrentAddress < 64 * 1024 * 1024)
	{
		*pdwCurrentAddress = 0xdeadbeef;

		if (*pdwCurrentAddress != 0xdeadbeef)
			return FALSE;

		pdwCurrentAddress += 1024 * 1024 / sizeof(DWORD);
	}

	return TRUE;
}
