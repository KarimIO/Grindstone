#include "Space.hpp"
#include "Utilities/Logger.hpp"
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
#include "../Systems/RenderSpriteSystem.hpp"
#include "Engine.hpp"
#include "Editor.hpp"

void handleReflParams(reflect::TypeDescriptor_Struct::Category &refl, unsigned char * componentPtr, rapidjson::Value &params) {
	for (auto &mem : refl.members) {
		std::string n = mem.stored_name;

		unsigned char *p = componentPtr + mem.offset;
		
		if (params.HasMember(n.c_str())) {
			switch (mem.type->type)
			{
			case reflect::TypeDescriptor::ReflString:
				(*(std::string *)p) = params[n.c_str()].GetString();
				break;
			case reflect::TypeDescriptor::ReflBool:
				(*(bool *)p) = params[n.c_str()].GetBool();
				break;
			case reflect::TypeDescriptor::ReflInt:
				(*(int *)p) = params[n.c_str()].GetInt();
				break;
			case reflect::TypeDescriptor::ReflFloat: {
				float v = params[n.c_str()].GetFloat();
				(*(float *)p) = v;
				break;
			}
			case reflect::TypeDescriptor::ReflDouble: {
				double v = params[n.c_str()].GetDouble();
				(*(double *)p) = v;
				break;
			}
			case reflect::TypeDescriptor::ReflVec2: {
				auto a = params[n.c_str()].GetArray();
				glm::vec2 &v = (*(glm::vec2 *)p);
				v.x = a[0].GetFloat();
				v.y = a[1].GetFloat();
				break;
			}
			case reflect::TypeDescriptor::ReflVec3: {
				auto a = params[n.c_str()].GetArray();
				glm::vec3 &v = (*(glm::vec3 *)p);
				v.x = a[0].GetFloat();
				v.y = a[1].GetFloat();
				v.z = a[2].GetFloat();
				break;
			}
			case reflect::TypeDescriptor::ReflVec4: {
				auto a = params[n.c_str()].GetArray();
				glm::vec4 &v = (*(glm::vec4 *)p);
				v.x = a[0].GetFloat();
				v.y = a[1].GetFloat();
				v.z = a[2].GetFloat();
				v.w = a[3].GetFloat();
				break;
			}
			}
		}
	}

	for (auto &cat : refl.categories) {
		handleReflParams(cat, componentPtr, params);
	}
}

void Space::setComponentParams(ComponentHandle handle, ComponentType component_type, rapidjson::Value &params) {
	reflect::TypeDescriptor_Struct *refl = engine.getSystem(component_type)->getReflection();
	if (refl) {
		Component *component = getSubsystem(component_type)->getBaseComponent(handle);

		handleReflParams(refl->category, (unsigned char *)component, params);
	}
}

Space::Space(std::string name, rapidjson::Value &val) : name_(name) {
	memset(subsystems_, 0, sizeof(subsystems_));
	addSystem(new ControllerSubSystem(this));
	addSystem(new ColliderSubSystem(this));
	addSystem(new RigidBodySubSystem(this));
	addSystem(new RenderStaticMeshSubSystem(this));
	addSystem(new LightPointSubSystem(this));
	addSystem(new LightSpotSubSystem(this));
	addSystem(new LightDirectionalSubSystem(this));
	addSystem(new RenderSpriteSubSystem(this));
	addSystem(new TransformSubSystem(this));
	addSystem(new CubemapSubSystem(this));
	addSystem(new CameraSubSystem(this));
	addSystem(new RenderTerrainSubSystem((RenderTerrainSystem *)engine.getSystem(COMPONENT_RENDER_TERRAIN), this));

	for (rapidjson::Value::MemberIterator game_object_itr = val.MemberBegin(); game_object_itr != val.MemberEnd(); ++game_object_itr) {
		GameObjectHandle game_object_handle = (GameObjectHandle)objects_.size();
		GameObjectHandle parent_handle = -1;
		objects_.emplace_back(game_object_handle, game_object_itr->name.GetString(), parent_handle);
		GameObject &game_object = objects_.back();

		auto &comps = game_object_itr->value;
		for (rapidjson::Value::MemberIterator component_itr = comps.MemberBegin(); component_itr != comps.MemberEnd(); ++component_itr) {
			auto type_str = component_itr->name.GetString();
			if (strcmp(type_str, "PREFAB") == 0) {
				const char *params = component_itr->value.GetString();
				loadPrefab(params, game_object);
			}
			else {
				auto type = getComponentType(type_str);
				if (type == COMPONENT_BASE) {
					GRIND_WARN("Could not get component: {0}", type_str);
				}
				else {
					rapidjson::Value &params = component_itr->value;
					auto subsystem = subsystems_[type];

					if (subsystem == nullptr) {
						GRIND_ERROR("No subsystem initialized for: {0}", component_names[type]);
					}
					else {
						ComponentHandle handle = game_object.getComponentHandle(type);
						if (handle == -1) {
							handle = subsystem->addComponent(game_object_handle);
							game_object.setComponentHandle(type, handle);
						}

						setComponentParams(handle, type, params);
					}
				}
			}
		}
	}

	for (auto &subsys : subsystems_) {
		if (subsys)
			subsys->initialize();
	}

#ifdef INCLUDE_EDITOR
	if (engine.getEditor())
		engine.getEditor()->refreshSceneGraph();
#endif
}

Space::Space(const Space &s) {
	name_ = s.name_;

	for (GameObject &g : objects_) {
		objects_.emplace_back(g);
	}

	for (int i = 0; i < NUM_COMPONENTS; ++i) {
		/*if (s.subsystems_[i]) {
			subsystems_[i] = s.subsystems_[i]->copy();
		}
		else*/
		subsystems_[i] = nullptr;
	}
}

void Space::clear() {
	for (int i = 0; i < NUM_COMPONENTS; ++i) {
		if (subsystems_[i])
			delete subsystems_[i];
	}
}

Space::~Space() {
	clear();
}

void Space::loadPrefab(std::string name, GameObject &game_object) {
	name = "../assets/" + name;

	GRIND_WARN("Loading Prefab: {0}", name);

	std::string buffer;
	if (!ReadFile(name, buffer)) {
		GRIND_WARN("Failed to load prefab {0}", name);
		return;
	}

	rapidjson::Document document;
	document.Parse(buffer.c_str());

	GameObjectHandle game_object_handle = game_object.getID();

	for (rapidjson::Value::MemberIterator game_object_itr = document.MemberBegin(); game_object_itr != document.MemberEnd(); ++game_object_itr) {
		auto &comps = game_object_itr->value;
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
				rapidjson::Value &params = component_itr->value;
				auto subsystem = subsystems_[type];

				ComponentHandle handle = game_object.getComponentHandle(type);
				if (handle == -1) {
					handle = subsystem->addComponent(game_object_handle);
					game_object.setComponentHandle(type, handle);
				}

				setComponentParams(handle, type, params);
			}
			//}
		}
	}
}

SubSystem * Space::getSubsystem(ComponentType type) {
	return subsystems_[type];
}

GameObject & Space::getObject(GameObjectHandle handle) {
	return objects_[handle];
}

std::string Space::getName() {
	return name_;
}

SubSystem *Space::addSystem(SubSystem * system) {
	subsystems_[system->getSystemType()] = system;
	return system;
}