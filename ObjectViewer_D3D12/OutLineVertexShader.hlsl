#include"BasicShader.hlsli"
Output main(
	float4 pos : POSITION,
	float4 normal : NORMAL,
	float2 uv : TEXCOORD)
{
	Output output;
	pos.xyz += normal.xyz * 0.2f;
	output.svpos = mul(mul(mul(p_perspective, v_camera), w), pos);

	return output;
}