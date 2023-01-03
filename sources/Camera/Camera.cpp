#include "Camera.h"
#include "../Math/math.h"
#include "../Input/Input.h"

camera::camera(ID3D11Device* device, const XMFLOAT4& position, const XMFLOAT4& focus, const XMFLOAT4& up, bool perspective_projection, float fovy_or_view_width, float aspect_ratio, float near_z, float far_z)
	: position(position), focus(focus), up(up), perspective_projection(perspective_projection), fovy_or_view_width(fovy_or_view_width), aspect_ratio(aspect_ratio), near_z(near_z), far_z(far_z)
{
	camera_constants_buffer = std::make_unique<Descartes::constant_buffer<camera_constants>>(device);
}

camera::camera(ID3D11Device* device)
{
	camera_constants_buffer = std::make_unique<Descartes::constant_buffer<camera_constants>>(device);
}

void camera::active(ID3D11DeviceContext* immediate_context, int slot, bool set_in_vs, bool set_in_ps, bool set_in_cs,bool set_in_gs)
{
	XMVECTOR E = XMLoadFloat4(&position);
	XMVECTOR F = XMLoadFloat4(&focus);
	XMVECTOR U = XMLoadFloat4(&up);
	XMVECTOR D = XMVector3Normalize(F - E);
	XMStoreFloat4(&camera_constants_buffer->data.position, E);
	XMStoreFloat4(&camera_constants_buffer->data.direction, D);

	XMMATRIX V = XMMatrixLookAtLH(E, F, U);
	XMStoreFloat4x4(&camera_constants_buffer->data.view, V);
	XMMATRIX P = perspective_projection ? XMMatrixPerspectiveFovLH(fovy_or_view_width * 0.01745f, aspect_ratio, near_z, far_z) : XMMatrixOrthographicLH(fovy_or_view_width, fovy_or_view_width / aspect_ratio, 1.0f, 200.0f);
	XMStoreFloat4x4(&camera_constants_buffer->data.projection, P);
	XMStoreFloat4x4(&camera_constants_buffer->data.view_projection, V * P);
	XMStoreFloat4x4(&camera_constants_buffer->data.inverse_view_projection, DirectX::XMMatrixInverse(0, V * P));
	
	camera_constants_buffer->active(immediate_context, slot, set_in_vs, set_in_ps, set_in_cs,set_in_gs);
}

void camera::reset(const XMFLOAT4& target, const XMFLOAT4& direction, float focal_length, float height_above_ground)
{
	focus = target;
	position = XMFLOAT4(focus.x - direction.x * focal_length, focus.y - direction.y * focal_length + height_above_ground, focus.z - direction.z * focal_length, 1.0f);
}


void camera::freelook_update(float elapsedTime, DirectX::XMFLOAT4 pos)
{
	GamePad& gamePad = Input::Instance().GetGamePad();
	Mouse& mouse = Input::Instance().GetMouse();

	using namespace DirectX;

		DirectX::XMVECTOR Orientation = DirectX::XMLoadFloat4(&cameraOrientation);
		DirectX::XMMATRIX m = DirectX::XMMatrixRotationQuaternion(Orientation);
		DirectX::XMFLOAT4X4 m4x4 = {};
		DirectX::XMStoreFloat4x4(&m4x4, m);

		//回転行列から全方向ベクトルを取り出す
		DirectX::XMVECTOR Right = { m4x4._11, m4x4._12, m4x4._13 };
		DirectX::XMVECTOR Up = { m4x4._21, m4x4._22, m4x4._23 };
		DirectX::XMVECTOR Forward = { m4x4._31, m4x4._32, m4x4._33 };

		DirectX::XMVECTOR NPos = DirectX::XMLoadFloat4(&position);

		DirectX::XMFLOAT3 axisUp = { 0,1,0 };
		DirectX::XMVECTOR AxisUp = DirectX::XMLoadFloat3(&axisUp);
		DirectX::XMFLOAT3 axisRight{};
		DirectX::XMVECTOR AxisRight = Right;
		DirectX::XMStoreFloat3(&axisRight, AxisRight);
		axisRight.y = 0;
		AxisRight = DirectX::XMLoadFloat3(&axisRight);

		//---------------------------------
		// moving
		//---------------------------------
		if (gamePad.GetAxisLX() > 0.2f || (gamePad.GetButton() & GamePad::KEY_D))
		{
			//right
			NPos += Right * cameraSpeed * elapsedTime;
		}
		if (gamePad.GetAxisLX() < -0.2f || (gamePad.GetButton() & GamePad::KEY_A))
		{
			//left
			NPos += -Right * cameraSpeed * elapsedTime;
		}
		if (gamePad.GetAxisLY() > 0.2f || (gamePad.GetButton() & GamePad::KEY_W))
		{
			//forward
			NPos += Forward * cameraSpeed * elapsedTime;
		}
		if (gamePad.GetAxisLY() < -0.2f || (gamePad.GetButton() & GamePad::KEY_S))
		{
			//back
			NPos += -Forward * cameraSpeed * elapsedTime;
		}

		/*mouse.SetPositionX(mouse.GetScreenWidth() / 2);
		mouse.SetPositionY(mouse.GetScreenHeight() / 2);*/

		int subX = mouse.GetPositionX() - mouse.GetOldPositionX();
		int subY = mouse.GetPositionY() - mouse.GetOldPositionY();

		//---------------------------------
		//camera direction
		//---------------------------------
		if (mouse.GetButton() & Mouse::BTN_RIGHT) {
			//if (gamePad.GetAxisRX() > 0.2f || (gamePad.GetButton() & GamePad::KEY_L))
		if (subX > 0)
		{
			//right
			DirectX::XMVECTOR Q = DirectX::XMQuaternionRotationAxis(AxisUp, cameraAngleSpeed);
			DirectX::XMVECTOR OQ = DirectX::XMQuaternionMultiply(Orientation, Q);
			Orientation = DirectX::XMQuaternionSlerp(Orientation, OQ, 0.1f);
		}
		//if (gamePad.GetAxisRX() < -0.2f || (gamePad.GetButton() & GamePad::KEY_J))
		if (subX < 0)
		{
			//left
			DirectX::XMVECTOR Q = DirectX::XMQuaternionRotationAxis(AxisUp, -cameraAngleSpeed);
			DirectX::XMVECTOR OQ = DirectX::XMQuaternionMultiply(Orientation, Q);
			Orientation = DirectX::XMQuaternionSlerp(Orientation, OQ, 0.1f);
		}
		//if (gamePad.GetAxisRY() > 0.2f || (gamePad.GetButton() & GamePad::KEY_I))
		if (subY < 0)
		{
			//up
			DirectX::XMVECTOR Q = DirectX::XMQuaternionRotationAxis(AxisRight, -cameraAngleSpeed);
			DirectX::XMVECTOR OQ = DirectX::XMQuaternionMultiply(Orientation, Q);
			Orientation = DirectX::XMQuaternionSlerp(Orientation, OQ, 0.1f);
		}
		//if (gamePad.GetAxisRY() < -0.2f || (gamePad.GetButton() & GamePad::KEY_K))
		if (subY > 0)
		{
			//down
			DirectX::XMVECTOR Q = DirectX::XMQuaternionRotationAxis(AxisRight, cameraAngleSpeed);
			DirectX::XMVECTOR OQ = DirectX::XMQuaternionMultiply(Orientation, Q);
			Orientation = DirectX::XMQuaternionSlerp(Orientation, OQ, 0.1f);
		}
		}

		DirectX::XMStoreFloat4(&cameraOrientation, Orientation);

		DirectX::XMStoreFloat4(&position, NPos);
		DirectX::XMVECTOR Target = NPos + Forward;
		DirectX::XMFLOAT4 target{};
		DirectX::XMStoreFloat4(&target, Target);
		focus = target;//Todo
		DirectX::XMFLOAT4 up{};
		DirectX::XMStoreFloat4(&up, AxisUp);
		this->up = up;

		//this->update(elapsedTime);
	
}

//update
	void camera::update(XMFLOAT4 target_position, float elapsed_time)
	{
		Mouse& mouse = Input::Instance().GetMouse();

		float elapsed_time_for_camera = elapsed_time;

		//camera calculations
		XMVECTOR CP = XMLoadFloat4(&position);
		XMVECTOR CF = XMLoadFloat4(&focus);
		XMVECTOR CX, CY, CZ;
		CY = XMVectorSet(0, 1, 0, 0);
		CZ = XMVector3Normalize(CF - CP);
		CX = XMVector3Normalize(XMVector3Cross(CY, CZ));

		float zoom = 0;
		zoom = -static_cast<float>(mouse.GetWheel());

		float elevation = 0;
		if (mouse.GetButton() & Mouse::BTN_RIGHT)
		{
			int diffrence = mouse.GetOldPositionY() - mouse.GetPositionY();
			if (diffrence != 0)
				elevation = diffrence > 0 ? +1.0f : -1.0f;
		}

		float panorama = 0;
		if (mouse.GetButton() & Mouse::BTN_RIGHT)
		{
			int diffrence = mouse.GetOldPositionX() - mouse.GetPositionX();
			if (diffrence != 0)
				panorama = diffrence > 0 ? +1.0f : -1.0f;
		}

		float sensitivity_zoom = 10.0f * elapsed_time_for_camera;
		focal_length = std::max<float>(1.5f, std::max<float>(0.0f, focal_length + zoom * sensitivity_zoom));

		float sensitivity_panorama = 2.0f * elapsed_time_for_camera;
		XMMATRIX RY = XMMatrixRotationAxis(CY, panorama * sensitivity_panorama);

		float sensitivity_elevation = 1.0f * elapsed_time_for_camera;
		XMMATRIX RX = XMMatrixRotationAxis(CX, elevation * sensitivity_elevation);

		CP = focal_length * XMVector3Normalize(XMVector3Transform(CP - CF, RX * RY)) + CF;
		XMStoreFloat4(&position, CP);
		focus = XMFLOAT4(target_position.x, target_position.y + 1.5f, target_position.z, 1.0f);
	
}

//first person update
void camera::firstperson_update(float elapsedTime,DirectX::XMFLOAT4 pos)
{
	Mouse& mouse = Input::Instance().GetMouse();
	GamePad& gamePad = Input::Instance().GetGamePad();
	using namespace DirectX;

	DirectX::XMVECTOR Orientation = DirectX::XMLoadFloat4(&cameraOrientation);
	DirectX::XMMATRIX m = DirectX::XMMatrixRotationQuaternion(Orientation);
	DirectX::XMFLOAT4X4 m4x4 = {};
	DirectX::XMStoreFloat4x4(&m4x4, m);

	//Extract all direction vectors from a rotation matrix
	//回転行列から全方向ベクトルを取り出す
	DirectX::XMVECTOR Right = { m4x4._11, m4x4._12, m4x4._13 };
	DirectX::XMVECTOR Up = { m4x4._21, m4x4._22, m4x4._23 };
	DirectX::XMVECTOR Forward = { m4x4._31, m4x4._32, m4x4._33 };

	DirectX::XMVECTOR NPos = DirectX::XMLoadFloat4(&pos);

	DirectX::XMFLOAT3 axisUp = { 0,1,0 };
	DirectX::XMVECTOR AxisUp = DirectX::XMLoadFloat3(&axisUp);
	DirectX::XMFLOAT3 axisRight{};
	DirectX::XMVECTOR AxisRight = Right;
	DirectX::XMStoreFloat3(&axisRight, AxisRight);
	axisRight.y = 0;
	AxisRight = DirectX::XMLoadFloat3(&axisRight);


	/*mouse.SetPositionX(mouse.GetScreenWidth() / 2);
	mouse.SetPositionY(mouse.GetScreenHeight() / 2);*/

	int subX  = mouse.GetPositionX() - mouse.GetOldPositionX();
	int subY  = mouse.GetPositionY() - mouse.GetOldPositionY();

	//---------------------------------
	//camera direction
	//---------------------------------
	if (mouse.GetButton() & Mouse::BTN_RIGHT) {
		//if (gamePad.GetAxisRX() > 0.2f || (gamePad.GetButton() & GamePad::KEY_L))
		if (subX > 0)
		{
			//right
			DirectX::XMVECTOR Q = DirectX::XMQuaternionRotationAxis(AxisUp, cameraAngleSpeed);
			DirectX::XMVECTOR OQ = DirectX::XMQuaternionMultiply(Orientation, Q);
			Orientation = DirectX::XMQuaternionSlerp(Orientation, OQ, 0.1f);
		}
		//if (gamePad.GetAxisRX() < -0.2f || (gamePad.GetButton() & GamePad::KEY_J))
		if (subX < 0)
		{
			//left
			DirectX::XMVECTOR Q = DirectX::XMQuaternionRotationAxis(AxisUp, -cameraAngleSpeed);
			DirectX::XMVECTOR OQ = DirectX::XMQuaternionMultiply(Orientation, Q);
			Orientation = DirectX::XMQuaternionSlerp(Orientation, OQ, 0.1f);
		}
		//if (gamePad.GetAxisRY() > 0.2f || (gamePad.GetButton() & GamePad::KEY_I))
		if (subY < 0)
		{
			//up
			DirectX::XMVECTOR Q = DirectX::XMQuaternionRotationAxis(AxisRight, -cameraAngleSpeed);
			DirectX::XMVECTOR OQ = DirectX::XMQuaternionMultiply(Orientation, Q);
			Orientation = DirectX::XMQuaternionSlerp(Orientation, OQ, 0.1f);
		}
		//if (gamePad.GetAxisRY() < -0.2f || (gamePad.GetButton() & GamePad::KEY_K))
		if (subY > 0)
		{
			//down
			DirectX::XMVECTOR Q = DirectX::XMQuaternionRotationAxis(AxisRight, cameraAngleSpeed);
			DirectX::XMVECTOR OQ = DirectX::XMQuaternionMultiply(Orientation, Q);
			Orientation = DirectX::XMQuaternionSlerp(Orientation, OQ, 0.1f);
		}
	}

	DirectX::XMStoreFloat4(&cameraOrientation, Orientation);

	DirectX::XMStoreFloat4(&position, NPos);
	DirectX::XMVECTOR Target = NPos + Forward;
	DirectX::XMFLOAT4 target{};
	DirectX::XMStoreFloat4(&target, Target);
	focus = target;
	DirectX::XMFLOAT4 up{};
	DirectX::XMStoreFloat4(&up, AxisUp);
	this->up = up;

}


void camera::set_view(const DirectX::XMFLOAT4& eye, const DirectX::XMFLOAT4& focus, DirectX::XMFLOAT4& up)
{
	DirectX::XMVECTOR Eye = DirectX::XMLoadFloat4(&eye);
	DirectX::XMVECTOR Focus = DirectX::XMLoadFloat4(&focus);
	DirectX::XMVECTOR Up = DirectX::XMLoadFloat4(&up);
	DirectX::XMMATRIX View = DirectX::XMMatrixLookAtLH(Eye, Focus, Up);
	DirectX::XMStoreFloat4x4(&camera_constants_buffer->data.view, View);

	// Inverse the view and convert it back to a world matrix
	//ビューを逆行列化し、ワールド行列に戻す
	DirectX::XMMATRIX World = DirectX::XMMatrixInverse(nullptr, View);
	DirectX::XMFLOAT4X4 world;
	DirectX::XMStoreFloat4x4(&world, World);

	//Take out the camera direction
	//カメラの方向を取り出す
	this->right.x = world._11;
	this->right.y = world._12;
	this->right.z = world._13;

	this->up.x = world._21;
	this->up.y = world._22;
	this->up.z = world._23;

	this->front.x = world._31;
	this->front.y = world._32;
	this->front.z = world._33;

	//x11 12 13 14
	//u21 22 23 24
	//f31 32 33 34
	//41 42 43 44

	this->eye = eye;
	this->focus = focus;
}

//debug imgui
void camera::DrawDebugGUI(bool flag)
{
	if (flag) {
		ImGui::SetNextWindowPos(ImVec2(10, 10), ImGuiCond_FirstUseEver);
		ImGui::SetNextWindowSize(ImVec2(300, 300), ImGuiCond_FirstUseEver);

		if (ImGui::Begin("Camera", nullptr, ImGuiWindowFlags_None))
		{
			//transform
			if (ImGui::CollapsingHeader("CameraTransform", ImGuiTreeNodeFlags_DefaultOpen))
			{
				//position
				ImGui::SliderFloat3("Position", &position.x, -100, 100);
				//focus
				ImGui::SliderFloat3("camerafocus", &focus.x, -180.0f, 180.0f);

			}
		}
		ImGui::End();
	}
}

