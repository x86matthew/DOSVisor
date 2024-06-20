#include "DosVisor.h"

DWORD dwGlobal_StopKeyBuffer = 0;
HANDLE hGlobal_KeyBufferThread = NULL;

BYTE bGlobal_KeyState[MAX_VIRTUAL_KEY_COUNT];

BYTE bGlobal_KeyBuffer[MAX_KEY_BUFFER_LENGTH];
DWORD dwGlobal_CurrKeyBufferCount = 0;

CRITICAL_SECTION Global_KeyBufferCriticalSection;

DWORD GetKeyInfo(BYTE bVirtualKey, WORD *pwScanCode, BYTE *pbAscii)
{
	WORD wScanCode = 0;
	BYTE bAscii = 0;

	// get scan-code/ascii values from virtual key code
	wScanCode = (WORD)MapVirtualKey(bVirtualKey, 0);
	bAscii = (BYTE)MapVirtualKey(bVirtualKey, 2);

	*pwScanCode = wScanCode;
	*pbAscii = bAscii;

	return 0;
}

DWORD AddToKeyBuffer(BYTE bVirtualKey)
{
	// (this function is called by KeyBufferThread - already locked)
	if(dwGlobal_CurrKeyBufferCount >= MAX_KEY_BUFFER_LENGTH)
	{
		// key buffer is full
		return 1;
	}

	// add key to buffer
	bGlobal_KeyBuffer[dwGlobal_CurrKeyBufferCount] = bVirtualKey;
	dwGlobal_CurrKeyBufferCount++;

	return 0;
}

DWORD CheckKeyWaiting(DWORD dwRemoveFromKeyBuffer, BYTE *pbVirtualKey)
{
	DWORD dwKeyWaiting = 0;
	BYTE bVirtualKey = 0;

	EnterCriticalSection(&Global_KeyBufferCriticalSection);

	// check if there any keys in the buffer
	if(dwGlobal_CurrKeyBufferCount != 0)
	{
		dwKeyWaiting = 1;
		bVirtualKey = bGlobal_KeyBuffer[0];

		if(dwRemoveFromKeyBuffer != 0)
		{
			// remove from key buffer
			dwGlobal_CurrKeyBufferCount--;
			memmove(&bGlobal_KeyBuffer[0], &bGlobal_KeyBuffer[1], dwGlobal_CurrKeyBufferCount);
		}
	}

	LeaveCriticalSection(&Global_KeyBufferCriticalSection);

	if(dwKeyWaiting == 0)
	{
		// no key waiting
		return 1;
	}

	if(pbVirtualKey != NULL)
	{
		// store key
		*pbVirtualKey = bVirtualKey;
	}

	return 0;
}

DWORD WaitForKeyPress(DWORD dwRemoveFromKeyBuffer, BYTE *pbVirtualKey)
{
	// wait until the key buffer is not empty
	for(;;)
	{
		if(CheckKeyWaiting(dwRemoveFromKeyBuffer, pbVirtualKey) == 0)
		{
			break;
		}

		Sleep(1);
	}

	return 0;
}

DWORD WINAPI KeyBufferThread(LPVOID lpArg)
{
	// check for keys
	for(;;)
	{
		if(dwGlobal_StopKeyBuffer != 0)
		{
			break;
		}

		EnterCriticalSection(&Global_KeyBufferCriticalSection);

		// check all keys
		for(DWORD i = 0; i < MAX_VIRTUAL_KEY_COUNT; i++)
		{
			if(i == VK_LBUTTON || i == VK_RBUTTON || i == VK_MBUTTON)
			{
				// ignore mouse buttons
				continue;
			}

			// check if key is currently down
			if(((GetAsyncKeyState(i) >> 15) & 1) == 1)
			{
				if(bGlobal_KeyState[i] == 0)
				{
					// new key press
					bGlobal_KeyState[i] = 1;

					// check if the DOS terminal is in focus
					if(GetConsoleWindow() == GetForegroundWindow())
					{
						// add to key buffer
						AddToKeyBuffer((BYTE)i);
					}
				}
			}
			else
			{
				if(bGlobal_KeyState[i] != 0)
				{
					// key up
					bGlobal_KeyState[i] = 0;
				}
			}
		}

		LeaveCriticalSection(&Global_KeyBufferCriticalSection);

		// wait 10ms
		Sleep(10);
	}

	return 0;
}

DWORD InitialiseKeyBuffer()
{
	// initialise keybuffer
	memset(bGlobal_KeyState, 0, sizeof(bGlobal_KeyState));
	memset(bGlobal_KeyBuffer, 0, sizeof(bGlobal_KeyBuffer));
	dwGlobal_CurrKeyBufferCount = 0;

	InitializeCriticalSection(&Global_KeyBufferCriticalSection);

	hGlobal_KeyBufferThread = CreateThread(NULL, 0, KeyBufferThread, NULL, 0, NULL);
	if(hGlobal_KeyBufferThread == NULL)
	{
		DeleteCriticalSection(&Global_KeyBufferCriticalSection);
		return 1;
	}

	return 0;
}

DWORD CloseKeyBuffer()
{
	dwGlobal_StopKeyBuffer = 1;
	WaitForSingleObject(hGlobal_KeyBufferThread, INFINITE);
	CloseHandle(hGlobal_KeyBufferThread);

	DeleteCriticalSection(&Global_KeyBufferCriticalSection);

	return 0;
}
