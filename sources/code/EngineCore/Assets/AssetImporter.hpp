#pragma once

#include <string>
#include <Common/ResourcePipeline/Uuid.hpp>

namespace Grindstone {
	class AssetImporter {
	public:
		virtual void* ProcessLoadedFile(Uuid uuid, char* fileContents, size_t fileSize) = 0;
		virtual bool TryGetIfLoaded(Uuid uuid, void*& output) = 0;
	};
}
