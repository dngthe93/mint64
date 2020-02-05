
#ifndef __UTILITY_H__
#define __UTILITY_H__

#include "Types.h"

void kMemSet(void *pvDst, BYTE bData, unsigned int iSize);
int kMemCpy(void *pvDst, const void *pvSrc, unsigned int iSize);
int kMemCmp(const void *pvDst, const void *pvSrc, unsigned int iSize);
BOOL kSetInterruptFlag(BOOL bEnableInterrupt);

#endif /*__UTILITY_H__*/
