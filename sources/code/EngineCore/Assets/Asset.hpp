#pragma once

#include <string>
#include <Common/ResourcePipeline/Uuid.hpp>

namespace Grindstone {
	using AssetType = uint16_t;

	struct Asset {
		Asset(Uuid uuid, std::string_view name) : uuid(uuid), name(name) {}

		Uuid uuid;
		std::string name;
		size_t referenceCount = 0;

		static AssetType GetStaticType() { return 0; }
		virtual AssetType GetAssetType() const { return GetStaticType(); }

		virtual bool operator==(const Asset& other) const {
			return uuid == other.uuid;
		}

		virtual bool operator!=(const Asset& other) const {
			return !(*this == other);
		}
	};

	template<typename T>
	struct AssetReference {
		Uuid uuid;
		T* asset = nullptr;
	};

	#define DEFINE_ASSET_TYPE static AssetType assetType; /* Assigned from AssetImporter.hpp */ \
		static AssetType GetStaticType() { return assetType; } \
		virtual AssetType GetAssetType() const override { return GetStaticType(); }
}
