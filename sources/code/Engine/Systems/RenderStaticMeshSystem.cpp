#include "Core/Engine.hpp"
#include "RenderStaticMeshSystem.hpp"
#include "TransformSystem.hpp"
#include "../Utilities/SettingsFile.hpp"
#include "../AssetManagers/ModelManager.hpp"

RenderStaticMeshComponent::RenderStaticMeshComponent(GameObjectHandle object_handle, ComponentHandle handle) :
	Component(COMPONENT_RENDER_STATIC_MESH, object_handle, handle), path_("Hi there"), model_handle_(69) {}

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

void RenderStaticMeshSubSystem::initialize() {
	for (auto &c : components_) {
		c.model_handle_ = engine.getModelManager()->preloadModel(c.handle_, c.path_);
	}
}

void RenderStaticMeshSubSystem::removeComponent(ComponentHandle id) {
	components_.erase(components_.begin() + id);
}

RenderStaticMeshComponent &RenderStaticMeshSubSystem::getComponent(ComponentHandle id) {
	return components_[id];
}

Component * RenderStaticMeshSubSystem::getBaseComponent(ComponentHandle component_handle) {
	return &components_[component_handle];
}

size_t RenderStaticMeshSubSystem::getNumComponents() {
	return components_.size();
}

void RenderStaticMeshSystem::update() {
}

RenderStaticMeshSystem::RenderStaticMeshSystem() : System(COMPONENT_RENDER_STATIC_MESH) {}

void handleRenderMeshStaticPath(void *owner) {
	RenderStaticMeshComponent *component = ((RenderStaticMeshComponent *)owner);
	if (component->model_handle_ != -1)
		engine.getModelManager()->removeModelInstance(component->model_handle_, component->handle_);
	component->model_handle_ = engine.getModelManager()->loadModel(component->handle_, component->path_);
}

REFLECT_STRUCT_BEGIN(RenderStaticMeshComponent, RenderStaticMeshSystem, COMPONENT_RENDER_STATIC_MESH)
REFLECT_STRUCT_MEMBER_D(path_, "Path to Model", "path", reflect::SaveSetAndView, handleRenderMeshStaticPath)
REFLECT_NO_SUBCAT()
REFLECT_STRUCT_END()
