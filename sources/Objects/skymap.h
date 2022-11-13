#pragma once
#include <d3d11.h>
#include <DirectXMath.h>
#include "../Graphics/load/substance.h"
#include "../Graphics/others/constant_buffer.h"
#include "../Graphics/others/render_state.h"
#include "../Graphics/mesh/static_mesh.h"

//背景　(スカイドームなど)
class skydome
{
public:
    virtual ~skydome() = default;

    void Initialize(ID3D11Device* device);
    void update(float elapsedTime);
    void Render(ID3D11DeviceContext* dc);
    //デバッグ用GUI描画
    void DrawDebugGUI();

protected:

    DirectX::XMFLOAT3       position = { 0.0f,0.0f,0.0f };
    DirectX::XMFLOAT3       angle = { 0,0,0 };
    DirectX::XMFLOAT3       scale = { 50.0f,50.0f,50.0f };
    DirectX::XMFLOAT4X4     transform = {
        1,0,0,0,
        0,1,0,0,
        0,0,1,0,
        0,0,0,1
    };

    //行列更新処理
    void UpdateTransform();
    // static_mesh* mesh;

private:
    D3D11_TEXTURE2D_DESC skymap_texture2dDesc;
    Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> skymap_texture;
    std::unique_ptr<static_mesh> sky;

};

