#pragma once

#include <filesystem>
#include <string>
#include <vector>
#include <map>

#include <assimp/scene.h>
#include <glm/glm.hpp>

#include <Common/Formats/Model.hpp>
#include <Common/ResourcePipeline/Uuid.hpp>
#include <EditorCommon/Editor/Importer.hpp>

namespace Grindstone::Editor::Importers {
	const Grindstone::Editor::ImporterVersion modelImporterVersion = 1;
	void ImportModel(Grindstone::Editor::AssetRegistry& assetRegistry, Grindstone::Assets::AssetManager& assetManager, const std::filesystem::path& path);
}
