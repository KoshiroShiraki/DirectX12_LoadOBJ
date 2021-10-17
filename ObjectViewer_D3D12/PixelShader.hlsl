#include"BasicShader.hlsli"

float4 main(Output input) : SV_TARGET
{
	float3 light = normalize(float3(1.0f,-1.0f,1.0f));
	float ka = 1.0f;
	float kb = 1.0f;
	float kc = 1.0f;

	float4 f_ambient = ambient * ka;
	float4 f_diffuse = diffuse * (dot(light, input.normal)) * kb;
	return f_ambient + f_diffuse;
}