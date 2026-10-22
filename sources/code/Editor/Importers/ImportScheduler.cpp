#include <filesystem>
#include <mutex>

#include <Common/Logger.hpp>
#include <Editor/AssetRegistry.hpp>
#include <Editor/EditorManager.hpp>
#include <Editor/Importers/ImporterRegistry.hpp>
#include <Editor/TaskSystem.hpp>
#include <EditorCommon/Editor/Importer.hpp>
#include <EngineCore/Assets/AssetManager.hpp>
#include <EngineCore/EngineCore.hpp>

#include "ImportScheduler.hpp"

bool Grindstone::Importers::ImportScheduler::QueueImport(const std::filesystem::path& path) {
	if (!std::filesystem::exists(path)) {
		GPRINT_ERROR(LogSource::EditorImporter, "Trying to import a nonexistant file: {}", path.string().c_str());
		return false;
	}

	/*
	std::scoped_lock lck(queueMutex);
	// Only enqueue an import if we are not already attempting to import it.
	auto it = std::find(pendingImports.begin(), pendingImports.end(), path);
	if (it == pendingImports.end()) {
		pendingImports.push_back(path);
	}
	*/

	Grindstone::Editor::TaskSystem& taskSystem = Editor::Manager::GetInstance().GetTaskSystem();

	std::string jobName = "Importing " + path.filename().string();
	taskSystem.Execute(jobName, [this, path] {
		Import(path);
		GPRINT_TRACE(LogSource::Editor, "Finished importing {}.", path.string().c_str());
	});

	return true;
}

bool Grindstone::Importers::ImportScheduler::Import(const std::filesystem::path& path) {	
	ImporterRegistry& importerRegistry = Editor::Manager::GetInstance().GetImporterRegistry();
	
	Grindstone::Editor::ImporterFactory importerFactory = importerRegistry.GetImporterFactoryByPath(path).factory;
	if (importerFactory == nullptr) {
		return false;
	}

	Grindstone::Editor::AssetRegistry& assetRegistry = Editor::Manager::GetInstance().GetAssetRegistry();
	Grindstone::Assets::AssetManager& assetManager = *Editor::Manager::GetEngineCore().assetManager;

	importerFactory(assetRegistry, assetManager, path);
	return true;
}

void Grindstone::Importers::ImportScheduler::ProcessImport() {

}
