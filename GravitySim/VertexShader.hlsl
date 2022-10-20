
struct VS_Output {
	float4 pos: SV_POSITION;
	float3 color : COLOR;
	float3 direction : DIRECTION;
	int instanceId : INSTANCE_ID;
};

struct VS_Input {
	float3 pos : POS;
	uint instanceId : INDEX;
	matrix mat : MATRIX;
	float speed : SPEED;
	float3 direction : DIRECTION;
	float3 color : COLOR;
	float mass : MASS;
	float3 position : POSITION;
};

VS_Output main(VS_Input input)
{
	float4 hotColor = float4(1.0f, 0.2, 0.2, 1.0f);
	float4 mediumColor = float4(0.2f, 1.0f, 0.2f, 1.0f);
	float4 coldColor = float4(0.2f, 0.2f, 1.0f, 1.0f);

	VS_Output output;
	//output.pos = mul(float4(input.pos, 1.0f), matrices[input.instanceId]);
	output.pos = mul(float4(input.pos, 1.0f) , input.mat);
	/*output.pos = output.pos * input.mat1;
	output.pos = output.pos * input.mat2;
	output.pos = output.pos * input.mat3;*/
	//output.pos = float4(input.pos.x, input.pos.y, 0.0f, 1.0f);
	output.instanceId = input.instanceId;
	//float interpolationFactor = clamp(input.speed * 0.1, 0.0f, 1.0f);
	output.color = lerp(coldColor, mediumColor, input.speed * 2);
	output.color = lerp(output.color, hotColor, input.speed);
	//output.color = input.color;
	//output.color = hotColor;
	//output.color = float3(input.instanceId, 0.0, 0.0);
	output.direction = output.color;
	
	if (input.instanceId == 0) {
		output.color = float4(1.0f, 1.0f, 1.0f, 1.0f);
		//output.pos = float4(output.pos.xyz, -2);
	}

	return output;
}