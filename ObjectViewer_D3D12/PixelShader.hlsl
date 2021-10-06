#include"BasicShader.hlsli"

float4 main(Output input) : SV_TARGET
{
	float3 light = normalize(float3(0,-1,1));


	return float4(diffuse, 1.0f);
}