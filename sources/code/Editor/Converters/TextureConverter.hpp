#pragma once

#include <string>

namespace Grindstone {
	namespace Converters {
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

		class TextureConverter {
		public:
			void Convert(const char* path);
		private:
			unsigned char CombinePixels(unsigned char* pixelSrc);
			unsigned char* CreateMip(unsigned char* pixel, int width, int height);
			void ExtractBlock(
				const unsigned char* inPtr,
				unsigned char* colorBlock
			);
			void ConvertBC123();
			void OutputDds(unsigned char* outPixels, int contentSize);
			int CalculateMipMapLevelCount(int width, int height);

			std::string path;
			Compression compression;
			unsigned char* sourcePixels;
			int texWidth, texHeight, texChannels;
		};

		void ImportTexture(const char* inputPath);
	}
}
