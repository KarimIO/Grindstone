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
	: assetRegistry(&assetRegistry) {
	Load(assetRegistry, path);
}

void MetaFile::Load(AssetRegistry& assetRegistry, const std::filesystem::path& baseAssetPath) {
	this->assetRegistry = &assetRegistry;
	this->baseAssetPath = baseAssetPath;
	isDirty = false;
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

	metaVersion = 0;
	if (document.HasMember("metaFileVersion")) {
		metaVersion = document["metaFileVersion"].GetUint();
	}

	importerVersion = 0;
	if (document.HasMember("assetImporterVersion")) {
		importerVersion = document["assetImporterVersion"].GetUint();
	}

	Uuid defaultUuid;
	if (
		!document.HasMember("defaultUuid") ||
		!Grindstone::Uuid::MakeFromString(document["defaultUuid"].GetString(), defaultUuid)
	) {
		defaultUuid = Uuid::CreateRandom();
	}

	auto subassetsArray = document["subassets"].GetArray();
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

		Uuid uuid;
		if (!Uuid::MakeFromString(subasset["uuid"].GetString(), uuid)) {
			continue;
		}

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

	if (document.HasMember("importerSettings")) {
		rapidjson::Value& settingsArray = document["importerSettings"];
		for (
			rapidjson::Value::ConstMemberIterator itr = settingsArray.MemberBegin();
			itr != settingsArray.MemberEnd();
			++itr
		) {
			importerSettings.SetUnknown(itr->name.GetString(), itr->value.GetString());
		}
	}
}

void MetaFile::Save(uint32_t currentImporterVersion) {
	if (importerVersion == currentImporterVersion && !isDirty && !importerSettings.isDirty) {
		return;
	}

	importerVersion = currentImporterVersion;

	rapidjson::StringBuffer documentStringBuffer;
	rapidjson::PrettyWriter<rapidjson::StringBuffer> documentWriter = rapidjson::PrettyWriter<rapidjson::StringBuffer>(documentStringBuffer);
	GS_ASSERT(assetRegistry != nullptr);

	{
		documentWriter.StartObject();

		documentWriter.Key("assetImporterVersion");
		documentWriter.Uint(currentImporterVersion);

		documentWriter.Key("metaFileVersion");
		documentWriter.Uint(currentMetaFileVersion);

		documentWriter.Key("defaultUuid");
		documentWriter.String(defaultSubasset.uuid.ToString().c_str());

		{
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

				assetRegistry->UpdateEntry(
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

				assetRegistry->UpdateEntry(
					baseAssetPath,
					subasset.subassetIdentifier,
					subasset.displayName,
					subasset.address,
					subasset.uuid,
					subasset.assetType
				);
			}
			documentWriter.EndArray();
		}

		{
			documentWriter.Key("importerSettings");
			documentWriter.StartObject();
			for (const auto& it : importerSettings) {
				const std::string& key = it.first;
				const std::string& value = it.second.value;

				documentWriter.Key(key.c_str());
				documentWriter.String(value.c_str());
			}
			documentWriter.EndObject();
		}

		documentWriter.EndObject();
	}

	const char* content = documentStringBuffer.GetString();
	std::ofstream file(metaFilePath);
	file.write((const char*)content, strlen(content));
	file.flush();
	file.close();
}

void MetaFile::SaveWithoutImporterVersionChange() {
	Save(importerVersion);
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

Grindstone::Uuid MetaFile::GetOrCreateDefaultSubassetUuid(const std::string& subassetName, AssetType assetType) {
	if (defaultSubasset.subassetIdentifier == subassetName && assetType == defaultSubasset.assetType) {
		if (!defaultSubasset.uuid.IsValid()) {
			defaultSubasset.uuid = Uuid::CreateRandom();
			isDirty = true;
		}

		if (defaultSubasset.displayName.empty()) {
			defaultSubasset.displayName = subassetName;
			isDirty = true;
		}

		return defaultSubasset.uuid;
	}

	isDirty = true;

	Subasset* outSubasset = nullptr;
	if (TryGetSubasset(subassetName, outSubasset)) {
		defaultSubasset = *outSubasset;

		subassets.erase((subassets.begin() + (outSubasset - subassets.data())));

		return defaultSubasset.uuid;
	}

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

bool MetaFile::TryGetSubassetUuid(const std::string& subassetName, Uuid& outUuid) const {
	if (defaultSubasset.subassetIdentifier == subassetName) {
		outUuid = defaultSubasset.uuid;
		return true;
	}

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

bool MetaFile::IsOutdatedImporterVersion(Grindstone::Editor::ImporterVersion currentImporterVersion) const {
	return (currentImporterVersion == 0) || (importerVersion != currentImporterVersion);
}

bool MetaFile::IsOutdatedMetaVersion() const {
	return metaVersion < currentMetaFileVersion;
}

Grindstone::Uuid MetaFile::GetOrCreateSubassetUuid(const std::string& subassetName, AssetType assetType) {
	for (auto& subasset : subassets) {
		if (subasset.subassetIdentifier == subassetName) {
			return subasset.uuid;
		}
	}

	Uuid uuid = Uuid::CreateRandom();
	subassets.emplace_back(subassetName, subassetName, "", uuid, assetType);
	isDirty = true;
	return uuid;
}

size_t MetaFile::GetSubassetCount() const {
	return subassets.size();
}

bool Grindstone::Editor::MetaFile::TryGetSubasset(const std::string& subassetName, Subasset*& outSubasset) {
	if (defaultSubasset.subassetIdentifier == subassetName) {
		outSubasset = &defaultSubasset;
		return true;
	}

	for (Subasset& subasset : subassets) {
		if (subasset.subassetIdentifier == subassetName) {
			outSubasset = &subasset;
			return true;
		}
	}

	return false;
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
