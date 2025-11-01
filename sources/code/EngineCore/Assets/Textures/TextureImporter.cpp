#include <filesystem>
#include <string>
#include <string_view>

#include <Common/Formats/DdsParser.hpp>
#include <Common/Formats/Dds.hpp>
#include <Common/Graphics/Core.hpp>
#include <Common/ResourcePipeline/Uuid.hpp>
#include <EngineCore/Assets/AssetManager.hpp>
#include <EngineCore/EngineCore.hpp>
#include <EngineCore/Logger.hpp>

#include "TextureImporter.hpp"

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

	DdsParseOutput output{};
	if (!TryParseDds(textureAsset.name.c_str(), result.buffer.GetSpan(), output)) {
		textureAsset.assetLoadStatus = Grindstone::AssetLoadStatus::Failed;
		return false;
	}

	Grindstone::GraphicsAPI::Image::CreateInfo imgCreateInfo;
	imgCreateInfo.debugName = textureAsset.name.c_str();
	imgCreateInfo.initialData = reinterpret_cast<const char*>(&output.data.GetBegin());
	imgCreateInfo.initialDataSize = output.data.GetSize();
	imgCreateInfo.mipLevels = output.mipLevels;
	imgCreateInfo.format = output.format;
	imgCreateInfo.width = output.width;
	imgCreateInfo.height = output.height;
	imgCreateInfo.depth = output.depth;
	imgCreateInfo.imageDimensions = output.dimensions;
	imgCreateInfo.arrayLayers = output.arraySize;
	imgCreateInfo.imageUsage =
		GraphicsAPI::ImageUsageFlags::Sampled |
		GraphicsAPI::ImageUsageFlags::TransferSrc |
		GraphicsAPI::ImageUsageFlags::TransferDst;
	imgCreateInfo.imageUsage.Set(GraphicsAPI::ImageUsageFlags::Cubemap, output.isCubemap);
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
	auto textureIterator = assets.emplace(uuid, TextureAsset(uuid));
	TextureAsset& textureAsset = textureIterator.first->second;

	textureAsset.assetLoadStatus = AssetLoadStatus::Loading;
	if (!LoadTextureAsset(textureAsset)) {
		return nullptr;
	}

	return &textureAsset;
}

void TextureImporter::QueueReloadAsset(Uuid uuid) {
	auto textureIterator = assets.find(uuid);
	if (textureIterator == assets.end()) {
		return;
	}

	EngineCore& engineCore = EngineCore::GetInstance();
	textureIterator->second.assetLoadStatus = AssetLoadStatus::Reloading;
	GraphicsAPI::Image* image = textureIterator->second.image;
	GraphicsAPI::Sampler* defaultSampler = textureIterator->second.defaultSampler;
	engineCore.PushDeletion([image, defaultSampler]() {
		EngineCore& engineCore = EngineCore::GetInstance();
		GraphicsAPI::Core* graphicsCore = engineCore.GetGraphicsCore();
		if (image != nullptr) {
			graphicsCore->DeleteImage(image);
		}

		if (defaultSampler != nullptr) {
			graphicsCore->DeleteSampler(defaultSampler);
		}
	});
	textureIterator->second.image = nullptr;
	textureIterator->second.defaultSampler = nullptr;

	LoadTextureAsset(textureIterator->second);
}

void TextureImporter::OnDeleteAsset(TextureAsset& asset) {
	EngineCore& engineCore = EngineCore::GetInstance();
	Grindstone::GraphicsAPI::Image* image = asset.image;
	Grindstone::GraphicsAPI::Sampler* sampler = asset.defaultSampler;
	engineCore.PushDeletion([image, sampler]() {
		EngineCore& engineCore = EngineCore::GetInstance();
		GraphicsAPI::Core* graphicsCore = engineCore.GetGraphicsCore();
		graphicsCore->DeleteImage(image);
		graphicsCore->DeleteSampler(sampler);
	});
	asset.image  = nullptr;
	asset.defaultSampler = nullptr;
}

TextureImporter::~TextureImporter() {
	EngineCore& engineCore = EngineCore::GetInstance();
	GraphicsAPI::Core* graphicsCore = engineCore.GetGraphicsCore();

	for (auto& asset : assets) {
		Grindstone::GraphicsAPI::Image* image = asset.second.image;
		Grindstone::GraphicsAPI::Sampler* sampler = asset.second.defaultSampler;
		graphicsCore->DeleteImage(image);
		graphicsCore->DeleteSampler(sampler);
	}
	assets.clear();

	for (auto& asset : texturesByAddress) {
		Grindstone::GraphicsAPI::Image* image = asset.second.image;
		Grindstone::GraphicsAPI::Sampler* sampler = asset.second.defaultSampler;
		graphicsCore->DeleteImage(image);
		graphicsCore->DeleteSampler(sampler);
	}
	texturesByAddress.clear();
}
