#pragma once

#include <vector>
#include <string>
#include <mutex>

#include <Common/Buffer.hpp>
#include <EngineCore/Assets/Loaders/AssetLoader.hpp>
#include <EngineCore/Assets/AssetReference.hpp>

#include "Asset.hpp"
#include "AssetImporter.hpp"

namespace Grindstone::Assets {
	class AssetManager {
	public:
		AssetManager(AssetLoader* assetLoader);
		~AssetManager();

		void ReloadQueuedAssets();
		virtual AssetImporter* GetManager(AssetType assetType);

		template<typename AssetImporterClass>
		AssetImporterClass* GetManager() {
			static_assert(std::is_base_of_v<Grindstone::AssetImporter, AssetImporterClass>, "AssetImporterClass not derived from Grindstone::AssetImporter");
			return static_cast<AssetImporterClass*>(GetManager(AssetImporterClass::GetStaticAssetType()));
		}

		virtual void QueueReloadAsset(AssetType assetType, Uuid uuid);
		virtual void* GetAssetByUuid(AssetType assetType, Uuid uuid);
		virtual Grindstone::Uuid GetUuidByAddress(AssetType assetType, std::string_view address);

		virtual AssetLoadBinaryResult LoadBinaryByUuid(AssetType assetType, Uuid uuid);
		virtual AssetLoadTextResult LoadTextByUuid(AssetType assetType, Uuid uuid);
		virtual const std::string& GetTypeName(AssetType assetType) const;

		virtual void* GetAndIncrementAssetCount(Grindstone::AssetType assetType, Grindstone::Uuid uuid);
		virtual void IncrementAssetCount(Grindstone::AssetType assetType, Grindstone::Uuid uuid);
		virtual void DecrementAssetCount(Grindstone::AssetType assetType, Grindstone::Uuid uuid);

		template<typename AssetImporterClass>
		void RegisterAssetType() {
			static_assert(std::is_base_of_v<Grindstone::AssetImporter, AssetImporterClass>, "AssetImporterClass not derived from Grindstone::AssetImporter");
			RegisterAssetType(AssetImporterClass::GetStaticAssetType(), AssetImporterClass::GetStaticAssetTypeName(), AllocatorCore::Allocate<AssetImporterClass>());
		}

		template<typename T>
		Grindstone::AssetReference<T> GetAssetReferenceByUuid(Grindstone::Uuid uuid) {
			static_assert(std::is_base_of_v<Grindstone::Asset, T>, "T not derived from Grindstone::Asset");

			if (!uuid.IsValid()) {
				return Grindstone::AssetReference<T>();
			}

			const size_t assetTypeSizeT = static_cast<size_t>(T::GetStaticType());
			if (assetTypeSizeT < 1 || assetTypeSizeT >= assetTypeImporters.size()) {
				return Grindstone::AssetReference<T>();
			}

			AssetImporter* assetImporter = assetTypeImporters[assetTypeSizeT];

			return Grindstone::AssetReference<T>::CreateAndIncrement(uuid);
		}

		template<typename T>
		Grindstone::AssetReference<T> GetAssetReferenceByAddress(std::string_view address) {
			static_assert(std::is_base_of_v<Grindstone::Asset, T>, "T not derived from Grindstone::Asset");
			Grindstone::Uuid uuid = GetUuidByAddress(T::GetStaticType(), address);
			return GetAssetReferenceByUuid<T>(uuid);
		}

		template<typename T>
		T* GetAssetByUuid(Uuid uuid) {
			static_assert(std::is_base_of_v<Grindstone::Asset, T>, "T not derived from Grindstone::Asset");
			void* assetPtr = GetAssetByUuid(T::GetStaticType(), uuid);
			return static_cast<T*>(assetPtr);
		};

		virtual void RegisterAssetType(AssetType assetType, const char* typeName, AssetImporter* importer);
		virtual void UnregisterAssetType(AssetType assetType);
	private:

		bool ownsAssetLoader = false;
		Grindstone::Assets::AssetLoader* assetLoader = nullptr;
		std::vector<std::string> assetTypeNames;
		std::vector<AssetImporter*> assetTypeImporters;
		std::vector<std::pair<AssetType, Uuid>> queuedAssetReloads;
		std::mutex reloadMutex;
	};
}
