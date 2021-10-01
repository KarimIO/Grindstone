#pragma once

#include <filesystem>
#include "Uuid.hpp"

namespace Grindstone {
	class MetaFile {
	public:
		~MetaFile();

		bool LoadOrCreateFromSourcePath(std::filesystem::path);
		std::filesystem::path GetCompiledFileFromUuid(Uuid uuid);
		void AddSubResource();
		void UpdateHash();
		void UpdateWriteDate();
	};
}
