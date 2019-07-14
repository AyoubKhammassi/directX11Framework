struct VSOut
{
	float4 color: COLOR;
	float4 pos: SV_POSITION;
};

cbuffer cbuf
{
	matrix transform;
}


VSOut	main(float4 pos : POSITION, float3 color : COLOR)
{
	VSOut o;
	o.pos = mul(float4(pos.x, pos.y, 0.0f, 1.0f), transform);
	o.color = float4(color, 1.0f);
	return o;
}