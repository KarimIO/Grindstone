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

	struct GenericAssetReference {
		Uuid uuid;
		void* asset = nullptr;
	};

	template<typename T>
	struct AssetReference : public GenericAssetReference {
		inline T& Get() {
			return *asset;
		}
	};

	#define DEFINE_ASSET_TYPE(name) \
		static std::string GetStaticTypeName() { return name; }\
		virtual std::string GetAssetTypeName() { return name; }\
		static AssetType assetType; /* Assigned from AssetImporter.hpp */ \
		static AssetType GetStaticType() { return assetType; } \
		virtual AssetType GetAssetType() const override { return GetStaticType(); } \
		static size_t GetStaticAssetTypeHash() { return std::hash<std::string>()(name); } \
		virtual size_t GetAssetTypeHash() { return GetStaticAssetTypeHash(); }
}
