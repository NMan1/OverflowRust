#include <Windows.h>
#include <iostream>
#include "driver/driver.h"
#include "menu/menu.h"
#include "helpers/settings.h"
#include "game/game.h"
#include "game/cheat.h"
#include "game/esp.h"
#include "renderer/renderer.h"

HANDLE memory_read = NULL, memory_write = NULL, memory_esp_write = NULL;

int main()
{
//#ifdef NDEBUG       
//	ShowWindow(GetConsoleWindow(), SW_HIDE);
//#endif
	if (driver::open_memory_handles())
		std::cout << "[-] Shared memory handles opened successfully" << std::endl;
	else
	{
		std::cout << "[!] Shared memory handles failed to open" << std::endl;
		std::cin.get();
		return 0;
	}

	driver::get_process_id("RustClient.exe"); // RustClient.exe
	
	game::game_assembly = (uintptr_t)driver::get_module_base_address("GameAssembly.dll");
	game::unity_player = (uintptr_t)driver::get_module_base_address("UnityPlayer.dll");
	if (!game::game_assembly || !game::unity_player)
	{
		std::cout << "[!] failed to aquire base address..." << std::endl;
		std::cin.get();
		return 0;
	}

	std::cout << "[+] aquired base address and PID" << std::endl;
	std::cout << "	[+] game assembly: " << std::hex << game::game_assembly << std::endl;
	std::cout << "	[+] unity player: " << std::hex << game::unity_player << std::endl;

	std::thread run_cheat(cheat::cheat_thread);
	//std::thread esp_render(game::run_esp);

	MSG msg;
	ZeroMemory(&msg, sizeof(msg));
	menu::setup_menu();
	while (true)
	{
		if (::PeekMessage(&msg, NULL, 0U, 0U, PM_REMOVE))
		{
			::TranslateMessage(&msg);
			::DispatchMessage(&msg);
			continue;
		}

		menu::begin_draw();
		menu::render_menu();
		menu::end_draw();

		if (settings::end)
			break;
	}

	menu::menu_shutdown();

	//driver::end();

	std::cin.get();
	return 1;
}