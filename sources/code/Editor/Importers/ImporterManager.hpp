#pragma once

#include <Editor/AssetRegistry.hpp>
#include <EngineCore/Assets/AssetManager.hpp>
#include <filesystem>
#include <map>
#include <string>

namespace Grindstone::Importers {
	class ImporterManager {
	public:
		using ImporterFactory = void(*)(Grindstone::Editor::AssetRegistry& assetRegistry, Grindstone::Assets::AssetManager& assetManger, const std::filesystem::path&);

		ImporterManager();
		bool Import(const std::filesystem::path& path);
		void AddImporterFactory(const std::string& extension, ImporterFactory importerToAdd);
		void RemoveImporterFactoryByExtension(const std::string& extension);
		bool HasImporter(const std::string& extension) const;
		bool HasImporter(const std::filesystem::path& path) const;
		ImporterFactory GetImporterFactoryByExtension(const std::string& extension) const;
		ImporterFactory GetImporterFactoryByPath(const std::filesystem::path& path) const;
	private:
		std::map<std::string, ImporterFactory> extensionsToImporterFactories;
	};
}
