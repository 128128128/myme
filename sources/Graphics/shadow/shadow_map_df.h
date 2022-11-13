#pragma once
#include <d3d11.h>
#include <DirectXMath.h>
#include <memory>

#include "../others/shader.h"
#include "../others/render_target.h"

class shadow_map
{
public:
	shadow_map();
	~shadow_map(){};
	void initialize(ID3D11Device* device);
	void active(ID3D11DeviceContext* immediate_context);
	void inactive(ID3D11DeviceContext* immediate_context);
	//Debug
	void DebugDrawGUI();

	Microsoft::WRL::ComPtr < ID3D11DepthStencilView> depth_stencil_view;

protected:
	struct vertex
	{
		DirectX::XMFLOAT3 position;
		DirectX::XMFLOAT3 normal;
		DirectX::XMFLOAT2 texcoord;
	};

	static const int ShadowMapSize = 4096;
	std::unique_ptr<pixel_shader> shadow_pixel_shader;
	Microsoft::WRL::ComPtr<ID3D11VertexShader> vertex_shader;
	Microsoft::WRL::ComPtr<ID3D11InputLayout> input_layout;
	Microsoft::WRL::ComPtr<ID3D11Buffer> vertex_buffer;

	Microsoft::WRL::ComPtr <ID3D11Texture2D>  Texture;
	Microsoft::WRL::ComPtr <ID3D11Texture2D> Depth;
	Microsoft::WRL::ComPtr < ID3D11RenderTargetView>  RTV_Shadow;
	Microsoft::WRL::ComPtr < ID3D11ShaderResourceView> SRV_Shadow;
	Microsoft::WRL::ComPtr < ID3D11SamplerState> SamplerState_Shadow;

	Microsoft::WRL::ComPtr <ID3D11Buffer> CBShadow;

	//struct CBShadowParam {
	//	Matrix ShadowVP;
	//};
protected:
	Microsoft::WRL::ComPtr <ID3D11Texture2D> Texture2;
	Microsoft::WRL::ComPtr < ID3D11RenderTargetView> RTV_Shadow2;
	Microsoft::WRL::ComPtr < ID3D11ShaderResourceView> SRV_Shadow2;
	std::unique_ptr<pixel_shader> blur_pixel_shader;
	Microsoft::WRL::ComPtr<ID3D11VertexShader> blur_vertex_shader;

	render_target render_target;
	Microsoft::WRL::ComPtr<ID3D11RenderTargetView> default_render_target_view;
	Microsoft::WRL::ComPtr<ID3D11DepthStencilView> default_depth_stencil_view;


};
