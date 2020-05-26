#include "../Core/Engine.hpp"
#include "../Core/Scene.hpp"
#include "../Core/Space.hpp"
#include "LightDirectionalSystem.hpp"
#include "TransformSystem.hpp"

#include <GraphicsCommon/GraphicsWrapper.hpp>
#include "AssetManagers/GraphicsPipelineManager.hpp"
#include "glm/gtx/transform.hpp"

#include "CameraSystem.hpp"
#include <limits>

void LightDirectionalSubSystem::CalcOrthoProjs(Camera &cam, LightDirectionalComponent &comp) {
	const unsigned int NUM_FRUSTUM_CORNERS = 8;
	unsigned int NUM_CASCADES = 3;
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

	for (unsigned int i = 0u; i < NUM_CASCADES; i++) {
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

LightDirectionalComponent::LightDirectionalComponent(GameObjectHandle object_handle, ComponentHandle id) : Component(COMPONENT_LIGHT_DIRECTIONAL, object_handle, id), shadow_dt_(nullptr), shadow_fbo_(nullptr) {
	camera_matrices_.resize(1);
}

LightDirectionalSubSystem::LightDirectionalSubSystem(Space *space) : SubSystem(COMPONENT_LIGHT_DIRECTIONAL, space) {}

ComponentHandle LightDirectionalSubSystem::addComponent(GameObjectHandle object_handle) {
	ComponentHandle component_handle = (ComponentHandle)components_.size();
	components_.emplace_back(object_handle, component_handle);

	return component_handle;
}

LightDirectionalSystem::LightDirectionalSystem() : System(COMPONENT_LIGHT_DIRECTIONAL) {}

void LightDirectionalSystem::update() {
	GRIND_PROFILE_FUNC();
	const Settings *settings = engine.getSettings();
	auto graphics_wrapper = engine.getGraphicsWrapper();

	bool invert_proj = settings->graphics_language_ == GraphicsLanguage::Vulkan;
	bool scale_proj = settings->graphics_language_ == GraphicsLanguage::DirectX;

	double aspect = 1.0;
	double near_dist = 0.1;

	for (auto space : engine.getSpaces()) {
		LightDirectionalSubSystem *subsystem = (LightDirectionalSubSystem *)space->getSubsystem(system_type_);
		TransformSubSystem * transf_sys = (TransformSubSystem *)space->getSubsystem(COMPONENT_TRANSFORM);
		for (auto &component : subsystem->components_) {
			if (component.properties_.shadow) {
				// CalculateView
				GameObjectHandle game_object_id = component.game_object_handle_;

				// Get Transform Info
				TransformComponent *transform = space->getObject(game_object_id).getComponent<TransformComponent>();
				
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
				auto dir = transform->getForward();

				component.view_ = glm::lookAt(glm::vec3(0.0f), dir, glm::vec3(0.0f, 1.0f, 0.0f));

				auto cam_sys = (CameraSubSystem *)space->getSubsystem(COMPONENT_CAMERA);
				for (int c_i = 0; c_i < cam_sys->getNumComponents(); ++c_i) {
					Camera &c = cam_sys->getComponent(c_i).camera_;
					subsystem->CalcOrthoProjs(c, component);
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
				ubo->updateBuffer(&component.camera_matrices_[0][0]);

				// Culling

				// Render
				component.shadow_fbo_->Bind(true);
				component.shadow_fbo_->Clear(Grindstone::GraphicsAPI::ClearMode::Depth);
				graphics_wrapper->setImmediateBlending(Grindstone::GraphicsAPI::BlendMode::None);
				engine.getGraphicsPipelineManager()->drawShadowsImmediate(0, 0, component.properties_.resolution, component.properties_.resolution);
			}
		}
	}
}

void LightDirectionalSystem::destroyGraphics() {
	for (auto space : engine.getSpaces()) {
		LightDirectionalSubSystem *sub = (LightDirectionalSubSystem *)space->getSubsystem(COMPONENT_LIGHT_DIRECTIONAL);
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

void LightDirectionalSystem::loadGraphics() {
	for (auto space : engine.getSpaces()) {
		LightDirectionalSubSystem *sub = (LightDirectionalSubSystem *)space->getSubsystem(COMPONENT_LIGHT_DIRECTIONAL);
		for (auto &c : sub->components_) {
			sub->setShadow(c.game_object_handle_, c.properties_.shadow);
		}
	}
}

void LightDirectionalSubSystem::setShadow(ComponentHandle h, bool shadow) {
	auto graphics_wrapper = engine.getGraphicsWrapper();
	auto &component = components_[h];

	if (component.shadow_dt_)	graphics_wrapper->deleteDepthTarget(component.shadow_dt_);
	if (component.shadow_fbo_)	graphics_wrapper->deleteFramebuffer(component.shadow_fbo_);

	component.shadow_dt_ = nullptr;
	component.shadow_fbo_ = nullptr;

	if (shadow) {

		Grindstone::GraphicsAPI::DepthTargetCreateInfo depth_image_ci(Grindstone::GraphicsAPI::DepthFormat::D24, component.properties_.resolution, component.properties_.resolution, true, false);
		component.shadow_dt_ = graphics_wrapper->createDepthTarget(depth_image_ci);

		Grindstone::GraphicsAPI::FramebufferCreateInfo fbci;
		fbci.num_render_target_lists = 0;
		fbci.render_target_lists = nullptr;
		fbci.depth_target = component.shadow_dt_;
		component.shadow_fbo_ = graphics_wrapper->createFramebuffer(fbci);
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

void LightDirectionalSubSystem::removeComponent(ComponentHandle handle) {
}

LightDirectionalSubSystem::~LightDirectionalSubSystem() {
}

REFLECT_STRUCT_BEGIN(LightDirectionalComponent, LightDirectionalSystem, COMPONENT_LIGHT_DIRECTIONAL)
REFLECT_STRUCT_MEMBER(properties_.color)
REFLECT_STRUCT_MEMBER(properties_.power)
REFLECT_STRUCT_MEMBER(properties_.sourceRadius)
REFLECT_STRUCT_MEMBER_D(properties_.shadow, "Enable Shadows", "castshadow", reflect::Metadata::SaveSetAndView, nullptr)
REFLECT_NO_SUBCAT()
REFLECT_STRUCT_END()
