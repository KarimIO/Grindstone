#pragma once

#include <filesystem>
#include <vector>

#include <Common/Buffer.hpp>
#include <Common/Graphics/GraphicsPipeline.hpp>
#include <Common/ResourcePipeline/Uuid.hpp>
#include <Common/ResourcePipeline/AssetType.hpp>

namespace Grindstone::Assets {
	enum class AssetLoadStatus {
		Success,
		FileNotFound,
		NotEnoughMemory,
		InvalidAssetType,
		AssetNotInRegistry,
	};

	struct AssetLoadResult {
		AssetLoadStatus status;
		Buffer buffer;
	};

	class AssetLoader {
	public:
		virtual AssetLoadResult Load(AssetType assetType, std::filesystem::path path, std::string& assetName) = 0;
		virtual AssetLoadResult Load(AssetType assetType, Uuid uuid, std::string& assetName) = 0;
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
