#include <algorithm>
#include <cstdint>
#include <cstring>
#include <filesystem>
#include <fstream>
#include <mutex>
#include <rapidjson/document.h>
#include <rapidjson/prettywriter.h>
#include <rapidjson/stringbuffer.h>
#include <string>
#include <unordered_set>
#include <variant>
#include <vector>

#include <Common/Logger.hpp>
#include <Common/ResourcePipeline/AssetType.hpp>
#include <Common/ResourcePipeline/Uuid.hpp>
#include <Editor/EditorManager.hpp>
#include <Editor/Importers/ImporterRegistry.hpp>
#include <EditorCommon/ResourcePipeline/MetaFile.hpp>
#include <EngineCore/Utils/Utilities.hpp>

#include "AssetRegistry.hpp"
#include "Importers/ImportScheduler.hpp"

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
	AssetEntryStatus status,
	const std::filesystem::path& path,
	const std::string_view subassetIdentifier,
	const std::string_view displayName,
	const std::string_view address, Uuid& uuid,
	AssetType assetType,
	std::filesystem::file_time_type sourceFileWrite,
	std::filesystem::file_time_type metaFileWrite,
	uint32_t assetImporterVersion,
	uint32_t metaFileVersion,
	uint64_t assetFileSize, uint64_t metaFileSize
) {
	std::scoped_lock lck(mutex);
	std::filesystem::path mountedPath;
	TryGetPathWithMountPoint(path, mountedPath);
	Utils::FixPathSlashes(mountedPath);

	AssetRegistry::Entry* entry = nullptr;
	auto assetIt = assets.find(uuid);

	bool isImport = false;
	if (assetIt != assets.end()) {
		entry = &(assetIt->second);
		isImport = entry->status == AssetEntryStatus::DiscoveredNeedsReimport ||
				   entry->status == AssetEntryStatus::Imported;
	} else {
		entry = &assets[uuid];
	}

	entry->status = status;
	entry->uuid = uuid;
	entry->displayName = std::string(displayName);
	entry->subassetIdentifier = std::string(subassetIdentifier);
	entry->address = std::string(address);
	entry->path = mountedPath;
	entry->assetType = assetType;
	entry->sourceFileWrite = sourceFileWrite;
	entry->metaFileWrite = metaFileWrite;
	entry->assetImporterVersion = assetImporterVersion;
	entry->metaFileVersion = metaFileVersion;
	entry->assetFileSize = assetFileSize;
	entry->metaFileSize = metaFileSize;

	Editor::Manager& editorManager = Editor::Manager::GetInstance();
	Editor::FileManager& fileManager = editorManager.GetFileManager();

	if (isImport) {
		for (Uuid dep : entry->reverseDependencies) {
			auto assetIt = assets.find(dep);
			if (assetIt != assets.end()) {
				std::filesystem::path mountedPath;
				TryGetAbsolutePathFromMountedPath(assetIt->second.path, mountedPath);

				fileManager.DispatchTask(mountedPath);
			}
		}
	}
}

void Grindstone::Editor::AssetRegistry::LinkHardDependencies(Uuid dependentUuid, const std::set<Dependency>& newDependencies) {
	std::scoped_lock lck(mutex);
	LinkHardDependenciesLocked(dependentUuid, newDependencies);
}

void Grindstone::Editor::AssetRegistry::LinkHardDependenciesLocked(Uuid dependentUuid, const std::set<Dependency>& newDependencies) {
	AssetRegistry::Entry& thisAsset = assets[dependentUuid];

	std::set<Dependency> missingNewDependencies;
	std::set<Dependency> missingOldDependencies;
	std::set_difference(thisAsset.dependencies.begin(), thisAsset.dependencies.end(), newDependencies.begin(), newDependencies.end(), std::inserter(missingOldDependencies, missingOldDependencies.end()));
	std::set_difference(newDependencies.begin(), newDependencies.end(), thisAsset.dependencies.begin(), thisAsset.dependencies.end(), std::inserter(missingNewDependencies, missingNewDependencies.end()));

	thisAsset.dependencies = newDependencies;
	
	for (const Dependency& dependency : missingNewDependencies) {
		if (std::holds_alternative<Grindstone::Uuid>(dependency)) {
			Grindstone::Uuid dependencyUuid = std::get<Grindstone::Uuid>(dependency);
			AssetRegistry::Entry& dependencyAsset = assets[dependencyUuid];
			dependencyAsset.reverseDependencies.insert(dependentUuid);
		}
		else {
			std::string dependencyAddress = std::get<std::string>(dependency);
			fileReverseDependencies[dependencyAddress].insert(dependentUuid);
		}
	}

	for (const Dependency& dependency : missingOldDependencies) {
		if (std::holds_alternative<Grindstone::Uuid>(dependency)) {
			Grindstone::Uuid dependencyUuid = std::get<Grindstone::Uuid>(dependency);
			AssetRegistry::Entry& dependencyAsset = assets[dependencyUuid];
			dependencyAsset.reverseDependencies.erase(dependentUuid);
		} else {
			std::string dependencyAddress = std::get<std::string>(dependency);
			fileReverseDependencies[dependencyAddress].erase(dependentUuid);
		}
	}
}

void AssetRegistry::WriteFile() {
	std::scoped_lock lck(mutex);
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
		if (entry.dependencies.size() > 0) {
			documentWriter.Key("dependencies");
			documentWriter.StartArray();
			for (const auto& dependency : entry.dependencies) {
				if (std::holds_alternative<Grindstone::Uuid>(dependency)) {
					Grindstone::Uuid uuid = std::get<Grindstone::Uuid>(dependency);
					documentWriter.String(uuid.ToString().c_str());
				}
				else {
					std::string path = std::get<std::string>(dependency);
					documentWriter.String(path.c_str());
				}
			}
			documentWriter.EndArray();
		}
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
	std::scoped_lock lck(mutex);
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
		const std::string subassetIdentifier = asset.HasMember("subassetIdentifier") ? asset["subassetIdentifier"].GetString() : "";
		std::string displayName = asset.HasMember("displayName") ? asset["displayName"].GetString() : "";
		const std::string address = asset.HasMember("address") ? asset["address"].GetString() : "";
		const std::filesystem::path path = asset.HasMember("path") ? asset["path"].GetString() : "";

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
			GPRINT_FATAL(Grindstone::LogSource::EngineCore, "Unable to make uuid for asset from {}", uuidAsStr);
			continue;
		}

		const AssetType assetType = GetAssetTypeFromString(asset["assetType"].GetString());

		auto& insertedAsset = assets[uuid] = Entry{
			.uuid = uuid,
			.displayName = displayName,
			.subassetIdentifier = subassetIdentifier,
			.address = address,
			.path = path,
			.assetType = assetType,
			.sourceFileWrite = sourceFileDuration,
			.metaFileWrite = metaFileDuration,
			.assetImporterVersion = assetImporterVersion,
			.metaFileVersion = metaFileVersion,
			.assetFileSize = assetFileSize,
			.metaFileSize = metaFileSize,
		};

		if (asset.HasMember("dependencies")) {
			std::set<Dependency> deserializedDependencies;
			rapidjson::Value& dependencies = asset["dependencies"];
			for (rapidjson::Value& dependency : dependencies.GetArray()) {
				if (!dependency.IsString()) {
					GPRINT_FATAL(Grindstone::LogSource::EngineCore, "Found dependency that is not a string for asset {}.", uuidAsStr);
				}

				const char* depAsCstr = dependency.GetString();
				Uuid depUuid;
				if (Grindstone::Uuid::MakeFromString(depAsCstr, depUuid)) {
					deserializedDependencies.emplace(depUuid);
				}
				else {
					std::string depAsString = depAsCstr;
					deserializedDependencies.emplace(depAsString);
				}

			}

			LinkHardDependenciesLocked(uuid, deserializedDependencies);
		}
	}
}

Grindstone::Uuid AssetRegistry::Import(const std::filesystem::path& path) {
	Grindstone::Editor::Manager& editor = Grindstone::Editor::Manager::GetInstance();
	Grindstone::Importers::ImportScheduler& importScheduler = editor.GetImportScheduler();

	if (importScheduler.Import(path)) {
		std::scoped_lock lck(mutex);
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
	std::scoped_lock lck(mutex);
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
