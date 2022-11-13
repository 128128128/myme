//ShadowMap.cpp
#include "shadow_map_df.h"
#include "../../Functions/misc.h"
#include "../imgui/imgui.h"


shadow_map::shadow_map()
{
}

void shadow_map::initialize(ID3D11Device* device) {

	render_target.initialize(device);

	HRESULT hr{ S_OK };

	vertex vertices[]
	{
		{ { -1.0, +1.0, 0 }, { 1, 1, 1 }, { 0, 0 } },
		{ { +1.0, +1.0, 0 }, { 1, 1, 1 }, { 1, 0 } },
		{ { -1.0, -1.0, 0 }, { 1, 1, 1 }, { 0, 1 } },
		{ { +1.0, -1.0, 0 }, { 1, 1, 1 }, { 1, 1 } },
	};

	D3D11_BUFFER_DESC buffer_desc{};
	buffer_desc.ByteWidth = sizeof(vertices);
	buffer_desc.Usage = D3D11_USAGE_DYNAMIC;
	buffer_desc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	buffer_desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	buffer_desc.MiscFlags = 0;
	buffer_desc.StructureByteStride = 0;
	D3D11_SUBRESOURCE_DATA subresource_data{};
	subresource_data.pSysMem = vertices;
	subresource_data.SysMemPitch = 0;
	subresource_data.SysMemSlicePitch = 0;
	hr = device->CreateBuffer(&buffer_desc, &subresource_data, vertex_buffer.GetAddressOf());
	_ASSERT_EXPR(SUCCEEDED(hr), hr_trace(hr));

	D3D11_INPUT_ELEMENT_DESC input_element_desc[]
	{
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
	};

	//create vertex shadow shaders
    create_vs_from_cso(device, "shader//shadow_map_blur_df_vs.cso", blur_vertex_shader.GetAddressOf(), input_layout.GetAddressOf(), input_element_desc, _countof(input_element_desc));

	//create pixel shadow shaders
	shadow_pixel_shader = std::make_unique<pixel_shader>(device, "shader//shadow_map_df_ps.cso");
	blur_pixel_shader = std::make_unique<pixel_shader>(device, "shader//shadow_map_df_blur_ps.cso");

	// create constant buffer
	//D3D11_BUFFER_DESC bd;
	//ZeroMemory(&bd, sizeof(D3D11_BUFFER_DESC));
	//bd.ByteWidth = sizeof(CBShadowParam);
	//bd.Usage = D3D11_USAGE_DEFAULT;
	//bd.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	//bd.CPUAccessFlags = 0;
	//device->CreateBuffer(&bd, NULL, CBShadow.GetAddressOf());


	{
		// setting render target
		D3D11_TEXTURE2D_DESC td;
		ZeroMemory(&td, sizeof(D3D11_TEXTURE2D_DESC));
		// Resolution change
		td.Width = ShadowMapSize;
		td.Height = ShadowMapSize;
		td.MipLevels = 1;
		td.ArraySize = 1;
		//td.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
		td.Format = DXGI_FORMAT_R32G32_FLOAT;
		td.SampleDesc.Count = 1;
		td.SampleDesc.Quality = 0;
		td.Usage = D3D11_USAGE_DEFAULT;
		td.BindFlags = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE;
		td.CPUAccessFlags = 0;
		td.MiscFlags = 0;
		// create texture
		hr = device->CreateTexture2D(&td, NULL, Texture.ReleaseAndGetAddressOf());
		if (FAILED(hr)) return;

		hr = device->CreateTexture2D(&td, NULL, Texture2.ReleaseAndGetAddressOf());
		if (FAILED(hr)) return;

		//	render target view
		D3D11_RENDER_TARGET_VIEW_DESC rtvDesc;
		memset(&rtvDesc, 0, sizeof(rtvDesc));
		rtvDesc.Format = td.Format;
		rtvDesc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2D;
		hr = device->CreateRenderTargetView(Texture.Get(), &rtvDesc, RTV_Shadow.ReleaseAndGetAddressOf());
		hr = device->CreateRenderTargetView(Texture2.Get(), &rtvDesc, RTV_Shadow2.ReleaseAndGetAddressOf());

		// setting shader resource view
		D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc;
		memset(&srvDesc, 0, sizeof(srvDesc));
		srvDesc.Format = rtvDesc.Format;
		srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
		srvDesc.Texture2D.MipLevels = 1;
		// create shader resource view
		hr = device->CreateShaderResourceView(Texture.Get(), &srvDesc, SRV_Shadow.ReleaseAndGetAddressOf());
		if (FAILED(hr)) return;
		hr = device->CreateShaderResourceView(Texture2.Get(), &srvDesc, SRV_Shadow2.ReleaseAndGetAddressOf());
		if (FAILED(hr)) return;

		// setting sampler state
		D3D11_SAMPLER_DESC smpDesc;
		memset(&smpDesc, 0, sizeof(smpDesc));
		smpDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
		smpDesc.AddressU = D3D11_TEXTURE_ADDRESS_BORDER;
		smpDesc.AddressV = D3D11_TEXTURE_ADDRESS_BORDER;
		smpDesc.AddressW = D3D11_TEXTURE_ADDRESS_BORDER;
		smpDesc.ComparisonFunc = D3D11_COMPARISON_NEVER;
		smpDesc.BorderColor[0] = D3D11_FLOAT32_MAX;
		smpDesc.BorderColor[1] = D3D11_FLOAT32_MAX;
		smpDesc.BorderColor[2] = D3D11_FLOAT32_MAX;
		smpDesc.BorderColor[3] = D3D11_FLOAT32_MAX;
		smpDesc.MinLOD = 0;
		smpDesc.MaxLOD = D3D11_FLOAT32_MAX;

		// create sampler state
		device->CreateSamplerState(&smpDesc, SamplerState_Shadow.ReleaseAndGetAddressOf());
	}

	// 深度ステンシル設定
	{
		// 深度ステンシル設定
		D3D11_TEXTURE2D_DESC td;
		ZeroMemory(&td, sizeof(D3D11_TEXTURE2D_DESC));
		td.Width = ShadowMapSize;
		td.Height = ShadowMapSize;
		td.MipLevels = 1;
		td.ArraySize = 1;
		td.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
		td.SampleDesc.Count = 1;
		td.SampleDesc.Quality = 0;
		td.Usage = D3D11_USAGE_DEFAULT;
		td.BindFlags = D3D11_BIND_DEPTH_STENCIL;
		td.CPUAccessFlags = 0;
		td.MiscFlags = 0;

		// 深度ステンシルテクスチャ生成
		hr = device->CreateTexture2D(&td, NULL, Depth.GetAddressOf());
		if (FAILED(hr)) return;

		// 深度ステンシルビュー設定
		D3D11_DEPTH_STENCIL_VIEW_DESC dsvd;
		ZeroMemory(&dsvd, sizeof(D3D11_DEPTH_STENCIL_VIEW_DESC));
		dsvd.Format = td.Format;
		dsvd.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
		dsvd.Texture2D.MipSlice = 0;

		// create depth stencil view
		hr = device->CreateDepthStencilView(Depth.Get(), &dsvd, depth_stencil_view.ReleaseAndGetAddressOf());
		if (FAILED(hr)) return;
	}


}

void shadow_map:: active(ID3D11DeviceContext* immediate_context)
{
	//get default render target
	immediate_context->OMGetRenderTargets(1, default_render_target_view.ReleaseAndGetAddressOf(), default_depth_stencil_view.ReleaseAndGetAddressOf());

	ID3D11RenderTargetView* targets[] = { RTV_Shadow.Get() };

	//setting render target
	immediate_context->OMSetRenderTargets(1, targets, depth_stencil_view.Get());
	float clearColor[4] = { D3D11_FLOAT32_MAX, D3D11_FLOAT32_MAX, 0, 0 };
	immediate_context->ClearRenderTargetView(RTV_Shadow.Get(), clearColor);
	immediate_context->ClearDepthStencilView(depth_stencil_view.Get(), D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);

	// setting viewport
	D3D11_VIEWPORT vp = {
		0,0,
		ShadowMapSize,
		ShadowMapSize,
		0,1 };
	immediate_context->RSSetViewports(1, &vp);

	// update constntbuffer
	//Matrix view;
	//Matrix proj;
	//          幅　 高　 近　  遠
	//proj.Ortho(100, 100, 0.1f, 100);
	//view.LookAt(target - lightdir * 30, target, Vector3(0, 1, 0));
	//view.LookAt(Vector3(10,10,10), Vector3(0,0,0), Vector3(0, 1, 0));

	// コンスタントバッファ更新
	//CBShadowParam cb;
	//cb.ShadowVP = view * proj;
	//immediate_context->UpdateSubresource(CBShadow.Get(), 0, NULL, &cb, 0, 0);

	// スロット10に設定
	//immediate_context->VSSetConstantBuffers(10, 1, CBShadow.GetAddressOf());
	//immediate_context->PSSetConstantBuffers(10, 1, CBShadow.GetAddressOf());

	shadow_pixel_shader->active(immediate_context);
	//immediate_context->VSSetShader(vertex_shader.Get(), nullptr, 0);
}

void shadow_map::inactive(ID3D11DeviceContext* immediate_context)
{
	//blur the distance
	//set render target
	immediate_context->OMSetRenderTargets(
		1, RTV_Shadow2.GetAddressOf(), depth_stencil_view.Get());

	immediate_context->ClearDepthStencilView(
		depth_stencil_view.Get(), D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);

	immediate_context->PSSetShaderResources(
		0, 1, SRV_Shadow.GetAddressOf());

	blur_pixel_shader->active(immediate_context);
	immediate_context->VSSetShader(blur_vertex_shader.Get(), nullptr, 0);
	render_target.render(immediate_context);

	immediate_context->OMSetRenderTargets(1, default_render_target_view.GetAddressOf(), default_depth_stencil_view.Get());

	// set slot14
	immediate_context->PSSetShaderResources(14, 1, SRV_Shadow2.GetAddressOf());
	immediate_context->PSSetSamplers(14, 1, SamplerState_Shadow.GetAddressOf());
}

void shadow_map::DebugDrawGUI()
{
	ImGui::SetNextWindowPos(ImVec2(10, 10), ImGuiCond_FirstUseEver);
	ImGui::SetNextWindowSize(ImVec2(300, 300), ImGuiCond_FirstUseEver);

	if (ImGui::Begin("shadow", nullptr, ImGuiWindowFlags_None))
	{
		
		ImGui::Image(SRV_Shadow2.Get(), { 320,180 });
	}
	ImGui::End();
}