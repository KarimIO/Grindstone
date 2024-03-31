#pragma once

#include <filesystem>

#include <Common/ResourcePipeline/Uuid.hpp>
#include <Editor/Importers/Importer.hpp>

namespace Grindstone {
	namespace Importers {
		class AudioImporter : public Importer {
		public:
			void Import(std::filesystem::path& path) override;
		};

		void ImportAudio(std::filesystem::path& inputPath);
	}
}
