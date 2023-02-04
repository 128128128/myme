#include "water_fall_particles.hlsli"
#include "noise.hlsli"

RWStructuredBuffer<particle> waterfall_particle_buffer : register(u0);


[numthreads(256, 1, 1)]
void main(uint3 dtid : SV_DISPATCHTHREADID)
{
	uint id = dtid.x;

	particle p = waterfall_particle_buffer[id];


	if (length(p.position.xz - current_eye_position.xz) > outermost_radius)
	{
		p.position.xz = current_eye_position.xz - (p.position.xz - previous_eye_position.xz);
	}

	const float amplitude = 5.3;
	//p.velocity.x = amplitude * p.direction.x;
	//p.velocity.z = amplitude * p.direction.z;
	//p.velocity.y =  amplitude;
	//p.velocity += p.direction * elapsed_time;
	p.position += p.velocity * elapsed_time;

	//when out of height area
	if (p.position.y < current_eye_position.y - snowfall_area_height * 0.5)
	{
		p.position.y += snowfall_area_height;
	}
	else if (p.position.y > current_eye_position.y + snowfall_area_height * 0.5)
	{
		p.position.y -= snowfall_area_height;
	}

	waterfall_particle_buffer[id] = p;

}