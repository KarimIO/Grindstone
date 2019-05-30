#include "GameObject.hpp"
#include <climits>

GameObject::GameObject(const GameObject & g) {
	id_ = g.id_;
	name_ = g.name_;

	memcpy(components_, g.components_, sizeof(components_));
}

GameObject::GameObject(GameObjectHandle id, std::string name) : id_(id), name_(name) {
	memset(components_, UINT_MAX, sizeof(components_));
}

bool GameObject::operator== (GameObject &other) {
	return id_ == other.id_;
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

std::string GameObject::getName() {
	return name_;
}

void GameObject::setName(std::string str) {
	name_ = str;
}

GameObject::~GameObject() {
	// Delete all Components from Systems
}