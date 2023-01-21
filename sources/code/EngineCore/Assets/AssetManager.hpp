#pragma once

#include <vector>
#include <string>
#include "Asset.hpp"
#include "AssetImporter.hpp"

namespace Grindstone {
	namespace Assets {
		class AssetLoader;

		class AssetManager {
		public:
			AssetManager();
			virtual void* GetAsset(AssetType assetType, Uuid uuid);
			bool LoadFile(Uuid uuid, char*& dataPtr, size_t& fileSize);
			std::string& GetTypeName(AssetType assetType);

			template<typename T>
			T GetAsset(Uuid uuid) {
				return *GetAsset(T::assetType, uuid);
			}

			// TODO: Register these into a file, so we can refer to types by number, and
			// if there is a new type, we can change all assetTypes in meta files.
			template <typename AssetT, typename AssetImporterT>
			void RegisterAssetType() {
				AssetT::assetType = (AssetType)assetTypeNames.size();
			}
		private:
			AssetLoader* assetLoader = nullptr;
			std::vector<std::string> assetTypeNames;
			std::vector<AssetImporter*> assetTypeImporters;
		};
	}
}
