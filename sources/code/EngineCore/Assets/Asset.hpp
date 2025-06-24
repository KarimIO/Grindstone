#pragma once

#include <string>
#include <type_traits>

#include <Common/ResourcePipeline/Uuid.hpp>
#include <Common/ResourcePipeline/AssetType.hpp>
#include <EngineCore/EngineCore.hpp>

namespace Grindstone {
	enum class AssetLoadStatus {
		// This asset has not been loaded yet, or has been unloaded.
		Unloaded = 0,
		// This asset is in the process of being loaded.
		Loading,
		// This asset is ready for use.
		Ready,
		// This asset was previously loaded, and is now being loaded again.
		Reloading,
		// This asset failed to load because it was not found.
		Missing,
		// This asset failed to load due to an error in the file or the loading process.
		Failed
	};

	struct Asset {
		Asset() = default;
		Asset(Uuid uuid, std::string_view name) : uuid(uuid), name(name) {}

		Uuid uuid;
		std::string name;
		size_t referenceCount = 1;
		AssetLoadStatus assetLoadStatus = AssetLoadStatus::Unloaded;

		static AssetType GetStaticType() { return AssetType::Undefined; }
		virtual AssetType GetAssetType() const { return GetStaticType(); }

		virtual bool operator==(const Asset& other) const {
			return uuid == other.uuid;
		}

		virtual bool operator!=(const Asset& other) const {
			return !(*this == other);
		}
	};

	#define DEFINE_ASSET_TYPE(name, assetType) \
		static std::string GetStaticTypeName() { return name; }\
		virtual std::string GetAssetTypeName() { return name; }\
		static AssetType GetStaticType() { return assetType; }\
		virtual AssetType GetAssetType() const override { return assetType; }\
		static size_t GetStaticAssetTypeHash() { return std::hash<std::string>()(name); }\
		virtual size_t GetAssetTypeHash() { return GetStaticAssetTypeHash(); }
}
