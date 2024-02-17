#include <fstream>
#include <filesystem>
#include <EngineCore/Utils/Utilities.hpp>
#include <EngineCore/EngineCore.hpp>
#include <EngineCore/Logger.hpp>
#include <Common/Graphics/Core.hpp>
#include <Common/Assets/ArchiveDirectory.hpp>
#include <Common/Assets/ArchiveDirectoryFile.hpp>
#include "ArchiveDirectoryDeserializer.hpp"
using namespace Grindstone::Assets;

ArchiveDirectoryDeserializer::ArchiveDirectoryDeserializer(ArchiveDirectory& archiveDirectory) : archiveDirectory(archiveDirectory) {
	Load();
}

void ArchiveDirectoryDeserializer::Load() {
	// TODO: How should we get this filename? Maybe search for all asset directories?
	std::filesystem::path path = EngineCore::GetInstance().GetProjectPath() / "archives/TestArchive.gdir";
	Load(path);
}

void ArchiveDirectoryDeserializer::Load(std::filesystem::path path) {
	if (!std::filesystem::exists(path)) {
		return;
	}

	std::ifstream file(path, std::ios::binary);
	file.unsetf(std::ios::skipws);

	file.seekg(0, std::ios::end);
	size_t fileSize = file.tellg();
	file.seekg(0, std::ios::beg);

	if (fileSize < sizeof(ArchiveDirectoryFile::Header)) {
		// Too small
		return;
	}

	// TODO: Use allocator
	// TODO: Catch if cannot allocate memory
	std::vector<char> fileData(fileSize);
	file.read(fileData.data(), fileSize);
	file.close();

	char* offset = fileData.data();

	ArchiveDirectoryFile::Header* header = reinterpret_cast<ArchiveDirectoryFile::Header*>(offset);
	if (strncmp(header->signature, "GDIR", 4) != 0) {
		return;
	}

	// Unsupported version
	if (header->version != ArchiveDirectoryFile::CURRENT_VERSION) {
		return;
	}


	char* assetTypeSectionCharPtr = offset + sizeof(ArchiveDirectoryFile::Header);

	archiveDirectory.strings.resize(header->stringsSize);
	char* srcPtr = assetTypeSectionCharPtr + header->archiveIndexSize + header->assetInfoIndexSize + header->assetTypeIndexSize;
	memcpy(archiveDirectory.strings.data(), srcPtr, header->stringsSize);

	ArchiveDirectoryFile::AssetTypeSectionInfo* assetTypeSectionArr = reinterpret_cast<ArchiveDirectoryFile::AssetTypeSectionInfo*>(assetTypeSectionCharPtr);
	archiveDirectory.assetTypeIndices.resize(header->assetTypeCount);
	for (uint32_t assetTypeIndex = 0; assetTypeIndex < header->assetTypeCount; ++assetTypeIndex) {
		ArchiveDirectoryFile::AssetTypeSectionInfo& assetTypeSectionInfo = assetTypeSectionArr[assetTypeIndex];

		ArchiveDirectory::AssetTypeIndex& assetTypeDst = archiveDirectory.assetTypeIndices[assetTypeIndex];
		std::map<Uuid, ArchiveDirectory::AssetInfo>& assetTypeMap = assetTypeDst.assets;

		ArchiveDirectoryFile::AssetInfo* srcAssetArr = reinterpret_cast<ArchiveDirectoryFile::AssetInfo*>(offset + assetTypeSectionInfo.offset);
		for (uint16_t i = 0; i < assetTypeSectionInfo.count; ++i) {
			ArchiveDirectoryFile::AssetInfo& srcAsset = srcAssetArr[i];

			ArchiveDirectory::AssetInfo& dstAsset = assetTypeMap[srcAsset.uuid];
			dstAsset.archiveIndex = srcAsset.archiveIndex;
			dstAsset.crc = srcAsset.crc;
			dstAsset.filename = std::string_view(archiveDirectory.strings.data() + srcAsset.filenameOffset, srcAsset.filenameSize);
			dstAsset.offset = srcAsset.offset;
			dstAsset.size = srcAsset.size;
		}
	}

}
