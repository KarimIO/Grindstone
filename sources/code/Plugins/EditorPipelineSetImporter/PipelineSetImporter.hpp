#pragma once

#include <string>
#include <vector>

#include <Editor/AssetRegistry.hpp>
#include <EngineCore/Assets/AssetManager.hpp>

namespace Grindstone::Editor::Importers {
	const Grindstone::Editor::ImporterVersion pipelineSetImporterVersion = 0;

	void ImportShadersFromGlsl(Grindstone::Editor::AssetRegistry& assetRegistry, Grindstone::Assets::AssetManager& assetManager, const std::filesystem::path& path);
}
