#include "CameraSystem.hpp"
#include "../Core/Engine.hpp"
#include "../Core/Scene.hpp"
#include "../Core/Space.hpp"
#include "TransformSystem.hpp"
#include <glm/gtx/transform.hpp>

CameraComponent::CameraComponent(Space *space, GameObjectHandle object_handle, ComponentHandle handle) :
	camera_(space), Component(COMPONENT_CAMERA, object_handle, handle) {

	camera_.enable_reflections_ = engine.getSettings()->enable_reflections_;
	camera_.setViewport(engine.getSettings()->resolution_x_, engine.getSettings()->resolution_y_);
	camera_.initialize();
	camera_.setEnabled(true);
}

CameraSubSystem::CameraSubSystem(Space *space) : SubSystem(COMPONENT_CAMERA, space) {
}

ComponentHandle CameraSubSystem::addComponent(GameObjectHandle object_handle) {
	ComponentHandle component_handle = (ComponentHandle)components_.size();
	components_.emplace_back(space_, object_handle, component_handle);

	return component_handle;
}

CameraComponent & CameraSubSystem::getComponent(ComponentHandle handle) {
	return components_[handle];
}

Component * CameraSubSystem::getBaseComponent(ComponentHandle component_handle) {
	return &components_[component_handle];
}

size_t CameraSubSystem::getNumComponents() {
	return components_.size();
}

void CameraSubSystem::removeComponent(ComponentHandle id) {
	components_.erase(components_.begin() + id);
}

CameraSubSystem::~CameraSubSystem() {}

void CameraSystem::update() {
	GRIND_PROFILE_FUNC();

	for (auto space : engine.getSpaces()) {
		TransformSubSystem *transformsub = (TransformSubSystem *)space->getSubsystem(COMPONENT_TRANSFORM);
		CameraSubSystem *subsystem = (CameraSubSystem *)space->getSubsystem(system_type_);
		for (auto &comp : subsystem->components_) {
			ComponentHandle transformc = space->getObject(comp.game_object_handle_).getComponentHandle(COMPONENT_TRANSFORM);

			glm::vec3 pos = transformsub->getPosition(transformc);
			glm::vec3 fwd = transformsub->getForward(transformc);
			glm::vec3 up = transformsub->getUp(transformc);

			comp.camera_.setPosition(pos);
			comp.camera_.setDirections(fwd, up);
			comp.camera_.render();
		}
	}
}

void CameraSystem::loadGraphics() {
	for (auto space : engine.getSpaces()) {
		CameraSubSystem *sub = (CameraSubSystem *)space->getSubsystem(COMPONENT_CAMERA);
		for (auto &c : sub->components_) {
			c.camera_.reloadGraphics();
		}
	}
}

void CameraSystem::destroyGraphics() {
	for (auto space : engine.getSpaces()) {
		CameraSubSystem *sub = (CameraSubSystem *)space->getSubsystem(COMPONENT_CAMERA);
		for (auto &c : sub->components_) {
			c.camera_.destroyGraphics();
		}
	}
}


CameraSystem::CameraSystem() : System(COMPONENT_CAMERA) {
}

REFLECT_STRUCT_BEGIN(CameraComponent, CameraSystem, COMPONENT_CAMERA)
REFLECT_NO_SUBCAT()
REFLECT_STRUCT_END()
