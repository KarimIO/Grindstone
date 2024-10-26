#pragma once

#include <vector>
#include <string>
#include <mutex>

#include <Common/Graphics/GraphicsPipeline.hpp>
#include <Common/Buffer.hpp>
#include <EngineCore/Assets/Loaders/AssetLoader.hpp>

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
			return static_cast<AssetImporterClass*>(GetManager(AssetImporterClass::GetStaticAssetType()));
		}

		virtual void QueueReloadAsset(AssetType assetType, Uuid uuid);
		virtual void* GetAsset(AssetType assetType, const char* path);
		virtual void* GetAsset(AssetType assetType, Uuid uuid);
		virtual void* IncrementAssetUse(AssetType assetType, Uuid uuid);
		virtual void DecrementAssetUse(AssetType assetType, Uuid uuid);
		virtual AssetLoadResult LoadFile(AssetType assetType, const char* path, std::string& assetName);
		virtual AssetLoadResult LoadFile(AssetType assetType, Uuid uuid, std::string& assetName);
		virtual bool LoadFileText(AssetType assetType, Uuid uuid, std::string& assetName, std::string& dataPtr);
		virtual bool LoadFileText(AssetType assetType, std::filesystem::path path, std::string& assetName, std::string& dataPtr);
		virtual bool LoadShaderSet(
			Uuid uuid,
			uint8_t shaderStagesBitMask,
			size_t numShaderStages,
			std::vector<GraphicsAPI::GraphicsPipeline::CreateInfo::ShaderStageData>& shaderStageCreateInfos,
			std::vector<std::vector<char>>& fileData
		);
		virtual bool LoadShaderStage(
			Uuid uuid,
			GraphicsAPI::ShaderStage shaderStage,
			GraphicsAPI::GraphicsPipeline::CreateInfo::ShaderStageData& stageCreateInfo,
			std::vector<char>& fileData
		);
		virtual std::string& GetTypeName(AssetType assetType);

		template<typename T>
		T* GetAsset(Uuid uuid) {
			void* assetPtr = GetAsset(T::GetStaticType(), uuid);
			return static_cast<T*>(assetPtr);
		};

		template<typename T>
		T* GetAsset(Grindstone::AssetReference<T> assetReference) {
			void* assetPtr = GetAsset(T::GetStaticType(), assetReference.uuid);
			return static_cast<T*>(assetPtr);
		};

		template<typename T>
		T* IncrementAssetUse(Uuid uuid) {
			void* assetPtr = IncrementAssetUse(T::GetStaticType(), uuid);
			return static_cast<T*>(assetPtr);
		};

		// TODO: Register these into a file, so we can refer to types by number, and
		// if there is a new type, we can change all assetTypes in meta files.
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
