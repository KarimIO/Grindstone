#pragma once

#include <string>
#include <Common/ResourcePipeline/Uuid.hpp>
#include <EditorCommon/Editor/Importer.hpp>

namespace Grindstone::Editor::Importers {
	const Grindstone::Editor::ImporterVersion textureImporterVersion = 1;
	void ImportTexture(Grindstone::Editor::AssetRegistry& assetRegistry, Grindstone::Assets::AssetManager& assetManger, const std::filesystem::path& inputPath);
}
