#pragma once

#include <filesystem>

#include <Common/Buffer.hpp>
#include <Common/Assets/ArchiveDirectory.hpp>

#include "AssetLoader.hpp"

namespace Grindstone::Assets {
	class ArchiveAssetLoader : public AssetLoader {
	public:
		ArchiveAssetLoader();
		void InitializeDirectory();
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
			const GraphicsAPI::ShaderStage shaderStage,
			GraphicsAPI::GraphicsPipeline::CreateInfo::ShaderStageData& stageCreateInfo,
			std::vector<char>& fileData
		) override;
		virtual bool LoadShaderStageByUuid(
			Uuid uuid,
			GraphicsAPI::ShaderStage shaderStage,
			GraphicsAPI::GraphicsPipeline::CreateInfo::ShaderStageData& shaderStageCreateInfo,
			std::vector<char>& fileData
		) override;
	protected:
		AssetLoadBinaryResult LoadAsset(const ArchiveDirectory::AssetInfo& assetInfo);
		ArchiveDirectory archiveDirectory;

		Buffer lastBuffer;
		uint16_t lastBufferIndex = UINT16_MAX;
	};
}
