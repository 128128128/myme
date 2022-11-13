#include "sprite_batch.h" 
#include "../../Functions/misc.h"

#include <sstream>

#include <functional>

#include "../load/texture.h"
#include "../others/shader.h"

sprite_batch::sprite_batch(ID3D11Device* device, const char* filename, size_t max_instance, bool force_srgb, bool enable_caching, D3D11_FILTER sampler_filter, D3D11_TEXTURE_ADDRESS_MODE sampler_texture_address_mode, DirectX::XMFLOAT4 sampler_boarder_colour) : MAX_INSTANCES(max_instance)
{
	HRESULT hr{ S_OK };

	vertex vertices[] =
	{
		{ DirectX::XMFLOAT3(0, 0, 0), DirectX::XMFLOAT2(0, 0) },
		{ DirectX::XMFLOAT3(1, 0, 0), DirectX::XMFLOAT2(1, 0) },
		{ DirectX::XMFLOAT3(0, 1, 0), DirectX::XMFLOAT2(0, 1) },
		{ DirectX::XMFLOAT3(1, 1, 0), DirectX::XMFLOAT2(1, 1) },
	};

	D3D11_BUFFER_DESC buffer_desc{};
	buffer_desc.ByteWidth = sizeof(vertices);
	buffer_desc.Usage = D3D11_USAGE_DYNAMIC;
	buffer_desc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	buffer_desc.CPUAccessFlags = 0;
	buffer_desc.MiscFlags = 0;
	buffer_desc.StructureByteStride = 0;
	D3D11_SUBRESOURCE_DATA subresource_data{};
	subresource_data.pSysMem = vertices;
	subresource_data.SysMemPitch = 0; //Not use for vertex buffers.
	subresource_data.SysMemSlicePitch = 0; //Not use for vertex buffers.
	hr = device->CreateBuffer(&buffer_desc, &subresource_data, vertex_buffer.GetAddressOf());
	_ASSERT_EXPR(SUCCEEDED(hr), hr_trace(hr));

	const std::string shader_source =
		"Texture2D texture_map : register( t0 );\n"
		"SamplerState texture_map_sampler_state : register( s0 );\n"
		"struct VS_OUT\n"
		"{\n"
		"	float4 position : SV_POSITION;\n"
		"	float2 texcoord : TEXCOORD;\n"
		"	float4 colour : COLOR;\n"
		"};\n"
		"VS_OUT VS(float4 position : POSITION, float2 texcoord : TEXCOORD, float4 colour : COLOR, column_major float4x4 ndc_transform : NDC_TRANSFORM, float4 texcoord_transform : TEXCOORD_TRANSFORM)\n"
		"{\n"
		"	VS_OUT vout;\n"
		"	vout.position = mul(position, ndc_transform);\n"
		"	vout.texcoord = texcoord * texcoord_transform.zw + texcoord_transform.xy;\n"
		"	vout.colour = colour;\n"
		"	return vout;\n"
		"}\n"
		"float4 PS(VS_OUT pin) : SV_TARGET\n"
		"{\n"
		"	return texture_map.Sample(texture_map_sampler_state, pin.texcoord) * pin.colour;\n"
		"}\n";
	static const D3D11_INPUT_ELEMENT_DESC input_element_desc[] =
	{
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "NDC_TRANSFORM", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 1, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_INSTANCE_DATA, 1 },
		{ "NDC_TRANSFORM", 1, DXGI_FORMAT_R32G32B32A32_FLOAT, 1, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_INSTANCE_DATA, 1 },
		{ "NDC_TRANSFORM", 2, DXGI_FORMAT_R32G32B32A32_FLOAT, 1, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_INSTANCE_DATA, 1 },
		{ "NDC_TRANSFORM", 3, DXGI_FORMAT_R32G32B32A32_FLOAT, 1, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_INSTANCE_DATA, 1 },
		{ "TEXCOORD_TRANSFORM", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 1, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_INSTANCE_DATA, 1 },
		{ "COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 1, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_INSTANCE_DATA, 1 },
	};
	create_vs_from_source(device, "sprite_batch_vs", shader_source, "VS", vertex_shader.ReleaseAndGetAddressOf(), input_layout.ReleaseAndGetAddressOf(), const_cast<D3D11_INPUT_ELEMENT_DESC*>(input_element_desc), _countof(input_element_desc));
	create_ps_from_source(device, "sprite_batch_ps", shader_source, "PS", pixel_shader.ReleaseAndGetAddressOf());

	instance* instances = new instance[MAX_INSTANCES];
	{
		D3D11_BUFFER_DESC buffer_desc = {};
		D3D11_SUBRESOURCE_DATA subresource_data = {};

		buffer_desc.ByteWidth = sizeof(instance) * MAX_INSTANCES;
		buffer_desc.Usage = D3D11_USAGE_DYNAMIC;
		buffer_desc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
		buffer_desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
		buffer_desc.MiscFlags = 0;
		buffer_desc.StructureByteStride = 0;
		subresource_data.pSysMem = instances;
		subresource_data.SysMemPitch = 0; //Not use for vertex buffers.mm 
		subresource_data.SysMemSlicePitch = 0; //Not use for vertex buffers.
		hr = device->CreateBuffer(&buffer_desc, &subresource_data, instance_buffer.GetAddressOf());
		_ASSERT_EXPR(SUCCEEDED(hr), hr_trace(hr));
	}
	delete[] instances;

	load_texture_from_file(device, filename, shader_resource_view.GetAddressOf(), force_srgb, enable_caching);
	texture2d_description(shader_resource_view.Get(), texture2d_desc);

	D3D11_SAMPLER_DESC sampler_desc;
	sampler_desc.Filter = sampler_filter;
	sampler_desc.AddressU = sampler_texture_address_mode;
	sampler_desc.AddressV = sampler_texture_address_mode;
	sampler_desc.AddressW = sampler_texture_address_mode;
	sampler_desc.MipLODBias = 0;
	sampler_desc.MaxAnisotropy = 16;
	sampler_desc.ComparisonFunc = D3D11_COMPARISON_ALWAYS;
	memcpy(sampler_desc.BorderColor, &sampler_boarder_colour, sizeof(DirectX::XMFLOAT4));
	sampler_desc.MinLOD = 0;
	sampler_desc.MaxLOD = D3D11_FLOAT32_MAX;
	hr = device->CreateSamplerState(&sampler_desc, embedded_sampler_state.GetAddressOf());
	_ASSERT_EXPR(SUCCEEDED(hr), hr_trace(hr));

	D3D11_RASTERIZER_DESC rasterizer_desc = {};
	rasterizer_desc.FillMode = D3D11_FILL_SOLID; //D3D11_FILL_WIREFRAME, D3D11_FILL_SOLID
	rasterizer_desc.CullMode = D3D11_CULL_NONE; //D3D11_CULL_NONE, D3D11_CULL_FRONT, D3D11_CULL_BACK   
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
void sprite_batch::render(ID3D11DeviceContext* immediate_context,
	float dx, float dy, float dw, float dh,
	float r, float g, float b, float a,
	float angle/*degree*/)
{
	render(immediate_context, dx, dy, dw, dh, r, g, b, a, angle, 0.0f, 0.0f, static_cast<float>(texture2d_desc.Width), static_cast<float>(texture2d_desc.Height));
}
void sprite_batch::render(ID3D11DeviceContext* immediate_context,
	float dx, float dy, float dw, float dh,
	float r, float g, float b, float a,
	float angle/*degree*/,
	float sx, float sy, float sw, float sh)
{
	_ASSERT_EXPR(count_instance < MAX_INSTANCES, L"Number of instances must be less than MAX_INSTANCES.");

	float cx = dw * 0.5f, cy = dh * 0.5f; /*Center of Rotation*/
#if 0
	DirectX::XMVECTOR scaling = DirectX::XMVectorSet(dw, dh, 1.0f, 0.0f);
	DirectX::XMVECTOR origin = DirectX::XMVectorSet(cx, cy, 0.0f, 0.0f);
	DirectX::XMVECTOR translation = DirectX::XMVectorSet(dx, dy, 0.0f, 0.0f);
	DirectX::XMMATRIX M = DirectX::XMMatrixAffineTransformation2D(scaling, origin, angle * 0.01745f, translation);
	DirectX::XMMATRIX N(
		2.0f / viewport.Width, 0.0f, 0.0f, 0.0f,
		0.0f, -2.0f / viewport.Height, 0.0f, 0.0f,
		0.0f, 0.0f, 1.0f, 0.0f,
		-1.0f, 1.0f, 0.0f, 1.0f);
	XMStoreFloat4x4(&instances[count_instance].ndc_transform, DirectX::XMMatrixTranspose(M * N)); //column_major
#else
	float c = cosf(angle * 0.01745f);
	float s = sinf(angle * 0.01745f);
	float w = 2.0f / viewport.Width;
	float h = -2.0f / viewport.Height;
	instances[count_instance].ndc_transform._11 = w * dw * c;
	instances[count_instance].ndc_transform._21 = h * dw * s;
	instances[count_instance].ndc_transform._31 = 0.0f;
	instances[count_instance].ndc_transform._41 = 0.0f;
	instances[count_instance].ndc_transform._12 = w * dh * -s;
	instances[count_instance].ndc_transform._22 = h * dh * c;
	instances[count_instance].ndc_transform._32 = 0.0f;
	instances[count_instance].ndc_transform._42 = 0.0f;
	instances[count_instance].ndc_transform._13 = 0.0f;
	instances[count_instance].ndc_transform._23 = 0.0f;
	instances[count_instance].ndc_transform._33 = 1.0f;
	instances[count_instance].ndc_transform._43 = 0.0f;
	instances[count_instance].ndc_transform._14 = w * (-cx * c + -cy * -s + cx + dx) - 1.0f;
	instances[count_instance].ndc_transform._24 = h * (-cx * s + -cy * c + cy + dy) + 1.0f;
	instances[count_instance].ndc_transform._34 = 0.0f;
	instances[count_instance].ndc_transform._44 = 1.0f;
#endif
	float tw = static_cast<float>(texture2d_desc.Width);
	float th = static_cast<float>(texture2d_desc.Height);
	instances[count_instance].texcoord_transform = DirectX::XMFLOAT4(sx / tw, sy / th, sw / tw, sh / th);
	instances[count_instance].color = DirectX::XMFLOAT4(r, g, b, a);

	count_instance++;
}
void sprite_batch::render(ID3D11DeviceContext* immediate_context, float dx, float dy, float dw, float dh)
{
	render(immediate_context, dx, dy, dw, dh, 1.0f, 1.0f, 1.0f, 1.0f, 0.0f,
		0.0f, 0.0f, static_cast<float>(texture2d_desc.Width), static_cast<float>(texture2d_desc.Height));
}

void sprite_batch::begin(ID3D11DeviceContext* immediate_context, bool use_embedded_vertex_shader, bool use_embedded_pixel_shader,
	bool use_embedded_rasterizer_state, bool use_embedded_depth_stencil_state, bool use_embedded_sampler_state)
{
	_ASSERT_EXPR(!using_embedded_rasterizer_state, L"billboard::using_embedded_rasterizer_state is status ailment");
	using_embedded_rasterizer_state = use_embedded_rasterizer_state;
	if (use_embedded_rasterizer_state)
	{
		immediate_context->RSGetState(default_rasterizer_state.ReleaseAndGetAddressOf());
		immediate_context->RSSetState(embedded_rasterizer_state.Get());
	}

	_ASSERT_EXPR(!using_embedded_depth_stencil_state, L"billboard::using_embedded_rasterizer_state is status ailment");
	using_embedded_depth_stencil_state = use_embedded_depth_stencil_state;
	if (use_embedded_depth_stencil_state)
	{
		immediate_context->OMGetDepthStencilState(default_depth_stencil_state.ReleaseAndGetAddressOf(), 0);
		immediate_context->OMSetDepthStencilState(embedded_depth_stencil_state.Get(), 1);
	}

	_ASSERT_EXPR(!using_embedded_sampler_state, L"billboard::using_embedded_rasterizer_state is status ailment");
	using_embedded_sampler_state = use_embedded_sampler_state;
	if (use_embedded_sampler_state)
	{
		immediate_context->PSGetSamplers(0, 1, default_sampler_state.ReleaseAndGetAddressOf());
		immediate_context->PSSetSamplers(0, 1, embedded_sampler_state.GetAddressOf());
	}

	if (use_embedded_vertex_shader)
	{
		immediate_context->VSSetShader(vertex_shader.Get(), 0, 0);
	}
	if (use_embedded_pixel_shader)
	{
		immediate_context->PSSetShader(pixel_shader.Get(), 0, 0);
	}

	UINT strides[2] = { sizeof(vertex), sizeof(instance) };
	UINT offsets[2] = { 0, 0 };
	ID3D11Buffer* vbs[2] = { vertex_buffer.Get(), instance_buffer.Get() };
	immediate_context->IASetVertexBuffers(0, 2, vbs, strides, offsets);
	immediate_context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);
	immediate_context->IASetInputLayout(input_layout.Get());

	immediate_context->PSSetShaderResources(0, 1, shader_resource_view.GetAddressOf());

	UINT num_viewports = 1;
	immediate_context->RSGetViewports(&num_viewports, &viewport);

	D3D11_MAP map = D3D11_MAP_WRITE_DISCARD;
	D3D11_MAPPED_SUBRESOURCE mapped_buffer;
	HRESULT hr = immediate_context->Map(instance_buffer.Get(), 0, map, 0, &mapped_buffer);
	_ASSERT_EXPR(SUCCEEDED(hr), hr_trace(hr));
	instances = static_cast<instance*>(mapped_buffer.pData);

	count_instance = 0;
}

void sprite_batch::end(ID3D11DeviceContext* immediate_context)
{
	immediate_context->Unmap(instance_buffer.Get(), 0);
	immediate_context->DrawInstanced(4, count_instance, 0, 0);

	if (using_embedded_rasterizer_state)
	{
		immediate_context->RSSetState(default_rasterizer_state.Get());
		using_embedded_rasterizer_state = false;
	}
	if (using_embedded_depth_stencil_state)
	{
		immediate_context->OMSetDepthStencilState(default_depth_stencil_state.Get(), 1);
		using_embedded_depth_stencil_state = false;
	}
	if (using_embedded_sampler_state)
	{
		immediate_context->PSSetSamplers(0, 1, default_sampler_state.GetAddressOf());
		using_embedded_sampler_state = false;
	}

	immediate_context->IASetInputLayout(0);
	immediate_context->VSSetShader(0, 0, 0);
	immediate_context->PSSetShader(0, 0, 0);

	ID3D11ShaderResourceView* null_shader_resource_view = 0;
	immediate_context->PSSetShaderResources(0, 1, &null_shader_resource_view);
}
