#pragma once

#include <string>

#include <Common/ResourcePipeline/Uuid.hpp>
#include <Common/Editor/Importer.hpp>

namespace Grindstone::Editor::Importers {
	class MaterialImporter : public Importer {
	public:
		void Import(const std::filesystem::path& path) override;
		Uuid GetUuidAfterImport() const;
	private:
		Uuid uuid;
	};

	void ImportMaterial(const std::filesystem::path& inputPath);
}
