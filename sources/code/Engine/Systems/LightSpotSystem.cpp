#include "../Core/Engine.hpp"
#include "../Core/Scene.hpp"
#include "../Core/Space.hpp"
#include "LightSpotSystem.hpp"
#include "TransformSystem.hpp"

#include <GraphicsCommon/GraphicsWrapper.hpp>
#include "AssetManagers/GraphicsPipelineManager.hpp"
#include "glm/gtx/transform.hpp"

LightSpotComponent::LightSpotComponent(GameObjectHandle object_handle, ComponentHandle id) : Component(COMPONENT_LIGHT_SPOT, object_handle, id), shadow_dt_(nullptr), shadow_fbo_(nullptr) {
}

LightSpotSubSystem::LightSpotSubSystem(Space *space) : SubSystem(COMPONENT_LIGHT_SPOT, space) {}

ComponentHandle LightSpotSubSystem::addComponent(GameObjectHandle object_handle) {
	ComponentHandle component_handle = (ComponentHandle)components_.size();
	components_.emplace_back(object_handle, component_handle);
	auto &component = components_.back();

	return component_handle;
}

void LightSpotSystem::destroyGraphics() {
	for (auto space : engine.getSpaces()) {
		LightSpotSubSystem *sub = (LightSpotSubSystem *)space->getSubsystem(COMPONENT_LIGHT_SPOT);
		for (auto &c : sub->components_) {
			if (c.shadow_dt_)
				engine.getGraphicsWrapper()->deleteDepthTarget(c.shadow_dt_);
				
			if (c.shadow_fbo_)
				engine.getGraphicsWrapper()->deleteFramebuffer(c.shadow_fbo_);

			c.shadow_dt_ = nullptr;
			c.shadow_fbo_ = nullptr;
		}
	}
}

void LightSpotSystem::loadGraphics() {
	for (auto space : engine.getSpaces()) {
		LightSpotSubSystem *sub = (LightSpotSubSystem *)space->getSubsystem(COMPONENT_LIGHT_SPOT);
		for (auto &c : sub->components_) {
			c.setShadow(c.properties_.shadow);
		}
	}
}

LightSpotSystem::LightSpotSystem() : System(COMPONENT_LIGHT_SPOT) {}

void LightSpotSystem::update() {
	GRIND_PROFILE_FUNC();
	const Settings *settings = engine.getSettings();

	bool invert_proj = settings->graphics_language_ == GraphicsLanguage::Vulkan;
	bool scale_proj = settings->graphics_language_ == GraphicsLanguage::DirectX;

	double aspect = 1.0;
	double near_dist = 0.1;

	for (auto space : engine.getSpaces()) {
		LightSpotSubSystem *subsystem = (LightSpotSubSystem *)space->getSubsystem(system_type_);
		for (auto &component : subsystem->components_) {
			if (component.properties_.shadow) {
				// CalculateView
				GameObjectHandle game_object_id = component.game_object_handle_;

				// Get Transform Info
				ComponentHandle transform_id = space->getObject(game_object_id).getComponentHandle(COMPONENT_TRANSFORM);
				TransformComponent *transform = space->getObject(game_object_id).getComponent<TransformComponent>();

				// Calculate Projection
				double fov = component.properties_.outerAngle * 2.0;
				double far_dist = component.properties_.attenuationRadius;
				component.shadow_mat_ = glm::perspective(fov, aspect, near_dist, far_dist);

				if (invert_proj)
					component.shadow_mat_[1][1] *= -1;

				if (scale_proj) {
					const glm::mat4 scale = glm::mat4(1.0f, 0, 0, 0,
						0, 1.0f, 0, 0,
						0, 0, 0.5f, 0,
						0, 0, 0.25f, 1.0f);

					component.shadow_mat_ = scale * component.shadow_mat_;
				}

				// CalculateView
				glm::vec3 pos = transform->getPosition();
				component.shadow_mat_ = component.shadow_mat_ * glm::lookAt(
					pos,
					pos + transform->getForward(),
					transform->getUp()
				);

				auto ubo = engine.getUniformBuffer();
				ubo->bind();
				ubo->updateBuffer(&component.shadow_mat_);

				// Culling

				// Render
				component.shadow_fbo_->Bind(true);
				component.shadow_fbo_->Clear(Grindstone::GraphicsAPI::ClearMode::Depth);
				engine.getGraphicsWrapper()->setImmediateBlending(Grindstone::GraphicsAPI::BlendMode::None);
				engine.getGraphicsPipelineManager()->drawShadowsImmediate(0, 0, component.properties_.resolution, component.properties_.resolution);
			}
		}
	}
}

void LightSpotComponent::setShadow(bool shadow) {
	auto graphics_wrapper = engine.getGraphicsWrapper();

	if (shadow_dt_)	graphics_wrapper->deleteDepthTarget(shadow_dt_);
	if (shadow_fbo_)	graphics_wrapper->deleteFramebuffer(shadow_fbo_);

	shadow_dt_ = nullptr;
	shadow_fbo_ = nullptr;

	if (shadow) {
		Grindstone::GraphicsAPI::DepthTargetCreateInfo depth_image_ci(Grindstone::GraphicsAPI::DepthFormat::D24, properties_.resolution, properties_.resolution, true, false);
		shadow_dt_ = graphics_wrapper->createDepthTarget(depth_image_ci);

		Grindstone::GraphicsAPI::FramebufferCreateInfo fbci;
		fbci.num_render_target_lists = 0;
		fbci.render_target_lists = nullptr;
		fbci.depth_target = shadow_dt_;
		shadow_fbo_ = graphics_wrapper->createFramebuffer(fbci);
	}
}

void LightSpotSubSystem::initialize() {
	for (auto &c : components_) {
		c.setShadow(c.properties_.shadow);
	}
}

LightSpotComponent & LightSpotSubSystem::getComponent(ComponentHandle handle) {
	return components_[handle];
}

Component * LightSpotSubSystem::getBaseComponent(ComponentHandle component_handle) {
	return &components_[component_handle];
}

size_t LightSpotSubSystem::getNumComponents() {
	return components_.size();
}

void LightSpotSubSystem::removeComponent(ComponentHandle handle) {
}

LightSpotSubSystem::~LightSpotSubSystem() {
}


void handleLightPathShadow(void *owner) {
	LightSpotComponent *component = ((LightSpotComponent *)owner);
	component->setShadow(component->properties_.shadow);
}

REFLECT_STRUCT_BEGIN(LightSpotComponent, LightSpotSystem, COMPONENT_LIGHT_SPOT)
REFLECT_STRUCT_MEMBER(properties_.color)
REFLECT_STRUCT_MEMBER(properties_.power)
REFLECT_STRUCT_MEMBER(properties_.attenuationRadius)
REFLECT_STRUCT_MEMBER(properties_.innerAngle)
REFLECT_STRUCT_MEMBER(properties_.outerAngle)
REFLECT_STRUCT_MEMBER_D(properties_.shadow, "Enable Shadows", "castshadow", reflect::Metadata::SaveSetAndView, handleLightPathShadow)
REFLECT_NO_SUBCAT()
REFLECT_STRUCT_END()
