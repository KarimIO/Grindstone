#pragma once

#include <filesystem>

#include <Common/ResourcePipeline/Uuid.hpp>
#include <Common/Editor/Importer.hpp>

namespace Grindstone::Editor::Importers {
	const Grindstone::Editor::ImporterVersion audioImporterVersion = 1;
	void ImportAudio(AssetRegistry& assetRegistry, Assets::AssetManager& assetManger, const std::filesystem::path& inputPath);
}
