
#include "Types.h"


void kPrintString(int iX, int iY, const char *pcString);


void Main()
{
	kPrintString(0, 10, "Switch To IA-32e mode success");
	kPrintString(0, 11, "IA-32e C Language Kernel Start..............................[Pass]");

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
