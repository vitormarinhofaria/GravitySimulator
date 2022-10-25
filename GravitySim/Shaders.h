#pragma once
#include "pch.h"
#include "Renderer.h"

enum class ShaderType {
	Compute,
	Vertex,
	Pixel
};

class ShaderManager;

class Shader {
public:
	const void Bind(Renderer& r);
	const ShaderType mType;
	
	/// <summary>
	/// Transfers the internal pointer ownership to the caller.
	/// </summary>
	void* ToOwned();

	Shader& operator=(const Shader& other) = delete;
	Shader(const Shader& other) = delete;

	Shader(Shader&& other) noexcept = delete;
	Shader& operator=(Shader&& other) noexcept;
	~Shader();
private:
	void* mShader;
	Shader(void* shader, ShaderType type);
	friend class ShaderManager;
};

class ShaderManager
{
public:
	/// <summary>
	/// 
	/// </summary>
	/// <param name="shader">The Shader path, relative or absulute if USE_FS is defined.</param>
	/// <param name="type"></param>
	/// <param name="defines"></param>
	/// <param name="outBlob">(optional) If set, will be used to as the blob for compiling the shader. Caller is the owner and should clean up the resource</param>
	/// <param name="entryPoint">(optional) The shader entry point function. Defaults to 'main'</param>
	/// <returns></returns>
	static Shader Load(std::filesystem::path shader, ShaderType type, D3D_SHADER_MACRO defines[] = nullptr, ID3D10Blob** outBlob = nullptr, const char* entryPoint = "main");
};

