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
			unsigned char* CreateMip(unsigned char* pixel, int width, int height);
			void ExtractBlock(
				const unsigned char* inPtr,
				unsigned int width,
				unsigned char* colorBlock
			);
			void ConvertBC123();

			std::string path;
			Compression compression;
			unsigned char* sourcePixels;
			int texWidth, texHeight, texChannels;
		};

		void ImportTexture(const char* inputPath);
	}
}
