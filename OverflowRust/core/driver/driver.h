#pragma once
#include "imports.h"
#include <string_view>
#include <mutex>

static std::mutex mtx;

#include <chrono>

class timer
{
private:
	std::chrono::time_point<std::chrono::steady_clock> m_StartTime;

public:
	void Start()
	{
		m_StartTime = std::chrono::high_resolution_clock::now();
	}
	float GetDuration()
	{
		auto stop = std::chrono::high_resolution_clock::now();
		auto duration = std::chrono::duration_cast<std::chrono::microseconds>(stop - m_StartTime);
		return duration.count();
	}
};

typedef struct _copy_memory
{
	BOOLEAN called;
	BOOLEAN read;
	BOOLEAN read_string;
	void* buffer_address;
	UINT_PTR  address;
	ULONGLONG size;
	void* output;

	BOOLEAN   write;
	BOOLEAN write_string;

	BOOLEAN  get_base;
	ULONG64 base_address;
	const char* module_name;

	BOOLEAN get_pid;
	const char* process_name;
	ULONG pid_of_source;

	BOOLEAN alloc_memory;
	ULONG	alloc_type;

	BOOLEAN		change_protection;
	ULONG		protection;
	ULONG		protection_old;

	BOOLEAN get_thread_context;
	BOOLEAN set_thread_context;

	BOOLEAN end;

	HWND window_handle;
	UINT_PTR thread_context;
}copy_memory;

static void call_hook()
{
	static void* control_function = GetProcAddress(LoadLibrary("win32u.dll"), "NtDxgkCreateTrackedWorkload");
	static const auto control = static_cast<uint64_t(__stdcall*)()>(control_function);
	control();
}

extern HANDLE memory_read, memory_write, memory_esp_write;

namespace driver
{
	static inline void close_handles()
	{
		CloseHandle(memory_read);
		CloseHandle(memory_write);
		CloseHandle(memory_esp_write);
		return;
	}

	static std::string GetLastErrorAsString()
	{
		//Get the error message, if any.
		DWORD errorMessageID = ::GetLastError();
		if (errorMessageID == 0)
			return std::string(); //No error message has been recorded

		LPSTR messageBuffer = nullptr;
		size_t size = FormatMessageA(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
			NULL, errorMessageID, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), (LPSTR)&messageBuffer, 0, NULL);

		std::string message(messageBuffer, size);

		//Free the buffer.
		LocalFree(messageBuffer);

		return message;
	}

	static bool open_memory_handles()
	{
		// Get Handle Read To Map
		memory_read = OpenFileMapping(FILE_MAP_READ, FALSE, "Global\\L33T");
		if (memory_read == INVALID_HANDLE_VALUE || !memory_read)
			return false;

		memory_write = OpenFileMapping(FILE_MAP_WRITE, FALSE, "Global\\L33T");
		if (memory_write == INVALID_HANDLE_VALUE || !memory_write)
			return false;

		memory_esp_write = OpenFileMapping(FILE_MAP_WRITE, FALSE, "Global\\L33TGANG");
		if (memory_esp_write == INVALID_HANDLE_VALUE || !memory_esp_write)
			return false;

		return true;
	}

	static void clear_map(copy_memory* map)
	{
		copy_memory m{ 0 };
		RtlCopyMemory(map, &m, sizeof(m));
	}

	static bool end(bool esp_driver = false)
	{
		copy_memory m = { 0 };
		m.called = TRUE;
		m.end = TRUE;
		m.get_pid = FALSE;
		m.get_base = FALSE;
		m.read = FALSE;
		m.read_string = FALSE;
		m.write = FALSE;
		m.write_string = FALSE;
		m.change_protection = FALSE;
		m.alloc_memory = FALSE;
		m.get_thread_context = FALSE;
		m.set_thread_context = FALSE;

		auto map_view = (copy_memory*)MapViewOfFile(esp_driver ? memory_esp_write : memory_write, FILE_MAP_WRITE, 0, 0, 4096);
		if (!map_view)
		{
			std::cout << "[!] map_view failed" << std::endl;
			return false;
		}

		RtlCopyMemory(map_view, &m, sizeof(m));

		call_hook();
		clear_map(map_view);
		UnmapViewOfFile(map_view);
		return true;
	}

	static bool get_process_id(const char* process_name)
	{
		copy_memory m = { 0 };
		m.called = TRUE;
		m.get_pid = TRUE;
		m.process_name = process_name;
		m.get_base = FALSE;
		m.read = FALSE;
		m.read_string = FALSE;
		m.write = FALSE;
		m.write_string = FALSE;
		m.change_protection = FALSE;
		m.alloc_memory = FALSE;
		m.get_thread_context = FALSE;
		m.set_thread_context = FALSE;

		auto map_view = (copy_memory*)MapViewOfFile(memory_write, FILE_MAP_WRITE, 0, 0, 4096);
		if (!map_view)
		{
			std::cout << "[!] map_view failed" << std::endl;
			return false;
		}

		RtlCopyMemory(map_view, &m, sizeof(m));

		call_hook();
		clear_map(map_view);
		UnmapViewOfFile(map_view);
		return true;
	}

	static void change_protection(uint64_t address, uint32_t page_protection, std::size_t size)
	{
		if (!address)
			return;

		mtx.lock();
		copy_memory m = { 0 };
		m.called = TRUE;
		m.address = address;
		m.protection = page_protection;
		m.size = size;
		m.change_protection = TRUE;
		m.get_pid = FALSE;
		m.get_base = FALSE;
		m.read = FALSE;
		m.read_string = FALSE;
		m.write = FALSE;
		m.write_string = FALSE;
		m.alloc_memory = FALSE;
		m.protection_old = 0;
		m.get_thread_context = FALSE;
		m.set_thread_context = FALSE;

		auto map_view = (copy_memory*)MapViewOfFile(memory_write, FILE_MAP_WRITE, 0, 0, 4096);
		if (!map_view)
		{
			std::cout << "[!] map_view failed" << std::endl;
			return;
		}

		RtlCopyMemory(map_view, &m, sizeof(m));
		call_hook();

		clear_map(map_view);
		UnmapViewOfFile(map_view);
		mtx.unlock();
	}

	static ULONG64 get_module_base_address(const char* module_name)
	{
		copy_memory m = { 0 };
		m.called = TRUE;

		m.get_base = TRUE;
		m.read = FALSE;
		m.get_pid = FALSE;
		m.read_string = FALSE;
		m.write = FALSE;
		m.write_string = FALSE;
		m.module_name = module_name;
		m.change_protection = FALSE;
		m.alloc_memory = FALSE;
		m.get_thread_context = FALSE;
		m.set_thread_context = FALSE;

		auto map_view = (copy_memory*)MapViewOfFile(memory_write, FILE_MAP_WRITE, 0, 0, 4096);
		if (!map_view)
		{
			std::cout << "[!] map_view failed" << std::endl;
			return NULL;
		}

		RtlCopyMemory(map_view, &m, sizeof(m));
		call_hook();

		auto received = (copy_memory*)MapViewOfFile(memory_read, FILE_MAP_READ, 0, 0, sizeof(copy_memory));
		if (!received)
		{
			std::cout << "[!] failed to read received" << std::endl;
			return NULL;
		}

		auto temp = received->base_address;
		UnmapViewOfFile(received);
		clear_map(map_view);
		UnmapViewOfFile(map_view);
		return temp;
	}

	template <class T>
	T read(UINT_PTR ReadAddress, bool esp_driver = false)
	{
		if (!ReadAddress)
			return T{};

		mtx.lock();
		T response{};

		copy_memory m;
		m.called = TRUE;
		m.size = sizeof(T);
		m.address = ReadAddress;
		m.read = TRUE;
		m.get_pid = FALSE;
		m.read_string = FALSE;
		m.write_string = FALSE;
		m.write = FALSE;
		m.get_base = FALSE;
		m.change_protection = FALSE;
		m.alloc_memory = FALSE;
		m.output = &response;
		m.get_thread_context = FALSE;
		m.set_thread_context = FALSE;

		auto map_view = (copy_memory*)MapViewOfFile(esp_driver ? memory_esp_write : memory_write, FILE_MAP_WRITE, 0, 0, 4096);

		if (!map_view)
		{
			std::cout << "[!] map_view failed: " << GetLastErrorAsString() << std::endl;
			return T{};
		}

		RtlCopyMemory(map_view, &m, sizeof(m));

		call_hook();
		clear_map(map_view);
		//UnmapViewOfFile(map_view);

		auto temp = response;
		mtx.unlock();
		return temp;
	}	
	
	static void read(UINT_PTR ReadAddress, void* buffer, uintptr_t size, bool esp_driver = false)
	{
		if (!ReadAddress)
			return;

		mtx.lock();

		copy_memory m;
		m.called = TRUE;
		m.size = size;
		m.address = ReadAddress;
		m.read = TRUE;
		m.get_pid = FALSE;
		m.read_string = FALSE;
		m.write_string = FALSE;
		m.write = FALSE;
		m.get_base = FALSE;
		m.change_protection = FALSE;
		m.alloc_memory = FALSE;
		m.output = buffer;
		m.get_thread_context = FALSE;
		m.set_thread_context = FALSE;

		auto map_view = (copy_memory*)MapViewOfFile(esp_driver ? memory_esp_write : memory_write, FILE_MAP_WRITE, 0, 0, 4096);

		if (!map_view)
		{
			std::cout << "[!] map_view failed: " << GetLastErrorAsString() << std::endl;
			return;
		}

		RtlCopyMemory(map_view, &m, sizeof(m));

		call_hook();
		clear_map(map_view);
		//UnmapViewOfFile(map_view);
		mtx.unlock();
	}

	static bool WriteVirtualMemoryRaw(UINT_PTR WriteAddress, UINT_PTR SourceAddress, SIZE_T WriteSize);

	template<typename S>
	bool write(UINT_PTR WriteAddress, const S& value)
	{
		if (!WriteAddress)
			return false;

		return WriteVirtualMemoryRaw(WriteAddress, (UINT_PTR)&value, sizeof(S));
	}
	bool WriteVirtualMemoryRaw(UINT_PTR WriteAddress, UINT_PTR SourceAddress, SIZE_T WriteSize)
	{
		mtx.lock();
		copy_memory m;
		m.called = TRUE;
		m.address = WriteAddress;
		m.pid_of_source = GetCurrentProcessId();
		m.write = TRUE;
		m.get_pid = FALSE;
		m.read = FALSE;
		m.read_string = FALSE;
		m.get_base = FALSE;
		m.write_string = FALSE;
		m.change_protection = FALSE;
		m.buffer_address = (void*)SourceAddress;
		m.size = WriteSize;
		m.alloc_memory = FALSE;
		m.get_thread_context = FALSE;
		m.set_thread_context = FALSE;

		auto map_view = (copy_memory*)MapViewOfFile(memory_write, FILE_MAP_WRITE, 0, 0, 4096);
		if (!map_view)
		{
			std::cout << "[!] map_view failed" << std::endl;
			return false;
		}

		RtlCopyMemory(map_view, &m, sizeof(m));

		call_hook();
		clear_map(map_view);
		//UnmapViewOfFile(map_view);

		mtx.unlock();
		return true;
	}

	static std::string read_string(UINT_PTR String_address, SIZE_T size, bool esp_driver=false)
	{
		std::unique_ptr<char[]> buffer(new char[size]);
		read(String_address, buffer.get(), size);
		return std::string(buffer.get());
	}	
	
	static std::wstring read_wstring(UINT_PTR String_address, SIZE_T size, bool esp_driver=false)
	{
		const auto buffer = std::make_unique<wchar_t[]>(size);
		read(String_address, buffer.get(), size * 2);
		return std::wstring(buffer.get());
	}

	static bool write_string(UINT_PTR String_address, void* buffer, SIZE_T size)
	{
		if (!String_address)
			return false;

		mtx.lock();
		copy_memory m;
		m.called = TRUE;
		m.write_string = TRUE;
		m.read = FALSE;
		m.get_pid = FALSE;
		m.read_string = FALSE;
		m.get_base = FALSE;
		m.write = FALSE;
		m.address = String_address;
		m.buffer_address = buffer;
		m.size = size;
		m.change_protection = FALSE;
		m.alloc_memory = FALSE;
		m.get_thread_context = FALSE;
		m.set_thread_context = FALSE;

		auto map_view = (copy_memory*)MapViewOfFile(memory_write, FILE_MAP_WRITE, 0, 0, 4096);
		if (!map_view)
		{
			std::cout << "[!] map_view failed" << std::endl;
			return false;
		}

		RtlCopyMemory(map_view, &m, sizeof(m));

		call_hook();
		clear_map(map_view);
		UnmapViewOfFile(map_view);

		mtx.unlock();
		return true;
	}

	static UINT_PTR virtual_alloc(UINT_PTR ReadAddress, ULONG alloc_type, SIZE_T size)
	{
		if (!ReadAddress)
			return false;

		mtx.lock();
		copy_memory m;
		m.called = TRUE;
		m.address = ReadAddress;
		m.read = FALSE;
		m.get_pid = FALSE;
		m.read_string = FALSE;
		m.write_string = FALSE;
		m.write = FALSE;
		m.get_base = FALSE;
		m.change_protection = FALSE;
		m.alloc_memory = TRUE;
		m.alloc_type = alloc_type;
		m.size = size;
		m.get_thread_context = FALSE;
		m.set_thread_context = FALSE;

		auto map_view = (copy_memory*)MapViewOfFile(memory_write, FILE_MAP_WRITE, 0, 0, 4096);
		if (!map_view)
		{
			std::cout << "[!] map_view failed" << std::endl;
			return false;
		}

		RtlCopyMemory(map_view, &m, sizeof(m));

		call_hook();

		auto received = (copy_memory*)MapViewOfFile(memory_read, FILE_MAP_READ, 0, 0, sizeof(copy_memory));
		if (!received)
		{
			std::cout << "[!] failed to read received" << std::endl;
			return NULL;
		}

		auto temp = received->output;
		UnmapViewOfFile(received);
		clear_map(map_view);
		UnmapViewOfFile(map_view);

		mtx.unlock();
		return *(UINT_PTR*)&temp;
	}

	static bool get_thread(HWND window_handle, uint64_t* thread_context)
	{
		mtx.lock();
		copy_memory m;
		m.called = TRUE;
		m.read = FALSE;
		m.get_pid = FALSE;
		m.read_string = FALSE;
		m.write_string = FALSE;
		m.write = FALSE;
		m.get_base = FALSE;
		m.change_protection = FALSE;
		m.alloc_memory = FALSE;
		m.get_thread_context = TRUE;
		m.set_thread_context = FALSE;
		m.window_handle = window_handle;

		auto map_view = (copy_memory*)MapViewOfFile(memory_write, FILE_MAP_WRITE, 0, 0, 4096);
		if (!map_view)
		{
			std::cout << "[!] map_view failed" << std::endl;
			return false;
		}

		RtlCopyMemory(map_view, &m, sizeof(m));

		call_hook();

		auto received = (copy_memory*)MapViewOfFile(memory_read, FILE_MAP_READ, 0, 0, sizeof(copy_memory));
		if (!received)
		{
			std::cout << "[!] failed to read received" << std::endl;
			return false;
		}

		*thread_context = *(UINT_PTR*)&received->output;
		UnmapViewOfFile(received);
		clear_map(map_view);
		UnmapViewOfFile(map_view);

		mtx.unlock();
		return true;
	}

	static bool set_thread(HWND window_handle, uint64_t thread_context)
	{
		mtx.lock();
		copy_memory m;
		m.called = TRUE;
		m.read = FALSE;
		m.get_pid = FALSE;
		m.read_string = FALSE;
		m.write_string = FALSE;
		m.write = FALSE;
		m.get_base = FALSE;
		m.change_protection = FALSE;
		m.alloc_memory = FALSE;
		m.get_thread_context = FALSE;
		m.set_thread_context = TRUE;
		m.window_handle = window_handle;
		m.thread_context = thread_context;

		auto map_view = (copy_memory*)MapViewOfFile(memory_write, FILE_MAP_WRITE, 0, 0, 4096);
		if (!map_view)
		{
			std::cout << "[!] map_view failed" << std::endl;
			return false;
		}

		RtlCopyMemory(map_view, &m, sizeof(m));

		call_hook();
		clear_map(map_view);
		UnmapViewOfFile(map_view);

		mtx.unlock();
		return true;
	}
}