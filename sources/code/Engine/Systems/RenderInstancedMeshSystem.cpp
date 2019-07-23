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

ComponentHandle RenderInstancedMeshSubSystem::addComponent(GameObjectHandle object_handle, rapidjson::Value &params) {
	ComponentHandle component_handle = (ComponentHandle)components_.size();
	components_.emplace_back(object_handle, component_handle);

	setComponent(component_handle, params);

	return component_handle;
}

void RenderInstancedMeshSubSystem::setComponent(ComponentHandle component_handle, rapidjson::Value & params) {
	auto &component = components_[component_handle];
	// engine.getModelManager()->preloadModel(component_handle, path);

	/*if (params.HasMember("path")) {
		auto path = params["path"].GetString();
		component.path_ = path;
		component.model_handle_ = engine.getModelManager()->preloadModel(component_handle, path);
	}*/
}

void RenderInstancedMeshSubSystem::removeComponent(ComponentHandle id) {
	components_.erase(components_.begin() + id);
}

RenderInstancedMeshComponent &RenderInstancedMeshSubSystem::getComponent(ComponentHandle id) {
	return components_[id];
}

size_t RenderInstancedMeshSubSystem::getNumComponents() {
	return components_.size();
}

void RenderInstancedMeshSubSystem::writeComponentToJson(ComponentHandle handle, rapidjson::PrettyWriter<rapidjson::StringBuffer> & w) {
	auto &c = getComponent(handle);

	w.Key("path");
	w.String(c.path_.c_str());
}

void RenderInstancedMeshSystem::update(double dt) {
}

RenderInstancedMeshSystem::RenderInstancedMeshSystem() : System(COMPONENT_CAMERA) {}
