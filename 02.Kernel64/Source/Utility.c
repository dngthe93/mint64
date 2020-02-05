
#include "Utility.h"
#include "AssemblyUtility.h"

void kMemSet(void *pvDst, BYTE bData, unsigned int iSize)
{
	unsigned int i;
	for (i = 0; i < iSize; i++)
		((BYTE*)pvDst)[i] = bData;
}

int kMemCpy(void *pvDst, const void *pvSrc, unsigned int iSize)
{
	unsigned int i;
	for (i = 0; i < iSize; i++)
		((BYTE*)pvDst)[i] = ((BYTE*)pvSrc)[i];

	return (int)iSize;
}

int kMemCmp(const void *pvDst, const void *pvSrc, unsigned int iSize)
{
	unsigned int i;
	char cTemp;

	for (i = 0; i < iSize; i++)
	{
		cTemp = ((char*)pvDst)[i] - ((char*)pvSrc)[i];
		if (cTemp)
			return cTemp;
	}

	return 0;
}

BOOL kSetInterruptFlag(BOOL bEnableInterrupt)
{
	QWORD qwRFLAGS;

	qwRFLAGS = kReadRFLAGS();

	if (bEnableInterrupt)
		kEnableInterrupt();
	else
		kDisableInterrupt();

	// IF bit (bit 9)
	return !!(qwRFLAGS & 0x0200);
}
