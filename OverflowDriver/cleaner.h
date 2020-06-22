#pragma once
#include "imports.h"

	PVOID g_KernelBase = NULL;
	ULONG g_KernelSize = 0;

	PVOID resolve_relative_address(_In_ PVOID Instruction, _In_ ULONG OffsetOffset, _In_ ULONG InstructionSize)
	{
		ULONG_PTR Instr = (ULONG_PTR)Instruction;
		LONG RipOffset = *(PLONG)(Instr + OffsetOffset);
		PVOID ResolvedAddr = (PVOID)(Instr + InstructionSize + RipOffset);

		return ResolvedAddr;
	}

	NTSTATUS pattern_scan(IN PCUCHAR pattern, IN UCHAR wildcard, IN ULONG_PTR len, IN const VOID* base, IN ULONG_PTR size, OUT PVOID* ppFound)
	{
		ASSERT(ppFound != NULL && pattern != NULL && base != NULL);
		if (ppFound == NULL || pattern == NULL || base == NULL)
			return STATUS_INVALID_PARAMETER;

		for (ULONG_PTR i = 0; i < size - len; i++)
		{
			BOOLEAN found = TRUE;
			for (ULONG_PTR j = 0; j < len; j++)
			{
				if (pattern[j] != wildcard && pattern[j] != ((PCUCHAR)base)[i + j])
				{
					found = FALSE;
					break;
				}
			}

			if (found != FALSE)
			{
				*ppFound = (PUCHAR)base + i;
				return STATUS_SUCCESS;
			}
		}

		return STATUS_NOT_FOUND;
	}

	PVOID get_kernel_base(OUT PULONG pSize)
	{
		NTSTATUS status = STATUS_SUCCESS;
		ULONG bytes = 0;
		PRTL_PROCESS_MODULES pMods = NULL;
		PVOID checkPtr = NULL;
		UNICODE_STRING routineName;

		// Already found
		if (g_KernelBase != NULL)
		{
			if (pSize)
				*pSize = g_KernelSize;
			return g_KernelBase;
		}

		RtlUnicodeStringInit(&routineName, L"NtOpenFile");

		checkPtr = MmGetSystemRoutineAddress(&routineName);
		if (checkPtr == NULL)
			return NULL;


		status = ZwQuerySystemInformation(SystemModuleInformation, 0, bytes, &bytes);

		if (bytes == 0)
			return NULL;

		pMods = (PRTL_PROCESS_MODULES)ExAllocatePoolWithTag(NonPagedPool, bytes, 0x454E4F45); // 'ENON'
		RtlZeroMemory(pMods, bytes);

		status = ZwQuerySystemInformation(SystemModuleInformation, pMods, bytes, &bytes);

		if (NT_SUCCESS(status))
		{
			PRTL_PROCESS_MODULE_INFORMATION pMod = pMods->Modules;

			for (ULONG i = 0; i < pMods->NumberOfModules; i++)
			{
				// System routine is inside module
				if (checkPtr >= pMod[i].ImageBase &&
					checkPtr < (PVOID)((PUCHAR)pMod[i].ImageBase + pMod[i].ImageSize))
				{
					g_KernelBase = pMod[i].ImageBase;
					g_KernelSize = pMod[i].ImageSize;
					if (pSize)
						*pSize = g_KernelSize;
					break;
				}
			}
		}

		if (pMods)
			ExFreePoolWithTag(pMods, 0x454E4F45); // 'ENON'

		return g_KernelBase;
	}


	NTSTATUS scan_section(IN PCCHAR section, IN PCUCHAR pattern, IN UCHAR wildcard, IN ULONG_PTR len, OUT PVOID* ppFound)
	{
		ASSERT(ppFound != NULL);
		if (ppFound == NULL)
			return STATUS_INVALID_PARAMETER;

		PVOID base = get_kernel_base(NULL);
		if (!base)
			return STATUS_NOT_FOUND;


		PIMAGE_NT_HEADERS64 pHdr = RtlImageNtHeader(base);
		if (!pHdr)
			return STATUS_INVALID_IMAGE_FORMAT;

		PIMAGE_SECTION_HEADER pFirstSection = (PIMAGE_SECTION_HEADER)(pHdr + 1);
		for (PIMAGE_SECTION_HEADER pSection = pFirstSection; pSection < pFirstSection + pHdr->FileHeader.NumberOfSections; pSection++)
		{
			ANSI_STRING s1, s2;
			RtlInitAnsiString(&s1, section);
			RtlInitAnsiString(&s2, (PCCHAR)pSection->Name);
			if (RtlCompareString(&s1, &s2, TRUE) == 0)
			{
				PVOID ptr = NULL;
				NTSTATUS status = pattern_scan(pattern, wildcard, len, (PUCHAR)base + pSection->VirtualAddress, pSection->Misc.VirtualSize, &ptr);
				if (NT_SUCCESS(status))
					*(PULONG)ppFound = (ULONG)((PUCHAR)ptr - (PUCHAR)base);

				return status;
			}
		}

		return STATUS_NOT_FOUND;
	}	
	
	NTSTATUS scan_section_with_base(IN PCCHAR section, IN PCUCHAR pattern, IN UCHAR wildcard, IN ULONG_PTR len, OUT PVOID* ppFound, IN PVOID base)
	{
		ASSERT(ppFound != NULL);
		if (ppFound == NULL)
			return STATUS_INVALID_PARAMETER;

		if (!base)
			return STATUS_NOT_FOUND;

		PIMAGE_NT_HEADERS64 pHdr = RtlImageNtHeader(base);
		if (!pHdr)
			return STATUS_INVALID_IMAGE_FORMAT;

		PIMAGE_SECTION_HEADER pFirstSection = (PIMAGE_SECTION_HEADER)(pHdr + 1);
		for (PIMAGE_SECTION_HEADER pSection = pFirstSection; pSection < pFirstSection + pHdr->FileHeader.NumberOfSections; pSection++)
		{
			ANSI_STRING s1, s2;
			RtlInitAnsiString(&s1, section);
			DbgPrintEx(0, 0, "Section: %s", pSection->Name);
			RtlInitAnsiString(&s2, (PCCHAR)pSection->Name);
			if (RtlCompareString(&s1, &s2, TRUE) == 0)
			{
				PVOID ptr = NULL;
				NTSTATUS status = pattern_scan(pattern, wildcard, len, (PUCHAR)base + pSection->VirtualAddress, pSection->Misc.VirtualSize, &ptr);
				if (NT_SUCCESS(status))
					*(PULONG)ppFound = (ULONG)((PUCHAR)ptr - (PUCHAR)base);

				return status;
			}
		}

		return STATUS_NOT_FOUND;
	}

	BOOLEAN LocatePiDDB(PERESOURCE* lock, PRTL_AVL_TABLE* table)
	{
		UCHAR PiDDBLockPtr_sig[] = "\x48\x8D\x0D\xCC\xCC\xCC\xCC\xE8\xCC\xCC\xCC\xCC\x48\x8B\x0D\xCC\xCC\xCC\xCC\x33\xDB";
		UCHAR PiDTablePtr_sig[] = "\x48\x8D\x0D\xCC\xCC\xCC\xCC\xE8\xCC\xCC\xCC\xCC\x3D\xCC\xCC\xCC\xCC\x0F\x83\xCC\xCC\xCC\xCC";

		PVOID PiDDBLockPtr = NULL;
		if (!NT_SUCCESS(scan_section("PAGE", PiDDBLockPtr_sig, 0xCC, sizeof(PiDDBLockPtr_sig) - 1, (&PiDDBLockPtr)))) {
			return FALSE;
		}

		RtlZeroMemory(PiDDBLockPtr_sig, sizeof(PiDDBLockPtr_sig) - 1);

		PVOID PiDTablePtr = NULL;
		if (!NT_SUCCESS(scan_section("PAGE", PiDTablePtr_sig, 0xCC, sizeof(PiDTablePtr_sig) - 1, (&PiDTablePtr)))) {
			return FALSE;
		}

		RtlZeroMemory(PiDTablePtr_sig, sizeof(PiDTablePtr_sig) - 1);


		UINT64 RealPtrPIDLock = NULL;

		RealPtrPIDLock = (UINT64)g_KernelBase + (UINT64)PiDDBLockPtr;


		*lock = (PERESOURCE)resolve_relative_address((PVOID)RealPtrPIDLock, 3, 7);


		UINT64 RealPtrPIDTable = NULL;

		RealPtrPIDTable = (UINT64)g_KernelBase + (UINT64)PiDTablePtr;


		*table = (PRTL_AVL_TABLE)(resolve_relative_address((PVOID)RealPtrPIDTable, 3, 7));

		return TRUE;
	}

	BOOLEAN clean_piddbcachetalbe() {
		PERESOURCE PiDDBLock = NULL;
		PRTL_AVL_TABLE PiDDBCacheTable = NULL;
		if (!LocatePiDDB(&PiDDBLock, &PiDDBCacheTable) && PiDDBLock == NULL && PiDDBCacheTable == NULL) {
			return FALSE;
		}

		// build a lookup entry

		PIDCacheobj lookupEntry;

		// this should work :D
		UNICODE_STRING DriverName = RTL_CONSTANT_STRING(L"iqvw64e.sys");
		// removed *DriverName no need for it
		lookupEntry.DriverName = DriverName;
		lookupEntry.TimeDateStamp = 0x5284EAC3; // intel_driver TimeStamp.

		// aquire the ddb lock
		ExAcquireResourceExclusiveLite(PiDDBLock, TRUE);

		// search our entry in the table

		// maybe something will bsod here.
		PIDCacheobj* pFoundEntry = (PIDCacheobj*)RtlLookupElementGenericTableAvl(PiDDBCacheTable, &lookupEntry);
		if (pFoundEntry == NULL)
		{
			// release the ddb resource lock
			ExReleaseResourceLite(PiDDBLock);
			return FALSE;
		}
		else
		{
			// first, unlink from the list
			RemoveEntryList(&pFoundEntry->List);
			// then delete the element from the avl table
			RtlDeleteElementGenericTableAvl(PiDDBCacheTable, pFoundEntry);

			// release the ddb resource lock
			ExReleaseResourceLite(PiDDBLock);
		}
		DbgPrintEx(0, 0, "Cleaned piddb\n");
		return TRUE;
	}
