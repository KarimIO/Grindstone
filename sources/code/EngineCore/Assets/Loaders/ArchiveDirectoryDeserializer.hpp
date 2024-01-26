#pragma once

#include <filesystem>

#include <Common/Assets/ArchiveDirectory.hpp>

namespace Grindstone::Assets {
	class ArchiveDirectoryDeserializer {
	public:
		ArchiveDirectoryDeserializer(ArchiveDirectory& archiveDirectory);
		void Load();
		void Load(std::filesystem::path path);
	private:
		ArchiveDirectory& archiveDirectory;
	};
}
