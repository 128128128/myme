#include <DirectXTex.h>

#include "texture.h"

#include <filesystem>
#include <mutex>
#include <Shlwapi.h>

#include "../../Functions/misc.h"

#include <WICTextureLoader.h>

#include "DDSTextureLoader.h"
using namespace DirectX;

#include <wrl.h>
using namespace Microsoft::WRL;

#include <string>
#include <map>
using namespace std;

#include <sstream>
#include <iomanip>


//HRESULT load_texture_from_file(ID3D11Device* device, const wchar_t* filename, ID3D11ShaderResourceView** shader_resource_view, D3D11_TEXTURE2D_DESC* texture2d_desc)
//{
//	HRESULT hr{ S_OK };
//	ComPtr<ID3D11Resource> resource;
//
//	std::filesystem::path dds_filename(filename);
//	dds_filename.replace_extension("dds");
//	if (std::filesystem::exists(dds_filename.c_str()))
//	{
//		Microsoft::WRL::ComPtr<ID3D11DeviceContext> immediate_context;
//		device->GetImmediateContext(immediate_context.GetAddressOf());
//		hr = DirectX::CreateDDSTextureFromFile(device, immediate_context.Get(), dds_filename.c_str(),
//			resource.GetAddressOf(), shader_resource_view);
//		_ASSERT_EXPR(SUCCEEDED(hr), hr_trace(hr));
//	}
//	else
//	{
//		hr = CreateWICTextureFromFile(device, filename, resource.GetAddressOf(), shader_resource_view);
//		_ASSERT_EXPR(SUCCEEDED(hr), hr_trace(hr));
//		resources.insert(make_pair(filename, *shader_resource_view));
//	}
//
//	ComPtr<ID3D11Texture2D> texture2d;
//	hr = resource.Get()->QueryInterface<ID3D11Texture2D>(texture2d.GetAddressOf());
//	_ASSERT_EXPR(SUCCEEDED(hr), hr_trace(hr));
//	texture2d->GetDesc(texture2d_desc);
//
//	return hr;
//}

//cached textures
static map<wstring, ComPtr<ID3D11ShaderResourceView>> texture_caches;
std::mutex cached_textures_mutex;
//texture load...テクスチャロード
void load_texture_from_file(ID3D11Device* device, const char* filename, ID3D11ShaderResourceView** shader_resource_view, bool force_srgb, bool enable_caching)
{
	HRESULT hr{ S_OK };
	
	wchar_t filename_w[256];
	MultiByteToWideChar(CP_ACP, 0, filename, static_cast<int>(strlen(filename) + 1), filename_w, 256);

	std::map<std::wstring, Microsoft::WRL::ComPtr<ID3D11ShaderResourceView>>::iterator it = texture_caches.find(filename_w);
	if (it != texture_caches.end())
	{
		*shader_resource_view = it->second.Get();
		(*shader_resource_view)->AddRef();
	}
	else
	{

		DirectX::TexMetadata metadata;
		DirectX::ScratchImage image;

		
		std::wstring extension = PathFindExtensionW(filename_w);
		std::transform(extension.begin(), extension.end(), extension.begin(), ::towlower);

		//WIC includes several built - in codecs.The following standard codecs are provided with the platform.
		//	Codec																	Mime Types								Decoders	Encoders
		//	BMP(Windows Bitmap Format), BMP Specification v5.						image / bmp								Yes			Yes
		//	GIF(Graphics Interchange Format 89a), GIF Specification 89a / 89m		image / gif								Yes			Yes
		//	ICO(Icon Format)														image / ico								Yes			No
		//	JPEG(Joint Photographic Experts Group), JFIF Specification 1.02			image / jpeg, image / jpe, image / jpg	Yes			Yes
		//	PNG(Portable Network Graphics), PNG Specification 1.2					image / png								Yes			Yes
		//	TIFF(Tagged Image File Format), TIFF Specification 6.0					image / tiff, image / tif				Yes			Yes
		//	Windows Media Photo, HD Photo Specification 1.0							image / vnd.ms - phot					Yes			Yes
		//	DDS(DirectDraw Surface)													image / vnd.ms - dds					Yes			Yes
		if (L".png" == extension || L".jpeg" == extension || L".jpg" == extension || L".jpe" == extension || L".gif" == extension || L".tiff" == extension || L".tif" == extension || L".bmp" == extension)
		{
			hr = DirectX::LoadFromWICFile(filename_w, 0, &metadata, image);
			_ASSERT_EXPR(SUCCEEDED(hr), hr_trace(hr));
		}
		else if (L".dds" == extension)
		{
			hr = DirectX::LoadFromDDSFile(filename_w, 0, &metadata, image);
			_ASSERT_EXPR(SUCCEEDED(hr), hr_trace(hr));
		}
		else if (L".tga" == extension || L".vda" == extension || L".icb" == extension || L".vst" == extension)
		{
			hr = DirectX::LoadFromTGAFile(filename_w, &metadata, image);
			_ASSERT_EXPR(SUCCEEDED(hr), hr_trace(hr));
		}
		else
		{
			_ASSERT_EXPR(false, L"File format not supported.");
		}

		hr = DirectX::CreateShaderResourceViewEx(
			device,
			image.GetImages(),
			image.GetImageCount(),
			metadata,
			D3D11_USAGE_DEFAULT,
			D3D11_BIND_SHADER_RESOURCE,
			0,
			D3D11_RESOURCE_MISC_TEXTURECUBE,
			force_srgb,
			shader_resource_view);
		_ASSERT_EXPR(SUCCEEDED(hr), hr_trace(hr));

		if (enable_caching)
		{
			texture_caches.insert(std::make_pair(filename_w, *shader_resource_view));
		}

		/*std::filesystem::path dds_filename(filename);
		dds_filename.replace_extension("dds");
		if (std::filesystem::exists(dds_filename.c_str()))
		{
			Microsoft::WRL::ComPtr<ID3D11DeviceContext> immediate_context;
			device->GetImmediateContext(immediate_context.GetAddressOf());
			hr = DirectX::CreateDDSTextureFromFile(device, immediate_context.Get(), dds_filename.c_str(),
				resource.GetAddressOf(), shader_resource_view);
			_ASSERT_EXPR(SUCCEEDED(hr), hr_trace(hr));
		}
		else
		{
			hr = CreateWICTextureFromFile(device, filename_w, resource.GetAddressOf(), shader_resource_view);
			_ASSERT_EXPR(SUCCEEDED(hr), hr_trace(hr));
			texture_caches.insert(make_pair(filename_w, *shader_resource_view));
		}

		ComPtr<ID3D11Texture2D> texture2d;
		hr = resource.Get()->QueryInterface<ID3D11Texture2D>(texture2d.GetAddressOf());
		_ASSERT_EXPR(SUCCEEDED(hr), hr_trace(hr));
		texture2d->GetDesc(texture2d_desc);

		return hr;*/
	}
}

void make_dummy_texture_for_substance(ID3D11Device* device, ID3D11ShaderResourceView** shader_resource_view, DWORD value/*0xAABBGGRR*/, UINT dimensions, bool force_srgb, bool enable_caching)
{
	std::stringstream filename;
	filename << "dummy_texture" << "." << std::hex << std::uppercase << value << "." << dimensions << "." << force_srgb;

	wchar_t filename_w[256];
	MultiByteToWideChar(CP_ACP, 0, filename.str().c_str(), static_cast<int>(filename.str().length() + 1), filename_w, 256);

	std::map<std::wstring, Microsoft::WRL::ComPtr<ID3D11ShaderResourceView>>::iterator it = texture_caches.find(filename_w);
	if (it != texture_caches.end())
	{
		*shader_resource_view = it->second.Get();
		(*shader_resource_view)->AddRef();
	}
	else
	{
		HRESULT hr = S_OK;

		D3D11_TEXTURE2D_DESC texture2d_desc = {};
		texture2d_desc.Width = dimensions;
		texture2d_desc.Height = dimensions;
		texture2d_desc.MipLevels = 1;
		texture2d_desc.ArraySize = 1;
		texture2d_desc.Format = force_srgb ? DXGI_FORMAT_R8G8B8A8_UNORM_SRGB : DXGI_FORMAT_R8G8B8A8_UNORM;
		texture2d_desc.SampleDesc.Count = 1;
		texture2d_desc.SampleDesc.Quality = 0;
		texture2d_desc.Usage = D3D11_USAGE_DEFAULT;
		texture2d_desc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
		texture2d_desc.CPUAccessFlags = 0;
		texture2d_desc.MiscFlags = 0;

		DWORD* sysmem = new DWORD[dimensions * dimensions];
		for (size_t i = 0; i < dimensions * dimensions; i++)
		{
			sysmem[i] = value;
		}
		D3D11_SUBRESOURCE_DATA subresource_data = {};
		subresource_data.pSysMem = sysmem;
		subresource_data.SysMemPitch = sizeof(DWORD) * dimensions;
		subresource_data.SysMemSlicePitch = 0;

		ID3D11Texture2D* texture2d;
		hr = device->CreateTexture2D(&texture2d_desc, &subresource_data, &texture2d);
		_ASSERT_EXPR(SUCCEEDED(hr), hr_trace(hr));

		D3D11_SHADER_RESOURCE_VIEW_DESC shader_resource_view_desc = {};
		shader_resource_view_desc.Format = texture2d_desc.Format;
		shader_resource_view_desc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
		shader_resource_view_desc.Texture2D.MipLevels = 1;

		hr = device->CreateShaderResourceView(texture2d, &shader_resource_view_desc, shader_resource_view);
		_ASSERT_EXPR(SUCCEEDED(hr), hr_trace(hr));

		if (hr == S_OK && enable_caching)
		{
			texture_caches.insert(std::make_pair(filename_w, *shader_resource_view));
		}

		texture2d->Release();
		delete[] sysmem;
	}
}

void release_all_cached_textures()
{
	texture_caches.clear();
}
void release_cached_texture(const char* filename)
{
	wchar_t filename_w[256];
	MultiByteToWideChar(CP_ACP, 0, filename, static_cast<int>(strlen(filename) + 1), filename_w, 256);

	std::map<std::wstring, Microsoft::WRL::ComPtr<ID3D11ShaderResourceView>>::iterator it = texture_caches.find(filename_w);
	if (it != texture_caches.end())
	{
		texture_caches.erase(filename_w);
	}
}

HRESULT make_dummy_texture(ID3D11Device* device, ID3D11ShaderResourceView** shader_resource_view, DWORD value/*0xAABBGGRR*/, UINT dimension)
{
	HRESULT hr{ S_OK };

	wstringstream keyname;
	keyname << setw(8) << setfill(L'0') << hex << uppercase << value << L"." << dec << dimension;
	map<wstring, ComPtr<ID3D11ShaderResourceView>>::iterator it = texture_caches.find(keyname.str().c_str());
	if (it != texture_caches.end())
	{
		*shader_resource_view = it->second.Get();
		(*shader_resource_view)->AddRef();
	}
	else
	{
		D3D11_TEXTURE2D_DESC texture2d_desc{};
		texture2d_desc.Width = dimension;
		texture2d_desc.Height = dimension;
		texture2d_desc.MipLevels = 1;
		texture2d_desc.ArraySize = 1;
		texture2d_desc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
		texture2d_desc.SampleDesc.Count = 1;
		texture2d_desc.SampleDesc.Quality = 0;
		texture2d_desc.Usage = D3D11_USAGE_DEFAULT;
		texture2d_desc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
		texture2d_desc.CPUAccessFlags = 0;
		texture2d_desc.MiscFlags = 0;

		size_t texels = static_cast<size_t>(dimension) * dimension;
		unique_ptr<DWORD[]> sysmem{ make_unique< DWORD[]>(texels) };
		for (size_t i = 0; i < texels; ++i)
		{
			sysmem[i] = value;
		}

		D3D11_SUBRESOURCE_DATA subresource_data{};
		subresource_data.pSysMem = sysmem.get();
		subresource_data.SysMemPitch = sizeof(DWORD) * dimension;
		subresource_data.SysMemSlicePitch = 0;

		ComPtr<ID3D11Texture2D> texture2d;
		hr = device->CreateTexture2D(&texture2d_desc, &subresource_data, &texture2d);
		_ASSERT_EXPR(SUCCEEDED(hr), hr_trace(hr));

		D3D11_SHADER_RESOURCE_VIEW_DESC shader_resource_view_desc{};
		shader_resource_view_desc.Format = texture2d_desc.Format;
		shader_resource_view_desc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
		shader_resource_view_desc.Texture2D.MipLevels = 1;

		hr = device->CreateShaderResourceView(texture2d.Get(), &shader_resource_view_desc, shader_resource_view);
		_ASSERT_EXPR(SUCCEEDED(hr), hr_trace(hr));
		texture_caches.insert(std::make_pair(keyname.str().c_str(), *shader_resource_view));
	}
	return hr;
}

//texture description...テクスチャ概要取得
void texture2d_description(ID3D11ShaderResourceView* shader_resource_view, D3D11_TEXTURE2D_DESC& texture2d_desc)
{
	Microsoft::WRL::ComPtr<ID3D11Resource> resource;
	shader_resource_view->GetResource(resource.GetAddressOf());

	Microsoft::WRL::ComPtr<ID3D11Texture2D> texture2d;
	HRESULT hr = resource.Get()->QueryInterface<ID3D11Texture2D>(texture2d.GetAddressOf());
	_ASSERT_EXPR(SUCCEEDED(hr), hr_trace(hr));

	texture2d->GetDesc(&texture2d_desc);
}

D3D11_TEXTURE2D_DESC fetch_texture2d_description(ID3D11ShaderResourceView* shader_resource_view)
{
	D3D11_TEXTURE2D_DESC texture2d_desc;
	Microsoft::WRL::ComPtr<ID3D11Resource> resource;
	shader_resource_view->GetResource(resource.GetAddressOf());

	Microsoft::WRL::ComPtr<ID3D11Texture2D> texture2d;
	HRESULT hr = resource.Get()->QueryInterface<ID3D11Texture2D>(texture2d.GetAddressOf());
	_ASSERT_EXPR(SUCCEEDED(hr), hr_trace(hr));

	texture2d->GetDesc(&texture2d_desc);
	return texture2d_desc;
}