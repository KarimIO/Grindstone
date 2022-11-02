#include <filesystem>
#include "Common/Formats/Dds.hpp"
#include "Common/Graphics/Core.hpp"
#include "EngineCore/EngineCore.hpp"
#include "EngineCore/Utils/Utilities.hpp"
#include "TextureImporter.hpp"
using namespace Grindstone;

void* TextureImporter::ProcessLoadedFile(Uuid uuid, std::vector<char>& contents) {
	if (strncmp(contents.data(), "DDS ", 4) != 0) {
		throw std::runtime_error("Invalid DDS file: No magic 'DDS' keyword.");
	}

	DDSHeader header;
	std::memcpy(&header, contents.data() + 4, sizeof(header));

	bool hasAlpha = header.ddspf.dwFlags & DDPF_ALPHAPIXELS;

	Grindstone::GraphicsAPI::ColorFormat format;
	switch (header.ddspf.dwFourCC) {
	case FOURCC_DXT1:
		format = hasAlpha
			? Grindstone::GraphicsAPI::ColorFormat::RGBA_DXT1
			: Grindstone::GraphicsAPI::ColorFormat::RGB_DXT1;
		break;
	case FOURCC_DXT3:
		format = Grindstone::GraphicsAPI::ColorFormat::RGBA_DXT3;
		break;
	case FOURCC_DXT5:
		format = Grindstone::GraphicsAPI::ColorFormat::RGBA_DXT5;
		break;
	default:
		throw std::runtime_error("Invalid FourCC in texture");
	}

	std::string debugName = uuid.ToString();

	const char* imgPtr = contents.data() + 4 + sizeof(DDSHeader);
	Grindstone::GraphicsAPI::Texture::CreateInfo createInfo;
	createInfo.debugName = debugName.c_str();
	createInfo.data = imgPtr;
	createInfo.mipmaps = (uint16_t)header.dwMipMapCount;
	createInfo.format = format;
	createInfo.width = header.dwWidth;
	createInfo.height = header.dwHeight;
	createInfo.isCubemap = false;

	GraphicsAPI::Core* graphicsCore = EngineCore::GetInstance().GetGraphicsCore();
	Grindstone::GraphicsAPI::Texture* texture = graphicsCore->CreateTexture(createInfo);

	auto asset = textures.emplace(uuid, texture);
	return &asset.second;
}

bool TextureImporter::TryGetIfLoaded(Uuid uuid, void*& output) {
	auto& textureInMap = textures.find(uuid);
	if (textureInMap != textures.end()) {
		output = &textureInMap->second;
		return true;
	}

	return false;
}
