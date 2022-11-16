#include "light_manager.h"
#include "../../imgui/imgui.h"

//initialize
void light_manager::initialize(ID3D11Device *device)
{
	HRESULT hr;
	hr = create_ps_from_cso(device, "shader//dir_light_ps.cso", dir_light_ps.GetAddressOf());
	hr = create_ps_from_cso(device, "shader//dir_light_shadow_ps.cso", dir_light_shadow_ps.GetAddressOf());

	light_screen = std::make_unique<fullscreen_quad>(device);

#if CAST_SHADOW
	DirectX::XMFLOAT3 shadow_light_dir = { 1.0f, 1.0f, -1.0 };
	DirectX::XMFLOAT3 shadow_color = { 0.2f, 0.2f, 0.2f };
	shadow_dir_light = std::make_unique<DirectionalLight>(graphics, shadow_light_dir, shadow_color.x, shadow_color.y, shadow_color.z);
#endif
	_ASSERT_EXPR(SUCCEEDED(hr), hr_trace(hr));
}

light_manager::~light_manager()
{
	lights.clear();
}

//register lights
void light_manager::register_light(std::string name, std::shared_ptr<light> light)
{
	//ID
	int id = 0;
	//���̖��O
	std::string  unique_name = name;
	for (auto& l : lights)
	{
		//�������O�����Ԃ��Ă�����
		if (name == l.first)
		{
			//���̖��O�̌��ɐ�����t������
			id++;
			name = unique_name + std::to_string(id);
		}
	}
	//�}�b�v�ɓo�^
	lights[name] = light;
	//���C�g�ɕύX�������O��o�^
	light->name = name;
}


void light_manager::draw(ID3D11DeviceContext* immediate_context, ID3D11ShaderResourceView** rtv, int rtv_num)
{

	//light for shadow
#if CAST_SHADOW
	shadow_dir_light->light_constants->bind(graphics.get_dc().Get(), 7);
	light_screen->blit(graphics.get_dc().Get(), rtv, 0, rtv_num, shadow_map_light.Get());
#endif
	//normal light draw
	for (auto& light : lights)
	{
		//�����N���؂�Ă��Ȃ����`�F�b�N
		if (!light.second.expired())
		{
			//�Ď����Ă��郉�C�g�̊֐�����
			light.second.lock()->light_constants->active(immediate_context, 2,true,true);
			light_screen->blit(immediate_context, rtv, 0 , rtv_num, dir_light_ps.Get());
		}
		//�v�f���Ȃ��̂�map�̗̈���Ƃ��Ă���Ƃ��Ɍx�����o��
		_ASSERT_EXPR(!light.second.expired(), L"light_map��nullptr�����݂��Ă��܂�\n delete_light()���ĂіY��Ă���\��������܂�");
	}
}

//debug gui
void light_manager::DrawDebugGUI()
{
	ImGui::Begin("Lights", nullptr, ImGuiWindowFlags_None);
	if (display_imgui)
	{
#if CAST_SHADOW
		shadow_dir_light->debug_gui(-1);
#endif
		for (auto& l : lights)
		{
			if (!l.second.expired())
			{
				l.second.lock()->DebugDrawGUI(l.first);
			}
		}
	}
	ImGui::End();
}

//delete lights
void light_manager::delete_light(std::string name)
{
	//�w��̃L�[�̗v�f���}�b�v����폜
	lights.erase(name);
}
