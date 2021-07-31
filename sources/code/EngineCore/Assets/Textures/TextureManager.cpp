#include <filesystem>
#include "Common/Formats/Dds.hpp"
#include "Common/Graphics/Core.hpp"
#include "EngineCore/EngineCore.hpp"
#include "EngineCore/Utils/Utilities.hpp"
#include "TextureManager.hpp"
using namespace Grindstone;

TextureAsset& TextureManager::LoadTexture(const char* path) {
	TextureAsset* texture = nullptr;
	if (TryGetTexture(path, texture)) {
		return *texture;
	}

	return CreateTextureFromFile(path);
}

bool TextureManager::TryGetTexture(const char* path, TextureAsset*& texture) {
	auto& textureInMap = textures.find(path);
	if (textureInMap != textures.end()) {
		texture = &textureInMap->second;
		return true;
	}

	return false;
}

TextureAsset TextureManager::CreateFromDds(const char* data, size_t fileSize) {
	if (strncmp(data, "DDS ", 4) != 0) {
		throw std::runtime_error("Invalid DDS file: No magic 'DDS' keyword.");
	}

	DDSHeader header;
	std::memcpy(&header, data + 4, sizeof(header));

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

	const char* imgPtr = data + 4 + sizeof(DDSHeader);
	Grindstone::GraphicsAPI::Texture::CreateInfo createInfo;
	createInfo.data = imgPtr;
	createInfo.mipmaps = (uint16_t)header.dwMipMapCount;
	createInfo.format = format;
	createInfo.width = header.dwWidth;
	createInfo.height = header.dwHeight;
	createInfo.isCubemap = false;

	GraphicsAPI::Core* graphicsCore = EngineCore::GetInstance().getGraphicsCore();
	Grindstone::GraphicsAPI::Texture* texture = graphicsCore->CreateTexture(createInfo);
	
	return TextureAsset{texture};
}

TextureAsset& TextureManager::CreateTextureFromFile(const char* path) {
	if (!std::filesystem::exists(path)) {
		throw std::runtime_error("Failed to load texture!");
	}

	std::vector<char> file = Utils::LoadFile(path);
	textures[path] = CreateFromDds(file.data(), file.size());

	return textures[path];
}
