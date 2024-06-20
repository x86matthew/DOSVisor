#include "DosVisor.h"

DWORD InterruptHandler_0x10(CpuRegisterStateStruct *pCpuRegisterState, DosSystemMemoryStruct *pDosSystemMemory)
{
	if(pCpuRegisterState->AH == 0x01)
	{
		// set cursor shape

		// (not implemented, but continue with no error)
	}
	else if(pCpuRegisterState->AH == 0x02)
	{
		// set cursor position
		Terminal_SetCursorPos(pCpuRegisterState->DL, pCpuRegisterState->DH);
	}
	else if(pCpuRegisterState->AH == 0x06)
	{
		// scroll up
		if(pCpuRegisterState->AL == 0)
		{
			// clear screen
			Terminal_Clear();
		}
		else
		{
			// not implemented
			return 1;
		}
	}
	else if(pCpuRegisterState->AH == 0x09)
	{
		// write character
		for(DWORD i = 0; i < pCpuRegisterState->CX; i++)
		{
			Terminal_WriteChar(pCpuRegisterState->AL, 1, pCpuRegisterState->BL);
		}
	}
	else if(pCpuRegisterState->AH == 0x0E)
	{
		// teletype output
		Terminal_WriteChar(pCpuRegisterState->AL, 0, pCpuRegisterState->BL);
	}
	else if(pCpuRegisterState->AH == 0x0F)
	{
		// get video mode - 80x25 (page 0)
		pCpuRegisterState->AH = 80;
		pCpuRegisterState->AL = 3;
		pCpuRegisterState->BH = 0;
	}
	else if(pCpuRegisterState->AH == 0x10)
	{
		// set/get palette registers

		// (not implemented, but continue with no error)
	}
	else
	{
		// unhandled
		return 1;
	}

	return 0;
}
