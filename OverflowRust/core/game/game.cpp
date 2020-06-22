#include "game.h"

namespace game
{
	uintptr_t game_assembly, unity_player, camera_instance;
	uintptr_t base_networkable, local_player, local_pos_component;

	uintptr_t buffer_list = NULL, object_list = NULL;
	std::mutex entity_mutex, draw_mutex, local_mutex;
	std::vector<std::pair<uintptr_t, entity_type>> draw_list;

	bool get_networkable()
	{
		base_networkable = driver::read<uintptr_t>(game_assembly + networkable);
		return base_networkable ? true : false;
	}

	bool get_buffer_list()
	{
		auto unk1 = driver::read<uintptr_t>(base_networkable + 0xB8);
		if (!unk1)
			return false;

		auto client_entities = driver::read<uintptr_t>(unk1);
		auto entity_realm = driver::read<uintptr_t>(client_entities + 0x10);
		buffer_list = driver::read<uintptr_t>(entity_realm + 0x28);
		return buffer_list ? true : false;
	}

	bool get_object_list()
	{
		object_list = driver::read<uintptr_t>(buffer_list + 0x18);
		return object_list ? true : false;
	}

	int get_object_list_size()
	{
		return driver::read<int>(buffer_list + 0x10);
	}

	uintptr_t get_base_player(uintptr_t object)
	{
		auto object_unk = driver::read<uintptr_t>(object + 0x18);
		if (!object_unk)
			return {};

		return driver::read<uintptr_t>(object_unk + 0x28);
	}

	std::string get_class_name(uintptr_t object)
	{
		auto object_unk = driver::read<uintptr_t>(object);
		if (!object_unk)
			return {};

		return driver::read_string(driver::read<uintptr_t>(object_unk + 0x10), 13);
	}

	uintptr_t get_object_pos_component(uintptr_t entity, bool esp_driver)
	{
		auto player_visual = driver::read<uintptr_t>(entity + 0x8);
		if (!player_visual)
			return NULL;

		return driver::read<uintptr_t>(player_visual + 0x38);
	}	
	
	vec3_t get_object_pos(uintptr_t entity, bool esp_driver)
	{
		auto player_visual = driver::read<uintptr_t>(entity + 0x8);
		if (!player_visual)
			return { -1, -1, -1 };

		auto visual_state = driver::read<uintptr_t>(player_visual + 0x38);
		if (!visual_state)
			return { -1, -1, -1 };

		return driver::read<vec3_t>(visual_state + 0x90, esp_driver);
	}

	matrix4x4 get_view_matrix(bool esp_driver)
	{
		if (!camera_instance)
		{
			auto gom_ = driver::read<uintptr_t>(unity_player + gom, esp_driver);
			if (!gom_)
				return {};

			auto tagged_objects = driver::read<uintptr_t>(gom_ + 0x8, esp_driver);
			if (!tagged_objects)
				return {};

			auto game_object = driver::read<uintptr_t>(tagged_objects + 0x10, esp_driver);
			if (!tagged_objects)
				return {};

			auto object_class = driver::read<uintptr_t>(game_object + 0x30, esp_driver);
			if (!tagged_objects)
				return {};

			camera_instance = driver::read<uintptr_t>(object_class + 0x18, esp_driver);
		}

		if (camera_instance)
			return driver::read<matrix4x4>(camera_instance + 0xDC, esp_driver);
		else
			return {};
	}

	bool world_to_screen(const vec3_t& entity_pos, vec_t& screen_pos, bool esp_driver)
	{
		auto view_matrix = get_view_matrix(esp_driver);
		vec3_t trans_vec{ view_matrix._14, view_matrix._24, view_matrix._34 };
		vec3_t right_vec{ view_matrix._11, view_matrix._21, view_matrix._31 };
		vec3_t up_vec{ view_matrix._12, view_matrix._22, view_matrix._32 };

		float w = trans_vec.Dot(entity_pos) + view_matrix._44;
		if (w < 0.098f)
			return false;
		float y = up_vec.Dot(entity_pos) + view_matrix._42;
		float x = right_vec.Dot(entity_pos) + view_matrix._41;
		screen_pos = vec_t((1920 / 2) * (1.f + x / w), (1080 / 2) * (1.f - y / w));
		return true;
	}

	bool set_admin()
	{
		mtx.lock();
		auto _local_player = local_player;
		mtx.unlock();

		if (!_local_player)
			return false;

		auto player_flags = driver::read<uintptr_t>(_local_player + 0x5B8);
		if (!player_flags)
			return {};

		player_flags |= 4;

		driver::write(_local_player + 0x5B8, player_flags);
		return true;
	}

	uintptr_t get_active_item(uintptr_t _local_player)
	{
		return  driver::read<uintptr_t>(_local_player + 0x530);
	}

	int get_item_id(uintptr_t item)
	{
		return  driver::read<uintptr_t>(item + 0x28);
	}

	std::wstring get_item_name(uintptr_t item)
	{
		auto unk = driver::read<uintptr_t>(item + 0x20);
		unk = driver::read<uintptr_t>(unk + 0x20);
		return driver::read_wstring(unk + 0x14, 14);
	}

	bool get_recoil_properties(uintptr_t weapon, std::string name)
	{
		auto base_projectile = driver::read<uintptr_t>(weapon + 0x98);
		if (!base_projectile)
			return false;

		auto recoil_prop = driver::read<uintptr_t>(base_projectile + 0x2C0);
		if (!recoil_prop)
			return false;

		int yaw_min = driver::read<float>(recoil_prop + 0x18);
		int yaw_max = driver::read<float>(recoil_prop + 0x1C);

		int pitch_min = driver::read<float>(recoil_prop + 0x20);
		int pitch_max = driver::read<float>(recoil_prop + 0x24);

		settings::yaw_min = yaw_min; settings::yaw_max = yaw_max; settings::pitch_min = pitch_min; settings::pitch_max = pitch_max;

		std::lock_guard guard(settings::recoil_mutex);
		settings::recoil_map[name] = { yaw_min, yaw_max, pitch_min, pitch_max };
	}

	uintptr_t get_active_weapon(uintptr_t _local_player)
	{
		if (!_local_player)
			return false;

		auto inventory = driver::read<uintptr_t>(_local_player + 0x5C8);
		if (!inventory)
			return {};

		auto contianer_belt = driver::read<uintptr_t>(inventory + 0x28);
		auto contents = driver::read<uintptr_t>(contianer_belt + 0x38);
		auto size = driver::read<int>(contents + 0x18);
		contents = driver::read<uintptr_t>(contents + 0x10);

		try {
			for (int i = 0; i < size; i++)
			{
				static std::vector<std::wstring>recorded{};
				auto item = driver::read<uintptr_t>(contents + (0x20 + (i * 0x8)));
				if (get_item_id(item) == get_active_item(_local_player))
				{
					static const auto weps = { L"shotgun", L"pistol", L"rifle", L"smg" };
					const auto item_name = get_item_name(item);
					for (auto wep : weps)
					{
						if (item_name.find(wep) != std::string::npos)
						{
							settings::current_weapon = to_string(item_name);

							// check if we've iterated over this weapon already
							try {
								if (std::find(recorded.begin(), recorded.end(), item_name) == recorded.end())
								{
									get_recoil_properties(item, settings::current_weapon);
									recorded.push_back(item_name);
								}
							}
							catch (const std::exception& exc) {
								std::cout << exc.what() << std::endl;
							}

							return item;
						}
					}
				}
			}
		}
		catch (const std::exception& exc) {
			std::cout << exc.what() << std::endl;
		}

		return {};
	}

	bool set_automatic(uintptr_t weapon)
	{
		auto base_projectile = driver::read<uintptr_t>(weapon + 0x98);
		if (!base_projectile)
			return false;

		return driver::write<bool>(base_projectile + 0x270, settings::auto_pistol);
	}

	bool set_recoil_props(uintptr_t weapon)
	{
		auto base_projectile = driver::read<uintptr_t>(weapon + 0x98);
		if (!base_projectile)
			return false;

		auto recoil_prop = driver::read<uintptr_t>(base_projectile + 0x2C0);
		if (!recoil_prop)
			return false;

		driver::write<float>(recoil_prop + 0x18, int(settings::yaw_min));
		driver::write<float>(recoil_prop + 0x1C, int(settings::yaw_max));

		driver::write<float>(recoil_prop + 0x20, int(settings::pitch_min));
		driver::write<float>(recoil_prop + 0x24, int(settings::pitch_max));
		return true;
	}
}