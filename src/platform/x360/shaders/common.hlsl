struct VS_IN
{
	float3 aCoord : POSITION;
	float2 aTexCoord : TEXCOORD0;
	float4 aColor : COLOR;
};

#ifdef VERTEX

float4x4 uProjMatrix : register(c0);
float4x4 uViewMatrix : register(c4);
float4 uColor : register(c8);

#else

sampler2D sDiffuse : register(s0);
sampler3D sTrackTex : register(s1);

#endif