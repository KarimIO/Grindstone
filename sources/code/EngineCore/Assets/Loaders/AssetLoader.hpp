#pragma once

#include <filesystem>
#include <vector>

#include <Common/Graphics/GraphicsPipeline.hpp>
#include <Common/ResourcePipeline/Uuid.hpp>
#include <Common/ResourcePipeline/AssetType.hpp>

namespace Grindstone::Assets {
	class AssetLoader {
	public:
		virtual void Load(AssetType assetType, std::filesystem::path path, std::string& assetName, char*& outContents, size_t& fileSize) = 0;
		virtual void Load(AssetType assetType, Uuid uuid, std::string& assetName, char*& outContents, size_t& fileSize) = 0;
		virtual bool LoadText(AssetType assetType, Uuid uuid, std::string& assetName, std::string& outContents) = 0;
		virtual bool LoadText(AssetType assetType, std::filesystem::path path, std::string& assetName, std::string& outContents) = 0;
		virtual bool LoadShaderStage(
			Uuid uuid,
			GraphicsAPI::ShaderStage shaderStage,
			GraphicsAPI::ShaderStageCreateInfo& shaderStageCreateInfo,
			std::vector<char>& fileData
		) = 0;
	};
}
