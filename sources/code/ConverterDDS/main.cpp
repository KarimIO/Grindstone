#include <fstream>
#include <string>
#include <iostream>

#define STB_IMAGE_IMPLEMENTATION
#define STB_DXT_IMPLEMENTATION
#include <stb/stb_image.h>
#include <stb/stb_dxt.h>

unsigned char *PadAlpha(unsigned char *in, int width, int height) {
	unsigned char *out = new unsigned char[width * height * 4];
    for (int i = 0; i < height; i++) {
        for (int j = 0; j < width; j++) {
            int ip = i * width + j;
            int op = ip * 4;
            ip *= 3;
            out[op + 0] = in[ip + 0];
            out[op + 1] = in[ip + 1];
            out[op + 2] = in[ip + 2];
            out[op + 3] = 0;
        }
    }
    return out;
}

typedef unsigned long DWORD;

struct DDS_PIXELFORMAT {
	DWORD dwSize = 32;
	DWORD dwFlags;
	DWORD dwFourCC;
	DWORD dwRGBBitCount;
	DWORD dwRBitMask;
	DWORD dwGBitMask;
	DWORD dwBBitMask;
	DWORD dwABitMask;
};

#define DDPF_ALPHAPIXELS 0x1
#define DDPF_FOURCC 0x4
#define DDPF_RGB 0x40

#define DDSD_CAPS			0x1
#define DDSD_HEIGHT			0x2
#define DDSD_WIDTH			0x4
#define DDSD_PITCH			0x8
#define DDSD_PIXELFORMAT	0x1000
#define DDSD_MIPMAPCOUNT	0x20000
#define DDSD_LINEARSIZE		0x80000
#define DDSD_DEPTH			0x800000
#define DDSD_REQUIRED		DDSD_CAPS | DDSD_HEIGHT | DDSD_WIDTH | DDSD_PIXELFORMAT | DDSD_LINEARSIZE;

struct DDSHeader {
	DWORD           dwSize = 124;
	DWORD           dwFlags;
	DWORD           dwHeight;
	DWORD           dwWidth;
	DWORD           dwPitchOrLinearSize;
	DWORD           dwDepth;
	DWORD           dwMipMapCount;
	DWORD           dwReserved1[11];
	DDS_PIXELFORMAT ddspf;
	DWORD           dwCaps;
	DWORD           dwCaps2;
	DWORD           dwCaps3;
	DWORD           dwCaps4;
	DWORD           dwReserved2;
} DDS_HEADER;

#define MAKEFOURCC(c0, c1, c2, c3)	((DWORD)(char)(c0) | ((DWORD)(char)(c1) << 8) | \
									((DWORD)(char)(c2) << 16) | ((DWORD)(char)(c3) << 24))
#define MAKEFOURCCS(str)			MAKEFOURCC(str[0], str[1], str[2], str[3])
#define FOURCC_DXT1 MAKEFOURCC('D', 'X', 'T', '1')
#define FOURCC_DXT3 MAKEFOURCC('D', 'X', 'T', '3')
#define FOURCC_DXT5 MAKEFOURCC('D', 'X', 'T', '5')
#define FOURCC_BC4 MAKEFOURCC('A', 'T', 'I', '1')
#define FOURCC_BC5 MAKEFOURCC('A', 'T', 'I', '2')

#define DDSCAPS_COMPLEX		0x8
#define DDSCAPS_MIPMAP		0x400000
#define DDSCAPS_TEXTURE		0x1000

enum Compression {
	C_UNCOMPRESSED = 0,
	C_BC1,
	C_BC2,
	C_BC3,
	C_BC4,
	C_BC5,
	C_BC6H,
	C_BC7
};

void ConvertBC123(unsigned char *pixels, int width, int height, int bcnlevel, std::string path) {
	// No Alpha
	DDS_PIXELFORMAT outFormat;
	outFormat.dwFlags = DDPF_FOURCC;
	/*outFormat.dwRGBBitCount;
	outFormat.dwRBitMask;
	outFormat.dwGBitMask;
	outFormat.dwBBitMask;
	outFormat.dwABitMask;*/

	DDSHeader outHeader;
	outHeader.dwFlags = DDSD_REQUIRED;
	outHeader.dwHeight = height;
	outHeader.dwWidth = width;
	outHeader.dwDepth = 1;
	outHeader.dwCaps = DDSCAPS_TEXTURE;
	outHeader.dwMipMapCount = 1;
	bool alpha = false;
	switch (bcnlevel) {
	case 1:
		outHeader.dwPitchOrLinearSize = width * height / 2;
		outFormat.dwFourCC = FOURCC_DXT1;
		break;
	case 2:
		outHeader.dwPitchOrLinearSize = width * height;
		outFormat.dwFourCC = FOURCC_DXT3;
		break;
	case 3:
		outHeader.dwPitchOrLinearSize = width * height;
		outFormat.dwFourCC = FOURCC_DXT5;
		break;
	}
	outHeader.ddspf = outFormat;

	int wq = width / 4;
	int hq = height / 4;
	unsigned char *outData = new unsigned char[outHeader.dwPitchOrLinearSize];
	// TODO: Pad if non multiple of 4
	for (int i = 0; i < hq; i++) {
		for (int j = 0; j < wq; j++) {
			int pos = i * hq + j;
			stb_compress_dxt_block(outData + (pos * 8), pixels + (pos * 16 * 4), false, STB_DXT_NORMAL);
		}
	}

	std::ofstream out("out.dds", std::ios::binary);
	if (out.fail()) {
		std::cerr << "Failed to output to out.dds\n";
		return;
	}
	const char filecode[4] = { 'D', 'D', 'S', ' ' };
	out.write((const char *)&filecode, sizeof(char) * 4);
	out.write((const char *)&outHeader, sizeof(outHeader));
	out.write((const char *)outData, outHeader.dwPitchOrLinearSize);
	out.close();

	delete[] outData;
}


void ConvertBC45(unsigned char *pixels, int width, int height, int bcnlevel, std::string path) {
	// No Alpha
	DDS_PIXELFORMAT outFormat;
	outFormat.dwFlags = DDPF_FOURCC;
	/*outFormat.dwRGBBitCount;
	outFormat.dwRBitMask;
	outFormat.dwGBitMask;
	outFormat.dwBBitMask;
	outFormat.dwABitMask;*/

	DDSHeader outHeader;
	outHeader.dwFlags = DDSD_REQUIRED;
	outHeader.dwHeight = height;
	outHeader.dwWidth = width;
	outHeader.dwDepth = 1;
	outHeader.dwMipMapCount = 1;
	outHeader.dwCaps = DDSCAPS_TEXTURE;
	int wq = width / 4;
	int hq = height / 4;
	unsigned char *outData;

	switch (bcnlevel) {
	case 4:
		outHeader.dwPitchOrLinearSize = width * height / 2;
		outFormat.dwFourCC = FOURCC_BC4;
		outData = new unsigned char[outHeader.dwPitchOrLinearSize];

		for (int i = 0; i < hq; i++) {
			for (int j = 0; j < wq; j++) {
				int pos = i * hq + j;
				stb_compress_bc4_block(outData + (pos * 8), pixels + (pos * 16 * 1));
			}
		}
		break;
	case 5:
		outHeader.dwPitchOrLinearSize = width * height;
		outFormat.dwFourCC = FOURCC_BC5;
		outData = new unsigned char[outHeader.dwPitchOrLinearSize];

		for (int i = 0; i < hq; i++) {
			for (int j = 0; j < wq; j++) {
				int pos = i * hq + j;
				stb_compress_bc5_block(outData + (pos * 16), pixels + (pos * 16 * 2));
			}
		}
		break;
	}
	outHeader.ddspf = outFormat;

	std::ofstream out("out.dds", std::ios::binary);
	if (out.fail()) {
		std::cerr << "Failed to output to out.dds\n";
		return;
	}
	const char filecode[4] = { 'D', 'D', 'S', ' ' };
	out.write((const char *)&filecode, sizeof(char) * 4);
	out.write((const char *)&outHeader, sizeof(outHeader));
	out.write((const char *)outData, outHeader.dwPitchOrLinearSize);
	out.close();

	delete[] outData;
}

void ConvertBCn(unsigned char *pixels, int width, int height, int bcnlevel, std::string path) {
    switch (bcnlevel) {
        case 1:
        case 2:
        case 3:
            ConvertBC123(pixels, width, height, bcnlevel, path);
            break;
		case 4:
		case 5:
            ConvertBC45(pixels, width, height, bcnlevel, path);
            break;
        case 6:
            //ConvertBC6H(pixels, width, height, path);
            break;
        case 7:
            //ConvertBC7(pixels, width, height, path);
            break;
		default:
			std::cerr << "Invalid Conversion!\n";
			break;
    }
}

int main() {
	// TODO: Allow Providing multiple textures.
	//		Type detection is up to user, either by prefix, suffix,
	//		or checking the contents by format type. Then info can 
	//		be directly inputted for each item.
	
	std::string path = "normal.png";
    int texWidth, texHeight, texChannels;
    stbi_uc *pixels = stbi_load(path.c_str(), &texWidth, &texHeight, &texChannels, 2);
    if (!pixels) {
        printf("Texture failed to load!: %s \nPress enter to exit. ", path.c_str());
		std::cin.get();
        return NULL;
    }

	unsigned char *fixedPixels = pixels;
    /*if (texChannels == 3) {
        fixedPixels = PadAlpha(pixels, texWidth, texHeight);
        delete[] pixels;
    }*/

    std::string outpath = "out.dds";
    ConvertBCn(fixedPixels, texWidth, texHeight, C_BC5, outpath);
	delete[] fixedPixels;

	std::cout << "Conversion Complete. Press Enter to exit. ";
	std::cin.get();
}