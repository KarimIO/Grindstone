#include "Core/Engine.hpp"
#include "RenderSkeletalMeshSystem.hpp"
#include "TransformSystem.hpp"
#include "../Core/Engine.hpp"
#include "../Utilities/SettingsFile.hpp"
#include "../AssetManagers/ModelManager.hpp"

RenderSkeletalMeshComponent::RenderSkeletalMeshComponent(GameObjectHandle object_handle, ComponentHandle handle) :
	Component(COMPONENT_RENDER_SKELETAL_MESH, object_handle, handle) {}

RenderSkeletalMeshSubSystem::RenderSkeletalMeshSubSystem(Space *space) : SubSystem(COMPONENT_RENDER_SKELETAL_MESH, space) {
}

ComponentHandle RenderSkeletalMeshSubSystem::addComponent(GameObjectHandle object_handle) {
	ComponentHandle component_handle = (ComponentHandle)components_.size();
	components_.emplace_back(object_handle, component_handle);
	auto &component = components_.back();

	return component_handle;
}

RenderSkeletalMeshSubSystem::~RenderSkeletalMeshSubSystem() {
}

void RenderSkeletalMeshSubSystem::removeComponent(ComponentHandle id) {
	components_.erase(components_.begin() + id);
}

RenderSkeletalMeshComponent &RenderSkeletalMeshSubSystem::getComponent(ComponentHandle id) {
	return components_[id];
}

Component * RenderSkeletalMeshSubSystem::getBaseComponent(ComponentHandle component_handle) {
	return &components_[component_handle];
}

size_t RenderSkeletalMeshSubSystem::getNumComponents() {
	return components_.size();
}

void RenderSkeletalMeshSystem::update(double dt) {
}

RenderSkeletalMeshSystem::RenderSkeletalMeshSystem() : System(COMPONENT_CAMERA) {}
