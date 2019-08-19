#include "ColliderSystem.hpp"
#include <btBulletDynamicsCommon.h>
#include "../Utilities/Logger.hpp"
#include "../Core/Space.hpp"
#include "TransformSystem.hpp"

ColliderComponent::ColliderComponent(GameObjectHandle object_handle, ComponentHandle id) : Component(COMPONENT_COLLISION, object_handle, id) {}

ColliderSystem::ColliderSystem() : System(COMPONENT_COLLISION) {}

ColliderSubSystem::ColliderSubSystem(Space *space) : SubSystem(COMPONENT_COLLISION, space) {}

ComponentHandle ColliderSubSystem::addComponent(GameObjectHandle object_handle) {
	ComponentHandle component_handle = (ComponentHandle)components_.size();
	components_.emplace_back(object_handle, component_handle);

	return component_handle;
}

ComponentHandle ColliderSubSystem::addComponent(GameObjectHandle object_handle, rapidjson::Value &params) {
	ComponentHandle component_handle = (ComponentHandle)components_.size();
	components_.emplace_back(object_handle, component_handle);

	setComponent(component_handle, params);

	return component_handle;
}

void ColliderSubSystem::setComponent(ComponentHandle component_handle, rapidjson::Value & params) {
	auto &component = components_[component_handle];
	auto object_handle = component.game_object_handle_;

	ComponentHandle transform_id = space_->getObject(object_handle).getComponentHandle(COMPONENT_TRANSFORM);
	TransformSubSystem *transform_sub = (TransformSubSystem *)(space_->getSubsystem(COMPONENT_TRANSFORM));
	TransformComponent &transform_component = transform_sub->getComponent(transform_id);

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
		else if (type == "box") {
			if (params.HasMember("offset")) {
				auto shape = params["offset"].GetArray();
				component.box_offset_[0] = shape[0].GetFloat();
				component.box_offset_[1] = shape[1].GetFloat();
				component.box_offset_[2] = shape[2].GetFloat();
			}
			else {
				component.box_offset_[0] = 0.0f;
				component.box_offset_[1] = 0.0f;
				component.box_offset_[2] = 0.0f;
			}

			if (params.HasMember("size")) {
				auto shape = params["size"].GetArray();
				component.box_size_[0] = shape[0].GetFloat();
				component.box_size_[1] = shape[2].GetFloat();
				component.box_size_[2] = shape[1].GetFloat();
			}
			else {
				component.box_offset_[0] = 1.0f;
				component.box_offset_[1] = 1.0f;
				component.box_offset_[2] = 1.0f;
			}

			auto &c = component.plane_shape_;

			component.shape_ = new btBoxShape(btVector3(component.box_size_[0] / 2.0f, component.box_size_[1] / 2.0f, component.box_size_[2] / 2.0f));
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
			GRIND_WARN("Invalid shape.");
		}
	}
	else {
		GRIND_WARN("Invalid shape.");
	}
	
	glm::vec3 scale = transform_component.scale_;

	component.shape_->setUserIndex(component.game_object_handle_);
	// setUserPointer((void*)rigidBody);
	component.shape_->setLocalScaling(btVector3(scale.x, scale.y, scale.z));
}

ColliderComponent & ColliderSubSystem::getComponent(ComponentHandle handle) {
	return components_[handle];
}

Component * ColliderSubSystem::getBaseComponent(ComponentHandle component_handle) {
	return &components_[component_handle];
}

size_t ColliderSubSystem::getNumComponents() {
	return components_.size();
}

void ColliderSubSystem::writeComponentToJson(ComponentHandle handle, rapidjson::PrettyWriter<rapidjson::StringBuffer> & w) {
	auto &c = getComponent(handle);
	switch (c.shape_type_) {
	case ColliderComponent::ShapeType::CAPSULE: {
		w.Key("type");
		w.String("capsule");

		w.Key("shape");
		w.Double(c.capsule_radius_);
		w.Key("height");
		w.Double(c.capsule_height_);
		break;
	}
	case ColliderComponent::ShapeType::PLANE: {
		w.Key("type");
		w.String("plane");

		w.Key("shape");
		w.StartArray();
		w.Double(c.plane_shape_[0]);
		w.Double(c.plane_shape_[1]);
		w.Double(c.plane_shape_[2]);
		w.Double(c.plane_shape_[3]);
		w.EndArray();
		break;
	}
	case ColliderComponent::ShapeType::BOX: {
		w.Key("type");
		w.String("box");

		w.Key("offset");
		w.StartArray();
		w.Double(c.box_offset_[0]);
		w.Double(c.box_offset_[1]);
		w.Double(c.box_offset_[2]);
		w.EndArray();

		w.Key("size");
		w.StartArray();
		w.Double(c.box_size_[0]);
		w.Double(c.box_size_[1]);
		w.Double(c.box_size_[2]);
		w.EndArray();
		break;
	}
	case ColliderComponent::ShapeType::SPHERE: {
		w.Key("sphere");
		w.String("plane");

		w.Key("radius");
		w.Double(c.sphere_radius_);
		break;
	}
	default:
		GRIND_WARN("Invalid shape.");
		break;
	}
}

void ColliderSystem::update(double dt) {
}

void ColliderSubSystem::removeComponent(ComponentHandle handle) {
}

ColliderSubSystem::~ColliderSubSystem() {
}