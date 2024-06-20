#include "DosVisor.h"

DWORD VmExit_Halt(VmExitStateStruct *pVmExitState)
{
	// halt - wait for key press (don't remove from key buffer)
	WaitForKeyPress(0, NULL);

	return 0;
}
