#pragma once

#include <string>
#include "Common/ResourcePipeline/Uuid.hpp"

namespace Grindstone {
	struct Mesh3d;
	using MeshReference = Mesh3d*;

	struct AssetFile {
		Uuid uuid;
		std::string name;
	};
}
