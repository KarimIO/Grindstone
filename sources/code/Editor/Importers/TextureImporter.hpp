#pragma once

#include <string>
#include "Common/ResourcePipeline/Uuid.hpp"
#include "Importer.hpp"

namespace Grindstone {
	namespace Importers {
		enum class Compression {
			Detect = 0,
			Uncompressed,
			BC1,
			BC3,
			BC4,
			BC5,
			BC6H,
			BC7
		};

		class TextureImporter : public Importer {
		public:
			void Import(std::filesystem::path& path) override;
			Uuid GetUuid();
		private:
			Uuid uuid;
			void GenerateFaceBC123(uint32_t minMipLevel, uint32_t faceIterator, uint8_t* outData);
			void GenerateFaceBC4(uint32_t minMipLevel, uint32_t faceIterator, uint8_t* outData);
			uint8_t CombinePixels(uint8_t* pixelSrc, uint64_t index, uint64_t pitch);
			uint8_t* CreateMip(uint8_t* pixel, uint32_t width, uint32_t height);
			void ExtractBlock(
				const uint8_t* inPtr,
				const uint32_t levelWidth,
				uint8_t* colorBlock
			);
			void ConvertBC123();
			void ConvertBC4();
			void GenerateMipList(uint32_t minMipLevel, std::vector<uint8_t*>& uncompressedMips);
			void OutputDds(uint8_t* outPixels, uint32_t contentSize);
			uint32_t CalculateMipMapLevelCount(uint32_t width, uint32_t height);

			std::filesystem::path path;
			Compression compression = Compression::Uncompressed;
			uint8_t* sourcePixels = nullptr;
			uint32_t texWidth = 0;
			uint32_t texHeight = 0;
			uint32_t texChannels = 0;
			uint32_t targetTexChannels = 0;
		};

		void ImportTexture(std::filesystem::path& inputPath);
	}
}
