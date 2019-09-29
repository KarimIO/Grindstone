#include "../Core/Engine.hpp"
#include "../Core/Scene.hpp"
#include "../Core/Space.hpp"
#include "LightDirectionalSystem.hpp"
#include "TransformSystem.hpp"

#include <GraphicsWrapper.hpp>
#include "AssetManagers/GraphicsPipelineManager.hpp"
#include "glm/gtx/transform.hpp"

#include "CameraSystem.hpp"
#include "Core/Editor.hpp"
#include <limits>

void LightDirectionalSubSystem::CalcOrthoProjs(Camera &cam, LightDirectionalComponent &comp) {
	const int NUM_FRUSTUM_CORNERS = 8;
	int NUM_CASCADES = 3;
	float m_cascadeEnd[4];

	float znear = cam.getNear();
	float zfar = cam.getFar();
	float diff = zfar - znear;
	m_cascadeEnd[0] = znear;
	m_cascadeEnd[1] = 0.25f * diff + znear,
	m_cascadeEnd[2] = 0.85f * diff + znear,
	m_cascadeEnd[3] = zfar;

	// Get the inverse of the view transform
	glm::mat4 Cam = cam.getView();
	glm::mat4 CamInv = glm::inverse(Cam);

	// Get the light space tranform

	float ar = cam.getAspectRatio();
	float tanHalfHFOV = tanf(glm::radians(cam.getFov() / 2.0f));
	float tanHalfVFOV = tanf(glm::radians((cam.getFov() * ar) / 2.0f));

	for (unsigned int i = 0; i < NUM_CASCADES; i++) {
		float xn = m_cascadeEnd[i] * tanHalfHFOV;
		float xf = m_cascadeEnd[i + 1] * tanHalfHFOV;
		float yn = m_cascadeEnd[i] * tanHalfVFOV;
		float yf = m_cascadeEnd[i + 1] * tanHalfVFOV;

		glm::vec4 frustumCorners[NUM_FRUSTUM_CORNERS] = {
			// near face
			glm::vec4(xn, yn, m_cascadeEnd[i], 1.0),
			glm::vec4(-xn, yn, m_cascadeEnd[i], 1.0),
			glm::vec4(xn, -yn, m_cascadeEnd[i], 1.0),
			glm::vec4(-xn, -yn, m_cascadeEnd[i], 1.0),

			// far face
			glm::vec4(xf, yf, m_cascadeEnd[i + 1], 1.0),
			glm::vec4(-xf, yf, m_cascadeEnd[i + 1], 1.0),
			glm::vec4(xf, -yf, m_cascadeEnd[i + 1], 1.0),
			glm::vec4(-xf, -yf, m_cascadeEnd[i + 1], 1.0)
		};

		glm::vec4 frustumCornersL[NUM_FRUSTUM_CORNERS];
#undef max
#undef min
		float minX = std::numeric_limits<float>::max();
		float maxX = std::numeric_limits<float>::min();
		float minY = std::numeric_limits<float>::max();
		float maxY = std::numeric_limits<float>::min();
		float minZ = std::numeric_limits<float>::max();
		float maxZ = std::numeric_limits<float>::min();

		for (unsigned int j = 0; j < NUM_FRUSTUM_CORNERS; j++) {

			// Transform the frustum coordinate from view to world space
			glm::vec4 vW = CamInv * frustumCorners[j];

			// Transform the frustum coordinate from world to light space
			frustumCornersL[j] = comp.view_ * vW;

			minX = glm::min(minX, frustumCornersL[j].x);
			maxX = glm::max(maxX, frustumCornersL[j].x);
			minY = glm::min(minY, frustumCornersL[j].y);
			maxY = glm::max(maxY, frustumCornersL[j].y);
			minZ = glm::min(minZ, frustumCornersL[j].z);
			maxZ = glm::max(maxZ, frustumCornersL[j].z);
		}

		comp.camera_matrices_[0][i] = glm::ortho(minX, maxX, minY, maxY, minZ, maxZ);
	}
}

LightDirectionalComponent::LightDirectionalComponent(GameObjectHandle object_handle, ComponentHandle id) : Component(COMPONENT_LIGHT_DIRECTIONAL, object_handle, id) {
	camera_matrices_.resize(1);
}

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
			TransformSubSystem * transf_sys = (TransformSubSystem *)space->getSubsystem(COMPONENT_TRANSFORM);
			for (auto &component : subsystem->components_) {
				if (component.properties_.shadow) {
					// CalculateView
					GameObjectHandle game_object_id = component.game_object_handle_;

					// Get Transform Info
					ComponentHandle transform_id = space->getObject(game_object_id).getComponentHandle(COMPONENT_TRANSFORM);
					TransformSubSystem *transform = (TransformSubSystem *)(space->getSubsystem(COMPONENT_TRANSFORM));

					// Calculate Projection
					/*component.shadow_mat_ = glm::ortho<float>(-10, 10, -10, 10, -10, 30);

					if (invert_proj)
						component.shadow_mat_[1][1] *= -1;

					if (scale_proj) {
						const glm::mat4 scale = glm::mat4(1.0f, 0, 0, 0,
							0, 1.0f, 0, 0,
							0, 0, 0.5f, 0,
							0, 0, 0.25f, 1.0f);

						component.shadow_mat_ = scale * component.shadow_mat_;
					}*/

					auto transf_comp_id = space->getObject(component.game_object_handle_).getComponentHandle(COMPONENT_TRANSFORM);
					auto dir = transf_sys->getForward(transf_comp_id);

					component.view_ = glm::lookAt(glm::vec3(0.0f), dir, glm::vec3(0.0f, 1.0f, 0.0f));

					auto cam_sys = (CameraSubSystem *)space->getSubsystem(COMPONENT_CAMERA);
					for (int c_i = 0; c_i < cam_sys->getNumComponents(); ++c_i) {
						Camera &c = cam_sys->getComponent(c_i).camera_;
						subsystem->CalcOrthoProjs(c, component);
					}

					auto editor = engine.getEditor();
					if (editor) {
						for (auto &v : editor->viewports_) {
							subsystem->CalcOrthoProjs(*v.camera_, component);
						}
					}

					// CalculateView
					/*glm::vec3 pos = transform->getPosition(transform_id);
					component.shadow_mat_ = component.shadow_mat_ * glm::lookAt(
						transform->getForward(transform_id) * 20.0f,
						glm::vec3(0, 0, 0),
						glm::vec3(0, 1, 0)
					);*/

					auto ubo = engine.getUniformBuffer();
					ubo->Bind();
					ubo->UpdateUniformBuffer(&component.camera_matrices_[0][0]);

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

ComponentHandle LightDirectionalSubSystem::addComponent(GameObjectHandle object_handle, rapidjson::Value &params) {
	ComponentHandle component_handle = (ComponentHandle)components_.size();
	components_.emplace_back(object_handle, component_handle);

	setComponent(component_handle, params);

	return component_handle;
}

void LightDirectionalSubSystem::setComponent(ComponentHandle component_handle, rapidjson::Value & params) {
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
	else
		component.properties_.shadow = false;
}

void LightDirectionalSubSystem::setShadow(ComponentHandle h, bool shadow) {
	auto &component = components_[h];

	if (shadow) {
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

void LightDirectionalSubSystem::initialize() {
	for (auto &c : components_) {
		setShadow(c.handle_, c.properties_.shadow);
	}
}

LightDirectionalComponent & LightDirectionalSubSystem::getComponent(ComponentHandle handle) {
	return components_[handle];
}

Component * LightDirectionalSubSystem::getBaseComponent(ComponentHandle component_handle) {
	return &components_[component_handle];
}

size_t LightDirectionalSubSystem::getNumComponents() {
	return components_.size();
}

void LightDirectionalSubSystem::writeComponentToJson(ComponentHandle handle, rapidjson::PrettyWriter<rapidjson::StringBuffer> & w) {
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
	w.Double(c.properties_.sourceRadius);
	
	w.Key("shadowresolution");
	w.Uint(c.properties_.resolution);

	w.Key("castshadow");
	w.Bool(c.properties_.shadow);
}

void LightDirectionalSubSystem::removeComponent(ComponentHandle handle) {
}

LightDirectionalSubSystem::~LightDirectionalSubSystem() {
}

REFLECT_STRUCT_BEGIN(LightDirectionalComponent, LightDirectionalSystem)
REFLECT_STRUCT_MEMBER(properties_.color)
REFLECT_STRUCT_MEMBER(properties_.power)
REFLECT_STRUCT_MEMBER(properties_.sourceRadius)
REFLECT_STRUCT_MEMBER_D(properties_.shadow, "Enable Shadows", "castshadow", reflect::Metadata::SaveSetAndView)
REFLECT_NO_SUBCAT()
REFLECT_STRUCT_END()
