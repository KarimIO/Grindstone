#pragma once

#include <string>
#include <vector>
#include <map>

#include "Mesh3d.hpp"

namespace Grindstone {
	class Mesh3dManager {
		public:
			Mesh3d& LoadMesh3d(const char* path);
			bool TryGetMesh3d(const char* path, Mesh3d*& mesh3d);
		private:
			Mesh3d& CreateMesh3dFromFile(const char* path);
		private:
			std::map<std::string, Mesh3d> meshes;
	};
}
