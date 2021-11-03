#pragma once

#include <filesystem>
#include "Importer.hpp"
#include "Common/ResourcePipeline/Uuid.hpp"

namespace Grindstone {
	namespace Importers {
		class AudioImporter : public Importer {
		public:
			void Import(std::filesystem::path& path) override;
		};

		void ImportAudio(std::filesystem::path& inputPath);
	}
}
