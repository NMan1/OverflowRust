#pragma once
#include "../driver/driver.h"
#include "../helpers/settings.h"
#include "../renderer/renderer.h"
#include "game.h"
#include <dwmapi.h>
#include <locale>
#include <codecvt>
#include "cheat.h"

namespace thread = std::this_thread;
using ms = std::chrono::milliseconds;

#define M_PI 3.14159265358979323846
#define M_RADPI		57.295779513082f
#define M_PI_F		((float)(M_PI))	// Shouldn't collide with anything.
#define RAD2DEG( x  )  ( (float)(x) * (float)(180.f / M_PI_F) )
#define DEG2RAD( x  )  ( (float)(x) * (float)(M_PI_F / 180.f) )

namespace game
{
	static HWND find_window()
	{
		auto exterior_window_handle = driver::read<HWND>(game::unity_player + 0x178DB48);
		if (!exterior_window_handle)
		{
			printf("[!] can't get exterior window's handle\n");
			std::cin.get();
			return 0;
		}
		std::cout << "[+] found hijackwindow: " << std::hex << exterior_window_handle << std::endl;
		return exterior_window_handle;
	}

	static void run_esp()
	{
		d2d_window_t window{ };
		_renderer renderer{ window._handle, find_window() };
		window.~d2d_window_t();

		while (!GetAsyncKeyState(VK_END) && !settings::end)
		{
			renderer.begin_scene();

			if (settings::esp_toggle)
			{
				game::draw_mutex.lock();
				for (auto& object : game::draw_list)
				{
					clr color;
					int distance;
					switch (object.second)
					{
					case BasePlayer:
						if (!settings::esp_baseplayer.toggle)
							continue;
						color = settings::esp_baseplayer.color;
						distance = settings::esp_baseplayer.distance;
						break;
					case Scientist:
						if (!settings::esp_scientist.toggle)
							continue;
						color = settings::esp_scientist.color;
						distance = settings::esp_scientist.distance;
						break;
					case StashContainer:
						if (!settings::esp_stash.toggle)
							continue;
						color = settings::esp_stash.color;
						distance = settings::esp_stash.distance;
						break;
					}

					auto pos = driver::read<vec3_t>(object.first + 0x90);
					if (pos == vec3_t{ -1, -1, -1 }) // if it returns invalid positon, means the object was null, remove from list 
					{
						game::draw_list.erase(std::remove(game::draw_list.begin(), game::draw_list.end(), object), game::draw_list.end());
						continue;
					}

					auto local_pos = driver::read<vec3_t>(game::local_pos_component + 0x90);
					auto to_distance = local_pos.Distance(pos);

					if (to_distance > distance)
						continue;

					vec_t screen2d;
					if (!game::world_to_screen(pos, screen2d))
						continue;

					renderer.draw_line(1920 / 2, 1080, screen2d.x, screen2d.y, color);
				}
				game::draw_mutex.unlock();
			}

			renderer.draw_text(std::wstring_convert<std::codecvt_utf8<wchar_t>>().from_bytes("overflow"), 1800, 5, 22, false, clr(255, 0, 0));
			renderer.end_scene();
			thread::sleep_for(ms(1));
		}
		renderer.~_renderer();
	}
}