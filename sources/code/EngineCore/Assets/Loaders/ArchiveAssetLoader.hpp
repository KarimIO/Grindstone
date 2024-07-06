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
	protected:
		void LoadAsset(const ArchiveDirectory::AssetInfo& assetInfo, std::string& assetName, char*& outContents, size_t& fileSize);
		std::string GetShaderPath(Uuid uuid, GraphicsAPI::ShaderStage shaderStage);
		ArchiveDirectory archiveDirectory;

		Buffer lastBuffer;
		uint16_t lastBufferIndex = UINT16_MAX;
	};
}
