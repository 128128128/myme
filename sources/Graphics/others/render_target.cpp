//RenderTarget.cpp
#include "render_target.h"

void render_target::initialize(ID3D11Device* device)
{
	vertex v[4] = {
		{  1, 1,0, 0,0,1, 1,0 },
		{ -1, 1,0, 0,0,1, 0,0 },
		{  1,-1,0, 0,0,1, 1,1 },
		{ -1,-1,0, 0,0,1, 0,1 }
	};
	// 頂点バッファの生成.
	D3D11_BUFFER_DESC bd;
	ZeroMemory(&bd, sizeof(D3D11_BUFFER_DESC));
	bd.Usage = D3D11_USAGE_DEFAULT;
	bd.ByteWidth = sizeof(vertex) * 4;
	bd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	bd.CPUAccessFlags = 0;
	// サブリソースの設定.
	D3D11_SUBRESOURCE_DATA init_data;
	ZeroMemory(&init_data, sizeof(D3D11_SUBRESOURCE_DATA));
	init_data.pSysMem = v;
	device->CreateBuffer(&bd, &init_data, vertex_buffer.GetAddressOf());

	//	index
	int index[4] = { 0,1,2,3 };
	ZeroMemory(&bd, sizeof(D3D11_BUFFER_DESC));
	bd.Usage = D3D11_USAGE_DEFAULT;
	bd.ByteWidth = sizeof(int) * 4;
	bd.BindFlags = D3D11_BIND_INDEX_BUFFER;
	bd.CPUAccessFlags = 0;
	// サブリソースの設定.
	ZeroMemory(&init_data, sizeof(D3D11_SUBRESOURCE_DATA));
	init_data.pSysMem = index;
	device->CreateBuffer(&bd, &init_data, index_buffer.ReleaseAndGetAddressOf());
}

void render_target::render(ID3D11DeviceContext* immediate_context)
{
	UINT stride = sizeof(vertex);
	UINT offset = 0;
	immediate_context->IASetVertexBuffers(0, 1, vertex_buffer.GetAddressOf(), &stride, &offset);
	immediate_context->IASetIndexBuffer(index_buffer.Get(), DXGI_FORMAT_R32_UINT, 0);
	immediate_context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);

	immediate_context->DrawIndexed(4, 0, 0);
	immediate_context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
}

//bool RenderTarget::Create(
//	ID3D11Device* device,
//	int w,
//	int h,
//	int mipLevel,
//	int arraySize,
//	DXGI_FORMAT colorFormat,
//	DXGI_FORMAT depthStencilFormat,
//	float clearColor[4]
//)
//{
//	auto d3d_device = device;
//	m_width = w;
//	m_height = h;
//	//レンダリングターゲットとなるテクスチャを作成する。
//	if (!CreateRenderTargetTexture(d3d_device, w, h, mipLevel, arraySize, colorFormat, clearColor)) {
//		//	TK_ASSERT(false, "レンダリングターゲットとなるテクスチャの作成に失敗しました。");
//		MessageBoxA(nullptr, "レンダリングターゲットとなるテクスチャの作成に失敗しました。", "エラー", MB_OK);
//		return false;
//	}
//
//	//深度ステンシルバッファとなるテクスチャを作成する。
//	if (depthStencilFormat != DXGI_FORMAT_UNKNOWN) {
//		if (!CreateDepthStencilTexture(d3d_device, w, h, depthStencilFormat)) {
//			MessageBoxA(nullptr, "レンダリングターゲットとなるテクスチャの作成に失敗しました。", "エラー", MB_OK);
//			return false;
//		}
//	}
//
//	return true;
//}
//
//bool RenderTarget::CreateDepthStencilTexture(
//	ID3D11Device* device,
//	int w,
//	int h,
//	DXGI_FORMAT format)
//{
//	D3D11_TEXTURE2D_DESC td;
//	ZeroMemory(&td, sizeof(D3D11_TEXTURE2D_DESC));
//	td.Width = static_cast<UINT>(w);
//	td.Height = static_cast<UINT> (h);
//	td.MipLevels = 1;
//	td.ArraySize = 1;
//	td.Format = format;
//	td.SampleDesc.Count = 1;
//	td.SampleDesc.Quality = 0;
//	td.Usage = D3D11_USAGE_DEFAULT;
//	td.BindFlags = D3D11_BIND_DEPTH_STENCIL;
//	td.CPUAccessFlags = 0;
//	td.MiscFlags = 0;
//
//	// 深度ステンシルテクスチャ生成
//	HRESULT hr = device->CreateTexture2D(&td, NULL, Depth.GetAddressOf());
//	if (FAILED(hr)) return;
//
//	// 深度ステンシルビュー設定
//	D3D11_DEPTH_STENCIL_VIEW_DESC dsvd;
//	ZeroMemory(&dsvd, sizeof(D3D11_DEPTH_STENCIL_VIEW_DESC));
//	dsvd.Format = td.Format;
//	dsvd.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
//	dsvd.Texture2D.MipSlice = 0;
//
//	// create depth stencil view
//	hr = device->CreateDepthStencilView(Depth.Get(), &dsvd, depth_stencil_view.ReleaseAndGetAddressOf());
//
//	if (FAILED(hr)) return false;
//
//
//	
//	return true;
//}