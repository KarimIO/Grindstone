#ifndef IMAGE_CONVERTER_HPP
#define IMAGE_CONVERTER_HPP

#include <string>

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

void ExtractBlock(const unsigned char* inPtr, unsigned int width, unsigned char* colorBlock);

void ConvertBC123(unsigned char ***pixels, bool is_cubemap, int width, int height, Compression compression, std::string path, bool generateMips);

bool ConvertTexture(std::string input, bool is_cubemap, std::string output, Compression compression = Compression::Detect);

#endif