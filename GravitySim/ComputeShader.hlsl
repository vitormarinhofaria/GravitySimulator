cbuffer data : register(b0) {
	float instanceCount;
	bool staticSun;
	float sunMass;
	float3 sunPosition;
	int p;
	int p2;
}

struct InstanceData {
	float4x4 mat;
	float speed;
	float3 direction;
	float3 color;
	float mass;
	float3 position;
};

StructuredBuffer<InstanceData> bufferInput : register(t0);
RWStructuredBuffer<InstanceData> bufferOut : register(u0);

[numthreads(32, 1, 1)]
void main(uint3 DTid : SV_DispatchThreadID)
{
	uint id = DTid.x;
	double G = 6.6742E-11;
	InstanceData q = bufferInput[id];

	if (id == 0) {
		q.mass = 1000000.0f * sunMass;
	}

	for (uint y = 0; y < instanceCount; y += 1) {

		if (y == id) {
			continue;
		}

		InstanceData q2;
		if (y > id) {
			q2 = bufferOut[y];
		}
		else {
			q2 = bufferInput[y];
		}
		float distance = length(q.position - q2.position);
		distance = distance * distance;
		double Force = G * (double(q.mass * q2.mass) / distance);
		//q.speed = Force / q.mass;
		float3 d = q2.position - q.position;
		float3 direction = d * float3(Force / q.mass, Force / q.mass, Force / q.mass);
		q.direction = q.direction + direction;
	}
	q.speed = length(q.direction);
		q.position = q.position + q.direction;
	/*if (id != 0 || !staticSun) {
	}
	else if (staticSun) {
		q.position = sunPosition;
	}*/

	bufferOut[id] = q;
}