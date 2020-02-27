
#include <stdarg.h>
#include "Console.h"
#include "Keyboard.h"
#include "Utility.h"
#include "AssemblyUtility.h"


CONSOLEMANAGER gs_stConsoleManager = { 0 };

void kInitializeConsole(int iX, int iY)
{
	kMemSet(&gs_stConsoleManager, 0, sizeof(gs_stConsoleManager));

	kSetCursor(iX, iY);
}

void kSetCursor(int iX, int iY)
{
	int iLinearValue;

	iLinearValue = iY * CONSOLE_WIDTH + iX;

	// CRTC control address register (port 0x3D4)
	// 0x0E: upper byte of the cursor
	kOutPortByte(VGA_PORT_INDEX, VGA_INDEX_UPPERCURSOR);
	// CRTC control data register (port 0x3D5)
	kOutPortByte(VGA_PORT_DATA, iLinearValue >> 8);

	// CRTC control address register (port 0x3D4)
	// 0x0F: lower byte of the cursor
	kOutPortByte(VGA_PORT_INDEX, VGA_INDEX_LOWERCURSOR);
	// CRTC control data register (port 0x3D5)
	kOutPortByte(VGA_PORT_DATA, iLinearValue & 0xFF);

	// Update current print offset of the console manager
	gs_stConsoleManager.iCurrentPrintOffset = iLinearValue;
}

void kGetCursor(int *piX, int *piY)
{
	*piX = gs_stConsoleManager.iCurrentPrintOffset % CONSOLE_WIDTH;
	*piY = gs_stConsoleManager.iCurrentPrintOffset / CONSOLE_WIDTH;
}

void kPrintf(const char *pcFormatString, ...)
{
	va_list ap;
	char vcBuffer[1024];
	int iNextPrintOffset;

	va_start(ap, pcFormatString);
	kVSPrintf(vcBuffer, pcFormatString, ap);
	va_end(ap);

	iNextPrintOffset = kConsolePrintString(vcBuffer);

	// Update the position of the cursor
	kSetCursor(iNextPrintOffset % CONSOLE_WIDTH, iNextPrintOffset / CONSOLE_WIDTH);
}

int kConsolePrintString(const char *pcBuffer)
{
	CHARACTER *pstScreen = (CHARACTER*)CONSOLE_VIDEOMEMORYADDRESS;
	int i, j;
	int iLength;
	int iPrintOffset;

	iPrintOffset = gs_stConsoleManager.iCurrentPrintOffset;

	iLength = kStrLen(pcBuffer);
	for (i = 0; i < iLength; i++)
	{
		if (pcBuffer[i] == '\n')
			iPrintOffset += (CONSOLE_WIDTH - (iPrintOffset % CONSOLE_WIDTH));
		else if (pcBuffer[i] == '\t')
			iPrintOffset += (8 - (iPrintOffset % 8));
		else
		{
			// Print a character to the screen
			pstScreen[iPrintOffset].bCharactor = pcBuffer[i];
			pstScreen[iPrintOffset].bAttribute = CONSOLE_DEFAULTTEXTCOLOR;
			iPrintOffset++;
		}

		// iPrintOffset is bigger than the maximum size of the screen
		if (iPrintOffset >= CONSOLE_HEIGHT * CONSOLE_WIDTH)
		{
			// Scroll 1 line.
			kMemCpy(CONSOLE_VIDEOMEMORYADDRESS,
					CONSOLE_VIDEOMEMORYADDRESS + CONSOLE_WIDTH * sizeof(CHARACTER),
					(CONSOLE_HEIGHT - 1) * CONSOLE_WIDTH * sizeof(CHARACTER));

			// print all white spaces in the last line
			for (j = (CONSOLE_HEIGHT - 1) * CONSOLE_WIDTH;
					j < CONSOLE_HEIGHT * CONSOLE_WIDTH; j++)
			{
				pstScreen[j].bCharactor = ' ';
				pstScreen[j].bAttribute = CONSOLE_DEFAULTTEXTCOLOR;
			}

			iPrintOffset = (CONSOLE_HEIGHT - 1) * CONSOLE_WIDTH;
		}
	}

	return iPrintOffset;
}

void kClearScreen()
{
	CHARACTER *pstScreen = (CHARACTER*)CONSOLE_VIDEOMEMORYADDRESS;
	int i;

	for (i = 0; i < CONSOLE_WIDTH * CONSOLE_HEIGHT; i++)
	{
		pstScreen[i].bCharactor = ' ';
		pstScreen[i].bAttribute = CONSOLE_DEFAULTTEXTCOLOR;
	}

	kSetCursor(0, 0);
}

BYTE kGetCh()
{
	KEYDATA stData;

	while (1)
	{
		while (kGetKeyFromKeyQueue(&stData) == FALSE);

		if (stData.bFlags & KEY_FLAGS_DOWN)
			return stData.bASCIICode;
	}
}

void kPrintStringXY(int iX, int iY, const char *pcString)
{
	CHARACTER *pstScreen = (CHARACTER*)CONSOLE_VIDEOMEMORYADDRESS;
	int i;

	pstScreen += (iY * CONSOLE_WIDTH) + iX;

	for (i = 0; pcString[i]; i++)
	{
		pstScreen[i].bCharactor = pcString[i];
		pstScreen[i].bAttribute = CONSOLE_DEFAULTTEXTCOLOR;
	}
}
