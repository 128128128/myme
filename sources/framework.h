#pragma once

#include <windows.h>
#include <tchar.h>
#include <sstream>

#include "Input/Input.h"

#include "./Functions/misc.h"
#include "./Functions/high_resolution_timer.h"

#include "../imgui/imgui.h"
#include "../imgui/imgui_internal.h"
#include "../imgui/imgui_impl_dx11.h"
#include "../imgui/imgui_impl_win32.h"
extern LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
extern ImWchar glyphRangesJapanese[];

#include <d3d11.h>
#include<map>
#include <dxgi1_6.h>

//#include "./Graphics/sprite.h"
//
//#include "./Graphics/sprite_batch.h"
//
//#include <wrl.h>
//
//#include "./Graphics/geometric_primitive.h"
//
//#include "./Graphics/static_mesh.h"
//
//#include "./Graphics/skinned_mesh.h"
//
//#include "./Scenes/scene.h"

#include "Graphics/sprite/sprite.h"

#include "Graphics/sprite/sprite_batch.h"

#include <wrl.h>

#include "Graphics/mesh/geometric_primitive.h"

#include "Graphics/mesh/static_mesh.h"

#include "Graphics/mesh/dynamic_mesh.h"

#include "./Scenes/scene.h"

CONST LONG SCREEN_WIDTH{ 1280 };
CONST LONG SCREEN_HEIGHT{ 720 };
CONST BOOL FULLSCREEN{ FALSE };
CONST LPWSTR APPLICATION_NAME{ L"X3DGP" };

class framework
{
public:
	CONST HWND hwnd;

	std::map<std::string, std::unique_ptr<Scene>> scene;
	std::string current_scene;

	Microsoft::WRL::ComPtr<ID3D11Device> device;
	Microsoft::WRL::ComPtr<ID3D11DeviceContext> immediate_context;
	Microsoft::WRL::ComPtr<IDXGISwapChain> swap_chain;
	Microsoft::WRL::ComPtr<ID3D11RenderTargetView> render_target_view;
	Microsoft::WRL::ComPtr<ID3D11DepthStencilView> depth_stencil_view;

	Microsoft::WRL::ComPtr<ID3D11DepthStencilState> default_depth_stencil_state;
	Microsoft::WRL::ComPtr<ID3D11RasterizerState> default_rasterizer_state;
	Microsoft::WRL::ComPtr<ID3D11BlendState> default_blend_state;
	Microsoft::WRL::ComPtr<ID3D11SamplerState> default_sampler_state;

	framework(HWND hwnd);
	~framework()=default;

	framework(const framework&) = delete;
	framework& operator=(const framework&) = delete;
	framework(framework&&) noexcept = delete;
	framework& operator=(framework&&) noexcept = delete;

	int run()
	{
		MSG msg{};

		if (!initialize())
		{
			return 0;
		}

		IMGUI_CHECKVERSION();
		ImGui::CreateContext();
		ImGui::GetIO().Fonts->AddFontFromFileTTF("C:\\Windows\\Fonts\\consola.ttf", 14.0f, nullptr, glyphRangesJapanese);
		ImGui::GetIO().ConfigFlags = ImGuiConfigFlags_DockingEnable;
		ImGui_ImplWin32_Init(hwnd);
		ImGui_ImplDX11_Init(device.Get(), immediate_context.Get());
		ImGui::StyleColorsDark();

		while (WM_QUIT != msg.message)
		{
			if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
			{
				TranslateMessage(&msg);
				DispatchMessage(&msg);
			}
			else
			{
				tictoc.tick();
				calculate_frame_stats();
				update(tictoc.time_interval());
				render(tictoc.time_interval());
			}
		}

		ImGui_ImplDX11_Shutdown();
		ImGui_ImplWin32_Shutdown();
		ImGui::DestroyContext();

		BOOL fullscreen{};
		swap_chain->GetFullscreenState(&fullscreen, 0);
		if (fullscreen)
		{
			swap_chain->SetFullscreenState(FALSE, 0);
		}

		uninitialize();
			
	    return static_cast<int>(msg.wParam);
	}

	LRESULT CALLBACK handle_message(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam)
	{
		if (ImGui_ImplWin32_WndProcHandler(hwnd, msg, wparam, lparam)) { return true; }

		Mouse& mouse = Input::Instance().GetMouse();

		switch (msg)
		{
		case WM_PAINT:
		{
			PAINTSTRUCT ps;
			BeginPaint(hwnd, &ps);

			EndPaint(hwnd, &ps);
			break;
		}
		case WM_DESTROY:
			PostQuitMessage(0);
			break;
		case WM_CREATE:
			break;
		case WM_KEYDOWN:
			if (wparam == VK_ESCAPE)
			{
				PostMessage(hwnd, WM_CLOSE, 0, 0);
			}
			break;
		case WM_ENTERSIZEMOVE:
			tictoc.stop();
			break;
		case WM_EXITSIZEMOVE:
			tictoc.start();
			break;
		case WM_MOUSEWHEEL:
		{
			zDelta = GET_WHEEL_DELTA_WPARAM(wparam); // ‰ñ“]—Ê
			// ƒmƒbƒ`”‚ð‹‚ß‚é
			nNotch = zDelta / WHEEL_DELTA;

			// ‰ñ“]’l‚ð•Û‘¶
			mouse.SetWheel(nNotch);
		}
		default:
			return DefWindowProc(hwnd, msg, wparam, lparam);
		}
		return 0;
	}

private:
	bool initialize();
	void update(float elapsed_time/*Elapsed seconds from last frame*/);
	void render(float elapsed_time/*Elapsed seconds from last frame*/);
	void uninitialize();

	Input input;

	int zDelta;
	int nNotch;
private:
	high_resolution_timer tictoc;
	uint32_t frames{ 0 };
	float elapsed_time{ 0.0f };
	void calculate_frame_stats()
	{
		if (++frames, (tictoc.time_stamp() - elapsed_time) >= 1.0f)
		{
			float fps = static_cast<float>(frames);
			std::wostringstream outs;
			outs.precision(6);
			outs << APPLICATION_NAME << L" : FPS : " << fps << L" / " << L"Frame Time : " << 1000.0f / fps << L" (ms)";
			SetWindowTextW(hwnd, outs.str().c_str());

			frames = 0;
			elapsed_time += 1.0f;
		}
	}
};

