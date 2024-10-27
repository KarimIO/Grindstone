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
		virtual void* GetAsset(AssetType assetType, std::string_view address);
		virtual void* GetAsset(AssetType assetType, Uuid uuid);
		virtual void* IncrementAssetUse(AssetType assetType, Uuid uuid);
		virtual void DecrementAssetUse(AssetType assetType, Uuid uuid);
		virtual AssetLoadBinaryResult LoadBinaryByPath(AssetType assetType, const std::filesystem::path& path);
		virtual AssetLoadBinaryResult LoadBinaryByAddress(AssetType assetType, std::string_view address);
		virtual AssetLoadBinaryResult LoadBinaryByUuid(AssetType assetType, Uuid uuid);
		virtual AssetLoadTextResult LoadTextByPath(AssetType assetType, const std::filesystem::path& path);
		virtual AssetLoadTextResult LoadTextByAddress(AssetType assetType, std::string_view address);
		virtual AssetLoadTextResult LoadTextByUuid(AssetType assetType, Uuid uuid);
		virtual bool LoadShaderSetByUuid(
			Uuid uuid,
			uint8_t shaderStagesBitMask,
			size_t numShaderStages,
			std::vector<GraphicsAPI::GraphicsPipeline::CreateInfo::ShaderStageData>& shaderStageCreateInfos,
			std::vector<std::vector<char>>& fileData
		);
		virtual bool LoadShaderStageByUuid(
			Uuid uuid,
			GraphicsAPI::ShaderStage shaderStage,
			GraphicsAPI::GraphicsPipeline::CreateInfo::ShaderStageData& stageCreateInfo,
			std::vector<char>& fileData
		);
		virtual bool LoadShaderSetByAddress(
			std::string_view address,
			uint8_t shaderStagesBitMask,
			size_t numShaderStages,
			std::vector<GraphicsAPI::GraphicsPipeline::CreateInfo::ShaderStageData>& shaderStageCreateInfos,
			std::vector<std::vector<char>>& fileData
		);
		virtual bool LoadShaderStageByAddress(
			std::string_view address,
			GraphicsAPI::ShaderStage shaderStage,
			GraphicsAPI::GraphicsPipeline::CreateInfo::ShaderStageData& stageCreateInfo,
			std::vector<char>& fileData
		);
		virtual bool LoadShaderSetByPath(
			const std::filesystem::path& path,
			uint8_t shaderStagesBitMask,
			size_t numShaderStages,
			std::vector<GraphicsAPI::GraphicsPipeline::CreateInfo::ShaderStageData>& shaderStageCreateInfos,
			std::vector<std::vector<char>>& fileData
		);
		virtual bool LoadShaderStageByPath(
			const std::filesystem::path& path,
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
		T* GetAsset(std::string_view address) {
			void* assetPtr = GetAsset(T::GetStaticType(), address);
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
