#pragma once

#include <string>

struct StandardMaterialCreateInfo {
	std::string albedoPath;
	std::string specularPath;
	std::string normalPath;
	std::string roughnessPath;
};

bool CreateStandardMaterial(StandardMaterialCreateInfo ci, std::string path);

bool LoadMaterial(std::string input);