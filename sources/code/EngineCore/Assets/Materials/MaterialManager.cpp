#include "MaterialManager.hpp"
#include "rapidjson/document.h"
#include "EngineCore/Assets/Shaders/ShaderManager.hpp"
#include "EngineCore/Utils/Utilities.hpp"
#include "EngineCore/EngineCore.hpp"
using namespace Grindstone;

Material& MaterialManager::LoadMaterial(const char* path) {
	Material* material = nullptr;
	if (TryGetMaterial(path, material)) {
		return *material;
	}

	return CreateMaterialFromFile(path);
}

bool MaterialManager::TryGetMaterial(const char* path, Material*& material) {
	auto& materialInMap = materials.find(path);
	if (materialInMap != materials.end()) {
		material = &materialInMap->second;
		return true;
	}

	return false;
}

Material MaterialManager::CreateMaterialFromData(std::filesystem::path relativePath, const char* data) {
	rapidjson::Document document;
	document.Parse(data);

	if (!document.HasMember("name")) {
		throw std::runtime_error("No name found in material.");
	}
	const char* name = document["name"].GetString();

	if (!document.HasMember("shader")) {
		throw std::runtime_error("No shader found in material.");
	}

	std::filesystem::path shaderPath = relativePath / document["shader"].GetString();
	std::string shaderPathStr = shaderPath.string();
	const char* shaderPathCStr = shaderPathStr.c_str();
	ShaderManager* shaderManager = EngineCore::GetInstance().shaderManager;
	Shader* shader = &shaderManager->LoadShader(shaderPathCStr);
	return {
		name,
		shaderPathCStr,
		shader
	};
}

Material& MaterialManager::CreateMaterialFromFile(const char* path) {
	if (!std::filesystem::exists(path)) {
		throw std::runtime_error(std::string(path) + " material doesn't exist.");
	}

	std::filesystem::path parentDirectory = std::filesystem::path(path).parent_path();
	std::string fileContent = Utils::LoadFileText(path);
	materials[path] = CreateMaterialFromData(parentDirectory, fileContent.c_str());

	return materials[path];
}
