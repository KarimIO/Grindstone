#pragma once

#include <vector>
#include <string>
#include <mutex>

#include "Common/Graphics/GraphicsPipeline.hpp"

#include "Asset.hpp"
#include "AssetImporter.hpp"

namespace Grindstone::Assets {
	class AssetLoader;

	class AssetManager {
	public:
		AssetManager();
		void ReloadQueuedAssets();
		virtual void QueueReloadAsset(AssetType assetType, Uuid uuid);
		virtual void* GetAsset(AssetType assetType, const char* path);
		virtual void* GetAsset(AssetType assetType, Uuid uuid);
		virtual bool LoadFile(const char* path, char*& dataPtr, size_t& fileSize);
		virtual bool LoadFile(Uuid uuid, char*& dataPtr, size_t& fileSize);
		virtual bool LoadFileText(Uuid uuid, std::string& dataPtr);
		virtual bool LoadShaderSet(
			Uuid uuid,
			uint8_t shaderStagesBitMask,
			size_t numShaderStages,
			std::vector<GraphicsAPI::ShaderStageCreateInfo>& shaderStageCreateInfos,
			std::vector<std::vector<char>>& fileData
		);
		virtual bool LoadShaderStage(
			Uuid uuid,
			GraphicsAPI::ShaderStage shaderStage,
			GraphicsAPI::ShaderStageCreateInfo& stageCreateInfo,
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

		// TODO: Register these into a file, so we can refer to types by number, and
		// if there is a new type, we can change all assetTypes in meta files.
		virtual void RegisterAssetType(AssetType assetType, const char* typeName, AssetImporter* importer);
	private:
		AssetLoader* assetLoader = nullptr;
		std::vector<std::string> assetTypeNames;
		std::vector<AssetImporter*> assetTypeImporters;
		std::vector<std::pair<AssetType, Uuid>> queuedAssetReloads;
		std::mutex reloadMutex;
	};
}
