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
		float x, y, z;		// ���W
		float nx, ny, nz;	// �@��
		float tu, tv;		// UV
		float r, g, b, a;		// ���_�J���[
		float tanx, tany, tanz;	// �ڐ�
		float bx, by, bz;
	};
	Microsoft::WRL::ComPtr < ID3D11Buffer> vertex_buffer;
	Microsoft::WRL::ComPtr< ID3D11Buffer> index_buffer;
};

