#pragma once

#include <Common/HashedString.hpp>
#include <EditorCommon/Editor/Importer.hpp>
#include <Editor/AssetRegistry.hpp>
#include <EngineCore/Assets/AssetManager.hpp>

#include <filesystem>
#include <map>
#include <string>

namespace Grindstone::Importers {
	class ImporterManager {
	public:
		void Initialize();
		bool Import(const std::filesystem::path& path);
		void MapExtensionToImporterType(const std::string& extension, Grindstone::HashedString importerType);
		void AddImporterFactory(Grindstone::HashedString importerType, Grindstone::Editor::ImporterData importerData);
		void UnmapExtensionToImporterType(const std::string& extension);
		void RemoveImporterFactory(Grindstone::HashedString importerType);

		Grindstone::Editor::ImporterVersion GetImporterVersion(Grindstone::HashedString importerType) const;
		Grindstone::Editor::ImporterVersion GetImporterVersionByExtension(const std::string& extension) const;
		Grindstone::Editor::ImporterVersion GetImporterVersionByPath(const std::filesystem::path& path) const;

		bool HasImporter(Grindstone::HashedString importerType) const;
		bool HasImporterForExtension(const std::string& extension) const;
		bool HasImporterForPath(const std::filesystem::path& path) const;

		Grindstone::Editor::ImporterData GetImporterFactoryByName(Grindstone::HashedString importerType) const;
		Grindstone::Editor::ImporterData GetImporterFactoryByExtension(const std::string& extension) const;
		Grindstone::Editor::ImporterData GetImporterFactoryByPath(const std::filesystem::path& path) const;
	private:
		std::map<std::string, Grindstone::HashedString> extensionsToImporterFactories;
		std::map<Grindstone::HashedString, Grindstone::Editor::ImporterData> importerFactoriesMap;
	};
}
