#pragma once

#include <filesystem>
#include <string>
#include <vector>

#include <Common/ResourcePipeline/MetaFile.hpp>


namespace Grindstone::Editor {
	class MetaFile;

	struct File {
		std::filesystem::directory_entry directoryEntry;
		MetaFile metaFile;
		File(std::filesystem::directory_entry entry);
	};

	struct Directory {
		Directory* parentDirectory = nullptr;
		std::string name;
		std::filesystem::directory_entry path;
		std::vector<Directory*> subdirectories;
		std::vector<File*> files;

		Directory() = default;
		Directory(std::filesystem::directory_entry path, Directory* parentDirectory);
	};
}
