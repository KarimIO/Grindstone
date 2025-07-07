#include <filesystem>
#include <fstream>
#include <iosfwd>
#include <iterator>
#include <string>

#include <Common/Graphics/Core.hpp>
#include <Common/ResourcePipeline/AssetType.hpp>
#include <Common/ResourcePipeline/Uuid.hpp>
#include <EngineCore/EngineCore.hpp>
#include <EngineCore/Utils/Utilities.hpp>
#include <EngineCore/Logger.hpp>
#include <EngineCore/Utils/MemoryAllocator.hpp>

#include "AssetRegistry.hpp"
#include "EditorManager.hpp"
#include "FileAssetLoader.hpp"

using namespace Grindstone::Assets;

static AssetLoadBinaryResult LoadBinary(Grindstone::Editor::AssetRegistry::Entry& entry) {
	std::filesystem::path path = Grindstone::Editor::Manager::GetEngineCore().GetAssetPath(entry.uuid.ToString());

	if (!std::filesystem::exists(path)) {
		return { AssetLoadStatus::FileNotFound, {} };
	}

	GPRINT_INFO_V(Grindstone::LogSource::Editor, "Loading binary asset '{}' of type {}", entry.displayName, GetAssetTypeToString(entry.assetType));

	std::ifstream file(path, std::ios::binary | std::ios::ate);
	size_t fileSize = file.tellg();
	file.seekg(0, std::ios::beg);

	// TODO: Catch if cannot allocate memory
	Grindstone::Buffer buffer(fileSize);
	file.read(reinterpret_cast<char*>(buffer.Get()), fileSize);

	return std::move(AssetLoadBinaryResult{ AssetLoadStatus::Success, entry.displayName, std::move(buffer) });
}

static AssetLoadTextResult LoadText(Grindstone::Editor::AssetRegistry::Entry& entry) {
	std::filesystem::path path = Grindstone::Editor::Manager::GetEngineCore().GetAssetPath(entry.uuid.ToString());

	GPRINT_INFO_V(Grindstone::LogSource::Editor, "Loading text asset '{}' of type {}", entry.displayName, GetAssetTypeToString(entry.assetType));

	if (!std::filesystem::exists(path)) {
		return { AssetLoadStatus::FileNotFound, {} };
	}

	std::ifstream ifs(path, std::ios::in);

	std::string contents;
	std::getline(ifs, contents, '\0');

	return std::move(AssetLoadTextResult{ AssetLoadStatus::Success, entry.displayName, std::move(contents) });
}

AssetLoadBinaryResult FileAssetLoader::LoadBinaryByUuid(AssetType assetType, Uuid uuid) {
	const Editor::AssetRegistry& assetRegistry = Editor::Manager::GetInstance().GetAssetRegistry();

	Editor::AssetRegistry::Entry outEntry;
	if (!assetRegistry.TryGetAssetData(uuid, outEntry)) {
		GPRINT_ERROR_V(LogSource::Editor, "Could not get asset: {}", uuid.ToString());
		return std::move(AssetLoadBinaryResult{ AssetLoadStatus::AssetNotInRegistry, {} });
	}

	return ::LoadBinary(outEntry);
}


AssetLoadTextResult FileAssetLoader::LoadTextByUuid(AssetType assetType, Uuid uuid) {
	const Editor::AssetRegistry& assetRegistry = Editor::Manager::GetInstance().GetAssetRegistry();

	Editor::AssetRegistry::Entry outEntry;
	if (!assetRegistry.TryGetAssetData(uuid, outEntry)) {
		GPRINT_ERROR_V(LogSource::Editor, "Could not get asset: {}", uuid.ToString());
		return std::move(AssetLoadTextResult{ AssetLoadStatus::AssetNotInRegistry, {} });
	}

	return ::LoadText(outEntry);
}

Grindstone::Uuid Grindstone::Assets::FileAssetLoader::GetUuidByAddress(AssetType assetType, std::string_view address) {
	const Editor::AssetRegistry& assetRegistry = Editor::Manager::GetInstance().GetAssetRegistry();

	Editor::AssetRegistry::Entry outEntry;
	std::string addressString = std::string(address);
	if (!assetRegistry.TryGetAssetData(addressString, outEntry)) {
		GPRINT_ERROR_V(LogSource::Editor, "Could not get uuid of asset: {}", address);
		return Grindstone::Uuid();
	}

	return outEntry.uuid;
}
