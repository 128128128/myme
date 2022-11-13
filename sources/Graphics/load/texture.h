#pragma once

#include <d3d11.h>

//HRESULT load_texture_from_file(ID3D11Device* device, const wchar_t* filename, ID3D11ShaderResourceView** shader_resource_view, D3D11_TEXTURE2D_DESC* texture2d_desc);
void load_texture_from_file(ID3D11Device* device, const char* filename, ID3D11ShaderResourceView** shader_resource_view, bool force_srgb, bool enable_caching);

void make_dummy_texture_for_substance(ID3D11Device* device, ID3D11ShaderResourceView** shader_resource_view, DWORD value/*0xAABBGGRR*/, UINT dimensions, bool force_srgb, bool enable_caching);
void release_all_cached_textures();
void release_cached_texture(const char* filename);

HRESULT make_dummy_texture(ID3D11Device* device, ID3D11ShaderResourceView** shader_resource_view, DWORD value/*0xAABBGGRR*/, UINT dimension);

void texture2d_description(ID3D11ShaderResourceView* shader_resource_view, D3D11_TEXTURE2D_DESC& texture2d_desc);

D3D11_TEXTURE2D_DESC fetch_texture2d_description(ID3D11ShaderResourceView* shader_resource_view);
