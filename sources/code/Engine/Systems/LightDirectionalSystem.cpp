#include "../Core/Engine.hpp"
#include "../Core/Scene.hpp"
#include "../Core/Space.hpp"
#include "LightDirectionalSystem.hpp"
#include "TransformSystem.hpp"

#include <GraphicsWrapper.hpp>
#include "AssetManagers/GraphicsPipelineManager.hpp"
#include "glm/gtx/transform.hpp"

LightDirectionalComponent::LightDirectionalComponent(GameObjectHandle object_handle, ComponentHandle id) : Component(COMPONENT_LIGHT_DIRECTIONAL, object_handle, id) {}

LightDirectionalSubSystem::LightDirectionalSubSystem(Space *space) : SubSystem(COMPONENT_LIGHT_DIRECTIONAL, space) {}

ComponentHandle LightDirectionalSubSystem::addComponent(GameObjectHandle object_handle) {
	ComponentHandle component_handle = (ComponentHandle)components_.size();
	components_.emplace_back(object_handle, component_handle);

	return component_handle;
}

LightDirectionalSystem::LightDirectionalSystem() : System(COMPONENT_LIGHT_DIRECTIONAL) {}

void LightDirectionalSystem::update(double dt) {
	const Settings *settings = engine.getSettings();
	auto graphics_wrapper = engine.getGraphicsWrapper();

	bool invert_proj = settings->graphics_language_ == GRAPHICS_VULKAN;
	bool scale_proj = settings->graphics_language_ == GRAPHICS_DIRECTX;

	double aspect = 1.0;
	double near_dist = 0.1;

	auto &scenes = engine.getScenes();
	for (auto scene : scenes) {
		for (auto space : scene->spaces_) {
			LightDirectionalSubSystem *subsystem = (LightDirectionalSubSystem *)space->getSubsystem(system_type_);
			for (auto &component : subsystem->components_) {
				if (component.properties_.shadow) {
					// CalculateView
					GameObjectHandle game_object_id = component.game_object_handle_;

					// Get Transform Info
					ComponentHandle transform_id = space->getObject(game_object_id).getComponentHandle(COMPONENT_TRANSFORM);
					TransformSubSystem *transform = (TransformSubSystem *)(space->getSubsystem(COMPONENT_TRANSFORM));

					// Calculate Projection
					component.shadow_mat_ = glm::ortho<float>(-10, 10, -10, 10, -10, 30);

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
					glm::vec3 pos = transform->getPosition(transform_id);
					component.shadow_mat_ = component.shadow_mat_ * glm::lookAt(
						transform->getForward(transform_id) * 20.0f,
						glm::vec3(0, 0, 0),
						glm::vec3(0, 1, 0)
					);

					auto ubo = engine.getUniformBuffer();
					ubo->Bind();
					ubo->UpdateUniformBuffer(&component.shadow_mat_);

					// Culling

					// Render
					component.shadow_fbo_->Bind(true);
					component.shadow_fbo_->Clear(CLEAR_DEPTH);
					graphics_wrapper->SetImmediateBlending(BLEND_NONE);
					engine.getGraphicsPipelineManager()->drawShadowsImmediate(0, 0, component.properties_.resolution, component.properties_.resolution);
				}
			}
		}
	}
}

ComponentHandle LightDirectionalSubSystem::addComponent(GameObjectHandle object_handle, rapidjson::Value & params) {
	ComponentHandle component_handle = (ComponentHandle)components_.size();
	components_.emplace_back(object_handle, component_handle);
	auto &component = components_.back();

	if (params.HasMember("color")) {
		auto color = params["color"].GetArray();
		component.properties_.color.x = color[0].GetFloat();
		component.properties_.color.y = color[1].GetFloat();
		component.properties_.color.z = color[2].GetFloat();
	}

	if (params.HasMember("brightness")) {
		component.properties_.power = params["brightness"].GetFloat();
	}

	if (params.HasMember("radius")) {
		component.properties_.sourceRadius = params["radius"].GetFloat();
	}

	if (params.HasMember("shadowresolution")) {
		component.properties_.resolution = params["shadowresolution"].GetUint();
	}
	else {
		component.properties_.resolution = 2048;
	}

	if (params.HasMember("castshadow")) {
		component.properties_.shadow = params["castshadow"].GetBool();

		if (component.properties_.shadow) {
			auto graphics_wrapper = engine.getGraphicsWrapper();

			DepthTargetCreateInfo depth_image_ci(FORMAT_DEPTH_24, component.properties_.resolution, component.properties_.resolution, true, false);
			component.shadow_dt_ = graphics_wrapper->CreateDepthTarget(depth_image_ci);

			FramebufferCreateInfo fbci;
			fbci.num_render_target_lists = 0;
			fbci.render_target_lists = nullptr;
			fbci.depth_target = component.shadow_dt_;
			component.shadow_fbo_ = graphics_wrapper->CreateFramebuffer(fbci);
		}
	}

	return component_handle;
}

LightDirectionalComponent & LightDirectionalSubSystem::getComponent(ComponentHandle handle) {
	return components_[handle];
}

size_t LightDirectionalSubSystem::getNumComponents() {
	return components_.size();
}

void LightDirectionalSubSystem::removeComponent(ComponentHandle handle) {
}

LightDirectionalSubSystem::~LightDirectionalSubSystem() {
}
