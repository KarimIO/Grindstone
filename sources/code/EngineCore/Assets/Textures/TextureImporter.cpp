#include <filesystem>
#include <string>
#include <string_view>

#include <Common/Formats/Dds.hpp>
#include <Common/Graphics/Core.hpp>
#include <Common/ResourcePipeline/Uuid.hpp>
#include <EngineCore/Assets/AssetManager.hpp>
#include <EngineCore/EngineCore.hpp>

#include "TextureImporter.hpp"
#include <EngineCore/Logger.hpp>

using namespace Grindstone;
using namespace Grindstone::Formats::DDS;

void* TextureImporter::ProcessLoadedFile(Uuid uuid) {
	EngineCore& engineCore = EngineCore::GetInstance();
	Assets::AssetLoadBinaryResult result = engineCore.assetManager->LoadBinaryByUuid(AssetType::Texture, uuid);
	if (result.status != Assets::AssetLoadStatus::Success) {
		GPRINT_ERROR_V(LogSource::EngineCore, "Could not find texture with id {}.", uuid.ToString());
		return nullptr;
	}

	auto asset = assets.emplace(uuid, TextureAsset());
	return ProcessLoadedFile(uuid, result.displayName, std::move(result.buffer), asset.first->second);
}

void* TextureImporter::ProcessLoadedFile(std::string_view address) {
	EngineCore& engineCore = EngineCore::GetInstance();
	Assets::AssetLoadBinaryResult result = engineCore.assetManager->LoadBinaryByAddress(AssetType::Texture, address);
	if (result.status != Assets::AssetLoadStatus::Success) {
		GPRINT_ERROR_V(LogSource::EngineCore, "Could not find texture with address {}.", address);
		return nullptr;
	}

	auto asset = texturesByAddress.emplace(address, TextureAsset());
	return ProcessLoadedFile(Uuid(), result.displayName, std::move(result.buffer), asset.first->second);
}

void* TextureImporter::ProcessLoadedFile(Uuid uuid, std::string& assetName, Buffer buffer, TextureAsset& textureAsset) {
	EngineCore& engineCore = EngineCore::GetInstance();

	const char* fileContents = reinterpret_cast<const char*>(buffer.Get());
	if (strncmp(fileContents, "DDS ", 4) != 0) {
		GPRINT_WARN_V(LogSource::EngineCore, "Invalid texture file: {}", uuid.ToString());
		return nullptr;
	}

	DDSHeader header;
	std::memcpy(&header, fileContents + 4, sizeof(header));

	bool hasAlpha = header.ddspf.dwFlags & DDPF_ALPHAPIXELS;

	bool useDxgi = false;
	Grindstone::GraphicsAPI::ColorFormat format = GraphicsAPI::ColorFormat::Invalid;

	bool isFourCC = (header.ddspf.dwFlags & DDPF_FOURCC) > 0;
	bool isRGB = (header.ddspf.dwFlags & DDPF_RGB) > 0;

	if (isFourCC) {
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

		case FOURCC_R16:
			format = Grindstone::GraphicsAPI::ColorFormat::R16;
			break;
		case FOURCC_RG16:
			format = Grindstone::GraphicsAPI::ColorFormat::RG16;
			break;
		case FOURCC_RGBA16:
			format = Grindstone::GraphicsAPI::ColorFormat::RGBA16;
			break;
		case FOURCC_R32:
			format = Grindstone::GraphicsAPI::ColorFormat::R32;
			break;
		case FOURCC_RG32:
			format = Grindstone::GraphicsAPI::ColorFormat::RG32;
			break;
		case FOURCC_RGBA32:
			format = Grindstone::GraphicsAPI::ColorFormat::RGBA32;
			break;
		case FOURCC_DXGI:
			useDxgi = true;
			break;
		default:
			throw std::runtime_error("Invalid FourCC in texture");
		}
	}
	else if (isRGB) {
		switch (header.ddspf.dwRGBBitCount) {
		case 24:
			format = Grindstone::GraphicsAPI::ColorFormat::RGB8;
			break;
		case 32:
			format = Grindstone::GraphicsAPI::ColorFormat::RGBA8;
			break;
		default:
			throw std::runtime_error("Invalid rgb pixel format in texture");
		}
	}
	else {
		throw std::runtime_error("Invalid pixel format in texture");
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

	Grindstone::GraphicsAPI::Texture::CreateInfo createInfo;
	createInfo.debugName = assetName.c_str();
	createInfo.data = imgPtr;
	createInfo.size = header.dwSize;
	createInfo.mipmaps = static_cast<uint16_t>(header.dwMipMapCount);
	createInfo.format = format;
	createInfo.width = header.dwWidth;
	createInfo.height = header.dwHeight;
	createInfo.isCubemap = (header.dwCaps2 & DDS_CUBEMAP_ALLFACES);
	createInfo.options.shouldGenerateMipmaps = false;
	createInfo.options.anistropy = 16.0f;
	createInfo.options.mipMax = -1000.0f;
	createInfo.options.mipMax = 1000.0f;
	createInfo.options.mipFilter = TextureFilter::Linear;
	createInfo.options.minFilter = TextureFilter::Linear;
	createInfo.options.magFilter = TextureFilter::Linear;

	if (createInfo.isCubemap) {
		createInfo.options.wrapModeU =
		createInfo.options.wrapModeV =
		createInfo.options.wrapModeW =
		TextureWrapMode::ClampToEdge;
	}

	GraphicsAPI::Core* graphicsCore = engineCore.GetGraphicsCore();
	Grindstone::GraphicsAPI::Texture* texture = graphicsCore->CreateTexture(createInfo);

	textureAsset = TextureAsset(uuid, assetName, texture);
	return &textureAsset;
}

bool TextureImporter::TryGetIfLoaded(std::string_view address, void*& output) {
	auto& textureInMap = texturesByAddress.find(std::string(address));
	if (textureInMap != texturesByAddress.end()) {
		output = &textureInMap->second;
		return true;
	}

	return false;
}

void TextureImporter::QueueReloadAsset(Uuid uuid) {
	auto& textureInMap = assets.find(uuid);
	if (textureInMap == assets.end()) {
		return;
	}

	EngineCore& engineCore = EngineCore::GetInstance();
	GraphicsAPI::Core* graphicsCore = engineCore.GetGraphicsCore();

	Texture*& texture = textureInMap->second.texture;
	if (textureInMap->second.texture != nullptr) {
		graphicsCore->DeleteTexture(texture);
		texture = nullptr;
	}

	Grindstone::Assets::AssetLoadBinaryResult result = engineCore.assetManager->LoadBinaryByUuid(AssetType::Texture, uuid);
	if (result.status != Grindstone::Assets::AssetLoadStatus::Success) {
		GPRINT_WARN_V(LogSource::EngineCore, "Unable to load texture: {}", uuid.ToString());
		return;
	}

	ProcessLoadedFile(uuid, result.displayName, std::move(result.buffer), textureInMap->second);
}

TextureImporter::~TextureImporter() {
	EngineCore& engineCore = EngineCore::GetInstance();
	GraphicsAPI::Core* graphicsCore = engineCore.GetGraphicsCore();

	for (auto& asset : assets) {
		graphicsCore->DeleteTexture(asset.second.texture);
	}
	assets.clear();

	for (auto& asset : texturesByAddress) {
		graphicsCore->DeleteTexture(asset.second.texture);
	}
	texturesByAddress.clear();
}
