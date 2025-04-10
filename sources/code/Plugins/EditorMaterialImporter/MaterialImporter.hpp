#pragma once

#include <string>

#include <Common/ResourcePipeline/Uuid.hpp>
#include <Common/Editor/Importer.hpp>

namespace Grindstone::Editor::Importers {
	class MaterialImporter : public Importer {
	public:
		void Import(Grindstone::Editor::AssetRegistry& assetRegistry, Grindstone::Assets::AssetManager& assetManager, const std::filesystem::path& path) override;
		Uuid GetUuidAfterImport() const;
	private:
		Uuid uuid;
	};

	const Grindstone::Editor::ImporterVersion materialImporterVersion = 1;

	void ImportMaterial(Grindstone::Editor::AssetRegistry& assetRegistry, Grindstone::Assets::AssetManager& assetManager, const std::filesystem::path& inputPath);
}
