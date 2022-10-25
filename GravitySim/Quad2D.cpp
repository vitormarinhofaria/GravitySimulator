#include "pch.h"
#include "Quad2D.h"
#include "Utils.h"


void Quad2D::Draw()
{
	auto r = Renderer::Get();

	r->mDContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	r->mDContext->IASetInputLayout(inputLayout);

	const uint32_t stride = 3 * sizeof(float);
	const uint32_t offset = 0;
	r->mDContext->IASetVertexBuffers(0, 1, &vertBuffer, &stride, &offset);
	const uint32_t instStride = sizeof(uint32_t);
	const uint32_t instOffset = 0;
	r->mDContext->IASetVertexBuffers(1, 1, &instanceIndexBuffer, &instStride, &instOffset);
	const uint32_t dataStride = sizeof(InstanceData);
	const uint32_t dataOffset = 0;
	r->mDContext->IASetVertexBuffers(2, 1, &instanceDataBuffer, &dataStride, &dataOffset);

	r->mDContext->VSSetShader(vertexShader, nullptr, 0);
	r->mDContext->PSSetShader(pixelShader, nullptr, 0);

	//renderer.mDContext->DrawInstanced
	r->mDContext->DrawInstanced(mVerticesCount, mInstancesCount, 0, 0);
	//r.mDContext->Draw(3, 0);
}

void Quad2D::SetInput()
{
	auto r = Renderer::Get();

	D3D11_MAPPED_SUBRESOURCE res{};

	r->mDContext->Map(instanceDataBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &res);
	std::memcpy(res.pData, instanceData.data(), instanceData.size() * sizeof(InstanceData));
	r->mDContext->Unmap(instanceDataBuffer, 0);
}

Quad2D::Quad2D(uint32_t instCount, bool randomMass, bool randomDirection, float directionFactor, float spacingFactorX, float spacingFactorY) : mInstancesCount(instCount)
{
	auto renderer = Renderer::Get();
	std::vector<char> pixelCso = Utils::ReadFile("cso/PixelShader.cso");
	renderer->mDevice->CreatePixelShader(pixelCso.data(), pixelCso.size(), nullptr, &pixelShader);

	auto vertexCso = Utils::ReadFile("cso/VertexShader.cso");
	renderer->mDevice->CreateVertexShader(vertexCso.data(), vertexCso.size(), nullptr, &vertexShader);

	D3D11_INPUT_ELEMENT_DESC ied[] = {
		{"POS", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0},
		{"INDEX", 0, DXGI_FORMAT_R32_UINT, 1, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_INSTANCE_DATA, 1},
		{"MATRIX", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 2, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_INSTANCE_DATA, 1},
		{"MATRIX", 1, DXGI_FORMAT_R32G32B32A32_FLOAT, 2, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_INSTANCE_DATA, 1},
		{"MATRIX", 2, DXGI_FORMAT_R32G32B32A32_FLOAT, 2, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_INSTANCE_DATA, 1},
		{"MATRIX", 3, DXGI_FORMAT_R32G32B32A32_FLOAT, 2, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_INSTANCE_DATA, 1},
		{"SPEED", 0, DXGI_FORMAT_R32_FLOAT, 2, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_INSTANCE_DATA , 1},
		{"DIRECTION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 2, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_INSTANCE_DATA , 1},
		{"COLOR", 0, DXGI_FORMAT_R32G32B32_FLOAT, 2, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_INSTANCE_DATA , 1},
		{"MASS", 0, DXGI_FORMAT_R32_FLOAT, 2, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_INSTANCE_DATA , 1},
		{"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 2, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_INSTANCE_DATA , 1},
	};
	renderer->mDevice->CreateInputLayout(ied, ARRAYSIZE(ied), vertexCso.data(), vertexCso.size(), &inputLayout);

	std::vector<uint32_t> indices{};
	float aspect = 1280 / 720;
	using namespace DirectX;
	using namespace DirectX::SimpleMath;

	//Matrix projection = Matrix::CreatePerspectiveFieldOfView(80, aspect, 0.01f, 1000.0f);
	//Matrix view = Matrix::CreateLookAt({ 0.0, 0.0, -2.0f }, Vector3{ 0.0, 0.0, -2.0f }  + Vector3{ 0.0f, 0.0f, -1.0f }, {0.0f, -1.0f, 0.0f });
	Matrix projection = Matrix::CreateOrthographic(128.0, 72.0, 0.01f, 1000.0f);
	Matrix view = Matrix::Identity * Matrix::CreateTranslation({ 0.0f, 0.0f, -1.0f });
	instanceData.reserve(mInstancesCount);
	for (auto i = 0; i < mInstancesCount; i++) {
		SimpleMath::Matrix mat = Matrix::Identity;
		mat *= Matrix::CreateScale(0.5f, 0.5f, 0.5f);
		float factor = 8.0f;// +(mInstancesCount * 0.001f);
		float f2 = -0.5f;
		//srand(SDL_GetTicks64());

		Vector3 position;
		if (i == 0) {
			position = Vector3(20.0f, 8.0f, 0.0f);
		}
		else {
			position = Vector3{ Utils::RandomRange(-spacingFactorX, spacingFactorY) , Utils::RandomRange(-spacingFactorX, spacingFactorY), .0f };
		}
		//Vector3 position = Vector3{ float((double)rand() / RAND_MAX) +(float)1 , float((double)rand() / RAND_MAX) + 1, 0.0f };
		mat *= Matrix::CreateTranslation(position);
		Matrix m = mat * view * projection;
		//mat *= Matrix::CreateTranslation(t);

		InstanceData ins{};
		ins.matrix = m.Transpose();
		if (i % 2 == 0) {
			ins.color[0] = Utils::RandomRange(0.7f, 1.f);
			ins.color[1] = Utils::RandomRange(0.25f, 0.4f);
			ins.color[2] = Utils::RandomRange(0.25f, 0.4f);
		}
		else if (i % 3 == 0) {
			ins.color[0] = Utils::RandomRange(0.25f, 0.4f);
			ins.color[1] = Utils::RandomRange(0.7f, 1.0f);
			ins.color[2] = Utils::RandomRange(0.25f, 0.5f);
		}
		else {
			ins.color[0] = Utils::RandomRange(0.25f, 0.4f);
			ins.color[1] = Utils::RandomRange(0.25f, 0.4f);
			ins.color[2] = Utils::RandomRange(0.7f, 1.0f);
		}

		if (randomDirection) {
			ins.direction[0] = Utils::RandomRange(-directionFactor, directionFactor);
			ins.direction[1] = Utils::RandomRange(-directionFactor, directionFactor);
			ins.direction[2] = Utils::RandomRange(-directionFactor, directionFactor);
		}

		//ins.direction[0] = 0.015f;
		//ins.direction[1] = 0.15f;
		//ins.direction[2] = 0.015f;

		//ins.speed = Utils::RandomRange(0.0f, 2.f);
		//ins.speed = 0.0f;
		ins.position[0] = position.x;
		ins.position[1] = position.y;
		ins.position[2] = position.z;
		if (randomMass) {
			float randomChance = Utils::RandomRange(0.0f, 100.0f);

			if (randomChance > 90.0f) {
				float massFactor = Utils::RandomRange(50.0f, 150.0f);
				//ins.mass = Utils::RandomRange(100000.0f , 10000000.0f);
				ins.mass = BASE_MASS * massFactor;
			}
			else {
				float massFactor = Utils::RandomRange(12.0f, 20.0f);
				//ins.mass = Utils::RandomRange(100000.0f , 10000000.0f);
				ins.mass = BASE_MASS * massFactor;
			}
		}

		instanceData.push_back(ins);
		indices.push_back(i);
	}

	/*float quad[] = {
	0.5, 0.5, 0.0f,
	0.5, -0.5, 0.0f,
	-0.5, -0.5, 0.0f,
	-0.5, -0.5, 0.0f,
	-0.5, 0.5, 0.0,
	0.5, 0.5, 0.0
	};*/
	/*float quad[] = {
		0.0f, 0.5, 0.0f,
		0.5f, 0.0f, 0.0f,
		-0.5f, 0.0f, 0.0f,
		-0.5f, 0.0f, 0.0f,
		0.5f, 0.0f, 0.0f,
		0.0f, -0.5f, 0.0f,
	};*/
	float point = 0.35f;
	float quad[] = {
		0.0f, 0.0f, 0.0f,
		0.0f, 0.5, 0.0f,//
		point, point, 0.0f,

		0.0f, 0.0f, 0.0f,
		point, point, 0.0f,
		0.5f, 0.0f, 0.0f,//

		0.0f, 0.0f, 0.0f,
		0.5f, 0.0f, 0.0f,//
		point, -point, 0.0f,

		0.0f, 0.0f, 0.0f,
		point, -point, 0.0f,
		0.0f, -0.5f, 0.0f,//

		0.0f, 0.0f, 0.0f,
		0.0f, -0.5f, 0.0f,//
		-point, -point, 0.0f,

		0.0f, 0.0f, 0.0f,
		-point, -point, 0.0f,
		-0.5f, 0.0f, 0.0f,//

		0.0f, 0.0f, 0.0f,
		-0.5f, 0.0f, 0.0f,//
		-point, point, 0.0f,

		0.0f, 0.0f, 0.0f,
		-point, point, 0.0f,
		0.0f, 0.5f, 0.0f,//
	};

	mVerticesCount = ARRAYSIZE(quad) / 3;

	D3D11_BUFFER_DESC buffer{};
	buffer.ByteWidth = ARRAYSIZE(quad) * sizeof(float);
	buffer.Usage = D3D11_USAGE_IMMUTABLE;
	buffer.BindFlags = D3D11_BIND_VERTEX_BUFFER;

	D3D11_SUBRESOURCE_DATA buffData{};
	buffData.pSysMem = quad;
	renderer->mDevice->CreateBuffer(&buffer, &buffData, &vertBuffer);

	D3D11_BUFFER_DESC instBufferDesc{ .ByteWidth = (UINT)indices.size() * sizeof(uint32_t), .Usage = D3D11_USAGE_IMMUTABLE, .BindFlags = D3D11_BIND_VERTEX_BUFFER };
	D3D11_SUBRESOURCE_DATA instBufferData{ .pSysMem = indices.data() };
	renderer->mDevice->CreateBuffer(&instBufferDesc, &instBufferData, &instanceIndexBuffer);

	D3D11_BUFFER_DESC instDataDesc{ .ByteWidth = (UINT)instanceData.size() * sizeof(InstanceData), .Usage = D3D11_USAGE_DYNAMIC, .BindFlags = D3D11_BIND_VERTEX_BUFFER, .CPUAccessFlags = D3D11_CPU_ACCESS_WRITE };
	D3D11_SUBRESOURCE_DATA instDatSR{ .pSysMem = instanceData.data() };
	renderer->mDevice->CreateBuffer(&instDataDesc, &instDatSR, &instanceDataBuffer);
}

Quad2D::~Quad2D()
{
	if (vertBuffer) {
		pixelShader->Release();
		vertexShader->Release();
		vertBuffer->Release();
	}
}

Quad2D& Quad2D::operator=(Quad2D&& other) noexcept
{
	this->inputLayout = other.inputLayout;
	other.inputLayout = nullptr;

	this->instanceData = other.instanceData;

	this->instanceDataBuffer = other.instanceDataBuffer;
	other.instanceDataBuffer = nullptr;

	this->instanceIndexBuffer = other.instanceIndexBuffer;
	other.instanceIndexBuffer = nullptr;

	this->mInstancesCount = other.mInstancesCount;

	this->mVerticesCount = other.mVerticesCount;

	this->pixelShader = other.pixelShader;
	other.pixelShader = nullptr;

	this->vertexShader = other.vertexShader;
	other.vertexShader = nullptr;

	this->vertBuffer = other.vertBuffer;
	other.vertBuffer = nullptr;

	return *this;
}
