#include <d3dcompiler.h>
#ifdef _MSC_VER
#pragma comment(lib, "d3dcompiler") 
#endif

#include "rasterizer.h"

#include "shader.h"
#include "../../Graphics/load/texture.h"
#include "../../Functions/utility_graphics.h"
#include "../../Functions/misc.h"

namespace Descartes
{
	rasterizer::rasterizer(ID3D11Device* device, D3D11_FILTER sampler_filter, D3D11_TEXTURE_ADDRESS_MODE sampler_texture_address_mode, DirectX::XMFLOAT4 sampler_boarder_color)
	{
		HRESULT hr = S_OK;

		vertex vertices[] =
		{
			{ DirectX::XMFLOAT3(0, 0, 0), DirectX::XMFLOAT2(0, 0), DirectX::XMFLOAT4(1, 1, 1, 1) },
			{ DirectX::XMFLOAT3(0, 0, 0), DirectX::XMFLOAT2(0, 0), DirectX::XMFLOAT4(1, 1, 1, 1) },
			{ DirectX::XMFLOAT3(0, 0, 0), DirectX::XMFLOAT2(0, 0), DirectX::XMFLOAT4(1, 1, 1, 1) },
			{ DirectX::XMFLOAT3(0, 0, 0), DirectX::XMFLOAT2(0, 0), DirectX::XMFLOAT4(1, 1, 1, 1) },
		};

		D3D11_BUFFER_DESC buffer_desc = {};
		buffer_desc.ByteWidth = sizeof(vertices);
		buffer_desc.Usage = D3D11_USAGE_DYNAMIC;
		buffer_desc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
		buffer_desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
		buffer_desc.MiscFlags = 0;
		buffer_desc.StructureByteStride = 0;
		D3D11_SUBRESOURCE_DATA subresource_data = {};
		subresource_data.pSysMem = vertices;
		subresource_data.SysMemPitch = 0;
		subresource_data.SysMemSlicePitch = 0;
		hr = device->CreateBuffer(&buffer_desc, &subresource_data, vertex_buffer.GetAddressOf());
		_ASSERT_EXPR(SUCCEEDED(hr), hr_trace(hr));

		const std::string rasterizer_vs =
			"struct VS_OUT\n"
			"{\n"
			"	float4 position : SV_POSITION;\n"
			"	float2 texcoord : TEXCOORD;\n"
			"	float4 color : COLOR;\n"
			"};\n"
			"VS_OUT VS(float4 position : POSITION, float2 texcoord : TEXCOORD, float4 color : COLOR)\n"
			"{\n"
			"	VS_OUT vout;\n"
			"	vout.position = position;\n"
			"	vout.texcoord = texcoord;\n"
			"	vout.color = color;\n"
			"	return vout;\n"
			"}\n";
		static const D3D11_INPUT_ELEMENT_DESC input_element_desc[] =
		{
			{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
			{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
			{ "COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		};
		create_vs_from_source(device, "rasterizer_vs", rasterizer_vs, "VS", embedded_vertex_shader.ReleaseAndGetAddressOf(), embedded_input_layout.ReleaseAndGetAddressOf(), const_cast<D3D11_INPUT_ELEMENT_DESC*>(input_element_desc), _countof(input_element_desc));

		const std::string rasterizer_ps =
			"Texture2D texture_map : register( t0 );\n"
			"SamplerState texture_map_sampler_state : register( s0 );\n"
			"struct VS_OUT\n"
			"{\n"
			"	float4 position : SV_POSITION;\n"
			"	float2 texcoord : TEXCOORD;\n"
			"	float4 color : COLOR;\n"
			"};\n"
			"float4 PS(VS_OUT pin) : SV_TARGET\n"
			"{\n"
			"	return texture_map.Sample(texture_map_sampler_state, pin.texcoord) * pin.color;\n"
			"}\n";
		create_ps_from_source(device, "rasterizer_ps", rasterizer_ps, "PS", embedded_pixel_shaders[0].ReleaseAndGetAddressOf());

		const std::string rasterizer_ps_ms =
			"Texture2DMS<float4> texture_map : register(t0);\n"
			"struct VS_OUT\n"
			"{\n"
			"	float4 position : SV_POSITION;\n"
			"	float2 texcoord : TEXCOORD;\n"
			"	float4 color : COLOR;\n"
			"};\n"
			"float4 PS(VS_OUT pin, uint sample_index : SV_SAMPLEINDEX) : SV_TARGET\n"
			"{\n"
			"	uint width, height, number_of_samples;\n"
			"	texture_map.GetDimensions(width, height, number_of_samples);\n"
			"	return texture_map.Load(uint2(pin.texcoord.x * width, pin.texcoord.y * height), sample_index) * pin.color;\n"
			"}\n";
		create_ps_from_source(device, "rasterizer_ps_ms", rasterizer_ps_ms, "PS", embedded_pixel_shaders[1].ReleaseAndGetAddressOf());

		D3D11_SAMPLER_DESC sampler_desc;
		sampler_desc.Filter = sampler_filter; //D3D11_FILTER_MIN_MAG_MIP_POINT D3D11_FILTER_ANISOTROPIC
		sampler_desc.AddressU = sampler_texture_address_mode;
		sampler_desc.AddressV = sampler_texture_address_mode;
		sampler_desc.AddressW = sampler_texture_address_mode;
		sampler_desc.MipLODBias = 0;
		sampler_desc.MaxAnisotropy = 16;
		sampler_desc.ComparisonFunc = D3D11_COMPARISON_ALWAYS;
		memcpy(sampler_desc.BorderColor, &sampler_boarder_color, sizeof(DirectX::XMFLOAT4));
		sampler_desc.MinLOD = 0;
		sampler_desc.MaxLOD = D3D11_FLOAT32_MAX;
		hr = device->CreateSamplerState(&sampler_desc, embedded_sampler_state.GetAddressOf());
		_ASSERT_EXPR(SUCCEEDED(hr), hr_trace(hr));

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
		hr = device->CreateRasterizerState(&rasterizer_desc, embedded_rasterizer_state.GetAddressOf());
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
		hr = device->CreateDepthStencilState(&depth_stencil_desc, embedded_depth_stencil_state.GetAddressOf());
		_ASSERT_EXPR(SUCCEEDED(hr), hr_trace(hr));
	}
	void rasterizer::blit
	(
		ID3D11DeviceContext* immediate_context,
		ID3D11ShaderResourceView* shader_resource_view,
		float dx, float dy, float dw, float dh,
		float sx, float sy, float sw, float sh,
		float angle/*degree*/,
		float r, float g, float b, float a,
		bool use_embedded_vertex_shader,
		bool use_embedded_pixel_shader,
		bool use_embedded_rasterizer_state,
		bool use_embedded_depth_stencil_state,
		bool use_embedded_sampler_state
	) const
		// dx, dy : Coordinate of sprite's left-top corner in screen space 
		// dw, dh : Size of sprite in screen space 
		// angle : Raotation angle (Rotation centre is sprite's centre), unit is degree
		// r, g, b : Color of sprite's each vertices 
	{
		HRESULT hr = S_OK;

		D3D11_SHADER_RESOURCE_VIEW_DESC shader_resource_view_desc = {};
		shader_resource_view->GetDesc(&shader_resource_view_desc);
		bool multisampled = shader_resource_view_desc.ViewDimension == D3D11_SRV_DIMENSION_TEXTURE2D;

		Microsoft::WRL::ComPtr<ID3D11Resource> resource;
		shader_resource_view->GetResource(resource.GetAddressOf());

		Microsoft::WRL::ComPtr<ID3D11Texture2D> texture2d;
		hr = resource.Get()->QueryInterface<ID3D11Texture2D>(texture2d.GetAddressOf());
		_ASSERT_EXPR(SUCCEEDED(hr), hr_trace(hr));

		D3D11_TEXTURE2D_DESC texture2d_desc;
		texture2d->GetDesc(&texture2d_desc);

		D3D11_VIEWPORT viewport;
		UINT num_viewports = 1;
		immediate_context->RSGetViewports(&num_viewports, &viewport);
		float screen_width = viewport.Width;
		float screen_height = viewport.Height;

		// Set each sprite's vertices coordinate to screen space num BLEND_STATE
		// left-top
		float x0 = dx;
		float y0 = dy;
		// right-top
		float x1 = dx + dw;
		float y1 = dy;
		// left-bottom
		float x2 = dx;
		float y2 = dy + dh;
		// right-bottom
		float x3 = dx + dw;
		float y3 = dy + dh;

		// Translate sprite's centre to origin (rotate centre)
		float mx = dx + dw * 0.5f;
		float my = dy + dh * 0.5f;
		x0 -= mx;
		y0 -= my;
		x1 -= mx;
		y1 -= my;
		x2 -= mx;
		y2 -= my;
		x3 -= mx;
		y3 -= my;

		// Rotate each sprite's vertices by angle
		float rx, ry;
		float cos = cosf(angle * 0.01745f);
		float sin = sinf(angle * 0.01745f);
		rx = x0;
		ry = y0;
		x0 = cos * rx + -sin * ry;
		y0 = sin * rx + cos * ry;
		rx = x1;
		ry = y1;
		x1 = cos * rx + -sin * ry;
		y1 = sin * rx + cos * ry;
		rx = x2;
		ry = y2;
		x2 = cos * rx + -sin * ry;
		y2 = sin * rx + cos * ry;
		rx = x3;
		ry = y3;
		x3 = cos * rx + -sin * ry;
		y3 = sin * rx + cos * ry;

		// Translate sprite's centre to original position
		x0 += mx;
		y0 += my;
		x1 += mx;
		y1 += my;
		x2 += mx;
		y2 += my;
		x3 += mx;
		y3 += my;

		// Convert to NDC space
		x0 = 2.0f * x0 / screen_width - 1.0f;
		y0 = 1.0f - 2.0f * y0 / screen_height;
		x1 = 2.0f * x1 / screen_width - 1.0f;
		y1 = 1.0f - 2.0f * y1 / screen_height;
		x2 = 2.0f * x2 / screen_width - 1.0f;
		y2 = 1.0f - 2.0f * y2 / screen_height;
		x3 = 2.0f * x3 / screen_width - 1.0f;
		y3 = 1.0f - 2.0f * y3 / screen_height;

		D3D11_MAP map = D3D11_MAP_WRITE_DISCARD;
		D3D11_MAPPED_SUBRESOURCE mapped_buffer;
		hr = immediate_context->Map(vertex_buffer.Get(), 0, map, 0, &mapped_buffer);
		_ASSERT_EXPR(SUCCEEDED(hr), hr_trace(hr));

		vertex* vertices = static_cast<vertex*>(mapped_buffer.pData);
		vertices[0].position.x = x0;
		vertices[0].position.y = y0;
		vertices[1].position.x = x1;
		vertices[1].position.y = y1;
		vertices[2].position.x = x2;
		vertices[2].position.y = y2;
		vertices[3].position.x = x3;
		vertices[3].position.y = y3;
		vertices[0].position.z = vertices[1].position.z = vertices[2].position.z = vertices[3].position.z = 0.0f;

		DirectX::XMFLOAT4 color(r, g, b, a);
		vertices[0].color = vertices[1].color = vertices[2].color = vertices[3].color = color;

		vertices[0].texcoord.x = (sx) / texture2d_desc.Width;
		vertices[0].texcoord.y = (sy) / texture2d_desc.Height;
		vertices[1].texcoord.x = (sx + sw) / texture2d_desc.Width;
		vertices[1].texcoord.y = (sy) / texture2d_desc.Height;
		vertices[2].texcoord.x = (sx) / texture2d_desc.Width;
		vertices[2].texcoord.y = (sy + sh) / texture2d_desc.Height;
		vertices[3].texcoord.x = (sx + sw) / texture2d_desc.Width;
		vertices[3].texcoord.y = (sy + sh) / texture2d_desc.Height;

		immediate_context->Unmap(vertex_buffer.Get(), 0);

		UINT stride = sizeof(vertex);
		UINT offset = 0;
		immediate_context->IASetVertexBuffers(0, 1, vertex_buffer.GetAddressOf(), &stride, &offset);
		immediate_context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);
		immediate_context->IASetInputLayout(embedded_input_layout.Get());

		if (use_embedded_vertex_shader)
		{
			immediate_context->VSSetShader(embedded_vertex_shader.Get(), 0, 0);
		}
		if (use_embedded_pixel_shader)
		{
			immediate_context->PSSetShader(embedded_pixel_shaders[0].Get(), 0, 0);
		}
		immediate_context->PSSetShaderResources(0, 1, &shader_resource_view);

		Microsoft::WRL::ComPtr<ID3D11RasterizerState> default_rasterizer_state;
		if (use_embedded_rasterizer_state)
		{
			immediate_context->RSGetState(default_rasterizer_state.ReleaseAndGetAddressOf());
			immediate_context->RSSetState(embedded_rasterizer_state.Get());
		}
		Microsoft::WRL::ComPtr<ID3D11DepthStencilState> default_depth_stencil_state;
		if (use_embedded_depth_stencil_state)
		{
			immediate_context->OMGetDepthStencilState(default_depth_stencil_state.ReleaseAndGetAddressOf(), 0);
			immediate_context->OMSetDepthStencilState(embedded_depth_stencil_state.Get(), 1);
		}
		Microsoft::WRL::ComPtr<ID3D11SamplerState> default_sampler_state;
		if (use_embedded_sampler_state)
		{
			immediate_context->PSGetSamplers(0, 1, default_sampler_state.ReleaseAndGetAddressOf());
			immediate_context->PSSetSamplers(0, 1, embedded_sampler_state.GetAddressOf());
		}
		immediate_context->Draw(4, 0);

		immediate_context->IASetInputLayout(0);
		immediate_context->VSSetShader(0, 0, 0);
		immediate_context->PSSetShader(0, 0, 0);

		ID3D11ShaderResourceView* null_shader_resource_view = 0;
		immediate_context->PSSetShaderResources(0, 1, &null_shader_resource_view);

		if (default_rasterizer_state)
		{
			immediate_context->RSSetState(default_rasterizer_state.Get());
		}
		if (default_depth_stencil_state)
		{
			immediate_context->OMSetDepthStencilState(default_depth_stencil_state.Get(), 1);
		}
		if (default_sampler_state)
		{
			immediate_context->PSSetSamplers(0, 1, default_sampler_state.GetAddressOf());
		}
	}
	void rasterizer::blit
	(
		ID3D11DeviceContext* immediate_context,
		ID3D11ShaderResourceView* shader_resource_view,
		float dx, float dy, float dw, float dh,
		float angle/*degree*/,
		float r, float g, float b, float a,
		bool use_embedded_vertex_shader,
		bool use_embedded_pixel_shader,
		bool use_embedded_rasterizer_state,
		bool use_embedded_depth_stencil_state,
		bool use_embedded_sampler_state
	) const
	{
		HRESULT hr = S_OK;

		Microsoft::WRL::ComPtr<ID3D11Resource> resource;
		shader_resource_view->GetResource(resource.GetAddressOf());

		Microsoft::WRL::ComPtr<ID3D11Texture2D> texture2d;
		hr = resource.Get()->QueryInterface<ID3D11Texture2D>(texture2d.GetAddressOf());
		_ASSERT_EXPR(SUCCEEDED(hr), hr_trace(hr));

		D3D11_TEXTURE2D_DESC texture2d_desc;
		texture2d->GetDesc(&texture2d_desc);

		blit(immediate_context, shader_resource_view, dx, dy, dw, dh, 0.0f, 0.0f, static_cast<float>(texture2d_desc.Width), static_cast<float>(texture2d_desc.Height), angle, r, g, b, a,
			use_embedded_vertex_shader, use_embedded_pixel_shader, use_embedded_rasterizer_state, use_embedded_depth_stencil_state, use_embedded_sampler_state);
	}

	fullscreen_quad::fullscreen_quad(ID3D11Device* device)
	{
		HRESULT hr = S_OK;

		const std::string fullscreen_quad_vs =
			"struct VS_OUT\n"
			"{\n"
			"	float4 position : SV_POSITION;\n"
			"	float2 texcoord : TEXCOORD;\n"
			"};\n"
			"VS_OUT VS(in uint vertex_id : SV_VERTEXID)\n"
			"{\n"
			"	VS_OUT vout;\n"
			"	vout.texcoord = float2((vertex_id << 1) & 2, vertex_id & 2);\n"
			"	vout.position = float4(vout.texcoord * float2(2.0f, -2.0f) + float2(-1.0f, 1.0f), 0.0f, 1.0f);\n"
			"	return vout;\n"
			"}\n";
		create_vs_from_source(device, "fullscreen_quad_vs", fullscreen_quad_vs, "VS", embedded_vertex_shader.ReleaseAndGetAddressOf(), 0, 0, 0);

		const std::string fullscreen_quad_ps =
			"Texture2D texture_map : register( t0 );\n"
			"SamplerState texture_map_sampler_state : register( s0 );\n"
			"struct VS_OUT\n"
			"{\n"
			"	float4 position : SV_POSITION;\n"
			"	float2 texcoord : TEXCOORD;\n"
			"};\n"
			"float4 PS(VS_OUT pin) : SV_TARGET\n"
			"{\n"
			"	return texture_map.Sample(texture_map_sampler_state, pin.texcoord);\n"
			"}\n";
		create_ps_from_source(device, "fullscreen_quad_ps", fullscreen_quad_ps, "PS", embedded_pixel_shaders[0].ReleaseAndGetAddressOf());

		const std::string fullscreen_quad_ps_ms =
			"Texture2DMS<float4> texture_map : register(t0);\n"
			"struct VS_OUT\n"
			"{\n"
			"	float4 position : SV_POSITION;\n"
			"	float2 texcoord : TEXCOORD;\n"
			"};\n"
			"float4 PS(VS_OUT pin, uint sample_index : SV_SAMPLEINDEX) : SV_TARGET\n"
			"{\n"
			"	uint width, height, number_of_samples;\n"
			"	texture_map.GetDimensions(width, height, number_of_samples);\n"
			"	return texture_map.Load(uint2(pin.texcoord.x * width, pin.texcoord.y * height), sample_index);\n"
			"}\n";
		create_ps_from_source(device, "fullscreen_quad_ps_ms", fullscreen_quad_ps_ms, "PS", embedded_pixel_shaders[1].ReleaseAndGetAddressOf());

		D3D11_RASTERIZER_DESC rasterizer_desc = {};
		rasterizer_desc.FillMode = D3D11_FILL_SOLID;
		rasterizer_desc.CullMode = D3D11_CULL_BACK;
		rasterizer_desc.FrontCounterClockwise = FALSE;
		rasterizer_desc.DepthBias = 0;
		rasterizer_desc.DepthBiasClamp = 0;
		rasterizer_desc.SlopeScaledDepthBias = 0;
		rasterizer_desc.DepthClipEnable = FALSE;
		rasterizer_desc.ScissorEnable = FALSE;
		rasterizer_desc.MultisampleEnable = FALSE;
		rasterizer_desc.AntialiasedLineEnable = FALSE;
		hr = device->CreateRasterizerState(&rasterizer_desc, embedded_rasterizer_state.GetAddressOf());
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
		hr = device->CreateDepthStencilState(&depth_stencil_desc, embedded_depth_stencil_state.GetAddressOf());
		_ASSERT_EXPR(SUCCEEDED(hr), hr_trace(hr));
	}
	void fullscreen_quad::blit(ID3D11DeviceContext* immediate_context, bool use_embedded_rasterizer_state, bool use_embedded_depth_stencil_state, bool use_embedded_pixel_shader, bool enable_msaa)
	{
		Microsoft::WRL::ComPtr<ID3D11RasterizerState> default_rasterizer_state;
		if (use_embedded_rasterizer_state)
		{
			immediate_context->RSGetState(default_rasterizer_state.ReleaseAndGetAddressOf());
			immediate_context->RSSetState(embedded_rasterizer_state.Get());
		}

		Microsoft::WRL::ComPtr<ID3D11DepthStencilState> default_depth_stencil_state;
		if (use_embedded_depth_stencil_state)
		{
			immediate_context->OMGetDepthStencilState(default_depth_stencil_state.ReleaseAndGetAddressOf(), 0);
			immediate_context->OMSetDepthStencilState(embedded_depth_stencil_state.Get(), 1);
		}
		if (use_embedded_pixel_shader)
		{
			immediate_context->PSSetShader(embedded_pixel_shaders[0].Get(), 0, 0);
		}

		immediate_context->IASetVertexBuffers(0, 0, 0, 0, 0);
		immediate_context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);
		immediate_context->IASetInputLayout(0);

		immediate_context->VSSetShader(embedded_vertex_shader.Get(), 0, 0);

		immediate_context->Draw(4, 0);

		//immediate_context->IASetInputLayout(0);
		immediate_context->VSSetShader(nullptr, nullptr, 0);
		immediate_context->PSSetShader(nullptr, nullptr, 0);

		if (default_rasterizer_state)
		{
			immediate_context->RSSetState(default_rasterizer_state.Get());
		}
		if (default_depth_stencil_state)
		{
			immediate_context->OMSetDepthStencilState(default_depth_stencil_state.Get(), 1);
		}
	}
}