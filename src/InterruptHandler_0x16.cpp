#include "DosVisor.h"

DWORD InterruptHandler_0x16(CpuRegisterStateStruct *pCpuRegisterState, DosSystemMemoryStruct *pDosSystemMemory)
{
	BYTE bVirtualKey = 0;
	WORD wScanCode = 0;
	BYTE bAscii = 0;

	if(pCpuRegisterState->AH == 0x01 || pCpuRegisterState->AH == 0x11)
	{
		// check for key press (don't remove from key buffer)
		if(CheckKeyWaiting(0, &bVirtualKey) != 0)
		{
			// no key pressed - set ZF
			pCpuRegisterState->RFLAGS |= 0x40;
		}
		else
		{
			// key pressed - clear ZF
			pCpuRegisterState->RFLAGS &= ~0x40;

			GetKeyInfo(bVirtualKey, &wScanCode, &bAscii);
			pCpuRegisterState->AL = bAscii;
			pCpuRegisterState->AH = (BYTE)wScanCode;
		}
	}
	else if(pCpuRegisterState->AH == 0x00 || pCpuRegisterState->AH == 0x10)
	{
		// wait for key press (remove from key buffer)
		WaitForKeyPress(1, &bVirtualKey);

		GetKeyInfo(bVirtualKey, &wScanCode, &bAscii);
		pCpuRegisterState->AL = bAscii;
		pCpuRegisterState->AH = (BYTE)wScanCode;
	}
	else
	{
		// unhandled
		return 1;
	}

	return 0;
}
