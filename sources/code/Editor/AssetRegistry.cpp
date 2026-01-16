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
	AssetType assetType,
	std::filesystem::file_time_type sourceFileWrite,
	std::filesystem::file_time_type metaFileWrite,
	uint32_t assetImporterVersion,
	uint32_t metaFileVersion,
	uint64_t assetFileSize,
	uint64_t metaFileSize
) {
	std::filesystem::path mountedPath;
	TryGetPathWithMountPoint(path, mountedPath);
	Utils::FixPathSlashes(mountedPath);

	assets[uuid] = Entry{
		.uuid = uuid,
		.displayName = std::string(displayName),
		.subassetIdentifier = std::string(subassetIdentifier),
		.address = std::string(address),
		.path = mountedPath,
		.assetType = assetType,
		.sourceFileWrite = sourceFileWrite,
		.metaFileWrite = metaFileWrite,
		.assetImporterVersion = assetImporterVersion,
		.metaFileVersion = metaFileVersion,
		.assetFileSize = assetFileSize,
		.metaFileSize = metaFileSize
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
		documentWriter.Key("sourceFileWriteTime");
		auto systemAsseFiletWriteTime = std::chrono::clock_cast<std::chrono::system_clock>(entry.sourceFileWrite);
		documentWriter.Int64(std::chrono::duration_cast<std::chrono::nanoseconds>(systemAsseFiletWriteTime.time_since_epoch()).count());
		documentWriter.Key("metaFileWriteTime");
		auto systemMetaFileWriteTime = std::chrono::clock_cast<std::chrono::system_clock>(entry.metaFileWrite);
		documentWriter.Int64(std::chrono::duration_cast<std::chrono::nanoseconds>(systemMetaFileWriteTime.time_since_epoch()).count());
		documentWriter.Key("assetImporterVersion");
		documentWriter.Uint(entry.assetImporterVersion);
		documentWriter.Key("metaFileVersion");
		documentWriter.Uint(entry.metaFileVersion);
		documentWriter.Key("assetFileSize");
		documentWriter.Uint(entry.assetFileSize);
		documentWriter.Key("metaFileSize");
		documentWriter.Uint(entry.metaFileSize);
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
		const char* subassetIdentifier = asset.HasMember("subassetIdentifier") ? asset["subassetIdentifier"].GetString() : "";
		const char* displayName = asset.HasMember("displayName") ? asset["displayName"].GetString() : "";
		const char* address = asset.HasMember("address") ? asset["address"].GetString() : "";
		const char* path = asset.HasMember("path") ? asset["path"].GetString() : "";

		int64_t sourceFileWriteTime = asset.HasMember("sourceFileWriteTime") ? asset["sourceFileWriteTime"].GetInt64() : 0;
		int64_t metaFileWriteTime = asset.HasMember("metaFileWriteTime") ? asset["metaFileWriteTime"].GetInt64() : 0;

		std::filesystem::file_time_type sourceFileDuration = std::chrono::clock_cast<std::filesystem::file_time_type::clock>(
			std::chrono::system_clock::time_point(
				std::chrono::duration_cast<std::chrono::system_clock::duration>(
					std::chrono::nanoseconds{ sourceFileWriteTime }
				)
			)
		);
		std::filesystem::file_time_type metaFileDuration = std::chrono::clock_cast<std::filesystem::file_time_type::clock>(
			std::chrono::system_clock::time_point(
				std::chrono::duration_cast<std::chrono::system_clock::duration>(
					std::chrono::nanoseconds{ metaFileWriteTime }
				)
			)
		);

		unsigned int assetImporterVersion = asset.HasMember("assetImporterVersion") ? asset["assetImporterVersion"].GetUint() : 0;
		unsigned int metaFileVersion = asset.HasMember("metaFileVersion") ? asset["metaFileVersion"].GetUint() : 0;
		unsigned int assetFileSize = asset.HasMember("assetFileSize") ? asset["assetFileSize"].GetUint() : 0;
		unsigned int metaFileSize = asset.HasMember("metaFileSize") ? asset["metaFileSize"].GetUint() : 0;

		const char* uuidAsStr = asset.HasMember("uuid") ? asset["uuid"].GetString() : "";

		Uuid uuid;
		if (!Grindstone::Uuid::MakeFromString(uuidAsStr, uuid)) {
			GPRINT_FATAL_V(Grindstone::LogSource::EngineCore, "Unable to make uuid for asset from {}", uuidAsStr);
			continue;
		}

		const AssetType assetType = GetAssetTypeFromString(asset["assetType"].GetString());

		assets[uuid] = Entry{
			.uuid = uuid,
			.displayName = std::string(displayName),
			.subassetIdentifier = std::string(subassetIdentifier),
			.address = std::string(address),
			.path = path,
			.assetType = assetType,
			.sourceFileWrite = sourceFileDuration,
			.metaFileWrite = metaFileDuration,
			.assetImporterVersion = assetImporterVersion,
			.metaFileVersion = metaFileVersion,
			.assetFileSize = assetFileSize,
			.metaFileSize = metaFileSize,
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
