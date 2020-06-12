#include "Core/Engine.hpp"
#include "UiSystem.hpp"
#include "Rendering/Renderer2D.hpp"

#include "Core/Space.hpp"

Renderer2DManager r2dm;
Renderer2D r2d;

UiComponent::UiComponent(GameObjectHandle object_handle, ComponentHandle handle) :
	Component(COMPONENT_UI, object_handle, handle) {
	canvas_.initialize();
}

UiSubSystem::UiSubSystem(Space *space) : SubSystem(COMPONENT_UI, space) {
}

ComponentHandle UiSubSystem::addComponent(GameObjectHandle object_handle) {
	ComponentHandle component_handle = (ComponentHandle)components_.size();
	components_.emplace_back(object_handle, component_handle);
	auto &component = components_.back();

	return component_handle;
}

UiSubSystem::~UiSubSystem() {
}

void UiSubSystem::removeComponent(ComponentHandle id) {
	components_.erase(components_.begin() + id);
}

UiComponent &UiSubSystem::getComponent(ComponentHandle id) {
	return components_[id];
}

Component * UiSubSystem::getBaseComponent(ComponentHandle component_handle) {
	return &components_[component_handle];
}

size_t UiSubSystem::getNumComponents() {
	return components_.size();
}

void UiSystem::update() {
	GRIND_PROFILE_FUNC();
	for (auto space : engine.getSpaces()) {
		UiSubSystem* subsystem = (UiSubSystem*)space->getSubsystem(system_type_);

		for (auto& component : subsystem->components_) {
			//r2d.draw();
			//component.canvas_.update();
		}
	}
}

UiSystem::UiSystem() : System(COMPONENT_UI) {
	/*r2dm.initialize();
	r2d.setMaterial("../assets/materials/ui.gmat");
	r2d.resize(2);
	Quad2D &q2d = r2d.getQuadMemory(0);
	q2d.vertices[0].position = glm::vec2(-1.f, -1.f);
	q2d.vertices[1].position = glm::vec2(-1.f,  0.f);
	q2d.vertices[2].position = glm::vec2( 0.f,  0.f);
	q2d.vertices[3].position = glm::vec2( 0.f, -1.f);
	q2d.vertices[0].color = glm::vec4(1.f, 1.f, 1.f, 1.f);
	q2d.vertices[1].color = glm::vec4(1.f, 1.f, 1.f, 1.f);
	q2d.vertices[2].color = glm::vec4(1.f, 1.f, 1.f, 1.f);
	q2d.vertices[3].color = glm::vec4(1.f, 1.f, 1.f, 1.f);
	q2d.vertices[0].tex_coord = glm::vec2(0.f, 1.f);
	q2d.vertices[1].tex_coord = glm::vec2(0.f, 0.f);
	q2d.vertices[2].tex_coord = glm::vec2(1.f, 0.f);
	q2d.vertices[3].tex_coord = glm::vec2(1.f, 1.f);
	Quad2D& q2d1 = r2d.getQuadMemory(1);
	q2d1.vertices[0].position = glm::vec2(0.f, 0.f);
	q2d1.vertices[1].position = glm::vec2(0.f, 1.f);
	q2d1.vertices[2].position = glm::vec2(1.f, 1.f);
	q2d1.vertices[3].position = glm::vec2(1.f, 0.f);
	q2d1.vertices[0].color = glm::vec4(1.f, 1.f, 1.f, 1.f);
	q2d1.vertices[1].color = glm::vec4(1.f, 1.f, 1.f, 1.f);
	q2d1.vertices[2].color = glm::vec4(1.f, 1.f, 1.f, 1.f);
	q2d1.vertices[3].color = glm::vec4(1.f, 1.f, 1.f, 1.f);
	q2d1.vertices[0].tex_coord = glm::vec2(0.f, 1.f);
	q2d1.vertices[1].tex_coord = glm::vec2(0.f, 0.f);
	q2d1.vertices[2].tex_coord = glm::vec2(1.f, 0.f);
	q2d1.vertices[3].tex_coord = glm::vec2(1.f, 1.f);
	r2d.updateBuffers();*/
}

REFLECT_STRUCT_BEGIN(UiComponent, UiSystem, COMPONENT_UI)
REFLECT_NO_SUBCAT()
REFLECT_STRUCT_END()
