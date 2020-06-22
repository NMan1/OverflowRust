#include <ntifs.h>
#include "imports.h"
#include "hk.h"
#include "cleaner.h"
#include "helpers.h"
#include "gay.hpp"

HANDLE pid;
PEPROCESS process;
ULONG64 base_addy = NULL;
ULONG64 base_addy_two = NULL;
ULONG64 jmp_back = NULL;

char(*original_event)(PVOID a1);
PVOID(*original_entry)(uintptr_t a1, uintptr_t a2, uintptr_t a3, uintptr_t a4, uintptr_t a5);

PVOID hooked_entry(uintptr_t a1, uintptr_t a2, uintptr_t a3, uintptr_t a4, uintptr_t a5)
{
	static BOOLEAN do_once = TRUE;
	if (do_once)
	{
		DbgPrintEx(0, 0, "Hooked Entry");
		do_once = FALSE;
	}
	
	if (!NT_SUCCESS(read_shared_memory_esp()))
	{
		DbgPrintEx(0, 0, "Faled 1y");
		return NULL;
	}

	if (!shared_section_esp)
		return original_entry(a1, a2, a3, a4, a5);

	copy_memory* m = (copy_memory*)shared_section_esp;
	if (!m)
		return original_entry(a1, a2, a3, a4, a5);

	if (!m->called)
		return original_entry(a1, a2, a3, a4, a5);

	if (m->read != FALSE)
	{
		read_kernel_memory(process, m->address, m->output, m->size);
	}
	else if (m->read_string != FALSE)
	{
		PVOID kernelBuffer = ExAllocatePool(NonPagedPool, m->size);

		if (!kernelBuffer)
			return original_entry(a1, a2, a3, a4, a5);

		if (!memcpy(kernelBuffer, m->buffer_address, m->size))
			return original_entry(a1, a2, a3, a4, a5);

		read_kernel_memory(process, m->address, kernelBuffer, m->size);

		RtlZeroMemory(m->buffer_address, m->size);

		if (!memcpy(m->buffer_address, kernelBuffer, m->size))
		{
			ExFreePool(kernelBuffer);
			return original_entry(a1, a2, a3, a4, a5);
		}
		ExFreePool(kernelBuffer);
	}
	else if (m->end != FALSE)
	{
		if (shared_section_esp)
			ZwUnmapViewOfSection(NtCurrentProcess(), shared_section_esp);
		if (g_Section2)
			ZwClose(g_Section2);
	}

	return original_entry(a1, a2, a3, a4, a5);
}

char hooked_event(PVOID a1)
{
	static BOOLEAN do_once = TRUE;
	if (do_once)
	{
		DbgPrintEx(0, 0, "Hooked Event");
		do_once = FALSE;
	}

	if (!NT_SUCCESS(read_shared_memory()))
		return "";

	if (!shared_section)
		return "";

	copy_memory* m = (copy_memory*)shared_section;
	if (!m)
		return "";

	if (!m->called)
	{
		//if (!NT_SUCCESS(read_shared_memory_esp()))
		//	return "";

		//if (!shared_section_esp)
		//	return "";

		//m = (copy_memory*)shared_section_esp;
		//if (!m)
		//	return "";

		//if (!m->called)
		//{
		//	DbgPrintEx(0, 0, "Returning...");
		return "";
		//}
	}

	if (m->get_pid != FALSE)
		GetPid(&pid, m->process_name);
	else if (m->change_protection != FALSE)
		protect_virtual_memory(process, (PVOID)m->address, m->size, m->protection, m->protection_old);
	else if (m->get_base != FALSE)
	{
		ANSI_STRING AS;
		UNICODE_STRING ModuleNAme;

		RtlInitAnsiString(&AS, m->module_name);
		RtlAnsiStringToUnicodeString(&ModuleNAme, &AS, TRUE);

		PsLookupProcessByProcessId((HANDLE)pid, &process);
		if (!base_addy)
		{
			base_addy = get_module_base_x64(process, ModuleNAme);
			DbgPrintEx(0, 0, "\nBase of %wZ aquired: %p", ModuleNAme, base_addy);
			m->base_address = base_addy;
		}
		else
		{
			base_addy_two = get_module_base_x64(process, ModuleNAme);
			DbgPrintEx(0, 0, "\nBase of %wZ aquired: %p", ModuleNAme, base_addy_two);
			m->base_address = base_addy_two;
		}

		RtlFreeUnicodeString(&ModuleNAme);

		if (memcpy(shared_section, m, sizeof(copy_memory)) == 0)
			DbgPrintEx(0, 0, "Sending copy_memory back failed\n");

		//static DWORD old;
		//if (!old)
		//{
		//	protect_virtual_memory(pid, base_addy + 0x5AE06F0, sizeof(uintptr_t), PAGE_EXECUTE_READWRITE, &old);
		//}
	}
	else if (m->write != FALSE) 
	{
		PVOID kernelBuff = ExAllocatePool(NonPagedPool, m->size);

		if (!kernelBuff)
			return "";

		if (!memcpy(kernelBuff, m->buffer_address, m->size))
			return "";

		write_kernel_memory(process, m->address, kernelBuff, m->size);
		ExFreePool(kernelBuff);
	}
	else if (m->read != FALSE)
	{
		read_kernel_memory(process, m->address, m->output, m->size);
	}
	else if (m->read_string != FALSE) 
	{
		PVOID kernelBuffer = ExAllocatePool(NonPagedPool, m->size);

		if (!kernelBuffer)
			return "";

		if (!memcpy(kernelBuffer, m->buffer_address, m->size))
			return "";

		read_kernel_memory(process, m->address, kernelBuffer, m->size);

		RtlZeroMemory(m->buffer_address, m->size);

		if (!memcpy(m->buffer_address, kernelBuffer, m->size))
			return "";

		DbgPrintEx(0, 0, "String read: %s", (const char*)kernelBuffer);

		ExFreePool(kernelBuffer);
	}
	else if (m->write_string != FALSE) 
	{
		PVOID kernelBuffer1 = ExAllocatePool(NonPagedPool, m->size);

		if (!kernelBuffer1)
			return "";

		if (!memcpy(kernelBuffer1, m->buffer_address, m->size))
			return "";

		write_kernel_memory(process, m->address, kernelBuffer1, m->size);

		ExFreePool(kernelBuffer1);
	}
	else if (m->alloc_memory != FALSE)
	{
		PVOID AllocatedMemory = virtual_alloc(m->address, MEM_COMMIT, m->alloc_type, m->size, process);
		m->output = AllocatedMemory;
		if (memcpy(shared_section, m, sizeof(copy_memory)) == 0)
			DbgPrintEx(0, 0, "Sending copy_memory back failed\n");

		DbgPrintEx(0, 0, "\nAllocated at: %p\n", AllocatedMemory);
	}	
	else if (m->get_thread_context != FALSE)
		gay(m);
	else if (m->set_thread_context != FALSE)
		gay_two(m);
	else if (m->end != FALSE)
	{
		if (shared_section)
			ZwUnmapViewOfSection(NtCurrentProcess(), shared_section);
		if (g_Section)
			ZwClose(g_Section);

		if (shared_section_esp)
			ZwUnmapViewOfSection(NtCurrentProcess(), shared_section_esp);
		if (g_Section2)
			ZwClose(g_Section2);
	}

	return "";
}

// This is needed because when the driver returns all handles are closed
// therfore our seciton handle will be closed.
// creating it in a system thread prevents that
void create_memeory_thread()
{
	if (!NT_SUCCESS(create_shared_memory()))
	{
		DbgPrintEx(0, 0, "create_shared_memory failed:\n");
		return;
	}	
	
	if (!NT_SUCCESS(create_shared_memory_esp()))
	{
		DbgPrintEx(0, 0, "create_shared_memory_esp failed:\n");
		return;
	}
	PsTerminateSystemThread(STATUS_SUCCESS);
}

void real_entry()
{
	OBJECT_ATTRIBUTES obj_att = { 0 };
	HANDLE thread = NULL;
	DbgPrintEx(0, 0, "Real Entry Called.");

	clean_piddbcachetalbe();

	InitializeObjectAttributes(&obj_att, NULL, OBJ_KERNEL_HANDLE, NULL, NULL);
	NTSTATUS status = PsCreateSystemThread(&thread, THREAD_ALL_ACCESS, &obj_att, NULL, NULL, create_memeory_thread, NULL);
	if (!NT_SUCCESS(status))
	{
		DbgPrintEx(0, 0, "PsCreateSystemThread Failed:\n", status);
		return status;
	}

	HkDetourFunction(get_system_module_export("\\SystemRoot\\System32\\drivers\\watchdog.sys", "WdLogEvent5_WdError"), (PVOID)hooked_event, 16, (PVOID*)&original_event);
	//HkDetourFunction(get_system_module_export("\\SystemRoot\\System32\\drivers\\watchdog.sys", "WdLogNewEntry5_WdError"), (PVOID)hooked_entry, 17, (PVOID*)&original_entry);

	DbgPrintEx(0, 0, "Hooks applied!");
	ZwClose(thread);
}

NTSTATUS DriverEntry(_In_ PDRIVER_OBJECT DriverObj, _In_ PUNICODE_STRING RegistryPath)
{
	DbgPrintEx(0, 0, "Driver Created.\n");

	// Fix Paramms
	UNREFERENCED_PARAMETER(RegistryPath);
	UNREFERENCED_PARAMETER(DriverObj);

	real_entry();
	
	return STATUS_SUCCESS;
}