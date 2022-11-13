#include <Shlwapi.h>
#include <iostream>
#include <filesystem>
#include <DirectXMath.h>

#include "../Functions/utility_graphics.h"
#include "../Graphics/others/shader.h"
#include "../Graphics/load/texture.h"
#include "../../imgui/imgui.h"
#include "skymap.h"

void skydome::Initialize(ID3D11Device* device)
{
	sky = std::make_unique<static_mesh>(device, "./resources/sky.fbx");
}


void skydome::update(float elapsedTime)
{
	UpdateTransform();
}

void skydome::Render(ID3D11DeviceContext* dc)
{
	//dc->PSSetShaderResources(4, 1, skymap_texture.GetAddressOf());
	sky->render(dc, transform);
	//shader->Draw(dc, model);
}

void skydome::UpdateTransform()
{
	//�X�P�[���s����쐬
	DirectX::XMMATRIX S = DirectX::XMMatrixScaling(scale.x, scale.y, scale.z);

	//��]�s����쐬
	DirectX::XMMATRIX R = DirectX::XMMatrixRotationRollPitchYaw(angle.x, angle.y, angle.z);

	//�ʒu�s����쐬
	DirectX::XMMATRIX T = DirectX::XMMatrixTranslation(position.x, position.y, position.z);

	//3�̍s���g�ݍ��킹�A���[���h���쐬
	DirectX::XMMATRIX W = S * R * T;

	//�v�Z�������[���h�s������o��
	DirectX::XMStoreFloat4x4(&transform, W);
}

void skydome::DrawDebugGUI()
{
#ifdef _DEBUG
	ImGui::SetNextWindowPos(ImVec2(10, 10), ImGuiCond_FirstUseEver);
	ImGui::SetNextWindowSize(ImVec2(300, 300), ImGuiCond_FirstUseEver);

	if (ImGui::Begin("Sky", nullptr, ImGuiWindowFlags_None))
	{
		//�g�����X�t�H�[��
		if (ImGui::CollapsingHeader("SkyTransform", ImGuiTreeNodeFlags_DefaultOpen))
		{
			//�ʒu
			ImGui::SliderFloat3("Position", &position.x,-100,100);
			//�X�P�[��
			ImGui::InputFloat3("Scale", &scale.x);

			//��]
			DirectX::XMFLOAT3 a;
			a.x = DirectX::XMConvertToDegrees(angle.x);
			a.y = DirectX::XMConvertToDegrees(angle.y);
			a.z = DirectX::XMConvertToDegrees(angle.z);

			ImGui::InputFloat3("Angle", &a.x);

			angle.x = DirectX::XMConvertToRadians(a.x);
			angle.y = DirectX::XMConvertToRadians(a.y);
			angle.z = DirectX::XMConvertToRadians(a.z);

		}
	}
	ImGui::End();
#endif
}

