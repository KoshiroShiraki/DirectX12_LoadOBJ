#include"BasicShader.hlsli"

float4 main(Output input) : SV_TARGET
{
	float3 light = normalize(float3(1,-1,1));
	float brightness = dot(-light, input.normal);
	/*if (brightness < 0.6) brightness = 0.0f;
	else brightness = 1.0f;*/
	return float4(1.0f, 1.0f, 1.0f, 1.0f) * brightness;
}