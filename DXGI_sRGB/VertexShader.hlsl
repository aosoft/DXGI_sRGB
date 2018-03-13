struct PS_INPUT
{
	float4 pos : SV_POSITION;
	float2 uv : TEXCOORD0;
};

PS_INPUT main(float4 pos : POSITION)
{
	PS_INPUT ret;

	ret.pos = float4(pos.xy, 0.0, 1.0);
	ret.uv = float2(pos.zw);

	return ret;
}