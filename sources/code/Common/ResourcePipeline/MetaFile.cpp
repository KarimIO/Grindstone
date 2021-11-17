#include <fstream>
#include <rapidjson/document.h>
#include <rapidjson/prettywriter.h>

#include "EngineCore/Utils/Utilities.hpp"
#include "MetaFile.hpp"
using namespace Grindstone;

void MetaFile::Load(std::filesystem::path baseAssetPath) {
	path = baseAssetPath.string() + ".meta";
	if (!std::filesystem::exists(path)) {
		return;
	}

	std::string fileContents = Utils::LoadFileText(path.string().c_str());

	rapidjson::Document document;
	document.Parse(fileContents.c_str());

	Uuid defaultUuid;
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

		if (defaultUuid == uuid) {
			defaultSubasset.name = name;
			defaultSubasset.uuid = uuid;
		}
		else {
			subassets.emplace_back(name, uuid);
		}
	}
}

void MetaFile::Save() {
	rapidjson::StringBuffer documentStringBuffer;
	rapidjson::PrettyWriter<rapidjson::StringBuffer> documentWriter = rapidjson::PrettyWriter<rapidjson::StringBuffer>(documentStringBuffer);

	documentWriter.StartObject();
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
		documentWriter.EndObject();
	}

	for (auto& subasset : subassets) {
		documentWriter.StartObject();
		documentWriter.Key("name");
		documentWriter.String(subasset.name.c_str());
		documentWriter.Key("uuid");
		documentWriter.String(subasset.uuid.ToString().c_str());
		documentWriter.EndObject();
	}
	documentWriter.EndArray();
	documentWriter.EndObject();

	const char* content = documentStringBuffer.GetString();
	std::ofstream file(path);
	file.write((const char*)content, strlen(content));
	file.flush();
	file.close();
}

bool MetaFile::TryGetDefaultSubassetUuid(Uuid& outUuid) {
	if (defaultSubasset.name != "") {
		outUuid = defaultSubasset.uuid;
		return true;
	}

	return false;
}

Uuid Grindstone::MetaFile::GetOrCreateDefaultSubassetUuid(std::string& subassetName) {
	Uuid uuid;
	if (defaultSubasset.uuid.ToString() != "") {
		defaultSubasset.name = subassetName;
		defaultSubasset.uuid = uuid;
	}

	return uuid;
}

bool MetaFile::TryGetSubassetUuid(std::string& subassetName, Uuid& outUuid) {
	for (auto& subasset : subassets) {
		if (subasset.name == subassetName) {
			outUuid = subasset.uuid;
			return true;
		}
	}

	return false;
}

Uuid MetaFile::GetOrCreateSubassetUuid(std::string& subassetName) {
	Uuid uuid;
	if (!TryGetSubassetUuid(subassetName, uuid)) {
		subassets.emplace_back(subassetName, uuid);
	}

	return uuid;
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
