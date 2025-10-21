#include <Editor/EditorManager.hpp>
#include <Editor/ImguiEditor/ImguiEditor.hpp>
#include <Editor/ImguiEditor/ImguiRenderer.hpp>
#include <EngineCore/Logger.hpp>
#include <EngineCore/Utils/Utilities.hpp>
#include <Common/Graphics/Core.hpp>
#include <Common/Formats/DdsParser.hpp>

#include "ThumbnailManager.hpp"

using namespace Grindstone::Editor;
using ThumbnailManager = Grindstone::Editor::ThumbnailManager;

inline static std::filesystem::path GetThumbnailCacheFolder() {
	return Grindstone::EngineCore::GetInstance().GetProjectPath() / "thumbnailCache";
}

bool ThumbnailManager::Initialize() {
	Grindstone::Editor::Manager& editor = Grindstone::Editor::Manager::GetInstance();
	Grindstone::Editor::ImguiEditor::ImguiEditor& imguiEditor = editor.GetImguiEditor();
	Grindstone::Editor::ImguiEditor::ImguiRenderer& imguiRenderer = imguiEditor.GetImguiRenderer();
	Grindstone::GraphicsAPI::Core* graphicsCore = editor.GetEngineCore().GetGraphicsCore();

	using ImageUsage = Grindstone::GraphicsAPI::ImageUsageFlags;
	using ImageDimension = Grindstone::GraphicsAPI::ImageDimension;

	// TODO: Maybe have a dynamic limit based off settings or device specs to reduce
	// VRAM memory cost of the atlas.
	atlasPixelWidth = 2048;
	atlasPixelHeight = 2048;

	// Constant value based on vibes alone! Maybe one day we can justify larger icons?
	thumbnailPixelSize = 128;

	thumbnailsPerAtlasRow = atlasPixelWidth / thumbnailPixelSize;
	uint32_t thumbnailsPerAtlasColumn = atlasPixelHeight / thumbnailPixelSize;
	thumbnailCapacity = thumbnailsPerAtlasColumn * thumbnailsPerAtlasRow;
	thumbnailFreeCount = thumbnailCapacity;

	// Make sure we don't have dumb math, and use multiples of thumbnailPixelSize so we don't use extra memory unnecessarily.
	GS_ASSERT(atlasPixelWidth % thumbnailPixelSize == 0);
	GS_ASSERT(atlasPixelHeight % thumbnailPixelSize == 0);

	freeIndices.resize(thumbnailCapacity);
	for (uint16_t i = 0; i < thumbnailCapacity; ++i) {
		freeIndices[i] = i;
	}

	Grindstone::GraphicsAPI::Image::CreateInfo imageCreateInfo{
		.debugName = "Thumbnail Atlas",
		.width = atlasPixelWidth,
		.height = atlasPixelHeight,
		.depth = 1,
		.mipLevels = 1,
		.arrayLayers = 1,
		.format = Grindstone::GraphicsAPI::Format::BC3_UNORM_BLOCK,
		.imageDimensions = ImageDimension::Dimension2D,
		.imageUsage = ImageUsage::TransferSrc | ImageUsage::TransferDst | ImageUsage::Sampled,
		.initialData = nullptr,
		.initialDataSize = 0,
	};

	thumbnailAtlasImage = graphicsCore->CreateImage(imageCreateInfo);

	GraphicsAPI::DescriptorSetLayout::Binding descriptorLayoutBinding = { 0, 1, GraphicsAPI::BindingType::CombinedImageSampler, GraphicsAPI::ShaderStageBit::Fragment };
	GraphicsAPI::DescriptorSetLayout::CreateInfo descriptorLayoutCreateInfo{
		.debugName = "Editor Thumbnail Atlas Descriptor Set Layout",
		.bindings = &descriptorLayoutBinding,
		.bindingCount = 1
	};
	auto descriptorSetLayout = graphicsCore->CreateDescriptorSetLayout(descriptorLayoutCreateInfo);

	Grindstone::GraphicsAPI::Sampler::CreateInfo thumbnailAtlasSamplerCreateInfo{
		.debugName = "Thumbnail Atlas Sampler",
		.options = {
			.wrapModeU = GraphicsAPI::TextureWrapMode::Repeat,
			.wrapModeV = GraphicsAPI::TextureWrapMode::Repeat,
			.wrapModeW = GraphicsAPI::TextureWrapMode::Repeat,
			.mipFilter = GraphicsAPI::TextureFilter::Nearest,
			.minFilter = GraphicsAPI::TextureFilter::Linear,
			.magFilter = GraphicsAPI::TextureFilter::Linear,
			.anistropy = 0.0f,
			.mipMin = -1000.f,
			.mipMax = 1000.0f,
			.mipBias = 0.0f,
		}
	};
	auto thumbnailAtlasSampler = graphicsCore->CreateSampler(thumbnailAtlasSamplerCreateInfo);

	std::pair<GraphicsAPI::Image*, GraphicsAPI::Sampler*> descriptor = { thumbnailAtlasImage, thumbnailAtlasSampler };
	GraphicsAPI::DescriptorSet::Binding descriptorBinding = Grindstone::GraphicsAPI::DescriptorSet::Binding::CombinedImageSampler(&descriptor, 1);
	GraphicsAPI::DescriptorSet::CreateInfo descriptorCreateInfo{
		.debugName = "Editor Thumbnail Atlas Descriptor Set",
		.layout = descriptorSetLayout,
		.bindings = &descriptorBinding,
		.bindingCount = 1
	};
	thumbnailAtlasDescriptorSet = graphicsCore->CreateDescriptorSet(descriptorCreateInfo);

	// Match order used in the getters
	defaultIcons.folder =	LoadNamedAssetToAtlas("@EDITOR_ICONS/assetIcons/Folder");
	defaultIcons.plugin =	LoadNamedAssetToAtlas("@EDITOR_ICONS/assetIcons/Plugin");
	defaultIcons.generic =	LoadNamedAssetToAtlas("@EDITOR_ICONS/assetIcons/GenericBinary");
	defaultIcons.dotnet =	LoadNamedAssetToAtlas("@EDITOR_ICONS/assetIcons/Dotnet");
	defaultIcons.cmake =	LoadNamedAssetToAtlas("@EDITOR_ICONS/assetIcons/Cmake");

	for (uint16_t i = 0; i < static_cast<uint16_t>(AssetType::Count); ++i) {
		AssetType assetType = static_cast<AssetType>(i);
		uint16_t index = LoadNamedAssetToAtlas(std::string("@EDITOR_ICONS/assetIcons/") + std::string(GetAssetTypeToString(assetType)));
		defaultIcons.asset[i] = index;
	}

	return true;
}

uint16_t ThumbnailManager::LoadDdsBufferToAtlas(std::string_view name, Grindstone::Containers::BufferSpan inputBuffer) {
	Grindstone::Formats::DDS::DdsParseOutput output{};
	if (!Grindstone::Formats::DDS::TryParseDds(name.data(), inputBuffer, output)) {
		return std::numeric_limits<uint16_t>::max();
	}

	--thumbnailFreeCount;
	size_t sizeMinusOne = freeIndices.size() - 1;
	uint16_t thumbnailIndex = freeIndices[sizeMinusOne];
	freeIndices.resize(sizeMinusOne);

	uint32_t thumbnailX = thumbnailIndex % thumbnailsPerAtlasRow;
	uint32_t thumbnailY = thumbnailIndex / thumbnailsPerAtlasRow;

	GraphicsAPI::Image::ImageRegion region{
			.bufferOffset = 0,
			.bufferRowLength = 0,
			.bufferImageHeight = 0,
			.x = static_cast<int32_t>(thumbnailX * thumbnailPixelSize),
			.y = static_cast<int32_t>(thumbnailY * thumbnailPixelSize),
			.z = 0,
			.width = output.width,
			.height = output.height,
			.depth = 1,
			.mipLevel = 0,
			.baseArrayLayer = 0,
			.arrayLayerCount = 1,
	};

	thumbnailAtlasImage->UploadDataRegions(&output.data.GetBegin(), output.data.GetSize(), &region, 1);
	return thumbnailIndex;
}

uint16_t ThumbnailManager::LoadThumbnailByPathToAtlas(const std::filesystem::path& path) {
	std::string name = path.string();

	if (thumbnailFreeCount == 0) {
		GPRINT_WARN_V(LogSource::EngineCore, "Could not allocate a thumbnail for asset at path '{}'", name.c_str());
		return std::numeric_limits<uint16_t>::max();
	}

	Grindstone::Buffer buffer = Grindstone::Utils::LoadFile(name.c_str());

	return LoadDdsBufferToAtlas(name, buffer.GetSpan());
}


uint16_t ThumbnailManager::LoadNamedAssetToAtlas(std::string_view name) {
	if (thumbnailFreeCount == 0) {
		GPRINT_WARN_V(LogSource::EngineCore, "Could not allocate a thumbnail for asset {}", name);
		return std::numeric_limits<uint16_t>::max();
	}

	EngineCore& engineCore = EngineCore::GetInstance();
	Grindstone::Uuid uuid = engineCore.assetManager->GetUuidByAddress(AssetType::Texture, name.data());
	Grindstone::Assets::AssetLoadBinaryResult result = engineCore.assetManager->LoadBinaryByUuid(AssetType::Texture, uuid);
	if (result.status != Grindstone::Assets::AssetLoadStatus::Success) {
		GPRINT_WARN_V(LogSource::EngineCore, "Unable to load thumbnail for icon: {}", name);
		return std::numeric_limits<uint16_t>::max();
	}

	return LoadDdsBufferToAtlas(name, result.buffer.GetSpan());
}

Grindstone::GraphicsAPI::DescriptorSet* ThumbnailManager::GetAtlasTextureDescriptorSet() {
	return thumbnailAtlasDescriptorSet;
}

ThumbnailManager::AtlasCoords ThumbnailManager::GetCoordsByIndex(uint32_t index) const {
	uint32_t thumbnailX = index % thumbnailsPerAtlasRow;
	uint32_t thumbnailY = index / thumbnailsPerAtlasRow;

	return ThumbnailManager::AtlasCoords{
		.uv0x = static_cast<float>(thumbnailX) * thumbnailPixelSize / atlasPixelWidth,
		.uv0y = static_cast<float>(thumbnailY) * thumbnailPixelSize / atlasPixelHeight,
		.uv1x = static_cast<float>(thumbnailX + 1) * thumbnailPixelSize / atlasPixelWidth,
		.uv1y = static_cast<float>(thumbnailY + 1) * thumbnailPixelSize / atlasPixelHeight,
	};
}

ThumbnailManager::AtlasCoords ThumbnailManager::GetFolderIconCoords() const {
	return GetCoordsByIndex(defaultIcons.folder);
}

ThumbnailManager::AtlasCoords ThumbnailManager::GetPluginIconCoords() const {
	return GetCoordsByIndex(defaultIcons.plugin);
}

ThumbnailManager::AtlasCoords ThumbnailManager::GetGenericBinaryIconCoords() const {
	return GetCoordsByIndex(defaultIcons.generic);
}

ThumbnailManager::AtlasCoords ThumbnailManager::GetDotnetIconCoords() const {
	return GetCoordsByIndex(defaultIcons.dotnet);
}

ThumbnailManager::AtlasCoords ThumbnailManager::GetCmakeIconCoords() const {
	return GetCoordsByIndex(defaultIcons.cmake);
}

void ThumbnailManager::RegisterGenerator(AssetType type, ThumbnailGenerateFn generator) {
	auto assetIterator = generators.find(type);
	if (assetIterator != generators.end()) {
		GPRINT_WARN_V(
			Grindstone::LogSource::Editor,
			"Trying to register thumbnail generator for {} when one already is registered. Overwriting it.",
			GetAssetTypeToString(type)
		);
	}

	generators[type] = generator;
}

void ThumbnailManager::UnregisterGenerator(AssetType type, ThumbnailGenerateFn generator) {
	auto assetIterator = generators.find(type);
	if (assetIterator == generators.end()) {
		GPRINT_WARN_V(
			Grindstone::LogSource::Editor,
			"Trying to unregister thumbnail generator for {} but none found.",
			GetAssetTypeToString(type)
		);
	}

	if (assetIterator->second == generator) {
		GPRINT_WARN_V(
			Grindstone::LogSource::Editor,
			"Trying to register unregister generator for {}, but found a different generator. Ignoring this Unregister call.",
			GetAssetTypeToString(type)
		);
	}
	else {
		generators.erase(assetIterator);
	}
}

// This function will get existing icons but will not request new icons to be generated.
ThumbnailManager::AtlasCoords ThumbnailManager::GetThumbnailCoordsFromCache(Grindstone::AssetType assetType, Grindstone::Uuid uuid) {
	auto assetIterator = iconsByUuid.find(uuid);
	if (assetIterator != iconsByUuid.end()) {
		return assetIterator->second.coords;
	}

	std::filesystem::path path = GetThumbnailCacheFolder() / uuid.ToString();
	if (std::filesystem::exists(path)) {
		uint16_t thumbnailIndex = LoadThumbnailByPathToAtlas(path);
		if (thumbnailIndex != std::numeric_limits<uint16_t>().max()) {
			auto& iconInMap = iconsByUuid[uuid];
			iconInMap.isResolved = true;
			iconInMap.coords = GetCoordsByIndex(thumbnailIndex);
		}
	}

	return GetCoordsByIndex(defaultIcons.asset[static_cast<size_t>(assetType)]);
}

ThumbnailManager::AtlasCoords ThumbnailManager::RequestThumbnail(Grindstone::AssetType assetType, Grindstone::Uuid uuid) {
	auto assetIterator = iconsByUuid.find(uuid);
	if (assetIterator == iconsByUuid.end()) {
		std::string uuidAsStr = uuid.ToString();

		GPRINT_WARN_V(
			Grindstone::LogSource::Editor,
			"Trying to get existing thumbnail from storage of type '{}' with uuid '{}' but none found.",
			GetAssetTypeToString(assetType),
			uuidAsStr
		);

		return GetCoordsByIndex(defaultIcons.asset[static_cast<size_t>(assetType)]);
	}

	return assetIterator->second.coords;
}

bool ThumbnailManager::FreeThumbnailFromMemory(Grindstone::Uuid uuid) {
	auto assetIterator = iconsByUuid.find(uuid);
	if (assetIterator == iconsByUuid.end()) {
		std::string uuidAsStr = uuid.ToString();
		GPRINT_WARN_V(
			Grindstone::LogSource::Editor,
			"Trying to free thumbnail for uuid '{}' in memory cache, but none found.",
			uuidAsStr
		);

		return false;
	}

	iconsByUuid.erase(assetIterator);

	return true;
}

bool ThumbnailManager::DeleteThumbnailFromStorage(Grindstone::Uuid uuid) {
	auto assetIterator = iconsByUuid.find(uuid);
	if (assetIterator != iconsByUuid.end()) {
		iconsByUuid.erase(assetIterator);
	}

	std::filesystem::path path = GetThumbnailCacheFolder() / uuid.ToString();
	if (std::filesystem::exists(path)) {
		std::filesystem::remove(path);
		return true;
	}
	else {
		std::string uuidAsStr = uuid.ToString();
		GPRINT_WARN_V(
			Grindstone::LogSource::Editor,
			"Trying to delete thumbnail file for uuid '{}' in storage cache, but file not found.",
			uuidAsStr
		);

		return false;
	}
}
