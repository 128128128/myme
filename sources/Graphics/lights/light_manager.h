#pragma once
#define CAST_SHADOW 0
#include <map>

#include "light.h"
#include "../others/shader.h"
#include "../deferred/df_fullscreen_quad.h"

class light_manager
{
	light_manager() {};
	~light_manager();
public:
	void initialize(ID3D11Device* device);
	//ライトを監視対象に追加
	void  register_light(std::string name, std::shared_ptr<light> light);
	//ライトをG-Bufferに送る
	void draw(ID3D11DeviceContext* immediate_context, ID3D11ShaderResourceView** rtv, int rtv_num);

	void DrawDebugGUI(bool flag=false);

	void delete_light(std::string name);

	static light_manager& instance()
	{
		static light_manager light_manager;
		return light_manager;
	}

#if CAST_SHADOW
	DirectX::XMFLOAT3 get_shadow_dir_light_dir() { return shadow_dir_light.get()->get_direction(); }
#endif
private:
	std::map<std::string, std::weak_ptr<light>>lights;
	std::unique_ptr<fullscreen_quad> light_screen;
#if CAST_SHADOW
	std::unique_ptr<DirectionalLight> shadow_dir_light;//shadow map light
#endif
	Microsoft::WRL::ComPtr<ID3D11PixelShader> dir_light_shadow_ps;
	Microsoft::WRL::ComPtr<ID3D11PixelShader> dir_light_ps;

	bool display_imgui = true;

};