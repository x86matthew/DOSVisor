#include "DosVisor.h"

HRESULT (WINAPI *WHvCreatePartition)(WHV_PARTITION_HANDLE* Partition) = NULL;
HRESULT (WINAPI *WHvDeletePartition)(WHV_PARTITION_HANDLE Partition) = NULL;
HRESULT (WINAPI *WHvMapGpaRange)(WHV_PARTITION_HANDLE Partition, VOID* SourceAddress, WHV_GUEST_PHYSICAL_ADDRESS GuestAddress, UINT64 SizeInBytes, WHV_MAP_GPA_RANGE_FLAGS Flags) = NULL;
HRESULT (WINAPI *WHvSetVirtualProcessorRegisters)(WHV_PARTITION_HANDLE Partition, UINT32 VpIndex, const WHV_REGISTER_NAME* RegisterNames, UINT32 RegisterCount, const WHV_REGISTER_VALUE* RegisterValues) = NULL;
HRESULT (WINAPI *WHvRunVirtualProcessor)(WHV_PARTITION_HANDLE Partition, UINT32 VpIndex, VOID* ExitContext, UINT32 ExitContextSizeInBytes) = NULL;
HRESULT (WINAPI *WHvSetPartitionProperty)(WHV_PARTITION_HANDLE Partition, WHV_PARTITION_PROPERTY_CODE PropertyCode, const VOID* PropertyBuffer, UINT32 PropertyBufferSizeInBytes) = NULL;
HRESULT (WINAPI *WHvSetupPartition)(WHV_PARTITION_HANDLE Partition) = NULL;
HRESULT (WINAPI *WHvCreateVirtualProcessor)(WHV_PARTITION_HANDLE Partition, UINT32 VpIndex, UINT32 Flags) = NULL;
HRESULT (WINAPI *WHvGetVirtualProcessorRegisters)(WHV_PARTITION_HANDLE Partition, UINT32 VpIndex, const WHV_REGISTER_NAME* RegisterNames, UINT32 RegisterCount, WHV_REGISTER_VALUE* RegisterValues) = NULL;
HRESULT (WINAPI *WHvGetCapability)(WHV_CAPABILITY_CODE CapabilityCode, VOID* CapabilityBuffer, UINT32 CapabilityBufferSizeInBytes, UINT32 *WrittenSizeInBytes) = NULL;

ImportFunctionStruct Global_ImportHypervisorPlatformFunctionList[] =
{
	{ "WHvCreatePartition", (void**)&WHvCreatePartition },
	{ "WHvDeletePartition", (void**)&WHvDeletePartition },
	{ "WHvMapGpaRange", (void**)&WHvMapGpaRange },
	{ "WHvSetVirtualProcessorRegisters", (void**)&WHvSetVirtualProcessorRegisters },
	{ "WHvRunVirtualProcessor", (void**)&WHvRunVirtualProcessor },
	{ "WHvSetPartitionProperty", (void**)&WHvSetPartitionProperty },
	{ "WHvSetupPartition", (void**)&WHvSetupPartition },
	{ "WHvCreateVirtualProcessor", (void**)&WHvCreateVirtualProcessor },
	{ "WHvGetVirtualProcessorRegisters", (void**)&WHvGetVirtualProcessorRegisters },
	{ "WHvGetCapability", (void**)&WHvGetCapability },
};

// enable this flag to single-step the target program
DWORD dwGlobal_DebugTrace = 0;

// enable this flag to log all interrupts
DWORD dwGlobal_DebugInterrupts = 0;

DWORD InitialiseHypervisorPlatform()
{
	HMODULE hModule = NULL;
	void *pImportAddr = NULL;
	DWORD dwFunctionCount = 0;
	WHV_CAPABILITY HypervisorCapability;
	UINT32 dwHypervisorCapabilitySize = 0;

	// load hypervisor module
	hModule = LoadLibraryA("winhvplatform.dll");
	if(hModule == NULL)
	{
		return 1;
	}

	// resolve imported functions
	dwFunctionCount = sizeof(Global_ImportHypervisorPlatformFunctionList) / sizeof(Global_ImportHypervisorPlatformFunctionList[0]);
	for(DWORD i = 0; i < dwFunctionCount; i++)
	{
		// resolve current function
		pImportAddr = GetProcAddress(hModule, Global_ImportHypervisorPlatformFunctionList[i].pName);
		if(pImportAddr == NULL)
		{
			return 1;
		}

		// store function ptr
		*Global_ImportHypervisorPlatformFunctionList[i].pFunctionPtrAddr = pImportAddr;
	}

	// ensure the hypervisor platform is enabled
	memset(&HypervisorCapability, 0, sizeof(HypervisorCapability));
	if(WHvGetCapability(WHvCapabilityCodeHypervisorPresent, &HypervisorCapability, sizeof(HypervisorCapability), &dwHypervisorCapabilitySize) != S_OK)
	{
		return 1;
	}
	if(HypervisorCapability.HypervisorPresent == 0)
	{
		return 1;
	}

	return 0;
}

WHV_PARTITION_HANDLE CreateVirtualCPU()
{
	WHV_PARTITION_HANDLE hPartitionHandle = NULL;
	WHV_PARTITION_PROPERTY PartitionPropertyData;
	WHV_EXTENDED_VM_EXITS ExtendedVmExits;

	// create hypervisor partition
	if(WHvCreatePartition(&hPartitionHandle) != S_OK)
	{
		return NULL;
	}

	// single processor
	memset(&PartitionPropertyData, 0, sizeof(PartitionPropertyData));
	PartitionPropertyData.ProcessorCount = 1;
	if(WHvSetPartitionProperty(hPartitionHandle, WHvPartitionPropertyCodeProcessorCount, &PartitionPropertyData, sizeof(PartitionPropertyData)) != S_OK)
	{
		WHvDeletePartition(hPartitionHandle);
		return NULL;
	}

	// enable vmexit for cpuid instruction
	memset(&ExtendedVmExits, 0, sizeof(ExtendedVmExits));
	ExtendedVmExits.X64CpuidExit = 1;
	if(WHvSetPartitionProperty(hPartitionHandle, WHvPartitionPropertyCodeExtendedVmExits, &ExtendedVmExits, sizeof(ExtendedVmExits)) != S_OK)
	{
		WHvDeletePartition(hPartitionHandle);
		return NULL;
	}

	// hypervisor partition ready
	if(WHvSetupPartition(hPartitionHandle) != S_OK)
	{
		WHvDeletePartition(hPartitionHandle);
		return NULL;
	}

	// create virtual CPU
	if(WHvCreateVirtualProcessor(hPartitionHandle, 0, 0) != S_OK)
	{
		WHvDeletePartition(hPartitionHandle);
		return NULL;
	}

	return hPartitionHandle;
}

BYTE *AllocateSharedMemory(WHV_PARTITION_HANDLE hPartitionHandle, BYTE *pGuestBaseAddress, DWORD dwSize)
{
	BYTE *pMemory = NULL;

	// allocate memory on host
	pMemory = (BYTE*)VirtualAlloc(NULL, dwSize, MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE);
	if(pMemory == NULL)
	{
		return NULL;
	}

	// share allocated memory with guest
	if(WHvMapGpaRange(hPartitionHandle, (LPVOID)pMemory, (WHV_GUEST_PHYSICAL_ADDRESS)pGuestBaseAddress, dwSize, WHvMapGpaRangeFlagRead | WHvMapGpaRangeFlagWrite | WHvMapGpaRangeFlagExecute) != S_OK)
	{
		VirtualFree(pMemory, 0, MEM_RELEASE);
		return NULL;
	}

	return pMemory;
}

DWORD GetRegisterValue_64(WHV_PARTITION_HANDLE hPartitionHandle, WHV_REGISTER_NAME RegisterName, QWORD *pqwRegisterValue)
{
	WHV_REGISTER_VALUE RegisterValue;

	// get uint64 register value
	memset(&RegisterValue, 0, sizeof(RegisterValue));
	if(WHvGetVirtualProcessorRegisters(hPartitionHandle, 0, &RegisterName, 1, &RegisterValue) != S_OK)
	{
		return 1;
	}

	*pqwRegisterValue = RegisterValue.Reg64;

	return 0;
}

DWORD GetRegisterValue_Segment(WHV_PARTITION_HANDLE hPartitionHandle, WHV_REGISTER_NAME RegisterName, WORD *pwSegmentValue)
{
	WHV_REGISTER_VALUE RegisterValue;

	// get segment register value
	memset(&RegisterValue, 0, sizeof(RegisterValue));
	if(WHvGetVirtualProcessorRegisters(hPartitionHandle, 0, &RegisterName, 1, &RegisterValue) != S_OK)
	{
		return 1;
	}

	*pwSegmentValue = RegisterValue.Segment.Selector;

	return 0;
}

DWORD SetRegisterValue_64(WHV_PARTITION_HANDLE hPartitionHandle, WHV_REGISTER_NAME RegisterName, QWORD qwRegisterValue)
{
	WHV_REGISTER_VALUE RegisterValue;

	// set uint64 register value
	memset(&RegisterValue, 0, sizeof(RegisterValue));
	RegisterValue.Reg64 = qwRegisterValue;
	if(WHvSetVirtualProcessorRegisters(hPartitionHandle, 0, &RegisterName, 1, &RegisterValue) != S_OK)
	{
		return 1;
	}

	return 0;
}

DWORD SetRegisterValue_Segment(WHV_PARTITION_HANDLE hPartitionHandle, WHV_REGISTER_NAME RegisterName, WORD wSegmentValue, WORD wSegmentType)
{
	WHV_REGISTER_VALUE RegisterValue;

	// set segment register value
	memset(&RegisterValue, 0, sizeof(RegisterValue));
	RegisterValue.Segment.Base = (DWORD)wSegmentValue << 4;
	RegisterValue.Segment.Limit = 0xFFFF;
	RegisterValue.Segment.Selector = wSegmentValue;
	RegisterValue.Segment.SegmentType = wSegmentType;
	RegisterValue.Segment.NonSystemSegment = 1;
	RegisterValue.Segment.DescriptorPrivilegeLevel = 0;
	RegisterValue.Segment.Present = 1;
	RegisterValue.Segment.Reserved = 0;
	RegisterValue.Segment.Available = 1;
	RegisterValue.Segment.Long = 0;
	RegisterValue.Segment.Default = 0;
	RegisterValue.Segment.Granularity = 0;
	if(WHvSetVirtualProcessorRegisters(hPartitionHandle, 0, &RegisterName, 1, &RegisterValue) != S_OK)
	{
		return 1;
	}

	return 0;
}

DWORD GetRegisters(WHV_PARTITION_HANDLE hPartitionHandle, CpuRegisterStateStruct *pCpuRegisterState)
{
	CpuRegisterStateStruct CpuRegisterState;

	// get register values
	memset(&CpuRegisterState, 0, sizeof(CpuRegisterState));
	GetRegisterValue_64(hPartitionHandle, WHvX64RegisterRax, &CpuRegisterState.RAX);
	GetRegisterValue_64(hPartitionHandle, WHvX64RegisterRcx, &CpuRegisterState.RCX);
	GetRegisterValue_64(hPartitionHandle, WHvX64RegisterRdx, &CpuRegisterState.RDX);
	GetRegisterValue_64(hPartitionHandle, WHvX64RegisterRbx, &CpuRegisterState.RBX);
	GetRegisterValue_64(hPartitionHandle, WHvX64RegisterRsp, &CpuRegisterState.RSP);
	GetRegisterValue_64(hPartitionHandle, WHvX64RegisterRbp, &CpuRegisterState.RBP);
	GetRegisterValue_64(hPartitionHandle, WHvX64RegisterRsi, &CpuRegisterState.RSI);
	GetRegisterValue_64(hPartitionHandle, WHvX64RegisterRdi, &CpuRegisterState.RDI);
	GetRegisterValue_64(hPartitionHandle, WHvX64RegisterRip, &CpuRegisterState.RIP);
	GetRegisterValue_Segment(hPartitionHandle, WHvX64RegisterEs, &CpuRegisterState.ES);
	GetRegisterValue_Segment(hPartitionHandle, WHvX64RegisterCs, &CpuRegisterState.CS);
	GetRegisterValue_Segment(hPartitionHandle, WHvX64RegisterSs, &CpuRegisterState.SS);
	GetRegisterValue_Segment(hPartitionHandle, WHvX64RegisterDs, &CpuRegisterState.DS);
	GetRegisterValue_64(hPartitionHandle, WHvX64RegisterRflags, &CpuRegisterState.RFLAGS);
	memcpy(pCpuRegisterState, &CpuRegisterState, sizeof(CpuRegisterState));

	return 0;
}

DWORD SetRegisters(WHV_PARTITION_HANDLE hPartitionHandle, CpuRegisterStateStruct *pCpuRegisterState)
{
	// set register values
	SetRegisterValue_64(hPartitionHandle, WHvX64RegisterRax, pCpuRegisterState->RAX);
	SetRegisterValue_64(hPartitionHandle, WHvX64RegisterRcx, pCpuRegisterState->RCX);
	SetRegisterValue_64(hPartitionHandle, WHvX64RegisterRdx, pCpuRegisterState->RDX);
	SetRegisterValue_64(hPartitionHandle, WHvX64RegisterRbx, pCpuRegisterState->RBX);
	SetRegisterValue_64(hPartitionHandle, WHvX64RegisterRsp, pCpuRegisterState->RSP);
	SetRegisterValue_64(hPartitionHandle, WHvX64RegisterRbp, pCpuRegisterState->RBP);
	SetRegisterValue_64(hPartitionHandle, WHvX64RegisterRsi, pCpuRegisterState->RSI);
	SetRegisterValue_64(hPartitionHandle, WHvX64RegisterRdi, pCpuRegisterState->RDI);
	SetRegisterValue_64(hPartitionHandle, WHvX64RegisterRip, pCpuRegisterState->RIP);
	SetRegisterValue_Segment(hPartitionHandle, WHvX64RegisterEs, pCpuRegisterState->ES, SEGMENT_TYPE_DATA);
	SetRegisterValue_Segment(hPartitionHandle, WHvX64RegisterCs, pCpuRegisterState->CS, SEGMENT_TYPE_CODE);
	SetRegisterValue_Segment(hPartitionHandle, WHvX64RegisterSs, pCpuRegisterState->SS, SEGMENT_TYPE_DATA);
	SetRegisterValue_Segment(hPartitionHandle, WHvX64RegisterDs, pCpuRegisterState->DS, SEGMENT_TYPE_DATA);
	SetRegisterValue_64(hPartitionHandle, WHvX64RegisterRflags, pCpuRegisterState->RFLAGS);

	return 0;
}

DWORD ValidateHostDataPointer(DosSystemMemoryStruct *pDosSystemMemory, BYTE *pHostDataPointer)
{
	BYTE *pPtr = NULL;

	// ensure the address is within the mapped range
	if((QWORD)pHostDataPointer < (QWORD)pDosSystemMemory)
	{
		return 1;
	}

	// ensure the address is within the mapped range
	if((QWORD)((QWORD)pHostDataPointer - (QWORD)pDosSystemMemory) >= sizeof(DosSystemMemoryStruct))
	{
		return 1;
	}

	return 0;
}

DWORD ValidateHostDataPointerLength(DosSystemMemoryStruct *pDosSystemMemory, BYTE *pHostDataPointer, DWORD dwLength)
{
	// at least 1 byte must be validated
	if(dwLength == 0)
	{
		return 1;
	}

	// validate first byte in block
	if(ValidateHostDataPointer(pDosSystemMemory, pHostDataPointer) != 0)
	{
		return 1;
	}

	// validate last byte in block
	if(ValidateHostDataPointer(pDosSystemMemory, pHostDataPointer + dwLength - 1) != 0)
	{
		return 1;
	}

	return 0;
}

BYTE *GetHostDataPointer(DosSystemMemoryStruct *pDosSystemMemory, DWORD dwLinearAddress, DWORD dwValidateLength)
{
	BYTE *pPtr = NULL;

	// get host pointer
	pPtr = (BYTE*)pDosSystemMemory + dwLinearAddress;

	// validate entire block
	if(ValidateHostDataPointerLength(pDosSystemMemory, pPtr, dwValidateLength) != 0)
	{
		return NULL;
	}

	return pPtr;
}

DWORD LogSingleStep(CpuRegisterStateStruct *pCpuRegisterState)
{
	// log current instruction
	WriteLog("[%04X:%04X]\n", pCpuRegisterState->CS, pCpuRegisterState->IP);

	return 0;
}

DWORD PrepareHypervisorEnvironment(WHV_PARTITION_HANDLE hPartitionHandle, char *pExeFilePath, CpuRegisterStateStruct *pCpuRegisterState, DosSystemMemoryStruct **ppDosSystemMemory)
{
	CpuRegisterStateStruct CpuRegisterState;
	IMAGE_DOS_HEADER *pDosHeader = NULL;
	WORD wExeBaseSegment = 0;
	DWORD dwLinearOffset = 0;
	DWORD dwPspBaseLinear = 0;
	WORD wPspBaseSegment = 0;
	DosSystemMemoryStruct *pDosSystemMemory = NULL;

	// DOS memory layout should be 640kb
	if(sizeof(DosSystemMemoryStruct) != (640 * 1024))
	{
		return 1;
	}

	// allocate DOS system memory (shared with guest)
	pDosSystemMemory = (DosSystemMemoryStruct*)AllocateSharedMemory(hPartitionHandle, (BYTE*)0, sizeof(DosSystemMemoryStruct));
	if(pDosSystemMemory == NULL)
	{
		WriteLog("Error: Failed to allocate shared memory\n");
		return 1;
	}

	// initialise interrupts
	for(DWORD i = 0; i < INTERRUPT_COUNT; i++)
	{
		// write a single CPUID instruction to the current interrupt handler
		pDosSystemMemory->wInterruptHandlerCpuid[i] = 0xA20F;

		// set interrupt handler address in IVT
		dwLinearOffset = (DWORD)((BYTE*)&pDosSystemMemory->wInterruptHandlerCpuid[i] - (BYTE*)pDosSystemMemory);
		pDosSystemMemory->dwInterruptVectorTable[i] = MAKELONG(LINEAR_TO_OFFSET(dwLinearOffset), LINEAR_TO_SEG(dwLinearOffset));
	}

	// load program
	if(LoadProgram(pDosSystemMemory, pExeFilePath, &wExeBaseSegment) != 0)
	{
		WriteLog("Error: Failed to load program (%s)\n", pExeFilePath);
		VirtualFree(pDosSystemMemory, 0, MEM_RELEASE);
		return 1;
	}

	// get ptr to DOS header in mapped image
	pDosHeader = (IMAGE_DOS_HEADER*)pDosSystemMemory->bProgramMemory;

	// initialise PSP (not currently implemented - add fields as necessary)
	memset(pDosSystemMemory->PSP.bReserved, 0, sizeof(pDosSystemMemory->PSP.bReserved));

	// convert linear address to segmented format
	// (ensure that it isn't based at an unaligned address - this shouldn't happen)
	dwPspBaseLinear = (DWORD)((BYTE*)&pDosSystemMemory->PSP - (BYTE*)pDosSystemMemory);
	wPspBaseSegment = LINEAR_TO_SEG(dwPspBaseLinear);
	if(LINEAR_TO_OFFSET(dwPspBaseLinear) != 0)
	{
		VirtualFree(pDosSystemMemory, 0, MEM_RELEASE);
		return 1;
	}

	// set control registers (disable paging)
	SetRegisterValue_64(hPartitionHandle, WHvX64RegisterCr0, 0);
	SetRegisterValue_64(hPartitionHandle, WHvX64RegisterCr2, 0);
	SetRegisterValue_64(hPartitionHandle, WHvX64RegisterCr3, 0);
	SetRegisterValue_64(hPartitionHandle, WHvX64RegisterCr4, 0);

	// set initial register values
	memset(&CpuRegisterState, 0, sizeof(CpuRegisterState));
	GetRegisters(hPartitionHandle, &CpuRegisterState);
	CpuRegisterState.ES = wPspBaseSegment;
	CpuRegisterState.CS = pDosHeader->e_cs + wExeBaseSegment;
	CpuRegisterState.SS = pDosHeader->e_ss + wExeBaseSegment;
	CpuRegisterState.DS = wPspBaseSegment;
	CpuRegisterState.SP = pDosHeader->e_sp;
	CpuRegisterState.IP = pDosHeader->e_ip;
	if(dwGlobal_DebugTrace != 0)
	{
		// set trap flag
		CpuRegisterState.RFLAGS |= 0x100;
	}

	// store register values
	memcpy(pCpuRegisterState, &CpuRegisterState, sizeof(CpuRegisterState));

	// store system memory ptr
	*ppDosSystemMemory = pDosSystemMemory;

	return 0;
}

DWORD LaunchHypervisor(char *pExeFilePath)
{
	WHV_PARTITION_HANDLE hPartitionHandle = NULL;
	WHV_RUN_VP_EXIT_CONTEXT VmExitContext;
	DosSystemMemoryStruct *pDosSystemMemory = NULL;
	CpuRegisterStateStruct CpuRegisterState;
	VmExitStateStruct VmExitState;

	// initialise log pipe
	if(InitialiseLogServer() != 0)
	{
		return 1;
	}

	WriteLog("DOSVisor\n");
	WriteLog(" - x86matthew\n\n");

	// initialise hypervisor platform api
	if(InitialiseHypervisorPlatform() != 0)
	{
		WriteLog("Error: Failed to initialise hypervisor platform\n");
		CloseLogServer();
		return 1;
	}

	// initialise virtual CPU
	hPartitionHandle = CreateVirtualCPU();
	if(hPartitionHandle == NULL)
	{
		WriteLog("Error: Failed to create virtual CPU\n");
		CloseLogServer();
		return 1;
	}

	// prepare environment
	memset(&CpuRegisterState, 0, sizeof(CpuRegisterState));
	if(PrepareHypervisorEnvironment(hPartitionHandle, pExeFilePath, &CpuRegisterState, &pDosSystemMemory) != 0)
	{
		WriteLog("Error: Failed to prepare environment\n");
		WHvDeletePartition(hPartitionHandle);
		CloseLogServer();
		return 1;
	}

	WriteLog("Loaded executable: %s\n", pExeFilePath);

	// initialise BIOS timer
	if(InitialiseBiosTimer(pDosSystemMemory) != 0)
	{
		WriteLog("Error: Failed to initialise BIOS timer\n");
		VirtualFree(pDosSystemMemory, 0, MEM_RELEASE);
		WHvDeletePartition(hPartitionHandle);
		CloseLogServer();
		return 1;
	}

	// initialise key buffer
	if(InitialiseKeyBuffer() != 0)
	{
		WriteLog("Error: Failed to initialise key buffer\n");
		CloseBiosTimer();
		VirtualFree(pDosSystemMemory, 0, MEM_RELEASE);
		WHvDeletePartition(hPartitionHandle);
		CloseLogServer();
		return 1;
	}

	// set terminal size (80x25)
	Terminal_SetSize(80, 25);

	WriteLog("Starting execution...\n");

	// log first instruction if debug trace is enabled
	if(dwGlobal_DebugTrace != 0)
	{
		LogSingleStep(&CpuRegisterState);
	}

	for(;;)
	{
		// set registers before execution
		SetRegisters(hPartitionHandle, &CpuRegisterState);

		// resume virtual CPU
		memset(&VmExitContext, 0, sizeof(VmExitContext));
		if(WHvRunVirtualProcessor(hPartitionHandle, 0, &VmExitContext, sizeof(VmExitContext)) != S_OK)
		{
			break;
		}

		// caught vmexit - get registers
		GetRegisters(hPartitionHandle, &CpuRegisterState);

		// set vmexit state data
		memset(&VmExitState, 0, sizeof(VmExitState));
		VmExitState.pCpuRegisterState = &CpuRegisterState;
		VmExitState.pDosSystemMemory = pDosSystemMemory;
		VmExitState.pVmExitContext = &VmExitContext;

		// check vmexit type
		if(VmExitContext.ExitReason == WHvRunVpExitReasonX64Cpuid)
		{
			// cpuid
			if(VmExit_Cpuid(&VmExitState) != 0)
			{
				WriteLog("Error: VmExit_Cpuid\n", VmExitContext.ExitReason);
				break;
			}
		}
		else if(VmExitContext.ExitReason == WHvRunVpExitReasonX64IoPortAccess)
		{
			// io port (in/out)
			if(VmExit_IoPort(&VmExitState) != 0)
			{
				WriteLog("Error: VmExit_IoPort\n", VmExitContext.ExitReason);
				break;
			}
		}
		else if(VmExitContext.ExitReason == WHvRunVpExitReasonX64Halt)
		{
			// halt
			if(VmExit_Halt(&VmExitState) != 0)
			{
				WriteLog("Error: VmExit_Halt\n", VmExitContext.ExitReason);
				break;
			}
		}
		else if(VmExitContext.ExitReason == WHvRunVpExitReasonMemoryAccess)
		{
			// invalid memory access
			if(VmExit_MemoryAccess(&VmExitState) != 0)
			{
				WriteLog("Error: VmExit_MemoryAccess\n", VmExitContext.ExitReason);
				break;
			}
		}
		else
		{
			WriteLog("Error: Unhandled VmExit (0x%08X)\n", VmExitContext.ExitReason);
			break;
		}
	}

	// flush stdin buffer to prevent any buffered key-presses from being sent to the console after exit
	FlushConsoleInputBuffer(GetStdHandle(STD_INPUT_HANDLE));

	// clean up
	CloseKeyBuffer();
	CloseBiosTimer();
	WHvDeletePartition(hPartitionHandle);
	VirtualFree(pDosSystemMemory, 0, MEM_RELEASE);
	CloseLogServer();

	return 0;
}
