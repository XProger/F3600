#include "common.hlsl"

struct VS_OUT
{
	float4 oPosition : POSITION;
	float2 vTexCoord : TEXCOORD0;
	float3 vColor : COLOR;
};

#ifdef VERTEX

VS_OUT main(VS_IN In)
{
	VS_OUT Out;
	Out.oPosition = mul(mul(float4(In.aCoord.xyz, 1.0), uViewMatrix), uProjMatrix);
	Out.vTexCoord = In.aTexCoord;
	Out.vColor = In.aColor.zyx;
	return Out;
}

#else

float4 main(VS_OUT In) : COLOR
{
	float4 fragColor = tex2D(sDiffuse, In.vTexCoord);
	fragColor.xyz *= In.vColor * 2.0;
	clip(fragColor.a - 0.1f);
	return fragColor;
}

#endif
