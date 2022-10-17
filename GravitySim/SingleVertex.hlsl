struct VS_Out {
	float4 color: COLOR;
	float4 pos : SV_POSITION;
};

cbuffer data :register(b0)
{
	matrix mat;
};

VS_Out main( float3 pos : POSITION )
{
	VS_Out output;

	output.pos = mul(float4(pos, 1.0f), mat);
	output.color = float4(1.0f * pos.x, 1.0f * pos.y, 0.5f, 1.0f);

	return output;
}