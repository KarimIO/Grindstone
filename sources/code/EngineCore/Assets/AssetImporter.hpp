#pragma once

#include <string>
#include <Common/ResourcePipeline/Uuid.hpp>

namespace Grindstone {
	class AssetImporter {
	public:
		virtual void Load(Uuid& uuid) = 0;
		virtual void LazyLoad(Uuid& uuid) = 0;
	};
}
