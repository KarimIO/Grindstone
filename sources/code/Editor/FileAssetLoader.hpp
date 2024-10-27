#pragma once

#include <filesystem>

#include <EngineCore/Assets/Loaders/AssetLoader.hpp>

namespace Grindstone::Assets {
	class FileAssetLoader : public AssetLoader {
	public:
		virtual AssetLoadBinaryResult LoadBinaryByPath(AssetType assetType, const std::filesystem::path& path) override;
		virtual AssetLoadBinaryResult LoadBinaryByAddress(AssetType assetType, std::string_view address) override;
		virtual AssetLoadBinaryResult LoadBinaryByUuid(AssetType assetType, Uuid uuid) override;

		virtual AssetLoadTextResult LoadTextByPath(AssetType assetType, const std::filesystem::path& path) override;
		virtual AssetLoadTextResult LoadTextByAddress(AssetType assetType, std::string_view address) override;
		virtual AssetLoadTextResult LoadTextByUuid(AssetType assetType, Uuid uuid) override;

		virtual bool LoadShaderStageByPath(
			const std::filesystem::path& path,
			GraphicsAPI::ShaderStage shaderStage,
			GraphicsAPI::GraphicsPipeline::CreateInfo::ShaderStageData& shaderStageCreateInfo,
			std::vector<char>& fileData
		) override;
		virtual bool LoadShaderStageByAddress(
			std::string_view address,
			GraphicsAPI::ShaderStage shaderStage,
			GraphicsAPI::GraphicsPipeline::CreateInfo::ShaderStageData& shaderStageCreateInfo,
			std::vector<char>& fileData
		) override;
		virtual bool LoadShaderStageByUuid(
			Uuid uuid,
			GraphicsAPI::ShaderStage shaderStage,
			GraphicsAPI::GraphicsPipeline::CreateInfo::ShaderStageData& shaderStageCreateInfo,
			std::vector<char>& fileData
		) override;
	};
}
