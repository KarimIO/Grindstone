#include <cstdint>
#include <filesystem>
#include <fstream>
#include <rapidjson/document.h>
#include <rapidjson/prettywriter.h>
#include <rapidjson/stringbuffer.h>
#include <string>
#include <vector>

#include <Editor/AssetRegistry.hpp>
#include <EngineCore/Utils/Utilities.hpp>

#include "MetaFile.hpp"
#include "Uuid.hpp"
using namespace Grindstone::Editor;

const uint32_t currentMetaFileVersion = 1;

MetaFile::MetaFile(AssetRegistry& assetRegistry, const std::filesystem::path& path)
	: assetRegistry(assetRegistry) {
	Load(assetRegistry, path);
}

void MetaFile::Load(AssetRegistry& assetRegistry, const std::filesystem::path& baseAssetPath) {
	this->assetRegistry = assetRegistry;
	this->baseAssetPath = baseAssetPath;
	metaFilePath = baseAssetPath.string() + ".meta";

	if (!assetRegistry.TryGetPathWithMountPoint(baseAssetPath, mountedAssetPath)) {
		mountedAssetPath = baseAssetPath;
	}

	metaFilePath = Utils::FixPathSlashesReturn(metaFilePath);

	if (!std::filesystem::exists(metaFilePath)) {
		return;
	}

	std::string fileContents = Utils::LoadFileText(metaFilePath.string().c_str());

	rapidjson::Document document;
	if (document.Parse(fileContents.c_str()).GetParseError()) {
		return;
	}

	version = 0;
	if (document.HasMember("metaFileVersion")) {
		version = document["metaFileVersion"].GetUint();
	}

	Uuid defaultUuid = Uuid::CreateRandom();
	if (document.HasMember("defaultUuid")) {
		defaultUuid = document["defaultUuid"].GetString();
	}

	rapidjson::Value subassetsArray = document["subassets"].GetArray();
	for (
		rapidjson::Value* elementIterator = subassetsArray.Begin();
		elementIterator != subassetsArray.End();
		++elementIterator
	) {
		rapidjson::Value& subasset = *elementIterator;

		if (!subasset.HasMember("subassetIdentifier") ||
			!subasset.HasMember("uuid")
		) {
			isValid = false;
			continue;
		}

		std::string subassetIdentifier = subasset["subassetIdentifier"].GetString();
		std::string displayName = subasset.HasMember("displayName")
			? subasset["displayName"].GetString()
			: subassetIdentifier;

		std::string address = subasset.HasMember("address")
			? subasset["address"].GetString()
			: "";

		Uuid uuid(subasset["uuid"].GetString());
		AssetType assetType = AssetType::Undefined;
		if (subasset.HasMember("type")) {
			const char* assetTypeStr = subasset["type"].GetString();
			assetType = GetAssetTypeFromString(assetTypeStr);
		}

		if (defaultUuid == uuid) {
			defaultSubasset.subassetIdentifier = subassetIdentifier;
			defaultSubasset.displayName = displayName;
			defaultSubasset.address = address;
			defaultSubasset.uuid = uuid;
			defaultSubasset.assetType = assetType;
		}
		else {
			subassets.emplace_back(
				subassetIdentifier,
				displayName,
				address,
				uuid,
				assetType
			);
		}
	}
}

void MetaFile::Save() {
	rapidjson::StringBuffer documentStringBuffer;
	rapidjson::PrettyWriter<rapidjson::StringBuffer> documentWriter = rapidjson::PrettyWriter<rapidjson::StringBuffer>(documentStringBuffer);

	documentWriter.StartObject();
	documentWriter.Key("metaFileVersion");
	documentWriter.Uint(currentMetaFileVersion);
	documentWriter.Key("defaultUuid");
	documentWriter.String(defaultSubasset.uuid.ToString().c_str());
	documentWriter.Key("subassets");
	documentWriter.StartArray();

	if (defaultSubasset.subassetIdentifier != "") {
		documentWriter.StartObject();
		documentWriter.Key("displayName");
		documentWriter.String(defaultSubasset.displayName.c_str());
		documentWriter.Key("address");
		documentWriter.String(defaultSubasset.address.c_str());
		documentWriter.Key("subassetIdentifier");
		documentWriter.String(defaultSubasset.subassetIdentifier.c_str());
		documentWriter.Key("uuid");
		documentWriter.String(defaultSubasset.uuid.ToString().c_str());
		documentWriter.Key("type");
		documentWriter.String(GetAssetTypeToString(defaultSubasset.assetType));
		documentWriter.EndObject();

		assetRegistry.UpdateEntry(
			baseAssetPath,
			defaultSubasset.subassetIdentifier,
			defaultSubasset.displayName,
			defaultSubasset.address,
			defaultSubasset.uuid,
			defaultSubasset.assetType
		);
	}

	for (auto& subasset : subassets) {
		documentWriter.StartObject();
		documentWriter.Key("displayName");
		documentWriter.String(subasset.displayName.c_str());
		documentWriter.Key("address");
		documentWriter.String(subasset.address.c_str());
		documentWriter.Key("subassetIdentifier");
		documentWriter.String(subasset.subassetIdentifier.c_str());
		documentWriter.Key("uuid");
		documentWriter.String(subasset.uuid.ToString().c_str());
		documentWriter.Key("type");
		documentWriter.String(GetAssetTypeToString(subasset.assetType));
		documentWriter.EndObject();

		assetRegistry.UpdateEntry(
			baseAssetPath,
			subasset.subassetIdentifier,
			subasset.displayName,
			subasset.address,
			subasset.uuid,
			subasset.assetType
		);
	}
	documentWriter.EndArray();
	documentWriter.EndObject();


	const char* content = documentStringBuffer.GetString();
	std::ofstream file(metaFilePath);
	file.write((const char*)content, strlen(content));
	file.flush();
	file.close();
}

bool MetaFile::TryGetDefaultSubasset(MetaFile::Subasset& subasset) const {
	if (defaultSubasset.uuid.IsValid()) {
		subasset = defaultSubasset;
		return true;
	}

	return false;
}

bool MetaFile::TryGetDefaultSubassetUuid(Uuid& outUuid) const {
	if (defaultSubasset.uuid.IsValid()) {
		outUuid = defaultSubasset.uuid;
		return true;
	}

	return false;
}

Grindstone::Uuid MetaFile::GetOrCreateDefaultSubassetUuid(std::string& subassetName, AssetType assetType) {
	if (!defaultSubasset.uuid.IsValid()) {
		defaultSubasset.uuid = Uuid::CreateRandom();
	}

	defaultSubasset.assetType = assetType;

	if (defaultSubasset.displayName.empty()) {
		defaultSubasset.displayName = subassetName;
	}

	if (defaultSubasset.subassetIdentifier.empty()) {
		defaultSubasset.subassetIdentifier = subassetName;
	}

	return defaultSubasset.uuid;
}

bool MetaFile::TryGetSubassetUuid(std::string& subassetName, Uuid& outUuid) const {
	for (auto& subasset : subassets) {
		if (subasset.subassetIdentifier == subassetName) {
			outUuid = subasset.uuid;
			return true;
		}
	}

	return false;
}

bool MetaFile::IsValid() const {
	return isValid;
}

bool MetaFile::IsOutdatedVersion() const {
	return version < currentMetaFileVersion;
}

Grindstone::Uuid MetaFile::GetOrCreateSubassetUuid(std::string& subassetName, AssetType assetType) {
	for (auto& subasset : subassets) {
		if (subasset.subassetIdentifier == subassetName) {
			return subasset.uuid;
		}
	}

	Uuid uuid = Uuid::CreateRandom();
	subassets.emplace_back(subassetName, subassetName, "", uuid, assetType);
	return uuid;
}

size_t MetaFile::GetSubassetCount() const {
	return subassets.size();
}

bool Grindstone::Editor::MetaFile::TryGetSubasset(Uuid uuid, Subasset*& outSubasset) {
	if (defaultSubasset.uuid == uuid) {
		outSubasset = &defaultSubasset;
		return true;
	}

	for(Subasset& subasset : subassets) {
		if (subasset.uuid == uuid) {
			outSubasset = &subasset;
			return true;
		}
	}

	return false;
}

MetaFile::Iterator MetaFile::begin() noexcept {
	return subassets.begin();
}

MetaFile::ConstIterator MetaFile::begin() const noexcept {
	return subassets.begin();
}

MetaFile::Iterator MetaFile::end() noexcept {
	return subassets.end();
}

MetaFile::ConstIterator MetaFile::end() const noexcept {
	return subassets.end();
}

std::string MetaFile::MakeDefaultAddress(std::string_view subassetName) const {
	return mountedAssetPath.string() + ":" + std::string(subassetName);
}
