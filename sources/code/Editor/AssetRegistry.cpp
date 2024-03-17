#include <fstream>

#include "rapidjson/document.h"
#include "rapidjson/prettywriter.h"

#include "AssetRegistry.hpp"
#include "EngineCore/Utils/Utilities.hpp"

using namespace Grindstone::Editor;

void AssetRegistry::Initialize(const std::filesystem::path& projectPath) {
	assetsPath = projectPath / "assets";
	assetRegistryPath = projectPath / "compiledAssets/_assetRegistry.json";
	ReadFile();
}

void AssetRegistry::Cleanup() {
	assets.clear();
}

void AssetRegistry::UpdateEntry(std::filesystem::path& path, std::string& name, Uuid& uuid, AssetType assetType) {
	std::filesystem::path relativePath = std::filesystem::relative(path, assetsPath);
	relativePath = Utils::FixPathSlashesReturn(relativePath);

	assets[uuid] = Entry{
		uuid,
		name,
		relativePath,
		assetType
	};
}

void AssetRegistry::WriteFile() {
	rapidjson::StringBuffer documentStringBuffer;
	rapidjson::PrettyWriter<rapidjson::StringBuffer> documentWriter = rapidjson::PrettyWriter<rapidjson::StringBuffer>(documentStringBuffer);

	documentWriter.StartArray();
	for (auto& [_, entry] : assets) {
		documentWriter.StartObject();
		documentWriter.Key("name");
		documentWriter.String(entry.name.c_str());
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
		const char* name = asset["name"].GetString();
		const char* path = asset["path"].GetString();
		Uuid uuid = asset["uuid"].GetString();
		const AssetType assetType = GetAssetTypeFromString(asset["assetType"].GetString());

		assets[uuid] = Entry{
			uuid,
			name,
			path,
			assetType
		};
	}
}

bool AssetRegistry::HasAsset(Uuid uuid) {
	return assets.find(uuid) != assets.end();
}

bool AssetRegistry::TryGetAssetData(std::filesystem::path path, AssetRegistry::Entry& outEntry) {
	for (auto& assetEntries : assets) {
		if (assetEntries.second.path == path) {
			outEntry = assetEntries.second;
			return true;
		}
	}

	return false;
}

bool AssetRegistry::TryGetAssetData(Uuid uuid, AssetRegistry::Entry& outEntry) {
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
