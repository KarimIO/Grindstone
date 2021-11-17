#include <filesystem>
#include "Common/Formats/Dds.hpp"
#include "Common/Graphics/Core.hpp"
#include "EngineCore/EngineCore.hpp"
#include "EngineCore/Utils/Utilities.hpp"
#include "TextureManager.hpp"
using namespace Grindstone;

TextureAsset& TextureManager::LoadTexture(const char* path) {
	std::string fixedPath = path;
	Utils::FixStringSlashes(fixedPath);

	if (fixedPath.empty()) {
		return errorTexture;
	}

	try {
		TextureAsset* texture = nullptr;
		if (TryGetTexture(fixedPath, texture)) {
			return *texture;
		}

		return CreateNewTextureFromFile(fixedPath);
	}
	catch (std::runtime_error& e) {
		EngineCore::GetInstance().Print(LogSeverity::Info, e.what());
		return errorTexture;
	}
}

void TextureManager::ReloadTextureIfLoaded(const char* path) {
	std::string fixedPath = path;
	Utils::FixStringSlashes(fixedPath);

	TextureAsset* texture = nullptr;
	if (TryGetTexture(fixedPath, texture)) {
		LoadTextureFromFile(true, fixedPath, *texture);
	}
}

TextureAsset& Grindstone::TextureManager::GetDefaultTexture() {
	return errorTexture;
}

bool TextureManager::TryGetTexture(std::string& path, TextureAsset*& texture) {
	auto& textureInMap = textures.find(path);
	if (textureInMap != textures.end()) {
		texture = &textureInMap->second;
		return true;
	}

	return false;
}

void TextureManager::CreateFromDds(bool isReloading, TextureAsset& textureAsset, const char* fileName, const char* data, size_t fileSize) {
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
	createInfo.debugName = fileName;
	createInfo.data = imgPtr;
	createInfo.mipmaps = (uint16_t)header.dwMipMapCount;
	createInfo.format = format;
	createInfo.width = header.dwWidth;
	createInfo.height = header.dwHeight;
	createInfo.isCubemap = false;

	if (isReloading) {
		textureAsset.texture->RecreateTexture(createInfo);
	}
	else {
		GraphicsAPI::Core* graphicsCore = EngineCore::GetInstance().GetGraphicsCore();
		Grindstone::GraphicsAPI::Texture* texture = graphicsCore->CreateTexture(createInfo);
	
		textureAsset.texture = texture;
	}
}

void TextureManager::LoadTextureFromFile(bool isReloading, std::string& path, TextureAsset& textureAsset) {
	std::filesystem::path filePath = path;
	std::string& fileName = filePath.filename().string();

	if (!std::filesystem::exists(filePath)) {
		throw std::runtime_error("Failed to load texture!");
	}

	std::vector<char> file = Utils::LoadFile(path.c_str());
	CreateFromDds(isReloading, textureAsset, fileName.c_str(), file.data(), file.size());
}

TextureAsset& TextureManager::CreateNewTextureFromFile(std::string& path) {
	TextureAsset& textureAsset = textures[path];
	LoadTextureFromFile(false, path, textureAsset);

	return textureAsset;
}
