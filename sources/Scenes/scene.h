#pragma once

#include <cstdint>
#include <d3d11.h>


class Scene
{
public:
	virtual bool initialize(ID3D11Device* device, CONST LONG screen_width, CONST LONG screen_height) = 0;
	virtual const char* update(float& elapsed_time/*Elapsed seconds from last frame*/) = 0;
	virtual void render(ID3D11DeviceContext* immediate_context, float elapsed_time/*Elapsed seconds from last frame*/) = 0;
	virtual void finilize() = 0;

	Scene() = default;
	virtual ~Scene() = default;
	Scene(Scene&) = delete;
	void operator=(Scene&) = delete;
};
