#include "DosVisor.h"

DWORD InterruptHandler_0x1A(CpuRegisterStateStruct *pCpuRegisterState, DosSystemMemoryStruct *pDosSystemMemory)
{
	DWORD dwCounterValue = 0;
	DWORD *pdwCounterValue = NULL;

	if(pCpuRegisterState->AH == 0x00)
	{
		// read system clock counter
		pdwCounterValue = (DWORD*)&pDosSystemMemory->bReservedBIOS[0x6C];
		dwCounterValue = *pdwCounterValue;

		pCpuRegisterState->AL = 0;
		pCpuRegisterState->DX = LOWORD(dwCounterValue);
		pCpuRegisterState->CX = HIWORD(dwCounterValue);
	}
	else if(pCpuRegisterState->AH == 0x01)
	{
		// set system clock counter
		pdwCounterValue = (DWORD*)&pDosSystemMemory->bReservedBIOS[0x6C];
		dwCounterValue = MAKELONG(pCpuRegisterState->DX, pCpuRegisterState->CX);
		InterlockedExchange((LONG*)pdwCounterValue, dwCounterValue);
	}
	else
	{
		// unhandled
		return 1;
	}

	return 0;
}
