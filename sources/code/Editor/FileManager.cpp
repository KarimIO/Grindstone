#include <filesystem>
#include <efsw/include/efsw/efsw.hpp>
#include "FileManager.hpp"
#include "EditorManager.hpp"
#include "Common/Logging.hpp"
using namespace Grindstone;
using namespace Grindstone::Editor;

class UpdateListener : public efsw::FileWatchListener {
private:
	FileManager* fileManager = nullptr;
public:
	UpdateListener() {}

	void handleFileAction(efsw::WatchID watchid, const std::string& dir, const std::string& filename, efsw::Action action, std::string oldFilename = "") override {
		std::filesystem::path path = std::filesystem::path(dir) / filename;
		std::filesystem::directory_entry entry = std::filesystem::directory_entry(path);
		
		switch (action) {
		case efsw::Actions::Add:
			Editor::Manager::Print(LogSeverity::Info, "FILE has event added");
			if (fileManager) {
				fileManager->AddPath(entry);
			}
			break;
		case efsw::Actions::Delete:
			Editor::Manager::Print(LogSeverity::Info, "FILE has event delete");
			if (fileManager) {
				fileManager->DeletePath(entry);
			}
			break;
		case efsw::Actions::Modified:
			Editor::Manager::Print(LogSeverity::Info, "FILE has event modified");
			if (fileManager) {
				fileManager->ModifyPath(entry);
			}
			break;
		case efsw::Actions::Moved:
			Editor::Manager::Print(LogSeverity::Info, "FILE has event moved from");
			if (fileManager) {
				fileManager->MovePath(entry);
			}
			break;
		default:
			Editor::Manager::Print(LogSeverity::Info, "Invalid filesystem event!");
		}
	}
};

void FileManager::Initialize() {
	efsw::FileWatcher* fileWatcher = new efsw::FileWatcher();
	UpdateListener* listener = new UpdateListener();
	efsw::WatchID watchID = fileWatcher->addWatch(ASSET_FOLDER_PATH.string().c_str(), listener, true);
	fileWatcher->watch();

	rootDirectory.path = std::filesystem::directory_entry(ASSET_FOLDER_PATH);
	rootDirectory.parentDirectory = nullptr;
	CreateInitialFileStructure(rootDirectory, std::filesystem::directory_iterator(rootDirectory.path));
}

Directory& Grindstone::Editor::FileManager::GetRootDirectory() {
	return rootDirectory;
}

void FileManager::CreateInitialFileStructure(Directory& directory, std::filesystem::directory_iterator directoryIterator) {
	for (const auto& directoryEntry : directoryIterator) {
		if (directoryEntry.is_directory()) {
			Directory* newDirectory = new Directory(directoryEntry, &directory);
			directory.subdirectories.push_back(newDirectory);
			auto directoryIterator = std::filesystem::directory_iterator(directoryEntry);
			CreateInitialFileStructure(*newDirectory, directoryIterator);
		}
		else {
			directory.files.emplace_back(directoryEntry);
		}
	}
}

void FileManager::AddPath(std::filesystem::directory_entry directoryEntry) {
	if (directoryEntry.is_directory()) {
		AddFolder(directoryEntry.path());
	}
	else {
		AddFile(directoryEntry.path());
	}
}

void FileManager::AddFolder(std::filesystem::path folderPath) {

}

void FileManager::AddFile(std::filesystem::path filePath) {

}

void FileManager::ModifyPath(std::filesystem::directory_entry directoryEntry) {
	if (directoryEntry.is_directory()) {
		ModifyFolder(directoryEntry.path());
	}
	else {
		ModifyFile(directoryEntry.path());
	}
}

void FileManager::ModifyFolder(std::filesystem::path folderPath) {

}

void FileManager::ModifyFile(std::filesystem::path filePath) {

}

void FileManager::MovePath(std::filesystem::directory_entry directoryEntry) {
	if (directoryEntry.is_directory()) {
		MoveFolder(directoryEntry.path());
	}
	else {
		MoveFile(directoryEntry.path());
	}
}

void FileManager::MoveFolder(std::filesystem::path folderPath) {

}

void FileManager::MoveFile(std::filesystem::path filePath) {

}

void FileManager::DeletePath(std::filesystem::directory_entry directoryEntry) {
	if (directoryEntry.is_directory()) {
		DeleteFolder(directoryEntry.path());
	}
	else {
		DeleteFile(directoryEntry.path());
	}
}

void FileManager::DeleteFolder(std::filesystem::path folderPath) {

}

void FileManager::DeleteFile(std::filesystem::path filePath) {

}

void FileManager::GetClosestDirectory(std::filesystem::path path) {

}
