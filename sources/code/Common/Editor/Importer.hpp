#pragma once

#include <filesystem>

namespace Grindstone::Editor {
	class MetaFile;

	namespace Importers{
		class Importer {
		public:
			virtual void Import(const std::filesystem::path& path) = 0;
			~Importer();

		protected:
			MetaFile* metaFile = nullptr;
		};
	}
}
