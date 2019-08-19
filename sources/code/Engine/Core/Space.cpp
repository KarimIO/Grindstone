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
		objects_.emplace_back(game_object_handle, game_object_itr->name.GetString());
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
							auto handle = subsystem->addComponent(game_object_handle, params);
							game_object.setComponentHandle(type, handle);
						}
						else {
							subsystem->setComponent(handle, params);
						}
					}
				}
			}
		}
	}
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
				GRIND_WARN("Could not get component: {0}", type_str);
			}
			else {
				rapidjson::Value &params = component_itr->value;
				auto subsystem = subsystems_[type];

				ComponentHandle handle = game_object.getComponentHandle(type);
				if (handle == -1) {
					auto handle = subsystem->addComponent(game_object_handle, params);
					game_object.setComponentHandle(type, handle);
				}
				else {
					subsystem->setComponent(handle, params);
				}
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