#include "light.h"
#include "../../imgui/imgui.h"

light::light(ID3D11Device *device)
{
	light_constants = 
	std::make_unique<Descartes::constant_buffer<dir_light_param>>(device);
}


directional_light::directional_light(ID3D11Device* device, DirectX::XMFLOAT3 dir, DirectX::XMFLOAT3 color) :light(device)
{
	//direction light isodirection xAdirection yAdirection zA0.0p
	light_constants->data.dir_light_dir = { dir.x, dir.y, dir.z, 0.0f };
	light_constants->data.dir_light_color = { color.x, color.y, color.z, 1.0f };
}

void directional_light::DebugDrawGUI(std::string name)
{
	ImGui::Begin("light");
	if (ImGui::CollapsingHeader(name.c_str()))
	{
		ImGui::DragFloat3(std::string("dir " + name).c_str(), &light_constants->data.dir_light_dir.x, 0.1f);
		ImGui::DragFloat3(std::string("color " + name).c_str(), &light_constants->data.dir_light_color.x, 0.1f, 0.0f);
	}
	ImGui::End();
}

void directional_light::set_direction(DirectX::XMFLOAT3 dir)
{
	light_constants->data.dir_light_dir = { dir.x, dir.y, dir.z, 0.0f };
}
void directional_light::set_color(DirectX::XMFLOAT3 color)
{
	light_constants->data.dir_light_color = { color.x, color.y, color.z, 1.0f };
}

DirectX::XMFLOAT3 directional_light::get_direction()
{
	return DirectX::XMFLOAT3(light_constants->data.dir_light_dir.x, light_constants->data.dir_light_dir.y, light_constants->data.dir_light_dir.z);
}

DirectX::XMFLOAT3 directional_light::get_color()
{
	return DirectX::XMFLOAT3(light_constants->data.dir_light_color.x, light_constants->data.dir_light_color.y, light_constants->data.dir_light_color.z);
}


point_light::point_light(ID3D11Device* device, DirectX::XMFLOAT3 position, float distance, DirectX::XMFLOAT3 color) :light(device)
{
	//point light isoposition xAposition yAposition zAattenuation distance...Œ¸Š‹——£p
	light_constants->data.dir_light_dir = { position.x, position.y, position.z, distance };
	light_constants->data.dir_light_color = { color.x, color.y, color.z, 1.0f };

}

void point_light::DebugDrawGUI(std::string name)
{
	ImGui::Begin("light");
	if (ImGui::CollapsingHeader(name.c_str()))
	{
		ImGui::DragFloat3(std::string("position " + name).c_str(), &light_constants->data.dir_light_dir.x, 0.5f);
		ImGui::DragFloat3(std::string("color " + name).c_str(), &light_constants->data.dir_light_color.x, 0.1f, 0);
		ImGui::DragFloat(std::string("distance " + name).c_str(), &light_constants->data.dir_light_dir.w, 0.1f, 1, 500);
	}
	ImGui::End();
}

void point_light::set_position(DirectX::XMFLOAT3 position)
{
	light_constants->data.dir_light_dir.x = position.x;
	light_constants->data.dir_light_dir.y = position.y;
	light_constants->data.dir_light_dir.z = position.z;
}
void point_light::set_color(DirectX::XMFLOAT3 color)
{
	light_constants->data.dir_light_color = { color.x, color.y, color.z, 1.0f };
}
void point_light::set_distance(float d)
{
	light_constants->data.dir_light_dir.w = d;
}

DirectX::XMFLOAT3 point_light::get_position()
{
	return DirectX::XMFLOAT3(light_constants->data.dir_light_dir.x, light_constants->data.dir_light_dir.y, light_constants->data.dir_light_dir.z);
}

DirectX::XMFLOAT3 point_light::get_color()
{
	return DirectX::XMFLOAT3(light_constants->data.dir_light_color.x, light_constants->data.dir_light_color.y, light_constants->data.dir_light_color.z);
}

float point_light::get_distance()
{
	return light_constants->data.dir_light_dir.w;
}

