#pragma once

#include <string>

struct StandardMaterialCreateInfo {
	std::string albedoPath;
	std::string specularPath;
	std::string normalPath;
	std::string roughnessPath;
	float albedoColor[4];
	float metalness;
	float roughness;
};

bool CreateStandardMaterial(StandardMaterialCreateInfo ci, std::string path);

bool LoadMaterial(std::string input);