#pragma once

#include <string>
#include <vector>

#include <Common/ResourcePipeline/AssetType.hpp>
#include <Common/ResourcePipeline/Uuid.hpp>

namespace Grindstone::Assets {
	struct ArchiveDirectoryFile {
		const static uint32_t CURRENT_VERSION = 1;

		struct Header {
			const char signature[4] = { 'G', 'D', 'I', 'R' };
			uint32_t version = CURRENT_VERSION;
			uint32_t buildCode = 0;
			uint32_t headerSize = sizeof(Header);
			uint32_t assetTypeCount = static_cast<uint32_t>(AssetType::Count);
			uint32_t assetTypeIndexSize;
			uint32_t assetInfoIndexSize;
			uint32_t archiveIndexSize;
			uint32_t stringsSize;
		};

		struct AssetTypeSectionInfo {
			uint64_t offset;
			uint16_t count;
		};

		struct AssetInfo {
			Grindstone::Uuid uuid;
			uint64_t filenameOffset;
			uint16_t filenameSize;
			uint32_t crc;
			uint16_t archiveIndex;
			uint64_t offset;
			uint64_t size;
		};

		struct ArchiveInfo {
			uint32_t crc;
		};

		Header header;
		std::vector<AssetTypeSectionInfo> assetInfoIndex;
		std::vector<AssetInfo> assets;
		std::vector<ArchiveInfo> archives;
		std::vector<std::string> strings;
	};
}
