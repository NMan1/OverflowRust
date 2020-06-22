#pragma once
#include "imports.h"


// Shared Memory Vars
const WCHAR g_SharedSectionName[] = L"\\BaseNamedObjects\\L33T";
SECURITY_DESCRIPTOR SecDescriptor;
PVOID	Buffer = NULL;
ULONG	DaclLength;
PACL	Dacl;
HANDLE	g_Section = NULL;
PVOID   shared_section = NULL;

// Shared Memory Vars
const WCHAR g_SharedSectionName_esp[] = L"\\BaseNamedObjects\\L33TGANG";
SECURITY_DESCRIPTOR SecDescriptor2;
PVOID	Buffer2 = NULL;
ULONG	DaclLength2;
PACL	Dacl2;
HANDLE	g_Section2 = NULL;
PVOID   shared_section_esp = NULL;


NTSTATUS create_shared_memory()
{
	DbgPrint("Creating Memory.\n");
	NTSTATUS Status = STATUS_UNSUCCESSFUL;

	Status = RtlCreateSecurityDescriptor(&SecDescriptor, SECURITY_DESCRIPTOR_REVISION);
	if (!NT_SUCCESS(Status))
	{
		DbgPrintEx(0, 0, "RtlCreateSecurityDescriptor failed: %p\n", Status);
		return Status;
	}

	DaclLength = sizeof(ACL) + sizeof(ACCESS_ALLOWED_ACE) * 3 + RtlLengthSid(SeExports->SeLocalSystemSid) + RtlLengthSid(SeExports->SeAliasAdminsSid) +
		RtlLengthSid(SeExports->SeWorldSid);
	Dacl = ExAllocatePoolWithTag(PagedPool, DaclLength, 'lcaD');

	if (Dacl == NULL)
	{
		return STATUS_INSUFFICIENT_RESOURCES;
		DbgPrintEx(0, 0, "ExAllocatePoolWithTag failed: %p\n", Status);
	}

	Status = RtlCreateAcl(Dacl, DaclLength, ACL_REVISION);

	if (!NT_SUCCESS(Status))
	{
		ExFreePool(Dacl);
		DbgPrintEx(0, 0, "RtlCreateAcl failed: %p\n", Status);
		return Status;
	}

	Status = RtlAddAccessAllowedAce(Dacl, ACL_REVISION, FILE_ALL_ACCESS, SeExports->SeWorldSid);

	if (!NT_SUCCESS(Status))
	{
		ExFreePool(Dacl);
		DbgPrintEx(0, 0, "RtlAddAccessAllowedAce SeWorldSid failed: %p\n", Status);
		return Status;
	}

	Status = RtlAddAccessAllowedAce(Dacl,
		ACL_REVISION,
		FILE_ALL_ACCESS,
		SeExports->SeAliasAdminsSid);

	if (!NT_SUCCESS(Status))
	{
		ExFreePool(Dacl);
		DbgPrintEx(0, 0, "RtlAddAccessAllowedAce SeAliasAdminsSid failed  : %p\n", Status);
		return Status;
	}

	Status = RtlAddAccessAllowedAce(Dacl,
		ACL_REVISION,
		FILE_ALL_ACCESS,
		SeExports->SeLocalSystemSid);

	if (!NT_SUCCESS(Status))
	{
		ExFreePool(Dacl);
		DbgPrintEx(0, 0, "RtlAddAccessAllowedAce SeLocalSystemSid failed  : %p\n", Status);
		return Status;
	}

	Status = RtlSetDaclSecurityDescriptor(&SecDescriptor,
		TRUE,
		Dacl,
		FALSE);

	if (!NT_SUCCESS(Status))
	{
		ExFreePool(Dacl);
		DbgPrintEx(0, 0, "RtlSetDaclSecurityDescriptor failed  : %p\n", Status);
		return Status;
	}

	UNICODE_STRING SectionName = { 0 };
	RtlInitUnicodeString(&SectionName, g_SharedSectionName);

	OBJECT_ATTRIBUTES ObjAttributes = { 0 };
	InitializeObjectAttributes(&ObjAttributes, &SectionName, OBJ_CASE_INSENSITIVE, NULL, &SecDescriptor);

	LARGE_INTEGER lMaxSize = { 0 };
	lMaxSize.HighPart = 0;
	lMaxSize.LowPart = 1044 * 10;

	/* Begin Mapping */
	Status = ZwCreateSection(&g_Section, SECTION_ALL_ACCESS, &ObjAttributes, &lMaxSize, PAGE_READWRITE, SEC_COMMIT, NULL);
	if (!NT_SUCCESS(Status))
	{
		DbgPrintEx(0, 0, "Create Section Failed. Status: %p\n", Status);
		return Status;
	}

	//-----------------------------------------------------------------------------	
	//	 ZwMapViewOfSection
	//	-lMaxSize is the ammount of 'Room' the MapViewOfSection will look at
	//	-ViewSize is how much of the 'Room' will be mapped (if 0 then starts at beggining)
	//-----------------------------------------------------------------------------	

	SIZE_T ulViewSize = 0;
	Status = ZwMapViewOfSection(g_Section, NtCurrentProcess(), &shared_section, 0, lMaxSize.LowPart, NULL, &ulViewSize, ViewShare, 0, PAGE_READWRITE | PAGE_NOCACHE);
	if (!NT_SUCCESS(Status))
	{
		DbgPrintEx(0, 0, "Map View Section Failed. Status: %p\n", Status);
		ZwClose(g_Section); //Close Handle
		return Status;
	}

	DbgPrintEx(0, 0, "Shared Memory Created.\n\n");
	ExFreePool(Dacl);

	return Status;
}

NTSTATUS create_shared_memory_esp()
{
	DbgPrint("Creating ESP Memory.\n");
	NTSTATUS Status = STATUS_UNSUCCESSFUL;

	Status = RtlCreateSecurityDescriptor(&SecDescriptor2, SECURITY_DESCRIPTOR_REVISION);
	if (!NT_SUCCESS(Status))
	{
		DbgPrintEx(0, 0, "RtlCreateSecurityDescriptor ESP failed: %p\n", Status);
		return Status;
	}

	DaclLength2 = sizeof(ACL) + sizeof(ACCESS_ALLOWED_ACE) * 3 + RtlLengthSid(SeExports->SeLocalSystemSid) + RtlLengthSid(SeExports->SeAliasAdminsSid) +
		RtlLengthSid(SeExports->SeWorldSid);
	Dacl2 = ExAllocatePoolWithTag(PagedPool, DaclLength2, 'lcaG');

	if (Dacl2 == NULL)
	{
		return STATUS_INSUFFICIENT_RESOURCES;
		DbgPrintEx(0, 0, "ExAllocatePoolWithTag ESP failed: %p\n", Status);
	}

	Status = RtlCreateAcl(Dacl2, DaclLength2, ACL_REVISION);

	if (!NT_SUCCESS(Status))
	{
		ExFreePool(Dacl2);
		DbgPrintEx(0, 0, "RtlCreateAcl ESP failed: %p\n", Status);
		return Status;
	}

	Status = RtlAddAccessAllowedAce(Dacl2, ACL_REVISION, FILE_ALL_ACCESS, SeExports->SeWorldSid);

	if (!NT_SUCCESS(Status))
	{
		ExFreePool(Dacl2);
		DbgPrintEx(0, 0, "RtlAddAccessAllowedAce SeWorldSid ESP failed: %p\n", Status);
		return Status;
	}

	Status = RtlAddAccessAllowedAce(Dacl2,
		ACL_REVISION,
		FILE_ALL_ACCESS,
		SeExports->SeAliasAdminsSid);

	if (!NT_SUCCESS(Status))
	{
		ExFreePool(Dacl2);
		DbgPrintEx(0, 0, "RtlAddAccessAllowedAce SeAliasAdminsSid ESP failed  : %p\n", Status);
		return Status;
	}

	Status = RtlAddAccessAllowedAce(Dacl2,
		ACL_REVISION,
		FILE_ALL_ACCESS,
		SeExports->SeLocalSystemSid);

	if (!NT_SUCCESS(Status))
	{
		ExFreePool(Dacl2);
		DbgPrintEx(0, 0, "RtlAddAccessAllowedAce SeLocalSystemSid ESP failed  : %p\n", Status);
		return Status;
	}

	Status = RtlSetDaclSecurityDescriptor(&SecDescriptor2,
		TRUE,
		Dacl2,
		FALSE);

	if (!NT_SUCCESS(Status))
	{
		ExFreePool(Dacl2);
		DbgPrintEx(0, 0, "RtlSetDaclSecurityDescriptor ESP failed  : %p\n", Status);
		return Status;
	}

	UNICODE_STRING SectionName = { 0 };
	RtlInitUnicodeString(&SectionName, g_SharedSectionName_esp);

	OBJECT_ATTRIBUTES ObjAttributes = { 0 };
	InitializeObjectAttributes(&ObjAttributes, &SectionName, OBJ_CASE_INSENSITIVE, NULL, &SecDescriptor2);

	LARGE_INTEGER lMaxSize = { 0 };
	lMaxSize.HighPart = 0;
	lMaxSize.LowPart = 1044 * 10;

	/* Begin Mapping */
	Status = ZwCreateSection(&g_Section2, SECTION_ALL_ACCESS, &ObjAttributes, &lMaxSize, PAGE_READWRITE, SEC_COMMIT, NULL);
	if (!NT_SUCCESS(Status))
	{
		DbgPrintEx(0, 0, "Create Section Failed. ESP Status: %p\n", Status);
		return Status;
	}

	//-----------------------------------------------------------------------------	
	//	 ZwMapViewOfSection
	//	-lMaxSize is the ammount of 'Room' the MapViewOfSection will look at
	//	-ViewSize is how much of the 'Room' will be mapped (if 0 then starts at beggining)
	//-----------------------------------------------------------------------------	

	SIZE_T ulViewSize = 0;
	Status = ZwMapViewOfSection(g_Section2, NtCurrentProcess(), &shared_section_esp, 0, lMaxSize.LowPart, NULL, &ulViewSize, ViewShare, 0, PAGE_READWRITE | PAGE_NOCACHE);
	if (!NT_SUCCESS(Status))
	{
		DbgPrintEx(0, 0, "Map View Section Failed ESP. Status: %p\n", Status);
		ZwClose(g_Section2); //Close Handle
		return Status;
	}

	DbgPrintEx(0, 0, "Shared Memory Created ESP.\n\n");
	ExFreePool(Dacl2);

	return Status;
}

NTSTATUS read_shared_memory()
{
	NTSTATUS Status = STATUS_UNSUCCESSFUL;

	if (!g_Section)
		return Status;

	if (shared_section)
		ZwUnmapViewOfSection(NtCurrentProcess(), shared_section);

	SIZE_T ulViewSize = 1044 * 10;
	Status = ZwMapViewOfSection(g_Section, NtCurrentProcess(), &shared_section, 0, ulViewSize, NULL, &ulViewSize, ViewShare, 0, PAGE_READWRITE | PAGE_NOCACHE);
	if (!NT_SUCCESS(Status))
	{
		DbgPrintEx(0, 0, "Read shared memory failed. %p\n", Status);
		//ZwClose(g_Section); //Close Handle
		return Status;
	}

	return Status;
}

NTSTATUS read_shared_memory_esp()
{
	NTSTATUS Status = STATUS_UNSUCCESSFUL;

	if (!g_Section2)
		return Status;

	if (shared_section_esp)
		ZwUnmapViewOfSection(NtCurrentProcess(), shared_section_esp);

	SIZE_T ulViewSize = 1044 * 10;
	Status = ZwMapViewOfSection(g_Section2, NtCurrentProcess(), &shared_section_esp, 0, ulViewSize, NULL, &ulViewSize, ViewShare, 0, PAGE_READWRITE | PAGE_NOCACHE);
	if (!NT_SUCCESS(Status))
	{
		DbgPrintEx(0, 0, "Read shared Memory failed esp. %p\n", Status);
		//ZwClose(g_Section); //Close Handle
		return Status;
	}

	return Status;
}

VOID CloseMemory()
{
	// Free Section Memory
	if (shared_section)
		ZwUnmapViewOfSection(NtCurrentProcess(), shared_section);

	// Closing Handle
	if (g_Section)
		ZwClose(g_Section);
}

NTSTATUS GetPid(HANDLE* pid, const char* process)
{
	// ZwQuery
	ULONG CallBackLength = 0;
	PSYSTEM_PROCESS_INFO PSI = NULL;
	PSYSTEM_PROCESS_INFO pCurrent = NULL;
	PVOID BufferPid = NULL;
	NTSTATUS Status = STATUS_UNSUCCESSFUL;

	ANSI_STRING AS;
	UNICODE_STRING process_name;

	RtlInitAnsiString(&AS, process);
	RtlAnsiStringToUnicodeString(&process_name, &AS, TRUE);

	DbgPrintEx(0, 0, "\nSearch for %wZ process id", process_name);

here:; // need to loop because new process spawn making our buffer already to small
	if (!NT_SUCCESS(ZwQuerySystemInformation(SystemProcessInformation, NULL, NULL, &CallBackLength)))
	{
		BufferPid = ExAllocatePoolWithTag(NonPagedPool, CallBackLength, 0x616b7963); // aykc 
		if (!BufferPid)
		{
			DbgPrintEx(0, 0, "Failed To Allocate Buffer Notify Routine");
			return Status;
		}

		PSI = (PSYSTEM_PROCESS_INFO)BufferPid;
		Status = ZwQuerySystemInformation(SystemProcessInformation, PSI, CallBackLength, NULL);
		if (!NT_SUCCESS(Status))
		{
			//DbgPrintEx(0, 0, "Failed To Get Query System Process Information List: %p", Status);
			ExFreePoolWithTag(BufferPid, 0x616b7963);
			goto here;
			return Status = STATUS_INFO_LENGTH_MISMATCH;
		}
		DbgPrintEx(0, 0, "\nSearching For PID...");
		do
		{
			if (PSI->NextEntryOffset == 0)
				break;

			if (RtlEqualUnicodeString(&process_name, &PSI->ImageName, FALSE))
			{
				DbgPrintEx(0, 0, "PID %d | NAME %ws\n", PSI->UniqueProcessId, PSI->ImageName.Buffer);
				*pid = PSI->UniqueProcessId;
				Status = STATUS_SUCCESS;
				break;
			}

			PSI = (PSYSTEM_PROCESS_INFO)((unsigned char*)PSI + PSI->NextEntryOffset); // Calculate the address of the next entry.

		} while (PSI->NextEntryOffset);

		// Free Allocated Memory
		ExFreePoolWithTag(BufferPid, 0x616b7963);
	}

	return Status;
}

PVOID get_all_system_module_base(LPCSTR routine_name) {

	ULONG bytes = 0;
	NTSTATUS status = ZwQuerySystemInformation(SystemModuleInformation, 0, bytes, &bytes);

	if (!bytes)
		return 0;

	PRTL_PROCESS_MODULES modules = (PRTL_PROCESS_MODULES)ExAllocatePoolWithTag(NonPagedPool, bytes, 0x454E4F45); // 'ENON'

	status = ZwQuerySystemInformation(SystemModuleInformation, modules, bytes, &bytes);

	if (!NT_SUCCESS(status))
		return 0;

	PRTL_PROCESS_MODULE_INFORMATION module = modules->Modules;
	PVOID module_base = 0, module_size = 0;

	for (ULONG i = 0; i < modules->NumberOfModules; i++)
	{
		DbgPrintEx(0, 0, "[-] Module: %s\n", module[i].FullPathName);
		DbgPrintEx(0, 0, "	[-] Function: %p\n", RtlFindExportedRoutineByName(module[i].ImageBase, routine_name));
	}

	if (modules)
		ExFreePoolWithTag(modules, 0);

	if (module_base <= 0)
		return 0;

	return module_base;
}

PVOID get_system_module_base(const char* module_name, uintptr_t* size) {

	ULONG bytes = 0;
	NTSTATUS status = ZwQuerySystemInformation(SystemModuleInformation, 0, bytes, &bytes);

	if (!bytes)
		return 0;

	PRTL_PROCESS_MODULES modules = (PRTL_PROCESS_MODULES)ExAllocatePoolWithTag(NonPagedPool, bytes, 0x454E4F45); // 'ENON'

	status = ZwQuerySystemInformation(SystemModuleInformation, modules, bytes, &bytes);

	if (!NT_SUCCESS(status))
		return 0;

	PRTL_PROCESS_MODULE_INFORMATION module = modules->Modules;
	PVOID module_base = 0, module_size = 0;

	for (ULONG i = 0; i < modules->NumberOfModules; i++)
	{
		//DbgPrintEx(0, 0, "[-] Module: %s\n", module[i].FullPathName);
		//DbgPrintEx(0, 0, "	[-] Base: %p\n", module[i].ImageBase);
		if (strcmp((char*)module[i].FullPathName, module_name) == 0)
		{
			module_base = module[i].ImageBase;
			module_size = (PVOID)module[i].ImageSize;
			break;
		}
	}

	if (modules)
		ExFreePoolWithTag(modules, 0);

	if (module_base <= 0)
		return 0;

	*size = module_size;
	return module_base;
}

PVOID get_system_module_export(const char* module_name, LPCSTR routine_name)
{
	uintptr_t size;
	PVOID lpModule = get_system_module_base(module_name, &size);

	if (!lpModule)
		return NULL;

	return RtlFindExportedRoutineByName(lpModule, routine_name);
}

PVOID get_all_system_module_export(LPCSTR routine_name)
{
	get_all_system_module_base(routine_name);

	return NULL;
}

BOOLEAN read_memory(void* address, void* buffer, size_t size) {
	if (!RtlCopyMemory(buffer, address, size)) {
		return FALSE;
	}
	else
	{
		return TRUE;
	}
}

BOOLEAN write_memory(void* address, void* buffer, size_t size) {
	if (!RtlCopyMemory(address, buffer, size)) {
		return FALSE;
	}
	else
	{
		return TRUE;
	}
}

BOOLEAN write_to_read_only_memory(void* address, void* buffer, size_t size) {

	PMDL Mdl = IoAllocateMdl(address, size, FALSE, FALSE, NULL);

	if (!Mdl)
		return FALSE;

	// Locking and mapping memory with RW-rights:
	MmProbeAndLockPages(Mdl, KernelMode, IoReadAccess);
	PVOID Mapping = MmMapLockedPagesSpecifyCache(Mdl, KernelMode, MmNonCached, NULL, FALSE, NormalPagePriority);
	MmProtectMdlSystemAddress(Mdl, PAGE_READWRITE);

	// Write your buffer to mapping:
	write_memory(Mapping, buffer, size);

	// Resources freeing:
	MmUnmapLockedPages(Mapping, Mdl);
	MmUnlockPages(Mdl);
	IoFreeMdl(Mdl);

	return TRUE;
}

static unsigned __int64 PatternScan(void* start, size_t length, const char* pattern, const char* mask)
{
	const char* data = (const char*)start;
	const auto pattern_length = strlen(mask);

	for (size_t i = 0; i <= length - pattern_length; i++)
	{
		BOOLEAN accumulative_found = TRUE;

		for (size_t j = 0; j < pattern_length; j++)
		{
			if (!MmIsAddressValid((void*)((unsigned __int64)data + i + j)))
			{
				accumulative_found = FALSE;
				break;
			}

			if (data[i + j] != pattern[j] && mask[j] != '?')
			{
				accumulative_found = FALSE;
				break;
			}
		}

		if (accumulative_found)
		{
			return (unsigned __int64)((unsigned __int64)data + i);
		}
	}

	return NULL;
}

LARGE_INTEGER Timeout;
#define RELATIVE(wait) (-(wait))

#define NANOSECONDS(nanos) \
(((signed __int64)(nanos)) / 100L)

#define MICROSECONDS(micros) \
(((signed __int64)(micros)) * NANOSECONDS(1000L))

#define MILLISECONDS(milli) \
(((signed __int64)(milli)) * MICROSECONDS(1000L))

#define SECONDS(seconds) \
(((signed __int64)(seconds)) * MILLISECONDS(1000L))

BOOLEAN call_kernel_function(const char* module_name, void* kernel_function_address, LPCSTR funct_name) {
	if (!kernel_function_address)
		return FALSE;

	PVOID* function = (PVOID*)(get_system_module_export(module_name, funct_name));
	DbgPrintEx(0, 0, "\nFunction to hook: %p", function);
	DbgPrintEx(0, 0, "Our function: %p", kernel_function_address);

	if (!function)
		return FALSE;

	uintptr_t our_func = (uintptr_t)(kernel_function_address);
	if (!our_func)
		return FALSE;

	BYTE* instruction = (BYTE*)function; // 14

	//BYTE original[] = {
	//	0x49, 0x89, 0x40, 0x28,
	//	0x49, 0x89, 0x40, 0x20,
	//	0x49, 0x89, 0x40, 0x18,
	//	0x48, 0x83, 0xc4, 0x48,
	//	0x48, 0xB8, // mov rax, 
	//	0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, // [xxx]
	//	0xFF, 0xE0, // jmp rax // jmp back
	//	0x90, 0x90
	//}; // 28 bytes + 2 nopes = 30 bytes	
	
	BYTE original[] = {
		0x48, 0x8b, 0x05, 0x93, 0xdc, 0x05, 0x00, // } 13
		0xff, 0x15, 0x95, 0x65, 0x07, 0x00,       // } bytes
		0x48, 0xB8, // mov rax, 
		0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, // [xxx]
		0xFF, 0xE0, // jmp rax // jmp back
	}; 

	BYTE shell_code[] = {
		0x48, 0xB8, // mov rax, 
		0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, // [xxx]
		0xFF, 0xE0, // jmp rax // d0 for call
	};

	DbgPrintEx(0, 0, "Instruction: %p", *instruction);

	uintptr_t jump_back = (uintptr_t)((uintptr_t)instruction + sizeof(shell_code) + 1);

	DbgPrintEx(0, 0, "Jump back addy: %p", jump_back);

	//memcpy(shell_code + 2, &our_func, sizeof(our_func));d
	//write_to_read_only_memory(instruction, &shell_code, sizeof(shell_code));

	//memcpy(original + 15, &jump_back, sizeof(jump_back));
	//write_to_read_only_memory(our_func + 43, &original, sizeof(original));

	return TRUE;
}

BOOLEAN call_kernel_function_ts(void* kernel_function_address) {
	if (!kernel_function_address)
		return FALSE;

	PVOID* dxgk_routine
		= (PVOID*)(get_system_module_export("\\SystemRoot\\System32\\drivers\\dxgkrnl.sys", "NtDxgkGetTrackedWorkloadStatistics"));

	if (!dxgk_routine) {
		return FALSE;
	}

	/*
	just overwrite the first 15 bytes of any function in dxgkrnl.sys ez
	*/

	BYTE dxgk_original[] = { 0x4C, 0x8B, 0xDC, 0x49, 0x89, 0x5B, 0x18, 0x4D, 0x89, 0x4B, 0x20, 0x49, 0x89, 0x4B, 0x08 };

	BYTE shell_code_start[] =
	{
		0x48, 0xB8 // mov rax, [xxx]
	};

	BYTE shell_code_end[] = 
	{
		0xFF, 0xE0, // jmp rax
		0xCC //
	};

	RtlSecureZeroMemory(&dxgk_original, sizeof(dxgk_original));
	memcpy((PVOID)((ULONG_PTR)dxgk_original), &shell_code_start, sizeof(shell_code_start));
	uintptr_t test_address = (uintptr_t)(kernel_function_address);
	memcpy((PVOID)((ULONG_PTR)dxgk_original + sizeof(shell_code_start)), &test_address, sizeof(void*));
	memcpy((PVOID)((ULONG_PTR)dxgk_original + sizeof(shell_code_start) + sizeof(void*)), &shell_code_end, sizeof(shell_code_end));


	write_to_read_only_memory(dxgk_routine, &dxgk_original, sizeof(dxgk_original));
	DbgPrintEx(0, 0, "Real 222 Called.\n");

	return TRUE;
}


BOOLEAN write_kernel_memory(PEPROCESS process, uintptr_t address, void* buffer, SIZE_T size) {
	if (!address || !buffer || !size)
		return FALSE;

	if (address > 0x7fffffffffff)
		return FALSE;

	NTSTATUS Status = STATUS_SUCCESS;
	KAPC_STATE state;
	KeStackAttachProcess((PKPROCESS)process, &state);

	MEMORY_BASIC_INFORMATION info;
	DbgPrintEx(0, 0, "Write-in: %p\n", address);

	Status = ZwQueryVirtualMemory(ZwCurrentProcess(), (PVOID)address, MemoryBasicInformation, &info, sizeof(info), NULL);
	if (!NT_SUCCESS(Status)) {
		KeUnstackDetachProcess(&state);
		return FALSE;
	}

	if (((uintptr_t)info.BaseAddress + info.RegionSize) < (address + size))
	{
		KeUnstackDetachProcess(&state);
		return FALSE;
	}

	if (!(info.State & MEM_COMMIT) || (info.Protect & (PAGE_GUARD | PAGE_NOACCESS)))
	{
		KeUnstackDetachProcess(&state);
		return FALSE;
	}

	if ((info.Protect & PAGE_EXECUTE_READWRITE) || (info.Protect & PAGE_EXECUTE_WRITECOPY) || (info.Protect & PAGE_READWRITE) || (info.Protect & PAGE_WRITECOPY))
	{
		RtlCopyMemory((void*)address, buffer, size);
	}
	KeUnstackDetachProcess(&state);
	return TRUE;
}

NTSTATUS protect_virtual_memory(PEPROCESS process, PVOID address, ULONG size, ULONG protection, ULONG* protection_out)
{
	if (!process || !address || !size || !protection)
		return STATUS_INVALID_PARAMETER;

	if (address > 0x7fffffffffff)
		return FALSE;

	NTSTATUS status = STATUS_SUCCESS;

	ULONG protection_old = 0;

	KAPC_STATE state;
	KeStackAttachProcess(process, &state);

	status = ZwProtectVirtualMemory(NtCurrentProcess(), &address, &size, protection, &protection_old);

	KeUnstackDetachProcess(&state);

	if (NT_SUCCESS(status))
		*protection_out = protection_old;

	return status;
}

BOOLEAN read_kernel_memory(PEPROCESS process, uintptr_t address, void* buffer, SIZE_T size) {
	if (!address || !buffer || !size)
		return FALSE;

	if (address > 0x7fffffffffff)
		return FALSE;

	SIZE_T bytes = 0;
	NTSTATUS status = STATUS_SUCCESS;

	DbgPrintEx(0, 0, "Read-in: %p\n", address);

	__try {
		status = MmCopyVirtualMemory(process, (void*)address, (PEPROCESS)PsGetCurrentProcess(), buffer, size, KernelMode, &bytes);
		DbgPrintEx(0, 0, "Read-out: %p\n\n", buffer);
		return TRUE;
	}
	__except (EXCEPTION_EXECUTE_HANDLER) {
		DbgPrintEx(0, 0, "Read-failed, bytes copied: %x\n", bytes);
		return FALSE;
	}
}

ULONG64 get_module_base_x64(PEPROCESS proc, UNICODE_STRING module_name) {
	PPEB pPeb = PsGetProcessPeb(proc);

	if (!pPeb) {
		return 0; // failed
	}

	KAPC_STATE state;

	KeStackAttachProcess(proc, &state);

	PPEB_LDR_DATA pLdr = (PPEB_LDR_DATA)pPeb->Ldr;

	if (!pLdr) {
		KeUnstackDetachProcess(&state);
		return 0; // failed
	}

	// loop the linked list
	for (PLIST_ENTRY list = (PLIST_ENTRY)pLdr->ModuleListLoadOrder.Flink;
		list != &pLdr->ModuleListLoadOrder; list = (PLIST_ENTRY)list->Flink) {
		PLDR_DATA_TABLE_ENTRY pEntry =
			CONTAINING_RECORD(list, LDR_DATA_TABLE_ENTRY, InLoadOrderModuleList);
		if (RtlCompareUnicodeString(&pEntry->BaseDllName, &module_name, TRUE) ==
			0) {
			ULONG64 baseAddr = (ULONG64)pEntry->DllBase;
			KeUnstackDetachProcess(&state);
			return baseAddr;
		}
	}
	KeUnstackDetachProcess(&state);

	return 0;
}

DWORD64 virtual_alloc(DWORD64 BaseAddress, ULONG AllocType, ULONG Protection, SIZE_T Size, PEPROCESS proc)
{
	NTSTATUS Status = STATUS_SUCCESS;
	KAPC_STATE apc;
	ULONG old_protection;
	DWORD64 Addy = BaseAddress;

	KeStackAttachProcess(proc, &apc);
	Status = ZwAllocateVirtualMemory(ZwCurrentProcess(), &Addy, 0, &Size, AllocType, Protection);
	KeUnstackDetachProcess(&apc);

	if (!NT_SUCCESS(Status))
	{
		DbgPrintEx(0, 0, "ZwAllocateVirtualMemory Failed:%p\n", Status);
		return Status;
	}

	return Addy;
}
