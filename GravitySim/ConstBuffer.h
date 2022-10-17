#pragma once

#include "pch.h"

struct InstanceData{
	DirectX::SimpleMath::Matrix matrix;
	float speed;
	float direction[3];
	float color[3];
	float mass = 10000.0f;
	float position[3] = {0.0f, 0.0f, 0.0f};
};