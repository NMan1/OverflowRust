#pragma once
#include "imports.h"
#include "helpers.h"

BOOLEAN gay(copy_memory* m)
{
	tag_wnd*(*ValidateHwnd)(UINT_PTR) = (tag_wnd * (*)(UINT_PTR))(get_system_module_export("\\SystemRoot\\System32\\win32kbase.sys", "ValidateHwnd"));

	if (!ValidateHwnd)
	{
		DbgPrintEx(0, 0, "Can't find ValidateHwnd export, catastrophic error\n");
		return STATUS_SUCCESS;
	}

	const tag_wnd* window_instance = ValidateHwnd((UINT64)m->window_handle);

	if (!window_instance || !window_instance->thread_info)
	{
		DbgPrintEx(0, 0, "ValidateHwnd call failed (get)\n");
		return STATUS_SUCCESS;;
	}

	m->output = (void*)(window_instance->thread_info->owning_thread);

	if (memcpy(shared_section, m, sizeof(copy_memory)) == 0)
		DbgPrintEx(0, 0, "Sending copy_memory back failed\n");

	DbgPrintEx(0, 0, "\nThread context: %p\n", m->output);
}

BOOLEAN gay_two(copy_memory* m)
{
	tag_wnd* (*ValidateHwnd)(UINT_PTR) = (tag_wnd * (*)(UINT_PTR))(get_system_module_export("\\SystemRoot\\System32\\win32kbase.sys", "ValidateHwnd"));

	if (!ValidateHwnd)
	{
		DbgPrintEx(0, 0, "Can't find ValidateHwnd export, catastrophic error\n");
		return STATUS_SUCCESS;
	}

	const tag_wnd* window_instance = ValidateHwnd(m->window_handle);

	if (!window_instance || !window_instance->thread_info)
	{
		DbgPrintEx(0, 0, "ValidateHwnd call failed (set)\n");
		return STATUS_SUCCESS;
	}

	window_instance->thread_info->owning_thread = (PETHREAD)(m->thread_context);
	DbgPrintEx(0, 0, "\nThread ctx set to: %p\n", m->thread_context);
}
