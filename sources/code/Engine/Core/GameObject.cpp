#include "GameObject.hpp"
#include <climits>

#include "Space.hpp"

GameObject::GameObject(const GameObject & g) {
	id_ = g.id_;
	name_ = g.name_;
	space_ = g.space_;

	memcpy(components_, g.components_, sizeof(components_));
}

GameObject::GameObject(GameObjectHandle id, std::string name, Space *space) : id_(id), name_(name), space_(space) {
	memset(components_, UINT_MAX, sizeof(components_));
}

bool GameObject::operator== (GameObject &other) {
	return id_ == other.id_ && space_ == other.space_;
}

Component* GameObject::createComponent(ComponentType type) {
	SubSystem* subsys = space_->getSubsystem(type);

	if (subsys) {
		ComponentHandle comp = subsys->addComponent(id_);
		components_[type] = comp;
		return subsys->getBaseComponent(comp);
	}
	else {
		return nullptr;
	}
}

void GameObject::setComponentHandle(ComponentType type, ComponentHandle handle) {
	components_[type] = handle;
}

ComponentHandle GameObject::getComponentHandle(ComponentType type) {
	return components_[type];
}

Component* GameObject::getComponent(ComponentType type) {
	SubSystem* subsys = space_->getSubsystem(type);

	if (subsys) {
		ComponentHandle handle = components_[type];
		Component* component = subsys->getBaseComponent(handle);
		return component;
	}
	else
		return nullptr;
}

GameObjectHandle GameObject::getID() {
	return id_;
}

std::string GameObject::getName() {
	return name_;
}

void GameObject::setName(std::string str) {
	name_ = str;
}

void GameObject::removeComponent(ComponentType component_type) {
	ComponentHandle &component_handle = components_[component_type];

	component_handle = UINT_MAX;
}

void GameObject::removeAllComponents() {
	for (int i = 0; i < NUM_COMPONENTS - 1; ++i) {
		ComponentType component_type = ComponentType(i);
		removeComponent(component_type);
	}
}

GameObject::~GameObject() {
	// Delete all Components from Systems
}