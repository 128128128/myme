#pragma once

#include<DirectXMath.h>
using namespace DirectX;

#include <vector>

#include "../Graphics/load/substance.h"

namespace Descartes
{
	namespace collision
	{
		int intersect_ray_aabb(const float* p, const float* d, const float* min, const float* max, float* q, float& tmin);

		// return value is index of intersected triangle / -1 is a meaning of failure
		int ray_triangles_intersection
		(
			const float* positions,	// model space
			const std::uint32_t stride, // byte size of one vertex
			const std::uint32_t* indices,
			const size_t number_of_indices,
			const XMFLOAT3& ray_position, // model space
			const XMFLOAT3& ray_direction, // model space 
			float ray_length_limit, // model space 
			XMFLOAT3* intersection // model space
		);

		struct bounding_box_for_triangle
		{
			XMFLOAT3 min;
			XMFLOAT3 max;
		};
		int ray_triangles_intersection_downward
		(
			const float* positions,	// model space
			const std::uint32_t stride, // byte size of one vertex
			const std::uint32_t* indices,
			const size_t number_of_indices,
			const XMFLOAT3& ray_position, // model space
			float ray_length_limit, // model space 
			XMFLOAT3* intersection // model space
		);

		struct vertex
		{
			XMFLOAT3 position;
			vertex(const Descartes::vertex& rhs)
			{
				position = rhs.position;
			}
			vertex() = default;

			template<class T>
			void serialize(T& archive)
			{
				archive
				(
					position.x, position.y, position.z
				);
			}
		};

		class mesh : public Descartes::substance<vertex>
		{
		public:
			mesh(const char* filename);
			~mesh() = default;
			mesh(mesh&) = delete;
			mesh& operator =(mesh&) = delete;

			const char* ray_triangles_intersection
			(
				//const char *mesh_name, // intersection target mesh [in]
				const XMFLOAT3& ray_position, // world space [in]
				const XMFLOAT3& ray_direction, // world space [in]
				const XMFLOAT4X4& world_transform, // model to world space [in]
				XMFLOAT3* intersection // world space [out]
			);
			const char* ray_triangles_intersection_downward
			(
				//const char *mesh_name, // intersection target mesh [in]
				const XMFLOAT3& ray_position, // world space [in]
				const XMFLOAT4X4& world_transform, // model to world space [in]
				XMFLOAT3* intersection // world space [out]
			);
		};
	}

	struct view_frustum
	{
		//DirectX::XMFLOAT4 faces[6];
		DirectX::XMFLOAT3 normal[6];
		DirectX::XMVECTOR vertex[8];
		DirectX::XMFLOAT3 nearP[4];
		DirectX::XMFLOAT3 farP[4];
		float distance[8];

		//ビュープロジェクション内の頂点算出用位置ベクトル
		DirectX::XMVECTOR verts[8] =
		{
			// near plane corners
			{ -1, -1, 0 }, // [0]:左下
			{  1, -1, 0 }, // [1]:右下
			{  1,  1, 0 }, // [2]:右上
			{ -1,  1 ,0 }, // [3]:左上

			// far plane corners.
			{ -1, -1, 1 }, // [4]:左下
			{  1, -1, 1 }, // [5]:右下
			{  1,  1, 1 }, // [6]:右上
			{ -1,  1, 1 }  // [7]:左上
		};

		view_frustum(const DirectX::XMFLOAT4X4& view_projection_matrix,const DirectX::XMFLOAT4X4 view_inverse_projection)
		{
			// 左側面
			normal[0] = { view_projection_matrix._14 + view_projection_matrix._11,view_projection_matrix._24 + view_projection_matrix._21,view_projection_matrix._34 + view_projection_matrix._31 };
			// 右側面
			normal[1] = { view_projection_matrix._14 - view_projection_matrix._11,view_projection_matrix._24 - view_projection_matrix._21,view_projection_matrix._34 - view_projection_matrix._31 };
			// 下側面
			normal[2] = { view_projection_matrix._14 + view_projection_matrix._12,view_projection_matrix._24 + view_projection_matrix._22,view_projection_matrix._34 + view_projection_matrix._32 };
			// 上側面
			normal[3] = { view_projection_matrix._14 - view_projection_matrix._12,view_projection_matrix._24 - view_projection_matrix._22,view_projection_matrix._34 - view_projection_matrix._32 };
			// 奥側面
			normal[4] = { -view_projection_matrix._14 - view_projection_matrix._13,-view_projection_matrix._24 - view_projection_matrix._23,-view_projection_matrix._34 - view_projection_matrix._33 };
			// 手前側面
			normal[5] = { -view_projection_matrix._14 + view_projection_matrix._13,-view_projection_matrix._24 + view_projection_matrix._23,-view_projection_matrix._34 + view_projection_matrix._33 };


			DirectX::XMVECTOR FrustumNormal[6]{};
			for (int i = 0; i < 6; i++)
			{
				FrustumNormal[i] = DirectX::XMVector3Normalize(DirectX::XMLoadFloat3(&normal[i]));
				DirectX::XMStoreFloat3(&normal[i], FrustumNormal[i]);
			}

			DirectX::XMMATRIX matrix = DirectX::XMLoadFloat4x4(&view_inverse_projection);
			DirectX::XMMATRIX inverse_view_projection_matrix = DirectX::XMMatrixInverse(nullptr, matrix);



			for(int i=0;i<8/2;i++)
			{
				vertex[i] = DirectX::XMVector3TransformCoord(verts[i], inverse_view_projection_matrix);
				DirectX::XMStoreFloat3(&nearP[i], vertex[i]);
			}
		    for(int i=8/2;i<8;i++)
			{
				vertex[i] = DirectX::XMVector3TransformCoord(verts[i], inverse_view_projection_matrix);
				DirectX::XMStoreFloat3(&farP[i-8/2], vertex[i]);
			}

			//原点からの距離
			DirectX::XMStoreFloat(&distance[0], DirectX::XMVector3Dot(FrustumNormal[0], vertex[0]));
			DirectX::XMStoreFloat(&distance[1], DirectX::XMVector3Dot(FrustumNormal[1], vertex[1]));
			DirectX::XMStoreFloat(&distance[2], DirectX::XMVector3Dot(FrustumNormal[2], vertex[1]));
			DirectX::XMStoreFloat(&distance[3], DirectX::XMVector3Dot(FrustumNormal[3], vertex[6]));
			DirectX::XMStoreFloat(&distance[4], DirectX::XMVector3Dot(FrustumNormal[4], vertex[4]));
			DirectX::XMStoreFloat(&distance[5], DirectX::XMVector3Dot(FrustumNormal[5], vertex[0]));

		}

		//view_frustum(const DirectX::XMFLOAT4X4& view_projection_matrix)
		//{
		//	// Left Frustum Plane
		//	DirectX::XMStoreFloat4(&faces[0], DirectX::XMVector3Normalize(DirectX::XMVectorSet(
		//		view_projection_matrix._14 + view_projection_matrix._11,
		//		view_projection_matrix._24 + view_projection_matrix._21,
		//		view_projection_matrix._34 + view_projection_matrix._31,
		//		view_projection_matrix._44 + view_projection_matrix._41)));

		//	// Right Frustum Plane
		//	DirectX::XMStoreFloat4(&faces[1], DirectX::XMVector3Normalize(DirectX::XMVectorSet(
		//		view_projection_matrix._14 - view_projection_matrix._11,
		//		view_projection_matrix._24 - view_projection_matrix._21,
		//		view_projection_matrix._34 - view_projection_matrix._31,
		//		view_projection_matrix._44 - view_projection_matrix._41)));

		//	// Top Frustum Plane
		//	DirectX::XMStoreFloat4(&faces[2], DirectX::XMVector3Normalize(DirectX::XMVectorSet(
		//		view_projection_matrix._14 - view_projection_matrix._12,
		//		view_projection_matrix._24 - view_projection_matrix._22,
		//		view_projection_matrix._34 - view_projection_matrix._32,
		//		view_projection_matrix._44 - view_projection_matrix._42)));

		//	// Bottom Frustum Plane
		//	DirectX::XMStoreFloat4(&faces[3], DirectX::XMVector3Normalize(DirectX::XMVectorSet(
		//		view_projection_matrix._14 + view_projection_matrix._12,
		//		view_projection_matrix._24 + view_projection_matrix._22,
		//		view_projection_matrix._34 + view_projection_matrix._32,
		//		view_projection_matrix._44 + view_projection_matrix._42)));

		//	// Near Frustum Plane
		//	DirectX::XMStoreFloat4(&faces[4], DirectX::XMVector3Normalize(DirectX::XMVectorSet(
		//		view_projection_matrix._13,
		//		view_projection_matrix._23,
		//		view_projection_matrix._33,
		//		view_projection_matrix._43)));

		//	// Far Frustum Plane
		//	DirectX::XMStoreFloat4(&faces[5], DirectX::XMVector3Normalize(DirectX::XMVectorSet(
		//		view_projection_matrix._14 - view_projection_matrix._12,
		//		view_projection_matrix._24 - view_projection_matrix._22,
		//		view_projection_matrix._34 - view_projection_matrix._32,
		//		view_projection_matrix._44 - view_projection_matrix._42)));
		//}

		// https://www.braynzarsoft.net/viewtutorial/q16390-34-aabb-cpu-side-frustum-culling
		static int intersect_frustum_aabb(const view_frustum& view_frustum, const DirectX::XMFLOAT3 bounding_box[2]);
		static bool judge_cross(const view_frustum& view_frustum, const DirectX::XMFLOAT3 bounding_box[2]);
	};
}