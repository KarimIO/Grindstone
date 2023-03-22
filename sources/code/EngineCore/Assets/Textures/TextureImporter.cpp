#include <filesystem>
#include "Common/Formats/Dds.hpp"
#include "Common/Graphics/Core.hpp"
#include "EngineCore/EngineCore.hpp"
#include "EngineCore/Utils/Utilities.hpp"
#include "EngineCore/Assets/AssetManager.hpp"
#include "TextureImporter.hpp"
using namespace Grindstone;

void* TextureImporter::ProcessLoadedFile(Uuid uuid) {
	char* fileContents;
	size_t fileSize;

	EngineCore& engineCore = EngineCore::GetInstance();
	if (!engineCore.assetManager->LoadFile(uuid, fileContents, fileSize)) {
		std::string errorMsg = "Unable to load texture: " + uuid.ToString();
		engineCore.Print(LogSeverity::Warning, errorMsg.c_str());
		return nullptr;
	}

	if (strncmp(fileContents, "DDS ", 4) != 0) {
		std::string errorMsg = "Invalid texture file: " + uuid.ToString();
		engineCore.Print(LogSeverity::Warning, errorMsg.c_str());
		return nullptr;
	}

	DDSHeader header;
	std::memcpy(&header, fileContents + 4, sizeof(header));

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

	const char* imgPtr = fileContents + 4 + sizeof(DDSHeader);
	Grindstone::GraphicsAPI::Texture::CreateInfo createInfo;
	createInfo.debugName = debugName.c_str();
	createInfo.data = imgPtr;
	createInfo.mipmaps = (uint16_t)header.dwMipMapCount;
	createInfo.format = format;
	createInfo.width = header.dwWidth;
	createInfo.height = header.dwHeight;
	createInfo.isCubemap = false;

	GraphicsAPI::Core* graphicsCore = engineCore.GetGraphicsCore();
	Grindstone::GraphicsAPI::Texture* texture = graphicsCore->CreateTexture(createInfo);

	auto asset = textures.emplace(uuid, TextureAsset(uuid, debugName, texture));
	return &asset.first->second;
}

bool TextureImporter::TryGetIfLoaded(Uuid uuid, void*& output) {
	auto& textureInMap = textures.find(uuid);
	if (textureInMap != textures.end()) {
		output = &textureInMap->second;
		return true;
	}

	return false;
}
