#include "light_camera.h"



void light_camera::update()
{
	// Calculate the aspect ratio.ÅB
	aspect = 1280.0f / 720.0f;
	if (is_need_update_projection_matrix) {
		if (m_updateProjMatrixFunc == enUpdateProjMatrixFunc_Perspective) {
			// Compute the perspective transformation matrix.
			projection_matrix.MakeProjectionMatrix(
				view_angle,
				aspect,
				near,
				far
			);
		}
		else {
			// Compute the parallel projection matrix.
			projection_matrix.MakeOrthoProjectionMatrix(width, height, near, far);
		}
	}
	//Calculation of view matrix
	view_matrix.MakeLookAt(position, target, up);
	// Creation of view projection matrices.
	view_projection_matrix = view_matrix * projection_matrix;
	// Compute the inverse of the view matrix.
	view_matrix_inv.Inverse(view_matrix);

	forward.set(view_matrix_inv.m[2][0], view_matrix_inv.m[2][1], view_matrix_inv.m[2][2]);
	right.set(view_matrix_inv.m[0][0], view_matrix_inv.m[0][1], view_matrix_inv.m[0][2]);
	//Get the rotation matrix of the camera.
	camera_rotation = view_matrix_inv;
	camera_rotation.m[3][0] = 0.0f;
	camera_rotation.m[3][1] = 0.0f;
	camera_rotation.m[3][2] = 0.0f;
	camera_rotation.m[3][3] = 1.0f;

	Vector3 toPos;
	toPos.subtract(position, target);
	target_to_position_len = toPos.length();

	is_dirty = false;
}
void light_camera::calc_screen_position_from_world_position(Vector2& screenPos, const Vector3& worldPos) const
{
	float half_w = 1280.0f * 0.5f;
	float half_h = 720.0f * 0.5f;
	Vector4 _screenPos;
	_screenPos.set(worldPos.x, worldPos.y, worldPos.z, 1.0f);
	view_projection_matrix.Apply(_screenPos);
	screenPos.x = (_screenPos.x / _screenPos.w) * half_w;
	screenPos.y = (_screenPos.y / _screenPos.w) * half_h;
}
void light_camera::rotate_origin_target(const Quaternion& qRot)
{
	Vector3 cameraPos = position;
	Vector3 cameraTarget = target;
	Vector3 toPos = cameraPos - cameraTarget;
	qRot.apply(toPos);
	position = target + toPos;
	is_dirty = true;
}
