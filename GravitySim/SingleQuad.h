#pragma once
#include "pch.h"
#include "Renderer.h"

#define dxm DirectX::SimpleMath
struct DataBuffer {
	dxm::Matrix mat = dxm::Matrix::Identity;
};

class SingleQuad
{
	ID3D11Buffer* vertBuffer = nullptr;
	ID3D11VertexShader* vertexShader = nullptr;
	ID3D11PixelShader* pixelShader = nullptr;
	ID3D11InputLayout* inputLayout = nullptr;

	ID3D11Buffer* dataBuffer = nullptr;
	DataBuffer data{};
public:
	SingleQuad();
	~SingleQuad();
	void Draw();
	void SetMatrix(dxm::Matrix mat);
};

