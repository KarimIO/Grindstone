#pragma once

#include <vector>
#include <string>
#include "Asset.hpp"
#include "AssetImporter.hpp"

namespace Grindstone {
	class AssetManager {
	public:
		AssetManager();
		virtual void LoadFile(AssetType assetType, Uuid& uuid);
		virtual void LazyLoadFile(AssetType assetType, Uuid& uuid);
		std::string& GetTypeName(AssetType assetType);

		template <typename AssetType, typename AssetImporterType>
		void RegisterNewType() {
			T::assetType = assetTypeNames.size();
			assetTypeNames.emplace_back(typeid(AssetType).name());
			assetTypeImporters.emplace_back(new AssetImporterType());
		}
	private:
		std::vector<std::string> assetTypeNames;
		std::vector<AssetImporter*> assetTypeImporters;
	};
}
