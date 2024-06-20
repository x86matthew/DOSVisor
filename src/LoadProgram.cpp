#include "DosVisor.h"

DWORD LoadFileIntoMemory(char *pPath, BYTE **pFileData, DWORD *pdwFileSize)
{
	HANDLE hFile = NULL;
	DWORD dwFileSize = 0;
	BYTE *pFileDataBuffer = NULL;
	DWORD dwBytesRead = 0;
 
	// open file
	hFile = CreateFileA(pPath, GENERIC_READ, 0, NULL, OPEN_EXISTING, 0, NULL);
	if(hFile == INVALID_HANDLE_VALUE)
	{
		return 1;
	}
 
	// calculate file size
	dwFileSize = GetFileSize(hFile, NULL);
 
	// allocate buffer
	pFileDataBuffer = (BYTE*)malloc(dwFileSize);
	if(pFileDataBuffer == NULL)
	{
		CloseHandle(hFile);
		return 1;
	}
 
	// read file contents
	if(ReadFile(hFile, pFileDataBuffer, dwFileSize, &dwBytesRead, NULL) == 0)
	{
		CloseHandle(hFile);
		free(pFileDataBuffer);
		return 1;
	}
 
	// verify byte count
	if(dwBytesRead != dwFileSize)
	{
		CloseHandle(hFile);
		free(pFileDataBuffer);
		return 1;
	}
 
	// close file handle
	CloseHandle(hFile);
 
	// store values
	*pFileData = pFileDataBuffer;
	*pdwFileSize = dwFileSize;
 
	return 0;
}

DWORD FixRelocs(DosSystemMemoryStruct *pDosSystemMemory, IMAGE_DOS_HEADER *pDosHeader, WORD wExeBaseSegment)
{
	DWORD *pdwCurrRelocEntry = NULL;
	WORD wCurrRelocSegment = 0;
	WORD wCurrRelocOffset = 0;
	WORD *pwCurrRelocPtr = NULL;

	pdwCurrRelocEntry = (DWORD*)((BYTE*)pDosHeader + pDosHeader->e_lfarlc);
	for(DWORD i = 0; i < pDosHeader->e_crlc; i++)
	{
		// extract reloc info
		wCurrRelocSegment = HIWORD(*pdwCurrRelocEntry);
		wCurrRelocOffset = LOWORD(*pdwCurrRelocEntry);

		// get host ptr to current reloc entry
		pwCurrRelocPtr = (WORD*)GetHostDataPointer(pDosSystemMemory, SEG_TO_LINEAR(wExeBaseSegment + wCurrRelocSegment, wCurrRelocOffset), sizeof(WORD));
		if(pwCurrRelocPtr == NULL)
		{
			return 1;
		}

		// fix reloc
		*pwCurrRelocPtr += wExeBaseSegment;

		// move to the next entry
		pdwCurrRelocEntry++;
	}

	return 0;
}

DWORD LoadProgram(DosSystemMemoryStruct *pDosSystemMemory, char *pExeFilePath, WORD *pwExeBaseSegment)
{
	BYTE *pExeFileData = NULL;
	DWORD dwExeFileSize = 0;
	IMAGE_DOS_HEADER *pDosHeader = NULL;
	DWORD dwMapImageSize = 0;
	DWORD dwTotalProgramAlloc = 0;
	DWORD dwHeaderSize = 0;
	DWORD dwExeBaseLinear = 0;
	WORD wExeBaseSegment = 0;

	// load exe from disk
	if(LoadFileIntoMemory(pExeFilePath, &pExeFileData, &dwExeFileSize) != 0)
	{
		return 1;
	}

	// validate file size
	if(dwExeFileSize < sizeof(IMAGE_DOS_HEADER))
	{
		free(pExeFileData);
		return 1;
	}

	// validate DOS header
	pDosHeader = (IMAGE_DOS_HEADER*)pExeFileData;
	if(pDosHeader->e_magic != IMAGE_DOS_SIGNATURE)
	{
		free(pExeFileData);
		return 1;
	}

	// calculate the total size to map - this should match the file size in most cases.
	// however, it may be smaller than the file size if extra data is appended after
	// the DOS executable (eg in the case of DOS/win32 mixed binaries). it should never
	// be larger than the actual file size.
	dwMapImageSize = ((pDosHeader->e_cp - 1) * DOS_PAGE_SIZE) + pDosHeader->e_cblp;
	if(dwMapImageSize > dwExeFileSize)
	{
		free(pExeFileData);
		return 1;
	}

	// calculate total amount of memory to allocate -> image size + e_minalloc
	dwTotalProgramAlloc = dwMapImageSize + (pDosHeader->e_minalloc * DOS_PARAGRAPH_SIZE);
	if(dwTotalProgramAlloc > sizeof(pDosSystemMemory->bProgramMemory))
	{
		free(pExeFileData);
		return 1;
	}

	// copy program into VM memory
	memcpy(pDosSystemMemory->bProgramMemory, pExeFileData, dwMapImageSize);

	// calculate header size (including relocs)
	dwHeaderSize = pDosHeader->e_cparhdr * DOS_PARAGRAPH_SIZE;

	// get linear exe content base address (after headers)
	dwExeBaseLinear = (DWORD)((BYTE*)&pDosSystemMemory->bProgramMemory[dwHeaderSize] - (BYTE*)pDosSystemMemory);

	// convert linear address to segmented format
	// (ensure that it isn't based at an unaligned address - this shouldn't happen)
	wExeBaseSegment = LINEAR_TO_SEG(dwExeBaseLinear);
	if(LINEAR_TO_OFFSET(dwExeBaseLinear) != 0)
	{
		free(pExeFileData);
		return 1;
	}

	// fix relocs
	if(FixRelocs(pDosSystemMemory, pDosHeader, wExeBaseSegment) != 0)
	{
		free(pExeFileData);
		return 1;
	}

	// free original file data
	free(pExeFileData);

	// store exe base segment
	*pwExeBaseSegment = wExeBaseSegment;

	return 0;
}
