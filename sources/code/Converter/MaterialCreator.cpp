#include "MaterialCreator.hpp"
#include <fstream>
#include <iostream>
#include <vector>
#include <stdio.h>
#include <string.h>

bool CreateStandardMaterial(StandardMaterialCreateInfo ci, std::string path) {
	std::ofstream output(path);
	output << "shader: ../shaders/standard.json\n";
	output << "albedoTexture: " << ci.albedoPath << "\n";
	output << "normalTexture: " << ci.normalPath << "\n";
	output << "roughnessTexture: " << ci.roughnessPath << "\n";
	output << "metalnessTexture: " << ci.specularPath;
	output.close();
	return false;
}

bool LoadMaterial(std::string path) {
	std::ifstream input(path + ".gmat", std::ios::ate | std::ios::binary);

	if (!input.is_open()) {
		std::cerr << "Failed to open file: " << path.c_str() << ".gjm!\n";
		return false;
	}

	std::cout << "Material reading from: " << path << "!\n";

	size_t fileSize = (size_t)input.tellg();
	std::vector<char> buffer(fileSize);

	input.seekg(0);
	input.read(buffer.data(), fileSize);

	StandardMaterialCreateInfo ci;

	// Extend shader info here
	char *words = buffer.data();
	std::string shader = words;
	words = strchr(words, '\0') + 1;
	ci.albedoPath = words;
	words = strchr(words, '\0') + 1;
	ci.normalPath = words;
	words = strchr(words, '\0') + 1;
	ci.roughnessPath = words;
	words = strchr(words, '\0') + 1;
	ci.specularPath = words;
	std::cout << shader << "\n";
	std::cout << ci.albedoPath << "\n";
	std::cout << ci.normalPath << "\n";
	std::cout << ci.roughnessPath << "\n";
	std::cout << ci.specularPath << "\n";

	std::cout << "Material loaded!\n";

	return true;
}
