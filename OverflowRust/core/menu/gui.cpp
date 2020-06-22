#include <string>
#include <thread>
#include <dwmapi.h>
#include <iostream>
#include <ShlObj.h>
#include "menu.h"
#include "../helpers/settings.h"
#include "imgui/imgui_internal.h"

#pragma comment(lib, "dwmapi.lib")
#define STB_IMAGE_IMPLEMENTATION

using namespace ImGui;
using namespace settings;

// Forward declarations of helper functions
bool CreateDeviceD3D(HWND hWnd);
void CleanupDeviceD3D();
void CreateRenderTarget();
void CleanupRenderTarget();
ImVec4 clear_color;
LRESULT WINAPI WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
std::string status = {};

namespace menu
{
	ID3D11Device* g_pd3dDevice = NULL;
	ID3D11DeviceContext* g_pd3dDeviceContext = NULL;
	IDXGISwapChain* g_pSwapChain = NULL;
	ID3D11RenderTargetView* g_mainRenderTargetView = NULL;
	ID3D11BlendState* g_pBlendState = NULL;

	ImFont* font_title = nullptr;
	ImFont* font_large_menu = nullptr;
	ImFont* font_child_title = nullptr;
	ImFont* font_menu = nullptr;
	ImFont* font_bottom_info = nullptr;
	ImFont* font_spectators = nullptr;

	WNDCLASSEX wc = { NULL };
	HWND hwnd = NULL;
}

void menu::init()
{
	/* Colors */
	ImVec4* colors = ImGui::GetStyle().Colors;
	//colors[ImGuiCol_Text] = ImVec4(0.929, 0.290, 0.290, 1.00f); red
	colors[ImGuiCol_Text] = ImVec4(.6f, .6f, .6f, 1.00f); // grey
	colors[ImGuiCol_TextDisabled] = ImVec4(0.60f, 0.60f, 0.60f, 1.00f);
	colors[ImGuiCol_TextSelectedBg] = ImVec4(0.00f, 0.00f, 1.00f, 0.35f);
	colors[ImGuiCol_WindowBg] = menu_helpers::main_grey;
	//colors[ImGuiCol_WindowBg] = ImVec4(0.10f, 0.10f, 0.10f, 1.00f);
	colors[ImGuiCol_ChildWindowBg] = menu_helpers::main_light_grey;
	colors[ImGuiCol_PopupBg] = ImVec4(0.05f, 0.05f, 0.10f, 0.90f);
	colors[ImGuiCol_Border] = ImVec4(1, 1, 1, .8f);
	colors[ImGuiCol_BorderShadow] = ImVec4(0.f, 0, 0, .0f);
	colors[ImGuiCol_FrameBg] = menu_helpers::main_light_grey;
	colors[ImGuiCol_FrameBgHovered] = ImVec4(.6f, .6f, .6f, 0.40f);
	colors[ImGuiCol_FrameBgActive] = ImVec4(0.20f, 0.25f, 0.30f, 1.0f);
	colors[ImGuiCol_TitleBg] = ImVec4(0.10f, 0.10f, 0.10f, 1.00f);
	colors[ImGuiCol_TitleBgCollapsed] = ImVec4(0.20f, 0.20f, 0.20f, 1.00f);
	colors[ImGuiCol_TitleBgActive] = ImVec4(0.20f, 0.20f, 0.20f, 1.00f);
	colors[ImGuiCol_MenuBarBg] = ImVec4(0.40f, 0.40f, 0.55f, 0.80f);
	colors[ImGuiCol_ScrollbarBg] = ImVec4(0.10f, 0.10f, 0.10f, 1.0f);
	colors[ImGuiCol_ScrollbarGrab] = menu_helpers::main_red;
	colors[ImGuiCol_ScrollbarGrabHovered] = ImVec4(menu_helpers::main_red.x, menu_helpers::main_red.y, menu_helpers::main_red.z, .70f);
	colors[ImGuiCol_ScrollbarGrabActive] = ImVec4(menu_helpers::main_red.x, menu_helpers::main_red.y, menu_helpers::main_red.z, .70f);
	colors[ImGuiCol_Separator] = ImVec4(0.654, 0.094, 0.278, 1.f);
	colors[ImGuiCol_CheckMark] = menu_helpers::main_red;
	colors[ImGuiCol_SliderGrab] = ImVec4(1.00f, 1.00f, 1.00f, 0.30f);
	colors[ImGuiCol_SliderGrabActive] = ImVec4(0.80f, 0.50f, 0.50f, 1.00f);
	colors[ImGuiCol_Button] = ImVec4(0.10f, 0.10f, 0.10f, 1.00f);
	colors[ImGuiCol_ButtonHovered] = ImVec4(0.10f, 0.10f, 0.10f, 1.00f);
	colors[ImGuiCol_ButtonActive] = ImVec4(0.10f, 0.10f, 0.10f, 1.00f);
	//colors[ImGuiCol_Header] = ImVec4(237 / 255.f, 74 / 255.f, 74 / 255.f, .5f); //multicombo, combo selected item color.
	//colors[ImGuiCol_HeaderHovered] = ImVec4(35 / 255.f, 35 / 255.f, 35 / 255.f, 1.0f);
	//colors[ImGuiCol_HeaderActive] = ImVec4(35 / 255.f, 35 / 255.f, 35 / 255.f, 1.0f);	
	colors[ImGuiCol_Header] = ImVec4(0.1f, 0.1f, 0.1f, 1.); //multicombo, combo selected item color.
	colors[ImGuiCol_HeaderHovered] = ImVec4(0.26f, 0.26f, 0.26f, 1.f);
	colors[ImGuiCol_HeaderActive] = ImVec4(0.2f, 0.2f, 0.2f, 1.f);
	colors[ImGuiCol_ResizeGrip] = ImVec4(1.00f, 1.00f, 1.00f, 0.30f);
	colors[ImGuiCol_ResizeGripHovered] = ImVec4(1.00f, 1.00f, 1.00f, 0.60f);
	colors[ImGuiCol_ResizeGripActive] = ImVec4(1.00f, 1.00f, 1.00f, 0.90f);
	colors[ImGuiCol_PlotLines] = ImVec4(1.00f, 1.00f, 1.00f, 1.00f);
	colors[ImGuiCol_PlotLinesHovered] = ImVec4(0.90f, 0.70f, 0.00f, 1.00f);
	colors[ImGuiCol_PlotHistogram] = ImVec4(0.90f, 0.70f, 0.00f, 1.00f);
	colors[ImGuiCol_PlotHistogramHovered] = ImVec4(1.00f, 0.60f, 0.00f, 1.00f);
	colors[ImGuiCol_ModalWindowDarkening] = ImVec4(0.20f, 0.20f, 0.20f, 0.35f);

	SetColorEditOptions(ImGuiColorEditFlags_HEX);

	/* Setup Style */
	ImGuiStyle& style = GetStyle();

	style.Alpha = 1.f;
	style.WindowPadding = ImVec2(0, 0); // 8 or 9 x
	style.WindowMinSize = ImVec2(32, 32);
	style.WindowRounding = 0.0f;
	style.WindowTitleAlign = ImVec2(0.0f, 0.5f);
	style.ChildRounding = 0.0f;
	style.FramePadding = ImVec2(4, 3);
	style.FrameRounding = 0.0f;
	style.ItemSpacing = ImVec2(8, 8);
	style.ItemInnerSpacing = ImVec2(8, 8);
	style.TouchExtraPadding = ImVec2(0, 0);
	style.IndentSpacing = 21.0f;
	style.ColumnsMinSpacing = 0.0f;
	style.ScrollbarSize = 6.0f;
	style.ScrollbarRounding = 0.0f;
	style.GrabMinSize = 5.0f;
	style.GrabRounding = 0.0f;
	style.ButtonTextAlign = ImVec2(0.0f, 0.5f);
	style.DisplayWindowPadding = ImVec2(22, 22);
	style.DisplaySafeAreaPadding = ImVec2(4, 4);
	style.AntiAliasedLines = true;
	style.CurveTessellationTol = 1.f;

	ImGuiIO& io = GetIO();
	io.IniFilename = nullptr;
	io.LogFilename = nullptr;

	TCHAR szPath[MAX_PATH];
	SHGetFolderPath(NULL, CSIDL_FONTS, NULL, NULL, szPath);

	std::string Path = szPath;
	std::string verdana{ Path + "\\Verdana.ttf" };

	font_title = ImGui::GetIO().Fonts->AddFontFromFileTTF(verdana.c_str(), 24);
	font_large_menu = ImGui::GetIO().Fonts->AddFontFromFileTTF(verdana.c_str(), 20);
	font_child_title = ImGui::GetIO().Fonts->AddFontFromFileTTF(verdana.c_str(), 19);
	font_menu = ImGui::GetIO().Fonts->AddFontFromFileTTF(verdana.c_str(), 16);
	font_bottom_info = ImGui::GetIO().Fonts->AddFontFromFileTTF(verdana.c_str(), 15.5);
	font_spectators = ImGui::GetIO().Fonts->AddFontFromFileTTF(verdana.c_str(), 17.5);

	return;
}

void menu::menu_shutdown()
{
	ImGui_ImplDX11_Shutdown();
	ImGui_ImplWin32_Shutdown();
	ImGui::DestroyContext();

	CleanupDeviceD3D();
	::DestroyWindow(hwnd);
	::UnregisterClass(wc.lpszClassName, wc.hInstance);
	return;
}

void menu::begin_draw()
{
	// Start the Dear ImGui frame
	ImGui_ImplDX11_NewFrame();
	ImGui_ImplWin32_NewFrame();
	ImGui::NewFrame();
	wc.hCursor = LoadCursor(NULL, IDC_ARROW);
}

void menu::end_draw()
{
	// Rendering
	float clearColor[4] = { 0.0f,0.0f,0.0f,0.0f };
	ImGui::Render();
	g_pd3dDeviceContext->OMSetRenderTargets(1, &g_mainRenderTargetView, NULL);
	g_pd3dDeviceContext->ClearRenderTargetView(g_mainRenderTargetView, clearColor);
	ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());
	g_pSwapChain->Present(1, 0); // Present with vsync
}

void menu::setup_menu()
{
	// Create application window
	wc = { sizeof(WNDCLASSEX), NULL, WndProc, 0L, 0L, GetModuleHandle(NULL), NULL, NULL, NULL, NULL, ("Overflow"), NULL };
	::RegisterClassEx(&wc);
	hwnd = ::CreateWindow(wc.lpszClassName, (("Overflow  -  Rust")), 0, 1100, 300, 350, 450, NULL, NULL, wc.hInstance, NULL);

	// Initialize Direct3D
	if (!CreateDeviceD3D(hwnd))
	{
		CleanupDeviceD3D();
		::UnregisterClass(wc.lpszClassName, wc.hInstance);
		return;
	}

	SetLayeredWindowAttributes(hwnd, 0, 1.0f, LWA_ALPHA);
	SetLayeredWindowAttributes(hwnd, 0, RGB(0, 0, 0), LWA_COLORKEY);

	// Show the window
	::ShowWindow(hwnd, SW_SHOW);
	::UpdateWindow(hwnd);

	//MARGINS margins = { -1 };
	//DwmExtendFrameIntoClientArea(hwnd, &margins);

	// Setup Dear ImGui context
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO(); (void)io;

	// Setup Platform/Renderer bindings
	ImGui_ImplWin32_Init(hwnd);
	ImGui_ImplDX11_Init(g_pd3dDevice, g_pd3dDeviceContext);

	// Curosr
	wc.hCursor = LoadCursor(NULL, IDC_ARROW);

	// Colors
	init();

	// Our state
	bool show_demo_window = true;
	bool show_another_window = false;
	clear_color = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);

	return;
}

bool CreateDeviceD3D(HWND hWnd)
{
	// Setup swap chain
	DXGI_SWAP_CHAIN_DESC sd;
	ZeroMemory(&sd, sizeof(sd));
	sd.BufferCount = 2;
	sd.BufferDesc.Width = 0;
	sd.BufferDesc.Height = 0;
	sd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	sd.BufferDesc.RefreshRate.Numerator = 60;
	sd.BufferDesc.RefreshRate.Denominator = 1;
	sd.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;
	sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	sd.OutputWindow = hWnd;
	sd.SampleDesc.Count = 1;
	sd.SampleDesc.Quality = 0;
	sd.Windowed = TRUE;
	sd.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;

	UINT createDeviceFlags = 0;
	//createDeviceFlags |= D3D11_CREATE_DEVICE_DEBUG;
	D3D_FEATURE_LEVEL featureLevel;
	const D3D_FEATURE_LEVEL featureLevelArray[2] = { D3D_FEATURE_LEVEL_11_0, D3D_FEATURE_LEVEL_10_0, };
	if (D3D11CreateDeviceAndSwapChain(NULL, D3D_DRIVER_TYPE_HARDWARE, NULL, createDeviceFlags, featureLevelArray, 2, D3D11_SDK_VERSION, &sd, &menu::g_pSwapChain, &menu::g_pd3dDevice, &featureLevel, &menu::g_pd3dDeviceContext) != S_OK)
		return false;

	CreateRenderTarget();
	return true;
}

void CleanupDeviceD3D()
{
	CleanupRenderTarget();
	if (menu::g_pSwapChain) { menu::g_pSwapChain->Release(); menu::g_pSwapChain = NULL; }
	if (menu::g_pd3dDeviceContext) { menu::g_pd3dDeviceContext->Release(); menu::g_pd3dDeviceContext = NULL; }
	if (menu::g_pd3dDevice) { menu::g_pd3dDevice->Release(); menu::g_pd3dDevice = NULL; }
	return;
}

void CreateRenderTarget()
{
	ID3D11Texture2D* pBackBuffer;
	menu::g_pSwapChain->GetBuffer(0, IID_PPV_ARGS(&pBackBuffer));
	menu::g_pd3dDevice->CreateRenderTargetView(pBackBuffer, NULL, &menu::g_mainRenderTargetView);
	pBackBuffer->Release();
	return;
}

void CleanupRenderTarget()
{
	if (menu::g_mainRenderTargetView) { menu::g_mainRenderTargetView->Release(); menu::g_mainRenderTargetView = NULL; }
	return;
}

// Win32 message handler
extern LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
LRESULT WINAPI WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	RECT rect;
	if (ImGui_ImplWin32_WndProcHandler(hWnd, msg, wParam, lParam))
		return true;

	switch (msg)
	{
	case WM_SIZE:
		if (menu::g_pd3dDevice != NULL && wParam != SIZE_MINIMIZED)
		{
			CleanupRenderTarget();
			menu::g_pSwapChain->ResizeBuffers(0, (UINT)LOWORD(lParam), (UINT)HIWORD(lParam), DXGI_FORMAT_UNKNOWN, 0);
			CreateRenderTarget();
		}
		return 0;
	case WM_SYSCOMMAND:
		if ((wParam & 0xfff0) == SC_KEYMENU) // Disable ALT application menu
			return 0;
		break;
	case WM_DESTROY:
		::PostQuitMessage(0);
		return 0;
	}
	return ::DefWindowProc(hWnd, msg, wParam, lParam);
}