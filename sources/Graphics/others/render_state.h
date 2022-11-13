#pragma once
#include <d3d11.h>
#include <wrl.h>

#include "../../Functions/utility_graphics.h"
#include "../../Functions/misc.h"

enum class BLEND_MODE { NONE, ALPHA, ADD, SUBTRACT, REPLACE, MULTIPLY, LIGHTEN, DARKEN, SCREEN, ALPHA_TO_COVERAGE, END };
//NONE...nothing
//ALPHA...アルファ
//ADD...加算
//SUBTRACT...減算
//REPLACE...ReverseSubtract
//MULTIPLY...乗算
//LIGHTEN...比較(明)
//DARKEN...比較(暗)
//SCREEN...スクリーン
//ALPHA_TO_COVERAGE...アンチエイリアス

namespace Descartes
{
    ID3D11BlendState* create_blend_state(ID3D11Device* device, BLEND_MODE mode);

    class blend_state
    {
    public:
        Microsoft::WRL::ComPtr<ID3D11BlendState> state_obj;
        blend_state(ID3D11Device* device, BLEND_MODE mode)
        {
            state_obj.Attach(create_blend_state(device, mode));
        }
        blend_state(ID3D11Device* device, const D3D11_BLEND_DESC* blend_desc)
        {
            HRESULT hr = device->CreateBlendState(blend_desc, state_obj.GetAddressOf());
            _ASSERT_EXPR(SUCCEEDED(hr), hr_trace(hr));
        }
        ~blend_state() = default;
        blend_state(blend_state&) = delete;
        blend_state& operator =(blend_state&) = delete;

        void active(ID3D11DeviceContext* immediate_context)
        {
            UINT sample_mask = 0xFFFFFFFF;
            immediate_context->OMGetBlendState(state_default_obj.ReleaseAndGetAddressOf(), 0, &sample_mask);
            immediate_context->OMSetBlendState(state_obj.Get(), 0, 0xFFFFFFFF);
        }
        void inactive(ID3D11DeviceContext* immediate_context)
        {
            immediate_context->OMSetBlendState(state_default_obj.Get(), 0, 0xFFFFFFFF);
        }
    private:
        Microsoft::WRL::ComPtr<ID3D11BlendState> state_default_obj;
    };

	class rasterizer_state
	{
	public:
		Microsoft::WRL::ComPtr<ID3D11RasterizerState> state_obj;
		rasterizer_state(ID3D11Device* device, D3D11_FILL_MODE fill_mode = D3D11_FILL_SOLID, D3D11_CULL_MODE cull_mode = D3D11_CULL_BACK, bool front_counter_clockwise = true, bool antialiasing = true, bool depth_clip_enable = true)
		{
			D3D11_RASTERIZER_DESC rasterizer_desc = {};
			rasterizer_desc.FillMode = fill_mode;
			rasterizer_desc.CullMode = cull_mode;
			rasterizer_desc.FrontCounterClockwise = front_counter_clockwise;
			rasterizer_desc.DepthBias = D3D11_DEFAULT_DEPTH_BIAS;
			rasterizer_desc.DepthBiasClamp = D3D11_DEFAULT_DEPTH_BIAS_CLAMP;
			rasterizer_desc.SlopeScaledDepthBias = D3D11_DEFAULT_SLOPE_SCALED_DEPTH_BIAS;
			rasterizer_desc.DepthClipEnable = static_cast<BOOL>(depth_clip_enable);
			rasterizer_desc.ScissorEnable = FALSE;
			rasterizer_desc.MultisampleEnable = antialiasing && fill_mode == D3D11_FILL_SOLID ? TRUE : FALSE;
			rasterizer_desc.AntialiasedLineEnable = antialiasing ? TRUE : FALSE;
			HRESULT hr = device->CreateRasterizerState(&rasterizer_desc, state_obj.GetAddressOf());
			_ASSERT_EXPR(SUCCEEDED(hr), hr_trace(hr));
		}
		rasterizer_state(ID3D11Device* device, const D3D11_RASTERIZER_DESC* rasterizer_desc)
		{
			HRESULT hr = device->CreateRasterizerState(rasterizer_desc, state_obj.GetAddressOf());
			_ASSERT_EXPR(SUCCEEDED(hr), hr_trace(hr));
		}
		~rasterizer_state() = default;
		rasterizer_state(rasterizer_state&) = delete;
		rasterizer_state& operator =(rasterizer_state&) = delete;

		void active(ID3D11DeviceContext* immediate_context)
		{
			immediate_context->RSGetState(state_default_object.ReleaseAndGetAddressOf());
			immediate_context->RSSetState(state_obj.Get());
		}
		void inactive(ID3D11DeviceContext* immediate_context)
		{
			immediate_context->RSSetState(state_default_object.Get());
		}
	private:
		Microsoft::WRL::ComPtr<ID3D11RasterizerState> state_default_object;
	};

	class depth_stencil_state
	{
	public:
		Microsoft::WRL::ComPtr<ID3D11DepthStencilState> state_obj;
		depth_stencil_state(ID3D11Device* device, BOOL depth_enable = TRUE, D3D11_DEPTH_WRITE_MASK depth_write_mask = D3D11_DEPTH_WRITE_MASK_ALL, D3D11_COMPARISON_FUNC depth_func = D3D11_COMPARISON_LESS)
		{
			D3D11_DEPTH_STENCIL_DESC depth_stencil_desc;
			depth_stencil_desc.DepthEnable = depth_enable;
			depth_stencil_desc.DepthWriteMask = depth_write_mask;
			depth_stencil_desc.DepthFunc = depth_func;
			depth_stencil_desc.StencilEnable = FALSE;
			depth_stencil_desc.StencilReadMask = D3D11_DEFAULT_STENCIL_READ_MASK;
			depth_stencil_desc.StencilWriteMask = D3D11_DEFAULT_STENCIL_WRITE_MASK;
			depth_stencil_desc.FrontFace.StencilFailOp = D3D11_STENCIL_OP_KEEP;
			depth_stencil_desc.FrontFace.StencilDepthFailOp = D3D11_STENCIL_OP_INCR;
			depth_stencil_desc.FrontFace.StencilPassOp = D3D11_STENCIL_OP_KEEP;
			depth_stencil_desc.FrontFace.StencilFunc = D3D11_COMPARISON_ALWAYS;
			depth_stencil_desc.BackFace.StencilFailOp = D3D11_STENCIL_OP_KEEP;
			depth_stencil_desc.BackFace.StencilDepthFailOp = D3D11_STENCIL_OP_DECR;
			depth_stencil_desc.BackFace.StencilPassOp = D3D11_STENCIL_OP_KEEP;
			depth_stencil_desc.BackFace.StencilFunc = D3D11_COMPARISON_ALWAYS;
			HRESULT hr = device->CreateDepthStencilState(&depth_stencil_desc, state_obj.GetAddressOf());
			_ASSERT_EXPR(SUCCEEDED(hr), hr_trace(hr));
		}
		depth_stencil_state(ID3D11Device* device, const D3D11_DEPTH_STENCIL_DESC* depth_stencil_desc)
		{
			HRESULT hr = device->CreateDepthStencilState(depth_stencil_desc, state_obj.GetAddressOf());
			_ASSERT_EXPR(SUCCEEDED(hr), hr_trace(hr));
		}
		~depth_stencil_state() = default;
		depth_stencil_state(rasterizer_state&) = delete;
		depth_stencil_state& operator =(depth_stencil_state&) = delete;

		void active(ID3D11DeviceContext* immediate_context)
		{
			immediate_context->OMGetDepthStencilState(state_default_object.ReleaseAndGetAddressOf(), 0);
			immediate_context->OMSetDepthStencilState(state_obj.Get(), 1);
		}
		void inactive(ID3D11DeviceContext* immediate_context)
		{
			immediate_context->OMSetDepthStencilState(state_default_object.Get(), 1);
		}
	private:
		Microsoft::WRL::ComPtr<ID3D11DepthStencilState> state_default_object;
	};

	class sampler_state
	{
	public:
		Microsoft::WRL::ComPtr<ID3D11SamplerState> state_obj;
		sampler_state(ID3D11Device* device, D3D11_FILTER filter = D3D11_FILTER_ANISOTROPIC, D3D11_TEXTURE_ADDRESS_MODE address_mode = D3D11_TEXTURE_ADDRESS_WRAP, D3D11_COMPARISON_FUNC comparison_func = D3D11_COMPARISON_NEVER, float r = 1, float g = 1, float b = 1, float a = 1)
		{
		    D3D11_SAMPLER_DESC sampler_desc = {};
			sampler_desc.Filter = filter;
			sampler_desc.AddressU = address_mode;
			sampler_desc.AddressV = address_mode;
			sampler_desc.AddressW = address_mode;
			sampler_desc.MipLODBias = 0;
			sampler_desc.MaxAnisotropy = 16;
			sampler_desc.ComparisonFunc = comparison_func;
			float border_color[4] = { r, g, b, a };
			memcpy(sampler_desc.BorderColor, border_color, sizeof(float) * 4);
			sampler_desc.MinLOD = 0;
			sampler_desc.MaxLOD = D3D11_FLOAT32_MAX;
			HRESULT hr = device->CreateSamplerState(&sampler_desc, state_obj.GetAddressOf());
			_ASSERT_EXPR(SUCCEEDED(hr), hr_trace(hr));
		}
		sampler_state(ID3D11Device* device, const D3D11_SAMPLER_DESC* sampler_desc)
		{
			HRESULT hr = device->CreateSamplerState(sampler_desc, state_obj.GetAddressOf());
			_ASSERT_EXPR(SUCCEEDED(hr), hr_trace(hr));
		}
		~sampler_state() = default;
		sampler_state(sampler_state&) = delete;
		sampler_state& operator =(sampler_state&) = delete;

		void active(ID3D11DeviceContext* immediate_context, UINT slot, bool set_in_vs = false)
		{
			now_slot = slot;
			immediate_context->PSGetSamplers(now_slot, 1, state_default_objects[0].ReleaseAndGetAddressOf());
			immediate_context->VSSetSamplers(now_slot, 1, state_obj.GetAddressOf());

			immediate_context->PSSetSamplers(now_slot, 1, state_obj.GetAddressOf());
			if (set_in_vs)
			{
				immediate_context->VSGetSamplers(now_slot, 1, state_default_objects[1].ReleaseAndGetAddressOf());
			}
		}
		void inactive(ID3D11DeviceContext* immediate_context)
		{
			immediate_context->PSSetSamplers(now_slot, 1, state_default_objects[0].GetAddressOf());
			immediate_context->VSSetSamplers(now_slot, 1, state_default_objects[1].GetAddressOf());
		}
	private:
		UINT now_slot = 0;
		Microsoft::WRL::ComPtr<ID3D11SamplerState> state_default_objects[2];
	};
}

