#include "Scene.hpp"
#include "../Utilities/Logger.hpp"
#include "Utilities.hpp"
#include "Space.hpp"
#include "Engine.hpp"

#include "AssetManagers/AudioManager.hpp"
#include "AssetManagers/ModelManager.hpp"
#include "AssetManagers/GraphicsPipelineManager.hpp"
#include "AssetManagers/MaterialManager.hpp"
#include "AssetManagers/TextureManager.hpp"

#include "rapidjson/writer.h"
#include "rapidjson/stringbuffer.h"

using namespace rapidjson;

Scene::Scene(std::string path) {
	loadLevel(path);
}

std::string Scene::getName() {
	return name_;
}

std::string Scene::getPath() {
	return path_;
}

void Scene::loadLevel(std::string path) {
	LOG("Loading level: %s\n", path);

	path_ = path;
	
	// Load Scene File
	std::string buffer;
	ReadFile(path, buffer);

	rapidjson::Document document;
	document.Parse(buffer.c_str());

	if (document.HasMember("name"))
		name_ = document["name"].GetString();
	else
		name_ = path;
	
	for (rapidjson::Value::MemberIterator itr = document["spaces"].MemberBegin(); itr != document["spaces"].MemberEnd(); ++itr) {
		rapidjson::Value v = itr->value.GetObject();
		spaces_.push_back(new Space(itr->name.GetString(), v));
	}

	/*
	// Load Lazy-Loaded Assets
	// - Audio
	engine.getAudioManager()->loadPreloaded();
	// - Materials
	engine.getMaterialManager()->loadPreloaded();
	// - GraphicsPipeline
	engine.getGraphicsPipelineManager()->loadPreloaded();
	// - Texture
	engine.getTextureManager()->loadPreloaded();*/
	// - Model
	engine.getModelManager()->loadPreloaded();
}
