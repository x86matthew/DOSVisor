#define _CRT_SECURE_NO_WARNINGS
#include <stdio.h>
#include <windows.h>
#include "DosVisor.h"

DWORD Terminal_SetSize(DWORD dwWidth, DWORD dwHeight)
{
	char szCmd[128];

	// SetConsoleWindowInfo / SetConsoleScreenBufferSize should be used to
	// resize the console window but they have some "undocumented oddities"
	// which are not worth addressing in this example.
	memset(szCmd, 0, sizeof(szCmd));
	_snprintf(szCmd, sizeof(szCmd) - 1, "mode con cols=%u lines=%u", dwWidth, dwHeight);
	system(szCmd);

	return 0;
}

DWORD Terminal_Clear()
{
	// clear screen
	system("cls");

	return 0;
}

DWORD Terminal_WriteChar(char cChar, DWORD dwUpdateColour, WORD wColour)
{
	HANDLE hConsole = NULL;
	DWORD dwRead = 0;
	CONSOLE_SCREEN_BUFFER_INFO ConsoleScreenBufferInfo;
	WORD wExistingColour = 0;

	hConsole = GetStdHandle(STD_OUTPUT_HANDLE);

	if(dwUpdateColour != 0)
	{
		// update colour
		SetConsoleTextAttribute(hConsole, wColour);
	}
	else
	{
		// use existing colour
		memset(&ConsoleScreenBufferInfo, 0, sizeof(ConsoleScreenBufferInfo));
		GetConsoleScreenBufferInfo(hConsole, &ConsoleScreenBufferInfo);
		ReadConsoleOutputAttribute(hConsole, &wExistingColour, 1, ConsoleScreenBufferInfo.dwCursorPosition, &dwRead);
		SetConsoleTextAttribute(hConsole, wExistingColour);
	}

	// print character
	printf("%c", cChar);

	// return to default colours
	SetConsoleTextAttribute(hConsole, 0x7);

	return 0;
}

DWORD Terminal_SetCursorPos(DWORD dwX, DWORD dwY)
{
	HANDLE hConsole = NULL;
	COORD CharPosition;

	hConsole = GetStdHandle(STD_OUTPUT_HANDLE);

	// set cursor position
	CharPosition.X = (short)dwX;
	CharPosition.Y = (short)dwY;
	SetConsoleCursorPosition(hConsole, CharPosition);

	return 0;
}
