#include"BasicShader.hlsli"

float4 main(Output input) : SV_TARGET
{
	float3 light = normalize(float3(1,-1,1));

	float3 refLight = normalize(reflect(light, input.normal.xyz));
	float speVal = pow((dot(refLight, -input.ray)), Nspecular);

	float4 f_ambient = (ambient);
	float4 f_diffuse = (diffuse) * (-dot(light, input.normal));
	float4 f_specular = (specular) * speVal;
	float3 lpos = input.spos.xyz;
	float2 comparePos = (lpos + float2(1, -1)) * float2(0.5, -0.5);
	float4 lightColor = float4(light_col, 0);

	if (shadowTex.Sample(smp, comparePos) < lpos.z - 0.001f) {
		return float4(f_ambient + f_diffuse + lightColor) * 0.5;
	}
	return f_ambient + f_diffuse + lightColor;// +f_specular;
}