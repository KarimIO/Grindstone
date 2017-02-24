#include "LevelLoader.h"

#include "rapidjson/reader.h"
#include "rapidjson/filereadstream.h"
#include "rapidjson/istreamwrapper.h"
#include "rapidjson/error/en.h"

#include "../Core/Engine.h"
#include "../Systems/CBase.h"

enum {
	LEVEL_ROOT = 0,
	LEVEL_MAP,
	LEVEL_ENTITY,
	LEVEL_COMPONENT
};

enum {
	KEY_MAP_NAME = 0,
	KEY_MAP_VERSION,
	KEY_MAP_NUMENTS,
	KEY_ENTITY_NAME,
	KEY_ENTITY_POSITION,
	KEY_ENTITY_SCALE,
	KEY_ENTITY_ANGLES,
	KEY_COMPONENT_TYPE,
	KEY_COMPONENT_PATH,
	KEY_COMPONENT_COLOR,
	KEY_COMPONENT_BRIGHTNESS,
	KEY_COMPONENT_SHADOW,
	KEY_COMPONENT_RADIUS,
	KEY_COMPONENT_INNERANGLE,
	KEY_COMPONENT_OUTERANGLE
};

struct MyHandler : public rapidjson::BaseReaderHandler<rapidjson::UTF8<>, MyHandler> {
private:
	unsigned char *byteArray;
	unsigned char level = 0;
	unsigned char keyType;
	unsigned int numEnts;
	unsigned int componentID;
	unsigned char componentType;
	EBase *ent;
	unsigned int entityID;
	unsigned char subIterator;
public:
	bool Null() { std::cout << "Null()\n"; return true; }
	bool Bool(bool b) {
		if (keyType == KEY_COMPONENT_SHADOW) {
			if (componentType == COMPONENT_LIGHT_SPOT) {
				engine.lightSystem.spotLights[componentID].SetShadow(b);
			}
			else if (componentType == COMPONENT_LIGHT_POINT) {
				engine.lightSystem.pointLights[componentID].SetShadow(b);
			}
			else if (componentType == COMPONENT_LIGHT_DIRECTIONAL) {
				engine.lightSystem.directionalLights[componentID].SetShadow(b);
			}

		}
		return true;
	}
	bool Int(int i) {
		if (keyType == KEY_MAP_NUMENTS)
			engine.entities.reserve(i);
		return true;
	}
	bool Uint(unsigned u) { Int((int)u); return true; }
	bool Int64(int64_t i) { Int((int)i); return true; }
	bool Uint64(uint64_t u) { Int((int)u); return true; }
	bool Double(double d) {
		if (keyType == KEY_ENTITY_POSITION) {
			ent->position[subIterator++] = d;
		}
		else if (keyType == KEY_ENTITY_ANGLES) {
			ent->angles[subIterator++] = d;
		}
		else if (keyType == KEY_ENTITY_SCALE) {
			ent->scale[subIterator++] = d;
		}
		else {
			if (level == LEVEL_COMPONENT) {
				if (componentType == COMPONENT_LIGHT_SPOT) {
					if (keyType == KEY_COMPONENT_COLOR)
						engine.lightSystem.spotLights[componentID].lightColor[subIterator++] = d;
					else if (keyType == KEY_COMPONENT_BRIGHTNESS)
						engine.lightSystem.spotLights[componentID].intensity = d;
					else if (keyType == KEY_COMPONENT_RADIUS)
						engine.lightSystem.spotLights[componentID].lightRadius = d;
					else if (keyType == KEY_COMPONENT_INNERANGLE)
						engine.lightSystem.spotLights[componentID].innerSpotAngle = d;
					else if (keyType == KEY_COMPONENT_OUTERANGLE)
						engine.lightSystem.spotLights[componentID].outerSpotAngle = d;
				}
				else if (componentType == COMPONENT_LIGHT_POINT) {
					if (keyType == KEY_COMPONENT_COLOR)
						engine.lightSystem.pointLights[componentID].lightColor[subIterator++] = d;
					else if (keyType == KEY_COMPONENT_BRIGHTNESS)
						engine.lightSystem.pointLights[componentID].intensity = d;
					else if (keyType == KEY_COMPONENT_RADIUS)
						engine.lightSystem.pointLights[componentID].lightRadius = d;
				}
				else if (componentType == COMPONENT_LIGHT_DIRECTIONAL) {
					if (keyType == KEY_COMPONENT_COLOR)
						engine.lightSystem.directionalLights[componentID].lightColor[subIterator++] = d;
					else if (keyType == KEY_COMPONENT_BRIGHTNESS)
						engine.lightSystem.directionalLights[componentID].intensity = d;
					else if (keyType == KEY_COMPONENT_RADIUS)
						engine.lightSystem.directionalLights[componentID].sunRadius = d;
				}
			}
		}
		return true;
	}
	bool String(const char* str, rapidjson::SizeType length, bool copy) {
		std::cout << "String(" << str << ", " << length << ", " << std::boolalpha << copy << ")\n";
		if (keyType == KEY_COMPONENT_TYPE) {
			if (std::string(str) == "COMPONENT_POSITION") {
				componentType = COMPONENT_POSITION;
			}
			else if (std::string(str) == "COMPONENT_RENDER") {
				componentType = COMPONENT_RENDER;
				engine.geometryCache.AddComponent(ent->components[COMPONENT_RENDER]);
				componentID = ent->components[COMPONENT_RENDER];
			}
			else if (std::string(str) == "COMPONENT_LIGHT_POINT") {
				componentType = COMPONENT_LIGHT_POINT;
				engine.lightSystem.AddPointLight(entityID);
				componentID = ent->components[COMPONENT_LIGHT_POINT];
			}
			else if (std::string(str) == "COMPONENT_LIGHT_SPOT") {
				componentType = COMPONENT_LIGHT_SPOT;
				engine.lightSystem.AddSpotLight(entityID);
				componentID = ent->components[COMPONENT_LIGHT_SPOT];
			}
			else if (std::string(str) == "COMPONENT_LIGHT_DIRECTIONAL") {
				componentType = COMPONENT_LIGHT_DIRECTIONAL;
				engine.lightSystem.AddDirectionalLight(entityID);
				componentID = ent->components[COMPONENT_LIGHT_DIRECTIONAL];
			}
			else if (std::string(str) == "COMPONENT_PHYSICS") {
				componentType = COMPONENT_PHYSICS;
			}
			else if (std::string(str) == "COMPONENT_INPUT") {
				componentType = COMPONENT_INPUT;
			}
			else if (std::string(str) == "COMPONENT_AUDIO") {
				componentType = COMPONENT_AUDIO;
			}
			else if (std::string(str) == "COMPONENT_SCRIPT") {
				componentType = COMPONENT_SCRIPT;
			}
			else if (std::string(str) == "COMPONENT_GAME_LOGIC") {
				componentType = COMPONENT_GAME_LOGIC;
			}
		}
		else if (keyType == KEY_COMPONENT_PATH) {
			engine.geometryCache.PreloadModel3D(("../models/" + std::string(str)).c_str(), ent->components[componentType]);
		}

		return true;
	}
	bool StartObject() {
		level++;
		if (level == LEVEL_ENTITY) {
			entityID = engine.entities.size();
			engine.entities.push_back(EBase());
			ent = &engine.entities.back();
		}
		return true;
	}
	bool Key(const char* str, rapidjson::SizeType length, bool copy) {
		if (level == LEVEL_MAP) {
			if (std::string(str) == "name") {
				// Ignored for now
			}
			else if (std::string(str) == "version") {
				// Ignored for now
			}
			else if (std::string(str) == "numentities") {
				keyType = KEY_MAP_NUMENTS;
			}
			else if (std::string(str) == "entities") {
				// Ignored for now
			}
		}
		else if (level == LEVEL_ENTITY) {
			if (std::string(str) == "name") {
				keyType = KEY_ENTITY_NAME;
			}
			else if (std::string(str) == "position") {
				keyType = KEY_ENTITY_POSITION;
			}
			else if (std::string(str) == "angles") {
				keyType = KEY_ENTITY_ANGLES;
			}
			else if (std::string(str) == "scale") {
				keyType = KEY_ENTITY_SCALE;
			}
		}
		else if (level == LEVEL_COMPONENT) {
			if (std::string(str) == "componentType") {
				keyType = KEY_COMPONENT_TYPE;
			}
			else if (std::string(str) == "path") {
				keyType = KEY_COMPONENT_PATH;
			}
			else if (std::string(str) == "color") {
				keyType = KEY_COMPONENT_COLOR;
			}
			else if (std::string(str) == "brightness") {
				keyType = KEY_COMPONENT_BRIGHTNESS;
			}
			else if (std::string(str) == "castshadow") {
				keyType = KEY_COMPONENT_SHADOW;
			}
			else if (std::string(str) == "lightradius") {
				keyType = KEY_COMPONENT_RADIUS;
			}
			else if (std::string(str) == "sunradius") {
				keyType = KEY_COMPONENT_RADIUS;
			}
			else if (std::string(str) == "innerangle") {
				keyType = KEY_COMPONENT_INNERANGLE;
			}
			else if (std::string(str) == "outerangle") {
				keyType = KEY_COMPONENT_OUTERANGLE;
			}
		}
		return true;
	}
	bool EndObject(rapidjson::SizeType memberCount) { std::cout << "EndObject(" << memberCount << ")\n"; level--; return true; }
	bool StartArray() { std::cout << "StartArray()\n"; subIterator = 0; return true; }
	bool EndArray(rapidjson::SizeType elementCount) { std::cout << "EndArray(" << elementCount << ")\n"; return true; }
};

bool LoadLevel(std::string path) {
	rapidjson::Reader reader;
	MyHandler handler;
	std::ifstream input(path.c_str());
	rapidjson::IStreamWrapper isw(input);
	reader.Parse(isw, handler);

	return true;
}
