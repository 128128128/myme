#pragma once
#include "../../imgui/imgui.h"

#include <memory>

#include <DirectXMath.h>
using namespace DirectX;

#include "../Graphics/others/constant_buffer.h"
#include "../Math/math.h"

struct camera
{
private:
	struct camera_constants
	{
		XMFLOAT4 position;
		XMFLOAT4 direction;
		XMFLOAT4X4 view;
		XMFLOAT4X4 projection;
		XMFLOAT4X4 view_projection;
		XMFLOAT4X4 inverse_view;
		XMFLOAT4X4 inverse_view_projection;//ãtçsóÒ
	};

	std::unique_ptr<Descartes::constant_buffer<camera_constants>> camera_constants_buffer;
public:
	camera(ID3D11Device* device, const XMFLOAT4& position, const XMFLOAT4& focus, const XMFLOAT4& up, bool perspective_projection, float fovy_or_view_width, float aspect_ratio, float near_z, float far_z);
	camera(ID3D11Device* device);

	XMFLOAT4 position = XMFLOAT4(-100.3f, -7.7f, -65.3f, 1);
	XMFLOAT4 focus = XMFLOAT4(0, 0.0f, 0, 1);
	XMFLOAT4 up = XMFLOAT4(0, 1, 0, 0);//y-up
	DirectX::XMFLOAT4 front=XMFLOAT4(0, 0, 1,0);
	DirectX::XMFLOAT4 right= XMFLOAT4(0, 0, 1,0);
	XMFLOAT3 angle = XMFLOAT3{ 0,0,0 };
	float		view_angle = math::deg_to_rad(60.0f);	//(Radian)ÅB

	bool perspective_projection = true;
	float fovy_or_view_width = 60;
	float aspect_ratio = 1.8f;
	float near_z = 0.1f;
	float far_z = 200.0f;

	DirectX::XMFLOAT4 cameraOrientation{ 0,0,0,1 };
	float cameraSpeed = 8.0f;
	float cameraAngleSpeed = 0.25f;

	//these property methods become valid after calling 'active' method
	const XMFLOAT4& direction() const { return camera_constants_buffer->data.direction; }
	const XMFLOAT4X4& view() const { return camera_constants_buffer->data.view; }
	const XMFLOAT4X4& projection() const { return camera_constants_buffer->data.projection; }
	const XMFLOAT4X4& view_projection() const { return camera_constants_buffer->data.view_projection; }
	void set_view_projection(XMFLOAT4X4 view_projection) const { camera_constants_buffer->data.view_projection=view_projection; }
	const float& get_near_z() const { return near_z; }
	const float& get_far_z() const { return far_z; }
	const XMFLOAT4& get_up() const { return up; }
	const XMFLOAT4& get_right() const { return right; }
	const XMFLOAT4& get_front() const { return front; }
	const XMFLOAT4& get_position() const { return position; }
	const float& get_view_angle() const { return view_angle; }

	const XMFLOAT4X4& inverse_view_projection() const { return camera_constants_buffer->data.inverse_view_projection; }
	
	void freelook_update(float elapsedTime, DirectX::XMFLOAT4 pos);
	void update(XMFLOAT4 target_position, float elapsedTime);
	void firstperson_update(float elapsedTime, DirectX::XMFLOAT4 pos);
	void active(ID3D11DeviceContext* immediate_context, int slot, bool set_in_vs, bool set_in_ps,bool set_in_cs, bool set_in_gs);

	void reset(const XMFLOAT4& target, const XMFLOAT4& direction, float focal_length, float height_above_ground);
	void set_view(const DirectX::XMFLOAT4& eye, const DirectX::XMFLOAT4& focus, DirectX::XMFLOAT4& up);
	void set_target(const DirectX::XMFLOAT4& target) { this->target = target; };
	void set_position(const DirectX::XMFLOAT4& position) { this->position = position; };

	void DrawDebugGUI();

private:
	DirectX::XMFLOAT4 target{ 0,0,0 ,0};
	DirectX::XMFLOAT3 forward{};
	float rollSpeed = DirectX::XMConvertToRadians(90);
	float range = 10.0f;

	float maxAngleX = DirectX::XMConvertToRadians(45);
	float minAngleX = DirectX::XMConvertToRadians(-45);

	DirectX::XMFLOAT4 eye;
	float focal_length = 5;

	DirectX::XMFLOAT2 mouseVec{ 0,0 };

	float rollSpeedX{ 20 };
	float rollSpeedY{ 20 };
	float minAngle{ DirectX::XMConvertToRadians(-85) };
	float maxAngle{ DirectX::XMConvertToRadians(85) };
};