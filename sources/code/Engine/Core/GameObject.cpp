#include "GameObject.hpp"
#include <climits>

#include "Space.hpp"

GameObject::GameObject(const GameObject & g) {
	id_ = g.id_;
	name_ = g.name_;

	memcpy(components_, g.components_, sizeof(components_));
}

GameObject::GameObject(GameObjectHandle id, std::string name, Space *space, GameObjectHandle parent) : id_(id), name_(name), parent_(parent), space_(space) {
	memset(components_, UINT_MAX, sizeof(components_));
}

bool GameObject::operator== (GameObject &other) {
	return id_ == other.id_ && space_ == other.space_;
}

template<typename ComponentT>
ComponentT *GameObject::createComponent() {
	ComponentType t = T::getComponentType();
	return createComponent(t);
}

Component *GameObject::createComponent(ComponentType type) {
	SubSystem* subsys = space_->getSubsystem(type);

	if (subsys)
		return subsys->getBaseComponent(subsys->addComponent(id_));
	else
		nullptr;
}

void GameObject::setComponentHandle(ComponentType type, ComponentHandle handle) {
	components_[type] = handle;
}

ComponentHandle GameObject::getComponentHandle(ComponentType type) {
	return components_[type];
}

GameObjectHandle GameObject::getID() {
	return id_;
}

GameObjectHandle GameObject::getParentID() {
	return parent_;
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