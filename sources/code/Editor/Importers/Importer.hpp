#pragma once

#include <filesystem>

namespace Grindstone {
	class MetaFile;

	namespace Importers {
		class Importer {
		public:
			~Importer();
			virtual void Import(std::filesystem::path& path) = 0;
			MetaFile* metaFile = nullptr;
		};
	}
}
