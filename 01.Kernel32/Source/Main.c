
#include "Types.h"

void kPrintString(int iX, int iY, const char *pcString);

void Main()
{
	kPrintString(0, 3, "C Language Kernel Started~!!!");

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
