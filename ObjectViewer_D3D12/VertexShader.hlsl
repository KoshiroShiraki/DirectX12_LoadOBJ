#include"BasicShader.hlsli"
Output main(
	float4 pos : POSITION,
	float4 normal : NORMAL,
	float2 uv : TEXCOORD)
{
	Output output;
	output.svpos = mul(mul(mul(p_perspective, v_camera), w), pos);
	normal.w = 0;
	output.normal = normalize(mul(w, normal));
	output.vnormal = mul(v_camera, output.normal);
	output.uv = uv;
	output.ray = normalize(pos.xyz - eye);
	output.spos = mul(mul(mul(p_orthographic, v_light), w), pos);
	return output;
}