#pragma once
#include "pch.h"

struct Transform {
	DirectX::SimpleMath::Vector3 position;
	DirectX::SimpleMath::Vector3 rotation;
	DirectX::SimpleMath::Vector3 scale;

	DirectX::SimpleMath::Matrix GetMatrix() {
		using namespace DirectX::SimpleMath;
		return Matrix::Identity * 
			Matrix::CreateScale(scale) * 
			Matrix::CreateRotationX(rotation.x) * 
			Matrix::CreateRotationY(rotation.y) * 
			Matrix::CreateRotationZ(rotation.z) * 
			Matrix::CreateTranslation(position);
	}
};

namespace Utils {
	/// <summary>
	/// Read the entire file specified py fileName into a vector of 'char'
	/// </summary>
	/// <param name="fileName">The path of the file (full path or relative)</param>
	/// <returns></returns>
	std::vector<char> ReadFile(std::filesystem::path fileName);

	/// <summary>
	/// Generates a random number between 0.0 and 1.0
	/// </summary>
	/// <returns></returns>
	float RandNormalized();

	template<typename T>
	T RandomRange(T min, T max) {
		float r = (float)rand() / (float)RAND_MAX;
		return min + r * (max - min);
	}
}
