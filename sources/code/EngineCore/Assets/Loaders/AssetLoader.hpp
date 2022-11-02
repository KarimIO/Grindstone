#pragma once

#include <Common/ResourcePipeline/Uuid.hpp>

namespace Grindstone {
	namespace Assets {
		class AssetLoader {
		public:
			virtual void Load(Uuid uuid, std::vector<char>& outContents) = 0;
		};
	}
}
