#include "Settings.h"
#include <vector>
#include <map>
#include <mutex>

namespace settings
{
	bool menu_opened = true;
	bool end = false;
	bool cheat_thread = false;

	std::string current_weapon = "";
	std::map < std::string, std::vector<int> > recoil_map;
	std::mutex recoil_mutex;
	bool recoil_changed = true;

	bool esp_toggle = false;
	esp_var esp_stash = {false, 200, clr(255, 255, 0)};
	esp_var esp_scientist = { false, 200, clr(0, 255, 255) };
	esp_var esp_baseplayer = { false, 200, clr(50, 255, 50) };

	int set_admin = 0;
	bool spider_man = false;

	bool auto_pistol = false;
	bool auto_pistol_changed = true;
	bool allow_recoil = false;
	int yaw_min = -2;
	int yaw_max = 2;
	int pitch_min = -10;
	int pitch_max = -10;
}