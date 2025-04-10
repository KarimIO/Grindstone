#include <filesystem>
#include <string_view>

#include <Common/Logging.hpp>
#include <Common/ResourcePipeline/MetaFile.hpp>
#include <Editor/Importers/ImporterManager.hpp>
#include <Editor/ScriptBuilder/CSharpBuildManager.hpp>
#include <EngineCore/Logger.hpp>

#include "FileManager.hpp"
#include "EditorManager.hpp"

using namespace Grindstone;
using namespace Grindstone::Editor;

static void FileWatcherCallback(
	efsw_watcher watcher,
	efsw_watchid watchid,
	const char* dir,
	const char* filename,
	enum efsw_action action,
	const char* oldFilename,
	void* param
) {
	std::filesystem::path path = std::filesystem::path(dir) / filename;
	std::filesystem::directory_entry entry = std::filesystem::directory_entry(path);
	if (param == nullptr || entry.is_directory()) {
		return;
	}

	FileManager* fileManager = static_cast<FileManager*>(param);
	const Grindstone::Editor::FileManager::MountPoint* mountPoint = nullptr;
	for (const Grindstone::Editor::FileManager::MountPoint& currentMountPoint : fileManager->GetMountedDirectories()) {
		mountPoint = &currentMountPoint;
	}

	if (mountPoint == nullptr) {
		return;
	}

	bool isMetaFile = path.extension() == ".meta";

	switch (action) {
	case EFSW_ADD:
		if (isMetaFile) {
			fileManager->HandleAddFile(*mountPoint, entry);
		}
		else {
			fileManager->HandleAddMetaFile(*mountPoint, entry);
		}
		break;
	case EFSW_DELETE:
		if (isMetaFile) {
			fileManager->HandleDeleteFile(*mountPoint, entry);
		}
		else {
			fileManager->HandleDeleteMetaFile(*mountPoint, entry);
		}
		break;
	case EFSW_MODIFIED:
		if (isMetaFile) {
			fileManager->HandleModifyFile(*mountPoint, entry);
		}
		else {
			fileManager->HandleModifyMetaFile(*mountPoint, entry);
		}
		break;
	case EFSW_MOVED:
		if (isMetaFile) {
			fileManager->HandleMoveFile(*mountPoint, entry, oldFilename);
		}
		else {
			fileManager->HandleMoveMetaFile(*mountPoint, entry, oldFilename);
		}
		break;
	default:
		GPRINT_ERROR_V(LogSource::Editor, "Invalid filesystem event trying to process {}", path.string().c_str());
	}
}

void FileManager::WatchDirectory(std::string_view mountPoint, const std::filesystem::path& path) {
	std::filesystem::create_directories(path);

	efsw_watcher watcher = efsw_create(1);
	efsw_watchid watchID = efsw_addwatch(watcher, path.string().c_str(), FileWatcherCallback, 1, this);

	if (watchID < 0) {
		GPRINT_FATAL_V(LogSource::Editor, "Failed to watch path: {}", efsw_getlasterror());
	}
	else {
		efsw_watch(watcher);
	}

	MountPoint& mountPointEntry = mountedDirectories.emplace_back();
	mountPointEntry.watchID = watchID;
	mountPointEntry.mountPoint = std::string("$") + std::string(mountPoint);
	mountPointEntry.path = path;

	auto directoryIterator = std::filesystem::directory_entry(path);
	PreprocessFilesOnMount(
		mountPointEntry,
		std::filesystem::directory_iterator(directoryIterator)
	);
}

const std::vector<FileManager::MountPoint>& FileManager::GetMountedDirectories() const {
	return mountedDirectories;
}

const FileManager::MountPoint& FileManager::GetPrimaryMountPoint() const {
	return mountedDirectories[0];
}

void FileManager::DispatchTask(const std::filesystem::path& path) const {
	TaskSystem& taskSystem = Editor::Manager::GetInstance().GetTaskSystem();

	std::string jobName = "Importing " + path.filename().string();
	taskSystem.Execute(jobName, [path] {
		GPRINT_TRACE_V(LogSource::Editor, "Importing {}", path.string().c_str());
		std::filesystem::path nPath = path;
		auto& importManager = Editor::Manager::GetInstance().GetImporterManager();
		importManager.Import(nPath);
		GPRINT_TRACE_V(LogSource::Editor, "Finished importing {}", path.string().c_str());
	});
}

bool FileManager::TryGetPathWithMountPoint(const std::filesystem::path& path, std::filesystem::path& outMountedPath) const {
	for (const MountPoint& mountPoint : mountedDirectories) {
		const std::filesystem::path& root = mountPoint.path;
		std::string relative = std::filesystem::relative(path, root).string();
		if (relative.size() != 0 && (relative.size() == 1 || relative[0] != '.' && relative[1] != '.')) {
			outMountedPath = std::filesystem::path(mountPoint.mountPoint) / relative;
			return true;
		}
	}

	return false;
}

bool FileManager::TryGetAbsolutePathFromMountedPath(const std::filesystem::path& mountedPath, std::filesystem::path& outAbsolutePath) const {
	std::string mountingPointStr = mountedPath.string();
	if (mountingPointStr.empty() || mountingPointStr[0] != '$') {
		outAbsolutePath = mountedPath;
		return true;
	}

	size_t charPos = 0;
	for (char c : mountingPointStr) {
		if (c == '\\' || c == '/') {
			break;
		}
		else {
			charPos++;
		}
	}

	mountingPointStr = mountingPointStr.substr(0, charPos);

	for (const MountPoint& mountPoint : mountedDirectories) {
		if (mountingPointStr == mountPoint.mountPoint) {
			outAbsolutePath = mountPoint.path / std::filesystem::relative(mountedPath, mountingPointStr);
			return true;
		}
	}

	return false;
}

void FileManager::PreprocessFilesOnMount(
	const MountPoint& mountPoint,
	std::filesystem::directory_iterator directoryIterator
) const {
	for (const std::filesystem::directory_entry& directoryEntry : directoryIterator) {
		if (directoryEntry.is_directory()) {
			PreprocessFilesOnMount(
				mountPoint,
				std::filesystem::directory_iterator(directoryEntry)
			);
		}
		else {
			const std::filesystem::path& filePath = directoryEntry.path();
			std::string extension = filePath.extension().string();
			if (extension != ".meta") {
				if (extension == ".cs") {
					auto& csharpBuildManager = Editor::Manager::GetInstance().GetCSharpBuildManager();
					csharpBuildManager.AddFileInitial(filePath);
				}

				UpdateCompiledFileIfNecessary(mountPoint, filePath);
			}
		}
	}
}

static bool IsSubassetValid(
	const std::filesystem::path& mountedPath,
	Editor::AssetRegistry& assetRegistry,
	MetaFile::Subasset& subasset
) {
	AssetRegistry::Entry outEntry;
	if (
		!assetRegistry.TryGetAssetData(subasset.uuid, outEntry) ||
		outEntry.assetType != subasset.assetType ||
		// TODO: Fix this so path uses mount point
		outEntry.path != mountedPath
	) {
		return false;
	}

	outEntry.address = subasset.address;
	outEntry.displayName = subasset.displayName;
	outEntry.subassetIdentifier = subasset.subassetIdentifier;
	return true;
}

bool FileManager::CheckIfCompiledFileNeedsToBeUpdated(const MountPoint& mountPoint, const std::filesystem::path& path) const {
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
	if (assetFileLastWriteTime > metaFileLastWriteTime) {
		return true;
	}

	MetaFile metaFile(Editor::Manager::GetInstance().GetAssetRegistry(), path);
	if (metaFile.IsOutdatedVersion() || !metaFile.IsValid()) {
		return true;
	}

	std::filesystem::path mountedPath = std::filesystem::path(mountPoint.mountPoint) / std::filesystem::relative(path, mountPoint.path);
	Grindstone::Editor::AssetRegistry& assetRegistry = Editor::Manager::GetInstance().GetAssetRegistry();
	for (MetaFile::Subasset& subasset : metaFile) {
		if (!IsSubassetValid(mountedPath, assetRegistry, subasset)) {
			return true;
		}
	}

	MetaFile::Subasset defaultSubasset;
	if (metaFile.TryGetDefaultSubasset(defaultSubasset)) {
		if (!IsSubassetValid(mountedPath, assetRegistry, defaultSubasset)) {
			return true;
		}
	}

	return false;
}

void FileManager::UpdateCompiledFileIfNecessary(const MountPoint& mountPoint, const std::filesystem::path& path) const {
	if (CheckIfCompiledFileNeedsToBeUpdated(mountPoint, path)) {
		DispatchTask(path);
	}
}

void FileManager::HandleAddFile(const MountPoint& mountPoint, const std::filesystem::directory_entry& filePath) {
	UpdateCompiledFileIfNecessary(mountPoint, filePath);

	if (filePath.path().extension() == ".cs") {
		auto& csharpBuildManager = Editor::Manager::GetInstance().GetCSharpBuildManager();
		csharpBuildManager.OnFileAdded(filePath);
	}
}

void FileManager::HandleAddMetaFile(const MountPoint& mountPoint, const std::filesystem::directory_entry& directoryEntry) {
	auto mainEntry = GetFileFromMetaPath(directoryEntry);
	if (std::filesystem::exists(mainEntry)) {
		UpdateCompiledFileIfNecessary(mountPoint, mainEntry);
	}
}

void FileManager::HandleModifyMetaFile(const MountPoint& mountPoint, const std::filesystem::directory_entry& directoryEntry) {
}

void FileManager::HandleModifyFile(const MountPoint& mountPoint, const std::filesystem::directory_entry& directoryEntry) {
	if (directoryEntry.path().extension() == ".cs") {
		auto& csharpBuildManager = Editor::Manager::GetInstance().GetCSharpBuildManager();
		csharpBuildManager.OnFileModified(directoryEntry);
	}

	UpdateCompiledFileIfNecessary(mountPoint, directoryEntry);
}

void FileManager::HandleMoveMetaFile(const MountPoint& mountPoint, const std::filesystem::directory_entry& directoryEntry, std::string oldFilename) {
	auto oldMetaPath = (std::filesystem::directory_entry)oldFilename;
	auto oldFilePath = GetFileFromMetaPath(oldMetaPath);
	if (std::filesystem::exists(oldFilePath)) {
		UpdateCompiledFileIfNecessary(mountPoint, oldFilePath);
	}
}

void FileManager::HandleMoveFile(const MountPoint& mountPoint, const std::filesystem::directory_entry& directoryEntry, std::string oldFilename) {
	if (directoryEntry.path().extension() == ".cs") {
		auto& csharpBuildManager = Editor::Manager::GetInstance().GetCSharpBuildManager();
		csharpBuildManager.OnFileMoved(directoryEntry, oldFilename);
	}

	std::string metaFilePath = directoryEntry.path().string() + ".meta";
	std::string oldMetaFilePath = oldFilename + ".meta";
	if (std::filesystem::exists(oldMetaFilePath)) {
		std::filesystem::rename(oldMetaFilePath, metaFilePath);
	}

	UpdateCompiledFileIfNecessary(mountPoint, directoryEntry);
}

void FileManager::HandleDeleteMetaFile(const MountPoint& mountPoint, const std::filesystem::directory_entry& directoryEntry) {
	UpdateCompiledFileIfNecessary(mountPoint, GetFileFromMetaPath(directoryEntry));
}

void FileManager::HandleDeleteFile(const MountPoint& mountPoint, const std::filesystem::directory_entry& directoryEntry) {
	std::filesystem::path path = directoryEntry.path();
	if (path.extension() == ".cs") {
		auto& csharpBuildManager = Editor::Manager::GetInstance().GetCSharpBuildManager();
		csharpBuildManager.OnFileDeleted(path);
	}

	std::string metaFilePath = path.string() + ".meta";
	if (std::filesystem::exists(metaFilePath)) {
		std::filesystem::remove(metaFilePath);
	}
}

std::filesystem::directory_entry FileManager::GetFileFromMetaPath(const std::filesystem::directory_entry& path) {
	std::string pathStr = path.path().string();
	return (std::filesystem::directory_entry)pathStr.substr(0, pathStr.find_last_of('.'));
}
