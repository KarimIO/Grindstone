#include "SLight.hpp"
#include "../Core/Engine.hpp"
#include <glm/gtx/transform.hpp>
#include "CubeInfo.hpp"
#include <limits>

CPointLight::CPointLight(unsigned int entityID) {
	this->entityID = entityID;

	UniformBufferCreateInfo lightuboci;
	lightuboci.isDynamic = false;
	lightuboci.size = sizeof(LightPointUBO);
	lightuboci.binding = engine.pointLightUBB;
	lightUBO = engine.graphics_wrapper_->CreateUniformBuffer(lightuboci);
}

CPointLight::CPointLight(unsigned int entID, glm::vec3 color, float strength, bool cast, float radius) {
	lightUBOBuffer.attenuationRadius = radius;
	lightUBOBuffer.color = color;
	lightUBOBuffer.power = strength;
	lightUBOBuffer.shadow = cast;

	UniformBufferCreateInfo lightuboci;
	lightuboci.isDynamic = false;
	lightuboci.size = sizeof(LightPointUBO);
	lightuboci.binding = engine.pointLightUBB;
	lightUBO = engine.graphics_wrapper_->CreateUniformBuffer(lightuboci);

	castShadow = cast;

	int res = 1024;
	
	if (castShadow) {
		DepthTargetCreateInfo depth_image_ci(FORMAT_DEPTH_24, res, res, true, true);
		shadow_db_ = engine.graphics_wrapper_->CreateDepthTarget(depth_image_ci);

		FramebufferCreateInfo fbci;
		fbci.num_render_target_lists = 0;
		fbci.render_target_lists = nullptr;
		fbci.depth_target = shadow_db_;
		shadowFBO = engine.graphics_wrapper_->CreateFramebuffer(fbci);
	}
}

void CPointLight::SetShadow(bool state) {
	castShadow = state;
	lightUBOBuffer.shadow = castShadow;

	int res = 1024;
	
	if (castShadow) {
		DepthTargetCreateInfo depth_image_ci(FORMAT_DEPTH_24, res, res, true, true);
		shadow_db_ = engine.graphics_wrapper_->CreateDepthTarget(depth_image_ci);

		FramebufferCreateInfo fbci;
		fbci.num_render_target_lists = 0;
		fbci.render_target_lists = nullptr;
		fbci.depth_target = shadow_db_;
		shadowFBO = engine.graphics_wrapper_->CreateFramebuffer(fbci);
	}
}

void CPointLight::Bind() {
	if (castShadow) {
		shadowFBO->BindRead();
		shadowFBO->BindTextures(4);
	}

	Entity *entity = &engine.entities[entityID];
	unsigned int transID = entity->components_[COMPONENT_TRANSFORM];
	CTransform *trans = &engine.transformSystem.components[transID];
	lightUBOBuffer.position = trans->position;
	lightUBO->UpdateUniformBuffer(&lightUBOBuffer);
	lightUBO->Bind();
}

CSpotLight::CSpotLight(unsigned int entityID) {
	this->entityID = entityID;

	UniformBufferCreateInfo lightuboci;
	lightuboci.isDynamic = false;
	lightuboci.size = sizeof(LightSpotUBO);
	lightuboci.binding = engine.spotLightUBB;
	lightUBO = engine.graphics_wrapper_->CreateUniformBuffer(lightuboci);
}

CSpotLight::CSpotLight(unsigned int entID, glm::vec3 color, float strength, bool cast, float radius, float ia, float oa) {
	lightUBOBuffer.innerAngle = ia * 3.14159265359f / 360.0f;
	lightUBOBuffer.outerAngle = oa * 3.14159265359f / 360.0f;
	lightUBOBuffer.attenuationRadius = radius;
	lightUBOBuffer.color = color;
	lightUBOBuffer.power = strength;
	lightUBOBuffer.shadow = cast;

	castShadow = cast;

	UniformBufferCreateInfo lightuboci;
	lightuboci.isDynamic = false;
	lightuboci.size = sizeof(LightSpotUBO);
	lightuboci.binding = engine.spotLightUBB;
	lightUBO = engine.graphics_wrapper_->CreateUniformBuffer(lightuboci);
	if (cast) {
		DepthTargetCreateInfo depth_image_ci(FORMAT_DEPTH_24, 1024, 1024, true, false);
		shadow_db_ = engine.graphics_wrapper_->CreateDepthTarget(depth_image_ci);

		FramebufferCreateInfo fbci;
		fbci.num_render_target_lists = 0;
		fbci.render_target_lists = nullptr;
		fbci.depth_target = shadow_db_;
		shadowFBO = engine.graphics_wrapper_->CreateFramebuffer(fbci);
	}
}

void CSpotLight::SetShadow(bool state) {
	castShadow = state;
	lightUBOBuffer.shadow = state;

	if (state) {
		DepthTargetCreateInfo depth_image_ci(FORMAT_DEPTH_24, 1024, 1024, true, false);
		shadow_db_ = engine.graphics_wrapper_->CreateDepthTarget(depth_image_ci);

		FramebufferCreateInfo fbci;
		fbci.num_render_target_lists = 0;
		fbci.render_target_lists = nullptr;
		fbci.depth_target = shadow_db_;
		shadowFBO = engine.graphics_wrapper_->CreateFramebuffer(fbci);
	}
}

void CSpotLight::Bind() {
	if (castShadow) {
		shadowFBO->BindRead();
		shadowFBO->BindTextures(4);
		lightUBOBuffer.shadow_mat = calculateMatrixBasis();
	}

	Entity *entity = &engine.entities[entityID];
	unsigned int transID = entity->components_[COMPONENT_TRANSFORM];
	CTransform *trans = &engine.transformSystem.components[transID];
	lightUBOBuffer.position = trans->GetPosition();
	lightUBOBuffer.direction = trans->GetForward();
	lightUBO->UpdateUniformBuffer(&lightUBOBuffer);
	lightUBO->Bind();
}

glm::mat4 CSpotLight::calculateMatrix() {
	double camFar = lightUBOBuffer.attenuationRadius;
	double fov = lightUBOBuffer.outerAngle * 2.0;
	glm::mat4 projection = glm::perspective(fov, 1.0, 0.1, camFar);
	if (engine.settings.graphicsLanguage == GRAPHICS_VULKAN)
		projection[1][1] *= -1;

	if (engine.settings.graphicsLanguage == GRAPHICS_DIRECTX) {
		const glm::mat4 scale = glm::mat4(1.0f, 0, 0, 0,
			0, 1.0f, 0, 0,
			0, 0, 0.5f, 0,
			0, 0, 0.25f, 1.0f);

		projection = scale * projection;
	}

	unsigned int transformID = engine.entities[entityID].components_[COMPONENT_TRANSFORM];
	CTransform *transform = &engine.transformSystem.components[transformID];
	glm::mat4 view = glm::lookAt(
		transform->GetPosition(),
		transform->GetPosition() + transform->GetForward(),
		glm::vec3(0, 1, 0)
	);

	matrix_ = projection * view;

	return matrix_;
}

glm::mat4 CSpotLight::calculateMatrixBasis() {
	glm::mat4 bias_matrix(
		0.5, 0.0, 0.0, 0.0,
		0.0, 0.5, 0.0, 0.0,
		0.0, 0.0, 0.5, 0.0,
		0.5, 0.5, 0.5, 1.0
	);

	return bias_matrix * matrix_;
}

CDirectionalLight::CDirectionalLight(unsigned int entityID) {
	res = 1024;
	this->entityID = entityID;

	cascades_count_ = 4;
	matrices_.resize(cascades_count_);

	UniformBufferCreateInfo lightuboci;
	lightuboci.isDynamic = false;
	lightuboci.size = sizeof(LightDirectionalUBO);
	lightuboci.binding = engine.directionalLightUBB;
	lightUBO = engine.graphics_wrapper_->CreateUniformBuffer(lightuboci);
}

CDirectionalLight::CDirectionalLight(unsigned int entID, glm::vec3 color, float strength, bool cast, float radius) {
	lightUBOBuffer.sourceRadius = radius;
	lightUBOBuffer.color = color;
	lightUBOBuffer.power = strength;

	cascades_count_ = 4;
	matrices_.resize(cascades_count_);

	res = 1024;

	UniformBufferCreateInfo lightuboci;
	lightuboci.isDynamic = false;
	lightuboci.size = sizeof(LightDirectionalUBO);
	lightuboci.binding = engine.directionalLightUBB;
	lightUBO = engine.graphics_wrapper_->CreateUniformBuffer(lightuboci);

	SetShadow(cast);
}

void CDirectionalLight::SetShadow(bool state) {
	castShadow = state;
	lightUBOBuffer.shadow = castShadow;

	if (state) {
		DepthTargetCreateInfo depth_image_ci(FORMAT_DEPTH_24, res, res, true, false);
		shadow_db_ = engine.graphics_wrapper_->CreateDepthTarget(depth_image_ci);

		FramebufferCreateInfo fbci;
		fbci.num_render_target_lists = 0;
		fbci.render_target_lists = nullptr;
		fbci.depth_target = shadow_db_;
		shadowFBO = engine.graphics_wrapper_->CreateFramebuffer(fbci);
	}
}

void CDirectionalLight::Bind() {
	if (castShadow) {
		for (unsigned int i = 0; i < cascades_count_; ++i) {
			lightUBOBuffer.shadow_mat[i] = calculateMatrixBasis(matrices_[0]);

			shadowFBO->BindRead();
			shadowFBO->BindTextures(4 + i);
		}
	}

	Entity *entity = &engine.entities[entityID];
	unsigned int transID = entity->components_[COMPONENT_TRANSFORM];
	CTransform *trans = &engine.transformSystem.components[transID];
	lightUBOBuffer.direction = trans->GetForward();
	lightUBO->UpdateUniformBuffer(&lightUBOBuffer);
	lightUBO->Bind();
}

void CDirectionalLight::calculateMatrix() {
	const unsigned int num_cascades_corners_ = 8;

	CCamera *cam = &engine.cameraSystem.components[0];
	float dist_near = cam->GetNear();
	float dist_far = cam->GetFar();
	glm::mat4 cam_inv = glm::inverse(cam->GetView());

	unsigned int transformID = engine.entities[entityID].components_[COMPONENT_TRANSFORM];
	CTransform *transform = &engine.transformSystem.components[transformID];
	glm::mat4 light_view = glm::lookAt(
		glm::vec3(0, 0, 0),
		transform->GetForward(),
		glm::vec3(0, 1, 0)
	);

	float ar = cam->GetAspectRatio();
	float tanHalfHFOV = tanf(cam->GetFOV());
	float tanHalfVFOV = tanf(cam->GetFOV() * ar);

	for (unsigned int i = 0; i < cascades_count_ + 1; ++i) {
		lightUBOBuffer.cascade_distance[i] = (dist_far * float(cascades_count_ - i) + dist_near * float(i)) / float(cascades_count_);
	}

	lightUBO->UpdateUniformBuffer(&lightUBOBuffer);
	lightUBO->Bind();

	for (unsigned int i = 0; i < cascades_count_; i++) {
		float xn = lightUBOBuffer.cascade_distance[i] * tanHalfHFOV;
		float xf = lightUBOBuffer.cascade_distance[i + 1] * tanHalfHFOV;
		float yn = lightUBOBuffer.cascade_distance[i] * tanHalfVFOV;
		float yf = lightUBOBuffer.cascade_distance[i + 1] * tanHalfVFOV;

		glm::vec4 frustumCorners[num_cascades_corners_] = {
			// near face
			glm::vec4(xn, yn, lightUBOBuffer.cascade_distance[i], 1.0),
			glm::vec4(-xn, yn, lightUBOBuffer.cascade_distance[i], 1.0),
			glm::vec4(xn, -yn, lightUBOBuffer.cascade_distance[i], 1.0),
			glm::vec4(-xn, -yn, lightUBOBuffer.cascade_distance[i], 1.0),

			// far face
			glm::vec4(xf, yf, lightUBOBuffer.cascade_distance[i + 1], 1.0),
			glm::vec4(-xf, yf, lightUBOBuffer.cascade_distance[i + 1], 1.0),
			glm::vec4(xf, -yf, lightUBOBuffer.cascade_distance[i + 1], 1.0),
			glm::vec4(-xf, -yf, lightUBOBuffer.cascade_distance[i + 1], 1.0)
		};

		glm::vec4 frustumCornersL[num_cascades_corners_];

		float minX = std::numeric_limits<float>::max();
		float maxX = std::numeric_limits<float>::min();
		float minY = std::numeric_limits<float>::max();
		float maxY = std::numeric_limits<float>::min();
		float minZ = std::numeric_limits<float>::max();
		float maxZ = std::numeric_limits<float>::min();

		for (unsigned int j = 0; j < cascades_count_; j++) {
			// Transform the frustum coordinate from view to world space
			glm::vec4 vW = cam_inv * frustumCorners[j];

			// Transform the frustum coordinate from world to light space
			frustumCornersL[j] = light_view * vW;

			minX = glm::min(minX, frustumCornersL[j].x);
			maxX = glm::max(maxX, frustumCornersL[j].x);
			minY = glm::min(minY, frustumCornersL[j].y);
			maxY = glm::max(maxY, frustumCornersL[j].y);
			minZ = glm::min(minZ, frustumCornersL[j].z);
			maxZ = glm::max(maxZ, frustumCornersL[j].z);
		}

		matrices_[i] = glm::ortho(minX, maxX, minY, maxY, minZ, maxZ);
	}
}

glm::mat4 CDirectionalLight::calculateMatrixBasis(glm::mat4 matrix) {
	glm::mat4 bias_matrix(
		0.5, 0.0, 0.0, 0.0,
		0.0, 0.5, 0.0, 0.0,
		0.0, 0.0, 0.5, 0.0,
		0.5, 0.5, 0.5, 1.0
	);
	
	return bias_matrix * matrix;
}

void SLight::AddPointLight(unsigned int entityID, glm::vec3 lightColor, float intensity, bool castShadow, float lightRadius) {
	engine.entities[entityID].components_[COMPONENT_LIGHT_POINT] = (unsigned int)pointLights.size();
	pointLights.push_back(CPointLight(entityID, lightColor, intensity, engine.settings.enableShadows && castShadow, lightRadius));
}

void SLight::AddSpotLight(unsigned int entityID, glm::vec3 lightColor, float intensity, bool castShadow, float lightRadius, float innerSpotAngle, float outerSpotAngle) {
	engine.entities[entityID].components_[COMPONENT_LIGHT_SPOT] = (unsigned int)spotLights.size();
	spotLights.push_back(CSpotLight(entityID, lightColor, intensity, engine.settings.enableShadows && castShadow, lightRadius, innerSpotAngle, outerSpotAngle));
}

void SLight::AddDirectionalLight(unsigned int entityID) {
	engine.entities[entityID].components_[COMPONENT_LIGHT_DIRECTIONAL] = (unsigned int)directionalLights.size();
	directionalLights.push_back(CDirectionalLight(entityID));
}

void SLight::AddPointLight(unsigned int entityID) {
	engine.entities[entityID].components_[COMPONENT_LIGHT_POINT] = (unsigned int)pointLights.size();
	pointLights.push_back(CPointLight(entityID));
}

void SLight::AddSpotLight(unsigned int entityID) {
	engine.entities[entityID].components_[COMPONENT_LIGHT_SPOT] = (unsigned int)spotLights.size();
	spotLights.push_back(CSpotLight(entityID));
}

void SLight::AddDirectionalLight(unsigned int entityID, glm::vec3 lightColor, float intensity, bool castShadow, float sunRadius) {
	engine.entities[entityID].components_[COMPONENT_LIGHT_DIRECTIONAL] = (unsigned int)directionalLights.size();
	directionalLights.push_back(CDirectionalLight(entityID, lightColor, intensity, engine.settings.enableShadows && castShadow, sunRadius));
}

void SLight::SetPointers(GraphicsWrapper *gw, SGeometry *gc) {
	graphics_wrapper_ = gw;
	geometry_system = gc;

	UniformBufferBindingCreateInfo lightubbci;
	lightubbci.binding = 1;
	lightubbci.shaderLocation = "Light";
	lightubbci.size = sizeof(LightPointUBO);
	lightubbci.stages = SHADER_STAGE_FRAGMENT_BIT;
	engine.pointLightUBB = graphics_wrapper_->CreateUniformBufferBinding(lightubbci);

	lightubbci.binding = 1;
	lightubbci.shaderLocation = "Light";
	lightubbci.size = sizeof(LightSpotUBO);
	lightubbci.stages = SHADER_STAGE_FRAGMENT_BIT;
	engine.spotLightUBB = graphics_wrapper_->CreateUniformBufferBinding(lightubbci);

	lightubbci.binding = 1;
	lightubbci.shaderLocation = "Light";
	lightubbci.size = sizeof(LightDirectionalUBO);
	lightubbci.stages = SHADER_STAGE_FRAGMENT_BIT;
	engine.directionalLightUBB = graphics_wrapper_->CreateUniformBufferBinding(lightubbci);

	ShaderStageCreateInfo vi;
	ShaderStageCreateInfo fi;
	if (engine.settings.graphicsLanguage == GRAPHICS_OPENGL) {
		vi.fileName = "../assets/shaders/lights_deferred/pointVert.glsl";
		fi.fileName = "../assets/shaders/lights_deferred/pointFrag.glsl";
	}
	else if (engine.settings.graphicsLanguage == GRAPHICS_DIRECTX) {
		vi.fileName = "../assets/shaders/lights_deferred/pointVert.fxc";
		fi.fileName = "../assets/shaders/lights_deferred/pointFrag.fxc";
	}
	else {
		vi.fileName = "../assets/shaders/lights_deferred/pointVert.spv";
		fi.fileName = "../assets/shaders/lights_deferred/pointFrag.spv";
	}
	std::vector<char> vfile;
	if (!readFile(vi.fileName, vfile))
		return;
	vi.content = vfile.data();
	vi.size = (uint32_t)vfile.size();
	vi.type = SHADER_VERTEX;

	std::vector<char> ffile;
	if (!readFile(fi.fileName, ffile))
		return;
	fi.content = ffile.data();
	fi.size = (uint32_t)ffile.size();
	fi.type = SHADER_FRAGMENT;

	std::vector<ShaderStageCreateInfo> stages = { vi, fi };

	GraphicsPipelineCreateInfo pointGPCI;
	pointGPCI.cullMode = CULL_BACK;
	pointGPCI.bindings = &engine.planeVBD;
	pointGPCI.bindingsCount = 1;
	pointGPCI.attributes = &engine.planeVAD;
	pointGPCI.attributesCount = 1;
	pointGPCI.width = (float)engine.settings.resolutionX;
	pointGPCI.height = (float)engine.settings.resolutionY;
	pointGPCI.scissorW = engine.settings.resolutionX;
	pointGPCI.scissorH = engine.settings.resolutionY;
	pointGPCI.primitiveType = PRIM_TRIANGLE_STRIPS;
	pointGPCI.shaderStageCreateInfos = stages.data();
	pointGPCI.shaderStageCreateInfoCount = (uint32_t)stages.size();
	pointGPCI.textureBindings = &engine.tbl;
	pointGPCI.textureBindingCount = 1;
	std::vector<UniformBufferBinding *> ubbs = { engine.deffubb, engine.pointLightUBB };
	pointGPCI.uniformBufferBindings = ubbs.data();
	pointGPCI.uniformBufferBindingCount = (uint32_t)ubbs.size();
	m_pointLightPipeline = graphics_wrapper_->CreateGraphicsPipeline(pointGPCI);

	if (engine.settings.graphicsLanguage == GRAPHICS_OPENGL) {
		vi.fileName = "../assets/shaders/lights_deferred/pointVert.glsl";
		fi.fileName = "../assets/shaders/lights_deferred/spotFrag.glsl";
	}
	else if (engine.settings.graphicsLanguage == GRAPHICS_DIRECTX) {
		vi.fileName = "../assets/shaders/lights_deferred/pointVert.fxc";
		fi.fileName = "../assets/shaders/lights_deferred/spotFrag.fxc";
	}
	else {
		vi.fileName = "../assets/shaders/lights_deferred/pointVert.spv";
		fi.fileName = "../assets/shaders/lights_deferred/spotFrag.spv";
	}
	vfile.clear();
	if (!readFile(vi.fileName, vfile))
		return;
	vi.content = vfile.data();
	vi.size = (uint32_t)vfile.size();
	vi.type = SHADER_VERTEX;

	ffile.clear();
	if (!readFile(fi.fileName, ffile))
		return;
	fi.content = ffile.data();
	fi.size = (uint32_t)ffile.size();
	fi.type = SHADER_FRAGMENT;

	std::vector<ShaderStageCreateInfo> stagesSpot = { vi, fi };

	GraphicsPipelineCreateInfo spotGPCI;
	spotGPCI.cullMode = CULL_BACK;
	spotGPCI.bindings = &engine.planeVBD;
	spotGPCI.bindingsCount = 1;
	spotGPCI.attributes = &engine.planeVAD;
	spotGPCI.attributesCount = 1;
	spotGPCI.width = (float)engine.settings.resolutionX;
	spotGPCI.height = (float)engine.settings.resolutionY;
	spotGPCI.scissorW = engine.settings.resolutionX;
	spotGPCI.scissorH = engine.settings.resolutionY;
	spotGPCI.primitiveType = PRIM_TRIANGLE_STRIPS;
	spotGPCI.shaderStageCreateInfos = stagesSpot.data();
	spotGPCI.shaderStageCreateInfoCount = (uint32_t)stagesSpot.size();
	spotGPCI.textureBindings = &engine.tbl;
	spotGPCI.textureBindingCount = 1;
	ubbs.clear();
	ubbs = { engine.deffubb, engine.spotLightUBB };
	spotGPCI.uniformBufferBindings = ubbs.data();
	spotGPCI.uniformBufferBindingCount = (uint32_t)ubbs.size();
	m_spotLightPipeline = graphics_wrapper_->CreateGraphicsPipeline(spotGPCI);

	// DIRECTIONAL LIGHTS

	if (engine.settings.graphicsLanguage == GRAPHICS_OPENGL) {
		vi.fileName = "../assets/shaders/lights_deferred/pointVert.glsl";
		fi.fileName = "../assets/shaders/lights_deferred/directionalFrag.glsl";
	}
	else if (engine.settings.graphicsLanguage == GRAPHICS_DIRECTX) {
		vi.fileName = "../assets/shaders/lights_deferred/pointVert.fxc";
		fi.fileName = "../assets/shaders/lights_deferred/directionalFrag.fxc";
	}
	else {
		vi.fileName = "../assets/shaders/lights_deferred/pointVert.spv";
		fi.fileName = "../assets/shaders/lights_deferred/directionalFrag.spv";
	}
	vfile.clear();
	if (!readFile(vi.fileName, vfile))
		return;
	vi.content = vfile.data();
	vi.size = (uint32_t)vfile.size();
	vi.type = SHADER_VERTEX;

	ffile.clear();
	if (!readFile(fi.fileName, ffile))
		return;
	fi.content = ffile.data();
	fi.size = (uint32_t)ffile.size();
	fi.type = SHADER_FRAGMENT;

	std::vector<ShaderStageCreateInfo> stages_directional = { vi, fi };

	GraphicsPipelineCreateInfo directionalGPCI;
	directionalGPCI.cullMode = CULL_BACK;
	directionalGPCI.bindings = &engine.planeVBD;
	directionalGPCI.bindingsCount = 1;
	directionalGPCI.attributes = &engine.planeVAD;
	directionalGPCI.attributesCount = 1;
	directionalGPCI.width = (float)engine.settings.resolutionX;
	directionalGPCI.height = (float)engine.settings.resolutionY;
	directionalGPCI.scissorW = engine.settings.resolutionX;
	directionalGPCI.scissorH = engine.settings.resolutionY;
	directionalGPCI.primitiveType = PRIM_TRIANGLE_STRIPS;
	directionalGPCI.shaderStageCreateInfos = stages_directional.data();
	directionalGPCI.shaderStageCreateInfoCount = (uint32_t)stages_directional.size();
	directionalGPCI.textureBindings = &engine.tbl;
	directionalGPCI.textureBindingCount = 1;
	ubbs.clear();
	ubbs = { engine.deffubb, engine.directionalLightUBB };
	directionalGPCI.uniformBufferBindings = ubbs.data();
	directionalGPCI.uniformBufferBindingCount = (uint32_t)ubbs.size();
	m_directionalLightPipeline = graphics_wrapper_->CreateGraphicsPipeline(directionalGPCI);

	// CASCADED DIRECTIONAL LIGHTS

	if (engine.settings.graphicsLanguage == GRAPHICS_OPENGL) {
		vi.fileName = "../assets/shaders/lights_deferred/cascade_shadow_vert.glsl";
		fi.fileName = "../assets/shaders/lights_deferred/cascade_shadow_frag.glsl";
	}
	else if (engine.settings.graphicsLanguage == GRAPHICS_DIRECTX) {
		vi.fileName = "../assets/shaders/lights_deferred/cascade_shadow_vert.fxc";
		fi.fileName = "../assets/shaders/lights_deferred/cascade_shadow_frag.fxc";
	}
	else {
		vi.fileName = "../assets/shaders/lights_deferred/cascade_shadow_vert.spv";
		fi.fileName = "../assets/shaders/lights_deferred/cascade_shadow_frag.spv";
	}

	vfile.clear();
	if (!readFile(vi.fileName, vfile))
		return;
	vi.content = vfile.data();
	vi.size = (uint32_t)vfile.size();
	vi.type = SHADER_VERTEX;

	ffile.clear();
	if (!readFile(fi.fileName, ffile))
		return;
	fi.content = ffile.data();
	fi.size = (uint32_t)ffile.size();
	fi.type = SHADER_FRAGMENT;

	std::vector<ShaderStageCreateInfo> stages_cascade = { vi, fi };

	GraphicsPipelineCreateInfo cascadeGPCI;
	cascadeGPCI.cullMode = CULL_BACK;
	cascadeGPCI.bindings = &engine.planeVBD;
	cascadeGPCI.bindingsCount = 1;
	cascadeGPCI.attributes = &engine.planeVAD;
	cascadeGPCI.attributesCount = 1;
	cascadeGPCI.width = (float)engine.settings.resolutionX;
	cascadeGPCI.height = (float)engine.settings.resolutionY;
	cascadeGPCI.scissorW = engine.settings.resolutionX;
	cascadeGPCI.scissorH = engine.settings.resolutionY;
	cascadeGPCI.primitiveType = PRIM_TRIANGLE_STRIPS;
	cascadeGPCI.shaderStageCreateInfos = stages_cascade.data();
	cascadeGPCI.shaderStageCreateInfoCount = (uint32_t)stages_cascade.size();
	cascadeGPCI.textureBindings = &engine.tbl;
	cascadeGPCI.textureBindingCount = 1;
	ubbs.clear();
	ubbs = { engine.deffubb, engine.directionalLightUBB };
	cascadeGPCI.uniformBufferBindings = ubbs.data();
	cascadeGPCI.uniformBufferBindingCount = (uint32_t)ubbs.size();
	m_cascadeLightPipeline = graphics_wrapper_->CreateGraphicsPipeline(cascadeGPCI);
}

void SLight::DrawShadows() {
	graphics_wrapper_->SetImmediateBlending(BLEND_NONE);
	MainUBO ubo;

	for (auto &light : pointLights) {
		double camFar = light.lightUBOBuffer.attenuationRadius;
		glm::mat4 proj = glm::perspective(1.5708f, 1.0f, 0.1f, 100.0f);
		glm::mat4 view;

		if (light.castShadow) {
			//  gCubeDirections
			light.shadowFBO->BindWrite(true);

			unsigned int transformID = engine.entities[light.entityID].components_[COMPONENT_TRANSFORM];
			CTransform *transform = &engine.transformSystem.components[transformID];

			for (int i = 0; i < 6; ++i) {
				light.shadow_db_->BindFace(i);
				light.shadowFBO->Clear(CLEAR_DEPTH);
				view = glm::lookAt(transform->position, transform->position + gCubeDirections[i].Target, gCubeDirections[i].Up);
				
				ubo.pv = proj * view;
				ubo.eye_pos = transform->position;

				engine.ubo->UpdateUniformBuffer(&ubo);
				engine.ubo->Bind();
				engine.ubo2->Bind();
				
				engine.materialManager.DrawShadowsImmediate();
			}
		}
	}

	glm::mat4 pv;

	for (auto &light : spotLights) {
		if (light.castShadow) {
			light.shadowFBO->BindWrite(true);
			light.shadowFBO->Clear(CLEAR_DEPTH);

			unsigned int transformID = engine.entities[light.entityID].components_[COMPONENT_TRANSFORM];
			CTransform *transform = &engine.transformSystem.components[transformID];

			ubo.pv = light.calculateMatrix();
			ubo.eye_pos = transform->GetPosition();

			engine.ubo->UpdateUniformBuffer(&ubo);
			engine.ubo->Bind();
			engine.ubo2->Bind();

			engine.materialManager.DrawShadowsImmediate();
		}
	}

	for (auto &light : directionalLights) {
		if (light.castShadow) {
			light.shadowFBO->BindWrite(true);
			light.shadowFBO->Clear(CLEAR_DEPTH);

			unsigned int transformID = engine.entities[light.entityID].components_[COMPONENT_TRANSFORM];
			CTransform *transform = &engine.transformSystem.components[transformID];

			light.calculateMatrix();
			ubo.eye_pos = transform->GetPosition();

			for (unsigned int i = 0; i < light.cascades_count_; ++i) {
				ubo.pv = light.matrices_[i];

				engine.ubo->UpdateUniformBuffer(&ubo);
				engine.ubo->Bind();
				engine.ubo2->Bind();

				engine.materialManager.DrawShadowsImmediate();
			}
		}
	}
}
