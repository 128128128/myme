#include "framework.h"

//#include "./Graphics/shader.h"
//#include "./Graphics/texture.h"
//#include "./Scenes/game_scene.h"

#include <mutex>

#include "Graphics/others/shader.h"
#include "Graphics/load/texture.h"
#include "./Scenes/game_scene.h"
#include "./Scenes/create_terrain_scene.h"
#include "./Scenes/reflection_scene.h"
#include "./Scenes/ibl_scene.h"
#include "./Scenes/title_scene.h"

framework::framework(HWND hwnd) : hwnd(hwnd), input(hwnd)
{
}

//void acquire_high_performance_adapter(IDXGIFactory6* dxgi_factory6, IDXGIAdapter3** dxgi_adapter3)
//{
//	HRESULT hr = S_OK;
//
//	Microsoft::WRL::ComPtr<IDXGIAdapter3> enumerated_adapter;
//	for (UINT adapter_index = 0; DXGI_ERROR_NOT_FOUND != dxgi_factory6->EnumAdapterByGpuPreference(adapter_index, DXGI_GPU_PREFERENCE_HIGH_PERFORMANCE, IID_PPV_ARGS(enumerated_adapter.ReleaseAndGetAddressOf())); ++adapter_index)
//	{
//		DXGI_ADAPTER_DESC1 adapter_desc;
//		hr = enumerated_adapter->GetDesc1(&adapter_desc);
//		_ASSERT_EXPR(SUCCEEDED(hr), hr_trace(hr));
//
//		if (adapter_desc.VendorId == 0x1002/*AMD*/ || adapter_desc.VendorId == 0x10DE/*NVIDIA*/)
//		{
//			OutputDebugStringW((std::wstring(adapter_desc.Description) + L" has been selected.\n").c_str());
//			OutputDebugStringA(std::string("\tVendorId:" + std::to_string(adapter_desc.VendorId) + '\n').c_str());
//			OutputDebugStringA(std::string("\tDeviceId:" + std::to_string(adapter_desc.DeviceId) + '\n').c_str());
//			OutputDebugStringA(std::string("\tSubSysId:" + std::to_string(adapter_desc.SubSysId) + '\n').c_str());
//			OutputDebugStringA(std::string("\tRevision:" + std::to_string(adapter_desc.Revision) + '\n').c_str());
//			OutputDebugStringA(std::string("\tDedicatedVideoMemory:" + std::to_string(adapter_desc.DedicatedVideoMemory) + '\n').c_str());
//			OutputDebugStringA(std::string("\tDedicatedSystemMemory:" + std::to_string(adapter_desc.DedicatedSystemMemory) + '\n').c_str());
//			OutputDebugStringA(std::string("\tSharedSystemMemory:" + std::to_string(adapter_desc.SharedSystemMemory) + '\n').c_str());
//			OutputDebugStringA(std::string("\tAdapterLuid.HighPart:" + std::to_string(adapter_desc.AdapterLuid.HighPart) + '\n').c_str());
//			OutputDebugStringA(std::string("\tAdapterLuid.LowPart:" + std::to_string(adapter_desc.AdapterLuid.LowPart) + '\n').c_str());
//			OutputDebugStringA(std::string("\tFlags:" + std::to_string(adapter_desc.Flags) + '\n').c_str());
//			break;
//		}
//	}
//	*dxgi_adapter3 = enumerated_adapter.Detach();
//}


bool framework::initialize()
{
	HRESULT hr{ S_OK };

	Microsoft::WRL::ComPtr<IDXGIFactory> dxgi_factory;
	hr = CreateDXGIFactory(__uuidof(IDXGIFactory), reinterpret_cast<void**>(dxgi_factory.GetAddressOf()));
	_ASSERT_EXPR(SUCCEEDED(hr), hr_trace(hr));

	IDXGIAdapter* dxgi_adapter;
	std::vector <IDXGIAdapter*> dxgi_adapters;
	for (std::uint16_t i = 0; dxgi_factory->EnumAdapters(i, &dxgi_adapter) != DXGI_ERROR_NOT_FOUND; i++)
	{
		dxgi_adapters.push_back(dxgi_adapter);
	}
	dxgi_adapter = dxgi_adapters[0]; //TODO:
	DXGI_ADAPTER_DESC dxgi_adapter_desc;
	dxgi_adapter->GetDesc(&dxgi_adapter_desc);

	UINT create_device_flags{ 0 };
#ifdef _DEBUG
	create_device_flags |= D3D11_CREATE_DEVICE_DEBUG;
#endif
	D3D_DRIVER_TYPE driver_types[] =
	{
		D3D_DRIVER_TYPE_UNKNOWN,
		D3D_DRIVER_TYPE_HARDWARE,
		D3D_DRIVER_TYPE_WARP,
		D3D_DRIVER_TYPE_REFERENCE,
	};
	// used model version checked
	D3D_FEATURE_LEVEL feature_levels[] =
	{
		D3D_FEATURE_LEVEL_11_1,
		D3D_FEATURE_LEVEL_11_0,
		D3D_FEATURE_LEVEL_10_1,
		D3D_FEATURE_LEVEL_10_0,
		D3D_FEATURE_LEVEL_9_3,
		D3D_FEATURE_LEVEL_9_2,
		D3D_FEATURE_LEVEL_9_1,
	};
	D3D_FEATURE_LEVEL feature_level;

	for (D3D_DRIVER_TYPE driver_type : driver_types)
	{
		hr = D3D11CreateDevice(dxgi_adapter, driver_type, 0, create_device_flags, feature_levels, _countof(feature_levels), D3D11_SDK_VERSION, device.GetAddressOf(), &feature_level, immediate_context.GetAddressOf());
		if (SUCCEEDED(hr))
		{
			break;
		}

		_ASSERT_EXPR(SUCCEEDED(hr), hr_trace(hr));
		_ASSERT_EXPR(!(feature_level < D3D_FEATURE_LEVEL_11_0), L"sorry Direct3D Feature Level11 unsupported.");
	}
	{
		//swap chain setting 
		DXGI_SWAP_CHAIN_DESC swap_chain_desc{};

		//a value that describes the number of buffers in the swap chain
		swap_chain_desc.BufferCount = 1;

		// screen size setting
		swap_chain_desc.BufferDesc.Width = SCREEN_WIDTH;
		swap_chain_desc.BufferDesc.Height = SCREEN_HEIGHT;
		//a DXGI_FORMAT structure that describes the display format //表示形式
		swap_chain_desc.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
		swap_chain_desc.BufferDesc.RefreshRate.Numerator = 60;//60/1(Numerator/Denominator)回
		swap_chain_desc.BufferDesc.RefreshRate.Denominator = 1;//の更新を１秒間に行う

		//a DXGI_USAGE-typed
		//https://docs.microsoft.com/en-us/windows/win32/direct3ddxgi/dxgi-usage
		swap_chain_desc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
		swap_chain_desc.OutputWindow = hwnd;
		swap_chain_desc.SampleDesc.Count = 1; //multisampling per pixel
		swap_chain_desc.SampleDesc.Quality = 0; //quality 

		//switch fullscreen
		swap_chain_desc.Windowed = !FULLSCREEN;

		hr = dxgi_factory->CreateSwapChain(device.Get(), &swap_chain_desc, swap_chain.GetAddressOf());
		_ASSERT_EXPR(SUCCEEDED(hr), hr_trace(hr));
		/*hr = D3D11CreateDeviceAndSwapChain(NULL, D3D_DRIVER_TYPE_HARDWARE, NULL, create_device_flags,
			&feature_levels, 1, D3D11_SDK_VERSION, &swap_chain_desc,
			swap_chain.GetAddressOf(), device.GetAddressOf(), NULL, immediate_context.GetAddressOf());
		_ASSERT_EXPR(SUCCEEDED(hr), hr_trace(hr));*/
		//https://docs.microsoft.com/en-us/windows/win32/api/dxgi1_2/ns-dxgi1_2-dxgi_swap_chain_desc1
	}
	//-----------------------------------------------------------------
	//Rendertarget view
	//allow rendering to temporary intermediate buffer

	D3D11_TEXTURE2D_DESC render_target_desc;
	{Microsoft::WRL::ComPtr<ID3D11Texture2D> back_buffer{};
	hr = swap_chain->GetBuffer(0, __uuidof(ID3D11Texture2D), reinterpret_cast<LPVOID*>(back_buffer.GetAddressOf()));
	_ASSERT_EXPR(SUCCEEDED(hr), hr_trace(hr));

	hr = device->CreateRenderTargetView(back_buffer.Get(), NULL, render_target_view.GetAddressOf());
	_ASSERT_EXPR(SUCCEEDED(hr), hr_trace(hr));

	back_buffer->GetDesc(&render_target_desc);
	}
	//-----------------------------------------------------------------
	//Depthstencilbuffer
	D3D11_TEXTURE2D_DESC texture2d_desc{};
	{
		Microsoft::WRL::ComPtr<ID3D11Texture2D> depth_stencil_buffer{};
		texture2d_desc.Width = SCREEN_WIDTH;
		texture2d_desc.Height = SCREEN_HEIGHT;
		texture2d_desc.MipLevels = 1;//maximum number of mipmap levels
		texture2d_desc.ArraySize = 1;//number of textures in texture array
		texture2d_desc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
		//24bit for depth, 8bit for stencil , total 32bit Z buffer format 
		//https://docs.microsoft.com/en-us/windows/win32/api/dxgiformat/ne-dxgiformat-dxgi_format

		texture2d_desc.SampleDesc.Count = 1;
		texture2d_desc.SampleDesc.Quality = 0;
		//how the texture is to be read from and written to
		texture2d_desc.Usage = D3D11_USAGE_DEFAULT;
		//binding to pipeline stages
		texture2d_desc.BindFlags = D3D11_BIND_DEPTH_STENCIL;
		texture2d_desc.CPUAccessFlags = 0;
		texture2d_desc.MiscFlags = 0;
		hr = device->CreateTexture2D(&texture2d_desc, NULL, depth_stencil_buffer.GetAddressOf());
		_ASSERT_EXPR(SUCCEEDED(hr), hr_trace(hr));

		D3D11_DEPTH_STENCIL_VIEW_DESC depth_stencil_view_desc{};
		depth_stencil_view_desc.Format = texture2d_desc.Format;
		depth_stencil_view_desc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
		depth_stencil_view_desc.Texture2D.MipSlice = 0;
		hr = device->CreateDepthStencilView(depth_stencil_buffer.Get(), &depth_stencil_view_desc, depth_stencil_view.GetAddressOf());
		_ASSERT_EXPR(SUCCEEDED(hr), hr_trace(hr));
		//https://docs.microsoft.com/en-us/windows/win32/api/d3d11/ns-d3d11-d3d11_texture2d_desc
	}

	{
	    //setting viewport 
	D3D11_VIEWPORT viewport{};
	viewport.TopLeftX = 0;
	viewport.TopLeftY = 0;
	viewport.Width = static_cast<float>(SCREEN_WIDTH);
	viewport.Height = static_cast<float>(SCREEN_HEIGHT);
	viewport.MinDepth = 0.0f;
	viewport.MaxDepth = 1.0f;
	immediate_context->RSSetViewports(1, &viewport);
	//https://docs.microsoft.com/en-us/windows/win32/api/d3d11/ns-d3d11-cd3d11_viewport
    }
	{
		D3D11_SAMPLER_DESC sampler_desc;
		sampler_desc.Filter = D3D11_FILTER_ANISOTROPIC;
		sampler_desc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
		sampler_desc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
		sampler_desc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
		sampler_desc.MipLODBias = 0;
		sampler_desc.MaxAnisotropy = 16;
		sampler_desc.ComparisonFunc = D3D11_COMPARISON_ALWAYS;
		sampler_desc.BorderColor[0] = 0.0f;
		sampler_desc.BorderColor[1] = 0.0f;
		sampler_desc.BorderColor[2] = 0.0f;
		sampler_desc.BorderColor[3] = 0.0f;
		sampler_desc.MinLOD = 0;
		sampler_desc.MaxLOD = D3D11_FLOAT32_MAX;
		hr = device->CreateSamplerState(&sampler_desc, default_sampler_state.GetAddressOf());
		_ASSERT_EXPR(SUCCEEDED(hr), hr_trace(hr));
		//https://docs.microsoft.com/en-us/windows/win32/api/d3d11/ne-d3d11-d3d11_filter
	}
	{
		D3D11_DEPTH_STENCIL_DESC depth_stencil_desc;
		depth_stencil_desc.DepthEnable = TRUE;
		depth_stencil_desc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
		depth_stencil_desc.DepthFunc = D3D11_COMPARISON_LESS_EQUAL;
		depth_stencil_desc.StencilEnable = FALSE;
		depth_stencil_desc.StencilReadMask = 0xFF;
		depth_stencil_desc.StencilWriteMask = 0xFF;
		depth_stencil_desc.FrontFace.StencilFailOp = D3D11_STENCIL_OP_KEEP;
		depth_stencil_desc.FrontFace.StencilDepthFailOp = D3D11_STENCIL_OP_INCR;
		depth_stencil_desc.FrontFace.StencilPassOp = D3D11_STENCIL_OP_KEEP;
		depth_stencil_desc.FrontFace.StencilFunc = D3D11_COMPARISON_ALWAYS;
		depth_stencil_desc.BackFace.StencilFailOp = D3D11_STENCIL_OP_KEEP;
		depth_stencil_desc.BackFace.StencilDepthFailOp = D3D11_STENCIL_OP_DECR;
		depth_stencil_desc.BackFace.StencilPassOp = D3D11_STENCIL_OP_KEEP;
		depth_stencil_desc.BackFace.StencilFunc = D3D11_COMPARISON_ALWAYS;
		hr = device->CreateDepthStencilState(&depth_stencil_desc, default_depth_stencil_state.GetAddressOf());
		_ASSERT_EXPR(SUCCEEDED(hr), hr_trace(hr));
	}
	{
		D3D11_BLEND_DESC blend_desc = {};
		blend_desc.AlphaToCoverageEnable = FALSE;
		blend_desc.IndependentBlendEnable = FALSE;
		blend_desc.RenderTarget[0].BlendEnable = TRUE;
		blend_desc.RenderTarget[0].SrcBlend = D3D11_BLEND_SRC_ALPHA;
		blend_desc.RenderTarget[0].DestBlend = D3D11_BLEND_INV_SRC_ALPHA;
		blend_desc.RenderTarget[0].BlendOp = D3D11_BLEND_OP_ADD;
		blend_desc.RenderTarget[0].SrcBlendAlpha = D3D11_BLEND_ONE;
		blend_desc.RenderTarget[0].DestBlendAlpha = D3D11_BLEND_INV_SRC_ALPHA;
		blend_desc.RenderTarget[0].BlendOpAlpha = D3D11_BLEND_OP_ADD;
		blend_desc.RenderTarget[0].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;
		hr = device->CreateBlendState(&blend_desc, default_blend_state.GetAddressOf());
		_ASSERT_EXPR(SUCCEEDED(hr), hr_trace(hr));
	}
	{
		D3D11_RASTERIZER_DESC rasterizer_desc = {};
		rasterizer_desc.FillMode = D3D11_FILL_SOLID;
		rasterizer_desc.CullMode = D3D11_CULL_BACK;
		rasterizer_desc.FrontCounterClockwise = TRUE;
		rasterizer_desc.DepthBias = 0;
		rasterizer_desc.DepthBiasClamp = 0;
		rasterizer_desc.SlopeScaledDepthBias = 0;
		rasterizer_desc.DepthClipEnable = TRUE;
		rasterizer_desc.ScissorEnable = FALSE;
		rasterizer_desc.MultisampleEnable = TRUE;
		rasterizer_desc.AntialiasedLineEnable = FALSE;
		hr = device->CreateRasterizerState(&rasterizer_desc, default_rasterizer_state.GetAddressOf());
		_ASSERT_EXPR(SUCCEEDED(hr), hr_trace(hr));
	}

	float color[] = { 1.0f, 0.0f, 0.0f, 1.0f };
	immediate_context->ClearRenderTargetView(render_target_view.Get(), color);
	immediate_context->ClearDepthStencilView(depth_stencil_view.Get(), D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);

	immediate_context->OMSetRenderTargets(1, render_target_view.GetAddressOf(), depth_stencil_view.Get());
	immediate_context->OMSetDepthStencilState(default_depth_stencil_state.Get(), 1);

	current_scene = "ibl";

	scene["create_terrain"] = std::make_unique<create_terrain_scene>();
	scene["reflection"] = std::make_unique<reflection_scene>();
	scene["title"] = std::make_unique<title_scene>();
	scene["game"] = std::make_unique<game_scene>();
	scene["ibl"] = std::make_unique<ibl_scene>();
	scene[current_scene]->initialize(device.Get(), SCREEN_WIDTH,SCREEN_HEIGHT);


	return true;
}

void framework::update(float elapsed_time/*Elapsed seconds from last frame*/)
{

	input.Update();

		ImGui_ImplDX11_NewFrame();
		ImGui_ImplWin32_NewFrame();
		ImGui::NewFrame();

	static const char* next_scene = 0;
	next_scene = scene[current_scene]->update(elapsed_time);
	if (next_scene)
	{
		scene[current_scene]->finilize();
		scene[next_scene]->initialize(device.Get(), SCREEN_WIDTH, SCREEN_HEIGHT);
		current_scene = next_scene;
	}

}
void framework::render(float elapsed_time/*Elapsed seconds from last frame*/)
{
	// 別スレッド中にデバイスコンテキストが使われていた場合に // 同時アクセスしないように排他制御する
	std::lock_guard<std::mutex> lock(std::mutex);
	HRESULT hr{ S_OK };

	ID3D11RenderTargetView* null_render_target_views[D3D11_SIMULTANEOUS_RENDER_TARGET_COUNT] = {};
	ID3D11ShaderResourceView* null_shader_resource_views[D3D11_COMMONSHADER_INPUT_RESOURCE_SLOT_COUNT] = {};
	immediate_context->OMSetRenderTargets(_countof(null_render_target_views), null_render_target_views, 0);
	immediate_context->VSSetShaderResources(0, _countof(null_shader_resource_views), null_shader_resource_views);
	immediate_context->PSSetShaderResources(0, _countof(null_shader_resource_views), null_shader_resource_views);

	immediate_context->VSSetSamplers(0, 1, default_sampler_state.GetAddressOf());
	immediate_context->PSSetSamplers(0, 1, default_sampler_state.GetAddressOf());
	immediate_context->RSSetState(default_rasterizer_state.Get());
	immediate_context->OMSetBlendState(default_blend_state.Get(), 0, 0xFFFFFFFF);

	float color[] = { 0.0f, 1.0f, 0.0f, 1.0f };
	immediate_context->ClearRenderTargetView(render_target_view.Get(), color);
	immediate_context->ClearDepthStencilView(depth_stencil_view.Get(), D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);

	immediate_context->OMSetRenderTargets(1, render_target_view.GetAddressOf(), depth_stencil_view.Get());
	immediate_context->OMSetDepthStencilState(default_depth_stencil_state.Get(), 1);

	scene[current_scene]->render(immediate_context.Get(), elapsed_time);


		ImGui::Render();
		ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());

	UINT sync_interval{ 0 };
	swap_chain->Present(sync_interval, 0);
	_ASSERT_EXPR(SUCCEEDED(hr), hr_trace(hr));
}

void framework::uninitialize()
{
	scene[current_scene]->finilize();

}

