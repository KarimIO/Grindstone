#ifndef IMAGE_CONVERTER_HPP
#define IMAGE_CONVERTER_HPP

#include <string>

enum Compression {
    C_DETECT = 0,
	C_UNCOMPRESSED,
	C_BC1,
	C_BC2,
	C_BC3,
	C_BC4,
	C_BC5,
	C_BC6H,
	C_BC7
};

void ExtractBlock(const unsigned char* inPtr, unsigned int width, unsigned char* colorBlock);

void ConvertBC123(unsigned char ***pixels, bool is_cubemap, int width, int height, Compression compression, std::string path, bool generateMips);

bool ConvertTexture(std::string input, bool is_cubemap, std::string output, Compression compression = C_DETECT);

#endif