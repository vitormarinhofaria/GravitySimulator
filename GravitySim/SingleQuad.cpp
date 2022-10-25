#include "pch.h"
#include "SingleQuad.h"
#include "Utils.h"

SingleQuad::SingleQuad()
{
	auto r = Renderer::Get();
	auto pixelShaderCso = Utils::ReadFile("cso/SinglePixel.cso");
	auto vertexShaderCso = Utils::ReadFile("cso/SingleVertex.cso");

	r->mDevice->CreatePixelShader(pixelShaderCso.data(), pixelShaderCso.size(), nullptr, &pixelShader);
	r->mDevice->CreateVertexShader(vertexShaderCso.data(), vertexShaderCso.size(), nullptr, &vertexShader);

	D3D11_INPUT_ELEMENT_DESC ied[] = {
		{"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0},
	};
	r->mDevice->CreateInputLayout(ied, ARRAYSIZE(ied), vertexShaderCso.data(), vertexShaderCso.size(), &inputLayout);

	float verts[] = {
	0.5, 0.5, 0.0f,
	0.5, -0.5, 0.0f,
	-0.5, -0.5, 0.0f,
	-0.5, -0.5, 0.0f,
	-0.5, 0.5, 0.0,
	0.5, 0.5, 0.0
	};

	D3D11_BUFFER_DESC vertBuffDesc{
		.ByteWidth = ARRAYSIZE(verts) * sizeof(verts),
		.Usage = D3D11_USAGE_DEFAULT,
		.BindFlags = D3D11_BIND_VERTEX_BUFFER,
	};
	D3D11_SUBRESOURCE_DATA vertSR{ .pSysMem = verts };
	r->mDevice->CreateBuffer(&vertBuffDesc, &vertSR, &vertBuffer);

	D3D11_BUFFER_DESC dataBufferDesc{
		.ByteWidth = sizeof(DataBuffer),
		.Usage = D3D11_USAGE_DYNAMIC,
		.BindFlags = D3D11_BIND_CONSTANT_BUFFER,
		.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE
	};
	D3D11_SUBRESOURCE_DATA dataSR{ .pSysMem = &this->data };
	r->mDevice->CreateBuffer(&dataBufferDesc, &dataSR, &dataBuffer);
}

SingleQuad::~SingleQuad()
{
}

void SingleQuad::Draw()
{
	auto r = Renderer::Get();
	r->mDContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	r->mDContext->IASetInputLayout(inputLayout);
	r->mDContext->VSSetShader(vertexShader, nullptr, 0);
	r->mDContext->PSSetShader(pixelShader, nullptr, 0);

	r->mDContext->VSSetConstantBuffers(0, 1, &dataBuffer);

	const uint32_t stride = sizeof(float) * 3;
	const uint32_t offset = 0;
	r->mDContext->IASetVertexBuffers(0, 1, &vertBuffer, &stride, &offset);

	r->mDContext->Draw(6, 0);
}

void SingleQuad::SetMatrix(dxm::Matrix mat)
{
	auto r = Renderer::Get();
	D3D11_MAPPED_SUBRESOURCE resData{};
	r->mDContext->Map(this->dataBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &resData);
	DataBuffer* b = (DataBuffer*)resData.pData;
	b->mat = mat;
	r->mDContext->Unmap(this->dataBuffer, 0);
}
