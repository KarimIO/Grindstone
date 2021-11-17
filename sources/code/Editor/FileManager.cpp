#include <filesystem>
#include <efsw/efsw.hpp>
#include "FileManager.hpp"
#include "EditorManager.hpp"
#include "Common/Logging.hpp"
#include "Common/ResourcePipeline/MetaFile.hpp"
#include "Importers/ImporterManager.hpp"
using namespace Grindstone;
using namespace Grindstone::Editor;

class UpdateListener : public efsw::FileWatchListener {
private:
	FileManager* fileManager = nullptr;
public:
	UpdateListener(FileManager* fileManager) : fileManager(fileManager) {}

	void handleFileAction(efsw::WatchID watchid, const std::string& dir, const std::string& filename, efsw::Action action, std::string oldFilename = "") override {
		std::filesystem::path path = std::filesystem::path(dir) / filename;
		std::filesystem::directory_entry entry = std::filesystem::directory_entry(path);
		
		switch (action) {
		case efsw::Actions::Add:
			if (fileManager) {
				fileManager->HandleAddPath(entry);
			}
			break;
		case efsw::Actions::Delete:
			if (fileManager) {
				fileManager->HandleDeletePath(entry);
			}
			break;
		case efsw::Actions::Modified:
			if (fileManager) {
				fileManager->HandleModifyPath(entry);
			}
			break;
		case efsw::Actions::Moved:
			if (fileManager) {
				fileManager->HandleMovePath(entry, oldFilename);
			}
			break;
		default:
			Editor::Manager::Print(LogSeverity::Info, "Invalid filesystem event!");
		}
	}
};

void FileManager::Initialize() {
	efsw::FileWatcher* fileWatcher = new efsw::FileWatcher();
	UpdateListener* listener = new UpdateListener(this);
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
			if (directoryEntry.path().extension().string() != ".meta") {
				directory.files.emplace_back(new File(directoryEntry));
				UpdateCompiledFileIfNecessary(directoryEntry.path());
			}
		}
	}
}

bool FileManager::CheckIfCompiledFileNeedsToBeUpdated(std::filesystem::path path) {
	auto& importManager = Editor::Manager::GetInstance().GetImporterManager();
	if (!importManager.HasImporter(path)) {
		return false;
	}

	std::filesystem::path metaFilePath = path.string() + ".meta";
	if (!std::filesystem::exists(metaFilePath)) {
		return true;
	}

	auto metaFileLastWriteTime = std::filesystem::last_write_time(metaFilePath);
	auto assetFileLastWriteTime = std::filesystem::last_write_time(path);
	if (metaFileLastWriteTime > assetFileLastWriteTime) {
		return false;
	}

	return true;
}

void FileManager::UpdateCompiledFileIfNecessary(std::filesystem::path path) {
	if (CheckIfCompiledFileNeedsToBeUpdated(path)) {
		auto& importManager = Editor::Manager::GetInstance().GetImporterManager();
		importManager.Import(path);
	}
}

FileManager::FolderIterator FileManager::GetSubdirectoryInDirectory(Directory* directory, std::string filename) {
	return std::find_if(
		directory->subdirectories.begin(),
		directory->subdirectories.end(),
		[&](Directory* entry) {
			return entry->name == filename;
		}
	);
}

FileManager::FileIterator FileManager::GetFileInDirectory(Directory* directory, std::string filename) {
	return std::find_if(
		directory->files.begin(),
		directory->files.end(),
		[&](File* entry) {
			return entry->directoryEntry.path().filename().string() == filename;
		}
	);
}

Directory* FileManager::GetOrMakeSubdirectory(Directory* currentDirectory, std::string subdirectoryName) {
	for (auto subdir : currentDirectory->subdirectories) {
		if (subdir->name == subdirectoryName) {
			return subdir;
		}
	}

	std::filesystem::directory_entry newEntry = std::filesystem::directory_entry(currentDirectory->path / subdirectoryName);
	Directory* newSubdirectory = new Directory(newEntry, currentDirectory);
	currentDirectory->subdirectories.push_back(newSubdirectory);

	return newSubdirectory;
}

Directory* FileManager::GetFolderForPath(std::filesystem::path path) {
	std::string pathAsString = std::filesystem::relative(path, ASSET_FOLDER_PATH).string();
	std::replace(pathAsString.begin(), pathAsString.end(), '\\', '/');

	Directory* currentDirectory = &rootDirectory;
	std::size_t lastIndex = 0;

	while (true) {
		std::size_t newIndex = pathAsString.find("/", lastIndex);
		if (newIndex == -1) {
			break;
		}

		std::string node = pathAsString.substr(lastIndex, newIndex - lastIndex);

		currentDirectory = GetOrMakeSubdirectory(currentDirectory, node);

		Editor::Manager::Print(LogSeverity::Info, node.c_str());
		lastIndex = newIndex + 1;
	}

	return currentDirectory;
}

void FileManager::HandleAddPath(std::filesystem::directory_entry directoryEntry) {
	if (directoryEntry.is_directory()) {
		HandleAddFolder(directoryEntry);
	}
	else if (directoryEntry.path().extension() == ".meta") {
		HandleAddMetaFile(directoryEntry);
	}
	else {
		HandleAddFile(directoryEntry);
	}
}

void FileManager::HandleAddFolder(std::filesystem::directory_entry folderPath) {
	Directory* parentDirectory = GetFolderForPath(folderPath);
	Directory* newDirectory = new Directory(folderPath, parentDirectory);
	parentDirectory->subdirectories.push_back(newDirectory);
}

void FileManager::HandleAddFile(std::filesystem::directory_entry filePath) {
	Directory* entry = GetFolderForPath(filePath);
	entry->files.emplace_back(new File(filePath));
	UpdateCompiledFileIfNecessary(filePath);
}

void FileManager::HandleAddMetaFile(std::filesystem::directory_entry fileDirectoryEntry) {
	auto mainEntry = GetFileFromMetaPath(fileDirectoryEntry);
	if (std::filesystem::exists(mainEntry)) {
		UpdateCompiledFileIfNecessary(mainEntry);
	}
}

void FileManager::HandleModifyPath(std::filesystem::directory_entry directoryEntry) {
	if (directoryEntry.is_directory()) {
		HandleModifyFolder(directoryEntry);
	}
	else if (directoryEntry.path().extension() == ".meta") {
		HandleModifyMetaFile(directoryEntry);
	}
	else {
		HandleModifyFile(directoryEntry);
	}
}

void FileManager::HandleModifyFolder(std::filesystem::directory_entry folderPath) {

}

void FileManager::HandleModifyMetaFile(std::filesystem::directory_entry fileDirectoryEntry) {
}

void FileManager::HandleModifyFile(std::filesystem::directory_entry filePath) {
	UpdateCompiledFileIfNecessary(filePath);
}

void FileManager::HandleMovePath(std::filesystem::directory_entry directoryEntry, std::string oldFilename) {
	if (directoryEntry.is_directory()) {
		HandleMoveFolder(directoryEntry, oldFilename);
	}
	else if (directoryEntry.path().extension() == ".meta") {
		HandleMoveMetaFile(directoryEntry, oldFilename);
	}
	else {
		HandleMoveFile(directoryEntry, oldFilename);
	}
}

void FileManager::HandleMoveFolder(std::filesystem::directory_entry folderPath, std::string oldFilename) {

}

void FileManager::HandleMoveMetaFile(std::filesystem::directory_entry fileDirectoryEntry, std::string oldFilename) {
	auto oldMetaPath = (std::filesystem::directory_entry)oldFilename;
	auto oldFilePath = GetFileFromMetaPath(oldMetaPath);
	if (std::filesystem::exists(oldFilePath)) {
		UpdateCompiledFileIfNecessary(oldFilePath);
	}
}

void FileManager::HandleMoveFile(std::filesystem::directory_entry filePath, std::string oldFilename) {
	RemoveFileFromManager(filePath);

	Directory* entry = GetFolderForPath(filePath);
	entry->files.emplace_back(new File(filePath));

	std::string metaFilePath = filePath.path().string() + ".meta";
	std::string oldMetaFilePath = oldFilename + ".meta";
	if (std::filesystem::exists(oldMetaFilePath)) {
		std::filesystem::rename(oldMetaFilePath, metaFilePath);
	}

	UpdateCompiledFileIfNecessary(filePath);
}

void FileManager::HandleDeletePath(std::filesystem::directory_entry directoryEntry) {
	if (directoryEntry.is_directory()) {
		HandleDeleteFolder(directoryEntry);
	}
	else if (directoryEntry.path().extension() == ".meta") {
		HandleDeleteMetaFile(directoryEntry);
	}
	else {
		HandleDeleteFile(directoryEntry);
	}
}

void FileManager::HandleDeleteFolder(std::filesystem::directory_entry folderPath) {
	Directory* directory = GetFolderForPath(folderPath);
	std::filesystem::path path = folderPath.path();
	std::string fileName = path.filename().string();
	FolderIterator folderIterator = GetSubdirectoryInDirectory(directory, fileName);
	if (folderIterator != directory->subdirectories.end()) {
		directory->subdirectories.erase(folderIterator);
	}
}

void FileManager::HandleDeleteMetaFile(std::filesystem::directory_entry fileDirectoryEntry) {
	UpdateCompiledFileIfNecessary(GetFileFromMetaPath(fileDirectoryEntry));
}

void FileManager::HandleDeleteFile(std::filesystem::directory_entry fileDirectoryEntry) {
	RemoveFileFromManager(fileDirectoryEntry);

	std::filesystem::path path = fileDirectoryEntry.path();
	std::string metaFilePath = path.string() + ".meta";
	if (std::filesystem::exists(metaFilePath)) {
		std::filesystem::remove(metaFilePath);
	}
}

void FileManager::RemoveFileFromManager(std::filesystem::directory_entry fileDirectoryEntry) {
	Directory* directory = GetFolderForPath(fileDirectoryEntry);
	std::filesystem::path path = fileDirectoryEntry.path();
	std::string fileName = path.filename().string();
	FileIterator fileIterator = GetFileInDirectory(directory, fileName);
	if (fileIterator != directory->files.end()) {
		directory->files.erase(fileIterator);
	}
}

std::filesystem::directory_entry FileManager::GetFileFromMetaPath(std::filesystem::directory_entry path) {
	std::string pathStr = path.path().string();
	return (std::filesystem::directory_entry)pathStr.substr(0, pathStr.find_last_of('.'));
}
