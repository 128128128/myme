#include "snowfall_particles.hlsli"
#include "noise.hlsli"

RWStructuredBuffer<particle> snowfall_particle_buffer : register(u0);

//[numthreads(256, 1, 1)]
//void main(uint3 dtid : SV_DISPATCHTHREADID)
//{
//	uint id = dtid.x;
//	particle p = snowfall_particle_buffer[id];
//
//	p.position += p.velocity * 0.03;
//
//	if (p.position.y < current_eye_position.y - snowfall_area_height * 0.5)
//	{
//		p.position.y += snowfall_area_height;
//	}
//	else if (p.position.y > current_eye_position.y + snowfall_area_height * 0.5)
//	{
//		p.position.y -= snowfall_area_height;
//	}
//
//	snowfall_particle_buffer[id] = p;
//}


[numthreads(256, 1, 1)]
void main(uint3 dtid : SV_DISPATCHTHREADID)
{
	uint id = dtid.x;
	
		particle p = snowfall_particle_buffer[id];


		if (length(p.position.xz - current_eye_position.xz) > outermost_radius)
		{
			p.position.xz = current_eye_position.xz - (p.position.xz - previous_eye_position.xz);
		}

		const float amplitude = 0.3;
		p.velocity.x = snoise(p.position.xyz * frac(time) * 10.0) * amplitude;
		p.velocity.z = snoise(p.position.zyx * frac(time) * 10.0) * amplitude;
		p.position += p.velocity * elapsed_time;

		if (p.position.y < current_eye_position.y - snowfall_area_height * 0.5)
		{
			p.position.y += snowfall_area_height;
		}
		else if (p.position.y > current_eye_position.y + snowfall_area_height * 0.5)
		{
			p.position.y -= snowfall_area_height;
		}

		snowfall_particle_buffer[id] = p;
	
}

//void main(uint3 dtid : SV_DISPATCHTHREADID)
//{
//	uint id = dtid.x;
//	particle p = snowfall_particle_buffer[id];
//
//	if (length(p.position.xz - current_eye_position.xz) > outermost_radius)
//	{
//		p.position.xz = current_eye_position.xz - (p.position.xz - previous_eye_position.xz);
//	}
//	
//	const float amplitude = 0.3;
//	p.velocity.x = snoise(p.position.xyz * frac(light_direction.iTime) * 10.0) * amplitude;
//	p.velocity.z = snoise(p.position.zyx * frac(light_direction.iTime) * 10.0) * amplitude;
//	p.position += p.velocity * light_direction.elapsedTime;
//
//	if (p.position.y < current_eye_position.y - snowfall_area_height * 0.5)
//	{
//		p.position.y += snowfall_area_height;
//	}
//	else if (p.position.y > current_eye_position.y + snowfall_area_height * 0.5)
//	{
//		p.position.y -= snowfall_area_height;
//	}
//
//	snowfall_particle_buffer[id] = p;
//}