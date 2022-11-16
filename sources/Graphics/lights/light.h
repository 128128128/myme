#pragma once
#include <DirectXMath.h>
#include <memory>
#include <string>

#include "../others/constant_buffer.h"

class light
{
public:
	light(ID3D11Device *device);
	~light() {}

	virtual void DebugDrawGUI(std::string name) {};
protected:
	//constant buffer
	struct dir_light_param {
		DirectX::XMFLOAT4 dir_light_dir;
		DirectX::XMFLOAT4 dir_light_color;
	};

public:
	std::unique_ptr<Descartes::constant_buffer<dir_light_param>>light_constants{};

	std::string name;
};

//direction lights
class directional_light :public light
{
public:
	directional_light(ID3D11Device* device, DirectX::XMFLOAT3 dir, DirectX::XMFLOAT3 color);
	~directional_light() {}

	void DebugDrawGUI(std::string name) override;

	void set_direction(DirectX::XMFLOAT3 position);
	void set_color(DirectX::XMFLOAT3 color);

	DirectX::XMFLOAT3 get_direction();
	DirectX::XMFLOAT3 get_color();
};

//point light class
class point_light :public light
{
public:
	point_light(ID3D11Device* device, DirectX::XMFLOAT3 position, float distance, DirectX::XMFLOAT3 color);
	~point_light() {}

	void DebugDrawGUI(std::string name) override;

	void set_position(DirectX::XMFLOAT3 position);
	void set_color(DirectX::XMFLOAT3 color);
	void set_distance(float d);

	DirectX::XMFLOAT3 get_position();
	DirectX::XMFLOAT3 get_color();
	float get_distance();
};
