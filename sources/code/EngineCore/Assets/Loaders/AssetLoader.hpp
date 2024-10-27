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

	struct AssetLoadBinaryResult {
		AssetLoadStatus status;
		std::string displayName;
		Buffer buffer;
	};

	struct AssetLoadTextResult {
		AssetLoadStatus status;
		std::string displayName;
		std::string content;
	};

	class AssetLoader {
	public:
		virtual AssetLoadBinaryResult LoadBinaryByPath(AssetType assetType, const std::filesystem::path& path) = 0;
		virtual AssetLoadBinaryResult LoadBinaryByAddress(AssetType assetType, std::string_view address) = 0;
		virtual AssetLoadBinaryResult LoadBinaryByUuid(AssetType assetType, Uuid uuid) = 0;

		virtual AssetLoadTextResult LoadTextByPath(AssetType assetType, const std::filesystem::path& path) = 0;
		virtual AssetLoadTextResult LoadTextByAddress(AssetType assetType, std::string_view address) = 0;
		virtual AssetLoadTextResult LoadTextByUuid(AssetType assetType, Uuid uuid) = 0;

		virtual bool LoadShaderStageByPath(
			const std::filesystem::path& path,
			GraphicsAPI::ShaderStage shaderStage,
			GraphicsAPI::GraphicsPipeline::CreateInfo::ShaderStageData& shaderStageCreateInfo,
			std::vector<char>& fileData
		) = 0;
		virtual bool LoadShaderStageByAddress(
			std::string_view address,
			GraphicsAPI::ShaderStage shaderStage,
			GraphicsAPI::GraphicsPipeline::CreateInfo::ShaderStageData& shaderStageCreateInfo,
			std::vector<char>& fileData
		) = 0;
		virtual bool LoadShaderStageByUuid(
			Uuid uuid,
			GraphicsAPI::ShaderStage shaderStage,
			GraphicsAPI::GraphicsPipeline::CreateInfo::ShaderStageData& shaderStageCreateInfo,
			std::vector<char>& fileData
		) = 0;
	};
}
