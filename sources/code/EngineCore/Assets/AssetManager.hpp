#pragma once

#include <vector>
#include <string>
#include "Asset.hpp"
#include "AssetImporter.hpp"

namespace Grindstone {
	class AssetManager {
	public:
		AssetManager();
		virtual void LoadFile(AssetType assetType, Uuid uuid);
		std::string& GetTypeName(AssetType assetType);

		template<typename T>
		void LoadFile(Uuid uuid) {
			LoadFile(T::assetType, uuid);
		}

		// TODO: Register these into a file, so we can refer to types by number, and
		// if there is a new type, we can change all assetTypes in meta files.
		template <typename AssetT, typename AssetImporterT>
		void RegisterAssetType() {
			AssetT::assetType = (AssetType)assetTypeNames.size();
		}
	private:
		std::vector<std::string> assetTypeNames;
		std::vector<AssetImporter*> assetTypeImporters;
	};
}
