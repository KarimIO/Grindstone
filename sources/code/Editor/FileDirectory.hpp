#pragma once

#include <vector>
#include <filesystem>

const std::filesystem::path ASSET_FOLDER_PATH =
#ifdef _WIN32
	"..\\assets";
#else
	"../assets";
#endif

namespace Grindstone {
	namespace Editor {
		struct Directory {
			Directory* parentDirectory = nullptr;
			std::filesystem::directory_entry path;
			std::vector<Directory*> subdirectories;
			std::vector<std::filesystem::directory_entry> files;

			Directory() = default;
			Directory(std::filesystem::directory_entry path, Directory* parentDirectory) :
				path(path),
				parentDirectory(parentDirectory) {}
		};
	}
}
