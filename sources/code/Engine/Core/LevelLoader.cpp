#include "LevelLoader.hpp"

#include "rapidjson/reader.h"
#include "rapidjson/filereadstream.h"
#include "rapidjson/istreamwrapper.h"
#include "rapidjson/error/en.h"

#include "Engine.hpp"
#include "../Systems/CBase.hpp"

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
	KEY_SKY_MATERIAL,
	KEY_MAP_NUMCUBE,
	KEY_MAP_CUBEMAPS,
	KEY_ENTITY_NAME,
	KEY_COMPONENT_POSITION,
	KEY_COMPONENT_SCALE,
	KEY_COMPONENT_ANGLES,
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
	KEY_COMPONENT_HEIGHTMAP,
	KEY_COMPONENT_PHYSICS_SPHERE,
	KEY_COMPONENT_PHYSICS_CAPSULE,
	KEY_COMPONENT_PHYSICS_PLANE,
	KEY_COMPONENT_PHYSICS_MASS,
	KEY_COMPONENT_PHYSICS_INERTIA,
	KEY_COMPONENT_PHYSICS_FRICTION,
	KEY_COMPONENT_PHYSICS_RESTITUTION,
	KEY_COMPONENT_PHYSICS_DAMPING,
	KEY_COMPONENT_AUDIO_AUTOPLAY,
	KEY_COMPONENT_AUDIO_LOOPS,
	KEY_COMPONENT_AUDIO_VOLUME,
	KEY_COMPONENT_AUDIO_PITCH
};

#undef Bool

struct MyHandler : public rapidjson::BaseReaderHandler<rapidjson::UTF8<>, MyHandler> {
private:
	unsigned char *byteArray;
	unsigned char level = 0;
	unsigned char keyType;
	unsigned int componentID;
	unsigned char componentType;
	unsigned char subType;
	Entity *ent;
	unsigned int entityID;
	unsigned char subIterator;
	glm::vec2 v2;
	glm::vec3 position;
	glm::vec4 v4;
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
		else if(componentType == COMPONENT_AUDIO_SOURCE) {
			if (keyType == KEY_COMPONENT_AUDIO_LOOPS) {
				engine.audio_system_.GetComponent(componentID)->SetLooping(b);
			}
			else if (keyType == KEY_COMPONENT_AUDIO_AUTOPLAY) {
				if (b)
					engine.audio_system_.AddAutoplaySource(componentID);
			}
		}
		return true;
	}
	bool Int(int i) {
		if (keyType == KEY_MAP_NUMENTS)
			engine.entities.reserve(i);
		else if (keyType == KEY_MAP_NUMCUBE)
			engine.cubemapSystem.Reserve(i);
		//else if (componentType == COMPONENT_TERRAIN)
			//if (keyType == KEY_COMPONENT_PATCHES)
				//engine.terrainSystem.components_[componentID].numPatches = i;
		return true;
	}
	bool Uint(unsigned u) { Int((int)u); return true; }
	bool Int64(int64_t i) { Int((int)i); return true; }
	bool Uint64(uint64_t u) { Int((int)u); return true; }
	bool Double(double d) {
		if (keyType == KEY_MAP_CUBEMAPS) {
			position[subIterator++] = (float)d;
		}
		else {
			if (level == LEVEL_COMPONENT) {
				if (componentType == COMPONENT_TRANSFORM) {
					if (keyType == KEY_COMPONENT_POSITION) {
						engine.transformSystem.components[componentID].position[subIterator++] = (float)d;
					}
					else if (keyType == KEY_COMPONENT_ANGLES) {
						engine.transformSystem.components[componentID].angles[subIterator++] = (float)d*3.14159f / 180.0f;
					}
					else if (keyType == KEY_COMPONENT_SCALE) {
						engine.transformSystem.components[componentID].scale[subIterator++] = (float)d;
					}
				}
				else if (componentType == COMPONENT_LIGHT_SPOT) {
					LightSpotUBO &buffer = engine.lightSystem.spotLights[componentID].lightUBOBuffer;
					if (keyType == KEY_COMPONENT_COLOR)
						buffer.color[subIterator++] = (float)d;
					else if (keyType == KEY_COMPONENT_BRIGHTNESS)
						buffer.power = (float)d;
					else if (keyType == KEY_COMPONENT_RADIUS)
						buffer.attenuationRadius = (float)d;
					else if (keyType == KEY_COMPONENT_INNERANGLE)
						buffer.innerAngle = (float)d*3.14159f / 180.0f;
					else if (keyType == KEY_COMPONENT_OUTERANGLE)
						buffer.outerAngle = (float)d*3.14159f / 180.0f;
				}
				else if (componentType == COMPONENT_LIGHT_POINT) {
					LightPointUBO &buffer = engine.lightSystem.pointLights[componentID].lightUBOBuffer;
					if (keyType == KEY_COMPONENT_COLOR)
						buffer.color[subIterator++] = (float)d;
					else if (keyType == KEY_COMPONENT_BRIGHTNESS)
						buffer.power = (float)d;
					else if (keyType == KEY_COMPONENT_RADIUS)
						buffer.attenuationRadius = (float)d;
				}
				else if (componentType == COMPONENT_LIGHT_DIRECTIONAL) {
					LightDirectionalUBO &buffer = engine.lightSystem.directionalLights[componentID].lightUBOBuffer;
					if (keyType == KEY_COMPONENT_COLOR)
						buffer.color[subIterator++] = (float)d;
					else if (keyType == KEY_COMPONENT_BRIGHTNESS)
						buffer.power = (float)d;
					else if (keyType == KEY_COMPONENT_RADIUS)
						buffer.sourceRadius = (float)d;
				}
				else if (componentType == COMPONENT_PHYSICS) {
					if (keyType == KEY_COMPONENT_PHYSICS_PLANE) {
						v4[subIterator++] = (float)d;
					}
					else if (keyType == KEY_COMPONENT_PHYSICS_SPHERE) {
						engine.physicsSystem.Get(componentID)->SetShapeSphere((float)d);
					}
					else if (keyType == KEY_COMPONENT_PHYSICS_CAPSULE) {
						v4[subIterator++] = (float)d;
					}
					else if (keyType == KEY_COMPONENT_PHYSICS_INERTIA) {
						position[subIterator++] = (float)d;
					}
					else if (keyType == KEY_COMPONENT_PHYSICS_MASS) {
						engine.physicsSystem.Get(componentID)->SetMass((float)d);
					}
					else if (keyType == KEY_COMPONENT_PHYSICS_FRICTION) {
						engine.physicsSystem.Get(componentID)->SetFriction((float)d);
					}
					else if (keyType == KEY_COMPONENT_PHYSICS_RESTITUTION) {
						engine.physicsSystem.Get(componentID)->SetRestitution((float)d);
					}
					else if (keyType == KEY_COMPONENT_PHYSICS_DAMPING) {
						if (subIterator++ == 0) {
							position.x = (float)d;
						}
						else {
							engine.physicsSystem.Get(componentID)->SetDamping(position.x, (float)d);
						}
					}
				}
				else if(componentType == COMPONENT_AUDIO_SOURCE) {
					if (keyType == KEY_COMPONENT_AUDIO_VOLUME) {
						engine.audio_system_.GetComponent(componentID)->SetVolume(d);
					}
					else if (keyType == KEY_COMPONENT_AUDIO_PITCH) {
						engine.audio_system_.GetComponent(componentID)->SetPitch(d);
					}
				}
				/*else if (componentType == COMPONENT_TERRAIN) {
					if (keyType == KEY_COMPONENT_WIDTH)
						engine.terrainSystem.components_[componentID].width = (float)d;
					if (keyType == KEY_COMPONENT_HEIGHT)
						engine.terrainSystem.components_[componentID].height = (float)d;
					if (keyType == KEY_COMPONENT_LENGTH)
						engine.terrainSystem.components_[componentID].length = (float)d;
				}*/
			}
		}
		return true;
	}
	bool String(const char* str, rapidjson::SizeType length, bool copy) {
		if (keyType == KEY_COMPONENT_TYPE) {
			if (std::string(str) == "COMPONENT_TRANSFORM") {
				componentType = COMPONENT_TRANSFORM;
				engine.transformSystem.AddComponent(entityID, ent->components_[COMPONENT_TRANSFORM]);
				componentID = ent->components_[COMPONENT_TRANSFORM];
			}
			else if (std::string(str) == "COMPONENT_CONTROLLER") {
				componentType = COMPONENT_CONTROLLER;
				engine.controllerSystem.AddComponent(entityID, ent->components_[COMPONENT_CONTROLLER]);
				componentID = ent->components_[COMPONENT_CONTROLLER];
			}
			else if (std::string(str) == "COMPONENT_CAMERA") {
				componentType = COMPONENT_CAMERA;
				engine.cameraSystem.AddComponent(entityID, ent->components_[COMPONENT_CAMERA]);
				componentID = ent->components_[COMPONENT_CAMERA];
			}
			else if (std::string(str) == "COMPONENT_AUDIO_LISTENER") {
				componentType = COMPONENT_AUDIO_LISTENER;
				engine.audio_system_.AddListener(entityID, ent->components_[COMPONENT_AUDIO_LISTENER]);
				componentID = ent->components_[COMPONENT_AUDIO_LISTENER];
			}
			else if (std::string(str) == "COMPONENT_AUDIO_SOURCE") {
				componentType = COMPONENT_AUDIO_SOURCE;
				engine.audio_system_.AddSource(entityID, ent->components_[COMPONENT_AUDIO_SOURCE]);
				componentID = ent->components_[COMPONENT_AUDIO_SOURCE];
			}
			else if (std::string(str) == "COMPONENT_GEOMETRY_STATIC") {
				componentType = COMPONENT_GEOMETRY;
				subType = GEOMETRY_STATIC_MODEL;
				engine.geometry_system.AddComponent(entityID, ent->components_[componentType], GEOMETRY_STATIC_MODEL);
				componentID = ent->components_[componentType];
			}
			else if (std::string(str) == "COMPONENT_GEOMETRY_TERRAIN") {
				componentType = COMPONENT_GEOMETRY;
				subType = GEOMETRY_TERRAIN;
				engine.geometry_system.AddComponent(entityID, ent->components_[componentType], GEOMETRY_TERRAIN);
				componentID = ent->components_[componentType];
			}
			/*else if (std::string(str) == "COMPONENT_GEOMETRY_SKELETAL") {
				componentType = COMPONENT_TERRAIN;
				engine.terrainSystem.AddComponent(ent->components_[COMPONENT_TERRAIN]);
				componentID = ent->components_[COMPONENT_TERRAIN];
			}*/
			else if (std::string(str) == "COMPONENT_LIGHT_POINT") {
				componentType = COMPONENT_LIGHT_POINT;
				engine.lightSystem.AddPointLight(entityID);
				componentID = ent->components_[COMPONENT_LIGHT_POINT];
			}
			else if (std::string(str) == "COMPONENT_LIGHT_SPOT") {
				componentType = COMPONENT_LIGHT_SPOT;
				engine.lightSystem.AddSpotLight(entityID);
				componentID = ent->components_[COMPONENT_LIGHT_SPOT];
			}
			else if (std::string(str) == "COMPONENT_LIGHT_DIRECTIONAL") {
				componentType = COMPONENT_LIGHT_DIRECTIONAL;
				engine.lightSystem.AddDirectionalLight(entityID);
				componentID = ent->components_[COMPONENT_LIGHT_DIRECTIONAL];
			}
			else if (std::string(str) == "COMPONENT_PHYSICS") {
				componentType = COMPONENT_PHYSICS;
				engine.physicsSystem.AddComponent(entityID, ent->components_[COMPONENT_PHYSICS]);
				componentID = ent->components_[COMPONENT_PHYSICS];
			}
			else if (std::string(str) == "COMPONENT_INPUT") {
				componentType = COMPONENT_INPUT;
			}
			else if (std::string(str) == "COMPONENT_AUDIO_SOURCE") {
				componentType = COMPONENT_AUDIO_SOURCE;
			}
			else if (std::string(str) == "COMPONENT_AUDIO_LISTENER") {
				componentType = COMPONENT_AUDIO_LISTENER;
			}
			else if (std::string(str) == "COMPONENT_SCRIPT") {
				componentType = COMPONENT_SCRIPT;
			}
			else if (std::string(str) == "COMPONENT_GAME_LOGIC") {
				componentType = COMPONENT_GAME_LOGIC;
			}
		}
		else if (keyType == KEY_SKY_MATERIAL) {
			std::cout << "Skybox set to: " << str << std::endl;
			engine.skybox_.SetMaterial(str);
		}
		else if (keyType == KEY_COMPONENT_PATH) {
			if (componentType == COMPONENT_GEOMETRY) {
				engine.geometry_system.GetSystem(subType)->LoadGeometry(componentID, ("../assets/" + std::string(str)).c_str());
			}
			else if (componentType == COMPONENT_AUDIO_SOURCE) {
				engine.audio_system_.AddBuffer(componentID, ("../assets/" + std::string(str)).c_str());
			}
		}
		else if (keyType == KEY_MAP_NAME) {
			engine.level_name_ = str;
		}

		/*else if (componentType == COMPONENT_TERRAIN)
			if (keyType == KEY_COMPONENT_HEIGHTMAP)
				engine.terrainSystem.components_[componentID].heightmapPath = "../materials/" + std::string(str);
		*/
		return true;
	}
	bool StartObject() {
		level++;
		if (level == LEVEL_ENTITY) {
			entityID = (unsigned int)engine.entities.size();
			engine.entities.push_back(Entity());
			ent = &engine.entities[entityID];
		}
		return true;
	}
	bool Key(const char* str, rapidjson::SizeType length, bool copy) {
		if (level == LEVEL_MAP) {
			if (std::string(str) == "name") {
				keyType = KEY_MAP_NAME;
			}
			else if (std::string(str) == "version") {
				// Ignored for now
			}
			else if (std::string(str) == "skymaterial") {
				keyType = KEY_SKY_MATERIAL;
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
		}
		else if (level == LEVEL_COMPONENT) {
			if (std::string(str) == "componentType") {
				keyType = KEY_COMPONENT_TYPE;
			}
			else if (std::string(str) == "position") {
				keyType = KEY_COMPONENT_POSITION;
			}
			else if (std::string(str) == "angles") {
				keyType = KEY_COMPONENT_ANGLES;
			}
			else if (std::string(str) == "scale") {
				keyType = KEY_COMPONENT_SCALE;
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
			else if (std::string(str) == "create") {
				engine.physicsSystem.Get(componentID)->Create();
			}
			else if (std::string(str) == "shapePlane") {
				keyType = KEY_COMPONENT_PHYSICS_PLANE;
			}
			else if (std::string(str) == "shapeSphere") {
				keyType = KEY_COMPONENT_PHYSICS_SPHERE;
			}
			else if (std::string(str) == "shapeCapsule") {
				keyType = KEY_COMPONENT_PHYSICS_CAPSULE;
			}
			else if (std::string(str) == "mass") {
				keyType = KEY_COMPONENT_PHYSICS_MASS;
			}
			else if (std::string(str) == "inertia") {
				keyType = KEY_COMPONENT_PHYSICS_INERTIA;
			}
			else if (std::string(str) == "friction") {
				keyType = KEY_COMPONENT_PHYSICS_FRICTION;
			}
			else if (std::string(str) == "restitution") {
				keyType = KEY_COMPONENT_PHYSICS_RESTITUTION;
			}
			else if (std::string(str) == "damping") {
				keyType = KEY_COMPONENT_PHYSICS_DAMPING;
			}
			else if (componentType == COMPONENT_AUDIO_SOURCE) {
				if (std::string(str) == "loop") {
					keyType = KEY_COMPONENT_AUDIO_LOOPS;
				}
				else if (std::string(str) == "autoplay") {
					keyType = KEY_COMPONENT_AUDIO_AUTOPLAY;
				}
				else if (std::string(str) == "volume") {
					keyType = KEY_COMPONENT_AUDIO_VOLUME;
				}
				else if (std::string(str) == "pitch") {
					keyType = KEY_COMPONENT_AUDIO_PITCH;
				}
			}
		}
		return true;
	}
	bool EndObject(rapidjson::SizeType memberCount) {
		level--;
		return true;
	}
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
		else if (keyType == KEY_COMPONENT_PHYSICS_PLANE && level == LEVEL_COMPONENT) {
			engine.physicsSystem.Get(componentID)->SetShapePlane(v4.x, v4.y, v4.z, v4.w);
		}
		else if (keyType == KEY_COMPONENT_PHYSICS_CAPSULE && level == LEVEL_COMPONENT) {
			engine.physicsSystem.Get(componentID)->SetShapeCapsule(v2.x, v2.y);
		}
		else if (keyType == KEY_COMPONENT_PHYSICS_INERTIA && level == LEVEL_COMPONENT) {
			engine.physicsSystem.Get(componentID)->SetIntertia(position.x, position.y, position.z);
		}

		return true;
	}
};

bool LoadLevel(std::string path) {
	printf("Loading level: %s\n", path.c_str());

	// Set level file name
	int f = path.find_last_of('/');
	int l = path.find_last_of('.');
	engine.level_file_name_ = path.substr(f + 1, l - f - 1);

	rapidjson::Reader reader;
	MyHandler handler;
	std::ifstream input(path.c_str());
	rapidjson::IStreamWrapper isw(input);
	reader.Parse(isw, handler);
	input.close();

	return true;
}
