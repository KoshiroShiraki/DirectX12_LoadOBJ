#include"BasicShader.hlsli"

float4 main(Output input) : SV_TARGET
{
	float3 light = normalize(float3(0,-1,1));
	float f_ambient = dot(input.normal, float4(light,1.0f));

	return float4(ambient, 1.0f) + float4((diffuse)*f_ambient,1.0f);
}