#define _CRT_SECURE_NO_WARNINGS
#define _WIN32_WINNT 0x0500
#include <stdio.h>
#include <windows.h>
#include "WinHV.h"

#define QWORD UINT64

#define SINGLE_STEP_ENABLED 0

#define DOS_PAGE_SIZE 0x200
#define DOS_PARAGRAPH_SIZE 0x10

#define INTERRUPT_COUNT 0x100

#define SEGMENT_TYPE_CODE 11
#define SEGMENT_TYPE_DATA 3

#define SEG_TO_LINEAR(SEG, OFFSET) (((SEG) << 4) + OFFSET)
#define LINEAR_TO_SEG(LINEAR) (WORD)((LINEAR) >> 4)
#define LINEAR_TO_OFFSET(LINEAR) (WORD)((LINEAR) & 0xF)

#define MAX_VIRTUAL_KEY_COUNT 0xFF

#define MAX_KEY_BUFFER_LENGTH 64

#define LOG_PIPE_NAME "\\\\.\\pipe\\DosVisorLog"

struct ProgramSegmentPrefixStruct
{
	BYTE bReserved[0x100];
};

struct DosSystemMemoryStruct
{
	// 0x0 -> IVT
	DWORD dwInterruptVectorTable[INTERRUPT_COUNT];

	// 0x400 -> BIOS data
	BYTE bReservedBIOS[0x100];

	// 0x500 -> interrupt handler CPUID stubs (start of conventional memory)
	WORD wInterruptHandlerCpuid[INTERRUPT_COUNT];

	// 0x700 -> PSP
	ProgramSegmentPrefixStruct PSP;

	// 0x800 -> program data (remaining conventional memory)
	BYTE bProgramMemory[0x7F800];

	// 0x80000 -> extended BIOS data
	BYTE bReservedBIOS_Extended[0x20000];
};

struct CpuRegisterStateStruct
{
	// RAX
	union
	{
		struct
		{
			union
			{
				struct
				{
					BYTE AL;
					BYTE AH;
				};
				WORD AX;
			};
		};
		DWORD EAX;
		QWORD RAX;
	};

	// RCX
	union
	{
		struct
		{
			union
			{
				struct
				{
					BYTE CL;
					BYTE CH;
				};
				WORD CX;
			};
		};
		DWORD ECX;
		QWORD RCX;
	};

	// RDX
	union
	{
		struct
		{
			union
			{
				struct
				{
					BYTE DL;
					BYTE DH;
				};
				WORD DX;
			};
		};
		DWORD EDX;
		QWORD RDX;
	};

	// RBX
	union
	{
		struct
		{
			union
			{
				struct
				{
					BYTE BL;
					BYTE BH;
				};
				WORD BX;
			};
		};
		DWORD EBX;
		QWORD RBX;
	};

	// RSP
	union
	{
		WORD SP;
		DWORD ESP;
		QWORD RSP;
	};

	// RBP
	union
	{
		WORD BP;
		DWORD EBP;
		QWORD RBP;
	};

	// RSI
	union
	{
		WORD SI;
		DWORD ESI;
		QWORD RSI;
	};

	// RDI
	union
	{
		WORD DI;
		DWORD EDI;
		QWORD RDI;
	};

	// RIP
	union
	{
		WORD IP;
		DWORD EIP;
		QWORD RIP;
	};

	// FS/GS are not necessary because they didn't exist on CPUs from this era
	WORD ES;
	WORD CS;
	WORD SS;
	WORD DS;

	// flags
	QWORD RFLAGS;
};

struct VmExitStateStruct
{
	CpuRegisterStateStruct *pCpuRegisterState;
	DosSystemMemoryStruct *pDosSystemMemory;
	WHV_RUN_VP_EXIT_CONTEXT *pVmExitContext;
};

struct InterruptHandlerStruct
{
	BYTE bInterruptIndex;
	DWORD (*pInterruptHandler)(CpuRegisterStateStruct *pCpuRegisterState, DosSystemMemoryStruct *pDosSystemMemory);
};

struct ImportFunctionStruct
{
	char *pName;
	void **pFunctionPtrAddr;
};

extern DWORD Terminal_SetSize(DWORD dwWidth, DWORD dwHeight);
extern DWORD Terminal_WriteChar(char cChar, DWORD dwUpdateColour, WORD wColour);
extern DWORD Terminal_SetCursorPos(DWORD dwX, DWORD dwY);
extern DWORD InitialiseLogServer();
extern DWORD WriteLog(char *pStringFormat, ...);
extern DWORD LogClient();
extern DWORD Terminal_Clear();
extern DWORD LoadProgram(DosSystemMemoryStruct *pDosSystemMemory, char *pExeFilePath, WORD *pwExeBaseSegment);
extern DWORD LaunchHypervisor(char *pExeFilePath);
extern DWORD VmExit_Cpuid(VmExitStateStruct *pVmExitState);
extern DWORD VmExit_Halt(VmExitStateStruct *pVmExitState);
extern DWORD VmExit_IoPort(VmExitStateStruct *pVmExitState);
extern DWORD VmExit_MemoryAccess(VmExitStateStruct *pVmExitState);
extern DWORD InterruptHandler_0x10(CpuRegisterStateStruct *pCpuRegisterState, DosSystemMemoryStruct *pDosSystemMemory);
extern DWORD InterruptHandler_0x16(CpuRegisterStateStruct *pCpuRegisterState, DosSystemMemoryStruct *pDosSystemMemory);
extern DWORD InterruptHandler_0x1A(CpuRegisterStateStruct *pCpuRegisterState, DosSystemMemoryStruct *pDosSystemMemory);
extern DWORD InterruptHandler_0x21(CpuRegisterStateStruct *pCpuRegisterState, DosSystemMemoryStruct *pDosSystemMemory);
extern DWORD InterruptHandler_0x01(CpuRegisterStateStruct *pCpuRegisterState, DosSystemMemoryStruct *pDosSystemMemory);
extern DWORD ValidateHostDataPointer(DosSystemMemoryStruct *pDosSystemMemory, BYTE *pHostDataPointer);
extern DWORD ValidateHostDataPointerLength(DosSystemMemoryStruct *pDosSystemMemory, BYTE *pHostDataPointer, DWORD dwLength);
extern BYTE *GetHostDataPointer(DosSystemMemoryStruct *pDosSystemMemory, DWORD dwLinearAddress, DWORD dwValidateLength);
extern DWORD dwGlobal_DebugTrace;
extern DWORD dwGlobal_DisableLog;
extern DWORD InitialiseKeyBuffer();
extern DWORD CloseKeyBuffer();
extern DWORD WaitForKeyPress(DWORD dwRemoveFromKeyBuffer, BYTE *pbVirtualKey);
extern DWORD CheckKeyWaiting(DWORD dwRemoveFromKeyBuffer, BYTE *pbVirtualKey);
extern DWORD GetKeyInfo(BYTE bVirtualKey, WORD *pwScanCode, BYTE *pbAscii);
extern DWORD InitialiseBiosTimer(DosSystemMemoryStruct *pDosSystemMemory);
extern DWORD CloseBiosTimer();
extern QWORD GetElapsedTimeMicroseconds();
extern DWORD CloseLogServer();
extern DWORD LogSingleStep(CpuRegisterStateStruct *pCpuRegisterState);
extern DWORD dwGlobal_DebugInterrupts;
