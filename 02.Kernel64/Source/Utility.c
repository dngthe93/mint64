
#include "Utility.h"
#include "AssemblyUtility.h"
#include <stdarg.h>

volatile QWORD g_qwTickCount = 0;

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

int kStrLen(const char *pcBuffer)
{
	int i = -1;

	while (pcBuffer[++i]);
	return i;
}

static QWORD gs_qwTotalRAMMBSize = 0;

// Check the RAM size, starting from 64MB
// It must be called from the booting procedure,
//  for the very first time
void kCheckTotalRAMSize()
{
	DWORD *pdwCurrentAddress;
	DWORD dwPreviousValue;

	// 64MB: 0x4000000
	pdwCurrentAddress = (DWORD*)0x4000000;

	while (1)
	{
		// Backup previous value
		dwPreviousValue = *pdwCurrentAddress;

		*pdwCurrentAddress = 0x12345678;
		if (*pdwCurrentAddress != 0x12345678)
			break; // If write operation doesn't work

		// Restore previous value
		*pdwCurrentAddress = dwPreviousValue;

		// Check next +1MB byte
		pdwCurrentAddress += (0x400000 / 4);
	}

	gs_qwTotalRAMMBSize = (QWORD)pdwCurrentAddress / 0x100000;
}

QWORD kGetTotalRAMSize()
{
	return gs_qwTotalRAMMBSize;
}

long kAToI(const char *pcBuffer, int iRadix)
{
	long lReturn;

	switch (iRadix)
	{
	case 16:
		lReturn = kHexStringToQword(pcBuffer);
		break;

	case 10:
	default:
		lReturn = kDecimalStringToLong(pcBuffer);
		break;
	}

	return lReturn;
}

QWORD kHexStringToQword(const char *pcBuffer)
{
	QWORD qwValue = 0;
	int i;

	for (i = 0; pcBuffer[i]; i++)
	{
		qwValue *= 16;

		if ('A' <= pcBuffer[i] &&  pcBuffer[i] <= 'Z')
			qwValue += pcBuffer[i] - 'A' + 10;
		else if ('a' <= pcBuffer[i] && pcBuffer[i] <= 'z')
			qwValue += pcBuffer[i] - 'a' + 10;
		else
			qwValue += pcBuffer[i] - '0';
	}

	return qwValue;
}

long kDecimalStringToLong(const char *pcBuffer)
{
	long lValue = 0;
	int i;

	if (pcBuffer[0] == '-')
		i = 1;
	else
		i = 0;

	for (; pcBuffer[i]; i++)
	{
		lValue *= 10;
		lValue += pcBuffer[i] - '0';
	}

	if (pcBuffer[0] == '-')
		lValue = -lValue;

	return lValue;
}

int kIToA(long lValue, char *pcBuffer, int iRadix)
{
	int iReturn;

	switch (iRadix)
	{
	case 16:
		iReturn = kHexToString(lValue, pcBuffer);
		break;

	case 10:
	default:
		iReturn = kDecimalToString(lValue, pcBuffer);
		break;
	}

	return iReturn;
}

int kHexToString(QWORD qwValue, char *pcBuffer)
{
	QWORD i;
	QWORD qwCurrentValue;

	if (qwValue == 0)
	{
		pcBuffer[0] = '0';
		pcBuffer[1] = '\0';
		return 1;
	}

	for (i = 0; qwValue > 0; i++)
	{
		static char table[] = "0123456789ABCDEF";
		pcBuffer[i] = table[qwValue & 0xf];
		qwValue >>= 4;
	}
	pcBuffer[i] = '\0';

	kReverseString(pcBuffer);

	return i;
}

int kDecimalToString(long lValue, char *pcBuffer)
{
	long i;
	long iReturn = 0;

	if (lValue == 0)
	{
		pcBuffer[0] = '0';
		pcBuffer[1] = '\0';
		return 1;
	}

	if (lValue < 0)
	{
		*(pcBuffer++) = '-';
		lValue = -lValue;
		iReturn = 1;
	}

	for (i = 0; lValue > 0; i++)
	{
		pcBuffer[i] = '0' + (lValue % 10);
		lValue /= 10;
	}
	pcBuffer[i] = '\0';

	kReverseString(pcBuffer);

	return iReturn + i;
}

void kReverseString(char *pcBuffer)
{
	int iLength;
	int i;
	char cTemp;

	iLength = kStrLen(pcBuffer);
	for (i = 0; i < iLength / 2; i++)
	{
		cTemp = pcBuffer[i];
		pcBuffer[i] = pcBuffer[iLength - 1 - i];
		pcBuffer[iLength - 1 - i] = cTemp;
	}
}

int kSPrintf(char *pcBuffer, const char *pcFormatString, ...)
{
	va_list ap;
	int iReturn;

	va_start(ap, pcFormatString);
	iReturn = kVSPrintf(pcBuffer, pcFormatString, ap);
	va_end(ap);

	return iReturn;
}

int kVSPrintf(char *pcBuffer, const char *pcFormatString, va_list ap)
{
	QWORD i, j;
	int iBufferIndex = 0;
	int iFormatLength, iCopyLength;
	char *pcCopyString;
	QWORD qwValue;
	int iValue;

	iFormatLength = kStrLen(pcFormatString);
	for (i = 0; i < iFormatLength; i++)
	{
		if (pcFormatString[i] == '%')
		{
			i++;
			switch (pcFormatString[i])
			{
			case 's':
				pcCopyString = (char*)(va_arg(ap, char*));
				iCopyLength = kStrLen(pcCopyString);

				kMemCpy(pcBuffer + iBufferIndex, pcCopyString, iCopyLength);
				iBufferIndex += iCopyLength;
				break;

			case 'c':
				pcBuffer[iBufferIndex] = (char)(va_arg(ap, int));
				iBufferIndex++;
				break;

			case 'd':
			case 'i':
				iValue = (int)(va_arg(ap, int));
				iBufferIndex += kIToA(iValue, pcBuffer + iBufferIndex, 10);
				break;

			case 'x':
			case 'X':
				qwValue = (DWORD)(va_arg(ap, DWORD)) & 0xFFFFFFFF;
				iBufferIndex += kIToA(qwValue, pcBuffer + iBufferIndex, 16);
				break;

			case 'q':
			case 'Q':
			case 'p':
				qwValue = (QWORD)(va_arg(ap, QWORD));
				iBufferIndex += kIToA(qwValue, pcBuffer + iBufferIndex, 16);
				break;

			default:
				pcBuffer[iBufferIndex] = pcFormatString[i];
				iBufferIndex++;
				break;
			}
		}
		else
		{
			pcBuffer[iBufferIndex] = pcFormatString[i];
			iBufferIndex++;
		}
	}

	pcBuffer[iBufferIndex] = '\0';
	return iBufferIndex;
}

QWORD kGetTickCount()
{
	return g_qwTickCount;
}
