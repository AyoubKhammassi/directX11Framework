/*struct VSOut
{
	float4 color: COLOR;
	float4 pos: SV_POSITION;
};*/

cbuffer cbuf
{
	//we can declate matrices as row major because that's how are handled in the CPU, but matrices calculations are column major on the GPU so it's better to transpose the
	//matrices before sending it to the shader           KEY WORD: row_major
	matrix transform;
}


float4	main(float3 pos : POSITION) : SV_Position
{
	return mul(float4(pos, 1.0f), transform);

}