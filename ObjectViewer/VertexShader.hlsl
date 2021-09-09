#include"BasicShader.hlsli"
Output main(
	float4 pos : POSITION,
	float4 normal : NORMAL,
	float2 uv : TEXCOORD,
	min16uint2 boneno : BONE_NO,
	min16uint weight : WEIGHT)
{
	Output output;
	output.svpos = mul(mul(mul(p, v), w), pos);
	normal.w = 0;
	output.normal = mul(w, normal);
	output.vnormal = mul(v, output.normal);
	output.ray = normalize(pos.xyz - eye);
	output.uv = uv;

	return output;
}