#pragma once

#include <vector>
#include <filesystem>
#include "Common/ResourcePipeline/MetaFile.hpp"

const std::filesystem::path ASSET_FOLDER_PATH =
#ifdef _WIN32
	"..\\assets";
#else
	"../assets";
#endif

namespace Grindstone {
	namespace Editor {
		struct File {
			std::filesystem::directory_entry directoryEntry;
			MetaFile metaFile;
			File(std::filesystem::directory_entry entry)
			: directoryEntry(entry) {
				metaFile.Load(entry.path());
			}
		};

		struct Directory {
			Directory* parentDirectory = nullptr;
			std::string name;
			std::filesystem::directory_entry path;
			std::vector<Directory*> subdirectories;
			std::vector<File*> files;

			Directory() = default;
			Directory(std::filesystem::directory_entry path, Directory* parentDirectory) :
				path(path),
				parentDirectory(parentDirectory) {
				name = path.path().filename().string();
			}
		};
	}
}
