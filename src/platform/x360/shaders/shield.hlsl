#include "common.hlsl"

struct VS_OUT
{
	float4 oPosition : POSITION;
	float3 vColor : COLOR;
};

#ifdef VERTEX

VS_OUT main(VS_IN In)
{
	VS_OUT Out;
	float v = In.aCoord.x + In.aCoord.y + In.aCoord.z;
	float a = sin(v + uColor.r * 64.0) * 0.5 + 0.5;
	float b = sin(In.aCoord.z + uColor.r * 128.0) * 0.5 + 0.5;
	float3 pos = In.aCoord;
	pos.x += sign(pos.x) * a * 8.0;
	pos.y += sign(pos.y) * b * 8.0;
	b *= 0.25;
	Out.vColor = float3(a * 0.5 + b, 0.25 + b, 0.5 + b) * 0.5;
	Out.oPosition = mul(mul(float4(pos, 1.0), uViewMatrix), uProjMatrix);
	return Out;
}

#else

float4 main(VS_OUT In): COLOR
{
	return float4(In.vColor, 1.0);
}

#endif
