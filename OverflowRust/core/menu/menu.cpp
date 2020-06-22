#include <Lmcons.h>
#include <time.h>
#include "menu.h"
#include "../helpers/settings.h"
#include "imgui/imgui_internal.h"
#include "../game/game.h"


void menu::render_buttons()
{
	
}

void menu::render_menu()
{
	// Style
	auto& style = GetStyle();

	// Menu
	SetNextWindowPos(ImVec2(0, 0));
	SetNextWindowSize(ImVec2(350 - 16, 450 - 39));
	static const auto dwFlags = ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoTitleBar;
	Begin("Overflow", 0, dwFlags);
	{
		ImGui::PushFont(font_menu);
		ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1, 1, 1, 1));

		menu_helpers::apply_padding(menu_helpers::padding);
		ImGui::BeginChild("Main", ImVec2(324, 450 - 39 - 4));
		{
			menu_helpers::toggle("Esp Toggle", &settings::esp_toggle, menu_helpers::to_color(menu_helpers::main_red));
			menu_helpers::toggle("Player Esp", &settings::esp_baseplayer.toggle, menu_helpers::to_color(menu_helpers::main_red));
			menu_helpers::color_picker(&settings::esp_baseplayer.color);
			menu_helpers::slider("Player Distance", &settings::esp_baseplayer.distance, 1, 1000);			
			
			menu_helpers::toggle("Scientist Esp", &settings::esp_scientist.toggle, menu_helpers::to_color(menu_helpers::main_red));
			menu_helpers::color_picker(&settings::esp_scientist.color);
			menu_helpers::slider("Player Distance", &settings::esp_scientist.distance, 1, 1000);

			menu_helpers::toggle("Stash Esp", &settings::esp_stash.toggle, menu_helpers::to_color(menu_helpers::main_red));
			menu_helpers::color_picker(&settings::esp_stash.color);
			menu_helpers::slider("Stash Distance", &settings::esp_stash.distance, 1, 1000);

			ImGui::Separator();

			menu_helpers::toggle("Spiderman", &settings::spider_man, menu_helpers::to_color(menu_helpers::main_red));

			ImGui::HotKey("Set Admin", &settings::set_admin, menu::font_menu);
			if (GetAsyncKeyState(settings::set_admin))
				game::set_admin();
			
			ImGui::Separator();

			std::string text = "";
			try
			{ 
				std::lock_guard guard(settings::recoil_mutex);
				auto org_recoils = settings::recoil_map[settings::current_weapon];
				if (!org_recoils.empty())
					text.append(std::string(" (" + std::to_string(org_recoils[0]) + ", " + std::to_string(org_recoils[1]) + ", " + std::to_string(org_recoils[2]) + ", " + std::to_string(org_recoils[3]) + ")"));
				else
					settings::recoil_map.erase(settings::current_weapon);
			} catch(...) {};
	
			menu_helpers::push_text(menu::font_large_menu, settings::current_weapon + text);
			menu_helpers::toggle("Toggle Recoil", &settings::allow_recoil, menu_helpers::to_color(menu_helpers::main_red));
			if (menu_helpers::toggle("Auto Pistol", &settings::auto_pistol, menu_helpers::to_color(menu_helpers::main_red), menu::font_menu, true))
				settings::auto_pistol_changed = true;

			if (menu_helpers::slider<int>("Yaw Min", &settings::yaw_min, -30, 30))
				settings::recoil_changed = true;
			if (menu_helpers::slider<int>("Yaw Max", &settings::yaw_max, -30, 30))
				settings::recoil_changed = true;
			if (menu_helpers::slider<int>("Pitch Min", &settings::pitch_min, -30, 30))
				settings::recoil_changed = true;
			if (menu_helpers::slider<int>("Pitch Max", &settings::pitch_max, -30, 30))
				settings::recoil_changed = true;

			if (ImGui::Button("End Cheat"))
				settings::end = true;
		}
		ImGui::EndChild();

		ImGui::PopFont();
		ImGui::PopStyleColor();
	}
	End();

	return;
}