#include "create_terrain_scene.h"

#define screenW 1280
#define screenH 720
bool create_terrain_scene::initialize(ID3D11Device* device, const LONG screen_width, const LONG screen_height)
{
	HRESULT hr{ S_OK };

	//sampler setting
	D3D11_SAMPLER_DESC sampler_desc[]{
		{D3D11_FILTER_MIN_MAG_MIP_POINT/*Filter*/, D3D11_TEXTURE_ADDRESS_WRAP/*AddressU*/, D3D11_TEXTURE_ADDRESS_WRAP/*AddressV*/, D3D11_TEXTURE_ADDRESS_WRAP/*AddressW*/, 0.0f/*MipLODBias*/, 0/*MaxAnisotropy*/, D3D11_COMPARISON_NEVER/*ComparisonFunc*/, {0, 0, 0, 0}/*BorderColor[4]*/, 0.0f/*MinLOD*/, D3D11_FLOAT32_MAX/*MaxLOD*/},
		{D3D11_FILTER_MIN_MAG_MIP_LINEAR/*Filter*/, D3D11_TEXTURE_ADDRESS_WRAP/*AddressU*/, D3D11_TEXTURE_ADDRESS_WRAP/*AddressV*/, D3D11_TEXTURE_ADDRESS_WRAP/*AddressW*/, 0.0f/*MipLODBias*/, 0/*MaxAnisotropy*/, D3D11_COMPARISON_NEVER/*ComparisonFunc*/, {0, 0, 0, 0}/*BorderColor[4]*/, 0.0f/*MinLOD*/, D3D11_FLOAT32_MAX/*MaxLOD*/},
		{D3D11_FILTER_ANISOTROPIC/*Filter*/, D3D11_TEXTURE_ADDRESS_WRAP/*AddressU*/, D3D11_TEXTURE_ADDRESS_WRAP/*AddressV*/, D3D11_TEXTURE_ADDRESS_WRAP/*AddressW*/, 0.0f/*MipLODBias*/, 8/*MaxAnisotropy*/, D3D11_COMPARISON_NEVER/*ComparisonFunc*/, {0, 0, 0, 0}/*BorderColor[4]*/, 0.0f/*MinLOD*/, D3D11_FLOAT32_MAX/*MaxLOD*/},
		{D3D11_FILTER_MIN_MAG_MIP_POINT/*Filter*/, D3D11_TEXTURE_ADDRESS_CLAMP/*AddressU*/, D3D11_TEXTURE_ADDRESS_CLAMP/*AddressV*/, D3D11_TEXTURE_ADDRESS_CLAMP/*AddressW*/, 0.0f/*MipLODBias*/, 0/*MaxAnisotropy*/, D3D11_COMPARISON_NEVER/*ComparisonFunc*/, {0, 0, 0, 0}/*BorderColor[4]*/, 0.0f/*MinLOD*/, D3D11_FLOAT32_MAX/*MaxLOD*/},
		{D3D11_FILTER_MIN_MAG_MIP_LINEAR/*Filter*/, D3D11_TEXTURE_ADDRESS_CLAMP/*AddressU*/, D3D11_TEXTURE_ADDRESS_CLAMP/*AddressV*/, D3D11_TEXTURE_ADDRESS_CLAMP/*AddressW*/, 0.0f/*MipLODBias*/, 0/*MaxAnisotropy*/, D3D11_COMPARISON_NEVER/*ComparisonFunc*/, {0, 0, 0, 0}/*BorderColor[4]*/, 0.0f/*MinLOD*/, D3D11_FLOAT32_MAX/*MaxLOD*/},
		{D3D11_FILTER_ANISOTROPIC/*Filter*/, D3D11_TEXTURE_ADDRESS_CLAMP/*AddressU*/, D3D11_TEXTURE_ADDRESS_CLAMP/*AddressV*/, D3D11_TEXTURE_ADDRESS_CLAMP/*AddressW*/, 0.0f/*MipLODBias*/, 8/*MaxAnisotropy*/, D3D11_COMPARISON_NEVER/*ComparisonFunc*/, {0, 0, 0, 0}/*BorderColor[4]*/, 0.0f/*MinLOD*/, D3D11_FLOAT32_MAX/*MaxLOD*/},
		{D3D11_FILTER_MIN_MAG_MIP_POINT/*Filter*/, D3D11_TEXTURE_ADDRESS_BORDER/*AddressU*/, D3D11_TEXTURE_ADDRESS_BORDER/*AddressV*/, D3D11_TEXTURE_ADDRESS_BORDER/*AddressW*/, 0.0f/*MipLODBias*/, 0/*MaxAnisotropy*/, D3D11_COMPARISON_NEVER/*ComparisonFunc*/, {0, 0, 0, 1}/*BorderColor[4]*/, 0.0f/*MinLOD*/, D3D11_FLOAT32_MAX/*MaxLOD*/},
		{D3D11_FILTER_MIN_MAG_MIP_LINEAR/*Filter*/, D3D11_TEXTURE_ADDRESS_BORDER/*AddressU*/, D3D11_TEXTURE_ADDRESS_BORDER/*AddressV*/, D3D11_TEXTURE_ADDRESS_BORDER/*AddressW*/, 0.0f/*MipLODBias*/, 0/*MaxAnisotropy*/, D3D11_COMPARISON_NEVER/*ComparisonFunc*/, {0, 0, 0, 1}/*BorderColor[4]*/, 0.0f/*MinLOD*/, D3D11_FLOAT32_MAX/*MaxLOD*/},
		{D3D11_FILTER_MIN_MAG_MIP_POINT/*Filter*/, D3D11_TEXTURE_ADDRESS_BORDER/*AddressU*/, D3D11_TEXTURE_ADDRESS_BORDER/*AddressV*/, D3D11_TEXTURE_ADDRESS_BORDER/*AddressW*/, 0.0f/*MipLODBias*/, 0/*MaxAnisotropy*/, D3D11_COMPARISON_NEVER/*ComparisonFunc*/, {1, 1, 1, 1}/*BorderColor[4]*/, 0.0f/*MinLOD*/, D3D11_FLOAT32_MAX/*MaxLOD*/},
		{D3D11_FILTER_MIN_MAG_MIP_LINEAR/*Filter*/, D3D11_TEXTURE_ADDRESS_BORDER/*AddressU*/, D3D11_TEXTURE_ADDRESS_BORDER/*AddressV*/, D3D11_TEXTURE_ADDRESS_BORDER/*AddressW*/, 0.0f/*MipLODBias*/, 0/*MaxAnisotropy*/, D3D11_COMPARISON_NEVER/*ComparisonFunc*/, {1, 1, 1, 1}/*BorderColor[4]*/, 0.0f/*MinLOD*/, D3D11_FLOAT32_MAX/*MaxLOD*/},
		{D3D11_FILTER_COMPARISON_MIN_MAG_LINEAR_MIP_POINT/*Filter*/, D3D11_TEXTURE_ADDRESS_BORDER/*AddressU*/, D3D11_TEXTURE_ADDRESS_BORDER/*AddressV*/, D3D11_TEXTURE_ADDRESS_BORDER/*AddressW*/, 0.0f/*MipLODBias*/, 16/*MaxAnisotropy*/, D3D11_COMPARISON_LESS_EQUAL/*ComparisonFunc*/, {0, 0, 0, 0}/*BorderColor[4]*/, 0.0f/*MinLOD*/, D3D11_FLOAT32_MAX/*MaxLOD*/},
	};

	for (size_t sampler_index = 0; sampler_index < _countof(sampler_desc); ++sampler_index)
	{
		hr = device->CreateSamplerState(&sampler_desc[sampler_index], sampler_states[sampler_index].GetAddressOf());
		_ASSERT_EXPR(SUCCEEDED(hr), hr_trace(hr));
	}

	//depth setting
	D3D11_DEPTH_STENCIL_DESC depth_stencil_desc{};
	depth_stencil_desc.DepthEnable = TRUE;
	depth_stencil_desc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
	depth_stencil_desc.DepthFunc = D3D11_COMPARISON_LESS_EQUAL;
	hr = device->CreateDepthStencilState(&depth_stencil_desc, depth_stencil_states[static_cast<size_t>(DEPTH_STATE::ZT_ON_ZW_ON)].GetAddressOf());
	_ASSERT_EXPR(SUCCEEDED(hr), hr_trace(hr));
	depth_stencil_desc.DepthEnable = TRUE;
	depth_stencil_desc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ZERO;
	depth_stencil_desc.DepthFunc = D3D11_COMPARISON_LESS_EQUAL;
	hr = device->CreateDepthStencilState(&depth_stencil_desc, depth_stencil_states[static_cast<size_t>(DEPTH_STATE::ZT_ON_ZW_OFF)].GetAddressOf());
	_ASSERT_EXPR(SUCCEEDED(hr), hr_trace(hr));
	depth_stencil_desc.DepthEnable = FALSE;
	depth_stencil_desc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
	depth_stencil_desc.DepthFunc = D3D11_COMPARISON_LESS_EQUAL;
	hr = device->CreateDepthStencilState(&depth_stencil_desc, depth_stencil_states[static_cast<size_t>(DEPTH_STATE::ZT_OFF_ZW_ON)].GetAddressOf());
	_ASSERT_EXPR(SUCCEEDED(hr), hr_trace(hr));
	depth_stencil_desc.DepthEnable = FALSE;
	depth_stencil_desc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ZERO;
	depth_stencil_desc.DepthFunc = D3D11_COMPARISON_LESS_EQUAL;
	hr = device->CreateDepthStencilState(&depth_stencil_desc, depth_stencil_states[static_cast<size_t>(DEPTH_STATE::ZT_OFF_ZW_OFF)].GetAddressOf());
	_ASSERT_EXPR(SUCCEEDED(hr), hr_trace(hr));

	//blend
	{
	D3D11_BLEND_DESC blend_desc{};
	blend_desc.AlphaToCoverageEnable = FALSE;
	blend_desc.IndependentBlendEnable = FALSE;
	blend_desc.RenderTarget[0].BlendEnable = FALSE;
	blend_desc.RenderTarget[0].SrcBlend = D3D11_BLEND_ONE;
	blend_desc.RenderTarget[0].DestBlend = D3D11_BLEND_ZERO;
	blend_desc.RenderTarget[0].BlendOp = D3D11_BLEND_OP_ADD;
	blend_desc.RenderTarget[0].SrcBlendAlpha = D3D11_BLEND_ONE;
	blend_desc.RenderTarget[0].DestBlendAlpha = D3D11_BLEND_ZERO;
	blend_desc.RenderTarget[0].BlendOpAlpha = D3D11_BLEND_OP_ADD;
	blend_desc.RenderTarget[0].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;
	hr = device->CreateBlendState(&blend_desc, blend_states[static_cast<size_t>(BLEND_STATE::NONE)].GetAddressOf());
	_ASSERT_EXPR(SUCCEEDED(hr), hr_trace(hr));
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
	hr = device->CreateBlendState(&blend_desc, blend_states[static_cast<size_t>(BLEND_STATE::ALPHA)].GetAddressOf());
	_ASSERT_EXPR(SUCCEEDED(hr), hr_trace(hr));
	blend_desc.AlphaToCoverageEnable = FALSE;
	blend_desc.IndependentBlendEnable = FALSE;
	blend_desc.RenderTarget[0].BlendEnable = TRUE;
	blend_desc.RenderTarget[0].SrcBlend = D3D11_BLEND_SRC_ALPHA; //D3D11_BLEND_ONE D3D11_BLEND_SRC_ALPHA
	blend_desc.RenderTarget[0].DestBlend = D3D11_BLEND_ONE;
	blend_desc.RenderTarget[0].BlendOp = D3D11_BLEND_OP_ADD;
	blend_desc.RenderTarget[0].SrcBlendAlpha = D3D11_BLEND_ZERO;
	blend_desc.RenderTarget[0].DestBlendAlpha = D3D11_BLEND_ONE;
	blend_desc.RenderTarget[0].BlendOpAlpha = D3D11_BLEND_OP_ADD;
	blend_desc.RenderTarget[0].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;
	hr = device->CreateBlendState(&blend_desc, blend_states[static_cast<size_t>(BLEND_STATE::ADD)].GetAddressOf());
	_ASSERT_EXPR(SUCCEEDED(hr), hr_trace(hr));
	blend_desc.AlphaToCoverageEnable = FALSE;
	blend_desc.IndependentBlendEnable = FALSE;
	blend_desc.RenderTarget[0].BlendEnable = TRUE;
	blend_desc.RenderTarget[0].SrcBlend = D3D11_BLEND_ZERO; //D3D11_BLEND_DEST_COLOR
	blend_desc.RenderTarget[0].DestBlend = D3D11_BLEND_SRC_COLOR; //D3D11_BLEND_SRC_COLOR
	blend_desc.RenderTarget[0].BlendOp = D3D11_BLEND_OP_ADD;
	blend_desc.RenderTarget[0].SrcBlendAlpha = D3D11_BLEND_DEST_ALPHA;
	blend_desc.RenderTarget[0].DestBlendAlpha = D3D11_BLEND_ZERO;
	blend_desc.RenderTarget[0].BlendOpAlpha = D3D11_BLEND_OP_ADD;
	blend_desc.RenderTarget[0].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;
	hr = device->CreateBlendState(&blend_desc, blend_states[static_cast<size_t>(BLEND_STATE::MULTIPLY)].GetAddressOf());
	_ASSERT_EXPR(SUCCEEDED(hr), hr_trace(hr));
	blend_desc.AlphaToCoverageEnable = FALSE;
	blend_desc.IndependentBlendEnable = FALSE;
	blend_desc.RenderTarget[0].BlendEnable = TRUE;
	blend_desc.RenderTarget[0].SrcBlend = D3D11_BLEND_ONE; //D3D11_BLEND_DEST_COLOR
	blend_desc.RenderTarget[0].DestBlend = D3D11_BLEND_INV_SRC_ALPHA; //D3D11_BLEND_SRC_COLOR
	blend_desc.RenderTarget[0].BlendOp = D3D11_BLEND_OP_ADD;
	blend_desc.RenderTarget[0].SrcBlendAlpha = D3D11_BLEND_ONE;
	blend_desc.RenderTarget[0].DestBlendAlpha = D3D11_BLEND_ZERO;
	blend_desc.RenderTarget[0].BlendOpAlpha = D3D11_BLEND_OP_ADD;
	blend_desc.RenderTarget[0].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;
	hr = device->CreateBlendState(&blend_desc, blend_states[static_cast<size_t>(BLEND_STATE::TRANSMISSION)].GetAddressOf());
	_ASSERT_EXPR(SUCCEEDED(hr), hr_trace(hr));
    }

	//rasterizer
	{
		D3D11_RASTERIZER_DESC rasterizer_desc{};
		rasterizer_desc.FillMode = D3D11_FILL_SOLID;
		rasterizer_desc.CullMode = D3D11_CULL_BACK;
		rasterizer_desc.FrontCounterClockwise = TRUE;
		rasterizer_desc.DepthBias = 0;
		rasterizer_desc.DepthBiasClamp = 0;
		rasterizer_desc.SlopeScaledDepthBias = 0;
		rasterizer_desc.DepthClipEnable = TRUE;
		rasterizer_desc.ScissorEnable = FALSE;
		rasterizer_desc.MultisampleEnable = FALSE;
		rasterizer_desc.AntialiasedLineEnable = FALSE;
		hr = device->CreateRasterizerState(&rasterizer_desc, rasterizer_states[static_cast<size_t>(RASTER_STATE::SOLID)].GetAddressOf());
		_ASSERT_EXPR(SUCCEEDED(hr), hr_trace(hr));

		rasterizer_desc.FrontCounterClockwise = FALSE;
		hr = device->CreateRasterizerState(&rasterizer_desc, rasterizer_states[static_cast<size_t>(RASTER_STATE::SOLID_CW)].GetAddressOf());
		_ASSERT_EXPR(SUCCEEDED(hr), hr_trace(hr));

		rasterizer_desc.FillMode = D3D11_FILL_WIREFRAME;
		rasterizer_desc.CullMode = D3D11_CULL_BACK;
		rasterizer_desc.FrontCounterClockwise = TRUE;
		rasterizer_desc.AntialiasedLineEnable = TRUE;
		hr = device->CreateRasterizerState(&rasterizer_desc, rasterizer_states[static_cast<size_t>(RASTER_STATE::WIREFRAME)].GetAddressOf());
		_ASSERT_EXPR(SUCCEEDED(hr), hr_trace(hr));

		rasterizer_desc.FillMode = D3D11_FILL_SOLID;
		rasterizer_desc.CullMode = D3D11_CULL_NONE;
		rasterizer_desc.FrontCounterClockwise = TRUE;
		rasterizer_desc.AntialiasedLineEnable = TRUE;
		hr = device->CreateRasterizerState(&rasterizer_desc, rasterizer_states[static_cast<size_t>(RASTER_STATE::CULL_NONE)].GetAddressOf());
		_ASSERT_EXPR(SUCCEEDED(hr), hr_trace(hr));

		rasterizer_desc.FillMode = D3D11_FILL_WIREFRAME;
		rasterizer_desc.CullMode = D3D11_CULL_NONE;
		rasterizer_desc.FrontCounterClockwise = TRUE;
		rasterizer_desc.AntialiasedLineEnable = TRUE;
		hr = device->CreateRasterizerState(&rasterizer_desc, rasterizer_states[static_cast<size_t>(RASTER_STATE::WIREFRAME_CULL_NONE)].GetAddressOf());
		_ASSERT_EXPR(SUCCEEDED(hr), hr_trace(hr));
	}
	//constant buffer
	{
		D3D11_BUFFER_DESC buffer_desc{};
		buffer_desc.ByteWidth = sizeof(scene_constants);
		buffer_desc.Usage = D3D11_USAGE_DEFAULT;
		buffer_desc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
		buffer_desc.CPUAccessFlags = 0;
		buffer_desc.MiscFlags = 0;
		buffer_desc.StructureByteStride = 0;
		hr = device->CreateBuffer(&buffer_desc, nullptr, constant_buffers[0].GetAddressOf());
		_ASSERT_EXPR(SUCCEEDED(hr), hr_trace(hr));
	}
	{
		D3D11_TEXTURE2D_DESC td;
		ZeroMemory(&td, sizeof(D3D11_TEXTURE2D_DESC));
		td.Width = screen_width;
		td.Height = screen_height;
		td.MipLevels = 1;
		td.ArraySize = 1;
		td.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
		td.SampleDesc.Count = 1;
		td.SampleDesc.Quality = 0;
		td.Usage = D3D11_USAGE_DEFAULT;
		td.BindFlags = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE;
		td.CPUAccessFlags = 0;
		td.MiscFlags = 0;
		// create texture
		hr = device->CreateTexture2D(
			&td, NULL, tex_noise.GetAddressOf());
		if (FAILED(hr))
			_ASSERT_EXPR(SUCCEEDED(hr), hr_trace(hr));

		//	render target view
		D3D11_RENDER_TARGET_VIEW_DESC rtv_desc;
		memset(&rtv_desc, 0, sizeof(rtv_desc));
		rtv_desc.Format = td.Format;
		rtv_desc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2D;
		hr = device->CreateRenderTargetView(
			tex_noise.Get(), &rtv_desc, rtv_noise.GetAddressOf());

		//setting shader resource view
		D3D11_SHADER_RESOURCE_VIEW_DESC srv_desc;
		memset(&srv_desc, 0, sizeof(srv_desc));
		srv_desc.Format = rtv_desc.Format;
		srv_desc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
		srv_desc.Texture2D.MipLevels = 1;
		// create shader resource view
		hr = device->CreateShaderResourceView(
			tex_noise.Get(), &srv_desc, srv_noise.GetAddressOf());
		if (FAILED(hr)) 	
			_ASSERT_EXPR(SUCCEEDED(hr), hr_trace(hr));

	}
	{
		// depth stencil
		{
			// setting depth stencil 
			D3D11_TEXTURE2D_DESC td;
			ZeroMemory(&td, sizeof(D3D11_TEXTURE2D_DESC));
			td.Width = screenW;
			td.Height = screenH;
			td.MipLevels = 1;
			td.ArraySize = 1;
			td.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
			td.SampleDesc.Count = 1;
			td.SampleDesc.Quality = 0;
			td.Usage = D3D11_USAGE_DEFAULT;
			td.BindFlags = D3D11_BIND_DEPTH_STENCIL;
			td.CPUAccessFlags = 0;
			td.MiscFlags = 0;

			// create depth stencil texture
			hr = device->CreateTexture2D(&td, NULL, depth.GetAddressOf());
			if (FAILED(hr)) 
				_ASSERT_EXPR(SUCCEEDED(hr), hr_trace(hr));


			// setting depth stencil view
			D3D11_DEPTH_STENCIL_VIEW_DESC dsvd;
			ZeroMemory(&dsvd, sizeof(D3D11_DEPTH_STENCIL_VIEW_DESC));
			dsvd.Format = td.Format;
			dsvd.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
			dsvd.Texture2D.MipSlice = 0;

			// create depth stencil view
			hr = device->CreateDepthStencilView(depth.Get(), &dsvd, depth_stencil_view.GetAddressOf());
			if (FAILED(hr)) 
				_ASSERT_EXPR(SUCCEEDED(hr), hr_trace(hr));

		}
	}
	terrain = std::make_unique<class terrain>(device);
	noise = std::make_unique<noise_map>(device, "resources//sprite//noise//zatsu.png", true, true);
	return true;
}

bool wireframe = false;
const char* create_terrain_scene::update(float& elapsed_time/*Elapsed seconds from last frame*/)
{
	// Control the camera with the mouse
	static POINT prev_cursorpos;
	POINT curr_cursorpos;
	GetCursorPos(&curr_cursorpos);
	if (GetKeyState(VK_RBUTTON) < 0)
	{
		float dx = static_cast<float>(curr_cursorpos.x - prev_cursorpos.x);
		float dy = static_cast<float>(curr_cursorpos.y - prev_cursorpos.y);
		if (GetKeyState(VK_SHIFT) < 0)
		{
			const float sensitivity = 10.0f;
			scene_data.focus_position.x -= sensitivity * dx * elapsed_time;
			scene_data.focus_position.y += sensitivity * dy * elapsed_time;
		}
		else if (GetKeyState(VK_CONTROL) < 0)
		{
			const float sensitivity = 50.0f;
			camera_manipulator.radius += sensitivity * dy * elapsed_time;
		}
		else
		{
			const float sensitivity = 1.0f;
			camera_manipulator.phi += sensitivity * dx * elapsed_time;
			camera_manipulator.theta += sensitivity * dy * elapsed_time;
		}
	}
	prev_cursorpos = curr_cursorpos;

	ImGui::Begin("ImGUI");
	ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);

	ImGui::SliderFloat("light_direction.x", &scene_data.light_direction.x, -1.0f, +1.0f);
	ImGui::SliderFloat("light_direction.y", &scene_data.light_direction.y, -1.0f, +1.0f);
	ImGui::SliderFloat("light_direction.z", &scene_data.light_direction.z, -1.0f, +1.0f);

	ImGui::SliderFloat("vertical_scale", &terrain->vertical_scale, 0.0f, 50.0f, "%.6f");
	ImGui::SliderFloat("tesselation_max_subdivision", &terrain->tesselation_max_subdivision, 0.0f, 64.0f, "%.6f");
	ImGui::Checkbox("wireframe", &wireframe);
	ImGui::End();


	GamePad& gamePad = Input::Instance().GetGamePad();

	if (gamePad.GetButton() & GamePad::BTN_B)
		return "title";

	return 0;
}

void create_terrain_scene::render(ID3D11DeviceContext* immediate_context, float elapsed_time)
{


	for (size_t sampler_index = 0; sampler_index < _countof(sampler_states); ++sampler_index)
	{
		immediate_context->PSSetSamplers(static_cast<UINT>(sampler_index), 1, sampler_states[sampler_index].GetAddressOf());
	}
	immediate_context->OMSetBlendState(blend_states[static_cast<size_t>(BLEND_STATE::ALPHA)].Get(), nullptr, 0xFFFFFFFF);
	immediate_context->OMSetDepthStencilState(depth_stencil_states[static_cast<size_t>(DEPTH_STATE::ZT_ON_ZW_ON)].Get(), 0);
	immediate_context->RSSetState(rasterizer_states[static_cast<size_t>(RASTER_STATE::SOLID)].Get());

	D3D11_VIEWPORT viewport;
	UINT num_viewports{ 1 };
	immediate_context->RSGetViewports(&num_viewports, &viewport);

	float aspect_ratio{ viewport.Width / viewport.Height };
    DirectX::XMMATRIX P{DirectX::XMMatrixPerspectiveFovLH(DirectX::XMConvertToRadians(50), aspect_ratio, 0.1f, 1000.0f) };

	DirectX::XMVECTOR add = DirectX::XMVectorAdd(XMLoadFloat4(&scene_data.focus_position) , XMLoadFloat4(&camera_manipulator.to_cartesian()));
	XMStoreFloat4(&scene_data.eye_position, add);
    DirectX::XMMATRIX V{DirectX::XMMatrixLookAtLH(XMLoadFloat4(&scene_data.eye_position), XMLoadFloat4(&scene_data.focus_position), DirectX::XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f)) };
	XMStoreFloat4x4(&scene_data.view_projection, V * P);
	XMStoreFloat4x4(&scene_data.inv_view_projection, XMMatrixInverse(NULL, V * P));

	immediate_context->OMGetRenderTargets(1, default_render_target_view.ReleaseAndGetAddressOf(), default_depth_stencil_view.ReleaseAndGetAddressOf());

	immediate_context->OMSetRenderTargets(
		1, rtv_noise.GetAddressOf(), depth_stencil_view.Get());
	float clear_color[4] = { 0, 0, 0, 1 };
	immediate_context->ClearRenderTargetView(
		rtv_noise.Get(), clear_color);
	//noise rendering
	noise->render(immediate_context, 0, 0, screenW, screenH, 1, 1, 1, 1, 0);

	immediate_context->OMSetRenderTargets(1, default_render_target_view.GetAddressOf(), default_depth_stencil_view.Get());

	immediate_context->UpdateSubresource(constant_buffers[0].Get(), 0, 0, &scene_data, 0, 0);
	immediate_context->VSSetConstantBuffers(0, 1, constant_buffers[0].GetAddressOf());
	//immediate_context->HSSetConstantBuffers(0, 1, constant_buffers[0].GetAddressOf());
	immediate_context->GSSetConstantBuffers(0, 1, constant_buffers[0].GetAddressOf());
	immediate_context->PSSetConstantBuffers(0, 1, constant_buffers[0].GetAddressOf());

	immediate_context->PSSetShaderResources(2, 1, srv_noise.GetAddressOf());
    immediate_context->DSSetShaderResources(2, 1, srv_noise.GetAddressOf());

    DirectX::XMFLOAT4X4 view;
	XMStoreFloat4x4(&view, V);
    DirectX::XMFLOAT4X4 projection;
	XMStoreFloat4x4(&projection, P);
	if (wireframe)
		immediate_context->RSSetState(rasterizer_states[static_cast<size_t>(RASTER_STATE::WIREFRAME)].Get());
	else
		immediate_context->RSSetState(rasterizer_states[static_cast<size_t>(RASTER_STATE::SOLID)].Get());
	terrain->draw(immediate_context);


	//IMGUI
	noise->DebugDrawGUI();
	terrain->DebugDrawGUI();
	if (ImGui::Begin("noise", nullptr, ImGuiWindowFlags_None))
	{
		if (ImGui::CollapsingHeader("f_noise", ImGuiTreeNodeFlags_DefaultOpen))
		{
			//fractal
			ImGui::Image(srv_noise.Get(), { 320,180 });

		}
	}
	ImGui::End();

	ImGui::Render();
	ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());

}

