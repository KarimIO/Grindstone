#include "Mesh3dManager.hpp"
using namespace Grindstone;

Mesh3d& Mesh3dManager::LoadMesh3d(const char* path) {
	Mesh3d* mesh = nullptr;
	if (TryGetMesh3d(path, mesh)) {
		return *mesh;
	}

	return CreateMesh3dFromFile(path);
}

bool Mesh3dManager::TryGetMesh3d(const char* path, Mesh3d*& mesh) {
	auto& meshInMap = meshes.find(path);
	if (meshInMap != meshes.end()) {
		mesh = &meshInMap->second;
		return true;
	}

	return false;
}

Mesh3d& Mesh3dManager::CreateMesh3dFromFile(const char* path) {
	Mesh3d mesh;
	return mesh;
}
