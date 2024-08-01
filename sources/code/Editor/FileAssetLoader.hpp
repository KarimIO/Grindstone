#pragma once

#include <filesystem>

#include <EngineCore/Assets/Loaders/AssetLoader.hpp>

namespace Grindstone::Assets {
	class FileAssetLoader : public AssetLoader {
	public:
		virtual AssetLoadResult Load(AssetType assetType, std::filesystem::path path, std::string& assetName) override;
		virtual AssetLoadResult Load(AssetType assetType, Uuid uuid, std::string& assetName) override;
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
