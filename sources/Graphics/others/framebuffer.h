#pragma once

#include <d3d11.h>
#include <wrl.h>
#include <assert.h>
#include <memory>

#include "shader.h"

#include "../../Functions/misc.h"

namespace Descartes
{
	class framebuffer
	{
	public:
		//ˆê——
		//'render_target_texture2d_format' : DXGI_FORMAT_R8G8B8A8_UNORM / DXGI_FORMAT_R16G16B16A16_FLOAT / DXGI_FORMAT_R32G32B32A32_FLOAT / DXGI_FORMAT_UNKNOWN(not create)
		//'depth_stencil_texture2d_format' : DXGI_FORMAT_R24G8_TYPELESS / DXGI_FORMAT_R32_TYPELESS / DXGI_FORMAT_R16_TYPELESS / DXGI_FORMAT_UNKNOWN(not create)

		framebuffer(ID3D11Device* device, int width, int height, DXGI_FORMAT render_target_texture2d_format = DXGI_FORMAT_R8G8B8A8_UNORM, DXGI_FORMAT depth_stencil_texture2d_format = DXGI_FORMAT_R24G8_TYPELESS,
			bool need_render_target_shader_resource_view = true, bool need_depth_stencil_shader_resource_view = true);

		virtual ~framebuffer() = default;
		framebuffer(framebuffer&) = delete;
		framebuffer& operator=(framebuffer&) = delete;

		Microsoft::WRL::ComPtr<ID3D11RenderTargetView> render_target_view;
		Microsoft::WRL::ComPtr<ID3D11DepthStencilView> depth_stencil_view;

		Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> render_target_shader_resource_view;
		Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> depth_stencil_shader_resource_view;

		D3D11_VIEWPORT viewport;

		void clear(ID3D11DeviceContext* immediate_context, float r = 0, float g = 0, float b = 0, float a = 1, std::uint32_t clear_flags = D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, float depth = 1, uint8_t stencil = 0)
		{
			float color[4] = { r, g, b, a };
			if (render_target_view)
			{
				immediate_context->ClearRenderTargetView(render_target_view.Get(), color);
			}
			if (depth_stencil_view)
			{
				immediate_context->ClearDepthStencilView(depth_stencil_view.Get(), clear_flags, depth, stencil);
			}
		}
		//clear only 'render_target_view'
		void clear_render_target_view(ID3D11DeviceContext* immediate_context, float r = 0, float g = 0, float b = 0, float a = 1)
		{
			float color[4] = { r, g, b, a };
			immediate_context->ClearRenderTargetView(render_target_view.Get(), color);
		}
		//only clear 'depth_stencil_view'
		void clear_depth_stencil_view(ID3D11DeviceContext* immediate_context, UINT clear_flags = D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, FLOAT depth = 1, UINT8 stencil = 0)
		{
			immediate_context->ClearDepthStencilView(depth_stencil_view.Get(), clear_flags, depth, stencil);
		}
		void active(ID3D11DeviceContext* immediate_context)
		{
			number_of_stored_viewports = D3D11_VIEWPORT_AND_SCISSORRECT_OBJECT_COUNT_PER_PIPELINE;
			immediate_context->RSGetViewports(&number_of_stored_viewports, default_viewports);
			immediate_context->RSSetViewports(1, &viewport);

			immediate_context->OMGetRenderTargets(1, default_render_target_view.ReleaseAndGetAddressOf(), default_depth_stencil_view.ReleaseAndGetAddressOf());
			immediate_context->OMSetRenderTargets(1, render_target_view.GetAddressOf(), depth_stencil_view.Get());
		}
		//active only 'render_target_view'
		void active_render_target_view(ID3D11DeviceContext* immediate_context)
		{
			number_of_stored_viewports = D3D11_VIEWPORT_AND_SCISSORRECT_OBJECT_COUNT_PER_PIPELINE;
			immediate_context->RSGetViewports(&number_of_stored_viewports, default_viewports);
			immediate_context->RSSetViewports(1, &viewport);

			immediate_context->OMGetRenderTargets(1, default_render_target_view.ReleaseAndGetAddressOf(), default_depth_stencil_view.ReleaseAndGetAddressOf());
			immediate_context->OMSetRenderTargets(1, render_target_view.GetAddressOf(), 0);
		}
		//active only 'depth_stencil_view'
		void active_depth_stencil_view(ID3D11DeviceContext* immediate_context)
		{
			number_of_stored_viewports = D3D11_VIEWPORT_AND_SCISSORRECT_OBJECT_COUNT_PER_PIPELINE;
			immediate_context->RSGetViewports(&number_of_stored_viewports, default_viewports);
			immediate_context->RSSetViewports(1, &viewport);

			immediate_context->OMGetRenderTargets(1, default_render_target_view.ReleaseAndGetAddressOf(), default_depth_stencil_view.ReleaseAndGetAddressOf());
			ID3D11RenderTargetView* null_render_target_view = 0;
			immediate_context->OMSetRenderTargets(1, &null_render_target_view, depth_stencil_view.Get());
		}
		void inactive(ID3D11DeviceContext* immediate_context)
		{
			immediate_context->RSSetViewports(number_of_stored_viewports, default_viewports);
			immediate_context->OMSetRenderTargets(1, default_render_target_view.GetAddressOf(), default_depth_stencil_view.Get());
		}
	private:
		UINT number_of_stored_viewports;
		D3D11_VIEWPORT default_viewports[D3D11_VIEWPORT_AND_SCISSORRECT_OBJECT_COUNT_PER_PIPELINE];

		Microsoft::WRL::ComPtr<ID3D11RenderTargetView> default_render_target_view;
		Microsoft::WRL::ComPtr<ID3D11DepthStencilView> default_depth_stencil_view;
	};


}
