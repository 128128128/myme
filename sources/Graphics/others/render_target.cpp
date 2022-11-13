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
