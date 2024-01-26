#pragma once

#include <filesystem>

#include "AssetLoader.hpp"

namespace Grindstone {
	namespace Assets {
		class FileAssetLoader : public AssetLoader {
		public:
			virtual void Load(AssetType assetType, std::filesystem::path path, char*& outContents, size_t& fileSize) override;
			virtual void Load(AssetType assetType, Uuid uuid, char*& outContents, size_t& fileSize) override;
			virtual bool LoadText(AssetType assetType, Uuid uuid, std::string& outContents) override;
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
