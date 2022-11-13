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

