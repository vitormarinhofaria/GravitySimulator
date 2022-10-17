struct VS_Output {
	float4 pos: SV_POSITION;
	float3 color : COLOR;
	float3 direction : DIRECTION;
	int instanceId : INSTANCE_ID;
};

float4 main(VS_Output input) : SV_TARGET
{
	return float4(input.color, 1.0f);
}