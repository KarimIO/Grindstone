#include "Space.hpp"

#include "Utilities.hpp"
#include <iostream>

#include "../Systems/LightSpotSystem.hpp"
#include "../Systems/LightPointSystem.hpp"
#include "../Systems/LightDirectionalSystem.hpp"
#include "../Systems/TransformSystem.hpp"
#include "../Systems/CameraSystem.hpp"
#include "../Systems/RenderStaticMeshSystem.hpp"
#include "../Systems/RenderTerrainSystem.hpp"
#include "../Systems/ControllerSystem.hpp"
#include "../Systems/ColliderSystem.hpp"
#include "../Systems/RigidBodySystem.hpp"
#include "../Systems/CubemapSystem.hpp"
#include "../Systems/ScriptSystem.hpp"
#include "../Systems/RenderSpriteSystem.hpp"
#include "Engine.hpp"
#include "Scene.hpp"

#include <glm/gtc/quaternion.hpp>

#include "Scene.hpp"
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
#include "rapidjson/document.h"

using namespace rapidjson;

void handleReflParams(reflect::TypeDescriptor_Struct::Category& refl, unsigned char* componentPtr, rapidjson::Value& params) {
	for (auto& mem : refl.members) {
		std::string n = mem.stored_name;

		//GRIND_WARN("\t{0}", n);

		unsigned char* p = componentPtr + mem.offset;

		if (params.HasMember(n.c_str())) {
			switch (mem.type->type)
			{
			default:
				GRIND_WARN("Unsupported member type");
				break;
			case reflect::TypeDescriptor::ReflString:
				(*(std::string*)p) = params[n.c_str()].GetString();
				break;
			case reflect::TypeDescriptor::ReflBool:
				(*(bool*)p) = params[n.c_str()].GetBool();
				break;
			case reflect::TypeDescriptor::ReflInt:
				(*(int*)p) = params[n.c_str()].GetInt();
				break;
			case reflect::TypeDescriptor::ReflFloat: {
				float v = params[n.c_str()].GetFloat();
				(*(float*)p) = v;
				break;
			}
			case reflect::TypeDescriptor::ReflDouble: {
				double v = params[n.c_str()].GetDouble();
				(*(double*)p) = v;
				break;
			}
			case reflect::TypeDescriptor::ReflVec2: {
				auto a = params[n.c_str()].GetArray();
				glm::vec2& v = (*(glm::vec2*)p);
				v.x = a[0].GetFloat();
				v.y = a[1].GetFloat();
				break;
			}
			case reflect::TypeDescriptor::ReflVec3: {
				auto a = params[n.c_str()].GetArray();
				glm::vec3& v = (*(glm::vec3*)p);
				v.x = a[0].GetFloat();
				v.y = a[1].GetFloat();
				v.z = a[2].GetFloat();
				break;
			}
			case reflect::TypeDescriptor::ReflVec4: {
				auto a = params[n.c_str()].GetArray();
				glm::vec4& v = (*(glm::vec4*)p);
				v.x = a[0].GetFloat();
				v.y = a[1].GetFloat();
				v.z = a[2].GetFloat();
				v.w = a[3].GetFloat();
				break;
			}
			case reflect::TypeDescriptor::ReflQuat: {
				auto a = params[n.c_str()].GetArray();
				glm::quat& v = (*(glm::quat*)p);
				v.x = a[0].GetFloat();
				v.y = a[1].GetFloat();
				v.z = a[2].GetFloat();
				v.w = a[3].GetFloat();
				break;
			}
			}
		}
	}

	for (auto& cat : refl.categories) {
		handleReflParams(cat, componentPtr, params);
	}
}

void setComponentParams(Space* space, ComponentHandle handle, ComponentType component_type, rapidjson::Value& params) {
	if (component_type == COMPONENT_COLLISION)
		GRIND_WARN("TET");

	reflect::TypeDescriptor_Struct* refl = engine.getSystem(component_type)->getReflection();
	if (refl) {
		Component* component = space->getSubsystem(component_type)->getBaseComponent(handle);

		handleReflParams(refl->category, (unsigned char*)component, params);
	}
}

Space::Space(const char *name) : name_(name) {
	memset(subsystems_, 0, sizeof(subsystems_));
	addSubsystem(new ControllerSubSystem(this));
	addSubsystem(new ColliderSubSystem(this));
	addSubsystem(new RigidBodySubSystem(this));
	addSubsystem(new RenderStaticMeshSubSystem(this));
	addSubsystem(new LightPointSubSystem(this));
	addSubsystem(new LightSpotSubSystem(this));
	addSubsystem(new LightDirectionalSubSystem(this));
	addSubsystem(new RenderSpriteSubSystem(this));
	addSubsystem(new TransformSubSystem(this));
	addSubsystem(new CubemapSubSystem(this));
	addSubsystem(new CameraSubSystem(this));
	addSubsystem(new ScriptSubSystem(this));
	// addSubsystem(new UiSubSystem(this));
	addSubsystem(new RenderTerrainSubSystem(this));
}

Space::Space(const Space &s) {
	name_ = s.name_;

	// TODO: Fix the copy function
	for (int i = 0; i < NUM_COMPONENTS; ++i) {
		/*if (s.subsystems_[i]) {
			subsystems_[i] = s.subsystems_[i]->copy();
		}
		else*/
		subsystems_[i] = nullptr;
	}
}

Space::~Space() {
	clear();
}

void Space::clear() {
	for (int i = 0; i < NUM_COMPONENTS; ++i) {
		if (subsystems_[i])
			delete subsystems_[i];
	}

	memset(subsystems_, 0, sizeof(subsystems_));
}

std::string Space::getPath() {
	return path_;
}

bool Space::reloadScene() {
	clear();
	return loadFromScene(path_);
}

bool Space::loadFromScene(std::string path) {
	GRIND_PROFILE_FUNC();
	GRIND_LOG("Loading level: %s\n", path);

	path_ = path;

	// Load Scene File
	std::string buffer;
	if (!ReadFile(path, buffer)) {
		return false;
	}

	rapidjson::Document document;
	document.Parse(buffer.c_str());

	if (document.HasMember("name"))
		name_ = document["name"].GetString();
	else
		name_ = path;

	for (rapidjson::Value::MemberIterator itr = document["objects"].MemberBegin(); itr != document["objects"].MemberEnd(); ++itr) {
		GameObjectHandle parent_handle = -1;
		
		GameObject& game_object = createObject(itr->name.GetString());
		//GRIND_WARN("NAME: {0}", game_object.getName());

		auto& comps = itr->value;
		for (rapidjson::Value::MemberIterator component_itr = comps.MemberBegin(); component_itr != comps.MemberEnd(); ++component_itr) {
			auto type_str = component_itr->name.GetString();
			if (strcmp(type_str, "PREFAB") == 0) {
				const char* params = component_itr->value.GetString();
				loadPrefab(params, game_object);
			}
			else {
				//GRIND_WARN("COMP: {0}", type_str);
				auto type = getComponentType(type_str);
				if (type == COMPONENT_BASE) {
					GRIND_WARN("Could not get component: {0}", type_str);
				}
				else {
					rapidjson::Value& params = component_itr->value;
					auto subsystem = getSubsystem(type);

					if (subsystem == nullptr) {
						GRIND_ERROR("No subsystem initialized for: {0}", component_names[type]);
					}
					else {
						ComponentHandle handle = game_object.getComponentHandle(type);
						if (handle == -1) {
							handle = game_object.createComponent(type)->handle_;
							game_object.setComponentHandle(type, handle);
						}

						setComponentParams(this, handle, type, params);
					}
				}
			}
		}
	}

	for (int i = 0; i < ComponentType::NUM_COMPONENTS; ++i) {
		auto subys = getSubsystem((ComponentType)i);
		if (subys) {
			subys->initialize();
		}
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

	return true;
}

bool Space::loadPrefab(std::string name, GameObject& game_object) {
	name = "../assets/" + name;

	GRIND_WARN("Loading Prefab: {0}", name);

	std::string buffer;
	if (!ReadFile(name, buffer)) {
		GRIND_WARN("Failed to load prefab {0}", name);
		return false;
	}

	rapidjson::Document document;
	document.Parse(buffer.c_str());

	GameObjectHandle game_object_handle = game_object.getID();

	for (rapidjson::Value::MemberIterator itr = document.MemberBegin(); itr != document.MemberEnd(); ++itr) {
		auto& comps = itr->value;
		for (rapidjson::Value::MemberIterator component_itr = comps.MemberBegin(); component_itr != comps.MemberEnd(); ++component_itr) {
			auto type_str = component_itr->name.GetString();
			/*if (strcmp(type_str, "PREFAB") == 0) {
				const char *params = component_itr->value.GetString();
				loadPrefab(params, game_object);
			}
			else {*/
			auto type = getComponentType(type_str);
			if (type == COMPONENT_BASE) {
				GRIND_WARN("No such component type: {0}", type_str);
			}
			else {
				rapidjson::Value& params = component_itr->value;
				auto subsystem = getSubsystem(type);

				ComponentHandle handle = game_object.getComponentHandle(type);
				if (handle == -1) {
					handle = subsystem->addComponent(game_object_handle);
					game_object.setComponentHandle(type, handle);
				}

				setComponentParams(this, handle, type, params);
			}
			//}
		}
	}

	return true;
}

GameObject& Space::getObject(GameObjectHandle handle) {
	return objects_[handle];
}

SubSystem * Space::getSubsystem(ComponentType type) {
	return subsystems_[type];
}

std::string Space::getName() {
	return name_;
}

SubSystem *Space::addSubsystem(SubSystem * system) {
	subsystems_[system->getSystemType()] = system;
	return system;
}

size_t Space::getNumObjects() {
	return objects_.size();
}

GameObject &Space::createObject(const char *name) {
	auto id = (GameObjectHandle)objects_.size();
	objects_.emplace_back(id, name, this, -1);

	return objects_.back();
}