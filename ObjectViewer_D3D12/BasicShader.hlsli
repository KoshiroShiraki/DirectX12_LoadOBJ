cbuffer cbuff0 : register(b0)
{
	matrix w;
	matrix v_light;
	matrix v_camera;
	matrix p_perspective;
	matrix p_orthographic;
	float3 eye;
	float3 light_col;
	float3 light_pos;
};

cbuffer Material : register(b1)
{
	float4 ambient;
	float4 diffuse;
	float4 specular;
	float Nspecular;
};

Texture2D<float> shadowTex : register(t0);

SamplerState smp: register(s0);

struct Output {
	float4 svpos : SV_POSITION;
	float4 normal : NORMAL0;
	float4 vnormal : NORMAL1;
	float2 uv : TEXCOORD;
	float3 ray : VECTOR;
	float4 spos : SPOS;
};