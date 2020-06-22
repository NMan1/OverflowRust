#include <thread>
#include "cheat.h"
#include "../helpers/settings.h"
#include "../menu/menu.h"
#include "game.h"
#include "esp.h"


void features();
bool cheat::can_draw = false;

void cheat::cheat_thread()
{
	if (!game::get_networkable())
		return;

	if (!game::get_buffer_list())
		return;

	if (!game::get_object_list())
		return;

	//std::thread feature_thread(features);
	//std::thread esp_render(game::run_esp);
	//Sleep(1500);

	while (true)
	{
		for (auto i = 0; i < game::get_object_list_size(); i++)
		{
			const auto element = driver::read<uintptr_t>(game::object_list + (0x20 + (i * 0x8)));
			const auto element_name = game::get_class_name(element);

			const auto base_mono_object = driver::read<uintptr_t>(element + 0x10);
			if (!base_mono_object)
				continue;

			auto object = driver::read<uintptr_t>(base_mono_object + 0x30);
			if (!object)
				continue;

			object = driver::read<uintptr_t>(object + 0x30);
			if (!object)
				continue;

			if (element_name.find("BasePlayer") != std::string::npos)
			{
				const auto base_player = game::get_base_player(object);
				if (!base_player)
					continue;

				if (!i && !game::local_player) // assign local player
				{
					game::local_player = object;
					game::local_pos_component = game::get_object_pos_component(object);
					std::cout << "[-] Local player: " << std::hex << game::local_player << std::endl;
				}
				else    // else push back entity
				{
					// lock access to entity 
					//std::lock_guard guard(game::entity_mutex);
					//std::find(game::entites.begin(), game::entites.end(), base_player) == game::entites.end() ? game::entites.push_back(std::make_pair(base_player, BasePlayer)) : void();

					// if not in draw_list then push back
					if (std::find(game::draw_list.begin(), game::draw_list.end(), std::make_pair(object, BasePlayer)) == game::draw_list.end())
					{
						std::lock_guard guard(game::draw_mutex);
						game::draw_list.push_back(std::make_pair(game::get_object_pos_component(object), BasePlayer));
					}
				}
			}
			else if (element_name.find("Scientist") != std::string::npos)
			{
				// if not in draw_list then push back
				if (std::find(game::draw_list.begin(), game::draw_list.end(), std::make_pair(object, Scientist)) == game::draw_list.end())
				{
					std::lock_guard guard(game::draw_mutex);
					game::draw_list.push_back(std::make_pair(game::get_object_pos_component(object), Scientist));
				}
			}			
			else if (element_name.find("StashContai") != std::string::npos)
			{
				// if not in draw_list then push back
				if (std::find(game::draw_list.begin(), game::draw_list.end(), std::make_pair(object, StashContainer)) == game::draw_list.end())
				{
					std::lock_guard guard(game::draw_mutex);
					game::draw_list.push_back(std::make_pair(game::get_object_pos_component(object), StashContainer));
				}
			}
		}
		/*std::this_thread::sleep_for(std::chrono::seconds(20));*/

		if (game::local_player)
			game::run_esp();
	}

	return;
}

void features()
{
	while (!settings::end)
	{
		try
		{
			//std::lock_guard guard(game::local_mutex);
			auto _local_player = game::local_player;

			if (_local_player)
			{
				if (settings::spider_man)
				{
					auto base_movement = driver::read<uintptr_t>(_local_player + 0x5E8);
					if (base_movement)
					{
						driver::write(base_movement + 0xAC, 0.f);
						driver::write(base_movement + 0xB0, 0.f);
					}
				}

				if (settings::auto_pistol_changed || settings::recoil_changed)
				{
					auto active_weapon = game::get_active_weapon(_local_player);
					if (active_weapon)
					{
						if (settings::auto_pistol && settings::auto_pistol_changed)
						{
							game::set_automatic(active_weapon);
							settings::recoil_changed = false;
						}

						if (settings::allow_recoil && settings::recoil_changed)
						{
							game::set_recoil_props(active_weapon);
							settings::recoil_changed = false;
						}
					}
				}
			}
		}
		catch (...)
		{

		}
		std::this_thread::sleep_for(std::chrono::milliseconds(1));
	}
}