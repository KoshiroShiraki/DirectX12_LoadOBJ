#include"BasicShader.hlsli"

float4 main(Output input) : SV_TARGET
{
	float3 light = normalize(float3(1,-1,1));
	float ka = 0.1f;
	float kb = 1.0f;
	float kc = 1.0f;

	float3 refLight = normalize(reflect(light, input.normal.xyz));
	float speVal = pow(saturate(dot(refLight, -input.ray)), Nspecular);

	float4 f_ambient = (ambient + float4(ambtex.Sample(smp, input.uv))) * ka;
	float4 f_diffuse = (diffuse + float4(diftex.Sample(smp, input.uv))) * (-dot(light, input.normal)) * kb;
	float4 f_specular = (specular + float4(spetex.Sample(smp, input.uv))) * Nspecular * speVal * kc;

	return f_ambient + f_diffuse/* + f_specular*/;
}