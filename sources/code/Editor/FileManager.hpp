#pragma once

#include <filesystem>
#include <efsw/efsw.h>
#include "FileDirectory.hpp"

namespace Grindstone::Editor {
	class FileManager {
	public:
		void WatchDirectory(std::string_view mountPoint, const std::filesystem::path& projectPath);
		Directory& GetPrimaryDirectory();

		virtual bool TryGetPathWithMountPoint(const std::filesystem::path& path, std::filesystem::path& outMountedPath) const;

		void HandleAddPath(std::filesystem::directory_entry directoryEntry);
		void HandleAddFolder(std::filesystem::directory_entry folderPath);
		void HandleAddMetaFile(std::filesystem::directory_entry filePath);
		void HandleAddFile(std::filesystem::directory_entry filePath);

		void HandleModifyPath(std::filesystem::directory_entry directoryEntry);
		void HandleModifyFolder(std::filesystem::directory_entry folderPath);
		void HandleModifyMetaFile(std::filesystem::directory_entry filePath);
		void HandleModifyFile(std::filesystem::directory_entry filePath);

		void HandleMovePath(std::filesystem::directory_entry directoryEntry, std::string oldFilename);
		void HandleMoveFolder(std::filesystem::directory_entry folderPath, std::string oldFilename);
		void HandleMoveMetaFile(std::filesystem::directory_entry filePath, std::string oldFilename);
		void HandleMoveFile(std::filesystem::directory_entry filePath, std::string oldFilename);

		void HandleDeletePath(std::filesystem::directory_entry directoryEntry);
		void HandleDeleteFolder(std::filesystem::directory_entry folderPath);
		void HandleDeleteMetaFile(std::filesystem::directory_entry filePath);
		void HandleDeleteFile(std::filesystem::directory_entry filePath);
	private:
		using FolderIterator = std::vector<Directory*>::iterator;
		using FileIterator = std::vector<File*>::iterator;
		FolderIterator GetSubdirectoryInDirectory(Directory* directory, std::string filename);
		FileIterator GetFileInDirectory(Directory* directory, std::string filename);
		Directory* GetOrMakeSubdirectory(Directory* currentDirectory, std::string subdirectoryName);
		Directory* GetFolderForPath(std::filesystem::path path);
		void CreateInitialFileStructure(Directory& directory, std::filesystem::directory_iterator directoryIterator);
		bool CheckIfCompiledFileNeedsToBeUpdated(std::filesystem::path path);
		void UpdateCompiledFileIfNecessaryOnInitialize(std::filesystem::path path);
		void UpdateCompiledFileIfNecessary(std::filesystem::path path);
		void RemoveFileFromManager(std::filesystem::directory_entry);
		std::filesystem::directory_entry GetFileFromMetaPath(std::filesystem::directory_entry);

		struct MountPoint {
			efsw_watchid watchID;
			std::string mountPoint;
			Directory rootDirectory;
		};

		std::vector<MountPoint> mountedDirectories;
	};
}
