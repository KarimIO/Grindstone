#pragma once

#include <string>
#include <Common/ResourcePipeline/Uuid.hpp>

namespace Grindstone {
	class AssetImporter {
	public:
		virtual void LoadFile(Uuid uuid) = 0;
	};
}
