#include "Core/Engine.hpp"
#include "RenderInstancedMeshSystem.hpp"
#include "TransformSystem.hpp"
#include "../Core/Engine.hpp"
#include "../Utilities/SettingsFile.hpp"
#include "../AssetManagers/ModelManager.hpp"

RenderInstancedMeshComponent::RenderInstancedMeshComponent(GameObjectHandle object_handle, ComponentHandle handle) :
	Component(COMPONENT_RENDER_INSTANCED_MESH, object_handle, handle) {}

RenderInstancedMeshSubSystem::RenderInstancedMeshSubSystem(Space *space) : SubSystem(COMPONENT_RENDER_INSTANCED_MESH, space) {
}

ComponentHandle RenderInstancedMeshSubSystem::addComponent(GameObjectHandle object_handle) {
	ComponentHandle component_handle = (ComponentHandle)components_.size();
	components_.emplace_back(object_handle, component_handle);
	auto &component = components_.back();

	return component_handle;
}

RenderInstancedMeshSubSystem::~RenderInstancedMeshSubSystem() {
}

void RenderInstancedMeshSubSystem::removeComponent(ComponentHandle id) {
	components_.erase(components_.begin() + id);
}

RenderInstancedMeshComponent &RenderInstancedMeshSubSystem::getComponent(ComponentHandle id) {
	return components_[id];
}

Component * RenderInstancedMeshSubSystem::getBaseComponent(ComponentHandle component_handle) {
	return &components_[component_handle];
}

size_t RenderInstancedMeshSubSystem::getNumComponents() {
	return components_.size();
}

void RenderInstancedMeshSystem::update() {
}

RenderInstancedMeshSystem::RenderInstancedMeshSystem() : System(COMPONENT_CAMERA) {}
