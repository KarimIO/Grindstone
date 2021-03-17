#pragma once

#include "StaticMesh.hpp"

namespace Grindstone {
	class StaticMeshManager {
	public:
		void initialize();
		
		void loadMesh(const char *path);
		void loadMeshImmediate(const char *path);
		void reloadMesh(const char *path);
		void unloadMesh(const char *path);
	private:
		std::vector<StaticMesh> staticMeshes;
		std::map<std::string, uint32_t> staticMeshMap;
	};
}
