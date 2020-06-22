#pragma once
#include <string>
#include <d3d9.h>
#include "color.hpp"
#include <vector>
#include <map>
#include <mutex>

struct esp_var
{
	bool toggle;
	int distance;
	clr color;
};

namespace settings
{
	extern bool menu_opened;
	extern bool end;
	extern bool cheat_thread;

	extern std::string current_weapon;
	extern std::map <std::string, std::vector<int>> recoil_map;
	extern std::mutex recoil_mutex;
	extern bool recoil_changed;

	extern bool esp_toggle;
	extern esp_var esp_stash;
	extern esp_var esp_baseplayer;
	extern esp_var esp_scientist;

	extern int set_admin;
	extern bool spider_man;

	extern bool auto_pistol;
	extern bool auto_pistol_changed;
	extern bool allow_recoil;
	extern int yaw_min;
	extern int yaw_max;
	extern int pitch_min;
	extern int pitch_max;
}