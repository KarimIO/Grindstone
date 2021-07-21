#include "MaterialManager.hpp"
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

Material& MaterialManager::CreateMaterialFromFile(const char* path) {
	Material m;
	return m;
}
