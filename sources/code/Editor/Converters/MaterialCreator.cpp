#include <fstream>
#include <iostream>
#include <vector>
#include <stdio.h>
#include <string.h>

#include "Editor/EditorManager.hpp"
#include "MaterialCreator.hpp"
using namespace Grindstone;

bool CreateStandardMaterial(StandardMaterialCreateInfo ci, std::string path) {
	std::ofstream output(path);
	output << "shader: ../assets/shaders/materials/standard_metalness/standard_metalness.json\n";
	if (ci.albedoPath != "") {
		output << "albedoTexture: " << ci.albedoPath << "\n";
	}
	output << "hasAlbedoTexture: " << (ci.albedoPath != "" ? "true" : "false") << "\n";

	if (ci.normalPath != "") {
		output << "normalTexture: " << ci.normalPath << "\n";
	}
	output << "hasNormalTexture: " << (ci.normalPath != "" ? "true" : "false") << "\n";

	if (ci.roughnessPath != "") {
		output << "roughnessTexture: " << ci.roughnessPath << "\n";
	}
	output << "hasRoughTexture: " << (ci.roughnessPath != "" ? "true" : "false") << "\n";

	if (ci.specularPath != "") {
		output << "metalnessTexture: " << ci.specularPath << "\n";
	}
	output << "hasMetalTexture: " << (ci.specularPath != "" ? "true" : "false") << "\n";

	output << "albedoConstant: " << ci.albedoColor[0] << " " << ci.albedoColor[1] << " " << ci.albedoColor[2] << " " << ci.albedoColor[3] << "\n";
	output << "metalnessConstant: " << ci.metalness << "\n";
	output << "roughnessConstant: " << ci.roughness << "\n";

	output.close();
	return false;
}

bool LoadMaterial(std::string path) {
	std::string mainPath = path + ".gmat";
	std::ifstream input(mainPath, std::ios::ate | std::ios::binary);

	if (!input.is_open()) {
		Editor::Manager::Print(LogSeverity::Error, "Failed to open file: %s!", mainPath.c_str());
		return false;
	}

	Editor::Manager::Print(LogSeverity::Info, "Material reading from: %s!", mainPath.c_str());

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

	Editor::Manager::Print(LogSeverity::Info, shader.c_str());
	Editor::Manager::Print(LogSeverity::Info, ci.albedoPath.c_str());
	Editor::Manager::Print(LogSeverity::Info, ci.normalPath.c_str());
	Editor::Manager::Print(LogSeverity::Info, ci.roughnessPath.c_str());
	Editor::Manager::Print(LogSeverity::Info, ci.specularPath.c_str());

	return true;
}
