#include <fstream>
#include <filesystem>
#include <EngineCore/Utils/Utilities.hpp>
#include <EngineCore/EngineCore.hpp>
#include <EngineCore/Logger.hpp>
#include <Common/Graphics/Core.hpp>

#include "ArchiveDirectoryDeserializer.hpp"
#include "ArchiveAssetLoader.hpp"
using namespace Grindstone::Assets;

ArchiveAssetLoader::ArchiveAssetLoader() {
	InitializeDirectory();
}

void ArchiveAssetLoader::InitializeDirectory() {
	ArchiveDirectoryDeserializer deserializer(archiveDirectory);
}

AssetLoadBinaryResult ArchiveAssetLoader::LoadBinaryByUuid(AssetType assetType, Uuid uuid) {
	size_t assetTypeIndex = static_cast<size_t>(assetType);

	if (assetTypeIndex >= static_cast<size_t>(AssetType::Count)) {
		GPRINT_ERROR_V(LogSource::EngineCore, "Invalid Asset Type when trying to load file: {}", uuid.ToString());
		return { AssetLoadStatus::InvalidAssetType, {} };
	}

	ArchiveDirectory::AssetTypeIndex& assetTypeSegment = archiveDirectory.assetTypeIndices[static_cast<size_t>(assetType)];
	auto& assetIterator = assetTypeSegment.assetsByUuid.find(uuid);

	if (assetIterator == assetTypeSegment.assetsByUuid.end()) {
		GPRINT_ERROR_V(LogSource::EngineCore, "Could not load asset: {}", uuid.ToString());
		return { AssetLoadStatus::AssetNotInRegistry, {} };
	}

	return LoadAsset(assetIterator->second);
}

AssetLoadTextResult ArchiveAssetLoader::LoadTextByUuid(AssetType assetType, Uuid uuid) {
	AssetLoadBinaryResult result = LoadBinaryByUuid(assetType, uuid);

	return {
		result.status,
		result.displayName,
		std::string(std::string_view(result.buffer.Get<const char>(0), (size_t)result.buffer.GetCapacity()))
	};
}

Grindstone::Uuid ArchiveAssetLoader::GetUuidByAddress(AssetType assetType, std::string_view address) {
	size_t assetTypeIndex = static_cast<size_t>(assetType);

	if (assetTypeIndex >= static_cast<size_t>(AssetType::Count)) {
		GPRINT_ERROR_V(LogSource::EngineCore, "Invalid Asset Type when trying to load file: {}", address);
		return Uuid();
	}

	ArchiveDirectory::AssetTypeIndex& assetTypeSegment = archiveDirectory.assetTypeIndices[static_cast<size_t>(assetType)];

	return assetTypeSegment.assetUuidByAddress[address];
}

AssetLoadBinaryResult ArchiveAssetLoader::LoadAsset(const ArchiveDirectory::AssetInfo& assetInfo) {
	if (lastBufferIndex != assetInfo.archiveIndex) {
		lastBufferIndex = assetInfo.archiveIndex;

		const std::string filename = "TestArchive_0.garc";
		const std::filesystem::path path = EngineCore::GetInstance().GetProjectPath() / "archives" / filename;
		const std::string filepathAsStr = path.string();
		lastBuffer = Utils::LoadFile(filepathAsStr.c_str());
	}

	Buffer buffer(lastBuffer.Get<Byte>(assetInfo.offset), assetInfo.size);
	return { AssetLoadStatus::Success, std::string(assetInfo.displayName), buffer };
}
