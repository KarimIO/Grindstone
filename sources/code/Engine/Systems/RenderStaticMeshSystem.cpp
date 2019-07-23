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

ComponentHandle RenderStaticMeshSubSystem::addComponent(GameObjectHandle object_handle) {
	ComponentHandle component_handle = (ComponentHandle)components_.size();
	components_.emplace_back(object_handle, component_handle);
	auto &component = components_.back();

	return component_handle;
}

RenderStaticMeshSubSystem::~RenderStaticMeshSubSystem() {
}

ComponentHandle RenderStaticMeshSubSystem::addComponent(GameObjectHandle object_handle, rapidjson::Value &params) {
	ComponentHandle component_handle = (ComponentHandle)components_.size();
	components_.emplace_back(object_handle, component_handle);

	setComponent(component_handle, params);

	return component_handle;
}

void RenderStaticMeshSubSystem::setComponent(ComponentHandle component_handle, rapidjson::Value & params) {
	auto &component = components_[component_handle];

	if (params.HasMember("path")) {
		auto path = params["path"].GetString();
		component.path_ = path;
		component.model_handle_ = engine.getModelManager()->preloadModel(component_handle, path);
	}
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

void RenderStaticMeshSubSystem::writeComponentToJson(ComponentHandle handle, rapidjson::PrettyWriter<rapidjson::StringBuffer> & w) {
	auto &c = getComponent(handle);

	w.Key("path");
	w.String(c.path_.c_str());
}

void RenderStaticMeshSystem::update(double dt) {
}

RenderStaticMeshSystem::RenderStaticMeshSystem() : System(COMPONENT_CAMERA) {}
