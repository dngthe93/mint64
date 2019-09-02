
#include "Types.h"
#include "Keyboard.h"


void kPrintString(int iX, int iY, const char *pcString);


void Main()
{
	char vcTemp[2] = { 0 };
	BYTE bFlags;
	BYTE bTemp;
	int i = 0;

	kPrintString(0, 10, "Switch To IA-32e mode success");
	kPrintString(0, 11, "IA-32e C Language Kernel Start..............................[Pass]");
	kPrintString(0, 12, "Keyboard Activate...........................................[    ]");

	if (kActivateKeyboard())
	{
		kPrintString(61, 12, "Pass");
		kChangeKeyboardLED(FALSE, FALSE, FALSE);
	}
	else
	{
		kPrintString(61, 12, "Fail");
		while (1);
	}

	while (1)
	{
		if (kIsOutputBufferFull())
		{ // Ready to read

			bTemp = kGetKeyboardScanCode();

			if (kConvertScanCodeToASCIICode(bTemp, &(vcTemp[0]), &bFlags))
			{
				//if (bFlags & KEY_FLAGS_DOWN)
				if (bFlags & KEY_FLAGS_DOWN && !(vcTemp[0] & 0x80))
					kPrintString(i++, 13, vcTemp);
			}
		}
	}

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
