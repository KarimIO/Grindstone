#pragma once

#include "FileDirectory.hpp"

namespace Grindstone {
	namespace Editor {
		class FileManager {
		public:
			void Initialize();

			Directory& GetRootDirectory();

			void AddPath(std::filesystem::directory_entry directoryEntry);
			void AddFolder(std::filesystem::path folderPath);
			void AddFile(std::filesystem::path filePath);

			void ModifyPath(std::filesystem::directory_entry directoryEntry);
			void ModifyFolder(std::filesystem::path folderPath);
			void ModifyFile(std::filesystem::path filePath);

			void MovePath(std::filesystem::directory_entry directoryEntry);
			void MoveFolder(std::filesystem::path folderPath);
			void MoveFile(std::filesystem::path filePath);

			void DeletePath(std::filesystem::directory_entry directoryEntry);
			void DeleteFolder(std::filesystem::path folderPath);
			void DeleteFile(std::filesystem::path filePath);

			void GetClosestDirectory(std::filesystem::path path);
		private:
			void CreateInitialFileStructure(Directory& directory, std::filesystem::directory_iterator directoryIterator);

			Directory rootDirectory;
		};
	}
}
