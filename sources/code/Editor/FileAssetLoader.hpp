#pragma once

#include <filesystem>

#include <EngineCore/Assets/Loaders/AssetLoader.hpp>

namespace Grindstone {
	namespace Assets {
		class FileAssetLoader : public AssetLoader {
		public:
			virtual void Load(AssetType assetType, std::filesystem::path path, std::string& assetName, char*& outContents, size_t& fileSize) override;
			virtual void Load(AssetType assetType, Uuid uuid, std::string& assetName, char*& outContents, size_t& fileSize) override;
			virtual bool LoadText(AssetType assetType, Uuid uuid, std::string& assetName, std::string& outContents) override;
			virtual bool LoadText(AssetType assetType, std::filesystem::path path, std::string& assetName, std::string& outContents) override;
			virtual bool LoadShaderStage(
				Uuid uuid,
				GraphicsAPI::ShaderStage shaderStage,
				GraphicsAPI::ShaderStageCreateInfo& shaderStageCreateInfo,
				std::vector<char>& fileData
			) override;
		private:
			std::string GetShaderPath(Uuid uuid, GraphicsAPI::ShaderStage shaderStage);
		};
	}
}
