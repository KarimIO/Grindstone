#pragma once

#include <string>
#include <Common/ResourcePipeline/Uuid.hpp>

namespace Grindstone {
	class AssetImporter {
	public:
		virtual void* ProcessLoadedFile(Uuid uuid) = 0;
		virtual bool TryGetIfLoaded(Uuid uuid, void*& output) = 0;
		virtual void* ProcessLoadedFile(const char* path) { return nullptr; };
		virtual bool TryGetIfLoaded(const char* path, void*& output) { return false; };
	};
}
