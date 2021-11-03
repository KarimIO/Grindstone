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
			unsigned char CombinePixels(unsigned char* pixelSrc);
			unsigned char* CreateMip(unsigned char* pixel, int width, int height);
			void ExtractBlock(
				const unsigned char* inPtr,
				unsigned char* colorBlock
			);
			void ConvertBC123();
			void OutputDds(unsigned char* outPixels, int contentSize);
			int CalculateMipMapLevelCount(int width, int height);

			std::filesystem::path path;
			Compression compression = Compression::Uncompressed;
			unsigned char* sourcePixels = nullptr;
			int texWidth = 0, texHeight = 0, texChannels = 0;
		};

		void ImportTexture(std::filesystem::path& inputPath);
	}
}
