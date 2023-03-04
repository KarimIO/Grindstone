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
			virtual bool LoadFile(Uuid uuid, char*& dataPtr, size_t& fileSize);
			virtual std::string& GetTypeName(AssetType assetType);

			template<typename T>
			T& GetAsset(Uuid uuid) {
				void* assetPtr = GetAsset(T::GetStaticType(), uuid);
				return *static_cast<T*>(assetPtr);
			}

			// TODO: Register these into a file, so we can refer to types by number, and
			// if there is a new type, we can change all assetTypes in meta files.
			virtual void RegisterAssetType(AssetType assetType, const char* typeName, AssetImporter* importer);
		private:
			AssetLoader* assetLoader = nullptr;
			std::vector<std::string> assetTypeNames;
			std::vector<AssetImporter*> assetTypeImporters;
		};
	}
}
