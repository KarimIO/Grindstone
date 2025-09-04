#include <cstring>
#include <filesystem>
#include <fstream>
#include <rapidjson/document.h>
#include <rapidjson/prettywriter.h>
#include <rapidjson/stringbuffer.h>
#include <string>
#include <vector>

#include <Common/ResourcePipeline/AssetType.hpp>
#include <EditorCommon/ResourcePipeline/MetaFile.hpp>
#include <Common/ResourcePipeline/Uuid.hpp>
#include <EngineCore/Utils/Utilities.hpp>
#include <EngineCore/Logger.hpp>
#include <Editor/Importers/ImporterManager.hpp>
#include <Editor/EditorManager.hpp>

#include "AssetRegistry.hpp"

using namespace Grindstone::Editor;

AssetRegistry::~AssetRegistry() {
	Cleanup();
}

void AssetRegistry::Initialize(const std::filesystem::path& projectPath) {
	assetsPath = projectPath / "assets";
	compiledAssetsPath = projectPath / "compiledAssets";
	assetRegistryPath = compiledAssetsPath / "_assetRegistry.json";
	ReadFile();
}

void AssetRegistry::Cleanup() {
	assets.clear();
}

void AssetRegistry::UpdateEntry(
	const std::filesystem::path& path,
	const std::string_view subassetIdentifier,
	const std::string_view displayName,
	const std::string_view address,
	Uuid& uuid,
	AssetType assetType
) {
	std::filesystem::path mountedPath;
	TryGetPathWithMountPoint(path, mountedPath);
	Utils::FixPathSlashes(mountedPath);

	assets[uuid] = Entry{
		uuid,
		std::string(displayName),
		std::string(subassetIdentifier),
		std::string(address),
		mountedPath,
		assetType
	};
}

void AssetRegistry::WriteFile() {
	rapidjson::StringBuffer documentStringBuffer;
	rapidjson::PrettyWriter<rapidjson::StringBuffer> documentWriter = rapidjson::PrettyWriter<rapidjson::StringBuffer>(documentStringBuffer);

	documentWriter.StartArray();
	for (auto& [_, entry] : assets) {
		documentWriter.StartObject();
		documentWriter.Key("displayName");
		documentWriter.String(entry.displayName.c_str());
		documentWriter.Key("subassetIdentifier");
		documentWriter.String(entry.subassetIdentifier.c_str());
		documentWriter.Key("address");
		documentWriter.String(entry.address.c_str());
		documentWriter.Key("path");
		documentWriter.String(entry.path.string().c_str());
		documentWriter.Key("uuid");
		documentWriter.String(entry.uuid.ToString().c_str());
		documentWriter.Key("assetType");
		documentWriter.String(GetAssetTypeToString(entry.assetType));
		documentWriter.EndObject();
	}
	documentWriter.EndArray();

	const char* content = documentStringBuffer.GetString();
	std::ofstream file(assetRegistryPath);
	file.write(content, static_cast<std::streamsize>(strlen(content)));
	file.flush();
	file.close();
}

void AssetRegistry::ReadFile() {
	assets.clear();
	if (!std::filesystem::exists(assetRegistryPath)) {
		return;
	}

	const std::string assetRegistryContent = Grindstone::Utils::LoadFileText(assetRegistryPath.string().c_str());

	rapidjson::Document document;
	document.Parse(assetRegistryContent.data());

	if (!document.IsArray()) {
		return;
	}

	for (
		rapidjson::Value* assetIterator = document.Begin();
		assetIterator != document.End();
		++assetIterator
	) {
		rapidjson::Value& asset = *assetIterator;
		const char* subassetIdentifier = asset["subassetIdentifier"].GetString();
		const char* displayName = asset["displayName"].GetString();
		const char* address = asset["address"].GetString();
		const char* path = asset["path"].GetString();
		Uuid uuid;
		if (!Grindstone::Uuid::MakeFromString(asset["uuid"].GetString(), uuid)) {
			GPRINT_FATAL_V(Grindstone::LogSource::EngineCore, "Unable to make uuid for asset from {}", asset["uuid"].GetString());
			continue;
		}
		const AssetType assetType = GetAssetTypeFromString(asset["assetType"].GetString());

		assets[uuid] = Entry{
			uuid,
			std::string(displayName),
			std::string(subassetIdentifier),
			std::string(address),
			path,
			assetType
		};
	}
}

Grindstone::Uuid AssetRegistry::Import(const std::filesystem::path& path) {
	Grindstone::Editor::Manager& editor = Grindstone::Editor::Manager::GetInstance();
	Grindstone::Importers::ImporterManager& importerManager = editor.GetImporterManager();

	if (importerManager.Import(path)) {
		std::filesystem::path mountedPath;
		TryGetPathWithMountPoint(path, mountedPath);
		Utils::FixPathSlashes(mountedPath);
		AssetRegistry::Entry outEntry;
		if (TryGetAssetData(mountedPath, outEntry)) {
			return outEntry.uuid;
		}
	}

	return Uuid();
}

Grindstone::Editor::MetaFile AssetRegistry::GetMetaFileByPath(const std::filesystem::path& path) {
	return Grindstone::Editor::MetaFile(*this, path);
}

const std::filesystem::path& AssetRegistry::GetCompiledAssetsPath() const {
	return compiledAssetsPath;
}

bool AssetRegistry::RemoveEntry(Uuid uuid) {
	return assets.erase(uuid) != 0;
}

bool AssetRegistry::HasAsset(Uuid uuid) const {
	return assets.find(uuid) != assets.end();
}

bool AssetRegistry::TryGetPathWithMountPoint(const std::filesystem::path& path, std::filesystem::path& outMountedPath) const {
	Editor::Manager& editorManager = Editor::Manager::GetInstance();
	Editor::FileManager& fileManager = editorManager.GetFileManager();
	return fileManager.TryGetPathWithMountPoint(path, outMountedPath);
}

bool Grindstone::Editor::AssetRegistry::TryGetAbsolutePathFromMountedPath(const std::filesystem::path& mountedPath, std::filesystem::path& outAbsolutePath) const {
	Editor::Manager& editorManager = Editor::Manager::GetInstance();
	Editor::FileManager& fileManager = editorManager.GetFileManager();
	return fileManager.TryGetAbsolutePathFromMountedPath(mountedPath, outAbsolutePath);
}


bool AssetRegistry::TryGetAssetDataFromAbsolutePath(const std::filesystem::path& path, AssetRegistry::Entry& outEntry) const {
	std::filesystem::path mountedPath;
	if (!TryGetPathWithMountPoint(path, mountedPath)) {
		return false;
	}

	for (const auto& [_, assetEntry] : assets) {
		if (assetEntry.path == mountedPath) {
			outEntry = assetEntry;
			return true;
		}
	}

	return false;
}

bool AssetRegistry::TryGetAssetData(const std::filesystem::path& path, AssetRegistry::Entry& outEntry) const {
	for (const auto& [_, assetEntry] : assets) {
		if (assetEntry.path == path) {
			outEntry = assetEntry;
			return true;
		}
	}

	return false;
}

bool AssetRegistry::TryGetAssetData(const std::string& address, AssetRegistry::Entry& outEntry) const {
	for (const auto& [_, assetEntry] : assets) {
		if (assetEntry.address == address) {
			outEntry = assetEntry;
			return true;
		}
	}

	return false;
}

bool AssetRegistry::TryGetAssetData(Uuid uuid, AssetRegistry::Entry& outEntry) const {
	const auto& assetIterator = assets.find(uuid);
	if (assetIterator == assets.end()) {
		return false;
	}

	outEntry = assetIterator->second;
	return true;
}

void AssetRegistry::FindAllFilesOfType(AssetType assetType, std::vector<Entry>& outEntries) const {
	for (const auto& [_, entry] : assets) {
		if (entry.assetType == assetType) {
			outEntries.push_back(entry);
		}
	}
}

std::unordered_set<Grindstone::Uuid> Grindstone::Editor::AssetRegistry::GetUsedUuids() const {
	std::unordered_set<Grindstone::Uuid> unusedUuids;

	for (const auto& [uuid, _] : assets) {
		unusedUuids.insert(uuid);
	}

	return unusedUuids;
}
