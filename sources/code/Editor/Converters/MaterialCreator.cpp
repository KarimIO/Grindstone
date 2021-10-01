#include <fstream>
#include <iostream>
#include <vector>
#include <stdio.h>
#include <string.h>

#include "Editor/EditorManager.hpp"
#include "MaterialCreator.hpp"
using namespace Grindstone;
using namespace Grindstone::Converters;

bool CreateStandardOrCutoutMaterial(Materials::StandardMaterialCreateInfo createInfo, std::string path, bool isCutout) {
	std::ofstream output(path);

	output << "{\n";
	
	output << "\t\"name\": \"" << createInfo.materialName << "\",\n";

	if (isCutout) {
		output << "\t\"shader\": \"cutout\",\n";
	}
	else {
		output << "\t\"shader\": \"test\",\n";
	}

	{
		output << "\t\"parameters\": {\n";
		output << "\t\t\"color\": [1, 1, 1, 1]\n";
		output << "\t},\n";
	}
	
	{
		output << "\t\"samplers\": {";
		output << "\t\t\"albedoTexture\": \"" << createInfo.albedoPath << "\"\n,";
		output << "\t\t\"normalTexture\": \"" << createInfo.normalPath << "\"\n,";
		output << "\t\t\"metalnessTexture\": \"" << createInfo.specularPath << "\"\n,";
		output << "\t\t\"roughnessTexture\": \"" << createInfo.roughnessPath << "\"\n";
		output << "\t}\n";
	}

	output << "}";
	output.close();
	return false;
}

bool Materials::CreateStandardMaterial(StandardMaterialCreateInfo ci, std::string path) {
	return CreateStandardOrCutoutMaterial(ci, path, false);
}

bool Materials::CreateCutoutMaterial(StandardMaterialCreateInfo ci, std::string path) {
	return CreateStandardOrCutoutMaterial(ci, path, true);
}
