#pragma once

#include <string>
#include <Common/ResourcePipeline/Uuid.hpp>
#include <Common/Editor/Importer.hpp>

#include "OutputFormats.hpp"

namespace Grindstone::Editor::Importers {
	class TextureImporter : public Importer {
	public:
		void Import(Grindstone::Editor::AssetRegistry& assetRegistry, Grindstone::Assets::AssetManager& assetManager, const std::filesystem::path& path) override;
		Uuid GetUuid() const;
	private:
		Uuid uuid;
		void GenerateFace(uint32_t minMipLevel, uint32_t faceIterator, uint32_t blockSize, uint8_t* outData);
		void Convert();
		uint8_t* ExtractFirstFace(uint8_t faceIndex);
		void GenerateMipList(uint8_t faceIndex, uint32_t minMipLevel, std::vector<uint8_t*>& uncompressedMips);
		void OutputDds(uint8_t* outPixels, uint64_t contentSize);
		uint32_t CalculateMipMapLevelCount(uint32_t width, uint32_t height);

		std::filesystem::path path;
		Grindstone::Assets::AssetManager* assetManager = nullptr;
		OutputFormat outputFormat = OutputFormat::Undefined;
		SourceChannelDataType sourceChannelDataType = SourceChannelDataType::Undefined;
		AssetRegistry* assetRegistry = nullptr;
		uint8_t* sourcePixels = nullptr;
		uint32_t sourceWidth = 0;
		uint32_t sourceHeight = 0;
		uint32_t texWidth = 0;
		uint32_t texHeight = 0;
		uint32_t texChannels = 0;
		uint32_t targetTexChannels = 0;
		uint8_t sourceSizePerPixel = 1;
		bool shouldGenerateMips = true;
		bool isSixInOneImageCubemap = false;
	};

	void ImportTexture(Grindstone::Editor::AssetRegistry& assetRegistry, Grindstone::Assets::AssetManager& assetManger, const std::filesystem::path& inputPath);
}
