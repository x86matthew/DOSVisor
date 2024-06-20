#include "DosVisor.h"

InterruptHandlerStruct Global_InterruptHandlerList[] =
{
	{ 0x01, InterruptHandler_0x01 },
	{ 0x10, InterruptHandler_0x10 },
	{ 0x16, InterruptHandler_0x16 },
	{ 0x1A, InterruptHandler_0x1A },
	{ 0x21, InterruptHandler_0x21 },
};

DWORD VmExit_Cpuid(VmExitStateStruct *pVmExitState)
{
	DWORD dwCaughtInterrupt = 0;
	BYTE bInterruptIndex = 0;
	DWORD dwCurrInstructionLinear = 0;
	DWORD dwInterruptHandlerCount = 0;
	InterruptHandlerStruct *pInterruptHandler = NULL;
	WORD *pwCurrStack = NULL;

	dwCurrInstructionLinear = SEG_TO_LINEAR(pVmExitState->pCpuRegisterState->CS, pVmExitState->pCpuRegisterState->IP);

	// check which interrupt executed the cpuid instruction
	for(DWORD i = 0; i < INTERRUPT_COUNT; i++)
	{
		if(dwCurrInstructionLinear == (DWORD)((BYTE*)&pVmExitState->pDosSystemMemory->wInterruptHandlerCpuid[i] - (BYTE*)pVmExitState->pDosSystemMemory))
		{
			// found interrupt index
			dwCaughtInterrupt = 1;
			bInterruptIndex = (BYTE)i;
		}
	}

	if(dwCaughtInterrupt == 0)
	{
		// interrupt index not found - this means something else executed the cpuid instruction which shouldn't happen in a DOS program
		return 1;
	}

	if(dwGlobal_DebugInterrupts != 0)
	{
		WriteLog("Caught interrupt 0x%02X (AH: 0x%02X)\n", bInterruptIndex, pVmExitState->pCpuRegisterState->AH);
	}

	dwInterruptHandlerCount = sizeof(Global_InterruptHandlerList) / sizeof(Global_InterruptHandlerList[0]);
	for(DWORD i = 0; i < dwInterruptHandlerCount; i++)
	{
		if(Global_InterruptHandlerList[i].bInterruptIndex == bInterruptIndex)
		{
			pInterruptHandler = &Global_InterruptHandlerList[i];
			break;
		}
	}

	if(pInterruptHandler == NULL)
	{
		// unhandled interrupt
		WriteLog("Error: Unhandled interrupt (0x%02X)\n", bInterruptIndex);
		return 1;
	}

	// call interrupt handler
	if(pInterruptHandler->pInterruptHandler(pVmExitState->pCpuRegisterState, pVmExitState->pDosSystemMemory) != 0)
	{
		WriteLog("Error: Interrupt handler failed (0x%02X)\n", bInterruptIndex);
		return 1;
	}

	// get pointer to the stack
	pwCurrStack = (WORD*)GetHostDataPointer(pVmExitState->pDosSystemMemory, SEG_TO_LINEAR(pVmExitState->pCpuRegisterState->SS, pVmExitState->pCpuRegisterState->SP), sizeof(WORD) * 3);
	if(pwCurrStack == NULL)
	{
		return 1;
	}

	// restore CS/IP from stack (ignore stored FLAGS value)
	pVmExitState->pCpuRegisterState->CS = *(WORD*)(pwCurrStack + 1);
	pVmExitState->pCpuRegisterState->IP = *(WORD*)pwCurrStack;

	if(dwGlobal_DebugTrace != 0)
	{
		// reset trap flag
		pVmExitState->pCpuRegisterState->RFLAGS |= 0x100;
		LogSingleStep(pVmExitState->pCpuRegisterState);
	}

	// IRET has been emulated - restore stack (IP, CS, FLAGS)
	pVmExitState->pCpuRegisterState->SP += sizeof(WORD) * 3;

	return 0;
}
