#include "common.hlsl"

struct VS_OUT
{
	float4 oPosition : POSITION;
	float2 vTexCoord : TEXCOORD0;
	float vIntensity : COLOR;
};

#ifdef VERTEX

VS_OUT main(VS_IN In)
{
	VS_OUT Out;
	Out.oPosition = mul(mul(float4(In.aCoord.xyz, 1.0), uViewMatrix), uProjMatrix);
	Out.vTexCoord = In.aTexCoord;
	Out.vIntensity = In.aColor.w;
	return Out;
}

#else

float4 main(VS_OUT In) : COLOR
{
	float s = 1.0 - saturate(tex2D(sDiffuse, In.vTexCoord).x * 2.0 * In.vIntensity);
	return float4(s, s, s, 1.0);
}

#endif
