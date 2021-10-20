cbuffer cbuff0 : register(b0)
{
	matrix w;
	matrix v;
	matrix p;
	float3 eye;
};

cbuffer Material : register(b1)
{
	float4 ambient;
	float4 diffuse;
	float4 specular;
	float Nspecular;
};

Texture2D<float4> ambtex : register(t0);
Texture2D<float4> diftex : register(t1);
Texture2D<float4> spetex : register(t2);

SamplerState smp: register(s0);

struct Output {
	float4 svpos : SV_POSITION;
	float4 normal : NORMAL0;
	float4 vnormal : NORMAL1;
	float2 uv : TEXCOORD;
	float3 ray : VECTOR;
};