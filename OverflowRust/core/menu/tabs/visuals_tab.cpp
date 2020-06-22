//#include "../menu.h"
//#include "../helpers/menu_helpers.h"
//
//const char* chams_type[] =
//{
//	"textured",
//	"metallic",
//	"flat",
//	"chrome",
//	"glow",
//	"crystal"
//	"wildfire"
//};
//
//void main_child();
//void weapon_child();
//void chams_child();
//void misc_child();
//void world_child();
//
//ImVec2 padding{ 5, 10 }; //  y = 17 with child title text
//ImVec2 real_padding{ 12, 17 }; // the real padding applied
//
//void menu::visuals_tab()
//{
//	// no need for sepreate functions
//	// but imo its just more ogranized
//	main_child();
//	weapon_child();
//	helpers::change_pos(ImVec2((menu.menu_size.x - real_padding.x * 3) * .5 + (real_padding.x * 2) - 4, padding.y - 2));
//	chams_child();
//	helpers::change_pos(ImVec2((menu.menu_size.x - real_padding.x * 3) * .5 + (real_padding.x * 2) - 4, (padding.y * 2) + ((menu.menu_size.y - real_padding.y * 3) * .4)));
//	misc_child();
//	helpers::change_pos(ImVec2((menu.menu_size.x - real_padding.x * 3) * .5 + (real_padding.x * 2) - 4, (padding.y * 3) + ((menu.menu_size.y - real_padding.y * 3) * .7 + 1)));
//	world_child();
//}
//
//void main_child()
//{
//	helpers::apply_padding(padding);
//	ImGui::BeginChild("ESP Main", ImVec2((menu.menu_size.x - real_padding.x * 3) * .5, (menu.menu_size.y - real_padding.y * 3) * .65), true, ImGuiWindowFlags_AlwaysUseWindowPadding);
//	{
//		auto window = ImGui::GetCurrentWindow();
//		//helpers::toggle("Toggle ESP", &settings::esp, helpers::to_color(helpers::main_red), menu.font_menu);
//		//helpers::color_picker(&settings::chams_clr);
//		////helpers::slider<int>("Size", &settings::speed_value, 0, 100);
//		//helpers::combo("Selection Type", &settings::aim_selection, chams_type, IM_ARRAYSIZE(chams_type));
//	}
//	ImGui::EndChild(true, menu.font_child_title);
//}
//
//void weapon_child()
//{
//	helpers::apply_padding(padding);
//	helpers::change_pos(ImVec2(ImGui::GetCursorPosX() , ImGui::GetCursorPosY() - 6));
//	ImGui::BeginChild("Weapons", ImVec2((menu.menu_size.x - real_padding.x * 3) * .5, (menu.menu_size.y - real_padding.y * 3) * .35 + 9), true, ImGuiWindowFlags_AlwaysUseWindowPadding);
//	{
//		auto window = ImGui::GetCurrentWindow();
//	}
//	ImGui::EndChild(true, menu.font_child_title);
//}
//
//void chams_child()
//{
//	helpers::apply_padding(padding);
//	ImGui::BeginChild("Chams", ImVec2((menu.menu_size.x - real_padding.x * 3) * .5, (menu.menu_size.y - real_padding.y * 3) * .4), true, ImGuiWindowFlags_AlwaysUseWindowPadding);
//	{
//		auto window = ImGui::GetCurrentWindow();
//	}
//	ImGui::EndChild(true, menu.font_child_title);
//}
//
//void misc_child()
//{
//	helpers::apply_padding(padding);
//	ImGui::BeginChild("Misc", ImVec2((menu.menu_size.x - real_padding.x * 3) * .5, (menu.menu_size.y - real_padding.y * 3) * .3), true, ImGuiWindowFlags_AlwaysUseWindowPadding);
//	{
//		auto window = ImGui::GetCurrentWindow();
//	}
//	ImGui::EndChild(true, menu.font_child_title);
//}
//
//void world_child()
//{
//	helpers::apply_padding(padding);
//	ImGui::BeginChild("World", ImVec2((menu.menu_size.x - real_padding.x * 3) * .5, (menu.menu_size.y - real_padding.y * 3) * .3 - 2), true, ImGuiWindowFlags_AlwaysUseWindowPadding);
//	{
//		auto window = ImGui::GetCurrentWindow();
//	}
//	ImGui::EndChild(true, menu.font_child_title);
//}
