#include "MaterialCreator.h"
#include <fstream>
#include <iostream>
#include <vector>
#include <stdio.h>
#include <string.h>

bool CreateMaterialJsonFile(StandardMaterialCreateInfo ci, std::string path) {
	std::ofstream output(path);
	output << "{\n\t\"shader\": \"../shaders/standard\",\n";
	output << "\t\"albedoTexture\": \"" << ci.albedoPath << "\",\n";
	output << "\t\"normalTexture\": \"" << ci.normalPath << "\",\n";
	output << "\t\"roughnessTexture\": \"" << ci.roughnessPath << "\",\n";
	output << "\t\"specularTexture\": \"" << ci.specularPath << "\"\n}";
	output.close();
	return false;
}

bool CreateMaterialBinaryFile(StandardMaterialCreateInfo ci, std::string path) {
	std::ofstream output(path, std::ios::binary);
	output << "../shaders/standard" << '\0';
	output << ci.albedoPath << '\0';
	output << ci.normalPath << '\0';
	output << ci.roughnessPath << '\0';
	output << ci.specularPath << '\0';
	output.close();
	return false;
}

bool LoadMaterial(std::string path) {
	std::ifstream input(path + ".gbm", std::ios::ate | std::ios::binary);

	if (!input.is_open()) {
		input.open(path + ".gjm", std::ios::ate | std::ios::binary);

		if (!input.is_open()) {
			std::cerr << "Failed to open file: " << path.c_str() << "!\n";
			return false;
		}
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
