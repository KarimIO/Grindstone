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
	LEVEL_COMPONENT,
	LEVEL_CUBEMAP
};

enum {
	KEY_MAP_NAME = 0,
	KEY_MAP_VERSION,
	KEY_MAP_NUMENTS,
	KEY_MAP_NUMCUBE,
	KEY_MAP_CUBEMAPS,
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
	KEY_COMPONENT_OUTERANGLE,
	KEY_COMPONENT_WIDTH,
	KEY_COMPONENT_HEIGHT,
	KEY_COMPONENT_LENGTH,
	KEY_COMPONENT_PATCHES,
	KEY_COMPONENT_HEIGHTMAP
};

struct MyHandler : public rapidjson::BaseReaderHandler<rapidjson::UTF8<>, MyHandler> {
private:
	unsigned char *byteArray;
	unsigned char level = 0;
	unsigned char keyType;
	unsigned int componentID;
	unsigned char componentType;
	EBase *ent;
	unsigned int entityID;
	unsigned char subIterator;
	glm::vec3 position;
public:
	bool Null() { return true; }
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
		else if (keyType == KEY_MAP_NUMCUBE)
			engine.cubemapSystem.Reserve(i);
		else if (componentType == COMPONENT_TERRAIN)
			if (keyType == KEY_COMPONENT_PATCHES)
				engine.terrainSystem.components[componentID].numPatches = i;
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
			ent->angles[subIterator++] = d*3.14159/180.0;
		}
		else if (keyType == KEY_ENTITY_SCALE) {
			ent->scale[subIterator++] = d;
		}
		else if (keyType == KEY_MAP_CUBEMAPS) {
			position[subIterator++] = d;
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
						engine.lightSystem.spotLights[componentID].innerSpotAngle = d*3.14159 / 180.0;
					else if (keyType == KEY_COMPONENT_OUTERANGLE)
						engine.lightSystem.spotLights[componentID].outerSpotAngle = d*3.14159 / 180.0;
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
				else if (componentType == COMPONENT_TERRAIN) {
					if (keyType == KEY_COMPONENT_WIDTH)
						engine.terrainSystem.components[componentID].width = d;
					if (keyType == KEY_COMPONENT_HEIGHT)
						engine.terrainSystem.components[componentID].height = d;
					if (keyType == KEY_COMPONENT_LENGTH)
						engine.terrainSystem.components[componentID].length = d;
				}
			}
		}
		return true;
	}
	bool String(const char* str, rapidjson::SizeType length, bool copy) {
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
			else if (std::string(str) == "COMPONENT_TERRAIN") {
				componentType = COMPONENT_TERRAIN;
				engine.terrainSystem.AddComponent(ent->components[COMPONENT_TERRAIN]);
				componentID = ent->components[COMPONENT_TERRAIN];
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

		else if (componentType == COMPONENT_TERRAIN)
			if (keyType == KEY_COMPONENT_HEIGHTMAP)
				engine.terrainSystem.components[componentID].heightmapPath = "../materials/" + std::string(str);

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
			else if (std::string(str) == "numcubemaps") {
				keyType = KEY_MAP_NUMCUBE;
			}
			else if (std::string(str) == "cubemaps") {
				keyType = KEY_MAP_CUBEMAPS;
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
			else if (std::string(str) == "width") {
				keyType = KEY_COMPONENT_WIDTH;
			}
			else if (std::string(str) == "height") {
				keyType = KEY_COMPONENT_HEIGHT;
			}
			else if (std::string(str) == "length") {
				keyType = KEY_COMPONENT_LENGTH;
			}
			else if (std::string(str) == "patches") {
				keyType = KEY_COMPONENT_PATCHES;
			}
			else if (std::string(str) == "heightmap") {
				keyType = KEY_COMPONENT_HEIGHTMAP;
			}
		}
		return true;
	}
	bool EndObject(rapidjson::SizeType memberCount) { level--; return true; }
	bool StartArray() {
		if (keyType == KEY_MAP_CUBEMAPS)
			level = LEVEL_CUBEMAP;

		subIterator = 0;
		return true;
	}
	bool EndArray(rapidjson::SizeType elementCount) {
		if (keyType == KEY_MAP_CUBEMAPS && level == LEVEL_CUBEMAP) {
			engine.cubemapSystem.AddCubemap(position);
			level = LEVEL_MAP;
		}

		return true;
	}
};

bool LoadLevel(std::string path) {
	printf("Loading level: %s\n", path.c_str());
	rapidjson::Reader reader;
	MyHandler handler;
	std::ifstream input(path.c_str());
	rapidjson::IStreamWrapper isw(input);
	reader.Parse(isw, handler);

	return true;
}
