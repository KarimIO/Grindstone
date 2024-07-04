#include <fstream>

#include <Common/Formats/Dds.hpp>
#include "DdsFileWriter.hpp"

void Grindstone::Editor::Importers::WriteDdsFile(const std::filesystem::path& outputPath, const std::vector<uint8_t>& fileContents, const DdsFileWriterOptions& options) {
	DDSHeader outHeader{};
	outHeader.dwSize = 124;
	outHeader.ddspf.dwFlags = DDPF_FOURCC;
	outHeader.dwFlags = DDSD_REQUIRED | DDSD_MIPMAPCOUNT;
	outHeader.dwWidth = options.texWidth;
	outHeader.dwHeight = options.texHeight;
	outHeader.dwDepth = options.texDepth;
	outHeader.dwCaps = DDSCAPS_TEXTURE | DDSCAPS_COMPLEX;
	outHeader.dwMipMapCount = options.mipmapCount;

	DDSHeaderExtended extendedHeader{};
	extendedHeader.miscFlag = 0;
	extendedHeader.arraySize = 1; // TODO: We should probably support texture arrays. Not sure how we'll do that in-editor though.
	extendedHeader.miscFlags2 = 0; // This must be zero to be compatible with D3DX utility libraries in other readers.

	if (options.mipmapCount > 1) {
		outHeader.dwCaps |= DDSCAPS_MIPMAP | DDSCAPS_COMPLEX;
	}

	if (options.isCubemap) {
		outHeader.dwCaps |= DDSCAPS_COMPLEX;
		outHeader.dwCaps2 = DDS_CUBEMAP_ALLFACES;
		extendedHeader.miscFlag = DDS_RESOURCE_MISC_TEXTURECUBE;
		extendedHeader.arraySize = 6;
	}

	if (options.texDepth >= 1) {
		extendedHeader.resourceDimension = D3d10ResourceDimension::D3D10_RESOURCE_DIMENSION_TEXTURE3D;
	}
	else if (options.texHeight >= 1) {
		extendedHeader.resourceDimension = D3d10ResourceDimension::D3D10_RESOURCE_DIMENSION_TEXTURE2D;
	}
	else {
		extendedHeader.resourceDimension = D3d10ResourceDimension::D3D10_RESOURCE_DIMENSION_TEXTURE1D;
	}

	switch (options.outputFormat) {
	default:
	case OutputFormat::BC1:
		outHeader.dwPitchOrLinearSize = options.texWidth * options.texHeight / 2;
		outHeader.ddspf.dwFourCC = FOURCC_DXT1;
		extendedHeader.dxgiFormat = DxgiFormat::DXGI_FORMAT_BC1_UNORM;
		break;
	case OutputFormat::BC3:
		outHeader.dwPitchOrLinearSize = options.texWidth * options.texHeight;
		outHeader.ddspf.dwFourCC = FOURCC_DXT5;
		extendedHeader.dxgiFormat = DxgiFormat::DXGI_FORMAT_BC3_UNORM;
		break;
	case OutputFormat::BC4:
		outHeader.dwPitchOrLinearSize = options.texWidth * options.texHeight / 2;
		outHeader.ddspf.dwFourCC = FOURCC_BC4;
		extendedHeader.dxgiFormat = DxgiFormat::DXGI_FORMAT_BC4_UNORM;
		break;
	case OutputFormat::BC6H:
		outHeader.dwPitchOrLinearSize = options.texWidth * options.texHeight;
		outHeader.ddspf.dwFourCC = FOURCC_DXGI;
		extendedHeader.dxgiFormat = DxgiFormat::DXGI_FORMAT_BC6H_UF16;
		break;
	}

	// We should use extended header if we have array textures.
	if (extendedHeader.arraySize > 1) {
		outHeader.ddspf.dwFourCC = FOURCC_DXGI;
	}

	char mark[] = { 'G', 'R', 'I', 'N', 'D', 'S', 'T', 'O', 'N', 'E' };
	std::memcpy(outHeader.dwReserved1, mark, sizeof(mark));

	std::filesystem::path parentPath = outputPath.parent_path();
	std::filesystem::create_directories(parentPath);

	std::ofstream out(outputPath, std::ios::binary);
	if (out.fail()) {
		throw std::runtime_error("Failed to output texture!");
	}

	const char filecode[4] = { 'D', 'D', 'S', ' ' };
	out.write(filecode, sizeof(char) * 4);
	out.write(reinterpret_cast<const char*>(&outHeader), sizeof(outHeader));
	// Only output the extended header if using DXGI in format
	if (outHeader.ddspf.dwFourCC == FOURCC_DXGI) {
		out.write(reinterpret_cast<const char*>(&outHeader), sizeof(outHeader));
	}
	out.write(reinterpret_cast<const char*>(fileContents.data()), fileContents.size());
	out.close();
}
