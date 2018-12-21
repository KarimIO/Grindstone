#include "Core/Engine.hpp"
#include "RenderStaticMeshSystem.hpp"
#include "TransformSystem.hpp"
#include "../Core/Engine.hpp"
#include "../Utilities/SettingsFile.hpp"
#include "../AssetManagers/ModelManager.hpp"

RenderStaticMeshComponent::RenderStaticMeshComponent(GameObjectHandle object_handle, ComponentHandle handle) :
	Component(COMPONENT_RENDER_STATIC_MESH, object_handle, handle) {}

RenderStaticMeshSubSystem::RenderStaticMeshSubSystem(Space *space) : SubSystem(COMPONENT_RENDER_STATIC_MESH, space) {
}

RenderStaticMeshSubSystem::~RenderStaticMeshSubSystem() {
}

ComponentHandle RenderStaticMeshSubSystem::addComponent(GameObjectHandle object_handle, rapidjson::Value &params) {
	ComponentHandle component_handle = (ComponentHandle)components_.size();
	components_.emplace_back(object_handle, component_handle);
	auto component = components_.back();

	if (params.HasMember("path")) {
		auto path = params["path"].GetString();
		component.path_ = path;
		component.model_handle_ = engine.getModelManager()->preloadModel(component_handle, path);
	}

	return component_handle;
}

void RenderStaticMeshSubSystem::removeComponent(ComponentHandle id) {
	components_.erase(components_.begin() + id);
}

RenderStaticMeshComponent &RenderStaticMeshSubSystem::getComponent(ComponentHandle id) {
	return components_[id];
}

size_t RenderStaticMeshSubSystem::getNumComponents() {
	return components_.size();
}

void RenderStaticMeshSystem::update(double dt) {
}

RenderStaticMeshSystem::RenderStaticMeshSystem() : System(COMPONENT_CAMERA) {}
