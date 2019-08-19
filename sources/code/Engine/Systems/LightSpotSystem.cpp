#include "../Core/Engine.hpp"
#include "../Core/Scene.hpp"
#include "../Core/Space.hpp"
#include "LightSpotSystem.hpp"
#include "TransformSystem.hpp"

#include <GraphicsWrapper.hpp>
#include "AssetManagers/GraphicsPipelineManager.hpp"
#include "glm/gtx/transform.hpp"

LightSpotComponent::LightSpotComponent(GameObjectHandle object_handle, ComponentHandle id) : Component(COMPONENT_LIGHT_SPOT, object_handle, id) {}

LightSpotSubSystem::LightSpotSubSystem(Space *space) : SubSystem(COMPONENT_LIGHT_SPOT, space) {}

ComponentHandle LightSpotSubSystem::addComponent(GameObjectHandle object_handle) {
	ComponentHandle component_handle = (ComponentHandle)components_.size();
	components_.emplace_back(object_handle, component_handle);
	auto &component = components_.back();

	return component_handle;
}

LightSpotSystem::LightSpotSystem() : System(COMPONENT_LIGHT_SPOT) {}

void LightSpotSystem::update(double dt) {
	const Settings *settings = engine.getSettings();

	bool invert_proj = settings->graphics_language_ == GRAPHICS_VULKAN;
	bool scale_proj = settings->graphics_language_ == GRAPHICS_DIRECTX;

	double aspect = 1.0;
	double near_dist = 0.1;

	auto &scenes = engine.getScenes();
	for (auto scene : scenes) {
		for (auto space : scene->spaces_) {
			LightSpotSubSystem *subsystem = (LightSpotSubSystem *)space->getSubsystem(system_type_);
			for (auto &component : subsystem->components_) {
				if (component.properties_.shadow) {
					// CalculateView
					GameObjectHandle game_object_id = component.game_object_handle_;

					// Get Transform Info
					ComponentHandle transform_id = space->getObject(game_object_id).getComponentHandle(COMPONENT_TRANSFORM);
					TransformSubSystem *transform = (TransformSubSystem *)(space->getSubsystem(COMPONENT_TRANSFORM));

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
					glm::vec3 pos = transform->getPosition(transform_id);
					component.shadow_mat_ = component.shadow_mat_ * glm::lookAt(
						pos,
						pos + transform->getForward(transform_id),
						transform->getUp(transform_id)
					);

					auto ubo = engine.getUniformBuffer();
					ubo->Bind();
					ubo->UpdateUniformBuffer(&component.shadow_mat_);

					// Culling

					// Render
					component.shadow_fbo_->Bind(true);
					component.shadow_fbo_->Clear(CLEAR_DEPTH);
					engine.getGraphicsWrapper()->SetImmediateBlending(BLEND_NONE);
					engine.getGraphicsPipelineManager()->drawShadowsImmediate(0, 0, component.properties_.resolution, component.properties_.resolution);
				}
			}
		}
	}
}

ComponentHandle LightSpotSubSystem::addComponent(GameObjectHandle object_handle, rapidjson::Value &params) {
	ComponentHandle component_handle = (ComponentHandle)components_.size();
	components_.emplace_back(object_handle, component_handle);

	setComponent(component_handle, params);

	return component_handle;
}

void LightSpotSubSystem::setComponent(ComponentHandle component_handle, rapidjson::Value & params) {
	auto &component = components_[component_handle];

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
		component.properties_.attenuationRadius = params["radius"].GetFloat();
	}

	if (params.HasMember("innerAngle")) {
		component.properties_.innerAngle = params["innerAngle"].GetFloat();
	}

	if (params.HasMember("outerAngle")) {
		component.properties_.outerAngle = params["outerAngle"].GetFloat();
	}

	if (params.HasMember("shadowresolution")) {
		component.properties_.resolution = params["shadowresolution"].GetUint();
	}
	else {
		component.properties_.resolution = 512;
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
	else
		component.properties_.shadow = false;
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

void LightSpotSubSystem::writeComponentToJson(ComponentHandle handle, rapidjson::PrettyWriter<rapidjson::StringBuffer> & w) {
	auto &c = getComponent(handle);

	w.Key("color");
	w.StartArray();
	w.Double(c.properties_.color.x);
	w.Double(c.properties_.color.y);
	w.Double(c.properties_.color.z);
	w.EndArray();

	w.Key("brightness");
	w.Double(c.properties_.power);
	
	w.Key("radius");
	w.Double(c.properties_.attenuationRadius);

	w.Key("innerAngle");
	w.Double(c.properties_.innerAngle);

	w.Key("outerAngle");
	w.Double(c.properties_.outerAngle);

	w.Key("shadowresolution");
	w.Int(c.properties_.resolution);

	w.Key("castshadow");
	w.Bool(c.properties_.shadow);
}

void LightSpotSubSystem::removeComponent(ComponentHandle handle) {
}

LightSpotSubSystem::~LightSpotSubSystem() {
}
