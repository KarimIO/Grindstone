#include "Space.hpp"
#include "Utilities/Logger.hpp"
#include <iostream>

//#include "../Systems/LightSystem.hpp"
#include "../Systems/TransformSystem.hpp"
#include "../Systems/CameraSystem.hpp"
#include "../Systems/RenderStaticMeshSystem.hpp"
#include "../Systems/ControllerSystem.hpp"
#include "../Systems/ColliderSystem.hpp"
#include "../Systems/RigidBodySystem.hpp"

Space::Space(std::string name, rapidjson::Value &val) : name_(name) {
	addSystem(new ControllerSubSystem(this));
	addSystem(new ColliderSubSystem(this));
	addSystem(new RigidBodySubSystem(this));
	addSystem(new RenderStaticMeshSubSystem(this));
	//addSystem(new LightSubSystem(this));
	addSystem(new TransformSubSystem(this));
	addSystem(new CameraSubSystem(this));

	for (rapidjson::Value::MemberIterator game_object_itr = val.MemberBegin(); game_object_itr != val.MemberEnd(); ++game_object_itr) {
		auto game_object_id = objects_.size();
		objects_.emplace_back(game_object_id, game_object_itr->name.GetString());
		GameObject &game_object = objects_.back();

		auto &comps = game_object_itr->value;
		for (rapidjson::Value::MemberIterator component_itr = comps.MemberBegin(); component_itr != comps.MemberEnd(); ++component_itr) {
			auto type_str = component_itr->name.GetString();
			if(type_str == "PREFAB") {
				std::cout << type_str << "\n";
			}
			else {
				auto type = getComponentType(type_str);
				if (type == COMPONENT_BASE) {
					LOG_WARN("Could not get component: %s.\n", type_str);
				}
				else {
					std::cout << type_str << " " << type << "\n";
					rapidjson::Value &params = component_itr->value;
					auto subsystem = subsystems_[type];
					auto handle = subsystem->addComponent(game_object_id, params);
					game_object.setComponentHandle(type, handle);
				}
			}
		}
	}
}

void Space::loadPrefab(std::string name, rapidjson::Value &val) {

}

SubSystem * Space::getSubsystem(ComponentType type) {
	return subsystems_[type];
}

GameObject & Space::getObject(GameObjectHandle handle) {
	return objects_[handle];
}

SubSystem *Space::addSystem(SubSystem * system) {
	subsystems_[system->getSystemType()] = system;
	return system;
}