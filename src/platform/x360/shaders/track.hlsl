#include "common.hlsl"

#define MAX_TRACK_TEX 22

struct VS_OUT
{
	float4 oPosition : POSITION;
	float3 vTexCoord : TEXCOORD0;
	float3 vColor : COLOR;
};

#ifdef VERTEX

VS_OUT main(VS_IN In)
{
	VS_OUT Out;
	Out.oPosition = mul(mul(float4(In.aCoord.xyz, 1.0), uViewMatrix), uProjMatrix);
	Out.vTexCoord.xy = In.aTexCoord;
	Out.vTexCoord.z = 1.0f / ( 2.0f * MAX_TRACK_TEX ) + (In.aColor.w * 255) / MAX_TRACK_TEX;
	Out.vColor = In.aColor.zyx;
	return Out;
}

#else

float4 main(VS_OUT In) : COLOR
{
	float4 fragColor = tex3D(sTrackTex, In.vTexCoord.xyz);
	fragColor.xyz *= In.vColor * 2.0;
	clip(fragColor.a - 0.1f);
	return fragColor;
}

#endif
