#include "DosVisor.h"

DWORD VmExit_MemoryAccess(VmExitStateStruct *pVmExitState)
{
	char *pAccessType = NULL;

	// check access type
	if(pVmExitState->pVmExitContext->MemoryAccess.AccessInfo.AccessType == WHvMemoryAccessRead)
	{
		pAccessType = "read";
	}
	else if(pVmExitState->pVmExitContext->MemoryAccess.AccessInfo.AccessType == WHvMemoryAccessWrite)
	{
		pAccessType = "write";
	}
	else if(pVmExitState->pVmExitContext->MemoryAccess.AccessInfo.AccessType == WHvMemoryAccessExecute)
	{
		pAccessType = "execute";
	}
	else
	{
		return 1;
	}

	// write error
	WriteLog("Error: Invalid memory access (%s address 0x%p)\n", pAccessType, pVmExitState->pVmExitContext->MemoryAccess.Gpa);

	// always error
	return 1;
}
