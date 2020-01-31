
#include "Types.h"
#include "Keyboard.h"
#include "Descriptor.h"
#include "AssemblyUtility.h"
#include "PIC.h"


void kPrintString(int iX, int iY, const char *pcString);


void Main()
{
	char vcTemp[2] = { 0 };
	BYTE bFlags;
	BYTE bTemp;
	int i = 0;

	kPrintString(0, 10, "Switch To IA-32e mode success");
	kPrintString(0, 11, "IA-32e C Language Kernel Start..............................[Pass]");


	kPrintString(0, 12, "GDT Initialize And Switch For IA-32e Mode...................[    ]");
	kInitializeGDTTableAndTSS();
	kLoadGDTR(GDTR_STARTADDRESS);
	kPrintString(61, 12, "Pass");


	kPrintString(0, 13, "TSS Segment Load............................................[    ]");
	kLoadTR(GDT_TSSSEGMENT);
	kPrintString(61, 13, "Pass");


	kPrintString(0, 14, "IDT Initialize..............................................[    ]");
	kInitializeIDTTables();
	kLoadIDTR(IDTR_STARTADDRESS);
	kPrintString(61, 14, "Pass");



	kPrintString(0, 15, "Keyboard Activate...........................................[    ]");

	if (kActivateKeyboard())
	{
		kPrintString(61, 15, "Pass");
		kChangeKeyboardLED(FALSE, FALSE, FALSE);
	}
	else
	{
		kPrintString(61, 15, "Fail");
		while (1);
	}

	kPrintString(0, 16, "PIC Controller And Interrupt Initialize.....................[    ]");
	kInitializePIC();
	kMaskPICInterrupt(0);
	kEnableInterrupt();
	kPrintString(61, 16, "Pass");



	while (1)
	{
		if (kIsOutputBufferFull())
		{ // Ready to read

			bTemp = kGetKeyboardScanCode();

			if (kConvertScanCodeToASCIICode(bTemp, &(vcTemp[0]), &bFlags))
			{
				//if (bFlags & KEY_FLAGS_DOWN)
				if (bFlags & KEY_FLAGS_DOWN && !(vcTemp[0] & 0x80))
				{
					kPrintString(i++, 17, vcTemp);

					if (vcTemp[0] == '0')
						bTemp = bTemp / 0;
				}
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
