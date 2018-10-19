#include "GameObject.hpp"

bool GameObject::operator== (GameObject &other) {
	return id_ == other.id_;
}

GameObjectHandle GameObject::getID() {
	return id_;
}

GameObject::~GameObject() {
	// Delete all Components from Systems
}