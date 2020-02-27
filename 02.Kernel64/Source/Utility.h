
#ifndef __UTILITY_H__
#define __UTILITY_H__

#include <stdarg.h>
#include "Types.h"

void kMemSet(void *pvDst, BYTE bData, unsigned int iSize);
int kMemCpy(void *pvDst, const void *pvSrc, unsigned int iSize);
int kMemCmp(const void *pvDst, const void *pvSrc, unsigned int iSize);
BOOL kSetInterruptFlag(BOOL bEnableInterrupt);
int kStrLen(const char *pcBuffer);
void kCheckTotalRAMSize();
QWORD kGetTotalRAMSize();
long kAToI(const char *pcBuffer, int iRadix);
QWORD kHexStringToQword(const char *pcBuffer);
long kDecimalStringToLong(const char *pcBuffer);
int kIToA(long lValue, char *pcBuffer, int iRadix);
int kHexToString(QWORD qwValue, char *pcBuffer);
int kDecimalToString(long lValue, char *pcBuffer);
void kReverseString(char *pcBuffer);
int kSPrintf(char *pcBuffer, const char *pcFormatString, ...);
int kVSPrintf(char *pcBuffer, const char *pcFormatString, va_list ap);

#endif /*__UTILITY_H__*/
