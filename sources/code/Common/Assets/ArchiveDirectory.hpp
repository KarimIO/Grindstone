#pragma once

#include <cstdint>
#include <map>
#include <string_view>
#include <vector>

#include <Common/ResourcePipeline/AssetType.hpp>
#include <Common/ResourcePipeline/Uuid.hpp>

namespace Grindstone::Assets {
	struct ArchiveDirectory {
		struct AssetInfo {
			std::string_view displayName;
			std::string_view address;
			uint32_t crc;
			uint16_t archiveIndex;
			uint64_t offset;
			uint64_t size;
		};

		struct AssetTypeIndex {
			std::map<Uuid, AssetInfo> assetsByUuid;
			std::map<std::string_view, Grindstone::Uuid> assetUuidByAddress;
		};

		struct ArchiveInfo {
			uint32_t crc;
		};

		ArchiveDirectory() {
			assetTypeIndices.resize(static_cast<size_t>(AssetType::Count));
		}

		std::vector<AssetTypeIndex> assetTypeIndices;
		std::vector<ArchiveInfo> archives;
		std::vector<char> strings;
	};
}
