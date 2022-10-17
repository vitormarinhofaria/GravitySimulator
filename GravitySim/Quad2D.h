#pragma once
#include "pch.h"
#include "Renderer.h"
#include "ConstBuffer.h"

#define INSTANCE_COUNT 800

class Quad2D
{
	ID3D11Buffer* vertBuffer = nullptr;
	ID3D11VertexShader* vertexShader = nullptr;
	ID3D11PixelShader* pixelShader = nullptr;
	ID3D11InputLayout* inputLayout = nullptr;
	ID3D11Buffer* instanceIndexBuffer = nullptr;
	ID3D11Buffer* instanceDataBuffer = nullptr;

	ID3D11Buffer* matricesBuffer = nullptr;
	ID3D11Buffer* colorsBuffer = nullptr;

public:
	void Draw(Renderer& renderer);
	void SetInput(Renderer& r);
	Quad2D(Renderer& renderer);
	~Quad2D();	
	std::vector<DirectX::SimpleMath::Matrix> mMatrices;
	std::vector<DirectX::SimpleMath::Vector3> mColors;
	std::vector<InstanceData> instanceData;
};

