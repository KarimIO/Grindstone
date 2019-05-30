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

ComponentHandle RenderSkeletalMeshSubSystem::addComponent(GameObjectHandle object_handle, rapidjson::Value &params) {
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

void RenderSkeletalMeshSubSystem::removeComponent(ComponentHandle id) {
	components_.erase(components_.begin() + id);
}

RenderSkeletalMeshComponent &RenderSkeletalMeshSubSystem::getComponent(ComponentHandle id) {
	return components_[id];
}

size_t RenderSkeletalMeshSubSystem::getNumComponents() {
	return components_.size();
}

void RenderSkeletalMeshSubSystem::writeComponentToJson(ComponentHandle handle, rapidjson::PrettyWriter<rapidjson::StringBuffer> & w) {
	auto &c = getComponent(handle);

	w.Key("path");
	w.String(c.path_.c_str());
}

void RenderSkeletalMeshSystem::update(double dt) {
}

RenderSkeletalMeshSystem::RenderSkeletalMeshSystem() : System(COMPONENT_CAMERA) {}
