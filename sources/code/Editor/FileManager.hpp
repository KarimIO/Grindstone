#pragma once

#include <set>
#include <filesystem>
#include <efsw/efsw.h>

namespace Grindstone::Editor {	
	class FileManager {
	public:
		struct MountPoint {
			efsw_watchid watchID;
			std::string mountPoint;
			std::filesystem::path path;
		};

		void WatchDirectory(
			std::string_view mountPoint,
			const std::filesystem::path& projectPath
		);
		
		const std::vector<MountPoint>& GetMountedDirectories() const;
		const FileManager::MountPoint& GetPrimaryMountPoint() const;
		void DispatchTask(const std::filesystem::path& path) const;
		virtual bool TryGetPathWithMountPoint(
			const std::filesystem::path& path,
			std::filesystem::path& outMountedPath
		) const;

		virtual bool TryGetAbsolutePathFromMountedPath(
			const std::filesystem::path& mountedPath,
			std::filesystem::path& outAbsolutePath
		) const;

		void HandleAddMetaFile(const MountPoint& mountPoint, const std::filesystem::directory_entry& filePath);
		void HandleAddFile(const MountPoint& mountPoint, const std::filesystem::directory_entry& filePath);

		void HandleModifyMetaFile(const MountPoint& mountPoint, const std::filesystem::directory_entry& filePath);
		void HandleModifyFile(const MountPoint& mountPoint, const std::filesystem::directory_entry& filePath);

		void HandleMoveMetaFile(const MountPoint& mountPoint, const std::filesystem::directory_entry& filePath, std::string oldFilename);
		void HandleMoveFile(const MountPoint& mountPoint, const std::filesystem::directory_entry& filePath, std::string oldFilename);

		void HandleDeleteMetaFile(const MountPoint& mountPoint, const std::filesystem::directory_entry& filePath);
		void HandleDeleteFile(const MountPoint& mountPoint, const std::filesystem::directory_entry& filePath);
	private:
		void PreprocessFilesOnMount(
			const MountPoint& mountPoint,
			std::filesystem::directory_iterator directoryIterator
		) const;
		bool CheckIfCompiledFileNeedsToBeUpdated(const MountPoint& mountPoint, const std::filesystem::path& path) const;
		void UpdateCompiledFileIfNecessary(const MountPoint& mountPoint, const std::filesystem::path& path) const;
		std::filesystem::directory_entry GetFileFromMetaPath(const std::filesystem::directory_entry& entry);

		std::vector<MountPoint> mountedDirectories;
	};
}
