struct PS_INPUT
{
	float4 pos : SV_POSITION;
	float2 uv : TEXCOORD0;
};

float4 main(PS_INPUT input) : SV_TARGET
{
	return float4(input.uv.x, input.uv.x, input.uv.x, input.uv.x);
}