#include "DosVisor.h"

DWORD InterruptHandler_0x01(CpuRegisterStateStruct *pCpuRegisterState, DosSystemMemoryStruct *pDosSystemMemory)
{
	if(dwGlobal_DebugTrace == 0)
	{
		// not in debug mode - unexpected interrupt
		return 1;
	}

	return 0;
}
