#include "DosVisor.h"

DWORD InterruptHandler_0x21(CpuRegisterStateStruct *pCpuRegisterState, DosSystemMemoryStruct *pDosSystemMemory)
{
	char *pStringPtr = NULL;
	SYSTEMTIME CurrTime;
	BYTE bTargetInterruptIndex = 0;
	DWORD dwInterruptHandlerLinear = 0;

	if(pCpuRegisterState->AH == 0x09)
	{
		// print string

		// get string pointer on host
		pStringPtr = (char*)GetHostDataPointer(pDosSystemMemory, SEG_TO_LINEAR(pCpuRegisterState->DS, pCpuRegisterState->DX), 1);
		if(pStringPtr == NULL)
		{
			return 1;
		}

		// print characters
		for(;;)
		{
			// ensure the current pointer is valid
			if(ValidateHostDataPointer(pDosSystemMemory, (BYTE*)pStringPtr) != 0)
			{
				return 1;
			}

			// check for terminator
			if(*pStringPtr == '$')
			{
				break;
			}

			// print current character
			printf("%c", *pStringPtr);

			// move to the next character
			pStringPtr++;
		}
	}
	else if(pCpuRegisterState->AH == 0x25)
	{
		// set interrupt handler
		bTargetInterruptIndex = pCpuRegisterState->AL;

		pDosSystemMemory->dwInterruptVectorTable[bTargetInterruptIndex] = MAKELONG(pCpuRegisterState->DX, pCpuRegisterState->DS);
	}
	else if(pCpuRegisterState->AH == 0x2C)
	{
		// get current time
		GetLocalTime(&CurrTime);
		pCpuRegisterState->CH = (BYTE)CurrTime.wHour;
		pCpuRegisterState->CL = (BYTE)CurrTime.wMinute;
		pCpuRegisterState->DH = (BYTE)CurrTime.wSecond;
		pCpuRegisterState->DL = (BYTE)(CurrTime.wMilliseconds / 10);
	}
	else if(pCpuRegisterState->AH == 0x35)
	{
		// get interrupt handler
		bTargetInterruptIndex = pCpuRegisterState->AL;
		dwInterruptHandlerLinear = (DWORD)((BYTE*)&pDosSystemMemory->dwInterruptVectorTable[bTargetInterruptIndex] - (BYTE*)pDosSystemMemory);

		pCpuRegisterState->ES = LINEAR_TO_SEG(dwInterruptHandlerLinear);
		pCpuRegisterState->BX = LINEAR_TO_OFFSET(dwInterruptHandlerLinear);
	}
	else if(pCpuRegisterState->AH == 0x4C)
	{
		// program finished
		WriteLog("Program exited with code: %u\n", pCpuRegisterState->AL);

		// disable logging to prevent further error messages
		dwGlobal_DisableLog = 1;
		return 1;
	}
	else
	{
		// unhandled
		WriteLog("Error: Unhandled int21 function: 0x%02X\n", pCpuRegisterState->AH);
		return 1;
	}
	
	return 0;
}
