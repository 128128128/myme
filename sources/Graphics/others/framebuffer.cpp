#include "framebuffer.h"
#include "../../Functions/utility_graphics.h"

namespace Descartes
{
	framebuffer::framebuffer(ID3D11Device* device, int width, int height, DXGI_FORMAT render_target_texture2d_format, DXGI_FORMAT depth_stencil_texture2d_format,
		bool need_render_target_shader_resource_view, bool need_depth_stencil_shader_resource_view)
	{
		HRESULT hr = S_OK;

		Microsoft::WRL::ComPtr<ID3D11Texture2D> render_target_buffer;
			D3D11_TEXTURE2D_DESC texture2d_desc = {};
			texture2d_desc.Width = width;
			texture2d_desc.Height = height;
			texture2d_desc.MipLevels = 1;
			texture2d_desc.ArraySize = 1;
			texture2d_desc.Format = render_target_texture2d_format;
			texture2d_desc.SampleDesc.Count = 1;
			texture2d_desc.SampleDesc.Quality = 0;
			texture2d_desc.Usage = D3D11_USAGE_DEFAULT;
			texture2d_desc.BindFlags = need_render_target_shader_resource_view ? D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE : D3D11_BIND_RENDER_TARGET;
			texture2d_desc.CPUAccessFlags = 0;
			texture2d_desc.MiscFlags = 0;

			hr = device->CreateTexture2D(&texture2d_desc, 0, render_target_buffer.GetAddressOf());
			_ASSERT_EXPR(SUCCEEDED(hr), hr_trace(hr));

			D3D11_RENDER_TARGET_VIEW_DESC render_target_view_desc = {};
			render_target_view_desc.Format = texture2d_desc.Format;
			render_target_view_desc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2D;
			hr = device->CreateRenderTargetView(render_target_buffer.Get(), &render_target_view_desc, render_target_view.GetAddressOf());
			_ASSERT_EXPR(SUCCEEDED(hr), hr_trace(hr));

			if (need_render_target_shader_resource_view)
			{
				D3D11_SHADER_RESOURCE_VIEW_DESC shader_resource_view_desc = {};
				shader_resource_view_desc.Format = texture2d_desc.Format;
				shader_resource_view_desc.ViewDimension =  D3D11_SRV_DIMENSION_TEXTURE2D;
				shader_resource_view_desc.Texture2D.MipLevels = 1;
				hr = device->CreateShaderResourceView(render_target_buffer.Get(), &shader_resource_view_desc, render_target_shader_resource_view.GetAddressOf());
				_ASSERT_EXPR(SUCCEEDED(hr), hr_trace(hr));
			}

			/// depthstencil fomat ˆê——
			Microsoft::WRL::ComPtr<ID3D11Texture2D> depth_stencil_buffer;
			const DXGI_FORMAT combinations_of_depth_stencil_formats[3][3] =
			{
				{ DXGI_FORMAT_R24G8_TYPELESS,  DXGI_FORMAT_D24_UNORM_S8_UINT ,DXGI_FORMAT_R24_UNORM_X8_TYPELESS },
				{ DXGI_FORMAT_R32_TYPELESS,  DXGI_FORMAT_D32_FLOAT, DXGI_FORMAT_R32_FLOAT },
				{ DXGI_FORMAT_R16_TYPELESS,  DXGI_FORMAT_D16_UNORM , DXGI_FORMAT_R16_UNORM },
			};
			int depth_stencil_texture2d_format_index = 0;
			switch (depth_stencil_texture2d_format)
			{
			case DXGI_FORMAT_R24G8_TYPELESS:
				depth_stencil_texture2d_format_index = 0;
				break;
			case DXGI_FORMAT_R32_TYPELESS:
				depth_stencil_texture2d_format_index = 1;
				break;
			case DXGI_FORMAT_R16_TYPELESS:
				depth_stencil_texture2d_format_index = 2;
				break;
			} 
			texture2d_desc.MipLevels = 1;
			texture2d_desc.Format = combinations_of_depth_stencil_formats[depth_stencil_texture2d_format_index][0];
			texture2d_desc.BindFlags = need_depth_stencil_shader_resource_view ? D3D11_BIND_DEPTH_STENCIL | D3D11_BIND_SHADER_RESOURCE : D3D11_BIND_DEPTH_STENCIL;
			texture2d_desc.MiscFlags = 0;
			hr = device->CreateTexture2D(&texture2d_desc, 0, depth_stencil_buffer.GetAddressOf());
			_ASSERT_EXPR(SUCCEEDED(hr), hr_trace(hr));

			D3D11_DEPTH_STENCIL_VIEW_DESC depth_stencil_view_desc = {};
			depth_stencil_view_desc.Format = combinations_of_depth_stencil_formats[depth_stencil_texture2d_format_index][1];
			depth_stencil_view_desc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
			depth_stencil_view_desc.Flags = 0;
			hr = device->CreateDepthStencilView(depth_stencil_buffer.Get(), &depth_stencil_view_desc, depth_stencil_view.GetAddressOf());
			_ASSERT_EXPR(SUCCEEDED(hr), hr_trace(hr));

			if (need_depth_stencil_shader_resource_view)
			{
				D3D11_SHADER_RESOURCE_VIEW_DESC shader_resource_view_desc = {};
				shader_resource_view_desc.Format = combinations_of_depth_stencil_formats[depth_stencil_texture2d_format_index][2];
				shader_resource_view_desc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
				shader_resource_view_desc.Texture2D.MipLevels = 1;
				hr = device->CreateShaderResourceView(depth_stencil_buffer.Get(), &shader_resource_view_desc, depth_stencil_shader_resource_view.GetAddressOf());
				_ASSERT_EXPR(SUCCEEDED(hr), hr_trace(hr));
			}
		
		viewport.Width = static_cast<float>(width);
		viewport.Height = static_cast<float>(height);
		viewport.MinDepth = 0.0f;
		viewport.MaxDepth = 1.0f;
		viewport.TopLeftX = 0.0f;
		viewport.TopLeftY = 0.0f;
	}

}
