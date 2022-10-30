#pragma once

#include <string>
#include <Common/ResourcePipeline/Uuid.hpp>

namespace Grindstone {
	using AssetType = uint16_t;

	struct Asset {
		Uuid uuid;
		std::string name;

		static AssetType GetStaticType() { return 0; }
		virtual AssetType GetAssetType() const { return GetStaticType(); }

		virtual bool operator==(const Asset& other) const {
			return uuid == other.uuid;
		}

		virtual bool operator!=(const Asset& other) const {
			return !(*this == other);
		}
	};

	#define DEFINE_ASSET_TYPE static AssetType assetType; /* Assigned from AssetImporter.hpp */ \
		static AssetType GetStaticAssetType() { return assetType; } \
		virtual AssetType GetAssetType() const override { return GetStaticType(); }
}
