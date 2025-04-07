#pragma once

#include <string>
#include <vector>

#include <rapidjson/document.h>
#include <rapidjson/prettywriter.h>

#include <Common/ResourcePipeline/Uuid.hpp>
#include <Common/Editor/Importer.hpp>

namespace Grindstone::Editor::Importers {
	void ImportShadersFromGlsl(Grindstone::Editor::AssetRegistry& assetRegistry, Grindstone::Assets::AssetManager& assetManager, const std::filesystem::path& path);
}
