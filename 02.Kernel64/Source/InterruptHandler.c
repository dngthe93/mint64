
#include "InterruptHandler.h"
#include "PIC.h"
#include "Keyboard.h"

void kPrintString(int iX, int iY, const char *pcString);

void kCommonExceptionHandler(int iVectorNumber, QWORD qwErrorCode)
{
	char vcBuffer[3] = { 0 };

	vcBuffer[0] = '0' + iVectorNumber / 10;
	vcBuffer[1] = '0' + iVectorNumber % 10;

	kPrintString(0, 0, "==================================================");
	kPrintString(0, 1, "                 Exception Occur~!!!!               ");
	kPrintString(0, 2, "                    Vector:                         ");
	kPrintString(27, 2, vcBuffer);
	kPrintString(0, 3, "==================================================");

	while (1);
}


void kCommonInterruptHandler(int iVectorNumber)
{
	char vcBuffer[] = "[INT:  , ]";
	static int g_iCommonInterruptCount = 0;


	vcBuffer[5] = '0' + iVectorNumber / 10;
	vcBuffer[6] = '0' + iVectorNumber % 10;

	vcBuffer[8] = '0' + g_iCommonInterruptCount;
	g_iCommonInterruptCount = (g_iCommonInterruptCount + 1) % 10;
	kPrintString(70, 0, vcBuffer);


	kSendEOIToPIC(iVectorNumber - PIC_IRQSTARTVECTOR);
}


void kKeyboardHandler(int iVectorNumber)
{
	char vcBuffer[] = "[INT:  , ]";
	static int g_iKeyboardInterruptCount = 0;

	// Print msgs to indicate that the interrupt has been generated
	vcBuffer[5] = '0' + iVectorNumber / 10;
	vcBuffer[6] = '0' + iVectorNumber % 10;

	vcBuffer[8] = '0' + g_iKeyboardInterruptCount;
	g_iKeyboardInterruptCount = (g_iKeyboardInterruptCount + 1) % 10;
	kPrintString(0, 0, vcBuffer);

	// Get keyboard keys from I/O port and put it to the queue
	if (kIsOutputBufferFull())
	{
		BYTE bTemp = kGetKeyboardScanCode();
		kConvertScanCodeAndPutQueue(bTemp);
	}

	kSendEOIToPIC(iVectorNumber - PIC_IRQSTARTVECTOR);
}
