#include "Types.h"
#include "AssemblyUtility.h"
#include "Keyboard.h"
#include "Queue.h"
#include "Utility.h"


static KEYBOARDMANAGER gs_stKeyboardManager = { 0 };

// A Queue to save the key states (by the interrupt handler)
static QUEUE gs_stKeyQueue;
static KEYDATA gs_vstKeyQueueBuffer[KEY_MAXQUEUECOUNT];

static KEYMAPPINGENTRY gs_vstKeyMappingTable[KEY_MAPPINGTABLEMAXCOUNT] =
{
/*  0x00  */  {   KEY_NONE        ,   KEY_NONE        },
/*  0x01  */  {   KEY_ESC         ,   KEY_ESC         },
/*  0x02  */  {   '1'             ,   '!'             },
/*  0x03  */  {   '2'             ,   '@'             },
/*  0x04  */  {   '3'             ,   '#'             },
/*  0x05  */  {   '4'             ,   '$'             },
/*  0x06  */  {   '5'             ,   '%'             },
/*  0x07  */  {   '6'             ,   '^'             },
/*  0x08  */  {   '7'             ,   '&'             },
/*  0x09  */  {   '8'             ,   '*'             },
/*  0x0A  */  {   '9'             ,   '('             },
/*  0x0B  */  {   '0'             ,   ')'             },
/*  0x0C  */  {   '-'             ,   '_'             },
/*  0x0D  */  {   '='             ,   '+'             },
/*  0x0E  */  {   KEY_BACKSPACE   ,   KEY_BACKSPACE   },
/*  0x0F  */  {   KEY_TAB         ,   KEY_TAB         },
/*  0x10  */  {   'q'             ,   'Q'             },
/*  0x11  */  {   'w'             ,   'W'             },
/*  0x12  */  {   'e'             ,   'E'             },
/*  0x13  */  {   'r'             ,   'R'             },
/*  0x14  */  {   't'             ,   'T'             },
/*  0x15  */  {   'y'             ,   'Y'             },
/*  0x16  */  {   'u'             ,   'U'             },
/*  0x17  */  {   'i'             ,   'I'             },
/*  0x18  */  {   'o'             ,   'O'             },
/*  0x19  */  {   'p'             ,   'P'             },
/*  0x1A  */  {   '['             ,   '{'             },
/*  0x1B  */  {   ']'             ,   '}'             },
/*  0x1C  */  {   '\n'            ,   '\n'            },
/*  0x1D  */  {   KEY_CTRL        ,   KEY_CTRL        },
/*  0x1E  */  {   'a'             ,   'A'             },
/*  0x1F  */  {   's'             ,   'S'             },
/*  0x20  */  {   'd'             ,   'D'             },
/*  0x21  */  {   'f'             ,   'F'             },
/*  0x22  */  {   'g'             ,   'G'             },
/*  0x23  */  {   'h'             ,   'H'             },
/*  0x24  */  {   'j'             ,   'J'             },
/*  0x25  */  {   'k'             ,   'K'             },
/*  0x26  */  {   'l'             ,   'L'             },
/*  0x27  */  {   ';'             ,   ':'             },
/*  0x28  */  {   '\''            ,   '\"'            },
/*  0x29  */  {   '`'             ,   '~'             },
/*  0x2A  */  {   KEY_LSHIFT      ,   KEY_LSHIFT      },
/*  0x2B  */  {   '\\'            ,   '|'             },
/*  0x2C  */  {   'z'             ,   'Z'             },
/*  0x2D  */  {   'x'             ,   'X'             },
/*  0x2E  */  {   'c'             ,   'C'             },
/*  0x2F  */  {   'v'             ,   'V'             },
/*  0x30  */  {   'b'             ,   'B'             },
/*  0x31  */  {   'n'             ,   'N'             },
/*  0x32  */  {   'm'             ,   'M'             },
/*  0x33  */  {   ','             ,   '<'             },
/*  0x34  */  {   '.'             ,   '>'             },
/*  0x35  */  {   '/'             ,   '?'             },
/*  0x36  */  {   KEY_RSHIFT      ,   KEY_RSHIFT      },
/*  0x37  */  {   '*'             ,   '*'             },
/*  0x38  */  {   KEY_LALT        ,   KEY_LALT        },
/*  0x39  */  {   ' '             ,   ' '             },
/*  0x3A  */  {   KEY_CAPSLOCK    ,   KEY_CAPSLOCK    },
/*  0x3B  */  {   KEY_F1          ,   KEY_F1          },
/*  0x3C  */  {   KEY_F2          ,   KEY_F2          },
/*  0x3D  */  {   KEY_F3          ,   KEY_F3          },
/*  0x3E  */  {   KEY_F4          ,   KEY_F4          },
/*  0x3F  */  {   KEY_F5          ,   KEY_F5          },
/*  0x40  */  {   KEY_F6          ,   KEY_F6          },
/*  0x41  */  {   KEY_F7          ,   KEY_F7          },
/*  0x42  */  {   KEY_F8          ,   KEY_F8          },
/*  0x43  */  {   KEY_F9          ,   KEY_F9          },
/*  0x44  */  {   KEY_F10         ,   KEY_F10         },
/*  0x45  */  {   KEY_NUMLOCK     ,   KEY_NUMLOCK     },
/*  0x46  */  {   KEY_SCROLLLOCK  ,   KEY_SCROLLLOCK  },
/*  0x47  */  {   KEY_HOME        ,   '7'             },
/*  0x48  */  {   KEY_UP          ,   '8'             },
/*  0x49  */  {   KEY_PAGEUP      ,   '9'             },
/*  0x4A  */  {   '-'             ,   '-'             },
/*  0x4B  */  {   KEY_LEFT        ,   '4'             },
/*  0x4C  */  {   KEY_CENTER      ,   '5'             },
/*  0x4D  */  {   KEY_RIGHT       ,   '6'             },
/*  0x4E  */  {   '+'             ,   '+'             },
/*  0x4F  */  {   KEY_END         ,   '1'             },
/*  0x50  */  {   KEY_DOWN        ,   '2'             },
/*  0x51  */  {   KEY_PAGEDOWN    ,   '3'             },
/*  0x52  */  {   KEY_INS         ,   '0'             },
/*  0x53  */  {   KEY_DEL         ,   '.'             },
/*  0x54  */  {   KEY_NONE        ,   KEY_NONE        },
/*  0x55  */  {   KEY_NONE        ,   KEY_NONE        },
/*  0x56  */  {   KEY_NONE        ,   KEY_NONE        },
/*  0x57  */  {   KEY_F11         ,   KEY_F11         },
/*  0x58  */  {   KEY_F12         ,   KEY_F12         }
};


BOOL kIsOutputBufferFull()
{
	// 0x64: Status Register of Keyboard Controller
	// 0x01: OUTB (Output Buffer State)
	if (kInPortByte(0x64) & 0x01)
		return TRUE;

	return FALSE;
}

BOOL kIsInputBufferFull()
{
	// 0x64: Status Register of Keyboard Controller
	// 0x02: INPB (Input Buffer State)
	if (kInPortByte(0x64) & 0x02)
		return TRUE;

	return FALSE;
}

BOOL kWaitForACKAndPutOtherScanCode()
{
	int i, j;
	BYTE bData;
	BOOL bResult = FALSE;

	for (j = 0; j < 100; j++)
	{
		for (i = 0; i < 0x10000; i++)
			if (kIsOutputBufferFull())
				break; // Output buffer is full

		bData = kInPortByte(0x60);
		if (bData == 0xFA)
		{ // ACK
			bResult = TRUE;
			break;
		}
		else
		{ // ~ACK
			kConvertScanCodeAndPutQueue(bData);
		}
	}

	return bResult;
}

BOOL kActivateKeyboard()
{
	int i, j;
	BOOL bPreviousInterrupt;
	BOOL bResult;

	// Disable Interrupt
	bPreviousInterrupt = kSetInterruptFlag(FALSE);

	// Activate Keyboard Device
	kOutPortByte(0x64, 0xAE);

	// Wait for the keyboard
	for (i = 0; i < 0x10000; i++)
		if (!kIsInputBufferFull())
			break; // Input buffer is ready

	// Activate Keyboard
	kOutPortByte(0x60, 0xF4);

	// Wait for the ACK msg
	bResult = kWaitForACKAndPutOtherScanCode();

	// Restore Interrupt State
	kSetInterruptFlag(bPreviousInterrupt);
	return bResult;
}

BYTE kGetKeyboardScanCode()
{
	// Wait until the output buffer is full
	while (!kIsOutputBufferFull());

	return kInPortByte(0x60);
}

void kEnableA20Gate()
{
	BYTE bOutputPortData;
	int i;

	// Read the output port value
	//  of the Keyboard Controller
	kOutPortByte(0x64, 0xD0);

	for (i = 0; i < 0x10000; i++)
		if (kIsOutputBufferFull())
			break; // Output buffer is full


	// 0x02: Enable A20 Gate
	bOutputPortData = kInPortByte(0x60) | 0x02;


	for (i = 0; i < 0x10000; i++)
		if (!kIsInputBufferFull())
			break; // Input buffer is ready

	kOutPortByte(0x64, 0xD1);

	kOutPortByte(0x60, bOutputPortData);
}

void kReboot()
{
	int i;

	for (i = 0; i < 0x10000; i++)
		if (!kIsInputBufferFull())
			break; // Input buffer is ready

	kOutPortByte(0x64, 0xD1);

	// Processor reset
	kOutPortByte(0x60, 0x00);

	while (TRUE);
}

BOOL kChangeKeyboardLED(BOOL bCapsLockOn, BOOL bNumLockOn, BOOL bScrollLockOn)
{
	int i, j;
	BOOL bPreviousInterrupt;
	BOOL bResult;
	BYTE bData;

	// Disable Interrupt
	bPreviousInterrupt = kSetInterruptFlag(FALSE);

	for (i = 0; i < 0x10000; i++)
		if (!kIsInputBufferFull())
			break; // Input buffer is ready

	// 0xED: Change LED status
	kOutPortByte(0x60, 0xED);
	for (i = 0; i < 0x10000; i++)
		if (!kIsInputBufferFull())
			break; // Keyboard takes the command

	// Wait for the ACK msg
	bResult = kWaitForACKAndPutOtherScanCode();

	if (!bResult)
	{
		kSetInterruptFlag(bPreviousInterrupt);
		return FALSE;
	}

	kOutPortByte(0x60, (bCapsLockOn << 2) | (bNumLockOn << 1) | bScrollLockOn);
	for (i = 0; i < 0x10000; i++)
		if (!kIsInputBufferFull())
			break; // Keyboard takes the command

	// Wait for the ACK msg
	bResult = kWaitForACKAndPutOtherScanCode();

	// Restore Interrupt State
	kSetInterruptFlag(bPreviousInterrupt);
	return bResult;
}

BOOL kIsUseCombinedCode(BYTE bScanCode)
{
	BYTE bDownScanCode;
	BOOL bUseCombinedKey = FALSE;

	bDownScanCode = bScanCode & 0x7F;

	if (kIsAlphabetScanCode(bDownScanCode))
	{
		// Shift && CapsLock = lower case
		bUseCombinedKey = gs_stKeyboardManager.bShiftDown ^ gs_stKeyboardManager.bCapsLockOn;
	}
	else if (kIsNumberOrSymbolScanCode(bDownScanCode))
	{
		bUseCombinedKey = gs_stKeyboardManager.bShiftDown;
	}
	else if (kIsNumberPadScanCode(bDownScanCode) &&
				!gs_stKeyboardManager.bExtendedCodeIn)
	{
		// bExtendedCodeIn -> Not Combined key
		bUseCombinedKey = gs_stKeyboardManager.bNumLockOn;
	}

	return bUseCombinedKey;
}

BOOL kIsAlphabetScanCode(BYTE bScanCode)
{
	BYTE bNormalCode = gs_vstKeyMappingTable[bScanCode].bNormalCode;
	
	return ('a' <= bNormalCode && bNormalCode <= 'z');
}

BOOL kIsNumberOrSymbolScanCode(BYTE bScanCode)
{
	return (0x02 <= bScanCode) && (bScanCode <= 0x35) && (!kIsAlphabetScanCode(bScanCode));
}

BOOL kIsNumberPadScanCode(BYTE bScanCode)
{
	return (0x47 <= bScanCode) && (bScanCode <= 0x53);
}

void UpdateCombinationKeyStatusAndLED(BYTE bScanCode)
{
	BOOL bDown;
	BYTE bDownScanCode;
	BOOL bLEDStatusChanged = FALSE;

	// LSB 0: Key Down
	// LSB 1: Key UP
	bDown = !(bScanCode & 0x80);
	bDownScanCode = bScanCode & 0x7F;

	if (bDownScanCode == 0x2A || bDownScanCode == 0x36)
	{ // LSHIFT or RSHIFT
		gs_stKeyboardManager.bShiftDown = bDown;
	}
	else if (bDownScanCode == 0x3A && bDown)
	{ // CAPSLOCK pressed
		gs_stKeyboardManager.bCapsLockOn ^= TRUE;
		bLEDStatusChanged = TRUE;
	}
	else if (bDownScanCode == 0x45 && bDown)
	{ // NUMLOCK
		gs_stKeyboardManager.bNumLockOn ^= TRUE;
		bLEDStatusChanged = TRUE;
	}
	else if (bDownScanCode == 0x46 && bDown)
	{ // SCROLLLOCK
		gs_stKeyboardManager.bScrollLockOn ^= TRUE;
		bLEDStatusChanged = TRUE;
	}

	if (bLEDStatusChanged)
		kChangeKeyboardLED(gs_stKeyboardManager.bCapsLockOn,
							gs_stKeyboardManager.bNumLockOn,
							gs_stKeyboardManager.bScrollLockOn);
}

BOOL kConvertScanCodeToASCIICode(BYTE bScanCode, BYTE *pbASCIICode, BOOL *pbFlags)
{
	if (gs_stKeyboardManager.iSkipCountForPause > 0)
	{
		gs_stKeyboardManager.iSkipCountForPause--;
		return FALSE;
	}

	if (bScanCode == 0xE1)
	{ // No Up-Code for Pause key
		*pbASCIICode = KEY_PAUSE;
		*pbFlags = KEY_FLAGS_DOWN;
		gs_stKeyboardManager.iSkipCountForPause = KEY_SKIPCOUNTFORPAUSE;
	}
	else if (bScanCode == 0xE0)
	{
		gs_stKeyboardManager.bExtendedCodeIn = TRUE;
		return FALSE;
	}


	if (kIsUseCombinedCode(bScanCode))
		*pbASCIICode = gs_vstKeyMappingTable[bScanCode & 0x7F].bCombinedCode;
	else
		*pbASCIICode = gs_vstKeyMappingTable[bScanCode & 0x7F].bNormalCode;

	if (gs_stKeyboardManager.bExtendedCodeIn)
	{ // If 0xE0 is sent just before
		*pbFlags = KEY_FLAGS_EXTENDEDKEY;
		gs_stKeyboardManager.bExtendedCodeIn = FALSE;
	}
	else
		*pbFlags = 0;

	if (!(bScanCode & 0x80))
		*pbFlags |= KEY_FLAGS_DOWN;

	UpdateCombinationKeyStatusAndLED(bScanCode);
	return TRUE;
}

BOOL kInitializeKeyboard()
{
	// Initialize Quue
	kInitializeQueue(&gs_stKeyQueue, gs_vstKeyQueueBuffer, KEY_MAXQUEUECOUNT, sizeof(KEYDATA));

	// Activate Keyboard
	return kActivateKeyboard();
}

BOOL kConvertScanCodeAndPutQueue(BYTE bScanCode)
{
	KEYDATA stData;
	BOOL bResult = FALSE;
	BOOL bPreviousInterrupt;

	stData.bScanCode = bScanCode;

	if (kConvertScanCodeToASCIICode(bScanCode, &(stData.bASCIICode), &(stData.bFlags)))
	{
		// Disable Interrupt
		bPreviousInterrupt = kSetInterruptFlag(FALSE);

		bResult = kPutQueue(&gs_stKeyQueue, &stData);

		// Restore Interrupt Status
		kSetInterruptFlag(bPreviousInterrupt);
	}

	return bResult;
}

BOOL kGetKeyFromKeyQueue(KEYDATA *pstData)
{
	BOOL bResult;
	BOOL bPreviousInterrupt;

	if (kIsQueueEmpty(&gs_stKeyQueue)) // Is this really necessary?
		return FALSE;

	// Disable Interrupt
	bPreviousInterrupt = kSetInterruptFlag(FALSE);

	bResult = kGetQueue(&gs_stKeyQueue, pstData);

	// Restore Interrupt Status
	kSetInterruptFlag(bPreviousInterrupt);

	return bResult;
}
