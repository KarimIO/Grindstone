#pragma once

#include <filesystem>

#include <Common/ResourcePipeline/Uuid.hpp>
#include <Common/Editor/Importer.hpp>

namespace Grindstone::Editor::Importers {
	class AudioImporter : public Importer {
	public:
		void Import(const std::filesystem::path& path) override;
	};

	void ImportAudio(const std::filesystem::path& inputPath);
}
