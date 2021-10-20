#include"BasicShader.hlsli"

float4 main(Output input) : SV_TARGET
{
	float3 light = normalize(float3(1.0,-1.0f,-1.0f));
	float ka = 0.1f;
	float kb = 1.0f;
	float kc = 1.0f;

	float4 f_ambient = (ambient + float4(ambtex.Sample(smp, input.uv))) * ka;
	float4 f_diffuse = (diffuse + float4(diftex.Sample(smp, input.uv))) * (-dot(light, input.normal)) * kb;

	return f_ambient + f_diffuse;
}