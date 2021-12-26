#include"finalRenderShader.hlsli"

float4 main(Output input) : SV_TARGET
{
	float w, h, levels;
	tex.GetDimensions(0, w, h, levels);
	return tex.Sample(smp, input.uv);
}