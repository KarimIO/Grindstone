#pragma once

#include <string>
#include <vector>

#include <Common/ResourcePipeline/AssetType.hpp>
#include <Common/ResourcePipeline/Uuid.hpp>

namespace Grindstone::Assets {
	struct ArchiveDirectory {
		struct AssetInfo {
			std::string_view filename;
			uint32_t crc;
			uint16_t archiveIndex;
			uint64_t offset;
			uint64_t size;
		};

		struct AssetTypeIndex {
			std::map<Uuid, AssetInfo> assets;
		};

		struct ArchiveInfo {
			uint32_t crc;
		};

		ArchiveDirectory() {
			assetTypeIndices.resize(static_cast<size_t>(AssetType::Count));
		}

		std::vector<AssetTypeIndex> assetTypeIndices;
		std::vector<ArchiveInfo> archives;
		std::vector<std::string> strings;
	};
}
