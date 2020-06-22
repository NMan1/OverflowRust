#pragma once
#include <dxgi.h>
#include <d3d11.h>
#define IMGUI_DEFINE_MATH_OPERATORS
#include "imgui/imgui.h"
#include "imgui/imgui_impl_win32.h"
#include "imgui/imgui_impl_dx11.h" 
#include "imgui/imgui_internal.h"
#include "../helpers/color.hpp"
#include "../helpers/vectors.h"
#include "../driver/driver.h"
#include "../helpers/settings.h"


using namespace ImGui;

namespace menu
{
	extern ID3D11Device* g_pd3dDevice;
	extern ID3D11DeviceContext* g_pd3dDeviceContext;
	extern IDXGISwapChain* g_pSwapChain;
	extern ID3D11RenderTargetView* g_mainRenderTargetView;
	extern ID3D11BlendState* g_pBlendState;

	extern ImFont* font_title;
	extern ImFont* font_large_menu;
	extern ImFont* font_child_title;
	extern ImFont* font_menu;
	extern ImFont* font_bottom_info;
	extern ImFont* font_spectators;

	extern WNDCLASSEX wc;
	extern HWND hwnd;
	void render_menu();

	void change_log();

	void legit_tab();

	void rage_tab();

	void visuals_tab();

	void misc_tab();

	void skins_tab();

	void settings_tab(); 

	void render_buttons();

    void init();

    void menu_shutdown();

    void begin_draw();

    void end_draw();

    void setup_menu();
};

namespace menu_helpers
{
	static ImVec2 real_padding{ 12, 17 }; // the real padding applied
	static ImVec2 padding{ 5, 5 }; //  y = 17 with child title text
	static ImVec4 main_red = { 1, 0.321, 0.321, 1 };
	static ImVec4 main_grey = { 0.133, 0.133, 0.133, 1 };
	static ImVec4 main_light_grey = { 0.149, 0.149, 0.149, 1 };
	static ImVec2 old_pos = { 0, 0 };
	static int old_width = 0;
	static int last_widget = -1;

	static clr to_color(ImVec4 in)
	{
		return clr(in.x * 255, in.y * 255, in.z * 255, in.w * 255);
	}

	static clr to_color(float* in)
	{
		return clr(in[0], in[1], in[2], in[3]);
	}

	static ImVec4 to_vec(clr in)
	{
		return ImVec4(in.r / 255, in.g / 255, in.b / 255, in.a / 255);
	}

	static void to_float(clr in, float* out)
	{
		out[0] = in.r;
		out[1] = in.g;
		out[2] = in.b;
		out[3] = in.a;
	}

	static void apply_padding(ImVec2 pad)
	{
		pad.x += ImGui::GetCursorPosX();
		pad.y += ImGui::GetCursorPosY();
		ImGui::SetCursorPos(pad);
	}

	static void push_text(ImFont* font, std::string text, clr clr = {}, ImVec2 pos = {})
	{
		if (pos.x != 0 && pos.y != 0)
		{
			ImGui::SetCursorPosY(pos.y); // center vertically
			ImGui::SetCursorPosX(pos.x); // positon left
		}
		else
			ImGui::SetCursorPosX(ImGui::GetCursorPosX() + 4); // positon left

		if (clr.r && clr.g && clr.b)
			ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(clr.r / 255, clr.g / 255, clr.b / 255, 1));

		ImGui::PushFont(font);
		ImGui::Text(text.c_str());
		ImGui::PopFont();

		if (clr.r && clr.g && clr.b)
			ImGui::PopStyleColor();
		last_widget = 0;
	}

	static bool toggle(const char* str_id, bool* v, clr clr, ImFont* font = menu::font_menu, bool same_line = false)
	{
		apply_padding(padding);

		if (same_line)
		{
			// far right center
			auto x = ((350 - 16) - 22) - ImGui::CalcTextSize(str_id).x - old_width - 15; // width of menu - text size - width - distance between slider and text
			old_pos.x = x;
			ImGui::SetCursorPos(old_pos);
		}
		else
			old_pos = ImGui::GetCursorPos();

		ImVec2 p = ImGui::GetCursorScreenPos();
		ImDrawList* draw_list = ImGui::GetWindowDrawList();

		float height = (ImGui::GetFontSize() + ImGui::GetStyle().FramePadding.y * 2) - 7;
		float width = height * 1.55f;
		float radius = height * 0.50f;
		old_width = width;

		auto gay = false;
		if (ImGui::InvisibleButton(str_id, ImVec2(width, height)))
		{
			*v = !*v;
			gay = true;
		}
		ImU32 col_bg;
		if (ImGui::IsItemHovered())
			col_bg = *v ? IM_COL32(clr.r, clr.g - 40, clr.b - 40, clr.a) : IM_COL32(133, 133, 133, 255); // if true/false and hovered
		else
			col_bg = *v ? IM_COL32(clr.r, clr.g, clr.b, clr.a) : IM_COL32(166, 166, 166, 255); // if true/false and not hovered

		draw_list->AddRectFilled(p, ImVec2(p.x + width, p.y + height), col_bg, height * 0.5f);
		draw_list->AddCircleFilled(ImVec2(*v ? (p.x + width - radius) : (p.x + radius), p.y + radius), radius, IM_COL32(255, 255, 255, 255));
		ImGui::PushFont(font);
		draw_list->AddText(ImVec2((p.x + width - radius) + 20, (p.y + radius) - ImGui::CalcTextSize(str_id).y * .5 - 1.5), ImColor(255, 255, 255, 255), str_id);
		ImGui::PopFont();
		last_widget = 2;
		return gay;
	}

	template <class t>
	static bool slider(const char* label, t* v, float v_min, float v_max)
	{
		ImGuiWindow* window = ImGui::GetCurrentWindow();
		if (window->SkipItems)
			return false;

		ImGuiContext& g = *GImGui;
		const ImGuiStyle& style = g.Style;
		const ImGuiID id = window->GetID(label);
		const float w = ImGui::CalcItemWidth() * .65;

		const ImVec2 label_size = ImGui::CalcTextSize(label, NULL, true);
		auto text_pos = ImVec2(window->DC.CursorPos.x + 5, window->DC.CursorPos.y);

		window->DC.CursorPos.x -= real_padding.x;
		window->DC.CursorPos.x += window->Size.x - w - real_padding.x + 3; // positions to far right
		const ImRect frame_bb(ImVec2(window->DC.CursorPos.x, window->DC.CursorPos.y + label_size.y * .5 - 2), window->DC.CursorPos + ImVec2(w, label_size.y + 2));
		const ImRect total_bb(frame_bb.Min, frame_bb.Max + ImVec2(label_size.x > 0.0f ? style.ItemInnerSpacing.x + label_size.x : 0.0f, 2.0f));

		// NB- we don't call ItemSize() yet because we may turn into a text edit box below
		if (!ImGui::ItemAdd(total_bb, id)) {
			ImGui::ItemSize(total_bb, style.FramePadding.y);
			return false;
		}

		const bool hovered = ImGui::ItemHoverable(frame_bb, id);
		if (hovered)
			ImGui::SetHoveredID(id);

		int decimal_precision = ImGui::ParseFormatPrecision("%.0f", 3);

		// Tabbing or CTRL-clicking on Slider turns it into an input box
		bool start_text_input = false;
		const bool tab_focus_requested = ImGui::FocusableItemRegister(window, g.ActiveId == id);
		if (tab_focus_requested || (hovered && g.IO.MouseClicked[0])) {
			ImGui::SetActiveID(id, window);
			ImGui::FocusWindow(window);

			if (tab_focus_requested || g.IO.KeyCtrl) {
				start_text_input = true;
				g.TempInputTextId = 0;
			}
		}
		if (start_text_input || (g.ActiveId == id && g.TempInputTextId == id))
			return ImGui::InputScalarAsWidgetReplacement(frame_bb, label, ImGuiDataType_Float, v, id, decimal_precision);

		ImGui::ItemSize(total_bb, style.FramePadding.y);
		
		// Actual slider behavior + render grab
		ImRect out_pos;
		float v_f = (float)*v;
		const bool value_changed = ImGui::SliderBehavior(frame_bb, id, "%.0f", std::is_same_v<t, int> ? &v_f : (float*)v, v_min, v_max, 1, decimal_precision, out_pos);
		std::is_same_v<t, int> ? *v = (int)v_f : true;

		// Display value using user-provided display format so user can add prefix/suffix/decorations to the value.
		char value_buf[64];
		const char* value_buf_end = value_buf + ImFormatString(value_buf, IM_ARRAYSIZE(value_buf), "%.0f", *v);
		if (label_size.x > 0.0f)
		{
			std::string text{ label };
			text.append(" (").append(std::is_same_v<t, int> ? std::to_string(*v) : value_buf).append(")");
			ImGui::RenderText(text_pos, text.c_str());
		}
		last_widget = 3;
		return value_changed;
	}

	static bool color_picker(clr* color)
	{
		float temp[4];
		to_float(*color, temp);
		auto val = ImGui::ColorEdit5(temp, ImGuiColorEditFlags_NoInputs);
		*color = to_color(temp);
		if (last_widget == 2)
			ImGui::SetCursorPosY(ImGui::GetCursorPosY() - 5);
		last_widget = 4;
		return val;
	}
}