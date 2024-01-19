#pragma once

#include <string>
#include <map>

#include <Common/ResourcePipeline/Uuid.hpp>
#include <Common/ResourcePipeline/AssetType.hpp>

namespace Grindstone {
	class AssetImporter {
	public:
		virtual void QueueReloadAsset(Uuid uuid) = 0;
		virtual void* ProcessLoadedFile(Uuid uuid) = 0;
		virtual bool TryGetIfLoaded(Uuid uuid, void*& output) = 0;
		virtual void* ProcessLoadedFile(const char* path) { return nullptr; };
		virtual bool TryGetIfLoaded(const char* path, void*& output) { return false; };
		virtual AssetType GetAssetType() { return assetType; }

	protected:

		AssetType assetType;

	};

	template<typename AssetStructType, AssetType internalAssetType>
	class SpecificAssetImporter : public AssetImporter {
	public:

		SpecificAssetImporter() { assetType = internalAssetType; }
		static AssetType GetStaticAssetType() { return internalAssetType; }

		virtual bool TryGetIfLoaded(Uuid uuid, void*& output) {
			auto assetInMap = assets.find(uuid);
			if (assetInMap != assets.end()) {
				output = &assetInMap->second;
				return true;
			}

			return false;
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
