#include <fstream>
#include <rapidjson/document.h>
#include <rapidjson/prettywriter.h>

#include <EngineCore/Utils/Utilities.hpp>
#include <Editor/EditorManager.hpp>
#include "MetaFile.hpp"
using namespace Grindstone::Editor;

const uint32_t currentMetaFileVersion = 1;

void MetaFile::Load(std::filesystem::path baseAssetPath) {
	this->baseAssetPath = baseAssetPath;
	path = baseAssetPath.string() + ".meta";
	if (!std::filesystem::exists(path)) {
		return;
	}

	std::string fileContents = Utils::LoadFileText(path.string().c_str());

	rapidjson::Document document;
	document.Parse(fileContents.c_str());

	version = 0;
	if (document.HasMember("version")) {
		version = document["version"].GetUint();
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
		std::string name = subasset["name"].GetString();
		Uuid uuid(subasset["uuid"].GetString());
		AssetType assetType = AssetType::Undefined;
		if (subasset.HasMember("type")) {
			const char* assetTypeStr = subasset["type"].GetString();
			assetType = GetAssetTypeFromString(assetTypeStr);
		}

		if (defaultUuid == uuid) {
			defaultSubasset.name = name;
			defaultSubasset.uuid = uuid;
			defaultSubasset.assetType = assetType;
		}
		else {
			subassets.emplace_back(name, uuid, assetType);
		}
	}
}

void MetaFile::Save() {
	rapidjson::StringBuffer documentStringBuffer;
	rapidjson::PrettyWriter<rapidjson::StringBuffer> documentWriter = rapidjson::PrettyWriter<rapidjson::StringBuffer>(documentStringBuffer);

	documentWriter.StartObject();
	documentWriter.Key("version");
	documentWriter.Uint(currentMetaFileVersion);
	documentWriter.Key("defaultUuid");
	documentWriter.String(defaultSubasset.uuid.ToString().c_str());
	documentWriter.Key("subassets");
	documentWriter.StartArray();

	if (defaultSubasset.name != "") {
		documentWriter.StartObject();
		documentWriter.Key("name");
		documentWriter.String(defaultSubasset.name.c_str());
		documentWriter.Key("uuid");
		documentWriter.String(defaultSubasset.uuid.ToString().c_str());
		documentWriter.Key("type");
		documentWriter.String(GetAssetTypeToString(defaultSubasset.assetType));
		documentWriter.EndObject();

		Editor::Manager::GetInstance().GetAssetRegistry().UpdateEntry(baseAssetPath, defaultSubasset.name, defaultSubasset.uuid, defaultSubasset.assetType);
	}

	for (auto& subasset : subassets) {
		documentWriter.StartObject();
		documentWriter.Key("name");
		documentWriter.String(subasset.name.c_str());
		documentWriter.Key("uuid");
		documentWriter.String(subasset.uuid.ToString().c_str());
		documentWriter.Key("type");
		documentWriter.String(GetAssetTypeToString(subasset.assetType));
		documentWriter.EndObject();

		Editor::Manager::GetInstance().GetAssetRegistry().UpdateEntry(baseAssetPath, subasset.name, subasset.uuid, subasset.assetType);
	}
	documentWriter.EndArray();
	documentWriter.EndObject();


	const char* content = documentStringBuffer.GetString();
	std::ofstream file(path);
	file.write((const char*)content, strlen(content));
	file.flush();
	file.close();
}

bool MetaFile::TryGetDefaultSubasset(MetaFile::Subasset& subasset) const {
	if (defaultSubasset.name != "") {
		subasset = defaultSubasset;
		return true;
	}

	return false;
}

bool MetaFile::TryGetDefaultSubassetUuid(Uuid& outUuid) const {
	if (defaultSubasset.name != "") {
		outUuid = defaultSubasset.uuid;
		return true;
	}

	return false;
}

Grindstone::Uuid MetaFile::GetOrCreateDefaultSubassetUuid(std::string& subassetName, AssetType assetType) {
	if (!defaultSubasset.uuid.IsValid()) {
		defaultSubasset.assetType = assetType;
		defaultSubasset.name = subassetName;
		defaultSubasset.uuid = Uuid::CreateRandom();
	}
	else
	{
		defaultSubasset.assetType = assetType;
		defaultSubasset.name = subassetName;
	}

	return defaultSubasset.uuid;
}

bool MetaFile::TryGetSubassetUuid(std::string& subassetName, Uuid& outUuid) const {
	for (auto& subasset : subassets) {
		if (subasset.name == subassetName) {
			outUuid = subasset.uuid;
			return true;
		}
	}

	return false;
}

bool MetaFile::IsOutdatedVersion() const {
	return version < currentMetaFileVersion;
}

Grindstone::Uuid MetaFile::GetOrCreateSubassetUuid(std::string& subassetName, AssetType assetType) {
	for (auto& subasset : subassets) {
		if (subasset.name == subassetName) {
			subasset.assetType = assetType;
			return subasset.uuid;
		}
	}

	Uuid uuid = Uuid::CreateRandom();
	subassets.emplace_back(subassetName, uuid, assetType);
	return uuid;
}

size_t MetaFile::GetSubassetCount() const {
	return subassets.size();
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

MetaFile::MetaFile(std::filesystem::path path) {
	Load(path);
}
