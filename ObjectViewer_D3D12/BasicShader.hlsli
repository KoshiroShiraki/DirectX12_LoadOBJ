//定数バッファ
cbuffer cbuff0 : register(b0)
{
	matrix w;
	matrix v;
	matrix p;
	float3 eye;
};

struct Output {
	float4 svpos : SV_POSITION;
	float4 normal : NORMAL0;
	float4 vnormal : NORMAL1;
	float2 uv : TEXCOORD;
};