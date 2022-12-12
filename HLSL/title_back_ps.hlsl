#include "title_back.hlsli"

#define PI 3.14159265359
#define TPI 6.28318530718
#define HPI 1.57079632679
#define SCALE 2.0
//#define PI radians(180.0)
#define TAU (PI*2.0)
#define CL(x,a,b) smoothstep(0.0,1.0,(2.0/3.0)*(x-a)/(b-a)+(1.0/6.0))*(b-a)+a // https://www.shadertoy.com/view/Ws3Xzr

float4 main(VS_OUT pin) :SV_TARGET
{
	/* {
		const float2 dis = float2(pin.position.x / 1280,pin.position.y / 720);
		return  float4(dis, abs(sin(itime)), 1.0);
	}*/
	{
   float2 iResolution = float2(1280,720);
   float st = radians(-31.0); // start time
   float t = st + (itime * TAU) / 3600.0;
   float n = (cos(t) > 0.0) ? sin(t) : 1.0 / sin(t);
   float z = pow(500.0, n); // autozoom
   z = clamp(z, CL(z, 1e-16, 1e-15), CL(z, 1e+17, 1e+18)); // clamp to prevent black screen
   float2 uv = (pin.position.xy - 0.5 * iResolution.xy) / iResolution.y * SCALE * z;
   float ls = (itime * TAU) / 5.0; // light animation speed
   float a = atan2(uv.x, -uv.y); // screen arc
   float i = a / TAU; // spiral increment 0.5 per 180Åã
   float r = pow(length(uv), 0.5 / n) - i; // archimedean at 0.5
   float cr = ceil(r); // round up radius
   float wr = cr + i; // winding ratio
   float vd = (cr * TAU + a) / (n * 2.0); // visual denominator
   float vd2 = vd * 2.0;
   float3 col = float(cos(wr * vd + ls)); // blend it
   col *= pow(sin(frac(r) * PI), floor(abs(n * 2.0)) + 5.0); // smooth edges
   col *= sin(vd2 * wr + PI / 2.0 + ls * 2.0); // this looks nice
   col *= 0.2 + abs(cos(vd2)); // dark spirals
   float3 g = lerp(float3(0,0,0), float3(1,1,1), pow(length(uv) / z, -1.0 / n)); // dark gradient
   col = min(col, g); // blend gradient with spiral
   float3 rgb = float3(cos(vd2) + 1.0, abs(sin(t)), cos(PI + vd2) + 1.0);
   col += (col * 2.0) - (rgb * 0.5); // add color
   return float4(col, 1.0);
	}

 //    {
	//	 float2 iResolution = float2(1280,720);
	//	 float2 p = (2.0 * pin.texcoord.xy - iResolution.xy) / iResolution.y;
	//float tau = 3.1415926535 * 2.0;
	//float a = atan2(p.x,p.y);
	//float r = length(p) * 0.75;
	//float2 uv = float2(a / tau,r);
	////get the color
	//float xCol = (uv.x - (itime / 3.0)) * 3.0;
	//xCol = fmod(xCol, 3.0);
	//float3 horColour = float3(0.25, 0.25, 0.25);
	//if (xCol < 1.0) {
	//	horColour.r += 1.0 - xCol;
	//	horColour.g += xCol;
	//}
	//else if (xCol < 2.0) {
	//	xCol -= 1.0;
	//	horColour.g += 1.0 - xCol;
	//	horColour.b += xCol;
	//}
	//else {
	//	xCol -= 2.0;
	//	horColour.b += 1.0 - xCol;
	//	horColour.r += xCol;
	//}
	//// draw color beam
	//uv = (2.0 * uv) - 1.0;
	//float beamWidth = (cos(uv.x * tau * 0.15 * floor(5.0 + 10.0 * cos(itime)))) * abs(1.0 / (30.0 * uv.y));
	//float3 horBeam = float3(beamWidth, 0, 0);
	//return (((horBeam)*horColour), 1.0);
 //    }

	
}
