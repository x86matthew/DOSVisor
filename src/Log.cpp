#include "DosVisor.h"

HANDLE hGlobal_LogPipe = NULL;
DWORD dwGlobal_DisableLog = 0;

DWORD LogClient()
{
	BYTE bByte = 0;
	DWORD dwRead = 0;
	HANDLE hPipe = NULL;

	// open named pipe from parent process
	hPipe = CreateFileA(LOG_PIPE_NAME, GENERIC_READ, 0, NULL, OPEN_EXISTING, 0, NULL);
	if(hPipe == INVALID_HANDLE_VALUE)
	{
		return 1;
	}

	// read from named pipe and print to console
	for(;;)
	{
		// get next character
		if(ReadFile(hPipe, &bByte, 1, &dwRead, NULL) == 0)
		{
			break;
		}

		if(dwRead == 0)
		{
			break;
		}

		printf("%c", bByte);
	}

	// finished
	CloseHandle(hPipe);
	system("pause");

	return 0;
}

DWORD InitialiseLogServer()
{
	STARTUPINFOA StartupInfo;
	PROCESS_INFORMATION ProcessInfo;
	char szCurrPath[512];
	char szCmd[512];

	// get current exe path
	memset(szCurrPath, 0, sizeof(szCurrPath));
	if(GetModuleFileNameA(NULL, szCurrPath, sizeof(szCurrPath) - 1) == 0)
	{
		return 1;
	}

	// create logging pipe
	hGlobal_LogPipe = CreateNamedPipeA(LOG_PIPE_NAME, PIPE_ACCESS_OUTBOUND, PIPE_TYPE_BYTE | PIPE_READMODE_BYTE | PIPE_WAIT, 1, 0, 0, 0, NULL);
	if(hGlobal_LogPipe == INVALID_HANDLE_VALUE)
	{
		return 1;
	}

	// create child process as log client
	memset(szCmd, 0, sizeof(szCmd));
	_snprintf(szCmd, sizeof(szCmd) - 1, "\"%s\" *log-client*", szCurrPath);
	memset(&StartupInfo, 0, sizeof(StartupInfo));
	StartupInfo.cb = sizeof(StartupInfo);
	if(CreateProcessA(NULL, szCmd, NULL, NULL, 0, CREATE_NEW_CONSOLE, NULL, NULL, &StartupInfo, &ProcessInfo) == 0)
	{
		CloseHandle(hGlobal_LogPipe);
		return 1;
	}

	// close handles (the child process will already exit when the pipe closes)
	CloseHandle(ProcessInfo.hProcess);
	CloseHandle(ProcessInfo.hThread);

	// wait for child process to connect
	ConnectNamedPipe(hGlobal_LogPipe, NULL);

	return 0;
}

DWORD CloseLogServer()
{
	// close log pipe
	CloseHandle(hGlobal_LogPipe);

	return 0;
}

DWORD WriteLog(char *pStringFormat, ...)
{
	va_list VaList;
	char szBuffer[1024];
	DWORD dwWritten = 0;

	if(dwGlobal_DisableLog != 0)
	{
		// logging disabled
		return 1;
	}

	// format string
	va_start(VaList, pStringFormat);
	memset(szBuffer, 0, sizeof(szBuffer));
	_vsnprintf(szBuffer, sizeof(szBuffer) - 1, pStringFormat, VaList);
	va_end(VaList);

	// write to pipe
	WriteFile(hGlobal_LogPipe, szBuffer, (DWORD)strlen(szBuffer), &dwWritten, NULL);

	return 0;
}
