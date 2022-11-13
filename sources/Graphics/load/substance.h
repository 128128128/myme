#pragma once

#include <d3d11.h>
#include <wrl.h>
#include <Shlwapi.h>
#include <DirectXMath.h>
using namespace DirectX;

#include <fbxsdk.h>
using namespace fbxsdk;

#include <vector>

#include <iostream>
#include <fstream>
#undef max
#undef min
#include <cereal/archives/binary.hpp>
#include <cereal/types/memory.hpp>
#include <cereal/types/vector.hpp>
#include <cereal/types/map.hpp>

#include "texture.h"
#include "../../Functions/utility_graphics.h"

#define MAX_BONE_INFLUENCES 4
#define MAX_BONES 300

namespace Descartes
{
	struct vertex
	{
		XMFLOAT3 position = {};
		XMFLOAT3 normal = {};
		XMFLOAT4 tangent = {};
		XMFLOAT3 binormal = {};
		XMFLOAT2 texcoord = {};
		XMFLOAT4 color = { 1, 1, 1, 1 };
		float bone_weights[MAX_BONE_INFLUENCES] = { 1, 0, 0, 0 };
		std::uint32_t bone_indices[MAX_BONE_INFLUENCES] = {};
	};

	struct organization_node//éqÉmÅ[Éh
	{
		FbxNodeAttribute::EType attribute;
		std::string name;
		std::vector<organization_node> children;

		template<class T>
		void serialize(T& archive)
		{
			archive(attribute, name, children);
		}
	};

	struct animation_take
	{
		std::string name;
		size_t sampling_rate;
		size_t number_of_keyframes = 0;

		template<class T>
		void serialize(T& archive)
		{
			archive(name, sampling_rate, number_of_keyframes);
		}
	};
	void fetch_takes_animations(FbxScene* scene, std::vector<animation_take>& animation_takes, size_t sampling_rate/*0:default value*/);

	struct bone
	{
		std::string name;

		XMFLOAT4X4 transform; // from bone space to global space
		XMFLOAT4X4 transform_to_parent; // from bone space to parent bone space

		template<class T>
		void serialize(T& archive)
		{
			archive
			(
				name,
				transform._11, transform._12, transform._13, transform._14,
				transform._21, transform._22, transform._23, transform._24,
				transform._31, transform._32, transform._33, transform._34,
				transform._41, transform._42, transform._43, transform._44,
				transform_to_parent._11, transform_to_parent._12, transform_to_parent._13, transform_to_parent._14,
				transform_to_parent._21, transform_to_parent._22, transform_to_parent._23, transform_to_parent._24,
				transform_to_parent._31, transform_to_parent._32, transform_to_parent._33, transform_to_parent._34,
				transform_to_parent._41, transform_to_parent._42, transform_to_parent._43, transform_to_parent._44
			);
		}
	};
	struct skeleton
	{
		std::vector<bone> bones;
		const bone* find(const char* bone_name) const
		{
			for (const bone& bone : bones)
			{
				if (bone.name == bone_name)
				{
					return &bone;
				}
			}
			return 0;
		}

		template<class T>
		void serialize(T& archive)
		{
			archive(bones);
		}
	};
	struct actor_transform
	{
		//std::string name;

		XMFLOAT4X4 transform; // from model space to global space
		template<class T>
		void serialize(T& archive)
		{
			archive
			(
				//name,
				transform._11, transform._12, transform._13, transform._14,
				transform._21, transform._22, transform._23, transform._24,
				transform._31, transform._32, transform._33, transform._34,
				transform._41, transform._42, transform._43, transform._44
			);
		}
	};
	struct animation
	{
		std::string name;
		std::vector<skeleton> skeleton_transforms;
		std::vector<actor_transform> actor_transforms;

		template<class T>
		void serialize(T& archive)
		{
			archive(name, skeleton_transforms, actor_transforms);
		}
	};
	void fetch_animation(FbxMesh* fbx_mesh, const animation_take& animation_take, animation& animation);
	void fetch_offset_matrices(FbxMesh* fbx_mesh, std::vector<bone>& offset_transforms);
	void blend_skeletons
	(
		const organization_node* parent,
		const DirectX::XMFLOAT4X4& parent_transform,
		const std::vector<bone>* skeletons[2],
		float ratio/*0 < ratio < 1*/,
		std::vector<bone>* blended_skeleton
	);

	struct material
	{
		std::string name;
		std::string shading_model;
		bool multiLayer;
		struct property
		{
			std::string name;

			XMFLOAT4 color = { 0.0f, 0.0f, 0.0f, 0.0f }; // w channel is used as shininess by only specular.
			float factor = 1.0f;

			std::string texture_filename;
			float u_scale = 1.0;
			float v_scale = 1.0;

			bool force_srgb = false; // This parameter is used on load.

			Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> shader_resource_view;

			template<class T>
			void serialize(T& archive)
			{
				archive
				(
					name,
					color.x, color.y, color.z, color.w,
					factor,
					texture_filename,
					u_scale, v_scale, force_srgb
				);
			}
		};
		std::vector<property> properties;

		template<class T>
		void serialize(T& archive)
		{
			archive(name, shading_model, multiLayer, properties);
		}
	};
	void fetch_materials(FbxNode* fbx_node, std::unordered_map<std::string, material>& materials);

	struct bounding_box
	{
		XMFLOAT3 min = { +FLT_MAX, +FLT_MAX, +FLT_MAX };
		XMFLOAT3 max = { -FLT_MAX, -FLT_MAX, -FLT_MAX };

		template<class T>
		void serialize(T& archive)
		{
			archive(min.x, min.y, min.z, max.x, max.y, max.z);
		}
	};

	struct subset
	{
		std::string name; // same as material name
		size_t index_start = 0;	// start number of index buffer
		size_t index_count = 0;	// number of vertices (indices)

		template<class T>
		void serialize(T& archive)
		{
			archive(name, index_start, index_count);
		}
	};
	void fetch_vertices_and_indices(FbxMesh* fbx_mesh, std::vector<vertex>& vertices, std::vector<std::uint32_t>& indices, std::vector<subset>& subsets);

	template <class T>
	struct substance
	{
		struct mesh
		{
			std::string name;

			std::vector<T> vertices;
			std::vector<std::uint32_t> indices;

			Microsoft::WRL::ComPtr<ID3D11Buffer> vertex_buffer;
			Microsoft::WRL::ComPtr<ID3D11Buffer> index_buffer;

			std::vector<subset> subsets;
			const char* find_material(size_t index_of_triangle) const
			{
				size_t index_of_indices = 3 * index_of_triangle; // to index of indices
				for (const subset& subset : subsets)
				{
					if (subset.index_start <= index_of_indices && subset.index_start + subset.index_count > index_of_indices)
					{
						return subset.name.empty() ? "" : subset.name.c_str();
					}
				}
				return 0;
			}

			std::vector<animation> animations;
			std::vector<bone> offset_transforms; // initial transforms from model space to bone spaces
			const animation* find_animation(const char* animation_name) const
			{
				for (const animation& animation : animations)
				{
					if (animation.name == animation_name)
					{
						return &animation;
					}
				}
				return 0;
			}

			XMFLOAT4X4 global_transform = { 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1 };
			bounding_box bounding_box; // model space

			template<class T>
			void serialize(T& archive)
			{
				archive
				(
					name, vertices, indices, subsets,
					global_transform._11, global_transform._12, global_transform._13, global_transform._14,
					global_transform._21, global_transform._22, global_transform._23, global_transform._24,
					global_transform._31, global_transform._32, global_transform._33, global_transform._34,
					global_transform._41, global_transform._42, global_transform._43, global_transform._44,
					animations, offset_transforms, bounding_box
				);
			}
		};
		std::vector<mesh> meshes;
		const mesh* find_mesh(const char* mesh_name) const
		{
			for (const mesh& mesh : meshes)
			{
				if (mesh.name == mesh_name)
				{
					return &mesh;
				}
			}
			return 0;
		}

		std::unordered_map<std::string, material> materials;
		const material* find_material(const char* material_name) const
		{
			std::unordered_map<std::string, material>::const_iterator iterator = materials.find(material_name);
			return iterator == materials.end() ? 0 : &(iterator->second);
		}

		std::vector<animation_take> animation_takes;
		organization_node root_node;

		substance() = default;
		virtual ~substance() = default;
		substance(substance&) = delete;
		substance& operator=(substance&) = delete;
		substance(const char* fbx_filename, bool triangulate, int animation_sampling_rate = 0 /*0:default value, -1:ignore animations*/, const char* extension = ".bin")
		{
			if (PathFileExistsA((std::string(fbx_filename) + extension).c_str()))
			{
				// Deserialize from 'fbx_filename + extension' file.
				std::ifstream ifs;
				ifs.open((std::string(fbx_filename) + extension).c_str(), std::ios::binary);
				cereal::BinaryInputArchive i_archive(ifs);
				i_archive(*this);
			}
			else
			{
				// Create from 'fbx_filename' file.
				create_from_fbx(fbx_filename, triangulate, animation_sampling_rate);

				// Serialize to 'fbx_filename + extension' file.
				std::ofstream ofs;
				ofs.open((std::string(fbx_filename) + extension).c_str(), std::ios::binary);
				cereal::BinaryOutputArchive o_archive(ofs);
				o_archive(*this);
			}
		}
		substance(const char* fbx_filename, bool triangulate, const char* animation_filenames[], size_t number_of_animations, int animation_sampling_rate = 0 /*0:default value, -1:ignore animations*/, const char* extension = ".bin")
		{
			if (PathFileExistsA((std::string(fbx_filename) + extension).c_str()))
			{
				// Deserialize from 'fbx_filename + extension' file.
				std::ifstream ifs;
				ifs.open((std::string(fbx_filename) + extension).c_str(), std::ios::binary);
				cereal::BinaryInputArchive i_archive(ifs);
				i_archive(*this);
			}
			else
			{
				// Create from 'fbx_filename' file.
				create_from_fbx(fbx_filename, triangulate, animation_sampling_rate);
				for (size_t i = 0; animation_filenames && i < number_of_animations; i++)
				{
					add_animations(animation_filenames[i], animation_sampling_rate);
				}

				// Serialize to 'fbx_filename + extension' file.
				std::ofstream ofs;
				ofs.open((std::string(fbx_filename) + extension).c_str(), std::ios::binary);
				cereal::BinaryOutputArchive o_archive(ofs);
				o_archive(*this);
			}
		}
		//getter
		const std::vector<mesh>& get_meshes() const { return meshes; }
	private:
		void create_from_fbx(const char* fbx_filename, bool triangulate, int animation_sampling_rate)
		{
			// Create the FBX SDK manager
			FbxManager* manager = FbxManager::Create();

			// Create an IOSettings object. IOSROOT is defined in Fbxiosettingspath.h.
			manager->SetIOSettings(FbxIOSettings::Create(manager, IOSROOT));

			// Create an importer.
			FbxImporter* importer = FbxImporter::Create(manager, "");

			// Initialize the importer.
			bool import_status = false;
			import_status = importer->Initialize(fbx_filename, -1, manager->GetIOSettings());
			_ASSERT_EXPR_A(import_status, importer->GetStatus().GetErrorString());

			// Create a new scene so it can be populated by the imported file.
			FbxScene* scene = FbxScene::Create(manager, "");

			// Import the contents of the file into the scene.
			import_status = importer->Import(scene);
			_ASSERT_EXPR_A(import_status, importer->GetStatus().GetErrorString());

#if 0
			FbxAxisSystem our_axis_system = FbxAxisSystem::DirectX;
			FbxAxisSystem scene_axis_system = scene->GetGlobalSettings().GetAxisSystem();
			if (scene_axis_system != our_axis_system)
			{
				FbxAxisSystem::DirectX.ConvertScene(scene);
			}
#endif
			// Convert axis system (to Y-UP Z-FRONT LHS)
			FbxAxisSystem axis_system_reference = scene->GetGlobalSettings().GetAxisSystem();
			FbxAxisSystem::ECoordSystem coord_system = axis_system_reference.GetCoorSystem();
			//int front_vector_sign = 1;
			//FbxAxisSystem::EFrontVector front_vector = axis_sytem_reference.GetFrontVector(front_vector_sign);
			int up_vector_sign = 1;
			FbxAxisSystem::EUpVector up_vector = axis_system_reference.GetUpVector(up_vector_sign);
			_ASSERT_EXPR(up_vector == FbxAxisSystem::eYAxis || up_vector == FbxAxisSystem::eZAxis, L"Not support X-Up axis system");
			if (up_vector == FbxAxisSystem::eYAxis)
			{
				axis_system_transform = XMFLOAT4X4(1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1);
				if (coord_system == FbxAxisSystem::eRightHanded)
				{
					axis_system_transform._11 = -1;
				}
				else if (coord_system == FbxAxisSystem::eLeftHanded)
				{
				}
			}
			else if (up_vector == FbxAxisSystem::eZAxis)
			{
				axis_system_transform = XMFLOAT4X4(1, 0, 0, 0, 0, 0, 1, 0, 0, 1, 0, 0, 0, 0, 0, 1);
				if (coord_system == FbxAxisSystem::eRightHanded)
				{
					axis_system_transform._11 = -1;
					axis_system_transform._23 = -1;
				}
				else if (coord_system == FbxAxisSystem::eLeftHanded)
				{
					axis_system_transform._11 = -1;
				}
			}

			// Convert unit system (to meter)
			if (scene->GetGlobalSettings().GetSystemUnit() == FbxSystemUnit::cm)
			{
				system_unit_transform._11 = system_unit_transform._22 = system_unit_transform._33 = 0.01f;
			}

			// Convert mesh, NURBS and patch into triangle mesh
			if (triangulate)
			{
				fbxsdk::FbxGeometryConverter geometry_converter(manager);
				geometry_converter.Triangulate(scene, /*replace*/true);
			}

			if (animation_sampling_rate > -1)
			{
				fetch_takes_animations(scene, animation_takes, animation_sampling_rate);
			}

			// Fetch node attributes and materials under this node recursively. Currently only mesh.
			std::vector<FbxNode*> fetched_meshes;
			std::function<void(FbxNode*)> traverse = [&](FbxNode* node)
			{
				fetch_materials(node, materials);
				FbxNodeAttribute* fbx_node_attribute = node->GetNodeAttribute();
				if (fbx_node_attribute)
				{
					switch (fbx_node_attribute->GetAttributeType())
					{
					case FbxNodeAttribute::eMesh:
						fetched_meshes.push_back(node);
						break;
					}
				}
				for (int i = 0; i < node->GetChildCount(); i++)
				{
					traverse(node->GetChild(i));
				}
			};
			traverse(scene->GetRootNode());

			meshes.resize(fetched_meshes.size());
			for (size_t i = 0; i < fetched_meshes.size(); i++)
			{
				FbxMesh* fbx_mesh = fetched_meshes.at(i)->GetMesh();

				mesh& mesh = meshes.at(i);
				mesh.name = fbx_mesh->GetNode()->GetName();

				FbxAMatrix global_transform = fbx_mesh->GetNode()->EvaluateGlobalTransform(0);
				fbxamatrix_to_xmfloat4x4(global_transform, mesh.global_transform);

				std::vector<vertex> vertices;
				fetch_vertices_and_indices(fbx_mesh, vertices, mesh.indices, mesh.subsets);

				for (const vertex& v : vertices)
				{
					mesh.vertices.push_back(v);
					mesh.bounding_box.max.x = std::max<float>(v.position.x, mesh.bounding_box.max.x);
					mesh.bounding_box.max.y = std::max<float>(v.position.y, mesh.bounding_box.max.y);
					mesh.bounding_box.max.z = std::max<float>(v.position.z, mesh.bounding_box.max.z);
					mesh.bounding_box.min.x = std::min<float>(v.position.x, mesh.bounding_box.min.x);
					mesh.bounding_box.min.y = std::min<float>(v.position.y, mesh.bounding_box.min.y);
					mesh.bounding_box.min.z = std::min<float>(v.position.z, mesh.bounding_box.min.z);
				}

				fetch_offset_matrices(fbx_mesh, mesh.offset_transforms);
				for (const animation_take& animation_take : animation_takes)
				{
					animation animation;
					fetch_animation(fbx_mesh, animation_take, animation);
					mesh.animations.push_back(animation);
				}
			}

			std::function<void(FbxNode*, organization_node&)> traverse_hierarchy = [&traverse_hierarchy](FbxNode* fbx_node, organization_node& node)
			{
				if (fbx_node)
				{
					FbxNodeAttribute* fbx_node_attribute = fbx_node->GetNodeAttribute();

					node.attribute = fbx_node_attribute ? fbx_node_attribute->GetAttributeType() : FbxNodeAttribute::eUnknown;
					node.name = fbx_node->GetName();

					int number_of_children = fbx_node->GetChildCount();
					node.children.resize(number_of_children);
					for (int i = 0; i < number_of_children; i++)
					{
						traverse_hierarchy(fbx_node->GetChild(i), node.children.at(i));
					}
				}
			};
			traverse_hierarchy(scene->GetRootNode(), root_node);

			manager->Destroy();
		}
	public:
		void create_render_objects(ID3D11Device* device, bool clear_vertex_conteiner)
		{
			for (mesh& mesh : meshes)
			{
				HRESULT hr;
				{
					D3D11_BUFFER_DESC buffer_desc = {};
					D3D11_SUBRESOURCE_DATA subresource_data = {};

					buffer_desc.ByteWidth = static_cast<UINT>(sizeof(T) * mesh.vertices.size());
					buffer_desc.Usage = D3D11_USAGE_DEFAULT;
					buffer_desc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
					buffer_desc.CPUAccessFlags = 0;
					buffer_desc.MiscFlags = 0;
					buffer_desc.StructureByteStride = 0;
					subresource_data.pSysMem = mesh.vertices.data();
					subresource_data.SysMemPitch = 0;
					subresource_data.SysMemSlicePitch = 0;

					hr = device->CreateBuffer(&buffer_desc, &subresource_data, mesh.vertex_buffer.ReleaseAndGetAddressOf());
					_ASSERT_EXPR(SUCCEEDED(hr), hr_trace(hr));
				}
				{
					D3D11_BUFFER_DESC buffer_desc = {};
					D3D11_SUBRESOURCE_DATA subresource_data = {};

					buffer_desc.ByteWidth = static_cast<UINT>(sizeof(std::uint32_t) * mesh.indices.size());
					buffer_desc.Usage = D3D11_USAGE_DEFAULT;
					buffer_desc.BindFlags = D3D11_BIND_INDEX_BUFFER;
					buffer_desc.CPUAccessFlags = 0;
					buffer_desc.MiscFlags = 0;
					buffer_desc.StructureByteStride = 0;
					subresource_data.pSysMem = mesh.indices.data();
					subresource_data.SysMemPitch = 0;
					subresource_data.SysMemSlicePitch = 0;
					hr = device->CreateBuffer(&buffer_desc, &subresource_data, mesh.index_buffer.ReleaseAndGetAddressOf());
					_ASSERT_EXPR(SUCCEEDED(hr), hr_trace(hr));
				}

				if (clear_vertex_conteiner)
				{
					mesh.vertices.clear();
					mesh.indices.clear();
				}
			}
		}
		// Load a texture file for each material.(if 'texture_filename' is empty then create a dummy shader resource view for each)
		void create_shader_resource_views(ID3D11Device* device, const char* media_directory)
		{
			for (std::unordered_map<std::string, material>::iterator iterator = materials.begin(); iterator != materials.end(); ++iterator)
			{
				material& material = iterator->second;
				for (material::property& property : material.properties)
				{
					char concatenated_resource_path[256];
					strcpy_s(concatenated_resource_path, media_directory);

					wchar_t filename_w[256];
					MultiByteToWideChar(CP_ACP, 0, property.texture_filename.c_str(), static_cast<int>(strlen(property.texture_filename.c_str()) + 1), filename_w, 256);
					PathRenameExtensionW(filename_w, L".dds");
					char c[256] ;
					size_t s;
					wcstombs_s(&s,  c, sizeof(c), filename_w, sizeof(filename_w));
					strcat_s(concatenated_resource_path, c);


					if (PathFileExistsA(concatenated_resource_path) && !PathIsDirectoryA(concatenated_resource_path))
					{
						load_texture_from_file(device, concatenated_resource_path, property.shader_resource_view.ReleaseAndGetAddressOf(), property.force_srgb,true);
					}
					else
					{
						std::uint32_t R = static_cast<std::uint8_t>(property.color.x * 255);
						std::uint32_t G = static_cast<std::uint8_t>(property.color.y * 255);
						std::uint32_t B = static_cast<std::uint8_t>(property.color.z * 255);
						std::uint32_t A = static_cast<std::uint8_t>(property.color.w * 255);
						std::uint32_t RGBA = R | (G << 8) | (B << 16) | (A << 24);
						//make_dummy_texture(device, property.shader_resource_view.ReleaseAndGetAddressOf(),16u, property.force_srgb);
						make_dummy_texture_for_substance(device, property.shader_resource_view.ReleaseAndGetAddressOf(), RGBA, 16u, property.force_srgb, true/*enable_caching*/);
					}
				}
			}
		}

		void add_animations(const char* fbx_filename, size_t sampling_rate /*0:use default value*/)
		{
			// Create the FBX SDK manager
			FbxManager* manager = FbxManager::Create();

			// Create an IOSettings object. IOSROOT is defined in <Fbxiosettingspath.h.>
			manager->SetIOSettings(FbxIOSettings::Create(manager, IOSROOT));

			// Create an importer.
			FbxImporter* importer = FbxImporter::Create(manager, "");

			// Initialize the importer.
			bool import_status = false;
			import_status = importer->Initialize(fbx_filename, -1, manager->GetIOSettings());
			_ASSERT_EXPR_A(import_status, importer->GetStatus().GetErrorString());

			// Create a new scene so it can be populated by the imported file.
			FbxScene* scene = FbxScene::Create(manager, "");

			// Import the contents of the file into the scene.
			import_status = importer->Import(scene);
			_ASSERT_EXPR_A(import_status, importer->GetStatus().GetErrorString());

			std::vector<animation_take> added_animation_takes;
			fetch_takes_animations(scene, added_animation_takes, sampling_rate);

			std::function<void(FbxNode*, FbxNode*, std::vector<bone>&, skeleton&, FbxTime)> traverse =
				[&traverse](FbxNode* node, FbxNode* parent_node, std::vector<bone>& offset_transforms, skeleton& skeleton, FbxTime time)
			{
				FbxNodeAttribute* fbx_node_attribute = node->GetNodeAttribute();
				if (fbx_node_attribute)
				{
					switch (fbx_node_attribute->GetAttributeType())
					{
					case FbxNodeAttribute::eMesh:
						break;
					case FbxNodeAttribute::eSkeleton:
						//'cluster_global_current_position' trnasforms from bone space to global space
						FbxAMatrix cluster_global_current_position;
						cluster_global_current_position = node->EvaluateGlobalTransform(time);

						XMFLOAT4X4 transform;
						fbxamatrix_to_xmfloat4x4(cluster_global_current_position, transform);

						std::string name = node->GetName();
						size_t bone_index;
						const size_t number_of_bones = offset_transforms.size();
						for (bone_index = 0; bone_index < number_of_bones; bone_index++)
						{
							if (name == offset_transforms.at(bone_index).name)
							{
								skeleton.bones.at(bone_index).name = name;
								skeleton.bones.at(bone_index).transform = transform;

								// make 'transform_to_parent' that trnasforms from bone space to parent bone space
								skeleton.bones.at(bone_index).transform_to_parent = XMFLOAT4X4(1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1);
								if (parent_node)
								{
									FbxAMatrix parent_cluster_global_current_position;
									parent_cluster_global_current_position = parent_node->EvaluateGlobalTransform(time);
									fbxamatrix_to_xmfloat4x4(parent_cluster_global_current_position.Inverse() * cluster_global_current_position, skeleton.bones.at(bone_index).transform_to_parent);
								}
								break;
							}
						}
						_ASSERT_EXPR_A(bone_index < number_of_bones, "Not found the bone");
						break;
					}
				}
				for (int i = 0; i < node->GetChildCount(); i++)
				{
					traverse(node->GetChild(i), node, offset_transforms, skeleton, time);
				}
			};

			for (animation_take& animation_take : added_animation_takes)
			{
				FbxAnimStack* animation_stack = scene->FindMember<FbxAnimStack>(animation_take.name.c_str());
				_ASSERT_EXPR_A(animation_stack, "This is a problem. The anim stack should be found in the scene!");
				scene->SetCurrentAnimationStack(animation_stack);

				for (mesh& mesh : meshes)
				{
					size_t number_of_bones = mesh.offset_transforms.size();

					float sampling_time = 1.0f / animation_take.sampling_rate;
					animation animation;
					animation.name = animation_take.name;

					FbxTime start_time;
					FbxTime stop_time;
					FbxTakeInfo* take_info = scene->GetTakeInfo(animation_take.name.c_str());
					if (take_info)
					{
						start_time = take_info->mLocalTimeSpan.GetStart();
						stop_time = take_info->mLocalTimeSpan.GetStop();
					}
					else
					{
						FbxTimeSpan time_line_time_span;
						scene->GetGlobalSettings().GetTimelineDefaultTimeSpan(time_line_time_span);

						start_time = time_line_time_span.GetStart();
						stop_time = time_line_time_span.GetStop();
					}

					FbxTime::EMode time_mode = scene->GetGlobalSettings().GetTimeMode();
					FbxTime smapling_step;
					smapling_step.SetTime(0, 0, 1, 0, 0, time_mode);
					smapling_step = static_cast<FbxLongLong>(smapling_step.Get() * sampling_time);
					for (FbxTime current_time = start_time; current_time < stop_time; current_time += smapling_step)
					{
						skeleton skeleton;
						skeleton.bones.resize(number_of_bones);
						traverse(scene->GetRootNode(), 0, mesh.offset_transforms, skeleton, current_time);
						animation.skeleton_transforms.push_back(skeleton);
					}
					_ASSERT_EXPR_A(animation_take.number_of_keyframes == animation.skeleton_transforms.size(), "There's no consistency");
					mesh.animations.push_back(animation);
				}
			}
			for (animation_take& added_animation_take : added_animation_takes)
			{
				animation_takes.push_back(added_animation_take);
			}
			manager->Destroy();
		}

		XMFLOAT4X4 axis_system_transform = { 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1 };
		XMFLOAT4X4 system_unit_transform = { 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1 };

		template<class T>
		void serialize(T& archive)
		{
			archive(meshes, animation_takes, materials, root_node,
				axis_system_transform._11, axis_system_transform._12, axis_system_transform._13, axis_system_transform._14,
				axis_system_transform._21, axis_system_transform._22, axis_system_transform._23, axis_system_transform._24,
				axis_system_transform._31, axis_system_transform._32, axis_system_transform._33, axis_system_transform._34,
				axis_system_transform._41, axis_system_transform._42, axis_system_transform._43, axis_system_transform._44,
				system_unit_transform._11, system_unit_transform._12, system_unit_transform._13, system_unit_transform._14,
				system_unit_transform._21, system_unit_transform._22, system_unit_transform._23, system_unit_transform._24,
				system_unit_transform._31, system_unit_transform._32, system_unit_transform._33, system_unit_transform._34,
				system_unit_transform._41, system_unit_transform._42, system_unit_transform._43, system_unit_transform._44
			);
		}
	};
}

inline void fbxamatrix_to_xmfloat4x4(const FbxAMatrix& fbxamatrix, XMFLOAT4X4& xmfloat4x4)
{
	for (size_t row = 0; row < 4; row++)
	{
		for (size_t column = 0; column < 4; column++)
		{
			xmfloat4x4.m[row][column] = static_cast<float>(fbxamatrix[static_cast<int>(row)][static_cast<int>(column)]);
		}
	}
}
