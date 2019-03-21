#include "ColliderSystem.hpp"
#include <btBulletDynamicsCommon.h>
#include "../Utilities/Logger.hpp"

ColliderComponent::ColliderComponent(GameObjectHandle object_handle, ComponentHandle id) : Component(COMPONENT_COLLISION, object_handle, id) {}

ColliderSystem::ColliderSystem() : System(COMPONENT_COLLISION) {}

ColliderSubSystem::ColliderSubSystem(Space *space) : SubSystem(COMPONENT_COLLISION, space) {}

ComponentHandle ColliderSubSystem::addComponent(GameObjectHandle object_handle) {
	ComponentHandle component_handle = (ComponentHandle)components_.size();
	components_.emplace_back(object_handle, component_handle);

	return component_handle;
}

ComponentHandle ColliderSubSystem::addComponent(GameObjectHandle object_handle, rapidjson::Value & params) {
	ComponentHandle component_handle = (ComponentHandle)components_.size();
	components_.emplace_back(object_handle, component_handle);
	auto &component = components_.back();

	if (params.HasMember("type")) {
		std::string type = params["type"].GetString();

		if (type == "plane") {
			if (params.HasMember("shape")) {
				auto shape = params["shape"].GetArray();
				component.plane_shape_[0] = shape[0].GetFloat();
				component.plane_shape_[1] = shape[1].GetFloat();
				component.plane_shape_[2] = shape[2].GetFloat();
				component.plane_shape_[3] = shape[3].GetFloat();
			}

			auto &c = component.plane_shape_;

			component.shape_ = new btStaticPlaneShape(btVector3(c[0], c[1], c[2]), c[3]);
		}
		else if (type == "sphere") {
			if (params.HasMember("radius")) {
				float shape = params["radius"].GetFloat();
				component.sphere_radius_ = shape;
			}

			component.shape_ = new btSphereShape(component.sphere_radius_);
		}
		else if (type == "capsule") {
			if (params.HasMember("radius")) {
				float radius = params["radius"].GetFloat();
				component.capsule_radius_ = radius;
			}

			if (params.HasMember("height")) {
				float height = params["height"].GetFloat();
				component.capsule_height_ = height;
			}

			component.shape_ = new btCapsuleShape(component.capsule_radius_, component.capsule_height_);
		}
		else {
			LOG_WARN("Invalid shape.");
		}
	}
	else {
		LOG_WARN("Invalid shape.");
	}

	return component_handle;
}

ColliderComponent & ColliderSubSystem::getComponent(ComponentHandle handle) {
	return components_[handle];
}

size_t ColliderSubSystem::getNumComponents() {
	return components_.size();
}

void ColliderSystem::update(double dt) {
}

void ColliderSubSystem::removeComponent(ComponentHandle handle) {
}

ColliderSubSystem::~ColliderSubSystem() {
}