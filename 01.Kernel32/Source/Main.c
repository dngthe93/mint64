
#include "Types.h"

void kPrintString(int iX, int iY, const char *pcString);
BOOL kInitializeKernel64Area();


void Main()
{
	kPrintString(0, 3, "C Language Kernel Started~!!!");

	if (kInitializeKernel64Area())
		kPrintString(0, 4, "IA-32e Kernel Area Initialization Complete");
	else
		kPrintString(0, 4, "kInitializeKernel64Area() failed");

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
