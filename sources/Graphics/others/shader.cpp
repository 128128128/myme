#include <d3dcompiler.h>
#include <memory>
#include <map>

#include "shader.h"

#include <unordered_map>

#include "../../Functions/misc.h"
using namespace std;

struct set_of_vertex_shader_and_input_layout
{
	set_of_vertex_shader_and_input_layout(ID3D11VertexShader* vertex_shader, ID3D11InputLayout* input_layout) : vertex_shader(vertex_shader), input_layout(input_layout) {}
	Microsoft::WRL::ComPtr<ID3D11VertexShader> vertex_shader;
	Microsoft::WRL::ComPtr<ID3D11InputLayout> input_layout;
};
static std::map<std::string, set_of_vertex_shader_and_input_layout> cached_vertex_shaders;

//create vertex shader...頂点シェーダーをcsoから作る
HRESULT create_vs_from_cso(ID3D11Device* device, const char* cso_name, ID3D11VertexShader** vertex_shader, ID3D11InputLayout** input_layout, D3D11_INPUT_ELEMENT_DESC* input_element_desc, UINT num_elements)
{
	FILE* fp{ nullptr };
	fopen_s(&fp, cso_name, "rb");
	_ASSERT_EXPR_A(fp, "CSO File not found");

	fseek(fp, 0, SEEK_END);
	long cso_sz{ ftell(fp) };
	fseek(fp, 0, SEEK_SET);

	unique_ptr<unsigned char[]> cso_data{ make_unique<unsigned char[]>(cso_sz) };
	fread(cso_data.get(), cso_sz, 1, fp);
	fclose(fp);

	HRESULT hr{ S_OK };
	hr = device->CreateVertexShader(cso_data.get(), cso_sz, nullptr, vertex_shader);
	_ASSERT_EXPR(SUCCEEDED(hr), hr_trace(hr));

	if (input_layout)
	{
		hr = device->CreateInputLayout(input_element_desc, num_elements, cso_data.get(), cso_sz, input_layout);
		_ASSERT_EXPR(SUCCEEDED(hr), hr_trace(hr));
	}

	return hr;
}

HRESULT create_vs_from_source(ID3D11Device* device, const char* key_name, const std::string& source, const char* entry_point, ID3D11VertexShader** vertex_shader, ID3D11InputLayout** input_layout, D3D11_INPUT_ELEMENT_DESC* input_element_desc, size_t num_elements, bool enable_caching)
{
	std::map<std::string, set_of_vertex_shader_and_input_layout>::iterator it = cached_vertex_shaders.find(key_name);
	if (it != cached_vertex_shaders.end())
	{
		*vertex_shader = it->second.vertex_shader.Get();
		(*vertex_shader)->AddRef();
		if (input_layout)
		{
			*input_layout = it->second.input_layout.Get();
			_ASSERT_EXPR_A(*input_layout, "cached input_layout must be not nil");
			(*input_layout)->AddRef();
		}
		return S_OK;
	}
	HRESULT hr = S_OK;
	UINT flags = D3DCOMPILE_ENABLE_STRICTNESS;
#if defined( DEBUG ) || defined( _DEBUG )
	flags |= D3DCOMPILE_DEBUG;
	flags |= D3D10_SHADER_SKIP_OPTIMIZATION;
#endif
	Microsoft::WRL::ComPtr<ID3DBlob> compiled_shader_blob;
	Microsoft::WRL::ComPtr<ID3DBlob> error_message_blob;
	hr = D3DCompile(source.c_str(), source.length(), 0, 0, 0, entry_point, "vs_5_0", flags, 0, compiled_shader_blob.GetAddressOf(), error_message_blob.GetAddressOf());
	_ASSERT_EXPR_A(SUCCEEDED(hr), reinterpret_cast<LPCSTR>(error_message_blob->GetBufferPointer()));
	hr = device->CreateVertexShader(compiled_shader_blob->GetBufferPointer(), compiled_shader_blob->GetBufferSize(), 0, vertex_shader);
	_ASSERT_EXPR_A(SUCCEEDED(hr), reinterpret_cast<LPCSTR>(error_message_blob->GetBufferPointer()));

	if (input_layout)
	{
		hr = device->CreateInputLayout(input_element_desc, num_elements, compiled_shader_blob->GetBufferPointer(), compiled_shader_blob->GetBufferSize(), input_layout);
		_ASSERT_EXPR_A(SUCCEEDED(hr), reinterpret_cast<LPCSTR>(error_message_blob->GetBufferPointer()));
	}

	if (enable_caching)
	{
		cached_vertex_shaders.insert(std::make_pair(key_name, set_of_vertex_shader_and_input_layout(*vertex_shader, input_layout ? *input_layout : 0)));
	}

	return hr;
}

void release_all_cached_vertex_shaders()
{
	cached_vertex_shaders.clear();
}

static std::map<std::string, Microsoft::WRL::ComPtr<ID3D11PixelShader>> cached_pixel_shaders;
//create vertex shader..ピクセルシェーダーをcsoから作る
HRESULT create_ps_from_cso(ID3D11Device* device, const char* cso_name, ID3D11PixelShader** pixel_shader)
{
	FILE* fp{ nullptr };
	fopen_s(&fp, cso_name, "rb");
	_ASSERT_EXPR_A(fp, "CSO File not found");

	fseek(fp, 0, SEEK_END);
	long cso_sz{ ftell(fp) };
	fseek(fp, 0, SEEK_SET);

	unique_ptr<unsigned char[]> cso_data{ make_unique<unsigned char[]>(cso_sz) };
	fread(cso_data.get(), cso_sz, 1, fp);
	fclose(fp);

	HRESULT hr{ S_OK };
	hr = device->CreatePixelShader(cso_data.get(), cso_sz, nullptr, pixel_shader);
	_ASSERT_EXPR(SUCCEEDED(hr), hr_trace(hr));

	return hr;
}

HRESULT create_ps_from_source(ID3D11Device* device, const char* key_name, const std::string& source, const char* entry_point, ID3D11PixelShader** pixel_shader, bool enable_caching)
{
	std::map<std::string, Microsoft::WRL::ComPtr<ID3D11PixelShader>>::iterator it = cached_pixel_shaders.find(key_name);
	if (it != cached_pixel_shaders.end())
	{
		*pixel_shader = it->second.Get();
		(*pixel_shader)->AddRef();
		return S_OK;
	}

	HRESULT hr = S_OK;
	UINT flags = D3DCOMPILE_ENABLE_STRICTNESS;
#if defined( DEBUG ) || defined( _DEBUG )
	flags |= D3DCOMPILE_DEBUG;
	flags |= D3D10_SHADER_SKIP_OPTIMIZATION;
#endif
	Microsoft::WRL::ComPtr<ID3DBlob> compiled_shader_blob;
	Microsoft::WRL::ComPtr<ID3DBlob> error_message_blob;
	hr = D3DCompile(source.c_str(), source.length(), 0, 0, 0, entry_point, "ps_5_0", flags, 0, compiled_shader_blob.GetAddressOf(), error_message_blob.GetAddressOf());
	_ASSERT_EXPR_A(SUCCEEDED(hr), reinterpret_cast<LPCSTR>(error_message_blob->GetBufferPointer()));
	hr = device->CreatePixelShader(compiled_shader_blob->GetBufferPointer(), compiled_shader_blob->GetBufferSize(), 0, pixel_shader);
	_ASSERT_EXPR_A(SUCCEEDED(hr), reinterpret_cast<LPCSTR>(error_message_blob->GetBufferPointer()));

	if (enable_caching)
	{
		cached_pixel_shaders.insert(std::make_pair(key_name, *pixel_shader));
	}

	return hr;
}

//all pixel shader delete
void release_all_cached_pixel_shaders()
{
	cached_pixel_shaders.clear();
}

//domain shader create
static unordered_map<std::string, Microsoft::WRL::ComPtr<ID3D11DomainShader>> managed_domain_shaders;
HRESULT create_ds_from_cso(ID3D11Device* device, const char* cso_name, ID3D11DomainShader** domain_shader)
{
	std::unordered_map<std::string, Microsoft::WRL::ComPtr<ID3D11DomainShader>>::iterator managed_domain_shader{ managed_domain_shaders.find(cso_name) };
	if (managed_domain_shader != managed_domain_shaders.end())
	{
		*domain_shader = managed_domain_shader->second.Get();
		(*domain_shader)->AddRef();
		return S_OK;
	}

	FILE* fp = nullptr;
	fopen_s(&fp, cso_name, "rb");
	_ASSERT_EXPR_A(fp, "CSO File not found");

	fseek(fp, 0, SEEK_END);
	long cso_sz = ftell(fp);
	fseek(fp, 0, SEEK_SET);

	std::unique_ptr<unsigned char[]> cso_data = std::make_unique<unsigned char[]>(cso_sz);
	fread(cso_data.get(), cso_sz, 1, fp);
	fclose(fp);

	HRESULT hr = device->CreateDomainShader(cso_data.get(), cso_sz, nullptr, domain_shader);
	_ASSERT_EXPR(SUCCEEDED(hr), hr_trace(hr));

	managed_domain_shaders.insert(std::make_pair(cso_name, *domain_shader));

	return hr;
}

//geometory shader create
static std::unordered_map<std::string, Microsoft::WRL::ComPtr<ID3D11GeometryShader>> managed_geometry_shaders;
HRESULT create_gs_from_cso(ID3D11Device* device, const char* cso_name, ID3D11GeometryShader** geometry_shader)
{
	std::unordered_map<std::string, Microsoft::WRL::ComPtr<ID3D11GeometryShader>>::iterator managed_geometry_shader{ managed_geometry_shaders.find(cso_name) };
	if (managed_geometry_shader != managed_geometry_shaders.end())
	{
		*geometry_shader = managed_geometry_shader->second.Get();
		(*geometry_shader)->AddRef();
		return S_OK;
	}

	FILE* fp = nullptr;
	fopen_s(&fp, cso_name, "rb");
	_ASSERT_EXPR_A(fp, "CSO File not found");

	fseek(fp, 0, SEEK_END);
	long cso_sz = ftell(fp);
	fseek(fp, 0, SEEK_SET);

	std::unique_ptr<unsigned char[]> cso_data = std::make_unique<unsigned char[]>(cso_sz);
	fread(cso_data.get(), cso_sz, 1, fp);
	fclose(fp);

	HRESULT hr = device->CreateGeometryShader(cso_data.get(), cso_sz, nullptr, geometry_shader);
	_ASSERT_EXPR(SUCCEEDED(hr), hr_trace(hr));

	managed_geometry_shaders.insert(std::make_pair(cso_name, *geometry_shader));

	return hr;
}

//compute shader create
static std::unordered_map<std::string, Microsoft::WRL::ComPtr<ID3D11ComputeShader>> managed_compute_shaders;
HRESULT create_cs_from_cso(ID3D11Device* device, const char* cso_name, ID3D11ComputeShader** compute_shader)
{
	std::unordered_map<std::string, Microsoft::WRL::ComPtr<ID3D11ComputeShader>>::iterator managed_compute_shader{ managed_compute_shaders.find(cso_name) };
	if (managed_compute_shader != managed_compute_shaders.end())
	{
		*compute_shader = managed_compute_shader->second.Get();
		(*compute_shader)->AddRef();
		return S_OK;
	}

	FILE* fp = nullptr;
	fopen_s(&fp, cso_name, "rb");
	_ASSERT_EXPR_A(fp, "CSO File not found");

	fseek(fp, 0, SEEK_END);
	long cso_sz = ftell(fp);
	fseek(fp, 0, SEEK_SET);

	std::unique_ptr<unsigned char[]> cso_data = std::make_unique<unsigned char[]>(cso_sz);
	fread(cso_data.get(), cso_sz, 1, fp);
	fclose(fp);

	HRESULT hr = device->CreateComputeShader(cso_data.get(), cso_sz, nullptr, compute_shader);
	_ASSERT_EXPR(SUCCEEDED(hr), hr_trace(hr));

	managed_compute_shaders.insert(std::make_pair(cso_name, *compute_shader));

	return hr;
}

//hull shader create
HRESULT create_hs_from_cso(ID3D11Device* device, const char* cso_name, ID3D11HullShader** hull_shader)
{
	FILE* fp = nullptr;
	fopen_s(&fp, cso_name, "rb");
	_ASSERT_EXPR_A(fp, "CSO File not found");

	fseek(fp, 0, SEEK_END);
	long cso_sz = ftell(fp);
	fseek(fp, 0, SEEK_SET);

	std::unique_ptr<unsigned char[]> cso_data = std::make_unique<unsigned char[]>(cso_sz);
	fread(cso_data.get(), cso_sz, 1, fp);
	fclose(fp);

	HRESULT hr = device->CreateHullShader(cso_data.get(), cso_sz, nullptr, hull_shader);
	_ASSERT_EXPR(SUCCEEDED(hr), hr_trace(hr));

	return hr;
}