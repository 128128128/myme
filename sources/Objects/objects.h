#pragma once
#include <d3d11.h>
#include <DirectXMath.h>
#include <memory>
#include "../Graphics/mesh/static_mesh.h"
#include "../Graphics/mesh/pbr_static_mesh.h"
#include "../Graphics/mesh/dynamic_mesh.h"
#include "../Graphics/others/shader.h"
#include "../Graphics/load/texture.h"

using namespace DirectX;
class Objects
{
public:
	Objects(){}
	virtual ~Objects(){}

	virtual void update(float elapsed_time) = 0;

	virtual void render(ID3D11DeviceContext* immediate_context)=0;

	virtual void reset(XMFLOAT4& location) = 0;

	//Debug
	virtual void DebugDrawGUI(bool flag=false) = 0;

};

class Cube:public Objects
{
public:
    std::unique_ptr<static_mesh> mesh;
	XMFLOAT4 position = XMFLOAT4(0.0f, 0.0f, 0.0f, 0.0f);
	XMFLOAT4 direction = XMFLOAT4(0.0f, 90.0f, 0.0f, 0.0f);
	XMFLOAT4 velocity = XMFLOAT4(0.0f, 0.0f, 0.0f, 0.0f);
	XMFLOAT3 scale = XMFLOAT3(1.0f, 1.0f, 1.0f);

public:
	Cube();
	~Cube() = default;

	void update(float elapsed_time)override;

	void render(ID3D11DeviceContext* immediate_context)override
	{
		mesh->render(immediate_context, world_transform);
	}

	void reset(XMFLOAT4& location)override
	{
		position = location;
		float angle = 180;
		direction.x = sinf(angle * 0.01745f);
		direction.y = 0;
		direction.z = cosf(angle * 0.01745f);
	}
	//Debug
	void DebugDrawGUI(bool flag = false)override;

	//getter setter
	XMFLOAT4 GetPosition() { return position; }
	void SetPosition(XMFLOAT4 pos) { position = pos; }
    XMFLOAT4 GetDirection() { return direction; }
	void SetDirection(XMFLOAT4 dir) { direction = dir; }


private:
	XMFLOAT4X4 world_transform;

};

class pbr_Stage
{
public:
	std::unique_ptr<pbr_static_mesh> mesh;
	XMFLOAT4 position = XMFLOAT4(0.0f, 0.0f, 0.0f, 0.0f);
	XMFLOAT4 direction = XMFLOAT4(0.0f, 0.0f, 0.0f, 0.0f);
	XMFLOAT4 velocity = XMFLOAT4(0.0f, 0.0f, 0.0f, 0.0f);
	XMFLOAT3 scale = XMFLOAT3(1.0f, 1.0f, 1.0f);

	struct pbr_constants
	{
		float metallic = 1;
		float roughness = 0;
		float pure_white = 1;
		float pad = 0.0f;
	};
	std::unique_ptr<Descartes::constant_buffer<pbr_constants>> pbr_constant_buffer;

	Microsoft::WRL::ComPtr <ID3D11ShaderResourceView> r_srv;
	Microsoft::WRL::ComPtr <ID3D11ShaderResourceView> m_srv;
public:
	pbr_Stage(ID3D11Device* device, const char* filename);
	~pbr_Stage() = default;

	void update(float elapsed_time);

	void render(ID3D11DeviceContext* immediate_context, ID3D11PixelShader* replaced_pixel_shader=nullptr)
	{
		pbr_constant_buffer->active(immediate_context, 4, true, true);
		//immediate_context->PSSetShaderResources(5, 1, r_srv.GetAddressOf());
		//immediate_context->PSSetShaderResources(6, 1, m_srv.GetAddressOf());
		mesh->render(immediate_context, world_transform, replaced_pixel_shader);

		pbr_constant_buffer->inactive(immediate_context);
	}

	void reset(XMFLOAT4& location)
	{
		position = location;
		float angle = 180;
		direction.x = sinf(angle * 0.01745f);
		direction.y = 0;
		direction.z = cosf(angle * 0.01745f);
	}
	//Debug
	void DebugDrawGUI(bool flag=false);

	//getter setter
	XMFLOAT4 GetPosition() { return position; }
	void SetPosition(XMFLOAT4 pos) { position = pos; }
	XMFLOAT4 GetDirection() { return direction; }
	void SetDirection(XMFLOAT4 dir) { direction = dir; }
	void SetScale(XMFLOAT3 s) { scale = s; }


private:
	XMFLOAT4X4 world_transform;

};

class pbr_Base
{
public:
	std::unique_ptr<pbr_static_mesh> mesh;
	XMFLOAT4 position = XMFLOAT4(0.0f, 0.0f, 0.0f, 0.0f);
	XMFLOAT4 direction = XMFLOAT4(0.0f, 0.0f, 0.0f, 0.0f);
	XMFLOAT4 velocity = XMFLOAT4(0.0f, 0.0f, 0.0f, 0.0f);
	XMFLOAT3 scale = XMFLOAT3(1.0f, 1.0f, 1.0f);

	struct pbr_constants
	{
		float metallic = 1;
		float roughness = 0;
		float pure_white = 1;
		float pad = 0.0f;
	};
	std::unique_ptr<Descartes::constant_buffer<pbr_constants>> pbr_constant_buffer;

public:
	pbr_Base(ID3D11Device* device, const char* filename);
	~pbr_Base() = default;

	void update(float elapsed_time);

	void render(ID3D11DeviceContext* immediate_context, ID3D11PixelShader* replaced_pixel_shader = nullptr)
	{
		pbr_constant_buffer->active(immediate_context, 4, true, true);
		
		mesh->render(immediate_context, world_transform, replaced_pixel_shader);

		pbr_constant_buffer->inactive(immediate_context);
	}

	void reset(XMFLOAT4& location)
	{
		position = location;
		float angle = 180;
		direction.x = sinf(angle * 0.01745f);
		direction.y = 0;
		direction.z = cosf(angle * 0.01745f);
	}
	//Debug
	void DebugDrawGUI(bool flag = false);

	//getter setter
	XMFLOAT4 GetPosition() { return position; }
	void SetPosition(XMFLOAT4 pos) { position = pos; }
	XMFLOAT4 GetDirection() { return direction; }
	void SetDirection(XMFLOAT4 dir) { direction = dir; }
	void SetScale(XMFLOAT3 s) { scale = s; }


private:
	XMFLOAT4X4 world_transform;

};

class Ground
{
public:
	std::unique_ptr<static_mesh> mesh;
	XMFLOAT4 position = XMFLOAT4(0.0f, 0.0f, 0.0f, 0.0f);
	XMFLOAT4 direction = XMFLOAT4(0.0f, 90.0f, 0.0f, 0.0f);
	XMFLOAT4 velocity = XMFLOAT4(0.0f, 0.0f, 0.0f, 0.0f);
	XMFLOAT3 scale = XMFLOAT3(1.0f, 1.0f, 1.0f);

public:
	Ground();
	~Ground() = default;

	void update(float elapsed_time);

	void render(ID3D11DeviceContext* immediate_context, ID3D11PixelShader* replaced_pixel_shader = nullptr)
	{
		mesh->render(immediate_context, world_transform, replaced_pixel_shader);
	}

	void reset(XMFLOAT4& location)
	{
		position = location;
		float angle = 180;
		direction.x = sinf(angle * 0.01745f);
		direction.y = 0;
		direction.z = cosf(angle * 0.01745f);
	}
	//Debug
	void DebugDrawGUI();

	//getter setter
	XMFLOAT4 GetPosition() { return position; }
	void SetPosition(XMFLOAT4 pos) { position = pos; }
	XMFLOAT4 GetDirection() { return direction; }
	void SetDirection(XMFLOAT4 dir) { direction = dir; }


private:
	XMFLOAT4X4 world_transform;

};

class VegetationSmall
{
public:
	XMFLOAT4 position = XMFLOAT4(0.0f, 0.0f, 0.0f, 0.0f);
	XMFLOAT4 direction = XMFLOAT4(0.0f, 90.0f, 0.0f, 0.0f);
	XMFLOAT4 velocity = XMFLOAT4(0.0f, 0.0f, 0.0f, 0.0f);
	XMFLOAT3 scale = XMFLOAT3(1.0f, 1.0f, 1.0f);

public:
	VegetationSmall(ID3D11Device* device);
	~VegetationSmall() = default;

	void update(float elapsed_time);

	void render(ID3D11DeviceContext* immediate_context);


	//Debug
	void DebugDrawGUI();

	//getter setter
	XMFLOAT4 GetPosition() { return position; }
	void SetPosition(XMFLOAT4 pos) { position = pos; }
	XMFLOAT4 GetDirection() { return direction; }
	void SetDirection(XMFLOAT4 dir) { direction = dir; }


private:
	std::unique_ptr<static_mesh> vegetation;
	XMFLOAT4X4 world_transform;

	//shaders
	std::unique_ptr<vertex_shader<static_mesh::vertex>>vs;
	std::unique_ptr<pixel_shader> ps;

	//shader resources
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> emissive_shader_resource_view;

	struct constants
	{
		float power = 1.0f;
		XMFLOAT3 color = { 1.0f,1.0f,1.0f };
	};
	std::unique_ptr<Descartes::constant_buffer<constants>> constants_buffer;
};
