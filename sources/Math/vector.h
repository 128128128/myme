#pragma once
#include <DirectXMath.h>
#include "math.h"
class Matrix;


/// <summary>
/// 2D vector class.
/// </summary>
class Vector2 {
public:
	static const Vector2 Zero;
	/// <summary>
	/// コンストラクタ。
	/// </summary>
	explicit Vector2()
	{
		x = 0.0f;
		y = 0.0f;
	}
	/// <summary>
	/// constracta
	/// </summary>
	/// <param name="x"></param>
	/// <param name="y"></param>
	Vector2(float x, float y)
	{
		this->x = x;
		this->y = y;
	}

	/// <summary>
	/// substitution operator : 代入演算子
	/// </summary>
	/// <param name="_v"></param>
	/// <returns></returns>
	Vector2& operator=(const Vector2& _v)
	{
		vec = _v.vec;
		return *this;
	}
	void Set(float _x, float _y)
	{
		vec.x = _x;
		vec.y = _y;
	}
	union {
		DirectX::XMFLOAT2 vec;
		struct { float x, y; };
		float v[2];
	};

	/// <summary>
	/// linear completion : 線形補完。
	/// </summary>
	/// <param name="t">completion ratio : 補完率</param>
	/// <param name="v0">Completion start vector : 補完開始のベクトル。</param>
	/// <param name="v1">Completion end vector : 補完終了のベクトル。</param>
	void lerp(float t, const Vector2& v0, const Vector2& v1)
	{
		x = v0.x + (v1.x - v0.x) * t;
		y = v0.y + (v1.y - v0.y) * t;
	}
	/// <summary>
	/// normalize
	/// </summary>
	void normalize()
	{
		DirectX::XMVECTOR xmv = DirectX::XMLoadFloat2(&vec);
		xmv = DirectX::XMVector2Normalize(xmv);
		DirectX::XMStoreFloat2(&vec, xmv);
	}
};
/// <summary>
/// 3D vector class.
/// </summary>
class Vector3{
public:
	union{
		DirectX::XMFLOAT3 vec;
		float v[3];
		struct { float x, y, z; };
	};
	static const Vector3 Zero;
	static const Vector3 Right;
	static const Vector3 Left;
	static const Vector3 Up;
	static const Vector3 Down;
	static const Vector3 Front;
	static const Vector3 Back;
	static const Vector3 AxisX;
	static const Vector3 AxisY;
	static const Vector3 AxisZ;
	static const Vector3 One;
public:
	/// <summary>
	/// Implicit conversion to XMVECTOR. : XMVECTORへの暗黙の変換。
	/// </summary>
	/// <returns></returns>
	operator DirectX::XMVECTOR() const
	{
		return DirectX::XMLoadFloat3(&vec);
	}
	/// <summary>
	/// Assignment operator. : 代入演算子。
	/// </summary>
	/// <param name="_v"></param>
	/// <returns></returns>
	Vector3& operator=(const Vector3& _v)
	{
		vec = _v.vec;
		return *this;
	}
	/// <summary>
	/// Constructor.
	/// </summary>
	/// <remarks>
	/// x,y,z全ての要素0で初期化されます。
	/// </remarks>
	explicit Vector3()
	{
		x = y = z = 0.0f;
	}
	/// <summary>
	/// Constructor.
	/// </summary>
	/// <param name="x"></param>
	/// <param name="y"></param>
	/// <param name="z"></param>
	Vector3(float x, float y, float z)
	{
		set(x, y, z);
	}
	/// <summary>
	/// linear completion : 線形補完
	/// </summary>
	/// <remarks>
	/// this = v0 + (v1-v0) * t;
	/// </remarks>
	/// <param name="t">Completion rate. : 補完率。</param>
	/// <param name="v0">completion start vector. : 補完開始のベクトル。</param>
	/// <param name="v1">completion end vector : 補完終了のベクトル。</param>
	void lerp(float t, const Vector3& v0, const Vector3& v1)
	{
		DirectX::XMVECTOR _v = DirectX::XMVectorLerp(
			DirectX::XMLoadFloat3(&v0.vec),
			DirectX::XMLoadFloat3(&v1.vec),
			t);
		DirectX::XMStoreFloat3(&vec, _v);
	}
	/// <summary>
	/// copy vector
	/// </summary>
	template<class TVector>
	void copy_to(TVector& dst) const
	{
		dst.x = x;
		dst.y = y;
		dst.z = z;
	}
	/// <summary>
	/// setting vector
	/// </summary>
	/// <param name="_x"></param>
	/// <param name="_y"></param>
	/// <param name="_z"></param>
	void set(float _x, float _y, float _z)
	{
		vec.x = _x;
		vec.y = _y;
		vec.z = _z;
	}
	template<class TVector>
	void set(TVector& _v)
	{
		set(_v.x, _v.y, _v.z);
	}
		
	/// <summary>
	/// Vector addition. :  ベクトルの加算。
	/// </summary>
	/// <remarks>
	/// this += _v;
	/// </remarks>
	/// <param name="_v">Vector to be added : 加算されるベクトル。</param>
	void add( const Vector3& _v) 
	{
		DirectX::XMVECTOR xmv0 = DirectX::XMLoadFloat3(&vec);
		DirectX::XMVECTOR xmv1 = DirectX::XMLoadFloat3(&_v.vec);
		DirectX::XMVECTOR xmvr = DirectX::XMVectorAdd(xmv0, xmv1);
		DirectX::XMStoreFloat3(&vec, xmvr);
	}
	/// <summary>
	/// Vector addition : ベクトルの加算。
	/// </summary>
	/// <remarks>
	/// this = v0 + v1;
	/// </remarks>
	void add( const Vector3& v0, const Vector3& v1 )
	{
		DirectX::XMVECTOR xmv0 = DirectX::XMLoadFloat3(&v0.vec);
		DirectX::XMVECTOR xmv1 = DirectX::XMLoadFloat3(&v1.vec);
		DirectX::XMVECTOR xmvr = DirectX::XMVectorAdd(xmv0, xmv1);
		DirectX::XMStoreFloat3(&vec, xmvr);
	}
	/// <summary>
	/// Vector subtraction. : ベクトルの減算。
	/// </summary>
	/// <remarks>
	/// this -= _v;
	/// </remarks>
	void subtract( const Vector3& _v )
	{
		DirectX::XMVECTOR xmv0 = DirectX::XMLoadFloat3(&vec);
		DirectX::XMVECTOR xmv1 = DirectX::XMLoadFloat3(&_v.vec);
		DirectX::XMVECTOR xmvr = DirectX::XMVectorSubtract(xmv0, xmv1);
		DirectX::XMStoreFloat3(&vec, xmvr);
	}
	/// <summary>
	/// Vector subtraction. : ベクトルの減算。
	/// </summary>
	/// <remarks>
	/// this = v0 - v1;
	/// </remarks>
	void subtract( const Vector3& v0, const Vector3& v1 )
	{
		DirectX::XMVECTOR xmv0 = DirectX::XMLoadFloat3(&v0.vec);
		DirectX::XMVECTOR xmv1 = DirectX::XMLoadFloat3(&v1.vec);
		DirectX::XMVECTOR xmvr = DirectX::XMVectorSubtract(xmv0, xmv1);
		DirectX::XMStoreFloat3(&vec, xmvr);
	}

	/// <summary>
	/// Calculate the inner product. : 内積を計算。
	/// </summary>
	/// <remarks>
	/// float d = this->x * _v.x + this->y * _v.y + this->z * _v.z;
	/// return d;
	/// </remarks>
	float dot( const Vector3& _v ) const
	{
		DirectX::XMVECTOR xmv0 = DirectX::XMLoadFloat3(&vec);
		DirectX::XMVECTOR xmv1 = DirectX::XMLoadFloat3(&_v.vec);
		return DirectX::XMVector3Dot(xmv0, xmv1).m128_f32[0];
	}

	/// <summary>
	/// cross
	/// </summary>
	/// <remarks>
	/// Vector3 v ;
	/// v.x = this->y * _v.z - this->z * _v.y;
	/// v.y = thiz->z * _v.x - this->x * _v.z;
	/// v.z = this->x * _v.y - this->y * _v.x;
	/// this = v;
	/// </remarks>
	void cross(const Vector3& _v)
	{
		DirectX::XMVECTOR xmv0 = DirectX::XMLoadFloat3(&vec);
		DirectX::XMVECTOR xmv1 = DirectX::XMLoadFloat3(&_v.vec);
		DirectX::XMVECTOR xmvr = DirectX::XMVector3Cross(xmv0, xmv1);
		DirectX::XMStoreFloat3(&vec, xmvr);
	}
	/// <summary>
	///cross。
	/// </summary>
	/// <remarks>
	/// this->x = v0.y * v1.z - v0.z * v1.y;
	/// this->y = v0.z * v1.x - v0.x * v1.z;
	/// this->z = v0.x * v1.y - v0.y * v1.x; 
	/// </remarks>
	void cross(const Vector3& v0, const Vector3& v1)
	{
		DirectX::XMVECTOR xmv0 = DirectX::XMLoadFloat3(&v0.vec);
		DirectX::XMVECTOR xmv1 = DirectX::XMLoadFloat3(&v1.vec);
		DirectX::XMVECTOR xmvr = DirectX::XMVector3Cross(xmv0, xmv1);
		DirectX::XMStoreFloat3(&vec, xmvr);
	}
	/// <summary>
	/// vector length
	/// </summary>
	float length() const
	{
		DirectX::XMVECTOR xmv = DirectX::XMLoadFloat3(&vec);
		return DirectX::XMVector3Length(xmv).m128_f32[0];
	}
	/// <summary>
	/// Get the square of the vector length. : ベクトルの長さの二乗を取得。
	/// </summary>
	float length_sq() const
	{
		DirectX::XMVECTOR xmv = DirectX::XMLoadFloat3(&vec);
		return DirectX::XMVector3LengthSq(xmv).m128_f32[0];
	}

	/// <summary>
	/// Scalar expansion of vectors. : ベクトルをスカラーで拡大。
	/// </summary>
	/// <remarks>
	/// 下記のような処理が行われています。
	/// this->x *= s;
	/// this->y *= s;
	/// this->z *= s;
	/// </remarks>
	void scale(float s)
	{
		DirectX::XMVECTOR xmv = DirectX::XMLoadFloat3(&vec);
		xmv = DirectX::XMVectorScale(xmv, s);
		DirectX::XMStoreFloat3(&vec, xmv);
	}

	/// <summary>
	/// normalize vector
	/// </summary>
	/// <remarks>
	///		下記のような処理が行われています。
	///		//ベクトルの大きさを求める。
	///		float len = this->x * this->x + this->y * this->y + this->z * this->z;
	///		len = sqrt(len);
	///		//ベクトルの大きさで、各要素を除算する。
	///		this->x /= len;
	///		this->y /= len;
	///		this->z /= len;
	/// </remarks>
	void normalize()
	{
		DirectX::XMVECTOR xmv = DirectX::XMLoadFloat3(&vec);
		xmv = DirectX::XMVector3Normalize(xmv);
		DirectX::XMStoreFloat3(&vec, xmv);
	}
	/// <summary>
	/// Divide a vector by a scalar. : ベクトルをスカラーで除算。
	/// </summary>
	/// <remarks>
	///		下記のような処理が行われています。
	///		this->x /= d;
	///		this->y /= d;
	///		this->z /= d;
	/// </remarks>
	void div(float d)
	{
		float fscale = 1.0f / d;
		scale(fscale);
	}

	/// <summary>
	/// 最大値を設定。
	/// </summary>
	/// <remarks>
	/// 下記のような処理が行われています。
	/// this->x = max( this->x, v.x );
	/// this->y = max( this->y, v.y );
	/// this->z = max( this->z, v.z );
	/// </remarks>
	void Max(const Vector3& v)
	{
		DirectX::XMVECTOR xmv0 = DirectX::XMLoadFloat3(&vec);
		DirectX::XMVECTOR xmv1 = DirectX::XMLoadFloat3(&v.vec);
		DirectX::XMStoreFloat3(&vec,  DirectX::XMVectorMax(xmv0, xmv1));
	}

	/// <summary>
	/// Set minimum value. : 最小値を設定。
	/// </summary>
	/// <remarks>
	/// 下記のような処理が行われています。
	/// this->x = min( this->x, v.x );
	/// this->y = min( this->y, v.y );
	/// this->z = min( this->z, v.z );
	/// </remarks>
	void Min(const Vector3& v)
	{
		DirectX::XMVECTOR xmv0 = DirectX::XMLoadFloat3(&vec);
		DirectX::XMVECTOR xmv1 = DirectX::XMLoadFloat3(&v.vec);
		DirectX::XMStoreFloat3(&vec, DirectX::XMVectorMin(xmv0, xmv1));
	}

	/// <summary>
	/// Additive assignment operator. : 加算代入演算子。
	/// </summary>
	/// <remarks>
	///	ベクトルクラスに下記のような加算代入の機能を提供します。
	///	Vector3 v0 = {20.0f, 30.0f, 40.0f};
	///	Vector3 v1 = {10.0f, 20.0f, 30.0f};
	///	v0 += v1;
	/// </remarks>
	const Vector3& operator+=(const Vector3& _v)
	{
		add(_v);
		return *this;
	}
	/// <summary>
	/// Multiply assignment operator. : 乗算代入演算子。
	/// </summary>
	/// <remarks>
	///	ベクトルクラスに下記のような乗算代入演算の機能を提供します。
	/// Vector3 v = {20.0f, 30.0f, 40.0f};
	///	v *= 10;
	/// </remarks>
	const Vector3& operator*=(float s) 
	{
		scale(s);
		return *this;
	}
	/// <summary>
	/// Subtraction assignment operator : 減算代入演算子。
	/// </summary>
	/// <remarks>
	/// ベクトルクラスに下記のような減算代入演算の機能を提供します。
	/// Vector3 v0 = {20.0f, 30.0f, 40.0f};
	///	Vector3 v1 = {10.0f, 20.0f, 30.0f};
	///	v0 -= v1;
	/// </remarks>
	const Vector3& operator-=(const Vector3& _v)
	{
		subtract(_v);
		return *this;
	}
	/// <summary>
	///  Division assignment operator. : 除算代入演算子。
	/// </summary>
	/// <remarks>
	/// ベクトルクラスに下記のような除算代入演算の機能を提供します。
	/// Vector3 v = {20.0f, 30.0f, 40.0f};
	///	v /= 10;
	/// </remarks>
	const Vector3& operator/=(const float s)
	{
		div(s);
		return *this;
	}
};
/// <summary>
///  4D vector class.
/// </summary>
class Vector4{
public:
	union{
		DirectX::XMFLOAT4 vec;
		struct { float x, y, z, w; };
		struct { float r, g, b, a; };
		float v[4];
	};
	static const Vector4 White;
	static const Vector4 Black;
	static const Vector4 Yellow;
	static const Vector4 Gray;
public:
	/// <summary>
	/// XMVECTORへの暗黙の型変換。
	/// </summary>
	/// <returns></returns>
	operator DirectX::XMVECTOR() const
	{
		return DirectX::XMLoadFloat4(&vec);
	}
	/// <summary>
	///  Constructor.
	/// </summary>
	/// <remarks>
	/// x,y,zが0.0、wは1.0で初期化されます。
	/// </remarks>
	explicit Vector4()
	{
		x = y = z = 0.0f;
		w = 1.0f;
	}
	/// <summary>
	///copy vector
	/// </summary>
	template<class TVector4>
	void copy_to(TVector4& dst) const
	{
		dst.x = x;
		dst.y = y;
		dst.z = z;
		dst.w = w;
	}
	
	/// <summary>
	/// Assignment operator. : 代入演算子。
	/// </summary>
	/// <remarks>
	/// ベクトルクラスに下記のような代入演算の機能を提供します。
	/// Vector4 v0 = {10.0f, 20.0f, 30.0f, 10.0f};
	/// Vector4 v1;
	/// v1 = v0;
	/// </remarks>
	Vector4& operator=(const Vector4& _v)
	{
		vec = _v.vec;
		return *this;
	}

	/// <summary>
	/// Constructor.
	/// </summary>
	Vector4(float x, float y, float z, float w)
	{
		set(x, y, z, w);
	}
	/// <summary>
	/// 3次元のベクトルデータを引数で受け取るコンストラクタ
	/// </summary>
	/// <remarks>
	/// wには1.0が格納されます。
	/// </remarks>
	Vector4(const Vector3& v)
	{
		set(v);
	}
	/// <summary>
	///Set each element of the vector.:  ベクトルの各要素を設定。
	/// </summary>
	void set(float _x, float _y, float _z, float _w)
	{
		this->x = _x;
		this->y = _y;
		this->z = _z;
		this->w = _w;
	}
	/// <summary>
	/// normalize。
	/// </summary>
	void normalize()
	{
		DirectX::XMVECTOR xmv = DirectX::XMLoadFloat4(&vec);
		xmv = DirectX::XMVector4Normalize(xmv);
		DirectX::XMStoreFloat4(&vec, xmv);
	}
	/// <summary>
	/// setting vector
	/// </summary>
	/// <param name="_v"></param>
	void set(const Vector4& _v)
	{
		*this = _v;
	}
	/// <summary>
	/// Vectors are set based on 3D vector data. : 3次元ベクトルデータをもとに、ベクトルを設定。
	/// </summary>
	/// <remarks>
	/// wには1.0が設定されます。
	/// </remarks>
	void set(const Vector3& _v)
	{
		this->x = _v.x;
		this->y = _v.y;
		this->z = _v.z;
		this->w = 1.0f;
	}
	/// <summary>
	/// addition vector
	/// </summary>
	/// <remarks>
	/// this += _v;
	/// </remarks>
	void add( const Vector4& _v )
	{
		DirectX::XMVECTOR xmv0 = DirectX::XMLoadFloat4(&vec);
		DirectX::XMVECTOR xmv1 = DirectX::XMLoadFloat4(&_v.vec);
		DirectX::XMVECTOR xmvr = DirectX::XMVectorAdd(xmv0, xmv1);
		DirectX::XMStoreFloat4(&vec, xmvr);
	}
	/// <summary>
	/// addition vector
	/// </summary>
	/// <remarks>
	/// this = v0 + v1;
	/// </remarks>
	void add( const Vector4& v0, const Vector4& v1 )
	{
		DirectX::XMVECTOR xmv0 = DirectX::XMLoadFloat4(&v0.vec);
		DirectX::XMVECTOR xmv1 = DirectX::XMLoadFloat4(&v1.vec);
		DirectX::XMVECTOR xmvr = DirectX::XMVectorAdd(xmv0, xmv1);
		DirectX::XMStoreFloat4(&vec, xmvr);
	}
	/// <summary>
	/// Vector subtraction. : ベクトルの減算。
	/// </summary>
	/// <remarks>
	/// this -= _v;
	/// </remarks>
	void subtract( const Vector4& _v )
	{
		DirectX::XMVECTOR xmv0 = DirectX::XMLoadFloat4(&vec);
		DirectX::XMVECTOR xmv1 = DirectX::XMLoadFloat4(&_v.vec);
		DirectX::XMVECTOR xmvr = DirectX::XMVectorSubtract(xmv0, xmv1);
		DirectX::XMStoreFloat4(&vec, xmvr);
	}
	/// <summary>
	/// Vector subtraction.
	/// </summary>
	/// <remarks>
	/// this = v0 - v1;
	/// </remarks>
	void subtract( const Vector4& v0, const Vector4& v1 )
	{
		DirectX::XMVECTOR xmv0 = DirectX::XMLoadFloat4(&v0.vec);
		DirectX::XMVECTOR xmv1 = DirectX::XMLoadFloat4(&v1.vec);
		DirectX::XMVECTOR xmvr = DirectX::XMVectorSubtract(xmv0, xmv1);
		DirectX::XMStoreFloat4(&vec, xmvr);
	}

	/// <summary>
	/// dot
	/// </summary>
	/// <remarks>
	/// float d = this->x * _v.x + this->y * _v.y + this->z * _v.z + this->w * _v.w;
	/// return d;
	/// </remarks>
	float dot( const Vector4& _v ) const
	{
		DirectX::XMVECTOR xmv0 = DirectX::XMLoadFloat4(&vec);
		DirectX::XMVECTOR xmv1 = DirectX::XMLoadFloat4(&_v.vec);
		return DirectX::XMVector4Dot(xmv0, xmv1).m128_f32[0];
	}
	/// <summary>
	/// vector length
	/// </summary>
	float length()
	{
		DirectX::XMVECTOR xmv = DirectX::XMLoadFloat4(&vec);
		return DirectX::XMVector4Length(xmv).m128_f32[0];
	}
	/// <summary>
	/// Get vector length squared
	/// </summary>
	float length_sq()
	{
		DirectX::XMVECTOR xmv = DirectX::XMLoadFloat4(&vec);
		return DirectX::XMVector4LengthSq(xmv).m128_f32[0];
	}
	/// <summary>
	/// Scalar expansion of vectors. : ベクトルをスカラーで拡大。
	/// </summary>
	/// <remarks>
	/// 下記のような処理が行われています。
	/// this->x *= s;
	/// this->y *= s;
	/// this->z *= s;
	/// this->w *= s;
	/// </remarks>
	void scale(float s)
	{
		DirectX::XMVECTOR xmv = DirectX::XMLoadFloat4(&vec);
		xmv = DirectX::XMVectorScale(xmv, s);
		DirectX::XMStoreFloat4(&vec, xmv);
	}
};

const Vector2 g_vec2Zero = { 0.0f, 0.0f };
const Vector3 g_vec3Zero = { 0.0f,  0.0f,  0.0f };
const Vector3 g_vec3Right = { 1.0f,  0.0f,  0.0f };
const Vector3 g_vec3Left = { -1.0f,  0.0f,  0.0f };
const Vector3 g_vec3Up = { 0.0f,  1.0f,  0.0f };
const Vector3 g_vec3Down = { 0.0f, -1.0f,  0.0f };
const Vector3 g_vec3Front = { 0.0f,   0.0f,  1.0f };
const Vector3 g_vec3Back = { 0.0f,   0.0f, -1.0f };
const Vector3 g_vec3AxisX = { 1.0f,  0.0f,  0.0f };
const Vector3 g_vec3AxisY = { 0.0f,  1.0f,  0.0f };
const Vector3 g_vec3AxisZ = { 0.0f,  0.0f,  1.0f };
const Vector3 g_vec3One = { 1.0f, 1.0f, 1.0f };

const Vector4 g_vec4White = { 1.0f, 1.0f, 1.0f, 1.0f };
const Vector4 g_vec4Black = { 0.0f, 0.0f, 0.0f, 1.0f };
const Vector4 g_vec4Gray = { 0.5f, 0.5f, 0.5f, 1.0f };
const Vector4 g_vec4Yellow = { 1.0f, 1.0f, 0.0f, 1.0f };

/// <summary>
/// Quaternion class
/// </summary>
class Quaternion : public Vector4{
public:
	static const Quaternion Identity;		//!<単位クォータニオン。
	Quaternion() 
	{
		x = y = z = 0.0f;
		w = 1.0f;
	}
	/// <summary>
	/// コンストラクタ。
	/// </summary>
	Quaternion(float x, float y, float z, float w) :
		Vector4(x, y, z, w)
	{
	}
	/// <summary>
	/// X軸周りの回転クォータニオンを作成。
	/// </summary>
	/// <param name="axis"></param>
	/// <param name="angle"></param>
	void set_rotationX(float angle)
	{
		set_rotation(g_vec3AxisX, angle);
	}
	/// <summary>
	/// X軸周りの回転クォータニオンを作成。
	/// </summary>
	/// <param name="angle">回転角度。単位Degree</param>
	void set_rotation_degX(float angle)
	{
		set_rotation_deg(g_vec3AxisX, angle);
	}
	/// <summary>
	/// Y軸周りの回転クォータニオンを作成。
	/// </summary>
	/// <param name="axis"></param>
	/// <param name="angle"></param>
	void set_rotationY(float angle)
	{
		set_rotation(g_vec3AxisY, angle);
	}
	/// <summary>
	/// Y軸周りの回転クォータニオンを作成。
	/// </summary>
	/// <param name="angle">回転角度。単位Degree</param>
	void set_rotation_degY(float angle)
	{
		set_rotation_deg(g_vec3AxisY, angle);
	}

	/// <summary>
	/// Z軸周りの回転クォータニオンを作成。
	/// </summary>
	/// <param name="axis"></param>
	/// <param name="angle"></param>
	void set_rotationZ(float angle)
	{
		set_rotation(g_vec3AxisZ, angle);
	}
	/// <summary>
	/// Z軸周りの回転クォータニオンを作成。
	/// </summary>
	/// <param name="angle">回転角度。単位Degree</param>
	void set_rotation_degZ(float angle)
	{
		set_rotation_deg(g_vec3AxisZ, angle);
	}

	

	/// <summary>
	/// 任意の軸周りの回転クォータニオンを作成。
	/// </summary>
	/// <param name="axis">回転軸</param>
	/// <param name="angle">回転角度。単位ラジアン。</param>
	void set_rotation( const Vector3& axis, float angle )
	{
		float s;
		float halfAngle = angle * 0.5f;
		s = sinf(halfAngle);
		w = cosf(halfAngle);
		x = axis.x * s;
		y = axis.y * s;
		z = axis.z * s;
	}
	/// <summary>
	/// 任意の軸周りの回転クォータニオンを作成。
	/// </summary>
	/// <param name="axis">回転軸</param>
	/// <param name="angle">回転角度。単位Degree</param>
	void set_rotation_deg(const Vector3& axis, float angle)
	{
		float s;
		float halfAngle = math::deg_to_rad(angle) * 0.5f;
		s = sinf(halfAngle);
		w = cosf(halfAngle);
		x = axis.x * s;
		y = axis.y * s;
		z = axis.z * s;
	}
	/// <summary>
	/// 行列からクォータニオンを作成。
	/// </summary>
	/// <param name="m">行列</param>
	void set_rotation(const Matrix& m);
	/// <summary>
	/// fromベクトルからtoベクトルに回転させるクォータニオンを作成。
	/// </summary>
	/// <param name="from">回転前のベクトル</param>
	/// <param name="to">回転後のベクトル</param>
	void set_rotation( Vector3 from,  Vector3 to);
	/// <summary>
	/// 球面線形補完
	/// </summary>
	/// <param name="t">補完率</param>
	/// <param name="q1">開始クォータニオン。</param>
	/// <param name="q2">終了クォータニオン。</param>
	void slerp(float t, Quaternion q1, Quaternion q2)
	{
		DirectX::XMVECTOR xmv = DirectX::XMQuaternionSlerp(
			DirectX::XMLoadFloat4(&q1.vec),
			DirectX::XMLoadFloat4(&q2.vec),
			t
		);
		DirectX::XMStoreFloat4(&vec, xmv);
	}
	/// <summary>
	/// Y軸周りの回転を加算。
	/// </summary>
	/// <returns>加算する回転角度。ラジアン単位。</returns>
	void add_rotationY(float angle)
	{
		Quaternion addRot;
		addRot.set_rotation(Vector3::AxisY, angle);
		*this *= addRot;
	}
	/// <summary>
	/// クォータニオン同士の乗算
	/// </summary>
	/// <param name="rot"></param>
	void multiply(const Quaternion& rot)
	{
		float pw, px, py, pz;
		float qw, qx, qy, qz;

		qw = w; qx = x; qy = y; qz = z;
		pw = rot.w; px = rot.x; py = rot.y; pz = rot.z;

		w = pw * qw - px * qx - py * qy - pz * qz;
		x = pw * qx + px * qw + py * qz - pz * qy;
		y = pw * qy - px * qz + py * qw + pz * qx;
		z = pw * qz + px * qy - py * qx + pz * qw;

	}
	/// <summary>
	/// クォータニオン同士の乗算。
	/// </summary>
	/// <param name="rot0"></param>
	/// <param name="rot1"></param>
	void multiply(const Quaternion& rot0, const Quaternion& rot1)
	{
		float pw, px, py, pz;
		float qw, qx, qy, qz;

		qw = rot0.w; qx = rot0.x; qy = rot0.y; qz = rot0.z;
		pw = rot1.w; px = rot1.x; py = rot1.y; pz = rot1.z;

		w = pw * qw - px * qx - py * qy - pz * qz;
		x = pw * qx + px * qw + py * qz - pz * qy;
		y = pw * qy - px * qz + py * qw + pz * qx;
		z = pw * qz + px * qy - py * qx + pz * qw;
	}
	/// <summary>
	/// クォータニオンの代入乗算演算子
	/// </summary>
	const Quaternion& operator*=(const Quaternion& rot0)
	{
		multiply(rot0, *this);
		return *this;
	}
	/// <summary>
	/// ベクトルにクォータニオンを適用する。
	/// </summary>
	void apply(Vector4& _v) const
	{
		DirectX::XMVECTOR xmv = DirectX::XMVector3Rotate(_v, *this);
		DirectX::XMStoreFloat4(&_v.vec, xmv);
	}
	/// <summary>
	/// ベクトルにクォータニオンを適用する。
	/// </summary>
	void apply(Vector3& _v) const
	{
		DirectX::XMVECTOR xmv = DirectX::XMVector3Rotate(_v, *this);
		DirectX::XMStoreFloat3(&_v.vec, xmv);
	}
};
	
//整数型のベクトルクラス。
__declspec(align(16)) class Vector4i {
public:
	union {
		struct { int x, y, z, w; };
		int v[4];
	};
};
/// <summary>
/// ベクトル同士の加算。
/// </summary>
static inline Vector3 operator+(const Vector3& v0, const Vector3& v1)
{
	Vector3 result;
	result.add(v0, v1);
	return result;
}
/// <summary>
/// ベクトルとスカラーの乗算。
/// </summary>

static inline Vector3 operator*(const Vector3& v, float s)
{
	Vector3 result;
	result = v;
	result.scale(s);
	return result;
}
/// <summary>
/// ベクトルとスカラーの除算。
/// </summary>
static inline Vector3 operator/(const Vector3& v, float s)
{
	Vector3 result;
	result = v;
	result.div(s);
	return result;
}
/// <summary>
/// ベクトル同士の減算。
/// </summary>
static inline Vector3 operator-(const Vector3& v0, const Vector3& v1)
{
	Vector3 result;
	result.subtract(v0, v1);
	return result;
}

/// <summary>
/// 外積を計算。
/// </summary>
static inline Vector3 Cross(const Vector3& v0, const Vector3& v1)
{
	Vector3 result;
	result.cross(v0, v1);
	return result;
}
/// <summary>
/// 内積を計算。
/// </summary>
static inline float Dot(const Vector3& v0, const Vector3& v1)
{
	return v0.dot(v1);
}

/// <summary>
/// ベクトル同士の加算。
/// </summary>

static inline Vector4 operator+(const Vector4& v0, const Vector4& v1)
{
	Vector4 result;
	result.add(v0, v1);
	return result;
}
/// <summary>
/// ベクトルとスカラーの乗算。
/// </summary>

static inline Vector4 operator*(const Vector4& v, float s)
{
	Vector4 result;
	result = v;
	result.scale(s);
	return result;
}

/// <summary>
/// ベクトル同士の減算。
/// </summary>
static inline Vector4 operator-(const Vector4& v0, const Vector4& v1)
{
	Vector4 result;
	result.subtract(v0, v1);
	return result;
}


/// <summary>
/// 内積を計算。
/// </summary>
static inline float Dot(const Vector4& v0, const Vector4& v1)
{
	return v0.dot(v1);
}


/// <summary>
/// クォータニオン同士の乗算。
/// </summary>
static inline Quaternion operator*(const Quaternion& q1, const Quaternion q2)
{
	Quaternion qRet;
	qRet.multiply(q2, q1);
	return qRet;
}

const Quaternion g_quatIdentity = { 0.0f,  0.0f, 0.0f, 1.0f };
