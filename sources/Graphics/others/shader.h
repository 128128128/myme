#pragma once

#include <d3d11.h>
#include <string>
#include <wrl.h>
#include <d3dcompiler.h>

//create shader 
HRESULT create_vs_from_cso(ID3D11Device* device, const char* cso_name, ID3D11VertexShader** vertex_shader, ID3D11InputLayout** input_layout, D3D11_INPUT_ELEMENT_DESC* input_element_desc, UINT num_elements);
HRESULT create_ps_from_cso(ID3D11Device* device, const char* cso_name, ID3D11PixelShader** pixel_shader);
HRESULT create_ds_from_cso(ID3D11Device* device, const char* cso_name, ID3D11DomainShader** domain_shader);
HRESULT create_cs_from_cso(ID3D11Device* device, const char* cso_name, ID3D11ComputeShader** compute_shader);
HRESULT create_gs_from_cso(ID3D11Device* device, const char* cso_name, ID3D11GeometryShader** geometry_shader);
HRESULT create_vs_from_source(ID3D11Device* device, const char* key_name, const std::string& source, const char* entry_point, ID3D11VertexShader** vertex_shader, ID3D11InputLayout** input_layout, D3D11_INPUT_ELEMENT_DESC* input_element_desc, size_t num_elements, bool enable_caching = true);
void release_all_cached_vertex_shaders();

HRESULT create_ps_from_source(ID3D11Device* device, const char* key_name, const std::string& source, const char* entry_point, ID3D11PixelShader** pixel_shader, bool enable_caching = true);
void release_all_cached_pixel_shaders();
HRESULT create_hs_from_cso(ID3D11Device* device, const char* cso_name, ID3D11HullShader** hull_shader);

template<class T>
class vertex_shader
{
public:
	//if you substitute 0 for 'cso' this creates null vertex shader object
	vertex_shader(ID3D11Device* device, const char* cso)
	{
		size_t num_elements;
		const D3D11_INPUT_ELEMENT_DESC* input_element_desc = T::acquire_input_element_desc(num_elements);
		if (cso)
		{
			create_vs_from_cso(device, cso, shader_object.GetAddressOf(), input_layout.GetAddressOf(), const_cast<D3D11_INPUT_ELEMENT_DESC*>(input_element_desc), static_cast<UINT>(num_elements));
		}
	}
	virtual ~vertex_shader() = default;
	vertex_shader(vertex_shader&) = delete;
	vertex_shader& operator=(vertex_shader&) = delete;

	Microsoft::WRL::ComPtr<ID3D11VertexShader> shader_object;
	Microsoft::WRL::ComPtr<ID3D11InputLayout> input_layout;

	void active(ID3D11DeviceContext* immediate_context)
	{
		immediate_context->IAGetInputLayout(default_input_layout.ReleaseAndGetAddressOf());
		immediate_context->VSGetShader(default_shader_object.ReleaseAndGetAddressOf(), 0, 0);

		immediate_context->IASetInputLayout(input_layout.Get());
		immediate_context->VSSetShader(shader_object.Get(), 0, 0);
	}
	void inactive(ID3D11DeviceContext* immediate_context)
	{
		immediate_context->IASetInputLayout(default_input_layout.Get());
		immediate_context->VSSetShader(default_shader_object.Get(), 0, 0);
	}
private:
	Microsoft::WRL::ComPtr<ID3D11VertexShader> default_shader_object;
	Microsoft::WRL::ComPtr<ID3D11InputLayout> default_input_layout;

};
class pixel_shader
{
public:
	//if you substitute 0 for 'cso' this creates null pixel shader object
	pixel_shader(ID3D11Device* device, const char* cso)
	{
		if (cso)
		{
			create_ps_from_cso(device, cso, shader_object.GetAddressOf());
		}
	}
	virtual ~pixel_shader() = default;
	pixel_shader(pixel_shader&) = delete;
	pixel_shader& operator=(pixel_shader&) = delete;

	Microsoft::WRL::ComPtr<ID3D11PixelShader> shader_object;

	void active(ID3D11DeviceContext* immediate_context)
	{
		immediate_context->PSGetShader(default_shader_object.ReleaseAndGetAddressOf(), 0, 0);
		immediate_context->PSSetShader(shader_object.Get(), 0, 0);
	}
	void inactive(ID3D11DeviceContext* immediate_context)
	{
		immediate_context->PSSetShader(default_shader_object.Get(), 0, 0);
	}
private:
	Microsoft::WRL::ComPtr<ID3D11PixelShader> default_shader_object;
};
