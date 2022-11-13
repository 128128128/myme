#define NOMINMAX

#include "collision.h"

namespace Descartes
{
	inline bool point_in_aabb(const float *p, const float *min, const float *max)
	{
		return (p[0] >= min[0] && p[0] <= max[0]) && (p[1] >= min[1] && p[1] <= max[1]) && (p[2] >= min[2] && p[2] <= max[2]);
	}

	// Intersect ray R(t) = p + t*d against AABB a. When intersecting,
	// return intersection distance tmin and point q of intersection
	int collision::intersect_ray_aabb(const float *p, const float *d, const float *min, const float *max, float *q, float &tmin)
	{
		tmin = 0.0f; // set to -FLT_MAX to get first hit on line
		float tmax = FLT_MAX; // set to max distance ray can travel (for segment)

		// For all three slabs
		for (int i = 0; i < 3; i++)
		{
			if (fabsf(d[i]) < FLT_EPSILON)
			{
				// Ray is parallel to slab. No hit if origin not within slab
				if (p[i] < min[i] || p[i] > max[i]) return 0;
			}
			else
			{
				// Compute intersection t value of ray with near and far plane of slab
				float ood = 1.0f / d[i];
				float t1 = (min[i] - p[i]) * ood;
				float t2 = (max[i] - p[i]) * ood;
				// Make t1 be intersection with near plane, t2 with far plane
				if (t1 > t2) std::swap(t1, t2);
				// Compute the intersection of slab intersections intervals
				tmin = std::max<float>(tmin, t1);
				tmax = std::min<float>(tmax, t2);
				// Exit with no collision as soon as slab intersection becomes empty
				if (tmin > tmax) return 0;
			}
		}
		// Ray intersects all 3 slabs. Return point (q) and intersection t value (tmin) 
		if (q)
		{
			for (int i = 0; i < 3; i++)
			{
				q[i] = p[i] + d[i] * tmin;
			}
		}
		return 1;
	}

	// return value is index of intersected triangle / -1 is a meaning of failure
	int collision::ray_triangles_intersection
	(
		const float *positions,	// model space
		const std::uint32_t stride, // byte size of one vertex
		const std::uint32_t*indices,
		const size_t number_of_indices,
		const XMFLOAT3 &ray_position, // model space
		const XMFLOAT3 &ray_direction, // model space 
		float ray_length_limit, // model space 
		XMFLOAT3 *intersection // model space
	)
	{
		//bool RH = true; // Right handed coordinate system
		bool CCW = true; // Counterclockwise 3D triangle ABC
		const int c0 = 0;
		const int c1 = CCW ? 1 : 2;
		const int c2 = CCW ? 2 : 1;

		// Ray-triangle intersection 
		// In this handout, we explore the steps needed to compute the intersection of a ray with a triangle, and then to
		// compute the barycentric coordinates of that intersection.First, we consider the geometry of such an intersection :
		//
		// where a ray with origin P and direction D intersects a triangle defined by its vertices, A, B, and C at intersection
		// point Q.The square region diagrammatically surrounding triangle ABC represents the supporting plane of the
		// triangle, i.e., the plane that the triangle lies on.
		//
		// To perform ray - triangle intersection, we must perform two steps :
		//
		//  1. Determine the point of intersection, Q.
		//  2. Determine if Q lies inside of triangle ABC.
		//
		int number_of_intersections = 0;
		int index_of_intersected_triangle = -1;

		float shortest_distance = FLT_MAX;
		XMVECTOR X; // closest cross point
		XMVECTOR P = XMVectorSet(ray_position.x, ray_position.y, ray_position.z, 1);
		XMVECTOR D = XMVector3Normalize(XMVectorSet(ray_direction.x, ray_direction.y, ray_direction.z, 0));

		const uint8_t*p = reinterpret_cast<const uint8_t*>(positions);
		const size_t number_of_triangles = number_of_indices / 3;
		for (size_t index_of_triangle = 0; index_of_triangle < number_of_triangles; index_of_triangle++)
		{
			XMFLOAT3 p0 = XMFLOAT3(
				(reinterpret_cast<const float *>(p + indices[index_of_triangle * 3 + c0] * stride))[0],
				(reinterpret_cast<const float *>(p + indices[index_of_triangle * 3 + c0] * stride))[1],
				(reinterpret_cast<const float *>(p + indices[index_of_triangle * 3 + c0] * stride))[2]);
			XMFLOAT3 p1 = XMFLOAT3(
				(reinterpret_cast<const float *>(p + indices[index_of_triangle * 3 + c1] * stride))[0],
				(reinterpret_cast<const float *>(p + indices[index_of_triangle * 3 + c1] * stride))[1],
				(reinterpret_cast<const float *>(p + indices[index_of_triangle * 3 + c1] * stride))[2]);
			XMFLOAT3 p2 = XMFLOAT3(
				(reinterpret_cast<const float *>(p + indices[index_of_triangle * 3 + c2] * stride))[0],
				(reinterpret_cast<const float *>(p + indices[index_of_triangle * 3 + c2] * stride))[1],
				(reinterpret_cast<const float *>(p + indices[index_of_triangle * 3 + c2] * stride))[2]);

			XMVECTOR A, B, C;
			A = XMLoadFloat3(&p0);
			B = XMLoadFloat3(&p1);
			C = XMLoadFloat3(&p2);

			// Determining Q
			// Determining the point of intersection Q in turn requires us to take two more steps :
			//  1. Determine the equation of the supporting plane for triangle ABC.
			//  2. Intersect the ray with the supporting plane.
			// We’ll handle these steps in reverse order.
			XMVECTOR Q;

			// 1. Determine the equation of the supporting plane for triangle ABC.
			XMVECTOR N = XMVector3Normalize(XMVector3Cross(XMVectorSubtract(B, A), XMVectorSubtract(C, A)));
			float d = XMVectorGetByIndex(XMVector3Dot(N, A), 0);

			// 2. Intersect the ray with the supporting plane.
			float denominator = XMVectorGetByIndex(XMVector3Dot(N, D), 0);
			if (denominator < 0) // Note that if N.D = 0 , then D is parallel to the plane and the ray does not intersect the plane.
			{
				float numerator = d - XMVectorGetByIndex(XMVector3Dot(N, P), 0);
				float t = numerator / denominator;

				if (t > 0 && t < ray_length_limit) // Forward and Length limit of Ray
				{
					Q = XMVectorAdd(P, XMVectorScale(D, t));

					// Triangle inside - outside testing

					// CHRISTER ERICSON. "REAL-TIME COLLISION DETECTION" pp.204
					// Test if point Q lies inside the counterclockwise 3D triangle ABC

					// Translate point and triangle so that point lies at origin
					static const XMVECTOR Z = XMVectorSet(0, 0, 0, 1);
					static const XMVECTOR E = XMVectorSet(FLT_EPSILON, FLT_EPSILON, FLT_EPSILON, 1);

					XMVECTOR QA = XMVectorSubtract(A, Q);
					XMVECTOR QB = XMVectorSubtract(B, Q);
					XMVECTOR QC = XMVectorSubtract(C, Q);

					XMVECTOR U = XMVector3Cross(QB, QC);
					if (XMVector3NearEqual(U, Z, E))
					{
						U = Z;
					}
					XMVECTOR V = XMVector3Cross(QC, QA);
					if (XMVector3NearEqual(V, Z, E))
					{
						V = Z;
					}
					if (XMVectorGetByIndex(XMVector3Dot(U, V), 0) < 0)
					{
						continue;
					}

					XMVECTOR W = XMVector3Cross(QA, QB);
					if (XMVector3NearEqual(W, Z, E))
					{
						W = Z;
					}
					if (XMVectorGetByIndex(XMVector3Dot(U, W), 0) < 0)
					{
						continue;
					}
					if (XMVectorGetByIndex(XMVector3Dot(V, W), 0) < 0)
					{
						continue;
					}

					// Otherwise Q must be in (or on) the triangle
					if (t < shortest_distance)
					{
						shortest_distance = t;
						index_of_intersected_triangle = static_cast<int>(index_of_triangle);
						X = Q;

					}
					number_of_intersections++;
				}
			}
		}
		if (number_of_intersections > 0)
		{
			XMStoreFloat3(intersection, X);
		}
		return index_of_intersected_triangle;
	}

	int collision::ray_triangles_intersection_downward
	(
		const float *positions,	// model space
		const std::uint32_t stride, // byte
		const std::uint32_t*indices,
		const size_t number_of_indices,
		const XMFLOAT3 &ray_position, // model space
		float ray_length_limit, // model space 
		//const XMFLOAT4X4 &model_to_world_transform, // model to world transform
		XMFLOAT3 *intersection // model space
	)
	{
		//bool RH = true; // Right handed coordinate system
		bool CCW = true; // Counterclockwise 3D triangle ABC
		const int c0 = 0;
		const int c1 = CCW ? 1 : 2;
		const int c2 = CCW ? 2 : 1;

		// Ray-triangle intersection 
		// In this handout, we explore the steps needed to compute the intersection of a ray with a triangle, and then to
		// compute the barycentric coordinates of that intersection.First, we consider the geometry of such an intersection :
		//
		// where a ray with origin P and direction D intersects a triangle defined by its vertices, A, B, and C at intersection
		// point Q.The square region diagrammatically surrounding triangle ABC represents the supporting plane of the
		// triangle, i.e., the plane that the triangle lies on.
		//
		// To perform ray - triangle intersection, we must perform two steps :
		//
		//  1. Determine the point of intersection, Q.
		//  2. Determine if Q lies inside of triangle ABC.
		//
		int number_of_intersections = 0;
		int index_of_intersected_triangle = -1;

		float shortest_distance = FLT_MAX;
		XMVECTOR X; // closest cross Point
		XMVECTOR P = XMVectorSet(ray_position.x, ray_position.y, ray_position.z, 1);
		XMVECTOR D = XMVector3Normalize(XMVectorSet(0, -1, 0, 0));

		const size_t number_of_triangles = number_of_indices / 3;

		const uint8_t*p = reinterpret_cast<const uint8_t*>(positions);
		for (size_t index_of_triangle = 0; index_of_triangle < number_of_triangles; index_of_triangle++)
		{
			XMFLOAT3 p0 = XMFLOAT3(
				(reinterpret_cast<const float *>(p + indices[index_of_triangle * 3 + c0] * stride))[0],
				(reinterpret_cast<const float *>(p + indices[index_of_triangle * 3 + c0] * stride))[1],
				(reinterpret_cast<const float *>(p + indices[index_of_triangle * 3 + c0] * stride))[2]);
			XMFLOAT3 p1 = XMFLOAT3(
				(reinterpret_cast<const float *>(p + indices[index_of_triangle * 3 + c1] * stride))[0],
				(reinterpret_cast<const float *>(p + indices[index_of_triangle * 3 + c1] * stride))[1],
				(reinterpret_cast<const float *>(p + indices[index_of_triangle * 3 + c1] * stride))[2]);
			XMFLOAT3 p2 = XMFLOAT3(
				(reinterpret_cast<const float *>(p + indices[index_of_triangle * 3 + c2] * stride))[0],
				(reinterpret_cast<const float *>(p + indices[index_of_triangle * 3 + c2] * stride))[1],
				(reinterpret_cast<const float *>(p + indices[index_of_triangle * 3 + c2] * stride))[2]);

			XMVECTOR A, B, C;
			A = XMLoadFloat3(&p0);
			B = XMLoadFloat3(&p1);
			C = XMLoadFloat3(&p2);

			// Determining Q
			// Determining the point of intersection Q in turn requires us to take two more steps :
			//  1. Determine the equation of the supporting plane for triangle ABC.
			//  2. Intersect the ray with the supporting plane.
			// We’ll handle these steps in reverse order.
			XMVECTOR Q;

			// 1. Determine the equation of the supporting plane for triangle ABC.
			XMVECTOR N = XMVector3Normalize(XMVector3Cross(XMVectorSubtract(B, A), XMVectorSubtract(C, A)));
			float d = XMVectorGetByIndex(XMVector3Dot(N, A), 0);

			// 2. Intersect the ray with the supporting plane.
			float denominator = XMVectorGetByIndex(XMVector3Dot(N, D), 0);
			if (denominator < 0) // Note that if N.D = 0 , then D is parallel to the plane and the ray does not intersect the plane.
			{
				float numerator = d - XMVectorGetByIndex(XMVector3Dot(N, P), 0);
				float t = numerator / denominator;

				if (t > 0 && t < ray_length_limit) // Forward and Length limit of Ray
				{
					Q = XMVectorAdd(P, XMVectorScale(D, t));

					// Triangle inside - outside testing

					// CHRISTER ERICSON. "REAL-TIME COLLISION DETECTION" pp.204
					// Test if point Q lies inside the counterclockwise 3D triangle ABC

					// Translate point and triangle so that point lies at origin
					static const XMVECTOR Z = XMVectorSet(0, 0, 0, 1);
					static const XMVECTOR E = XMVectorSet(FLT_EPSILON, FLT_EPSILON, FLT_EPSILON, 1);

					XMVECTOR QA = XMVectorSubtract(A, Q);
					XMVECTOR QB = XMVectorSubtract(B, Q);
					XMVECTOR QC = XMVectorSubtract(C, Q);

					XMVECTOR U = XMVector3Cross(QB, QC);
					if (XMVector3NearEqual(U, Z, E))
					{
						U = Z;
					}
					XMVECTOR V = XMVector3Cross(QC, QA);
					if (XMVector3NearEqual(V, Z, E))
					{
						V = Z;
					}
					if (XMVectorGetByIndex(XMVector3Dot(U, V), 0) < 0)
					{
						continue;
					}

					XMVECTOR W = XMVector3Cross(QA, QB);
					if (XMVector3NearEqual(W, Z, E))
					{
						W = Z;
					}
					if (XMVectorGetByIndex(XMVector3Dot(U, W), 0) < 0)
					{
						continue;
					}
					if (XMVectorGetByIndex(XMVector3Dot(V, W), 0) < 0)
					{
						continue;
					}

					// Otherwise Q must be in (or on) the triangle
					if (t < shortest_distance)
					{
						shortest_distance = t;
						index_of_intersected_triangle = static_cast<int>(index_of_triangle);
						X = Q;

					}
					number_of_intersections++;
				}
			}
		}
		if (number_of_intersections > 0)
		{
			XMStoreFloat3(intersection, X);
		}
		return index_of_intersected_triangle;
	}

	collision::mesh::mesh(const char *filename) : Descartes::substance<vertex>(filename, true, 0, ".cm")
	{
	}
	const char *collision::mesh::ray_triangles_intersection
	(
		const XMFLOAT3 &ray_position, // world space [in]
		const XMFLOAT3 &ray_direction, // world space [in]
		const XMFLOAT4X4 &world_transform, // model to world space [in]
		XMFLOAT3 *intersection // world space [out]
	)
	{
		for (const Descartes::substance<vertex>::mesh &mesh : meshes)
		{
			XMMATRIX M = XMLoadFloat4x4(&mesh.global_transform) * XMLoadFloat4x4(&system_unit_transform) * XMLoadFloat4x4(&axis_system_transform) * XMLoadFloat4x4(&world_transform);
			XMMATRIX T = XMMatrixInverse(0, M);
			XMFLOAT3 ray_position_in_model_space;
			XMFLOAT3 ray_direction_in_model_space;
			XMStoreFloat3(&ray_position_in_model_space, XMVector4Transform(XMVectorSet(ray_position.x, ray_position.y, ray_position.z, 1), T));
			XMStoreFloat3(&ray_direction_in_model_space, XMVector3Normalize(XMVector4Transform(XMVectorSet(ray_direction.x, ray_direction.y, ray_direction.z, 0), T)));

			if (!point_in_aabb(reinterpret_cast<const float *>(&ray_position_in_model_space), reinterpret_cast<const float *>(&mesh.bounding_box.min), reinterpret_cast<const float *>(&mesh.bounding_box.max)))
			{
				continue;
			}

			int index_of_intersected_triangle = collision::ray_triangles_intersection(&(mesh.vertices.at(0).position.x), sizeof(collision::vertex), &(mesh.indices.at(0)), mesh.indices.size(), ray_position_in_model_space, ray_direction_in_model_space, FLT_MAX, intersection);
			if (index_of_intersected_triangle != -1)
			{
				XMStoreFloat3(intersection, XMVector3Transform(XMVectorSet(intersection->x, intersection->y, intersection->z, 1), M));

				const char *material_name = mesh.find_material(index_of_intersected_triangle);
				return material_name ? material_name : "";
			}
		}
		return 0;
	}

	const char *collision::mesh::ray_triangles_intersection_downward
	(
		const XMFLOAT3 &ray_position, // world space [in]
		const XMFLOAT4X4 &world_transform, // model to world space [in]
		XMFLOAT3 *intersection // world space [out]
	)
	{
		for (const Descartes::substance<vertex>::mesh &mesh : meshes)
		{
			XMMATRIX M = XMLoadFloat4x4(&mesh.global_transform) * XMLoadFloat4x4(&system_unit_transform) * XMLoadFloat4x4(&axis_system_transform) * XMLoadFloat4x4(&world_transform);
			XMMATRIX T = XMMatrixInverse(0, M);
			XMFLOAT3 ray_position_in_model_space;
			XMStoreFloat3(&ray_position_in_model_space, XMVector4Transform(XMVectorSet(ray_position.x, ray_position.y, ray_position.z, 1), T));

			if (ray_position_in_model_space.x >= mesh.bounding_box.min.x && ray_position_in_model_space.x <= mesh.bounding_box.max.x &&
				ray_position_in_model_space.z >= mesh.bounding_box.min.z && ray_position_in_model_space.z <= mesh.bounding_box.max.z)
			{
				int index_of_intersected_triangle = collision::ray_triangles_intersection_downward(&(mesh.vertices.at(0).position.x), sizeof(collision::vertex), &(mesh.indices.at(0)), mesh.indices.size(), ray_position_in_model_space, FLT_MAX, intersection);
				if (index_of_intersected_triangle != -1)
				{
					XMStoreFloat3(intersection, XMVector3Transform(XMVectorSet(intersection->x, intersection->y, intersection->z, 1), M));

					const char *material_name = mesh.find_material(index_of_intersected_triangle);
					return material_name ? material_name : "";
				}
			}
		}
		return 0;
	}

	// https://www.braynzarsoft.net/viewtutorial/q16390-34-aabb-cpu-side-frustum-culling
	int view_frustum::intersect_frustum_aabb(const view_frustum& view_frustum, const DirectX::XMFLOAT3 bounding_box[2])
	{
		int cull{ false };
		for (int plane_index = 0; plane_index < 6; ++plane_index)
		{
			DirectX::XMVECTOR n{ DirectX::XMVectorSet(view_frustum.normal[plane_index].x, view_frustum.normal[plane_index].y, view_frustum.normal[plane_index].z, 0.0f) };
			//float d{ view_frustum.normal[plane_index].w };
			float d{ 0 };

			// Check each axis (x,y,z) to get the AABB vertex furthest away from the direction the plane is facing (plane normal)
			DirectX::XMFLOAT3 axis{
				view_frustum.normal[plane_index].x < 0.0f ? bounding_box[0].x : bounding_box[1].x, // Which AABB vertex is furthest down (plane normals direction) the x axis
				view_frustum.normal[plane_index].y < 0.0f ? bounding_box[0].y : bounding_box[1].y, // Which AABB vertex is furthest down (plane normals direction) the y axis
				view_frustum.normal[plane_index].z < 0.0f ? bounding_box[0].z : bounding_box[1].z  // Which AABB vertex is furthest down (plane normals direction) the z axis
			};

			// Now we get the signed distance from the AABB vertex that's furthest down the frustum planes normal,
			// and if the signed distance is negative, then the entire bounding box is behind the frustum plane, which means
			// that it should be culled
			if (DirectX::XMVectorGetX(DirectX::XMVector3Dot(n, DirectX::XMLoadFloat3(&axis))) + d < 0.0f)
			{
				cull = true;
				// Skip remaining planes to check and move on to next tree
				break;
			}
		}
		return cull;
	}

	bool view_frustum::judge_cross(const view_frustum& view_frustum, const DirectX::XMFLOAT3 mesh_world_bounding_box[2])
	{
		// 視錐台（フラスタム）との交差・内外判定
		// AABBが視錐台の内部にある   ：collisionState = 0
		// AABBが視錐台の境界と交わっている ：collisionState = 1
		// AABBが視錐台の外部にある   ：collisionState = 2

		// いったん内部にあるとしておく
		int collisionState = 0;
		int count = 0;
		for (int i = 0; i < 6; i++)
		{
			//④各平面の法線の成分を用いてAABBの８頂点の中から最近点と最遠点を求める
			DirectX::XMFLOAT3 center{};
		    center .x= (mesh_world_bounding_box[1].x - mesh_world_bounding_box[0].x) / 2;
		    center .y= (mesh_world_bounding_box[1].y - mesh_world_bounding_box[0].y) / 2;
		    center .z= (mesh_world_bounding_box[1].z - mesh_world_bounding_box[0].z) / 2;
			DirectX::XMFLOAT3 NegaPos = center; // 最近点
			DirectX::XMFLOAT3 PosiPos = center; // 最遠点

			DirectX::XMFLOAT3 bounding_size = {};
			bounding_size.x=fabsf(mesh_world_bounding_box[0].x - mesh_world_bounding_box[1].x)/2;
			bounding_size.y=fabsf(mesh_world_bounding_box[0].y - mesh_world_bounding_box[1].y)/2;
			bounding_size.z=fabsf(mesh_world_bounding_box[0].z - mesh_world_bounding_box[1].z)/2;

			if (view_frustum.normal[i].x > 0)
			{
				NegaPos.x -= fabsf(bounding_size.x);
				PosiPos.x += fabsf(bounding_size.x);
			}
			else if (view_frustum.normal[i].x < 0)
			{
				NegaPos.x += fabsf(bounding_size.x);
				PosiPos.x -= fabsf(bounding_size.x);
			}
			if (view_frustum.normal[i].y > 0)
			{
				NegaPos.y -= fabsf(bounding_size.y);
				PosiPos.y += fabsf(bounding_size.y);
			}
			else if (view_frustum.normal[i].y < 0)
			{
				NegaPos.y += fabsf(bounding_size.y);
				PosiPos.y -= fabsf(bounding_size.y);
			}
			if (view_frustum.normal[i].z > 0)
			{
				NegaPos.z -= fabsf(bounding_size.z);
				PosiPos.z += fabsf(bounding_size.z);
			}
			else if (view_frustum.normal[i].z < 0)
			{
				NegaPos.z += fabsf(bounding_size.z);
				PosiPos.z -= fabsf(bounding_size.z);
			}


			//⑤各平面との内積を計算し、交差・内外判定(表裏判定)を行う
			//  内部であれば、そのまま次の平面とのチェックに続ける
			DirectX::XMVECTOR Nega = DirectX::XMLoadFloat3(&NegaPos);
			DirectX::XMVECTOR Posi = DirectX::XMLoadFloat3(&PosiPos);

			//射影
			DirectX::XMVECTOR frus = DirectX::XMLoadFloat3(&view_frustum.normal[i]);
			DirectX::XMVECTOR NegaDot = DirectX::XMVector3Dot(frus, Nega);
			DirectX::XMVECTOR PosiDot = DirectX::XMVector3Dot(frus, Posi);

			float negaDot, posiDot;
			DirectX::XMStoreFloat(&negaDot, NegaDot);
			DirectX::XMStoreFloat(&posiDot, PosiDot);

			//  外部と分かれば処理をbreakし確定させる
			if ((view_frustum.distance[i] > negaDot && view_frustum.distance[i] > posiDot))
			{
				collisionState = 2;
				break;
			}
			if ((view_frustum.distance[i] < negaDot && view_frustum.distance[i] < posiDot))
			{
				count++;
			}
			//  交差状態であれば、ステータスを変更してから次の平面とのチェックに続ける
			if ((view_frustum.distance[i] > negaDot && view_frustum.distance[i] < posiDot) ||
				(view_frustum.distance[i] < negaDot && view_frustum.distance[i] > posiDot))
			{
				collisionState = 1;
				//break;
			}
		}
		if (count >= 6)
			collisionState = 0;

		return (collisionState == 1 || collisionState == 0) ? true : false;//true..Draw false..non
	}
}

