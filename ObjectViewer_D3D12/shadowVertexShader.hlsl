cbuffer cbuff0 : register(b0)
{
	matrix w;
	matrix v_light;
	matrix p;
};

struct Output {
	float4 svpos : SV_POSITION;
};

Output main(
	float4 pos : POSITION,
	float4 normal : NORMAL,
	float2 uv : TEXCOORD)
{
	Output output;
	output.svpos = mul(mul(mul(p, v_light), w), pos);

	return output;
}