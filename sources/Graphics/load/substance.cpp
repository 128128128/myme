#include "substance.h"

namespace Descartes
{
	void calculate_normal_vector(std::vector<vertex>& vertices, const std::vector<size_t>& indices);
	void calculate_tangent_vector(std::vector<vertex>& vertices, const std::vector<size_t>& indices);

	void fetch_takes_animations(FbxScene* scene, std::vector<animation_take>& animation_takes, size_t sampling_rate/*0:default value*/)
	{
		FbxArray<FbxString*> array_of_animation_stack_names;
		scene->FillAnimStackNameArray(array_of_animation_stack_names);
		const size_t animation_stack_count = array_of_animation_stack_names.GetCount();
		for (int animation_stack_index = 0; animation_stack_index < animation_stack_count; ++animation_stack_index)
		{
			animation_take animation_clip;
			animation_clip.name = array_of_animation_stack_names[animation_stack_index]->Buffer();

			FbxAnimStack* animation_stack = scene->FindMember<FbxAnimStack>(animation_clip.name.c_str());
			_ASSERT_EXPR_A(animation_stack, "This is a problem. The anim stack should be found in the scene!");
			scene->SetCurrentAnimationStack(animation_stack);

			FbxTime::EMode time_mode = scene->GetGlobalSettings().GetTimeMode();
			FbxTime frame_time;
			frame_time.SetTime(0, 0, 0, 1, 0, time_mode);
			sampling_rate = sampling_rate > 0 ? sampling_rate : static_cast<size_t>(frame_time.GetFrameRate(time_mode));
			animation_clip.sampling_rate = sampling_rate;

			FbxTime smapling_step;
			smapling_step.SetTime(0, 0, 1, 0, 0, time_mode);
			smapling_step = static_cast<FbxLongLong>(smapling_step.Get() * (1.0f / sampling_rate));

			FbxTakeInfo* take_info = scene->GetTakeInfo(animation_clip.name.c_str());
			FbxTime start_time;
			FbxTime stop_time;
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
			animation_clip.number_of_keyframes = static_cast<size_t>(((stop_time - start_time) / smapling_step).Get());
			animation_takes.push_back(animation_clip);
		}
		for (int index_of_animation_stack = 0; index_of_animation_stack < animation_stack_count; ++index_of_animation_stack)
		{
			delete array_of_animation_stack_names[index_of_animation_stack];
		}
	}

	struct bone_influence
	{
		std::uint32_t index;	// index of bone
		float weight; // weight of bone
	};
	struct bone_influences_per_control_point : public std::vector<bone_influence>
	{
		float total_weight = 0;
	};
	void fetch_bone_influences(const FbxMesh* fbx_mesh, std::vector<bone_influences_per_control_point>& influences)
	{
		const size_t number_of_control_points = fbx_mesh->GetControlPointsCount();
		influences.resize(number_of_control_points);

		const int number_of_deformers = fbx_mesh->GetDeformerCount(FbxDeformer::eSkin);
		for (int index_of_deformer = 0; index_of_deformer < number_of_deformers; ++index_of_deformer)
		{
			FbxSkin* skin = static_cast<FbxSkin*>(fbx_mesh->GetDeformer(index_of_deformer, FbxDeformer::eSkin));

#if 0
			FbxSkin::EType skinning_type = skin->GetSkinningType();
			_ASSERT_EXPR(skinning_type == FbxSkin::eLinear || skinning_type == FbxSkin::eRigid, L"Only support FbxSkin::eLinear or FbxSkin::eRigid");

			FbxCluster::ELinkMode link_mode = skin->GetCluster(0)->GetLinkMode();
			_ASSERT_EXPR(link_mode != FbxCluster::eAdditive, L"Only support FbxCluster::eNormalize or FbxCluster::eTotalOne");
#endif
			const int number_of_clusters = skin->GetClusterCount();
			for (int index_of_cluster = 0; index_of_cluster < number_of_clusters; ++index_of_cluster)
			{
				FbxCluster* cluster = skin->GetCluster(index_of_cluster);

				const size_t number_of_control_point_indices = cluster->GetControlPointIndicesCount();
				const int* array_of_control_point_indices = cluster->GetControlPointIndices();
				const double* array_of_control_point_weights = cluster->GetControlPointWeights();

				for (size_t index_of_control_point = 0; index_of_control_point < number_of_control_point_indices; ++index_of_control_point)
				{
					bone_influences_per_control_point& influences_per_control_point = influences.at(array_of_control_point_indices[index_of_control_point]);
					bone_influence influence;
					influence.index = index_of_cluster;
					influence.weight = static_cast<float>(array_of_control_point_weights[index_of_control_point]);
					influences_per_control_point.push_back(influence);
					influences_per_control_point.total_weight += influence.weight;
				}
			}
		}
	}

	void set_bone_influences_to_vertex(vertex& vertex, bone_influences_per_control_point& influences_per_control_point)
	{
		for (size_t index_of_influence = 0; index_of_influence < influences_per_control_point.size(); ++index_of_influence)
		{
			if (index_of_influence < MAX_BONE_INFLUENCES)
			{
				vertex.bone_weights[index_of_influence] = influences_per_control_point.at(index_of_influence).weight / influences_per_control_point.total_weight;
				vertex.bone_indices[index_of_influence] = influences_per_control_point.at(index_of_influence).index;
			}
			else
			{
#if 0
				for (size_t i = 0; i < MAX_BONE_INFLUENCES; ++i)
				{
					if (vertex.bone_weights[i] < influences_per_control_point.at(index_of_influence).weight)
					{
						vertex.bone_weights[i] += influences_per_control_point.at(index_of_influence).weight / influences_per_control_point.total_weight;
						vertex.bone_indices[i] = influences_per_control_point.at(index_of_influence).index;
						break;
					}
				}
#else
				size_t index_of_minimum_value = 0;
				float minimum_value = FLT_MAX;
				for (size_t i = 0; i < MAX_BONE_INFLUENCES; ++i)
				{
					if (minimum_value > vertex.bone_weights[i])
					{
						minimum_value = vertex.bone_weights[i];
						index_of_minimum_value = i;
					}
				}
				vertex.bone_weights[index_of_minimum_value] += influences_per_control_point.at(index_of_influence).weight / influences_per_control_point.total_weight;
				vertex.bone_indices[index_of_minimum_value] = influences_per_control_point.at(index_of_influence).index;
#endif
			}
		}
	}

	void fetch_offset_matrices(FbxMesh* fbx_mesh, std::vector<bone>& offset_transforms)
	{
		const int number_of_deformers = fbx_mesh->GetDeformerCount(FbxDeformer::eSkin);
		for (int index_of_deformer = 0; index_of_deformer < number_of_deformers; ++index_of_deformer)
		{
			FbxSkin* skin = static_cast<FbxSkin*>(fbx_mesh->GetDeformer(index_of_deformer, FbxDeformer::eSkin));

#if 1
			FbxSkin::EType skinning_type = skin->GetSkinningType();
			_ASSERT_EXPR(skinning_type == FbxSkin::eLinear || skinning_type == FbxSkin::eRigid, L"Only support FbxSkin::eLinear or FbxSkin::eRigid");

			FbxCluster::ELinkMode link_mode = skin->GetCluster(0)->GetLinkMode();
			_ASSERT_EXPR(link_mode != FbxCluster::eAdditive, L"Only support FbxCluster::eNormalize or FbxCluster::eTotalOne");
#endif
			const int number_of_clusters = skin->GetClusterCount();
			offset_transforms.resize(number_of_clusters);
			for (size_t index_of_cluster = 0; index_of_cluster < number_of_clusters; ++index_of_cluster)
			{
				bone& bone = offset_transforms.at(index_of_cluster);

				FbxCluster* cluster = skin->GetCluster(static_cast<int>(index_of_cluster));
				bone.name = cluster->GetLink()->GetName();

				// 'reference_global_init_position' trnasforms from model space to global space
				FbxAMatrix reference_global_init_position;
				cluster->GetTransformMatrix(reference_global_init_position);
#if 0
				const FbxVector4 T = fbx_mesh->GetNode()->GetGeometricTranslation(FbxNode::eSourcePivot);
				const FbxVector4 R = fbx_mesh->GetNode()->GetGeometricRotation(FbxNode::eSourcePivot);
				const FbxVector4 S = fbx_mesh->GetNode()->GetGeometricScaling(FbxNode::eSourcePivot);
				FbxAMatrix reference_geometry(T, R, S);
				reference_global_init_position *= reference_geometry;
#endif
				// 'cluster_global_init_position' transforms from bone space to global space
				FbxAMatrix cluster_global_init_position;
				cluster->GetTransformLinkMatrix(cluster_global_init_position);

				// Matrices are defined using the Column Major scheme. When a FbxAMatrix represents a transformation (translation, rotation and scale), the last row of the matrix represents the translation part of the transformation.
				// make 'bone.transform' that trnasforms from model space to bone space (offset matrix)
				fbxamatrix_to_xmfloat4x4(cluster_global_init_position.Inverse() * reference_global_init_position, bone.transform);

				// 'bone.transform_to_parent' is noe used in offset transforms
				bone.transform_to_parent = XMFLOAT4X4(1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1);
			}
		}
	}
	void fetch_keyframe_matrices(FbxMesh* fbx_mesh, XMFLOAT4X4& keyframe_transform, FbxTime time)
	{
		// 'reference_global_current_position' trnasforms from model space to global space
		FbxAMatrix reference_global_current_position;
		reference_global_current_position = fbx_mesh->GetNode()->EvaluateGlobalTransform(time);
		fbxamatrix_to_xmfloat4x4(reference_global_current_position, keyframe_transform);
	}
	void fetch_bone_matrices(FbxMesh* fbx_mesh, skeleton& skeleton, FbxTime time)
	{
		const int number_of_deformers = fbx_mesh->GetDeformerCount(FbxDeformer::eSkin);
		for (int index_of_deformer = 0; index_of_deformer < number_of_deformers; ++index_of_deformer)
		{
			FbxSkin* skin = static_cast<FbxSkin*>(fbx_mesh->GetDeformer(index_of_deformer, FbxDeformer::eSkin));

			FbxSkin::EType skinning_type = skin->GetSkinningType();
			_ASSERT_EXPR(skinning_type == FbxSkin::eLinear || skinning_type == FbxSkin::eRigid, L"only support FbxSkin::eLinear or FbxSkin::eRigid");

			FbxCluster::ELinkMode link_mode = skin->GetCluster(0)->GetLinkMode();
			_ASSERT_EXPR(link_mode != FbxCluster::eAdditive, L"only support FbxCluster::eNormalize or FbxCluster::eTotalOne");

			const int number_of_clusters = skin->GetClusterCount();
			skeleton.bones.resize(number_of_clusters);
			for (int index_of_cluster = 0; index_of_cluster < number_of_clusters; ++index_of_cluster)
			{
				bone& bone = skeleton.bones.at(index_of_cluster);

				FbxCluster* cluster = skin->GetCluster(index_of_cluster);
				bone.name = cluster->GetLink()->GetName();

				// 'cluster_global_current_position' trnasforms from bone space to global space
				FbxAMatrix cluster_global_current_position;
				cluster_global_current_position = cluster->GetLink()->EvaluateGlobalTransform(time);
				fbxamatrix_to_xmfloat4x4(cluster_global_current_position, bone.transform);

				// make 'bone.transform_to_parent' that trnasforms from bone space to parent bone space
				bone.transform_to_parent = XMFLOAT4X4(1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1);
				FbxNode* parent_node = cluster->GetLink()->GetParent();
				if (parent_node)
				{
					FbxAMatrix parent_cluster_global_current_position;
					parent_cluster_global_current_position = parent_node->EvaluateGlobalTransform(time);
					fbxamatrix_to_xmfloat4x4(parent_cluster_global_current_position.Inverse() * cluster_global_current_position, bone.transform_to_parent);
				}
			}
		}
	}
	void fetch_animation(FbxMesh* fbx_mesh, const animation_take& animation_take, animation& animation)
	{
		_ASSERT_EXPR_A(animation_take.sampling_rate > 0, "'animation_take.sampling_rate' is an invalid value.");

		FbxScene* scene = fbx_mesh->GetScene();
		FbxAnimStack* animation_stack = scene->FindMember<FbxAnimStack>(animation_take.name.c_str());
		scene->SetCurrentAnimationStack(animation_stack);

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
		smapling_step = static_cast<FbxLongLong>(smapling_step.Get() * (1.0f / animation_take.sampling_rate));

		for (FbxTime time = start_time; time < stop_time; time += smapling_step)
		{
			skeleton skeleton;
			fetch_bone_matrices(fbx_mesh, skeleton, time);
			animation.skeleton_transforms.push_back(skeleton);

			actor_transform actor_transform;
			fetch_keyframe_matrices(fbx_mesh, actor_transform.transform, time);
			animation.actor_transforms.push_back(actor_transform);
		}
		_ASSERT_EXPR_A(animation_take.number_of_keyframes == animation.skeleton_transforms.size(), "There's no consistency");
		_ASSERT_EXPR_A(animation_take.number_of_keyframes == animation.actor_transforms.size(), "There's no consistency");
		animation.name = animation_take.name;
	}

	void blend_skeletons
	(
		const organization_node* parent,
		const DirectX::XMFLOAT4X4& parent_transform,
		const std::vector<bone>* skeletons[2],
		float ratio/*0 < ratio < 1*/,
		std::vector<bone>* blended_skeleton
	)
	{
		for (const organization_node& node : parent->children)
		{
			if (node.attribute == FbxNodeAttribute::eSkeleton)
			{
				size_t bone_index;
				size_t number_of_bones = skeletons[0]->size();
				for (bone_index = 0; bone_index < number_of_bones; bone_index++)
				{
					if (node.name == skeletons[0]->at(bone_index).name)
					{
						break;
					}
				}
				_ASSERT_EXPR_A(bone_index < number_of_bones, "Not found the bone");

				const Descartes::bone* bones[2];
				bones[0] = &skeletons[0]->at(bone_index);
				bones[1] = &skeletons[1]->at(bone_index);

				XMFLOAT4X4 transform_to_parent;
				XMVECTOR Q[2];
				Q[0] = XMQuaternionRotationMatrix(XMLoadFloat4x4(&bones[0]->transform_to_parent));
				Q[1] = XMQuaternionRotationMatrix(XMLoadFloat4x4(&bones[1]->transform_to_parent));
				XMStoreFloat4x4(&transform_to_parent, XMMatrixRotationQuaternion(XMQuaternionNormalize(XMQuaternionSlerp(Q[0], Q[1], ratio))));
				transform_to_parent.m[3][0] = bones[0]->transform_to_parent.m[3][0] * (1 - ratio) + bones[1]->transform_to_parent.m[3][0] * ratio;
				transform_to_parent.m[3][1] = bones[0]->transform_to_parent.m[3][1] * (1 - ratio) + bones[1]->transform_to_parent.m[3][1] * ratio;
				transform_to_parent.m[3][2] = bones[0]->transform_to_parent.m[3][2] * (1 - ratio) + bones[1]->transform_to_parent.m[3][2] * ratio;
				transform_to_parent.m[0][3] = 0;
				transform_to_parent.m[1][3] = 0;
				transform_to_parent.m[2][3] = 0;
				transform_to_parent.m[3][3] = 1;

				XMFLOAT4X4 blended_transform(1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1);
				XMStoreFloat4x4(&blended_transform, XMLoadFloat4x4(&transform_to_parent) * XMLoadFloat4x4(&parent_transform));
				blended_skeleton->at(bone_index).name = node.name;
				blended_skeleton->at(bone_index).transform = blended_transform;
				blended_skeleton->at(bone_index).transform_to_parent = transform_to_parent;
				blend_skeletons(&node, blended_transform, skeletons, ratio, blended_skeleton);
			}
			else
			{
				blend_skeletons(&node, XMFLOAT4X4(1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1), skeletons, ratio, blended_skeleton);
			}
		}
	}

	void fetch_materials(FbxNode* fbx_node, std::unordered_map<std::string, material>& materials)
	{
		const int number_of_materials = fbx_node->GetMaterialCount();
		for (int index_of_material = 0; index_of_material < number_of_materials; ++index_of_material)
		{
			const FbxSurfaceMaterial* surface_material = fbx_node->GetMaterial(index_of_material);
			if (materials.find(surface_material->GetName()) != materials.end())
			{
				// This material is already registered.
				continue;
			}
			std::function<void(material&, const char*, const char*, bool)> fetch_material_property = [&surface_material](material& material, const char* property_name, const char* factor_name, bool force_srgb)
			{
				material::property property;
				property.name = property_name;
				property.force_srgb = force_srgb;

				const FbxProperty fbx_property = surface_material->FindProperty(property_name);
				const FbxProperty fbx_factor = surface_material->FindProperty(factor_name);
				if (fbx_property.IsValid())
				{
					property.color.x = static_cast<float>(fbx_property.Get<FbxDouble3>()[0]);
					property.color.y = static_cast<float>(fbx_property.Get<FbxDouble3>()[1]);
					property.color.z = static_cast<float>(fbx_property.Get<FbxDouble3>()[2]);
					property.color.w = 1.0f;
					const int number_of_textures = fbx_property.GetSrcObjectCount<FbxFileTexture>();
					if (number_of_textures > 0)
					{
						const FbxFileTexture* file_texture = fbx_property.GetSrcObject<FbxFileTexture>();
						if (file_texture)
						{
#if 0
							property.texture_filename = file_texture->GetFileName();
#else
							property.texture_filename = file_texture->GetRelativeFileName();
#endif
							property.u_scale = static_cast<float>(file_texture->GetScaleU());
							property.v_scale = static_cast<float>(file_texture->GetScaleV());
						}
					}
				}
				if (fbx_factor.IsValid())
				{
					property.factor = static_cast<float>(fbx_factor.Get<FbxDouble>());
				}
				material.properties.push_back(property);
			};
			// Fetch material properties.
			material material;
			material.name = surface_material->GetName();
			material.shading_model = surface_material->FindProperty(FbxSurfaceMaterial::sShadingModel).Get<FbxString>();
			material.multiLayer = surface_material->FindProperty(FbxSurfaceMaterial::sMultiLayer).Get<FbxBool>();

			//0
			fetch_material_property(material, FbxSurfaceMaterial::sDiffuse, FbxSurfaceMaterial::sDiffuseFactor, true/*force_srgb*/);

			//1
			fetch_material_property(material, FbxSurfaceMaterial::sAmbient, FbxSurfaceMaterial::sAmbientFactor, true/*force_srgb*/);

			//2
			fetch_material_property(material, FbxSurfaceMaterial::sSpecular, FbxSurfaceMaterial::sSpecularFactor, false/*force_srgb*/);
			FbxDouble shininess = surface_material->FindProperty(FbxSurfaceMaterial::sShininess).Get<FbxDouble>();
			material.properties.at(2).color.w = static_cast<float>(shininess);

			//3
			fetch_material_property(material, FbxSurfaceMaterial::sNormalMap, FbxSurfaceMaterial::sBumpFactor, false/*force_srgb*/);
			//material.properties.at(3).color.x = material.properties.at(3).color.y = 0.5f;
			//material.properties.at(3).color.z = material.properties.at(3).color.w = 1.0f;

			//4
			fetch_material_property(material, FbxSurfaceMaterial::sBump, FbxSurfaceMaterial::sBumpFactor, false/*force_srgb*/);

			//5
		    fetch_material_property(material, FbxSurfaceMaterial::sEmissive, FbxSurfaceMaterial::sEmissiveFactor, false/*force_srgb*/);

			//6
			fetch_material_property(material, FbxSurfaceMaterial::sTransparentColor, FbxSurfaceMaterial::sTransparencyFactor, false/*force_srgb*/);

			//7
			fetch_material_property(material, FbxSurfaceMaterial::sReflection, FbxSurfaceMaterial::sReflectionFactor, false/*force_srgb*/);

			//8
			fetch_material_property(material, FbxSurfaceMaterial::sDisplacementColor, FbxSurfaceMaterial::sDisplacementFactor, false/*force_srgb*/);

			//9
			fetch_material_property(material, FbxSurfaceMaterial::sVectorDisplacementColor, FbxSurfaceMaterial::sVectorDisplacementFactor, false/*force_srgb*/);

			materials.insert(std::make_pair(surface_material->GetName(), material));
		}
	}

	vertex fetch_vertex(FbxMesh* fbx_mesh, FbxStringList& uv_names, int index_of_polygon, int index_of_edge, bool flip_v_coordinates = true)
	{
		vertex vertex;

		bool has_normal = fbx_mesh->GetElementNormalCount() > 0;
		bool has_tangent = fbx_mesh->GetElementTangentCount() > 0;
		bool has_binormal = fbx_mesh->GetElementBinormalCount() > 0;
		bool has_uv = fbx_mesh->GetElementUVCount() > 0;
		bool has_vertex_color = fbx_mesh->GetElementVertexColorCount() > 0;

		const FbxVector4* array_of_control_points = fbx_mesh->GetControlPoints();
		const int index_of_control_point = fbx_mesh->GetPolygonVertex(index_of_polygon, index_of_edge);

		vertex.position.x = static_cast<float>(array_of_control_points[index_of_control_point][0]);
		vertex.position.y = static_cast<float>(array_of_control_points[index_of_control_point][1]);
		vertex.position.z = static_cast<float>(array_of_control_points[index_of_control_point][2]);

		if (has_normal)
		{
#if 1
			FbxVector4 normal;
			fbx_mesh->GetPolygonVertexNormal(index_of_polygon, index_of_edge, normal);
			vertex.normal.x = static_cast<float>(normal[0]);
			vertex.normal.y = static_cast<float>(normal[1]);
			vertex.normal.z = static_cast<float>(normal[2]);
#else
			const FbxGeometryElementNormal* geometry_element_normal = fbx_mesh->GetElementNormal(0);
			FbxGeometryElement::EMappingMode mapping_mode = geometry_element_normal->GetMappingMode();
			FbxGeometryElement::EReferenceMode reference_mode = geometry_element_normal->GetReferenceMode();

			if (mapping_mode == FbxGeometryElement::EMappingMode::eByControlPoint)
			{
				size_t index_of_vertex = 0;
				switch (reference_mode)
				{
				case FbxGeometryElement::EReferenceMode::eDirect:
					index_of_vertex = index_of_control_point;
					break;
				case FbxGeometryElement::EReferenceMode::eIndexToDirect:
					index_of_vertex = geometry_element_normal->GetIndexArray().GetAt(index_of_control_point);
					break;
				case FbxGeometryElement::EReferenceMode::eIndex:
				default:
					_ASSERT_EXPR(false, L"Invalid Reference Mode");
					throw std::exception("Invalid Reference Mode");
				}
				vertex.normal.x = static_cast<float>(geometry_element_normal->GetDirectArray().GetAt(index_of_vertex)[0]);
				vertex.normal.y = static_cast<float>(geometry_element_normal->GetDirectArray().GetAt(index_of_vertex)[1]);
				vertex.normal.z = static_cast<float>(geometry_element_normal->GetDirectArray().GetAt(index_of_vertex)[2]);
			}
			else if (mapping_mode == FbxGeometryElement::EMappingMode::eByPolygonVertex)
			{
				size_t index_of_vertex = 0;
				switch (reference_mode)
				{
				case FbxGeometryElement::EReferenceMode::eDirect:
					index_of_vertex = index_of_polygon * 3 + index_of_edge;
					break;
				case FbxGeometryElement::EReferenceMode::eIndexToDirect:
					index_of_vertex = geometry_element_normal->GetIndexArray().GetAt(index_of_control_point);
					break;
				case FbxGeometryElement::EReferenceMode::eIndex:
				default:
					_ASSERT_EXPR(false, L"Invalid Reference Mode");
					throw std::exception("Invalid Reference Mode");
				}
				vertex.normal.x = static_cast<float>(geometry_element_normal->GetDirectArray().GetAt(index_of_vertex)[0]);
				vertex.normal.y = static_cast<float>(geometry_element_normal->GetDirectArray().GetAt(index_of_vertex)[1]);
				vertex.normal.z = static_cast<float>(geometry_element_normal->GetDirectArray().GetAt(index_of_vertex)[2]);
			}
			else
			{
				_ASSERT_EXPR(false, L"Invalid Mapping Mode");
				throw std::exception("Invalid Mapping Mode");
			}
#endif
		}
		if (has_tangent)
		{
			const FbxGeometryElementTangent* geometry_element_tangent = fbx_mesh->GetElementTangent(0);
			/**	\enum EMappingMode		Determines how the element is mapped to a surface.
			* - \e eNone                The mapping is undetermined.
			* - \e eByControlPoint      There will be one mapping coordinate for each surface control point/vertex.
			* - \e eByPolygonVertex     There will be one mapping coordinate for each vertex, for every polygon of which it is a part.
			*							This means that a vertex will have as many mapping coordinates as polygons of which it is a part.
			* - \e eByPolygon           There can be only one mapping coordinate for the whole polygon.
			* - \e eByEdge              There will be one mapping coordinate for each unique edge in the mesh.
			*							This is meant to be used with smoothing layer elements.
			* - \e eAllSame             There can be only one mapping coordinate for the whole surface.
			*/
			FbxGeometryElement::EMappingMode mapping_mode = geometry_element_tangent->GetMappingMode();
			/** \enum EReferenceMode    Determines how the mapping information is stored in the array of coordinates.
			* - \e eDirect              This indicates that the mapping information for the n'th element is found in the n'th place of
			*							FbxLayerElementTemplate::mDirectArray.
			* - \e eIndex,              This symbol is kept for backward compatibility with FBX v5.0 files. In FBX v6.0 and higher,
			*							this symbol is replaced with eIndexToDirect.
			* - \e eIndexToDirect		This indicates that the FbxLayerElementTemplate::mIndexArray
			*							contains, for the n'th element, an index in the FbxLayerElementTemplate::mDirectArray
			*							array of mapping elements. eIndexToDirect is usually useful for storing eByPolygonVertex mapping
			*							mode elements coordinates. Since the same coordinates are usually
			*							repeated many times, this saves spaces by storing the coordinate only one time
			*							and then referring to them with an index. Materials and Textures are also referenced with this
			*							mode and the actual Material/Texture can be accessed via the FbxLayerElementTemplate::mDirectArray
			*/
			FbxGeometryElement::EReferenceMode reference_mode = geometry_element_tangent->GetReferenceMode();

			if (mapping_mode == FbxGeometryElement::EMappingMode::eByControlPoint)
			{
				int index_of_vertex = 0;
				switch (reference_mode)
				{
				case FbxGeometryElement::EReferenceMode::eDirect:
					index_of_vertex = index_of_control_point;
					break;
				case FbxGeometryElement::EReferenceMode::eIndexToDirect:
					index_of_vertex = geometry_element_tangent->GetIndexArray().GetAt(index_of_control_point);
					break;
				case FbxGeometryElement::EReferenceMode::eIndex:
				default:
					_ASSERT_EXPR(false, L"Invalid Reference Mode");
					throw std::exception("Invalid Reference Mode");
				}
				vertex.tangent.x = static_cast<float>(geometry_element_tangent->GetDirectArray().GetAt(index_of_vertex)[0]);
				vertex.tangent.y = static_cast<float>(geometry_element_tangent->GetDirectArray().GetAt(index_of_vertex)[1]);
				vertex.tangent.z = static_cast<float>(geometry_element_tangent->GetDirectArray().GetAt(index_of_vertex)[2]);
				vertex.tangent.w = static_cast<float>(geometry_element_tangent->GetDirectArray().GetAt(index_of_vertex)[3]);
			}
			else if (mapping_mode == FbxGeometryElement::EMappingMode::eByPolygonVertex)
			{
				int index_of_vertex = 0;
				switch (reference_mode)
				{
				case FbxGeometryElement::EReferenceMode::eDirect:
					index_of_vertex = index_of_polygon * 3 + index_of_edge;
					break;
				case FbxGeometryElement::EReferenceMode::eIndexToDirect:
					index_of_vertex = geometry_element_tangent->GetIndexArray().GetAt(index_of_control_point);
					break;
				case FbxGeometryElement::EReferenceMode::eIndex:
				default:
					_ASSERT_EXPR(false, L"Invalid Reference Mode");
					throw std::exception("Invalid Reference Mode");
				}
				vertex.tangent.x = static_cast<float>(geometry_element_tangent->GetDirectArray().GetAt(index_of_vertex)[0]);
				vertex.tangent.y = static_cast<float>(geometry_element_tangent->GetDirectArray().GetAt(index_of_vertex)[1]);
				vertex.tangent.z = static_cast<float>(geometry_element_tangent->GetDirectArray().GetAt(index_of_vertex)[2]);
				vertex.tangent.w = static_cast<float>(geometry_element_tangent->GetDirectArray().GetAt(index_of_vertex)[3]);
			}
			else
			{
				_ASSERT_EXPR(false, L"Invalid Mapping Mode");
				throw std::exception("Invalid Mapping Mode");
			}
		}
		if (has_binormal)
		{
			const FbxGeometryElementBinormal* geometry_element_binormal = fbx_mesh->GetElementBinormal(0);
			/**	\enum EMappingMode		Determines how the element is mapped to a surface.
			* - \e eNone                The mapping is undetermined.
			* - \e eByControlPoint      There will be one mapping coordinate for each surface control point/vertex.
			* - \e eByPolygonVertex     There will be one mapping coordinate for each vertex, for every polygon of which it is a part.
			*							This means that a vertex will have as many mapping coordinates as polygons of which it is a part.
			* - \e eByPolygon           There can be only one mapping coordinate for the whole polygon.
			* - \e eByEdge              There will be one mapping coordinate for each unique edge in the mesh.
			*							This is meant to be used with smoothing layer elements.
			* - \e eAllSame             There can be only one mapping coordinate for the whole surface.
			*/
			FbxGeometryElement::EMappingMode mapping_mode = geometry_element_binormal->GetMappingMode();
			/** \enum EReferenceMode    Determines how the mapping information is stored in the array of coordinates.
			* - \e eDirect              This indicates that the mapping information for the n'th element is found in the n'th place of
			*							FbxLayerElementTemplate::mDirectArray.
			* - \e eIndex,              This symbol is kept for backward compatibility with FBX v5.0 files. In FBX v6.0 and higher,
			*							this symbol is replaced with eIndexToDirect.
			* - \e eIndexToDirect		This indicates that the FbxLayerElementTemplate::mIndexArray
			*							contains, for the n'th element, an index in the FbxLayerElementTemplate::mDirectArray
			*							array of mapping elements. eIndexToDirect is usually useful for storing eByPolygonVertex mapping
			*							mode elements coordinates. Since the same coordinates are usually
			*							repeated many times, this saves spaces by storing the coordinate only one time
			*							and then referring to them with an index. Materials and Textures are also referenced with this
			*							mode and the actual Material/Texture can be accessed via the FbxLayerElementTemplate::mDirectArray
			*/
			FbxGeometryElement::EReferenceMode reference_mode = geometry_element_binormal->GetReferenceMode();

			if (mapping_mode == FbxGeometryElement::EMappingMode::eByControlPoint)
			{
				int index_of_vertex = 0;
				switch (reference_mode)
				{
				case FbxGeometryElement::EReferenceMode::eDirect:
					index_of_vertex = index_of_control_point;
					break;
				case FbxGeometryElement::EReferenceMode::eIndexToDirect:
					index_of_vertex = geometry_element_binormal->GetIndexArray().GetAt(index_of_control_point);
					break;
				case FbxGeometryElement::EReferenceMode::eIndex:
				default:
					_ASSERT_EXPR(false, L"Invalid Reference Mode");
					throw std::exception("Invalid Reference Mode");
				}
				vertex.binormal.x = static_cast<float>(geometry_element_binormal->GetDirectArray().GetAt(index_of_vertex)[0]);
				vertex.binormal.y = static_cast<float>(geometry_element_binormal->GetDirectArray().GetAt(index_of_vertex)[1]);
				vertex.binormal.z = static_cast<float>(geometry_element_binormal->GetDirectArray().GetAt(index_of_vertex)[2]);
			}
			else if (mapping_mode == FbxGeometryElement::EMappingMode::eByPolygonVertex)
			{
				int index_of_vertex = 0;
				switch (reference_mode)
				{
				case FbxGeometryElement::EReferenceMode::eDirect:
					index_of_vertex = index_of_polygon * 3 + index_of_edge;
					break;
				case FbxGeometryElement::EReferenceMode::eIndexToDirect:
					index_of_vertex = geometry_element_binormal->GetIndexArray().GetAt(index_of_control_point);
					break;
				case FbxGeometryElement::EReferenceMode::eIndex:
				default:
					_ASSERT_EXPR(false, L"Invalid Reference Mode");
					throw std::exception("Invalid Reference Mode");
				}
				vertex.binormal.x = static_cast<float>(geometry_element_binormal->GetDirectArray().GetAt(index_of_vertex)[0]);
				vertex.binormal.y = static_cast<float>(geometry_element_binormal->GetDirectArray().GetAt(index_of_vertex)[1]);
				vertex.binormal.z = static_cast<float>(geometry_element_binormal->GetDirectArray().GetAt(index_of_vertex)[2]);
			}
			else
			{
				_ASSERT_EXPR(false, L"Invalid Mapping Mode");
				throw std::exception("Invalid Mapping Mode");
			}
		}
		if (has_uv)
		{
#if 1
			FbxVector2 uv;
			bool unmapped_uv;
			fbx_mesh->GetPolygonVertexUV(index_of_polygon, index_of_edge, uv_names[0], uv, unmapped_uv);
			vertex.texcoord.x = static_cast<float>(uv[0]);
			vertex.texcoord.y = static_cast<float>(uv[1]);
#else
			const FbxGeometryElementUV* geometry_element_uv = fbx_mesh->GetElementUV(0);
			FbxGeometryElement::EMappingMode mapping_mode = geometry_element_uv->GetMappingMode();
			FbxGeometryElement::EReferenceMode reference_mode = geometry_element_uv->GetReferenceMode();
			if (mapping_mode == FbxGeometryElement::EMappingMode::eByControlPoint)
			{
				size_t index_of_vertex = 0;
				switch (reference_mode)
				{
				case FbxGeometryElement::EReferenceMode::eDirect:
					index_of_vertex = index_of_control_point;
					break;
				case FbxGeometryElement::EReferenceMode::eIndexToDirect:
					index_of_vertex = geometry_element_uv->GetIndexArray().GetAt(index_of_control_point);
					break;
				case FbxGeometryElement::EReferenceMode::eIndex:
				default:
					_ASSERT_EXPR(false, L"Invalid Reference Mode");
					throw std::exception("Invalid Reference Mode");
				}
				vertex.texcoord.x = static_cast<float>(geometry_element_uv->GetDirectArray().GetAt(index_of_vertex)[0]);
				vertex.texcoord.y = static_cast<float>(geometry_element_uv->GetDirectArray().GetAt(index_of_vertex)[1]);
			}
			else if (mapping_mode == FbxGeometryElement::EMappingMode::eByPolygonVertex)
			{
				size_t index_of_vertex = 0;
				switch (reference_mode)
				{
				case FbxGeometryElement::EReferenceMode::eDirect:
					index_of_vertex = index_of_polygon * 3 + index_of_edge;
					break;
				case FbxGeometryElement::EReferenceMode::eIndexToDirect:
					index_of_vertex = geometry_element_uv->GetIndexArray().GetAt(index_of_control_point);
					break;
				case FbxGeometryElement::EReferenceMode::eIndex:
				default:
					_ASSERT_EXPR(false, L"Invalid Reference Mode");
					throw std::exception("Invalid Reference Mode");
				}
				vertex.texcoord.x = static_cast<float>(geometry_element_uv->GetDirectArray().GetAt(index_of_vertex)[0]);
				vertex.texcoord.y = static_cast<float>(geometry_element_uv->GetDirectArray().GetAt(index_of_vertex)[1]);
			}
			else
			{
				_ASSERT_EXPR(false, L"Invalid Mapping Mode");
				throw std::exception("Invalid Mapping Mode");
			}
#endif
			if (flip_v_coordinates)
			{
				vertex.texcoord.y = 1 - vertex.texcoord.y;
			}
		}
		if (has_vertex_color)
		{
			const FbxGeometryElementVertexColor* geometry_element_vertex_color = fbx_mesh->GetElementVertexColor(0);
			FbxGeometryElement::EMappingMode mapping_mode = geometry_element_vertex_color->GetMappingMode();
			FbxGeometryElement::EReferenceMode reference_mode = geometry_element_vertex_color->GetReferenceMode();
			if (mapping_mode == FbxGeometryElement::EMappingMode::eByControlPoint)
			{
				int index_of_vertex = 0;
				switch (reference_mode)
				{
				case FbxGeometryElement::EReferenceMode::eDirect:
					index_of_vertex = index_of_control_point;
					break;
				case FbxGeometryElement::EReferenceMode::eIndexToDirect:
					index_of_vertex = geometry_element_vertex_color->GetIndexArray().GetAt(index_of_control_point);
					break;
				case FbxGeometryElement::EReferenceMode::eIndex:
				default:
					_ASSERT_EXPR(false, L"Invalid Reference Mode");
					throw std::exception("Invalid Reference Mode");
				}
				vertex.color.x = static_cast<float>(geometry_element_vertex_color->GetDirectArray().GetAt(index_of_vertex)[0]);
				vertex.color.y = static_cast<float>(geometry_element_vertex_color->GetDirectArray().GetAt(index_of_vertex)[1]);
				vertex.color.z = static_cast<float>(geometry_element_vertex_color->GetDirectArray().GetAt(index_of_vertex)[2]);
			}
			else if (mapping_mode == FbxGeometryElement::EMappingMode::eByPolygonVertex)
			{
				int index_of_vertex = 0;
				switch (reference_mode)
				{
				case FbxGeometryElement::EReferenceMode::eDirect:
					index_of_vertex = index_of_polygon * 3 + index_of_edge;
					break;
				case FbxGeometryElement::EReferenceMode::eIndexToDirect:
					index_of_vertex = geometry_element_vertex_color->GetIndexArray().GetAt(index_of_control_point);
					break;
				case FbxGeometryElement::EReferenceMode::eIndex:
				default:
					_ASSERT_EXPR(false, L"Invalid Reference Mode");
					throw std::exception("Invalid Reference Mode");
				}
				vertex.color.x = static_cast<float>(geometry_element_vertex_color->GetDirectArray().GetAt(index_of_vertex)[0]);
				vertex.color.y = static_cast<float>(geometry_element_vertex_color->GetDirectArray().GetAt(index_of_vertex)[1]);
				vertex.color.z = static_cast<float>(geometry_element_vertex_color->GetDirectArray().GetAt(index_of_vertex)[2]);
			}
			else
			{
				_ASSERT_EXPR(false, L"Invalid Mapping Mode");
				throw std::exception("Invalid Mapping Mode");
			}
		}
		return vertex;
	}
	vertex fetch_vertex_by_control_point(FbxMesh* fbx_mesh, FbxStringList& uv_names, int index_of_control_point, bool flip_v_coordinates = true)
	{
		vertex vertex;

		bool has_normal = fbx_mesh->GetElementNormalCount() > 0;
		bool has_tangent = fbx_mesh->GetElementTangentCount() > 0;
		bool has_binormal = fbx_mesh->GetElementBinormalCount() > 0;
		bool has_uv = fbx_mesh->GetElementUVCount() > 0;
		bool has_vertex_color = fbx_mesh->GetElementVertexColorCount() > 0;

		const FbxVector4* array_of_control_points = fbx_mesh->GetControlPoints();
		vertex.position.x = static_cast<float>(array_of_control_points[index_of_control_point][0]);
		vertex.position.y = static_cast<float>(array_of_control_points[index_of_control_point][1]);
		vertex.position.z = static_cast<float>(array_of_control_points[index_of_control_point][2]);

		if (has_normal)
		{
			const FbxGeometryElementNormal* geometry_element_normal = fbx_mesh->GetElementNormal(0);
			int index_of_vertex = index_of_control_point;
			if (geometry_element_normal->GetReferenceMode() == FbxGeometryElement::EReferenceMode::eIndexToDirect)
			{
				index_of_vertex = geometry_element_normal->GetIndexArray().GetAt(index_of_control_point);
			}
			vertex.normal.x = static_cast<float>(geometry_element_normal->GetDirectArray().GetAt(index_of_vertex)[0]);
			vertex.normal.y = static_cast<float>(geometry_element_normal->GetDirectArray().GetAt(index_of_vertex)[1]);
			vertex.normal.z = static_cast<float>(geometry_element_normal->GetDirectArray().GetAt(index_of_vertex)[2]);
		}
		if (has_tangent)
		{
			const FbxGeometryElementTangent* geometry_element_tangent = fbx_mesh->GetElementTangent(0);
			int index_of_vertex = index_of_control_point;
			if (geometry_element_tangent->GetReferenceMode() == FbxGeometryElement::EReferenceMode::eIndexToDirect)
			{
				index_of_vertex = geometry_element_tangent->GetIndexArray().GetAt(index_of_control_point);
			}
			vertex.tangent.x = static_cast<float>(geometry_element_tangent->GetDirectArray().GetAt(index_of_vertex)[0]);
			vertex.tangent.y = static_cast<float>(geometry_element_tangent->GetDirectArray().GetAt(index_of_vertex)[1]);
			vertex.tangent.z = static_cast<float>(geometry_element_tangent->GetDirectArray().GetAt(index_of_vertex)[2]);
			vertex.tangent.w = static_cast<float>(geometry_element_tangent->GetDirectArray().GetAt(index_of_vertex)[3]);
		}
		if (has_binormal)
		{
			const FbxGeometryElementBinormal* geometry_element_binormal = fbx_mesh->GetElementBinormal(0);
			int index_of_vertex = index_of_control_point;
			if (geometry_element_binormal->GetReferenceMode() == FbxGeometryElement::EReferenceMode::eIndexToDirect)
			{
				index_of_vertex = geometry_element_binormal->GetIndexArray().GetAt(index_of_control_point);
			}
			vertex.binormal.x = static_cast<float>(geometry_element_binormal->GetDirectArray().GetAt(index_of_vertex)[0]);
			vertex.binormal.y = static_cast<float>(geometry_element_binormal->GetDirectArray().GetAt(index_of_vertex)[1]);
			vertex.binormal.z = static_cast<float>(geometry_element_binormal->GetDirectArray().GetAt(index_of_vertex)[2]);
		}
		if (has_uv)
		{
			const FbxGeometryElementUV* geometry_element_uv = fbx_mesh->GetElementUV(0);
			int index_of_vertex = index_of_control_point;
			if (geometry_element_uv->GetReferenceMode() == FbxGeometryElement::EReferenceMode::eIndexToDirect)
			{
				index_of_vertex = geometry_element_uv->GetIndexArray().GetAt(index_of_control_point);
			}
			vertex.texcoord.x = static_cast<float>(geometry_element_uv->GetDirectArray().GetAt(index_of_vertex)[0]);
			vertex.texcoord.y = static_cast<float>(geometry_element_uv->GetDirectArray().GetAt(index_of_vertex)[1]);
			if (flip_v_coordinates)
			{
				vertex.texcoord.y = 1 - vertex.texcoord.y;
			}
		}
		if (has_vertex_color)
		{
			const FbxGeometryElementVertexColor* geometry_element_vertex_color = fbx_mesh->GetElementVertexColor(0);
			int index_of_vertex = index_of_control_point;
			if (geometry_element_vertex_color->GetReferenceMode() == FbxGeometryElement::EReferenceMode::eIndexToDirect)
			{
				index_of_vertex = geometry_element_vertex_color->GetIndexArray().GetAt(index_of_control_point);
			}
			vertex.color.x = static_cast<float>(geometry_element_vertex_color->GetDirectArray().GetAt(index_of_vertex)[0]);
			vertex.color.y = static_cast<float>(geometry_element_vertex_color->GetDirectArray().GetAt(index_of_vertex)[1]);
			vertex.color.z = static_cast<float>(geometry_element_vertex_color->GetDirectArray().GetAt(index_of_vertex)[2]);
			vertex.color.w = static_cast<float>(geometry_element_vertex_color->GetDirectArray().GetAt(index_of_vertex)[3]);
		}
		return vertex;
	}
	void fetch_vertices_and_indices(FbxMesh* fbx_mesh, std::vector<vertex>& vertices, std::vector<std::uint32_t>& indices, std::vector<subset>& subsets)
	{
		// How to Work with FBX SDK https://www.gamedev.net/articles/programming/graphics/how-to-work-with-fbx-sdk-r3582
		bool all_by_control_point = true;
		bool has_normal = fbx_mesh->GetElementNormalCount() > 0;
		bool has_tangent = fbx_mesh->GetElementTangentCount() > 0;
		bool has_binormal = fbx_mesh->GetElementBinormalCount() > 0;
		bool has_uv = fbx_mesh->GetElementUVCount() > 0;
		bool has_vertex_color = fbx_mesh->GetElementVertexColorCount() > 0;
		if (has_normal && fbx_mesh->GetElementNormal(0)->GetMappingMode() != FbxGeometryElement::EMappingMode::eByControlPoint)
		{
			all_by_control_point = false;
		}
		if (has_tangent && fbx_mesh->GetElementTangent(0)->GetMappingMode() != FbxGeometryElement::EMappingMode::eByControlPoint)
		{
			all_by_control_point = false;
		}
		if (has_binormal && fbx_mesh->GetElementBinormal(0)->GetMappingMode() != FbxGeometryElement::EMappingMode::eByControlPoint)
		{
			all_by_control_point = false;
		}
		if (has_uv && fbx_mesh->GetElementUV(0)->GetMappingMode() != FbxGeometryElement::EMappingMode::eByControlPoint)
		{
			all_by_control_point = false;
		}
		if (has_vertex_color && fbx_mesh->GetElementVertexColor(0)->GetMappingMode() != FbxGeometryElement::EMappingMode::eByControlPoint)
		{
			all_by_control_point = false;
		}

		const size_t number_of_polygons = fbx_mesh->GetPolygonCount();
		const FbxVector4* array_of_control_points = fbx_mesh->GetControlPoints();

		std::vector<bone_influences_per_control_point> bone_influences;
		fetch_bone_influences(fbx_mesh, bone_influences);

		FbxStringList uv_names;
		fbx_mesh->GetUVSetNames(uv_names);

		// Fetch vertices
		if (all_by_control_point)
		{
			const size_t number_of_control_points = fbx_mesh->GetControlPointsCount();
			for (size_t index_of_control_point = 0; index_of_control_point < number_of_control_points; ++index_of_control_point)
			{
				vertex vertex;
				vertex = fetch_vertex_by_control_point(fbx_mesh, uv_names, static_cast<int>(index_of_control_point));

				bone_influences_per_control_point influences_per_control_point = bone_influences.at(index_of_control_point);
				set_bone_influences_to_vertex(vertex, influences_per_control_point);

				vertices.push_back(vertex);
			}
		}
		else
		{
			for (int index_of_polygon = 0; index_of_polygon < number_of_polygons; index_of_polygon++)
			{
				for (int index_of_edge = 0; index_of_edge < 3; index_of_edge++)
				{
					vertex fetched_vertex;
					fetched_vertex = fetch_vertex(fbx_mesh, uv_names, index_of_polygon, index_of_edge);

					const size_t index_of_control_point = fbx_mesh->GetPolygonVertex(index_of_polygon, index_of_edge);
					bone_influences_per_control_point influences_per_control_point = bone_influences.at(index_of_control_point);
					set_bone_influences_to_vertex(fetched_vertex, influences_per_control_point);

					vertices.push_back(fetched_vertex);
				}
			}
		}

		// Build subsets for each material
		const size_t number_of_materials = fbx_mesh->GetNode()->GetMaterialCount();
		subsets.resize(number_of_materials > 0 ? number_of_materials : 1);
		for (size_t index_of_material = 0; index_of_material < number_of_materials; ++index_of_material)
		{
			const FbxSurfaceMaterial* surface_material = fbx_mesh->GetNode()->GetMaterial(static_cast<int>(index_of_material));
			subsets.at(index_of_material).name = surface_material->GetName();
		}
		if (number_of_materials > 0)
		{
			// Count the faces of each material
			const size_t number_of_polygons = fbx_mesh->GetPolygonCount();
			for (size_t index_of_polygon = 0; index_of_polygon < number_of_polygons; ++index_of_polygon)
			{
				const size_t index_of_material = fbx_mesh->GetElementMaterial()->GetIndexArray().GetAt(static_cast<int>(index_of_polygon));
				subsets.at(index_of_material).index_count += 3;
			}

			// Record the offset (how many vertex)
			size_t offset = 0;
			for (subset& subset : subsets)
			{
				subset.index_start = offset;
				offset += subset.index_count;
				// This will be used as counter in the following procedures, reset to zero
				subset.index_count = 0;
			}
		}

		// Rebulid indices array
		size_t vertex_count = 0;
		indices.resize(number_of_polygons * 3);
		for (int index_of_polygon = 0; index_of_polygon < number_of_polygons; index_of_polygon++)
		{
			// The material for current face
			size_t index_of_material;
			if (number_of_materials > 0)
			{
				index_of_material = fbx_mesh->GetElementMaterial()->GetIndexArray().GetAt(index_of_polygon);
			}
			else // If fbx_mesh has no material then make one dummy material
			{
				index_of_material = 0;
			}

			// Where should I save the vertex attribute index, according to the material
			subset& subset = subsets.at(index_of_material);
			const size_t index_offset = subset.index_start + subset.index_count;

			for (int index_of_edge = 0; index_of_edge < 3; ++index_of_edge)
			{
				const int index_of_control_point = fbx_mesh->GetPolygonVertex(index_of_polygon, index_of_edge);
				if (all_by_control_point)
				{
					indices.at(index_offset + index_of_edge) = static_cast<std::uint32_t>(index_of_control_point);
				}
				else
				{
					indices.at(index_offset + index_of_edge) = static_cast<std::uint32_t>(vertex_count);
				}
				vertex_count += 1;
			}
			subset.index_count += 3;
		}
		if (!has_normal)
		{
			//calculate_normal_vector(vertices, indices);
			has_normal = true;
		}
		if (!has_tangent && has_uv && has_normal)
		{
			//calculate_tangent_vector(vertices, indices);
			has_tangent = true;
		}
	}
	void calculate_tangent_vector(std::vector<vertex>& vertices, const std::vector<std::uint32_t>& indices)
	{
		size_t number_of_vertices = vertices.size();
		XMFLOAT3* tan1 = new XMFLOAT3[number_of_vertices * 2];
		XMFLOAT3* tan2 = tan1 + number_of_vertices;
		std::fill_n(tan1, number_of_vertices * 2, XMFLOAT3(0, 0, 0));

		size_t number_of_triangles = indices.size() / 3;
		for (size_t index_of_triangle = 0; index_of_triangle < number_of_triangles; index_of_triangle++)
		{
			const size_t i1 = indices.at(index_of_triangle * 3 + 0);
			const size_t i2 = indices.at(index_of_triangle * 3 + 1);
			const size_t i3 = indices.at(index_of_triangle * 3 + 2);

			const vertex& v1 = vertices.at(i1);
			const vertex& v2 = vertices.at(i2);
			const vertex& v3 = vertices.at(i3);

			const float x1 = v2.position.x - v1.position.x;
			const float x2 = v3.position.x - v1.position.x;
			const float y1 = v2.position.y - v1.position.y;
			const float y2 = v3.position.y - v1.position.y;
			const float z1 = v2.position.z - v1.position.z;
			const float z2 = v3.position.z - v1.position.z;

			const float s1 = v2.texcoord.x - v1.texcoord.x;
			const float s2 = v3.texcoord.x - v1.texcoord.x;
			bool flipped_v_coordinates = true;
			const float t1 = flipped_v_coordinates ? (1 - v2.texcoord.y) - (1 - v1.texcoord.y) : v2.texcoord.y - v1.texcoord.y;
			const float t2 = flipped_v_coordinates ? (1 - v3.texcoord.y) - (1 - v1.texcoord.y) : v3.texcoord.y - v1.texcoord.y;

			const float r = 1.0f / (s1 * t2 - s2 * t1);

			const XMFLOAT3 sdir((t2 * x1 - t1 * x2) * r, (t2 * y1 - t1 * y2) * r, (t2 * z1 - t1 * z2) * r);
			const XMFLOAT3 tdir((s1 * x2 - s2 * x1) * r, (s1 * y2 - s2 * y1) * r, (s1 * z2 - s2 * z1) * r);

			tan1[i1].x += sdir.x;
			tan1[i1].y += sdir.y;
			tan1[i1].z += sdir.z;
			tan1[i2].x += sdir.x;
			tan1[i2].y += sdir.y;
			tan1[i2].z += sdir.z;
			tan1[i3].x += sdir.x;
			tan1[i3].y += sdir.y;
			tan1[i3].z += sdir.z;

			tan2[i1].x += tdir.x;
			tan2[i1].y += tdir.y;
			tan2[i1].z += tdir.z;
			tan2[i2].x += tdir.x;
			tan2[i2].y += tdir.y;
			tan2[i2].z += tdir.z;
			tan2[i3].x += tdir.x;
			tan2[i3].y += tdir.y;
			tan2[i3].z += tdir.z;
		}

		for (size_t index_of_vertex = 0; index_of_vertex < number_of_vertices; index_of_vertex++)
		{
			vertex& v = vertices.at(index_of_vertex);

			XMVECTOR N = XMLoadFloat3(&v.normal);
			XMVECTOR T = XMLoadFloat3(&tan1[index_of_vertex]);
			XMVECTOR B = XMLoadFloat3(&tan2[index_of_vertex]);

			XMStoreFloat4(&v.tangent, XMVector3Normalize(XMVectorSubtract(T, XMVectorMultiply(N, XMVector3Dot(T, N)))));
			v.tangent.w = XMVectorGetX(XMVector3Dot(XMVector3Cross(N, T), B)) < 0.0f ? -1.0f : 1.0f;

			XMStoreFloat3(&v.binormal, XMVector3Normalize(XMVector3Cross(N, T)));
		}

		delete[] tan1;
	}
	void calculate_normal_vector(std::vector<vertex>& vertices, const std::vector<std::uint32_t>& indices)
	{
		// TODO:mada

	}
}