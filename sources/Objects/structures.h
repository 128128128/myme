#include "../Graphics/mesh/terrain_mesh.h"
#include "../Graphics/others/shader.h"

class Structures
{
public:
	XMFLOAT4 position = XMFLOAT4(0.0f, 0.0f, 0.0f, 0.0f);
	XMFLOAT4 direction = XMFLOAT4(0.0f, 90.0f, 0.0f, 0.0f);
	XMFLOAT4 velocity = XMFLOAT4(0.0f, 0.0f, 0.0f, 0.0f);
	XMFLOAT3 scale = XMFLOAT3(1.0f, 1.0f, 1.0f);

public:
	Structures(ID3D11Device* device);
	~Structures() = default;

	void update(float elapsed_time);

	void render(ID3D11DeviceContext* immediate_context, Descartes::view_frustum view_frustum);

	//Debug
	void DebugDrawGUI();

	//getter setter
	XMFLOAT4 get_position() { return position; }
	void set_position(XMFLOAT4 pos) { position = pos; }
	XMFLOAT4 get_direction() { return direction; }
	void set_direction(XMFLOAT4 dir) { direction = dir; }


private:
	std::unique_ptr<terrain_mesh> mesh;
	char rock_names[8];
	XMFLOAT4X4 world_transform;

	//shaders
	std::unique_ptr<vertex_shader<terrain_mesh::vertex>>vs;
	std::unique_ptr<pixel_shader> ps;

	struct structures_constants
	{
		float Falloff01 = 0.1f;
		float Falloff02 = 0.2f;
		float TextureScale01 = 0.2f;
		float TextureScale02 = 0.1f;
		float TextureScale03 = 0.5f;
		XMFLOAT3 pad = { 0.0f,0.0f,0.0f };
	};
	std::unique_ptr<Descartes::constant_buffer<structures_constants>> structures_constants_buffer;


};
