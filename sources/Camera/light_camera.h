#pragma once
#include "../Camera/light_camera.h"
#include "../Math/vector.h"
#include "../Math/matrix.h"
#include "../Math/math.h"

/// <summary>
/// camera class
/// </summary>
class light_camera {
public:
	/// <summary>
	/// Method of updating the projection matrix.
	/// </summary>
	enum en_update_proj_matrix_func {
		enUpdateProjMatrixFunc_Perspective,		// perspective projection matrix.
	                                                                        //If you want to make a picture with perspective, this is the way to go.
		enUpdateProjMatrixFunc_Ortho,			//Parallel projection;
	                                                                //if you want 2D expression, this is the way to go.
	};
	/// <summary>
	/// Update view and projection matrices.
	/// </summary>
	void update();
	/// <summary>
	///	Rotate the camera with the gazing point as the origin.
	/// �����_�����_�Ƃ��ăJ��������]������B
	/// </summary>
	/// <param name="qRot">Rotating quaternion</param>
	void rotate_origin_target(const Quaternion& qRot);

	/// <summary>
	/// move camera
	/// </summary>
	/// <param name="move">move param</param>
	void move(const Vector3& move)
	{
		position += move;
		target += move;
		is_dirty = true;
	}
	/// <summary>
	/// move target
	/// </summary>
	/// <param name="move">Amount of movement</param>
	void move_target(const Vector3& move)
	{
		target += move;
		is_dirty = true;
	}
	/// <summary>
	/// Move the viewpoint.
	/// </summary>
	/// <param name="move"></param>
	void move_position(const Vector3& move)
	{
		position += move;
		is_dirty = true;
	}
	/// <summary>
	///Move to the forward of the camera.
	/// </summary>
	/// <param name="moveForward"></param>
	void move_forward(float moveForward)
	{
		move(forward * moveForward);
	}
	/// <summary>
	/// Move to the right of the camera.
	/// </summary>
	/// <param name="moveRight"></param>
	void move_right(float moveRight)
	{
		move(right * moveRight);
	}
	/// <summary>
	/// Move to the up of the camera.
	/// </summary>
	/// <param name="moveUp"></param>
	void move_up(float moveUp)
	{
		move(up * moveUp);
	}
	/// <summary>
	/// set position
	/// </summary>
	void set_position(const Vector3& pos)
	{
		position = pos;
		is_dirty = true;
	}
	void set_position(float x, float y, float z)
	{
		set_position({ x, y, z });
	}
	/// <summary>
	/// get pos
	/// </summary>
	const Vector3& get_position() const
	{
		return position;
	}
	/// <summary>
	/// set target
	/// </summary>
	void set_target(float x, float y, float z)
	{
		set_target({ x, y, z });
	}
	void set_target(const Vector3& ftarget)
	{
		target = ftarget;
		is_dirty = true;
	}
	/// <summary>
	/// get target
	/// </summary>
	const Vector3& get_target() const
	{
		return target;
	}
	/// <summary>
	/// set up
	/// </summary>
	void set_up(const Vector3& fup)
	{
		up = fup;
		up.normalize();
	}
	void set_up(float x, float y, float z)
	{
		set_up({ x, y, z });
	}
	/// <summary>
	/// get up
	/// </summary>
	const Vector3& get_up() const
	{
		return up;
	}

	/// <summary>
	/// get inverse matrix
	/// </summary>
	const Matrix& get_view_matrix_inv()
	{
		if (is_dirty) {
			//need this
			update();
		}
		return view_matrix_inv;
	}
	/// <summary>
	///get view matrix
	/// </summary>
	const Matrix& get_view_matrix()
	{
		if (is_dirty) {
			//need update
			update();
		}
		return view_matrix;
	}
	/// <summary>
	/// get projection matrix
	/// </summary>
	const Matrix& get_projection_matrix()
	{
		if (is_dirty) {
			update();
		}
		return projection_matrix;
	}
	/// <summary>
	/// get view projection
	/// </summary>
	const Matrix& get_view_projection_matrix()
	{
		if (is_dirty) {
			//need update
			update();
		}
		return view_projection_matrix;
	}
	/// <summary>
	/// get camera rotation
	/// </summary>
	const Matrix& get_camera_rotation()
	{
		if (is_dirty) {
			update();
		}
		return camera_rotation;
	}
	/// <summary>
	/// Set the distance to the far plane.
	/// </summary>
	void set_far(float fFar)
	{
		far = fFar;
		is_dirty = true;
	}
	/// <summary>
	/// Set the distance to the near plane.
	/// </summary>
	void set_near(float fNear)
	{
		near = fNear;
		is_dirty = true;
	}
	/// <summary>
	/// get the distance to the far plane.
	/// </summary>
	float get_far() const
	{
		return far;
	}
	/// <summary>
	/// get the distance to the near plane.
	/// </summary>
	float get_near() const
	{
		return near;
	}
	/// <summary>
	/// Set the width of the parallel projection.
	/// </summary>
	/// <remarks>
	/// SetUpdateProjMatrixFunc��enUpdateProjMatrixFunc_Ortho���ݒ肳��Ă���Ƃ��Ɏg�p�����B
	/// </remarks>
	void set_width(float w)
	{
		width = w;
		is_dirty = true;
	}
	/// <summary>
	/// Set the hieght of the parallel projection.
	/// </summary>
	/// <remarks>
	/// SetUpdateProjMatrixFunc��enUpdateProjMatrixFunc_Ortho���ݒ肳��Ă���Ƃ��Ɏg�p�����B
	/// </remarks>
	void set_height(float h)
	{
		height = h;
		is_dirty = true;
	}
	/// <summary>
	/// get the width of the parallel projection.
	/// </summary>
	float get_width() const
	{
		return width;
	}
	/// <summary>
	/// get the height of the parallel projection.
	/// </summary>
	float get_height() const
	{
		return height;
	}
	/// <summary>
	/// Set parallel projection height.
	/// </summary>
	/// <remarks>
	/// SetUpdateProjMatrixFunc��enUpdateProjMatrixFunc_Ortho���ݒ肳��Ă���Ƃ��Ɏg�p�����B
	/// </remarks>
	void set_update_proj_matrix_func(en_update_proj_matrix_func func)
	{
		m_updateProjMatrixFunc = func;
		is_dirty = true;
	}
	/// <summary>
	/// set view angle
	/// </summary>
	/// <remarks>
	/// SetUpdateProjMatrixFunc��enUpdateProjMatrixFunc_Perspective���ݒ肳��Ă���Ƃ��Ɏg�p�����B
	/// </remarks>
	/// <param name="viewAngle">view angle�B�P�ʃ��W�A��</param>
	void set_view_angle(float viewAngle)
	{
		view_angle = viewAngle;
		is_dirty = true;
	}
	/// <summary>
	/// get angle
	/// </summary>
	/// <returns>view angle�Bunit radian</returns>
	float get_view_angle() const
	{
		return view_angle;
	}
	/// <summary>
	/// get the distance between the gazing point and the viewpoint.
	/// </summary>
	/// <returns></returns>
	float get_target_to_position_length() const
	{
		return target_to_position_len;
	}
	/// <summary>
	/// get the camera's forward direction.
	/// </summary>
	const Vector3& get_forward() const
	{
		return forward;
	}
	/// <summary>
	/// get the camera's right direction.
	/// </summary>

	const Vector3& get_right() const
	{
		return right;
	}
	/// <summary>
	/// get aspect
	/// </summary>
	float get_aspect() const
	{
		return aspect;
	}
	/// <summary>
	/// create camera copy
	/// </summary>
	/// <param name="dst"></param>
	void copy_to(light_camera& dst)
	{
		memcpy(&dst, this, sizeof(dst));
		is_dirty = true;
	}
	/// <summary>
	/// Calculate screen coordinates from world coordinates.
	/// </summary>
	/// <remarks>
	/// �v�Z�����X�N���[�����W�͉�ʂ̒��S��{0,0}�A�����{��ʂ̕�*-0.5,��ʂ̍���*-0.5}
	/// �E����{ ��ʂ̕� * 0.5,��ʂ̍��� * 0.5 }�Ƃ�����W�n�B
	/// </remarks>
	/// <param name="screenPos">Where screen coordinates are stored : �X�N���[�����W�̊i�[��</param>
	/// <param name="worldPos">world position : ���[���h���W</param>
	void calc_screen_position_from_world_position(Vector2& screenPos, const Vector3& worldPos) const;

protected:
	float		target_to_position_len = 1.0f;			//�����_�Ǝ��_�܂ŋ��� : Distance to gazing point and viewpoint.
	Vector3		position = { 0.0f, 0.0f, 1.0f };		//camera position : �J�����ʒu�B
	Vector3		up = g_vec3Up;						//dir up : �J�����̏�����B
	Vector3		target;								//taget  : �J�����̒��~�_�B
	Matrix		view_matrix;							//view matrix : �r���[�s��B
	Matrix		projection_matrix;						//projection matrix : �v���W�F�N�V�����s��B
	Matrix		view_projection_matrix;					//view projection matrix : �r���[�v���W�F�N�V�����s��B
	Matrix		view_matrix_inv;						//inverse view matrix : �r���[�s��̋t�s��B
	Matrix		camera_rotation;						//rotation matrix : �J�����̉�]�s��B
	Vector3		forward = g_vec3Front;				//dir forward : �J�����̑O���B
	Vector3		right = g_vec3Right;					//dir right : �J�����̉E�B
	float		near = 1.0f;							//near plane
	float		far = 5000.0f;						//far plane : �����ʁB
	float		view_angle = math::deg_to_rad(60.0f);	//view angle(radian) :  ��p(���W�A��)�B
	float		aspect = 1.0f;						//Aspect ratio
	float		width = 1280.0f;						//Width used when creating the parallel projection matrix.: ���s���e�s����쐬����Ƃ��Ɏg�p����镝�B
	float		height = 720.0f;						//Height used when creating the parallel projection matrix. : ���s���e�s����쐬����Ƃ��Ɏg�p����鍂���B
	en_update_proj_matrix_func m_updateProjMatrixFunc = enUpdateProjMatrixFunc_Perspective;	//How to update the projection matrix.
	bool		is_need_update_projection_matrix = true;
	bool		is_dirty = false;						//Dirty Flag.
};

