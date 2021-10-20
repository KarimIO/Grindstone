#pragma once

#include <filesystem>
#include <map>
#include <string>
#include <vector>
#include <functional>

#include "Importer.hpp"

namespace Grindstone {
	namespace Importers {
		class ImporterManager {
		public:
			using ImporterFactory = std::function<void(std::filesystem::path)>;

			ImporterManager();
			bool Import(std::filesystem::path& path);
			void AddImporterFactory(std::string extension, ImporterFactory importerToAdd);
			void RemoveImporterFactoryByExtension(std::string& extension);
			bool HasImporter(std::string& extension);
			bool HasImporter(std::filesystem::path& path);
			ImporterFactory GetImporterFactoryByExtension(std::string& extension);
			ImporterFactory GetImporterFactoryByPath(std::filesystem::path& path);
		private:
			std::map<std::string, ImporterFactory> extensionsToImporterFactories;
		};
	}
}
