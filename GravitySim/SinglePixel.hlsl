struct VS_Out {
	float4 color: COLOR;
};

float4 main(VS_Out input) : SV_TARGET
{
	return input.color;
}