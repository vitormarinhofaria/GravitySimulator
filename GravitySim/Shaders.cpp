#include "pch.h"
#include "Shaders.h"
#include "Utils.h"

const char* getTargetForType(ShaderType type) {
	switch (type)
	{
	case ShaderType::Compute:
		return "cs_5_0";
		break;
	case ShaderType::Vertex:
		return "vs_5_0";
		break;
	case ShaderType::Pixel:
		return "ps_5_0";
		break;
	default:
		break;
	}
	return "";
}

void* createShaderForType(ShaderType type, ID3D10Blob* blob) {
	auto* renderer = Renderer::Get();

	switch (type)
	{
	case ShaderType::Compute: {
		ID3D11ComputeShader* shader = nullptr;
		renderer->mDevice->CreateComputeShader(blob->GetBufferPointer(), blob->GetBufferSize(), nullptr, &shader);
		return shader;
		break;
	}
	case ShaderType::Vertex: {
		ID3D11VertexShader* shader = nullptr;
		renderer->mDevice->CreateVertexShader(blob->GetBufferPointer(), blob->GetBufferSize(), nullptr, &shader);
		return shader;
		break;
	}
	case ShaderType::Pixel: {
		ID3D11PixelShader* shader = nullptr;
		renderer->mDevice->CreatePixelShader(blob->GetBufferPointer(), blob->GetBufferSize(), nullptr, &shader);
		return shader;
		break;
	}
	default:
		break;
	}
	return nullptr;
}

Shader ShaderManager::Load(std::filesystem::path shader, ShaderType type, D3D_SHADER_MACRO defines[], ID3D10Blob** outBlob , const char* entryPoint)
{
	auto shaderInput = Utils::ReadFile(shader);
	ID3D10Blob* shaderBlob;
	D3DCompile(shaderInput.data(), shaderInput.size(), shader.string().c_str(), defines, D3D_COMPILE_STANDARD_FILE_INCLUDE, entryPoint, getTargetForType(type), D3DCOMPILE_OPTIMIZATION_LEVEL3, 0, &shaderBlob, nullptr);

	void* pShader = createShaderForType(type, shaderBlob);

	if (outBlob)
		*outBlob = shaderBlob;
	else
		shaderBlob->Release();


	return Shader(pShader, type);
}

const void Shader::Bind(Renderer& r)
{
	return void();
}

Shader& Shader::operator=(Shader&& other) noexcept
{
	auto s = Shader(other.mShader, other.mType);
	other.mShader = nullptr;
	return s;
}

Shader::~Shader()
{
	if (mShader) {
		IUnknown* u = (IUnknown*)mShader;
		u->Release();
	}
}

Shader::Shader(void* shader, ShaderType type) : mType(type), mShader(shader)
{
}

void* Shader::ToOwned() {
	void* temp = mShader;
	mShader = nullptr;
	return temp;
}
