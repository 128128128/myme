#pragma once

#include <d3d11.h>
#include <wrl.h>
#include <assert.h>
#include "../../Functions/misc.h"
#include "../../Functions/utility_graphics.h"
#define USAGE_DYNAMIC
namespace Descartes
{
	//https://docs.microsoft.com/en-us/windows/desktop/direct3dhlsl/dx-graphics-hlsl-constants
	//https://docs.microsoft.com/en-us/windows/desktop/dxtecharts/direct3d10-frequently-asked-questions
	//https://developer.nvidia.com/content/constant-buffers-without-constant-pain-0

	template <class T>
	struct constant_buffer
	{
		T data;
		constant_buffer(ID3D11Device *device)
		{
			_ASSERT_EXPR(sizeof(T) % 16 == 0, L"constant buffer's need to be 16 byte aligned");
			HRESULT hr = S_OK;
			D3D11_BUFFER_DESC buffer_desc = {};
			buffer_desc.ByteWidth = sizeof(T);
			buffer_desc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
			buffer_desc.MiscFlags = 0;
			buffer_desc.StructureByteStride = 0;
#ifdef USAGE_DYNAMIC
			//https://docs.microsoft.com/en-us/windows/desktop/direct3d11/how-to--use-dynamic-resources
			buffer_desc.Usage = D3D11_USAGE_DYNAMIC;
			buffer_desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
			hr = device->CreateBuffer(&buffer_desc, 0, buffer_object.GetAddressOf());
			_ASSERT_EXPR(SUCCEEDED(hr), hr_trace(hr));
#else
			buffer_desc.Usage = D3D11_USAGE_DEFAULT;
			buffer_desc.CPUAccessFlags = 0;
			D3D11_SUBRESOURCE_DATA subresource_data;
			subresource_data.pSysMem = &data;
			subresource_data.SysMemPitch = 0;
			subresource_data.SysMemSlicePitch = 0;
			hr = device->CreateBuffer(&buffer_desc, &subresource_data, buffer_object.GetAddressOf());
			_ASSERT_EXPR(SUCCEEDED(hr), hr_trace(hr));
#endif
		}
		virtual ~constant_buffer() = default;
		constant_buffer(constant_buffer &) = delete;
		constant_buffer &operator =(constant_buffer &) = delete;

		void active(ID3D11DeviceContext *immediate_context, int slot, bool vs_on = true, bool ps_on = true,bool cs_on=false,bool gs_on=false,bool hs_on=false,bool ds_on=false)
		{
			HRESULT hr = S_OK;
#ifdef USAGE_DYNAMIC
			D3D11_MAP map = D3D11_MAP_WRITE_DISCARD;
			D3D11_MAPPED_SUBRESOURCE mapped_buffer;
			
			hr = immediate_context->Map(buffer_object.Get(), 0, map, 0, &mapped_buffer);
			_ASSERT_EXPR(SUCCEEDED(hr), hr_trace(hr));
			memcpy_s(mapped_buffer.pData, sizeof(T), &data, sizeof(T));
			immediate_context->Unmap(buffer_object.Get(), 0);
#else
			immediate_context->UpdateSubresource(buffer_object.Get(), 0, 0, &data, 0, 0);
#endif
			if (vs_on)
			{
				immediate_context->VSSetConstantBuffers(slot, 1, buffer_object.GetAddressOf());
			}
			if (ps_on)
			{
				immediate_context->PSSetConstantBuffers(slot, 1, buffer_object.GetAddressOf());
			}
			if(cs_on)
			{
				immediate_context->CSSetConstantBuffers(slot, 1, buffer_object.GetAddressOf());
			}
			if(gs_on)
			{
				immediate_context->GSGetConstantBuffers(slot, 1, buffer_object.GetAddressOf());
			}
			if(hs_on)
			{
				immediate_context->HSGetConstantBuffers(slot, 1, buffer_object.GetAddressOf());
			}
			if(ds_on)
			{
				immediate_context->DSGetConstantBuffers(slot, 1, buffer_object.GetAddressOf());
			}
		}
		void inactive(ID3D11DeviceContext *immediate_context)
		{
			// NOP
		}
	
		Microsoft::WRL::ComPtr<ID3D11Buffer> buffer_object;

	};
}
