#include "GameObject.hpp"

GameObject::GameObject(GameObjectHandle id, std::string name) : id_(id), name_(name) {
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

GameObject::~GameObject() {
	// Delete all Components from Systems
}