#include "DosVisor.h"

BYTE bGlobal_CmosRegisterIndex = 0;

DWORD HandleIoPort_Read(WORD wPortNumber, WORD *pwValue)
{
	SYSTEMTIME Time;
	QWORD qwElapsedTime = 0;
	QWORD qwRefreshCount = 0;

	if(wPortNumber == 0x61)
	{
		// a common way to delay execution in DOS was to query the 0x61 port in a loop.
		// the 8253 PIT timer would flip bit #4 (RAM refresh toggle) every 15 microseconds,
		// meaning one full cycle took 30 microseconds. programs would wait for this bit
		// to flip twice which indicates that 30 microseconds has elapsed, and this was
		// typically used in conjunction with a multiplier.
		// this behaviour can be emulated by calculating the number of microseconds elapsed
		// since the emulator started, and divide this value by 15 - bit #4 will be set or
		// cleared depending on whether this value is odd or even.
		qwElapsedTime = GetElapsedTimeMicroseconds();
		qwRefreshCount = qwElapsedTime / 15;
		if((qwRefreshCount % 2) == 0)
		{
			// set bit 4
			*pwValue = 0x10;
		}
		else
		{
			// clear bit 4
			*pwValue = 0;
		}
	}
	else if(wPortNumber == 0x40)
	{
		// PIT channel 0 (not implemented)
	}
	else if(wPortNumber == 0x41)
	{
		// PIT channel 1 (not implemented)
	}
	else if(wPortNumber == 0x42)
	{
		// PIT channel 2 (not implemented)
	}
	else if(wPortNumber == 0x71)
	{
		GetLocalTime(&Time);
		if(bGlobal_CmosRegisterIndex == 0)
		{
			*pwValue = Time.wSecond;
		}
		else if(bGlobal_CmosRegisterIndex == 2)
		{
			*pwValue = Time.wMinute;
		}
		else
		{
			WriteLog("Error: Unimplemented CMOS register: 0x%02X\n", bGlobal_CmosRegisterIndex);
			return 1;
		}
	}
	else if(wPortNumber >= 0x220 && wPortNumber <= 0x223)
	{
		// sound blaster
	}	
	else
	{
		WriteLog("Error: Unknown port IO (read): 0x%02X\n", wPortNumber);
		return 1;
	}

	return 0;
}

DWORD HandleIoPort_Write(WORD wPortNumber, WORD wValue)
{
	if(wPortNumber >= 0x220 && wPortNumber <= 0x223)
	{
		// sound blaster (not implemented)
	}
	else if(wPortNumber == 0x61)
	{
		// speaker (not implemented)
	}
	else if(wPortNumber == 0x40)
	{
		// PIT channel 0 (not implemented)
	}
	else if(wPortNumber == 0x41)
	{
		// PIT channel 1 (not implemented)
	}
	else if(wPortNumber == 0x42)
	{
		// PIT channel 2 (not implemented)
	}
	else if(wPortNumber == 0x43)
	{
		// PIT control (not implemented)
	}
	else if(wPortNumber == 0x70)
	{
		// cmos
		bGlobal_CmosRegisterIndex = (BYTE)wValue; 
	}
	else
	{
		WriteLog("Error: Unknown port IO (write): 0x%02X, 0x%02X\n", wPortNumber, wValue);
		return 1;
	}

	return 0;
}

DWORD VmExit_IoPort(VmExitStateStruct *pVmExitState)
{
	WORD wValue = 0;

	if(pVmExitState->pVmExitContext->IoPortAccess.AccessInfo.IsWrite == 0)
	{
		// read from port ("in" instruction)
		if(HandleIoPort_Read(pVmExitState->pVmExitContext->IoPortAccess.PortNumber, &wValue) != 0)
		{
			return 1;
		}

		if(pVmExitState->pVmExitContext->IoPortAccess.AccessInfo.AccessSize == 1)
		{
			pVmExitState->pCpuRegisterState->AL = (BYTE)wValue;
		}
		else if(pVmExitState->pVmExitContext->IoPortAccess.AccessInfo.AccessSize == 2)
		{
			pVmExitState->pCpuRegisterState->AX = wValue;
		}
		else
		{
			return 1;
		}
	}
	else
	{
		// write to port ("out" instruction)
		if(pVmExitState->pVmExitContext->IoPortAccess.AccessInfo.AccessSize == 1)
		{
			wValue = pVmExitState->pCpuRegisterState->AL;
		}
		else if(pVmExitState->pVmExitContext->IoPortAccess.AccessInfo.AccessSize == 2)
		{
			wValue = pVmExitState->pCpuRegisterState->AX;
		}
		else
		{
			return 1;
		}

		if(HandleIoPort_Write(pVmExitState->pVmExitContext->IoPortAccess.PortNumber, wValue) != 0)
		{
			return 1;
		}
	}

	// skip over instruction
	pVmExitState->pCpuRegisterState->IP += pVmExitState->pVmExitContext->VpContext.InstructionLength;

	return 0;
}
