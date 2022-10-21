#pragma once

#include "pch.h"

#define BASE_MASS 10000.0f

struct InstanceData{
	DirectX::SimpleMath::Matrix matrix;
	float speed;
	float direction[3];
	float color[3];
	float mass = BASE_MASS * 12.0f;
	float position[3] = {0.0f, 0.0f, 0.0f};
};

struct AlignedInt {
	float instanceCount;
	bool staticSun;
	float sunMass;
	DirectX::SimpleMath::Vector3 sunPosition = { 0.0f,0.0f,0.0f };

	int padding[2];
};

static ID3D11ComputeShader* ComputeShader = nullptr;
static ID3D11Buffer* ComputeInputBuffer = nullptr;
static ID3D11Buffer* ComputeOutputBuffer = nullptr;
//static ID3D11Buffer* ComputeResultBuffer = nullptr;
static ID3D11ShaderResourceView* ComputeInputSRV = nullptr;
static ID3D11UnorderedAccessView* ComputeView = nullptr;
static ID3D11Buffer* ComputeConstBuffer = nullptr;
class Quad2D;
class Renderer;
static int m_ComputeUnitCount = 36;

void SetComputeShaderUnitsCount(int count);
void PrepareComputeShader(Renderer& renderer, Quad2D& quad);
void DispatchComputeShader(Renderer& r, Quad2D& quad, AlignedInt data);
void FreeComputeShader();
void UpdateShaderInput(Renderer& r, Quad2D& quad);
void ComputeShaderEndFrame(Renderer& r, Quad2D& quad);