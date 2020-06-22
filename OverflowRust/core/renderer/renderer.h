#pragma once
#include <algorithm>
#include <random>
#include <d3d11_2.h>
#include <d2d1_3helper.h>
#include <dwrite_3.h>
#include <dcomp.h>
#include <wrl.h>
#include <iterator>
#include <iostream>
#include <d3d11.h>
#include <dinput.h>
#include <tchar.h>
#include <string>
#include "../driver/driver.h"
#include <DirectXMath.h>

#pragma comment( lib, "dxgi" )
#pragma comment( lib, "d2d1" )
#pragma comment( lib, "d3d11" )
#pragma comment( lib, "dcomp" )
#pragma comment( lib, "dwrite" )

#define RET_CHK(x) if ( x != S_OK ) return;
#define RET_CHK2(x) if ( x != S_OK ) { driver::set_thread( remote_window, remote_thread ); return; }

class d2d_window_t
{
public:
	d2d_window_t()
	{
		_name.reserve(16u);
		std::generate_n(std::back_inserter(_name), 16u, []
			{
				thread_local std::mt19937_64 mersenne_engine(std::random_device{}());
				const std::uniform_int_distribution<> distribution(97, 122); // 'a', 'z'
				return static_cast<uint8_t>(distribution(mersenne_engine));
			});

		WNDCLASSA window_class
		{
			0,
			[](const HWND window, const UINT message, const WPARAM wparam, const LPARAM lparam) -> LRESULT
			{
				return DefWindowProcA(window, message, wparam, lparam);
			},
			0,
			0,
			GetModuleHandleW(nullptr),
			nullptr,
			nullptr,
			nullptr,
			nullptr,
			_name.c_str()
		};

		RegisterClassA(&window_class);

		_handle = CreateWindowExA(0, _name.c_str(), "", WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, nullptr, nullptr, window_class.hInstance, nullptr);

	}

	~d2d_window_t()
	{

		DestroyWindow(_handle);
		UnregisterClassA(_name.c_str(), GetModuleHandleW(nullptr));
	}

	HWND _handle;
	std::string _name;
};

namespace wnd_hjk
{
	using vec2_t = std::pair<float, float>;

	inline vec2_t screen_resolution{};
}

template <typename T>
using ComPtr = Microsoft::WRL::ComPtr<T>;

class _renderer
{
public:
	_renderer(const HWND local_window_handle, const HWND process_window_handle) : remote_window(process_window_handle), is_destroyed(false)
	{
		RET_CHK(D3D11CreateDevice(nullptr, D3D_DRIVER_TYPE_HARDWARE, nullptr, D3D11_CREATE_DEVICE_BGRA_SUPPORT, nullptr, 0, D3D11_SDK_VERSION, &d3d_device, nullptr, d3d_context.GetAddressOf()));

		printf("[+] hijacking window...\n", d3d_device.Get());
		printf("	[+] d3d11 device : 0x%p\n", d3d_device.Get());

		ComPtr<IDXGIDevice> dxgi_device;
		RET_CHK(d3d_device.As(&dxgi_device));

		printf("	[+] dxgi device : 0x%p\n", dxgi_device.Get());

		ComPtr<IDXGIFactory2> dxgi_factory;
		RET_CHK(CreateDXGIFactory2(0, __uuidof(IDXGIFactory2), reinterpret_cast<void**>(dxgi_factory.GetAddressOf())));

		printf("	[+] dxgi factory : 0x%p\n", dxgi_factory.Get());

		DXGI_SWAP_CHAIN_DESC1 description{ };
		description.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
		description.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
		description.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
		description.BufferCount = 2;
		description.AlphaMode = DXGI_ALPHA_MODE_PREMULTIPLIED;
		description.SampleDesc.Count = 1;

		RECT rect;
		if (!GetClientRect(remote_window, &rect))
			return;

		description.Width = static_cast<UINT>(rect.right - rect.left);
		description.Height = static_cast<UINT>(rect.bottom - rect.top);

		wnd_hjk::screen_resolution.first = static_cast<float>(description.Width);
		wnd_hjk::screen_resolution.second = static_cast<float>(description.Height);

		RET_CHK(dxgi_factory->CreateSwapChainForComposition(dxgi_device.Get(), &description, nullptr, dxgi_chain.GetAddressOf()));

		printf("	[+] dxgi swap chain : 0x%p\n", dxgi_chain.Get());

		const D2D1_FACTORY_OPTIONS factory_options = { D2D1_DEBUG_LEVEL_NONE };

		ComPtr<ID2D1Factory2> d2d_factory;
		RET_CHK(D2D1CreateFactory(D2D1_FACTORY_TYPE_SINGLE_THREADED, factory_options, d2d_factory.GetAddressOf()));

		printf("	[+] d2d1 factory : 0x%p\n", d2d_factory.Get());

		ComPtr<ID2D1Device1> d2d_device;
		RET_CHK(d2d_factory->CreateDevice(dxgi_device.Get(), d2d_device.GetAddressOf()));

		printf("	[+] d2d1 device : 0x%p\n", d2d_device.Get());

		RET_CHK(d2d_device->CreateDeviceContext(D2D1_DEVICE_CONTEXT_OPTIONS_ENABLE_MULTITHREADED_OPTIMIZATIONS, d2d_context.GetAddressOf()));

		printf("	[+] d2d1 context : 0x%p\n", d2d_context.Get());

		ComPtr<IDXGISurface2> dxgi_surface;
		RET_CHK(dxgi_chain->GetBuffer(0, __uuidof(IDXGISurface2), reinterpret_cast<void**>(dxgi_surface.GetAddressOf())));

		printf("	[+] dxgi surface : 0x%p\n", dxgi_surface.Get());

		D2D1_BITMAP_PROPERTIES1 bitmap_properties = { };
		bitmap_properties.pixelFormat.alphaMode = D2D1_ALPHA_MODE_PREMULTIPLIED;
		bitmap_properties.pixelFormat.format = DXGI_FORMAT_R8G8B8A8_UNORM;
		bitmap_properties.bitmapOptions = D2D1_BITMAP_OPTIONS_TARGET | D2D1_BITMAP_OPTIONS_CANNOT_DRAW;

		ComPtr<ID2D1Bitmap1> d2d_bitmap;
		RET_CHK(d2d_context->CreateBitmapFromDxgiSurface(dxgi_surface.Get(), bitmap_properties, d2d_bitmap.GetAddressOf()));

		printf("	[+] d2d1 bitmap : 0x%p\n", d2d_bitmap.Get());

		d2d_context->SetTarget(d2d_bitmap.Get());

		RET_CHK(d2d_context->CreateSolidColorBrush(D2D1::ColorF(D2D1::ColorF::Red), d2d_brush.GetAddressOf()));

		printf("	[+] d2d1 brush : 0x%p\n", d2d_brush.Get());

		create_factory();

		driver::get_thread(local_window_handle, &local_thread);
		driver::get_thread(remote_window, &remote_thread);

		printf("	[+] local window thread: 0x%llX\n	[+] remote window thread: 0x%llX\n", local_thread, remote_thread);

		printf("	[+] composing on window...\n");

		driver::set_thread(remote_window, local_thread);

		RET_CHK2(DCompositionCreateDevice(dxgi_device.Get(), __uuidof(IDCompositionDevice), reinterpret_cast<void**>(composition_device.GetAddressOf())))
		RET_CHK2(composition_device->CreateTargetForHwnd(remote_window, TRUE, composition_target.GetAddressOf()))
		RET_CHK2(composition_device->CreateVisual(composition_visual.GetAddressOf()))
		RET_CHK2(composition_visual->SetContent(dxgi_chain.Get()))
		RET_CHK2(composition_target->SetRoot(composition_visual.Get()))
		RET_CHK2(composition_device->Commit())
		RET_CHK2(composition_device->WaitForCommitCompletion())

		driver::set_thread(remote_window, remote_thread);
			
		printf("	[+] window composed\n");
		return;
	}

	void create_factory()
	{
		static const WCHAR msc_fontName[] = L"Verdana";
		static const FLOAT msc_fontSize = 8;
		HRESULT hr;
		hr = D2D1CreateFactory(D2D1_FACTORY_TYPE_SINGLE_THREADED, d2d_factory.GetAddressOf());

		if (SUCCEEDED(hr))
			hr = DWriteCreateFactory(DWRITE_FACTORY_TYPE_SHARED, __uuidof(d2d_write_factory), reinterpret_cast<IUnknown**>(d2d_write_factory.GetAddressOf()));
		if (SUCCEEDED(hr))
			hr = d2d_write_factory->CreateTextFormat(msc_fontName, NULL, DWRITE_FONT_WEIGHT_NORMAL, DWRITE_FONT_STYLE_NORMAL, DWRITE_FONT_STRETCH_NORMAL, msc_fontSize,L"", d2d_text_format.GetAddressOf());
	}

	void begin_scene()
	{
		d2d_context->BeginDraw();
		d2d_context->Clear();
	}

	void end_scene()
	{
		d2d_context->EndDraw();
		dxgi_chain->Present(0, 0);
	}

	void draw_rect(float x, float y, float w, float h, clr color)
	{
		const auto rectangle = D2D1::RectF(x, y, x + w, y + h);
		d2d_brush->SetColor(D2D1::ColorF(color.r / 255, color.g / 255, color.b / 255, color.a / 255));
		d2d_context->DrawRectangle(rectangle, d2d_brush.Get());
	}
		
	void draw_filled_rect(float x, float y, float w, float h, clr color)
	{
		const auto rectangle = D2D1::RectF(x, y, x + w, y + h);
		d2d_brush->SetColor(D2D1::ColorF(color.r / 255, color.g / 255, color.b / 255, color.a / 255));
		d2d_context->FillRectangle(rectangle, d2d_brush.Get());
	}

	void draw_health_bar(float x, float y, float w, float h, float val, float max, clr color)
	{
		draw_rect(x, y - 1, w + 1, h + 2, clr(0, 0, 0));
		draw_filled_rect(x, y + (h - (val / max * h)), w, (val / max * h), color);
	}
	  
	void draw_line(float x, float y, float x2, float y2, clr color)
	{
		const D2D1_POINT_2F from = { x, y };
		const D2D1_POINT_2F to = { x2, y2 };
		d2d_brush->SetColor(D2D1::ColorF(color.r / 255, color.g / 255, color.b / 255, color.a / 255));
		d2d_context->DrawLine(from, to, d2d_brush.Get());
	}	

	void draw_circle(float x, float y, float raidus, clr color)
	{
		D2D1_ELLIPSE ellipse = { D2D1_POINT_2F {x, y}, raidus, raidus };
		d2d_brush->SetColor(D2D1::ColorF(color.r / 255, color.g / 255, color.b / 255, color.a / 255));
		d2d_context->DrawEllipse(ellipse, d2d_brush.Get());
	}	

	void draw_filled_circle(float x, float y, float raidus, clr color)
	{
		D2D1_ELLIPSE ellipse = { D2D1_POINT_2F {x, y}, raidus, raidus };
		d2d_brush->SetColor(D2D1::ColorF(color.r / 255, color.g / 255, color.b / 255, color.a / 255));
		d2d_context->FillEllipse(ellipse, d2d_brush.Get());
	}
	
	void get_text_size(const std::wstring_view text, float* const width, float* const height)
	{
		if (!text.empty()) 
		{
			if (!width && !height) 
				return;
		
			RECT re;
			GetClientRect(remote_window, &re);
			FLOAT dpix, dpiy;
			dpix = static_cast<float>(re.right - re.left);
			dpiy = static_cast<float>(re.bottom - re.top);
			
			IDWriteTextLayout* layout = nullptr;
			const auto status = d2d_write_factory->CreateTextLayout(
				text.data(),
				static_cast<std::uint32_t>(text.length()),
				d2d_text_format.Get(),
				dpix,
				dpiy,
				&layout
			);

			if (SUCCEEDED(status)) 
			{
				DWRITE_TEXT_METRICS metrics{};
				if (SUCCEEDED(layout->GetMetrics(&metrics))) 
				{
					if (width)
						*width = metrics.width;
					if (height) 
						*height = metrics.height;
				}
				layout->Release();
			}
		}
	}

	void draw_text(std::wstring text, float x, float y, float font_size, bool center, clr color)
	{
		d2d_brush->SetColor(D2D1::ColorF(color.r / 255, color.g / 255, color.b / 255, color.a / 255));
		HRESULT res = d2d_write_factory->CreateTextLayout(text.c_str(), static_cast<UINT32>(text.length()) + 1, d2d_text_format.Get(), 1920, 1080, d2d_text_layout.GetAddressOf());
		if (SUCCEEDED(res))
		{
			float x2, y2;
			if (center)
				get_text_size(text, &x2, &y2);
			d2d_text_layout->SetFontSize(font_size, DWRITE_TEXT_RANGE{ 0, static_cast<UINT32>(text.length()) });
			D2D1_POINT_2F origin;
			center ? origin = { x - (x2 / 2), y } : origin = { x, y };
			d2d_context->DrawTextLayout(origin, d2d_text_layout.Get(), d2d_brush.Get());
			d2d_text_layout->Release();
		}
		//d2d_context->DrawTextA(text.c_str(), text.length() + 1, d2d_text_format.Get(), D2D1_RECT_F{ x, y }, d2d_brush.Get());
	}

	void manual_destruct()
	{
		if (is_destroyed)
			return;

		is_destroyed = true;

		driver::set_thread(remote_window, local_thread);

		composition_visual->SetContent(nullptr);
		composition_visual->Release();

		composition_target->SetRoot(nullptr);
		composition_target->Release();

		composition_device->Release();
		d2d_text_format->Release();
		d2d_text_layout->Release();
		d2d_write_factory->Release();
		d2d_factory->Release();

		driver::set_thread(remote_window, remote_thread);
	}

	~_renderer()
	{
		if (is_destroyed)
			return;

		this->manual_destruct();
	}

	ComPtr<IDXGISwapChain1> dxgi_chain;
	ComPtr<ID2D1DeviceContext> d2d_context;
	ComPtr<ID3D11DeviceContext> d3d_context;
	ComPtr<ID2D1Factory> d2d_factory;
	ComPtr<IDWriteFactory> d2d_write_factory;
	ComPtr<IDWriteTextFormat> d2d_text_format;
	ComPtr<IDWriteTextLayout> d2d_text_layout;
	
	ComPtr<IDCompositionDevice> composition_device;
	ComPtr<IDCompositionTarget> composition_target;
	ComPtr<IDCompositionVisual> composition_visual;
	ComPtr<ID2D1SolidColorBrush> d2d_brush;
	ComPtr<ID3D11Device> d3d_device;
	uint64_t local_thread;
	uint64_t remote_thread;
	
	HWND remote_window;

	bool is_destroyed;

};