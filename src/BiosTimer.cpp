#include "DosVisor.h"

DWORD dwGlobal_StopBiosTimer = 0;
HANDLE hGlobal_BiosTimerThread = NULL;

QWORD qwGlobal_PerformanceCounterStartTime = 0;
QWORD qwGlobal_PerformanceCounterFrequency = 0;

QWORD GetElapsedTimeMicroseconds()
{
    LARGE_INTEGER PerformanceCounterCurrentTime;

    QueryPerformanceCounter(&PerformanceCounterCurrentTime);

    return ((PerformanceCounterCurrentTime.QuadPart - qwGlobal_PerformanceCounterStartTime) * 1000000) / qwGlobal_PerformanceCounterFrequency;
}

DWORD WINAPI BiosTimerThread(LPVOID lpArg)
{
	DosSystemMemoryStruct *pDosSystemMemory = NULL;
	DWORD *pdwCounterValue = NULL;

	pDosSystemMemory = (DosSystemMemoryStruct*)lpArg;

	// increase the BIOS timer every 50ms (18.2065hz on real hardware)
	pdwCounterValue = (DWORD*)&pDosSystemMemory->bReservedBIOS[0x6C];
	for(;;)
	{
		if(dwGlobal_StopBiosTimer != 0)
		{
			break;
		}

		InterlockedIncrement((LONG*)pdwCounterValue);

		Sleep(50);
	}

	return 0;
}

DWORD InitialiseBiosTimer(DosSystemMemoryStruct *pDosSystemMemory)
{
	LARGE_INTEGER PerformanceCounterStartTime;
	LARGE_INTEGER PerformanceCounterFrequency;

	QueryPerformanceCounter(&PerformanceCounterStartTime); 
	qwGlobal_PerformanceCounterStartTime = PerformanceCounterStartTime.QuadPart;
	QueryPerformanceFrequency(&PerformanceCounterFrequency);
	qwGlobal_PerformanceCounterFrequency = PerformanceCounterFrequency.QuadPart;

	// start thread
	hGlobal_BiosTimerThread = CreateThread(NULL, 0, BiosTimerThread, (LPVOID)pDosSystemMemory, 0, NULL);
	if(hGlobal_BiosTimerThread == NULL)
	{
		return 1;
	}

	return 0;
}

DWORD CloseBiosTimer()
{
	// stop thread
	dwGlobal_StopBiosTimer = 1;
	WaitForSingleObject(hGlobal_BiosTimerThread, INFINITE);
	CloseHandle(hGlobal_BiosTimerThread);

	return 0;
}
