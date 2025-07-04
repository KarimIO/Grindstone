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

static bool LoadTextureAsset(TextureAsset& textureAsset) {
	EngineCore& engineCore = EngineCore::GetInstance();

	Grindstone::Assets::AssetLoadBinaryResult result = engineCore.assetManager->LoadBinaryByUuid(AssetType::Texture, textureAsset.uuid);
	if (result.status != Grindstone::Assets::AssetLoadStatus::Success) {
		GPRINT_WARN_V(LogSource::EngineCore, "Unable to load texture: {}", textureAsset.uuid.ToString());
		textureAsset.assetLoadStatus = Grindstone::AssetLoadStatus::Missing;
		return false;
	}

	textureAsset.name = result.displayName;

	const char* fileContents = reinterpret_cast<const char*>(result.buffer.Get());
	if (strncmp(fileContents, "DDS ", 4) != 0) {
		GPRINT_WARN_V(LogSource::EngineCore, "Invalid texture file: {}", textureAsset.uuid.ToString());
		textureAsset.assetLoadStatus = Grindstone::AssetLoadStatus::Failed;
		return false;
	}

	DDSHeader header;
	std::memcpy(&header, fileContents + 4, sizeof(header));

	bool hasAlpha = header.ddspf.dwFlags & DDPF_ALPHAPIXELS;

	bool useDxgi = false;
	Grindstone::GraphicsAPI::Format format = GraphicsAPI::Format::Invalid;

	bool isFourCC = (header.ddspf.dwFlags & DDPF_FOURCC) > 0;
	bool isRGB = (header.ddspf.dwFlags & DDPF_RGB) > 0;

	if (isFourCC) {
		switch (header.ddspf.dwFourCC) {
		case FOURCC_DXT1:
			format = hasAlpha
				? Grindstone::GraphicsAPI::Format::BC1_RGBA_UNORM_BLOCK
				: Grindstone::GraphicsAPI::Format::BC1_RGB_UNORM_BLOCK;
			break;
		case FOURCC_DXT3:
			format = Grindstone::GraphicsAPI::Format::BC2_UNORM_BLOCK;
			break;
		case FOURCC_DXT5:
			format = Grindstone::GraphicsAPI::Format::BC3_UNORM_BLOCK;
			break;
		case FOURCC_BC4:
			format = Grindstone::GraphicsAPI::Format::BC4_UNORM_BLOCK;
			break;
		case FOURCC_BC5:
			format = Grindstone::GraphicsAPI::Format::BC5_UNORM_BLOCK;
			break;

		case FOURCC_R16:
			format = Grindstone::GraphicsAPI::Format::R16_UNORM;
			break;
		case FOURCC_RG16:
			format = Grindstone::GraphicsAPI::Format::R16G16_UNORM;
			break;
		case FOURCC_RGBA16:
			format = Grindstone::GraphicsAPI::Format::R16G16B16A16_UNORM;
			break;
		case FOURCC_R32:
			format = Grindstone::GraphicsAPI::Format::R32_SFLOAT;
			break;
		case FOURCC_RG32:
			format = Grindstone::GraphicsAPI::Format::R32G32_SFLOAT;
			break;
		case FOURCC_RGBA32:
			format = Grindstone::GraphicsAPI::Format::R32G32B32A32_SFLOAT;
			break;
		case FOURCC_DXGI:
			useDxgi = true;
			break;
		default:
			GPRINT_ERROR_V(LogSource::EngineCore, "Invalid FourCC in texture with id {}.", textureAsset.uuid.ToString());
			textureAsset.assetLoadStatus = Grindstone::AssetLoadStatus::Failed;
			return false;
		}
	}
	else if (isRGB) {
		switch (header.ddspf.dwRGBBitCount) {
		case 24:
			format = Grindstone::GraphicsAPI::Format::R8G8B8_UINT;
			break;
		case 32:
			format = Grindstone::GraphicsAPI::Format::R8G8B8A8_UINT;
			break;
		default:
			GPRINT_ERROR_V(LogSource::EngineCore, "Invalid rgb pixel format in texture with id {}.", textureAsset.uuid.ToString());
			textureAsset.assetLoadStatus = Grindstone::AssetLoadStatus::Failed;
			return false;
		}
	}
	else {
		GPRINT_ERROR_V(LogSource::EngineCore, "Invalid pixel format in texture with id {}.", textureAsset.uuid.ToString());
		textureAsset.assetLoadStatus = Grindstone::AssetLoadStatus::Failed;
		return false;
	}

	Grindstone::GraphicsAPI::ImageDimension imageDimensions = Grindstone::GraphicsAPI::ImageDimension::Dimension2D;
	const char* imgPtr = fileContents + 4 + sizeof(DDSHeader);
	if (useDxgi) {
		DDSHeaderExtended extendedHeader;
		std::memcpy(&extendedHeader, imgPtr, sizeof(DDSHeaderExtended));
		if (extendedHeader.dxgiFormat == DxgiFormat::DXGI_FORMAT_BC6H_TYPELESS || extendedHeader.dxgiFormat == DxgiFormat::DXGI_FORMAT_BC6H_UF16) {
			format = Grindstone::GraphicsAPI::Format::BC6H_UFLOAT_BLOCK;
		}
		else if (extendedHeader.dxgiFormat == DxgiFormat::DXGI_FORMAT_BC6H_SF16) {
			format = Grindstone::GraphicsAPI::Format::BC6H_SFLOAT_BLOCK;
		}
		else {
			GPRINT_ERROR_V(LogSource::EngineCore, "Invalid extended DXGI format in texture with id {}.", textureAsset.uuid.ToString());
			textureAsset.assetLoadStatus = Grindstone::AssetLoadStatus::Failed;
			return false;
		}

		switch (extendedHeader.resourceDimension) {
		default:
			GPRINT_ERROR_V(LogSource::EngineCore, "Invalid dimensions in texture with id({}) and name({}).", textureAsset.uuid.ToString(), textureAsset.name.c_str());
			imageDimensions = Grindstone::GraphicsAPI::ImageDimension::Invalid;
			break;
		case D3d10ResourceDimension::D3D10_RESOURCE_DIMENSION_UNKNOWN:
			imageDimensions = Grindstone::GraphicsAPI::ImageDimension::Invalid;
			break;
		case D3d10ResourceDimension::D3D10_RESOURCE_DIMENSION_BUFFER:
			GPRINT_ERROR_V(LogSource::EngineCore, "Invalid dimensions \"Buffer\" in texture with id({}) and name({}).", textureAsset.uuid.ToString(), textureAsset.name.c_str());
			imageDimensions = Grindstone::GraphicsAPI::ImageDimension::Invalid;
			break;
		case D3d10ResourceDimension::D3D10_RESOURCE_DIMENSION_TEXTURE1D:
			imageDimensions = Grindstone::GraphicsAPI::ImageDimension::Dimension1D;
			break;
		case D3d10ResourceDimension::D3D10_RESOURCE_DIMENSION_TEXTURE2D:
			imageDimensions = Grindstone::GraphicsAPI::ImageDimension::Dimension2D;
			break;
		case D3d10ResourceDimension::D3D10_RESOURCE_DIMENSION_TEXTURE3D:
			imageDimensions = Grindstone::GraphicsAPI::ImageDimension::Dimension3D;
			break;
		}

		if (imageDimensions == Grindstone::GraphicsAPI::ImageDimension::Invalid) {
			if (header.dwHeight == 1) {
				imageDimensions = Grindstone::GraphicsAPI::ImageDimension::Dimension1D;
			}
			else if (header.dwDepth == 1) {
				imageDimensions = Grindstone::GraphicsAPI::ImageDimension::Dimension2D;
			}
			else {
				imageDimensions = Grindstone::GraphicsAPI::ImageDimension::Dimension3D;
			}
		}

		imgPtr += sizeof(DDSHeaderExtended);
	}

	bool hasCubemap = header.dwCaps2 & DDS_CUBEMAP_ALLFACES;

	Grindstone::GraphicsAPI::Image::CreateInfo imgCreateInfo;
	imgCreateInfo.debugName = textureAsset.name.c_str();
	imgCreateInfo.initialData = imgPtr;
	imgCreateInfo.initialDataSize = result.buffer.GetCapacity() - static_cast<uint64_t>(imgPtr - fileContents);
	imgCreateInfo.mipLevels = header.dwMipMapCount > 0 ? static_cast<uint32_t>(header.dwMipMapCount) : 1;
	imgCreateInfo.format = format;
	imgCreateInfo.width = header.dwWidth;
	imgCreateInfo.height = header.dwHeight;
	imgCreateInfo.depth = header.dwDepth > 0 ? header.dwDepth : 1;
	imgCreateInfo.imageDimensions = imageDimensions;
	imgCreateInfo.arrayLayers = hasCubemap ? 6 : 1;
	imgCreateInfo.imageUsage =
		GraphicsAPI::ImageUsageFlags::Sampled |
		GraphicsAPI::ImageUsageFlags::TransferSrc |
		GraphicsAPI::ImageUsageFlags::TransferDst;
	imgCreateInfo.imageUsage.Set(GraphicsAPI::ImageUsageFlags::Cubemap, hasCubemap);
	imgCreateInfo.imageUsage.Set(GraphicsAPI::ImageUsageFlags::GenerateMipmaps, false);

	std::string samplerName = textureAsset.name + " Sampler";
	Grindstone::GraphicsAPI::Sampler::CreateInfo samplerCreateInfo;
	samplerCreateInfo.debugName = samplerName.c_str();
	samplerCreateInfo.options.anistropy = 16.0f;
	samplerCreateInfo.options.mipMin = -1000.0f;
	samplerCreateInfo.options.mipMax = 1000.0f;
	samplerCreateInfo.options.mipFilter = GraphicsAPI::TextureFilter::Linear;
	samplerCreateInfo.options.minFilter = GraphicsAPI::TextureFilter::Linear;
	samplerCreateInfo.options.magFilter = GraphicsAPI::TextureFilter::Linear;
	samplerCreateInfo.options.wrapModeU = GraphicsAPI::TextureWrapMode::ClampToEdge;
	samplerCreateInfo.options.wrapModeV = GraphicsAPI::TextureWrapMode::ClampToEdge;
	samplerCreateInfo.options.wrapModeW = GraphicsAPI::TextureWrapMode::ClampToEdge;

	GraphicsAPI::Core* graphicsCore = engineCore.GetGraphicsCore();
	textureAsset.image = graphicsCore->CreateImage(imgCreateInfo);
	textureAsset.defaultSampler = graphicsCore->CreateSampler(samplerCreateInfo);
	textureAsset.assetLoadStatus = Grindstone::AssetLoadStatus::Ready;

	return true;
}

void* TextureImporter::LoadAsset(Uuid uuid) {
	auto& textureIterator = assets.emplace(uuid, TextureAsset(uuid));
	TextureAsset& textureAsset = textureIterator.first->second;

	textureAsset.assetLoadStatus = AssetLoadStatus::Loading;
	if (!LoadTextureAsset(textureAsset)) {
		return nullptr;
	}

	return &textureAsset;
}

void TextureImporter::QueueReloadAsset(Uuid uuid) {
	auto& textureInMap = assets.find(uuid);
	if (textureInMap == assets.end()) {
		return;
	}

	EngineCore& engineCore = EngineCore::GetInstance();
	GraphicsAPI::Core* graphicsCore = engineCore.GetGraphicsCore();

	textureInMap->second.assetLoadStatus = AssetLoadStatus::Reloading;
	GraphicsAPI::Image*& image = textureInMap->second.image;
	if (image != nullptr) {
		graphicsCore->DeleteImage(image);
		image = nullptr;
	}

	GraphicsAPI::Sampler*& defaultSampler = textureInMap->second.defaultSampler;
	if (defaultSampler != nullptr) {
		graphicsCore->DeleteSampler(defaultSampler);
		defaultSampler = nullptr;
	}

	LoadTextureAsset(textureInMap->second);
}

TextureImporter::~TextureImporter() {
	EngineCore& engineCore = EngineCore::GetInstance();
	GraphicsAPI::Core* graphicsCore = engineCore.GetGraphicsCore();

	for (auto& asset : assets) {
		graphicsCore->DeleteImage(asset.second.image);
		graphicsCore->DeleteSampler(asset.second.defaultSampler);
	}
	assets.clear();

	for (auto& asset : texturesByAddress) {
		graphicsCore->DeleteImage(asset.second.image);
		graphicsCore->DeleteSampler(asset.second.defaultSampler);
	}
	texturesByAddress.clear();
}
