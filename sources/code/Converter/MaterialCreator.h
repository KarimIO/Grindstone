#pragma once

#include <string>

struct StandardMaterialCreateInfo {
	std::string albedoPath;
	std::string specularPath;
	std::string normalPath;
	std::string roughnessPath;
};

bool CreateMaterialJsonFile(StandardMaterialCreateInfo ci, std::string path);
bool CreateMaterialBinaryFile(StandardMaterialCreateInfo ci, std::string path);
bool ConvertMaterialFileJsonToBinary(std::string input);

bool LoadMaterial(std::string input);