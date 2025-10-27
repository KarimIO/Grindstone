#pragma once

#include <unordered_map>
#include <filesystem>
#include <vector>

#include <Common/ResourcePipeline/Uuid.hpp>
#include <EngineCore/Assets/Asset.hpp>
#include <Common/Graphics/Image.hpp>

namespace Grindstone::GraphicsAPI {
	class Image;
	class DescriptorSet;
}

namespace Grindstone::Editor {
	using ThumbnailGenerateFn = bool(*)(Grindstone::Uuid uuid);
	class ThumbnailManager {
	public:
		struct AtlasCoords {
			float uv0x;
			float uv0y;
			float uv1x;
			float uv1y;
		};

		void RegisterGenerator(AssetType type, ThumbnailGenerateFn generator);
		void DeregisterGenerator(AssetType type, ThumbnailGenerateFn generator);

		bool Initialize();

		Grindstone::GraphicsAPI::DescriptorSet* GetAtlasTextureDescriptorSet();

		AtlasCoords GetCoordsByIndex(uint32_t index) const;

		AtlasCoords GetFolderIconCoords() const;
		AtlasCoords GetGenericBinaryIconCoords() const;
		AtlasCoords GetPluginIconCoords() const;
		AtlasCoords GetDotnetIconCoords() const;
		AtlasCoords GetCmakeIconCoords() const;

		// This function will get existing icons but will not request new icons to be generated.
		AtlasCoords GetThumbnailCoordsFromCache(AssetType type, Grindstone::Uuid uuid);

		// Frees existing thumbnails from memory.
		bool FreeThumbnailFromMemory(Grindstone::Uuid uuid);

		// This function will get existing icons and will request new icons to be generated if none exist.
		ThumbnailManager::AtlasCoords RequestThumbnail(AssetType type, Grindstone::Uuid uuid);

		// Frees existing thumbnails from storage if it exists.
		bool DeleteThumbnailFromStorage(Grindstone::Uuid uuid);

		// Creates all thumbnails dispatched via RequestThumbnail.
		void CreateRequestedThumbnails();

	protected:
		uint16_t LoadDdsBufferToAtlas(std::string_view name, Grindstone::Containers::BufferSpan inputBuffer);
		uint16_t LoadNamedAssetToAtlas(std::string_view name);
		uint16_t LoadThumbnailByPathToAtlas(const std::filesystem::path& path);

	protected:
		enum class IconStatus {
			Pending,
			Failed,
			Loading,
			Generating,
			Resolved
		};
		struct IconData {
			IconStatus status = IconStatus::Pending;
			AtlasCoords coords;
		};
		std::unordered_map<AssetType, ThumbnailGenerateFn> generators;
		std::unordered_map<Grindstone::Uuid, IconData> iconsByUuid;
		std::vector<std::pair<Grindstone::AssetType, Grindstone::Uuid>> requestedThumbnails;

		Grindstone::GraphicsAPI::Image* thumbnailAtlasImage = nullptr;
		Grindstone::GraphicsAPI::DescriptorSet* thumbnailAtlasDescriptorSet = nullptr;

		std::vector<uint16_t> freeIndices;

		struct {
			uint16_t folder;
			uint16_t plugin;
			uint16_t generic;
			uint16_t dotnet;
			uint16_t cmake;
			uint16_t asset[static_cast<size_t>(AssetType::Count)];
		} defaultIcons;

		uint32_t thumbnailFreeCount = 0;
		uint32_t thumbnailCapacity = 0;
		uint32_t thumbnailPixelSize = 0; // Width or height, as thumbnails are square.
		uint32_t atlasPixelWidth = 0;
		uint32_t atlasPixelHeight = 0;
		uint32_t thumbnailsPerAtlasRow = 0;
	};
}
