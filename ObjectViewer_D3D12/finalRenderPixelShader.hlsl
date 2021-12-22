#include"finalRenderShader.hlsli"

float4 main(Output input) : SV_TARGET
{
	float w, h, levels;
	tex.GetDimensions(0, w, h, levels);
	if (sqrt(pow(input.svpos.x - w / 2, 2) + pow(input.svpos.y - h / 2, 2)) > 500) return tex.Sample(smp, input.uv) * 0.3;
	return tex.Sample(smp, input.uv);
}