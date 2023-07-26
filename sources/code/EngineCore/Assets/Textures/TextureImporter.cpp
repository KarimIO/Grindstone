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

	auto asset = textures.emplace(uuid, TextureAsset());
	return ProcessLoadedFile(uuid, fileContents, fileSize, asset.first->second);
}

void* TextureImporter::ProcessLoadedFile(const char* path) {
	char* fileContents;
	size_t fileSize;

	EngineCore& engineCore = EngineCore::GetInstance();
	if (!engineCore.assetManager->LoadFile(path, fileContents, fileSize)) {
		std::string errorMsg = std::string("Unable to load texture: ") + path;
		engineCore.Print(LogSeverity::Warning, errorMsg.c_str());
		return nullptr;
	}

	auto asset = texturesByPath.emplace(path, TextureAsset());
	return ProcessLoadedFile(Uuid(), fileContents, fileSize, asset.first->second);
}

void* TextureImporter::ProcessLoadedFile(Uuid uuid, const char* fileContents, size_t fileSize, TextureAsset& textureAsset) {
	EngineCore& engineCore = EngineCore::GetInstance();

	if (strncmp(fileContents, "DDS ", 4) != 0) {
		std::string errorMsg = "Invalid texture file: " + uuid.ToString();
		engineCore.Print(LogSeverity::Warning, errorMsg.c_str());
		return nullptr;
	}

	DDSHeader header;
	std::memcpy(&header, fileContents + 4, sizeof(header));

	bool hasAlpha = header.ddspf.dwFlags & DDPF_ALPHAPIXELS;

	bool useDxgi = false;
	Grindstone::GraphicsAPI::ColorFormat format = GraphicsAPI::ColorFormat::Invalid;
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
	case FOURCC_BC4:
		format = Grindstone::GraphicsAPI::ColorFormat::BC4;
		break;
	case FOURCC_DXGI:
		useDxgi = true;
		break;
	default:
		throw std::runtime_error("Invalid FourCC in texture");
	}

	const char* imgPtr = fileContents + 4 + sizeof(DDSHeader);
	if (useDxgi) {
		DDSHeaderExtended extendedHeader;
		std::memcpy(&extendedHeader, imgPtr, sizeof(DDSHeaderExtended));
		if (extendedHeader.dxgiFormat == DxgiFormat::DXGI_FORMAT_BC6H_TYPELESS || extendedHeader.dxgiFormat == DxgiFormat::DXGI_FORMAT_BC6H_UF16 || extendedHeader.dxgiFormat == DxgiFormat::DXGI_FORMAT_BC6H_SF16) {
			format = Grindstone::GraphicsAPI::ColorFormat::BC6H;
		}
		else {
			throw std::runtime_error("Invalid extended DXGI format!");
		}

		imgPtr += sizeof(DDSHeaderExtended);
	}

	std::string debugName = uuid.ToString();

	Grindstone::GraphicsAPI::Texture::CreateInfo createInfo;
	createInfo.debugName = debugName.c_str();
	createInfo.data = imgPtr;
	createInfo.size = header.dwSize;
	createInfo.mipmaps = static_cast<uint16_t>(header.dwMipMapCount);
	createInfo.format = format;
	createInfo.width = header.dwWidth;
	createInfo.height = header.dwHeight;
	createInfo.isCubemap = (header.dwCaps2 & DDS_CUBEMAP_ALLFACES);
	createInfo.options.shouldGenerateMipmaps = false;
	if (createInfo.isCubemap) {
		createInfo.options.wrapModeU =
			createInfo.options.wrapModeV =
			createInfo.options.wrapModeW =
			TextureWrapMode::ClampToEdge;
	}

	GraphicsAPI::Core* graphicsCore = engineCore.GetGraphicsCore();
	Grindstone::GraphicsAPI::Texture* texture = graphicsCore->CreateTexture(createInfo);

	textureAsset = TextureAsset(uuid, debugName, texture);
	return &textureAsset;
}

bool TextureImporter::TryGetIfLoaded(Uuid uuid, void*& output) {
	auto& textureInMap = textures.find(uuid);
	if (textureInMap != textures.end()) {
		output = &textureInMap->second;
		return true;
	}

	return false;
}

bool TextureImporter::TryGetIfLoaded(const char* path, void*& output) {
	auto& textureInMap = texturesByPath.find(path);
	if (textureInMap != texturesByPath.end()) {
		output = &textureInMap->second;
		return true;
	}

	return false;
}

void TextureImporter::ReloadAsset(Uuid uuid) {
	auto& textureInMap = textures.find(uuid);
	if (textureInMap == textures.end()) {
		return;
	}

	EngineCore& engineCore = EngineCore::GetInstance();
	GraphicsAPI::Core* graphicsCore = engineCore.GetGraphicsCore();

	Texture*& texture = textureInMap->second.texture;
	if (textureInMap->second.texture != nullptr) {
		graphicsCore->DeleteTexture(texture);
		texture = nullptr;
	}

	char* fileContents;
	size_t fileSize;

	if (!engineCore.assetManager->LoadFile(uuid, fileContents, fileSize)) {
		std::string errorMsg = "Unable to load texture: " + uuid.ToString();
		engineCore.Print(LogSeverity::Warning, errorMsg.c_str());
		return;
	}

	ProcessLoadedFile(uuid, fileContents, fileSize, textureInMap->second);
}
