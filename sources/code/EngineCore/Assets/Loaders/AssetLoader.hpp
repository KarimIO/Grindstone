#pragma once

#include <Common/ResourcePipeline/Uuid.hpp>

namespace Grindstone {
	namespace Assets {
		class AssetLoader {
		public:
			virtual void Load(Uuid uuid, char*& outContents, size_t& fileSize) = 0;
			virtual bool LoadText(Uuid uuid, std::string& outContents) = 0;
		};
	}
}
