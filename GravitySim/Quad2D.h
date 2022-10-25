#pragma once
#include "pch.h"
#include "Renderer.h"
#include "ConstBuffer.h"

#ifdef NDEBUG
#define INSTANCE_COUNT_DEF 5000
#else
#define INSTANCE_COUNT_DEF 5000
#endif

class Quad2D
{
	ID3D11Buffer* vertBuffer = nullptr;
	ID3D11VertexShader* vertexShader = nullptr;
	ID3D11PixelShader* pixelShader = nullptr;
	ID3D11InputLayout* inputLayout = nullptr;
	ID3D11Buffer* instanceIndexBuffer = nullptr;
	ID3D11Buffer* instanceDataBuffer = nullptr;

	uint32_t mVerticesCount = 0;
	uint32_t mInstancesCount = 0;

public:
	void Draw();
	void SetInput();
	Quad2D(uint32_t count, bool randomMass = false, bool randomDirection = false, float directionFactor = 0.2f, float spacingFactorX = 8.0f, float spacingFactorY = 8.0f);
	~Quad2D();	
	//std::vector<DirectX::SimpleMath::Matrix> mMatrices;
	//std::vector<DirectX::SimpleMath::Vector3> mColors;
	std::vector<InstanceData> instanceData;

	Quad2D& operator=(const Quad2D& other) = delete;
	Quad2D(const Quad2D& other) = delete;
	
	Quad2D(Quad2D&& other) noexcept = delete;
	Quad2D& operator=(Quad2D&& other) noexcept;

};

