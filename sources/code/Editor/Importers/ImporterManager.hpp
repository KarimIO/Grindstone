#pragma once

#include <Common/Editor/Importer.hpp>
#include <Editor/AssetRegistry.hpp>
#include <EngineCore/Assets/AssetManager.hpp>

#include <filesystem>
#include <map>
#include <string>

namespace Grindstone::Importers {
	class ImporterManager {
	public:
		struct ImporterData {
			Grindstone::Editor::ImporterVersion importerVersion;
			Grindstone::Editor::ImporterFactory factory;
		};

		ImporterManager();
		bool Import(const std::filesystem::path& path);
		void AddImporterFactory(const std::string& extension, Grindstone::Editor::ImporterFactory importerToAdd, Grindstone::Editor::ImporterVersion importerVersion);
		void RemoveImporterFactoryByExtension(const std::string& extension);

		Grindstone::Editor::ImporterVersion GetImporterVersion(const std::string& extension);
		Grindstone::Editor::ImporterVersion GetImporterVersion(const std::filesystem::path& path) const;
		bool HasImporter(const std::string& extension) const;
		bool HasImporter(const std::filesystem::path& path) const;
		Grindstone::Editor::ImporterFactory GetImporterFactoryByExtension(const std::string& extension) const;
		Grindstone::Editor::ImporterFactory GetImporterFactoryByPath(const std::filesystem::path& path) const;
	private:
		std::map<std::string, ImporterData> extensionsToImporterFactories;
	};
}
