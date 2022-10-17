#include "pch.h"
#include "Utils.h"

namespace Utils {
	std::vector<char> ReadFile(std::filesystem::path fileName) {
		std::vector<char> vec;
		std::ifstream inputFile = std::ifstream(fileName, std::ios::binary);

		if (inputFile.is_open()) {
			auto fileSize = std::filesystem::file_size(fileName);
			vec.resize(fileSize);
			inputFile.read(vec.data(), fileSize);
		}

		return vec;
	}


	float RandNormalized() {
		return ((double)rand() / RAND_MAX);
	}
}