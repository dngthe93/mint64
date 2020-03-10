
#include "PIT.h"
#include "AssemblyUtility.h"

void kInitializePIT(WORD wCount, BOOL bPeriodic)
{
	kOutPortByte(PIT_PORT_CONTROL, PIT_COUNTER0_ONCE);

	if (bPeriodic)
		kOutPortByte(PIT_PORT_CONTROL, PIT_COUNTER0_PERIODIC);

	kOutPortByte(PIT_PORT_COUNTER0, wCount);
	kOutPortByte(PIT_PORT_COUNTER0, wCount >> 8);
}

WORD kReadCounter0()
{
	BYTE bHighByte, bLowByte;
	BYTE wTemp = 0;

	kOutPortByte(PIT_PORT_CONTROL, PIT_COUNTER0_LATCH);

	bLowByte = kInPortByte(PIT_PORT_COUNTER0);
	bHighByte = kInPortByte(PIT_PORT_COUNTER0);

	return (WORD)((bHighByte << 8) | bLowByte);
}

void kWaitUsingDirectPIT(WORD wCount)
{
	WORD wLastCounter0;
	WORD wCurrentCounter0;

	kInitializePIT(0, TRUE);

	wLastCounter0 = kReadCounter0();

	while (1)
	{
		wCurrentCounter0 = kReadCounter0();
		if ( ((wLastCounter0 - wCurrentCounter0) & 0xFFFF) >= wCount)
			break;
	}
}
