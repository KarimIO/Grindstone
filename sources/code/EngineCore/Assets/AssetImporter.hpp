#pragma once

#include <string>
#include <map>

#include <Common/ResourcePipeline/Uuid.hpp>
#include <Common/ResourcePipeline/AssetType.hpp>

namespace Grindstone {
	class AssetImporter {
	public:
		virtual ~AssetImporter() {};

		virtual void QueueReloadAsset(Uuid uuid) = 0;
		virtual void* LoadAsset(Uuid uuid) = 0;
		virtual bool TryGetIfLoaded(Uuid uuid, void*& output) = 0;
		virtual void* IncrementAssetUse(Uuid uuid) = 0;
		virtual void DecrementAssetUse(Uuid uuid) = 0;
		virtual void IncrementOrLoad(Uuid uuid) = 0;
		virtual AssetType GetAssetType() { return assetType; }

	protected:
		AssetType assetType = AssetType::Undefined;

	};

	template<typename AssetStructType, AssetType internalAssetType>
	class SpecificAssetImporter : public AssetImporter {
	public:

		SpecificAssetImporter() { assetType = internalAssetType; }
		static AssetType GetStaticAssetType() { return internalAssetType; }
		static const char* GetStaticAssetTypeName() { return GetAssetTypeToString(internalAssetType); }

		virtual void* IncrementAssetUse(Uuid uuid) override {
			void* output = nullptr;
			if (TryGetIfLoaded(uuid, output)) {
				AssetStructType* asset = static_cast<AssetStructType*>(output);
				++asset->referenceCount;
				return asset;
			}
			else {
				return LoadAsset(uuid);
			}

			return nullptr;
		}

		virtual void DecrementAssetUse(Uuid uuid) override {
			void* output = nullptr;
			if (TryGetIfLoaded(uuid, output) && output != nullptr) {
				AssetStructType* asset = static_cast<AssetStructType*>(output);
				if (asset->referenceCount <= 1) {
					assets.erase(uuid);
				}
				else {
					--asset->referenceCount;
				}
			}
		}

		virtual bool TryGetIfLoaded(Uuid uuid, void*& output) override {
			auto& assetInMap = assets.find(uuid);
			if (assetInMap != assets.end()) {
				output = &assetInMap->second;
				return true;
			}

			return false;
		}

		virtual void IncrementOrLoad(Uuid uuid) override {
			auto& assetInMap = assets.find(uuid);
			if (assetInMap != assets.end()) {
				++assetInMap->second.referenceCount;
			}

			LoadAsset(uuid);
		}

		size_t AssetCount() const { return assets.size(); }
		bool HasAssets() const { return assets.size() != 0; }

		auto begin() noexcept { return assets.begin(); }
		auto cbegin() const noexcept { return assets.begin(); }
		auto end() noexcept { return assets.end(); }
		auto cend() const noexcept { return assets.cend(); }

	protected:
		std::map<Uuid, AssetStructType> assets;

	};
}
