Texture2D tex : register(t0);
SamplerState texSampler : register(s0);

struct PS_INPUT
{
	float4 pos : SV_POSITION;
	float2 uv : TEXCOORD0;
};

float4 main(PS_INPUT input) : SV_TARGET
{
	return tex.Sample(texSampler, input.uv);
}