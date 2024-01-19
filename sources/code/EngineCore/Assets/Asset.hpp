#pragma once

#include <string>
#include <type_traits>

#include <Common/ResourcePipeline/Uuid.hpp>
#include <Common/ResourcePipeline/AssetType.hpp>
#include <EngineCore/EngineCore.hpp>

namespace Grindstone {
	struct Asset {
		Asset() = default;
		Asset(Uuid uuid, std::string_view name) : uuid(uuid), name(name) {}

		Uuid uuid;
		std::string name;
		size_t referenceCount = 1;

		static AssetType GetStaticType() { return AssetType::Undefined; }
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
	};

	template<typename T>
	struct AssetReference : public GenericAssetReference {
		static_assert(std::is_base_of<Grindstone::Asset, T>::value, "T not derived from Grindstone::Asset");

		T* Load(Uuid uuid) {
			this->uuid = uuid;
			return EngineCore::GetInstance().assetManager->GetAsset<T>(uuid);
		}

		T* Get() const {
			return EngineCore::GetInstance().assetManager->GetAsset<T>(uuid);
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
