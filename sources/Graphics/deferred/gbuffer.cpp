// Gbuffer.cpp
#include "gbuffer.h"
#include "../load/texture.h"
#include "../imgui/imgui.h"

#define screenW 1280
#define screenH 720




void gbuffer::initialize(ID3D11Device* device, DirectX::XMFLOAT3 dir)
{
	DirectX::XMFLOAT3 dir_color{ 1.0f,1.0f,1.0f };
	dir_light = std::make_shared<directional_light>(device, dir, dir_color);
	light_manager::instance().initialize(device);
	light_manager::instance().register_light("dir light", dir_light);

	//blend state
	blend_states[NONE] = std::make_unique<Descartes::blend_state>(device, BLEND_MODE::NONE);
	blend_states[ALPHA] = std::make_unique<Descartes::blend_state>(device, BLEND_MODE::ALPHA);
	blend_states[ADD] = std::make_unique<Descartes::blend_state>(device, BLEND_MODE::ADD);
	blend_states[ALPHA_TO_COVERAGE] = std::make_unique<Descartes::blend_state>(device, BLEND_MODE::ALPHA_TO_COVERAGE);


	HRESULT hr{ S_OK };
	{
		vertex v[4] = {
		{  1, 1,0, 0,0,1, 1,0 },
		{ -1, 1,0, 0,0,1, 0,0 },
		{  1,-1,0, 0,0,1, 1,1 },
		{ -1,-1,0, 0,0,1, 0,1 }
		};
		// create vertex buffer
		D3D11_BUFFER_DESC bd;
		ZeroMemory(&bd, sizeof(D3D11_BUFFER_DESC));
		bd.Usage = D3D11_USAGE_DEFAULT;
		bd.ByteWidth = sizeof(vertex) * 4;
		bd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
		bd.CPUAccessFlags = 0;
		// setting sub resource
		D3D11_SUBRESOURCE_DATA initData;
		ZeroMemory(&initData, sizeof(D3D11_SUBRESOURCE_DATA));
		initData.pSysMem = v;
		hr = device->CreateBuffer(&bd, &initData, vertex_buffer.GetAddressOf());
	    _ASSERT_EXPR(SUCCEEDED(hr), hr_trace(hr));
  
		//	index
		int index[4] = { 0,1,2,3 };
		ZeroMemory(&bd, sizeof(D3D11_BUFFER_DESC));
		bd.Usage = D3D11_USAGE_DEFAULT;
		bd.ByteWidth = sizeof(int) * 4;
		bd.BindFlags = D3D11_BIND_INDEX_BUFFER;
		bd.CPUAccessFlags = 0;
		// setting sub resource
		ZeroMemory(&initData, sizeof(D3D11_SUBRESOURCE_DATA));
		initData.pSysMem = index;
		hr = device->CreateBuffer(&bd, &initData, index_buffer.GetAddressOf());
		_ASSERT_EXPR(SUCCEEDED(hr), hr_trace(hr));
	}

	//input layout
	D3D11_INPUT_ELEMENT_DESC input_element_desc[]
	{
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 4 * 3, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 4 * 6, D3D11_INPUT_PER_VERTEX_DATA, 0 },
	};

	//shader create
	create_vs_from_cso(device, "shader//differred_light_vs.cso", differred_light_vs.GetAddressOf(), input_layout.GetAddressOf(), input_element_desc, _countof(input_element_desc));
	create_ps_from_cso(device, "shader//environment_light_ps.cso", env_light_ps.GetAddressOf());

	//create_ps_from_cso(device, "shader//composite_ps.cso", composite_ps.GetAddressOf());
	composite_ps = std::make_unique<pixel_shader>(device, "shader//composite_ps.cso");

	create_vs_from_cso(device, "shader//d_posteffect_vs.cso", post_effect_vs.GetAddressOf(), input_layout_1.GetAddressOf(), input_element_desc, _countof(input_element_desc));
	create_ps_from_cso(device, "shader//d_posteffect_ps.cso", post_effect_ps.GetAddressOf());

	//color buffer
	{
	D3D11_TEXTURE2D_DESC td;
	ZeroMemory(&td, sizeof(D3D11_TEXTURE2D_DESC));
	td.Width = screenW;
	td.Height = screenH;
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
		&td, NULL, tex_color.GetAddressOf());
	if (FAILED(hr)) return;

	//	render target view
	D3D11_RENDER_TARGET_VIEW_DESC rtvDesc;
	memset(&rtvDesc, 0, sizeof(rtvDesc));
	rtvDesc.Format = td.Format;
	rtvDesc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2D;
	hr = device->CreateRenderTargetView(
		tex_color.Get(), &rtvDesc, rtv_color.GetAddressOf());
    
	//setting shader resource view
	D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc;
	memset(&srvDesc, 0, sizeof(srvDesc));
	srvDesc.Format = rtvDesc.Format;
	srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
	srvDesc.Texture2D.MipLevels = 1;
	// create shader resource view
	hr = device->CreateShaderResourceView(
		tex_color.Get(), &srvDesc, srv_color.GetAddressOf());
	if (FAILED(hr)) return;
    }

	//depth buffer
	{
		D3D11_TEXTURE2D_DESC td;
		ZeroMemory(&td, sizeof(D3D11_TEXTURE2D_DESC));
		td.Width = screenW;
		td.Height = screenH;
		td.MipLevels = 1;
		td.ArraySize = 1;
		//td.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
		//赤のみの32bit floatテクスチャ
		td.Format = DXGI_FORMAT_R32_FLOAT;

		td.SampleDesc.Count = 1;
		td.SampleDesc.Quality = 0;
		td.Usage = D3D11_USAGE_DEFAULT;
		td.BindFlags = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE;
		td.CPUAccessFlags = 0;
		td.MiscFlags = 0;
		// create texture
		hr = device->CreateTexture2D(
			&td, NULL, tex_depth.GetAddressOf());
		if (FAILED(hr)) return;

		//render target 
		D3D11_RENDER_TARGET_VIEW_DESC rtv_desc;
		memset(&rtv_desc, 0, sizeof(rtv_desc));
		rtv_desc.Format = td.Format;
		rtv_desc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2D;
		hr = device->CreateRenderTargetView(
			tex_depth.Get(), &rtv_desc, rtv_depth.GetAddressOf());

		// setting shader resource
		D3D11_SHADER_RESOURCE_VIEW_DESC srv_desc;
		memset(&srv_desc, 0, sizeof(srv_desc));
		srv_desc.Format = rtv_desc.Format;
		srv_desc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
		srv_desc.Texture2D.MipLevels = 1;
		// create  shader resource
		hr = device->CreateShaderResourceView(
			tex_depth.Get(), &srv_desc, srv_depth.GetAddressOf());
		if (FAILED(hr)) return;
	}

	//light buffer
	{
		D3D11_TEXTURE2D_DESC td;
		ZeroMemory(&td, sizeof(D3D11_TEXTURE2D_DESC));
		td.Width = screenW;
		td.Height = screenH;
		td.MipLevels = 1;
		td.ArraySize = 1;
		td.Format = DXGI_FORMAT_R16G16B16A16_FLOAT;
		td.SampleDesc.Count = 1;
		td.SampleDesc.Quality = 0;
		td.Usage = D3D11_USAGE_DEFAULT;
		td.BindFlags = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE;
		td.CPUAccessFlags = 0;
		td.MiscFlags = 0;
		// create texture
		hr = device->CreateTexture2D(
			&td, NULL, tex_light.GetAddressOf());
		if (FAILED(hr)) return;

		//	render target view
		D3D11_RENDER_TARGET_VIEW_DESC rtvDesc;
		memset(&rtvDesc, 0, sizeof(rtvDesc));
		rtvDesc.Format = td.Format;
		rtvDesc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2D;
		hr = device->CreateRenderTargetView(
			tex_light.Get(), &rtvDesc, rtv_light.GetAddressOf());

		//setting shader resource view
		D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc;
		memset(&srvDesc, 0, sizeof(srvDesc));
		srvDesc.Format = rtvDesc.Format;
		srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
		srvDesc.Texture2D.MipLevels = 1;
		// create shader resource view
		hr = device->CreateShaderResourceView(
			tex_light.Get(), &srvDesc, srv_light.GetAddressOf());
		if (FAILED(hr)) return;
	}

	// normal buffer
	{
		D3D11_TEXTURE2D_DESC td;
		ZeroMemory(&td, sizeof(D3D11_TEXTURE2D_DESC));
		td.Width = screenW;
		td.Height = screenH;
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
			&td, NULL, tex_normal.GetAddressOf());
		if (FAILED(hr)) return;

		//	render target view
		D3D11_RENDER_TARGET_VIEW_DESC rtvDesc;
		memset(&rtvDesc, 0, sizeof(rtvDesc));
		rtvDesc.Format = td.Format;
		rtvDesc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2D;
		hr = device->CreateRenderTargetView(
			tex_normal.Get(), &rtvDesc, rtv_normal.GetAddressOf());

		// setting shader resource view
		D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc;
		memset(&srvDesc, 0, sizeof(srvDesc));
		srvDesc.Format = rtvDesc.Format;
		srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
		srvDesc.Texture2D.MipLevels = 1;
		// create  shader resource view
		hr = device->CreateShaderResourceView(
			tex_normal.Get(), &srvDesc, srv_normal.GetAddressOf());
		if (FAILED(hr)) return;
	}

	// position buffer
	{
		D3D11_TEXTURE2D_DESC td;
		ZeroMemory(&td, sizeof(D3D11_TEXTURE2D_DESC));
		td.Width = screenW;
		td.Height = screenH;
		td.MipLevels = 1;
		td.ArraySize = 1;
		td.Format = DXGI_FORMAT_R16G16B16A16_FLOAT;
		td.SampleDesc.Count = 1;
		td.SampleDesc.Quality = 0;
		td.Usage = D3D11_USAGE_DEFAULT;
		td.BindFlags = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE;
		td.CPUAccessFlags = 0;
		td.MiscFlags = 0;
		// create texture
		hr = device->CreateTexture2D(
			&td, NULL, tex_position.GetAddressOf());
		if (FAILED(hr)) return;

		//render target view
		D3D11_RENDER_TARGET_VIEW_DESC rtvDesc;
		memset(&rtvDesc, 0, sizeof(rtvDesc));
		rtvDesc.Format = td.Format;
		rtvDesc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2D;
		hr = device->CreateRenderTargetView(
			tex_position.Get(), &rtvDesc, rtv_position.GetAddressOf());

		// setting shader resource view
		D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc;
		memset(&srvDesc, 0, sizeof(srvDesc));
		srvDesc.Format = rtvDesc.Format;
		srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
		srvDesc.Texture2D.MipLevels = 1;
		// create shader resource view
		hr = device->CreateShaderResourceView(
			tex_position.Get(), &srvDesc, srv_position.GetAddressOf());
		if (FAILED(hr)) return;
	}


	// roughness+metallic buffer
	{
		D3D11_TEXTURE2D_DESC td;
		ZeroMemory(&td, sizeof(D3D11_TEXTURE2D_DESC));
		td.Width = screenW;
		td.Height = screenH;
		td.MipLevels = 1;
		td.ArraySize = 1;
		td.Format = DXGI_FORMAT_R16G16B16A16_UNORM;
		td.SampleDesc.Count = 1;
		td.SampleDesc.Quality = 0;
		td.Usage = D3D11_USAGE_DEFAULT;
		td.BindFlags = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE;
		td.CPUAccessFlags = 0;
		td.MiscFlags = 0;
		// create texture
		hr = device->CreateTexture2D(
			&td, NULL, tex_RM.GetAddressOf());
		if (FAILED(hr)) return;

		//	render target view
		D3D11_RENDER_TARGET_VIEW_DESC rtvDesc;
		memset(&rtvDesc, 0, sizeof(rtvDesc));
		rtvDesc.Format = td.Format;
		rtvDesc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2D;
		hr = device->CreateRenderTargetView(
			tex_RM.Get(), &rtvDesc, rtv_RM.GetAddressOf());

		// setting shader resource view
		D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc;
		memset(&srvDesc, 0, sizeof(srvDesc));
		srvDesc.Format = rtvDesc.Format;
		srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
		srvDesc.Texture2D.MipLevels = 1;
		// create shader resource view
		hr = device->CreateShaderResourceView(
			tex_RM.Get(), &srvDesc, srv_RM.GetAddressOf());
		if (FAILED(hr)) return;
	}

	
	//shadow param buffer
	{
		D3D11_TEXTURE2D_DESC td;
		ZeroMemory(&td, sizeof(D3D11_TEXTURE2D_DESC));
		td.Width = screenW;
		td.Height = screenH;
		td.MipLevels = 1;
		td.ArraySize = 1;
		//td.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
		//赤のみの32bit floatテクスチャ
		td.Format = DXGI_FORMAT_R32_FLOAT;

		td.SampleDesc.Count = 1;
		td.SampleDesc.Quality = 0;
		td.Usage = D3D11_USAGE_DEFAULT;
		td.BindFlags = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE;
		td.CPUAccessFlags = 0;
		td.MiscFlags = 0;
		// create texture
		hr = device->CreateTexture2D(
			&td, NULL, tex_shadow_param.GetAddressOf());
		if (FAILED(hr)) return;

		//render target 
		D3D11_RENDER_TARGET_VIEW_DESC rtv_desc;
		memset(&rtv_desc, 0, sizeof(rtv_desc));
		rtv_desc.Format = td.Format;
		rtv_desc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2D;
		hr = device->CreateRenderTargetView(
			tex_shadow_param.Get(), &rtv_desc, rtv_shadow_param.GetAddressOf());

		// setting shader resource
		D3D11_SHADER_RESOURCE_VIEW_DESC srv_desc;
		memset(&srv_desc, 0, sizeof(srv_desc));
		srv_desc.Format = rtv_desc.Format;
		srv_desc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
		srv_desc.Texture2D.MipLevels = 1;
		// create  shader resource
		hr = device->CreateShaderResourceView(
			tex_shadow_param.Get(), &srv_desc, srv_shadow_param.GetAddressOf());
		if (FAILED(hr)) return;
	}


	//deferred
	//composite buffer
	{
		D3D11_TEXTURE2D_DESC td;
		ZeroMemory(&td, sizeof(D3D11_TEXTURE2D_DESC));
		td.Width = screenW;
		td.Height = screenH;
		td.MipLevels = 1;
		td.ArraySize = 1;
		td.Format = DXGI_FORMAT_R16G16B16A16_FLOAT;
		td.SampleDesc.Count = 1;
		td.SampleDesc.Quality = 0;
		td.Usage = D3D11_USAGE_DEFAULT;
		td.BindFlags = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE;
		td.CPUAccessFlags = 0;
		td.MiscFlags = 0;
		// create texture
		hr = device->CreateTexture2D(
			&td, NULL, tex_composite.GetAddressOf());
		if (FAILED(hr)) return;

		//	render target view
		D3D11_RENDER_TARGET_VIEW_DESC rtvDesc;
		memset(&rtvDesc, 0, sizeof(rtvDesc));
		rtvDesc.Format = td.Format;
		rtvDesc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2D;
		hr = device->CreateRenderTargetView(
			tex_composite.Get(), &rtvDesc, rtv_composite.GetAddressOf());

		// setting shader resource view
		D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc;
		memset(&srvDesc, 0, sizeof(srvDesc));
		srvDesc.Format = rtvDesc.Format;
		srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
		srvDesc.Texture2D.MipLevels = 1;
		// create shader resource view
		hr = device->CreateShaderResourceView(
			tex_composite.Get(), &srvDesc, srv_composite.GetAddressOf());
		if (FAILED(hr)) return;
	}

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
		if (FAILED(hr)) return;

		// setting depth stencil view
		D3D11_DEPTH_STENCIL_VIEW_DESC dsvd;
		ZeroMemory(&dsvd, sizeof(D3D11_DEPTH_STENCIL_VIEW_DESC));
		dsvd.Format = td.Format;
		dsvd.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
		dsvd.Texture2D.MipSlice = 0;

		// create depth stencil view
		hr = device->CreateDepthStencilView(depth.Get(), &dsvd, depth_stencil_view.GetAddressOf());
		if (FAILED(hr)) return;
	}

	{
		D3D11_RASTERIZER_DESC rasterizer_desc = {};
		rasterizer_desc.FillMode = D3D11_FILL_SOLID;
		rasterizer_desc.CullMode = D3D11_CULL_NONE;
		rasterizer_desc.FrontCounterClockwise = FALSE;
		rasterizer_desc.DepthBias = 0;
		rasterizer_desc.DepthBiasClamp = 0;
		rasterizer_desc.SlopeScaledDepthBias = 0;
		rasterizer_desc.DepthClipEnable = FALSE;
		rasterizer_desc.ScissorEnable = FALSE;
		rasterizer_desc.MultisampleEnable = FALSE;
		rasterizer_desc.AntialiasedLineEnable = FALSE;
		hr = device->CreateRasterizerState(&rasterizer_desc, rasterizer_state.GetAddressOf());
		_ASSERT_EXPR(SUCCEEDED(hr), hr_trace(hr));

		D3D11_DEPTH_STENCIL_DESC depth_stencil_desc;
		depth_stencil_desc.DepthEnable = FALSE;
		depth_stencil_desc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ZERO;
		depth_stencil_desc.DepthFunc = D3D11_COMPARISON_ALWAYS;
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
		hr = device->CreateDepthStencilState(&depth_stencil_desc, depth_stencil_state.GetAddressOf());
		_ASSERT_EXPR(SUCCEEDED(hr), hr_trace(hr));
	}
	//constants buffer
	{
		d_posteffect_buffer = std::make_unique<Descartes::constant_buffer<d_posteffect_param>>(device);
	}


	load_texture_from_file(device, ".\\resources\\skymaps\\sky.png", env_shader_resource_view.GetAddressOf(), true, true);

	load_texture_from_file(device, "./environments/sunset_jhbcentral_4k/diffuse_iem.dds", diffuse_shader_resource_view.GetAddressOf(), true, true);
	load_texture_from_file(device, "./environments/sunset_jhbcentral_4k/specular_pmrem.dds", specular_shader_resource_view.GetAddressOf(), true, true);
	load_texture_from_file(device, "./environments/lut_ggx.dds", lut_ggx_shader_resource_view.GetAddressOf(), true, true);
}

void gbuffer::active(ID3D11DeviceContext* immediate_context)
{
	immediate_context->OMGetRenderTargets(1, default_render_target_view.ReleaseAndGetAddressOf(), default_depth_stencil_view.ReleaseAndGetAddressOf());

	// MRT(MultipleRenderTargets)
	ID3D11RenderTargetView* targets[] = {
		rtv_color.Get(),   //Target0
		rtv_depth.Get(),   //Target1
		rtv_normal.Get(),  //Target2
		rtv_position.Get(), //Target3
		rtv_RM.Get(),		//Target4
		rtv_shadow_param.Get()		//Target5
	};
	// setting render target view
	immediate_context->OMSetRenderTargets(
		5, targets, depth_stencil_view.Get());
	// R+M
	float clearRM[4] = { 0.8f, 0.1f, 0, 0 };
	immediate_context->ClearRenderTargetView(rtv_RM.Get(), clearRM);

	float clear_color[4] = { 0.5f, 0.5f, 0.5f, 1.0f };
	immediate_context->ClearRenderTargetView(rtv_color.Get(), clear_color);

	float clear_normal[4] = { 0, 0, 0, 0 };
	immediate_context->ClearRenderTargetView(
		rtv_normal.Get(), clear_normal);
	float clear_position[4] = { 0, 0, 0, 0 };
	immediate_context->ClearRenderTargetView(
		rtv_position.Get(), clear_position);

	float clear_depth[4] = { 5000, 1.0f, 1.0f, 1.0f };
	immediate_context->ClearRenderTargetView(
		rtv_depth.Get(), clear_depth);

	float clear_shadow_param[4] = { 0, 0, 0, 0 };
	immediate_context->ClearRenderTargetView(
		rtv_shadow_param.Get(), clear_shadow_param);

	immediate_context->ClearDepthStencilView(depth_stencil_view.Get(), D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);

	// setting view port
	D3D11_VIEWPORT vp = {
		0,0,
        static_cast<float>(screenW),
		static_cast<float>(screenH),
		0,1 };
	immediate_context->RSSetViewports(1, &vp);
}

void gbuffer::lightning(ID3D11DeviceContext* immediate_context)
{
	//render target to light buffer
	immediate_context->OMSetRenderTargets(
		1, rtv_light.GetAddressOf(), depth_stencil_view.Get());
	float clear_color[4] = { 0, 0, 0, 1 };
	immediate_context->ClearRenderTargetView(
		rtv_light.Get(), clear_color);

	//render from buffers
	immediate_context->PSSetShaderResources(
		0, 1, srv_color.GetAddressOf());
	immediate_context->PSSetShaderResources(
		1, 1, srv_normal.GetAddressOf());
	immediate_context->PSSetShaderResources(
		2, 1, srv_position.GetAddressOf());
	// Slot3 R+M
	immediate_context->PSSetShaderResources(
		3, 1, srv_RM.GetAddressOf());
	immediate_context->PSSetShaderResources(
		4, 1, srv_depth.GetAddressOf());
	immediate_context->PSSetShaderResources(15, 1, srv_color.GetAddressOf());

	ID3D11ShaderResourceView* g_buffers[]
	{
		srv_color.Get(),
		srv_normal.Get(),
		srv_position.Get(),
		srv_RM.Get(),
		srv_depth.Get()
	};
	UINT G_BUFFERS_NUM = ARRAYSIZE(g_buffers);

	//blend state addition and composition
	blend_states[ADD]->active(immediate_context);

	
	UINT stride = sizeof(vertex);
	UINT offset = 0;
	immediate_context->IASetVertexBuffers(0, 1,vertex_buffer.GetAddressOf(), &stride, &offset);
	immediate_context ->IASetIndexBuffer(index_buffer.Get(), DXGI_FORMAT_R32_UINT, 0);
	immediate_context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);


	immediate_context->IASetInputLayout(input_layout.Get());


	immediate_context->RSSetState(rasterizer_state.Get());
	immediate_context->OMSetDepthStencilState(depth_stencil_state.Get(), 1);

	immediate_context->VSSetShader(differred_light_vs.Get(), nullptr, 0);
	// ambient light
	immediate_context->PSSetShader(env_light_ps.Get(), nullptr, 0);
	immediate_context->DrawIndexed(4, 0, 0);
	// shadow map applying dir light
	/*immediate_context->PSSetShader(dir_light_shadow_ps.Get(), nullptr, 0);
	light_set(immediate_context,0);
	    immediate_context->DrawIndexed(4, 0, 0);*/


	//direction light and point light draw
	light_manager::instance().draw(immediate_context,g_buffers, G_BUFFERS_NUM);

	immediate_context->ClearRenderTargetView(rtv_composite.Get(), clear_color);
	immediate_context->OMSetRenderTargets(
		1, rtv_composite.GetAddressOf(), depth_stencil_view.Get());


	//blend state [ALPHA]
	blend_states[ADD]->inactive(immediate_context);
	blend_states[ALPHA]->active(immediate_context);

	immediate_context->VSSetShader(differred_light_vs.Get(), nullptr, 0);
	immediate_context->PSSetShaderResources(5, 1, srv_light.GetAddressOf());

	immediate_context->PSSetShaderResources(33, 1, diffuse_shader_resource_view.GetAddressOf());
	immediate_context->PSSetShaderResources(34, 1, specular_shader_resource_view.GetAddressOf());
	immediate_context->PSSetShaderResources(35, 1, lut_ggx_shader_resource_view.GetAddressOf());
	immediate_context->IASetInputLayout(input_layout.Get());

	composite_ps->active(immediate_context);
    immediate_context->DrawIndexed(4, 0, 0);
	composite_ps->inactive(immediate_context);
}

void gbuffer::inactive(ID3D11DeviceContext* immediate_context) {

	lightning(immediate_context);
	//back default render target
	immediate_context->OMSetRenderTargets(1, default_render_target_view.GetAddressOf(), default_depth_stencil_view.Get());

}

void gbuffer::render(ID3D11DeviceContext* immediate_context)
{
	//constants buffer
	d_posteffect_buffer->active(immediate_context, 9);

	//post effect
	immediate_context->IASetInputLayout(input_layout_1.Get());
	immediate_context->VSSetShader(post_effect_vs.Get(), nullptr, 0);
	immediate_context->PSSetShader(post_effect_ps.Get(), nullptr, 0);

	immediate_context->PSSetShaderResources(0, 1, srv_composite.GetAddressOf());
	immediate_context->PSSetShaderResources(1, 1, srv_depth.GetAddressOf());
	
	immediate_context->PSSetShaderResources(2, 1, srv_normal.GetAddressOf());
	
	UINT stride = sizeof(vertex);
	UINT offset = 0;
	immediate_context->IASetVertexBuffers(0, 1,vertex_buffer.GetAddressOf(), &stride, &offset);
	immediate_context->IASetIndexBuffer(index_buffer.Get(), DXGI_FORMAT_R32_UINT, 0);
	immediate_context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);

	immediate_context->DrawIndexed(4, 0, 0);
	immediate_context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
}


void gbuffer::DebugDrawGUI(bool flag)
{
	if (flag) {
		ImGui::SetNextWindowPos(ImVec2(10, 10), ImGuiCond_FirstUseEver);
		ImGui::SetNextWindowSize(ImVec2(300, 300), ImGuiCond_FirstUseEver);

		if (ImGui::Begin("g_buffer", nullptr, ImGuiWindowFlags_None))
		{
			if (ImGui::CollapsingHeader("texture", ImGuiTreeNodeFlags_DefaultOpen))
			{
				ImGui::Text("composite");
				ImGui::Image(srv_composite.Get(), { 320,180 });
				ImGui::Text("light");
				ImGui::Image(srv_light.Get(), { 320,180 });
				ImGui::Text("color");
				ImGui::Image(srv_color.Get(), { 320,180 });

				ImGui::Text("depth");
				ImGui::Image(srv_depth.Get(), { 320,180 });
				ImGui::Text("normal");
				ImGui::Image(srv_normal.Get(), { 320,180 });
				ImGui::Text("position");
				ImGui::Image(srv_position.Get(), { 320,180 });
				ImGui::Text("roughness metallic");
				ImGui::Image(srv_RM.Get(), { 320,180 });
				//ImGui::Text("shadow_param");
				//ImGui::Image(srv_shadow_param.Get(), { 320,180 });
			}
		}
		ImGui::End();
	}
}

void gbuffer::ef_DebugDrawGUI(bool flag)
{
	if (flag) {
		ImGui::SetNextWindowPos(ImVec2(10, 10), ImGuiCond_FirstUseEver);
		ImGui::SetNextWindowSize(ImVec2(300, 300), ImGuiCond_FirstUseEver);

		if (ImGui::Begin("post effect", nullptr, ImGuiWindowFlags_None))
		{
			if (ImGui::CollapsingHeader("posteffect configuration", ImGuiTreeNodeFlags_DefaultOpen))
			{
				ImGui::SliderFloat("blur", &d_posteffect_buffer->data.bloom, 0.0f, 5.0f);
				ImGui::SliderFloat("contrast", &d_posteffect_buffer->data.contrast, 0.0f, 1.0f);
				ImGui::SliderFloat("vignette", &d_posteffect_buffer->data.vignette, 0.0f, 1.0f);
				ImGui::SliderFloat("saturate", &d_posteffect_buffer->data.saturate, 0.0f, 1.0f);
				ImGui::SliderFloat("outline", &d_posteffect_buffer->data.outline, 0.0f, 5.0f);
				ImGui::SliderFloat("color_grading", &d_posteffect_buffer->data.color_grading, 0.0f, 1.0f);
			}
		}
		ImGui::End();
	}
}