#pragma once
#include <d3d11.h>
#include <wrl/client.h>

class render_target
{
public:
	render_target(){};
	~render_target(){};
	void initialize(ID3D11Device* device);
	void render(ID3D11DeviceContext* immediate_context);

protected:
	struct vertex {
		float x, y, z;		// 座標
		float nx, ny, nz;	// 法線
		float tu, tv;		// UV
		float r, g, b, a;		// 頂点カラー
		float tanx, tany, tanz;	// 接線
		float bx, by, bz;
	};
	Microsoft::WRL::ComPtr < ID3D11Buffer> vertex_buffer;
	Microsoft::WRL::ComPtr< ID3D11Buffer> index_buffer;
};


/// <summary>
/// レンダリングターゲット。
/// </summary>
//class RenderTarget {
//public:
//	/// <summary>
//	/// デストラクタ。
//	/// </summary>
//	~RenderTarget();
//	/// <summary>
//	/// レンダリングターゲットの作成。
//	/// </summary>
//	/// <param name="w">レンダリングターゲットの幅</param>
//	/// <param name="h">レンダリングターゲットの高さ</param>
//	/// <param name="mipLevel">ミップマップレベル。0を指定した場合はミップマップがサポートされているGPUでは1*1ピクセルまでのミップマップが作成される。</param>
//	/// <param name="arraySize">テクスチャ配列のサイズ</param>
//	/// <param name="colorFormat">カラーバッファのフォーマット。</param>
//	/// <param name="depthStencilFormat">深度ステンシルバッファのフォーマット。</param>
//	/// <returns>trueが返ってきたら作成成功</returns>
//	bool Create(
//		ID3D11Device*& device,
//		int w,
//		int h,
//		int mipLevel,
//		int arraySize,
//		DXGI_FORMAT colorFormat,
//		DXGI_FORMAT depthStencilFormat,
//		float clearColor[4] = nullptr
//	);
//	
//	/// <summary>
//	/// レンダリングターゲットとなるテクスチャを取得。
//	/// </summary>
//	/// <returns></returns>
//	Texture& GetRenderTargetTexture()
//	{
//		return m_renderTargetTexture;
//	}
//	/// <summary>
//	/// デプスステンシルバッファが存在しているか判定
//	/// </summary>
//	/// <returns></returns>
//	bool IsExsitDepthStencilBuffer() const
//	{
//		return m_depthStencilTexture;
//	}
//	/// <summary>
//	/// レンダリングターゲットの幅を取得。
//	/// </summary>
//	/// <returns></returns>
//	int GetWidth() const
//	{
//		return m_width;
//	}
//	/// <summary>
//	/// レンダリングターゲットの高さを取得。
//	/// </summary>
//	/// <returns></returns>
//	int GetHeight() const
//	{
//		return m_height;
//	}
//	/// <summary>
//	/// カラーバッファのフォーマットを取得。
//	/// </summary>
//	/// <returns></returns>
//	DXGI_FORMAT GetColorBufferFormat() const
//	{
//		return render_target_view.GetFormat();
//	}
//	const float* GetRTVClearColor() const
//	{
//		return m_rtvClearColor;
//	}
//	float GetDSVClearValue() const
//	{
//		return m_dsvClearValue;
//	}
//private:
//	/// <summary>
//	/// レンダリングターゲットとなるテクスチャを作成。
//	/// </summary>
//	/// <param name="ge">グラフィックエンジン</param>
//	/// <param name="d3dDevice">D3Dデバイス</param>
//	/// <param name="w">テクスチャの幅</param>
//	/// <param name="h">テクスチャの高さ</param>
//	/// <param name="mipLevel">ミップマップレベル</param>
//	/// <param name="arraySize">テクスチャ配列のサイズ</param>
//	/// <param name="format">テクスチャのフォーマット</param>
//	/// <returns>trueが返ってきたら成功。</returns>
//	bool CreateRenderTargetTexture(
//		ID3D11Device* device,
//		int w,
//		int h,
//		int mipLevel,
//		int arraySize,
//		DXGI_FORMAT format,
//		float clearColor[4]
//	);
//	/// <summary>
//	/// 深度ステンシルバッファとなるテクスチャを作成。
//	/// </summary>
//	/// <param name="ge">グラフィックエンジン</param>
//	/// <param name="d3dDevice">D3Dデバイス</param>
//	/// <param name="w">テクスチャの幅</param>
//	/// <param name="h">テクスチャの高さ</param>
//	/// <param name="format">テクスチャのフォーマット</param>
//	/// <returns>trueが返ってきたら成功</returns>
//	bool CreateDepthStencilTexture(
//	    ID3D11Device* device,
//		int w,
//		int h,
//		DXGI_FORMAT format);
//	
//private:
//	Microsoft::WRL::ComPtr <ID3D11Texture2D>  Texture;
//	Microsoft::WRL::ComPtr <ID3D11Texture2D>  Depth;
//	//Texture m_renderTargetTexture;
//	Microsoft::WRL::ComPtr < ID3D11RenderTargetView>  render_target_view;
//	//ID3D12Resource* m_renderTargetTextureDx12 = nullptr;//レンダリングターゲットとなるテクスチャ。
//	Microsoft::WRL::ComPtr < ID3D11DepthStencilView>depth_stencil_view;
//	//ID3D12Resource* m_depthStencilTexture = nullptr;	//深度ステンシルバッファとなるテクスチャ。
//
//	UINT m_rtvDescriptorSize = 0;						//フレームバッファのディスクリプタのサイズ。
//	UINT m_dsvDescriptorSize = 0;						//深度ステンシルバッファのディスクリプタのサイズ。
//	int m_width = 0;									//レンダリングターゲットの幅。
//	int m_height = 0;									//レンダリングターゲットの高さ。
//	float m_rtvClearColor[4] = { 0.0f, 0.0f, 0.0f, 1.0f };	//レンダリングターゲットビューのクリアカラー。
//	float m_dsvClearValue = 1.0f;							//DSVのクリアカラー。
//};
