#include "SLight.hpp"
#include "../Core/Engine.hpp"
#include <glm/gtx/transform.hpp>

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

	UniformBufferCreateInfo lightuboci;
	lightuboci.isDynamic = false;
	lightuboci.size = sizeof(LightPointUBO);
	lightuboci.binding = engine.pointLightUBB;
	lightUBO = engine.graphics_wrapper_->CreateUniformBuffer(lightuboci);

	castShadow = cast;

#if 0
	if (cast) {
		fbo->Initialize(0);
		fbo->AddDepthCubeBuffer(256, 256);
		fbo->Generate();
	}
#endif
}

void CPointLight::SetShadow(bool state) {
	castShadow = state;
#if 0
	if (state) {
		fbo = pfnCreateFramebuffer();
		fbo->Initialize(0);
		fbo->AddDepthCubeBuffer(128, 128);
		fbo->Generate();
	}
#endif
}

void CPointLight::Bind() {
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

	castShadow = cast;

	UniformBufferCreateInfo lightuboci;
	lightuboci.isDynamic = false;
	lightuboci.size = sizeof(LightSpotUBO);
	lightuboci.binding = engine.spotLightUBB;
	lightUBO = engine.graphics_wrapper_->CreateUniformBuffer(lightuboci);
	if (cast) {
		DepthTargetCreateInfo depth_image_ci(FORMAT_DEPTH_24, 512, 512);
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

	if (state) {
		DepthTargetCreateInfo depth_image_ci(FORMAT_DEPTH_24, 512, 512);
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
	this->entityID = entityID;

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

	castShadow = cast;

	UniformBufferCreateInfo lightuboci;
	lightuboci.isDynamic = false;
	lightuboci.size = sizeof(LightDirectionalUBO);
	lightuboci.binding = engine.directionalLightUBB;
	lightUBO = engine.graphics_wrapper_->CreateUniformBuffer(lightuboci);

	if (cast) {
		DepthTargetCreateInfo depth_image_ci(FORMAT_DEPTH_24, 512, 512);
		shadow_db_ = engine.graphics_wrapper_->CreateDepthTarget(depth_image_ci);

		FramebufferCreateInfo fbci;
		fbci.num_render_target_lists = 0;
		fbci.render_target_lists = nullptr;
		fbci.depth_target = shadow_db_;
		shadowFBO = engine.graphics_wrapper_->CreateFramebuffer(fbci);
	}
}

void CDirectionalLight::SetShadow(bool state) {
	castShadow = state;

	if (state) {
		DepthTargetCreateInfo depth_image_ci(FORMAT_DEPTH_24, 512, 512);
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
		lightUBOBuffer.shadow_mat = calculateMatrixBasis();
		shadowFBO->BindRead();
		shadowFBO->BindTextures(4);
	}

	Entity *entity = &engine.entities[entityID];
	unsigned int transID = entity->components_[COMPONENT_TRANSFORM];
	CTransform *trans = &engine.transformSystem.components[transID];
	lightUBOBuffer.direction = trans->GetForward();
	lightUBO->UpdateUniformBuffer(&lightUBOBuffer);
	lightUBO->Bind();
}

glm::mat4 CDirectionalLight::calculateMatrix() {
	glm::mat4 projection = glm::ortho<float>(-10, 10, -10, 10, -10, 30);
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
		transform->GetForward()*20.0f,
		glm::vec3(0, 0, 0),
		glm::vec3(0, 1, 0)
	);

	matrix_ = projection * view;

	return matrix_;
}

glm::mat4 CDirectionalLight::calculateMatrixBasis() {
	glm::mat4 bias_matrix(
		0.5, 0.0, 0.0, 0.0,
		0.0, 0.5, 0.0, 0.0,
		0.0, 0.0, 0.5, 0.0,
		0.5, 0.5, 0.5, 1.0
	);
	
	return bias_matrix * matrix_;
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

	UniformBufferBindingCreateInfo deffubbci;
	deffubbci.binding = 0;
	deffubbci.shaderLocation = "UniformBufferObject";
	deffubbci.size = sizeof(DefferedUBO);
	deffubbci.stages = SHADER_STAGE_FRAGMENT_BIT;
	engine.deffubb = graphics_wrapper_->CreateUniformBufferBinding(deffubbci);

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

	engine.bindings.reserve(5);
	engine.bindings.emplace_back("gbuffer0", 0); // R G B MatID
	engine.bindings.emplace_back("gbuffer1", 1); // nX nY nZ MatData
	engine.bindings.emplace_back("gbuffer2", 2); // sR sG sB Roughness
	engine.bindings.emplace_back("gbuffer3", 3); // Depth
	engine.bindings.emplace_back("shadow_map", 4); // Shadow Map

	TextureBindingLayoutCreateInfo tblci;
	tblci.bindingLocation = 1;
	tblci.bindings = engine.bindings.data();
	tblci.bindingCount = (uint32_t)engine.bindings.size();
	tblci.stages = SHADER_STAGE_FRAGMENT_BIT;
	engine.tbl = graphics_wrapper_->CreateTextureBindingLayout(tblci);

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

	std::vector<ShaderStageCreateInfo> directionalSpot = { vi, fi };

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
	directionalGPCI.shaderStageCreateInfos = directionalSpot.data();
	directionalGPCI.shaderStageCreateInfoCount = (uint32_t)directionalSpot.size();
	directionalGPCI.textureBindings = &engine.tbl;
	directionalGPCI.textureBindingCount = 1;
	ubbs.clear();
	ubbs = { engine.deffubb, engine.directionalLightUBB };
	directionalGPCI.uniformBufferBindings = ubbs.data();
	directionalGPCI.uniformBufferBindingCount = (uint32_t)ubbs.size();
	m_directionalLightPipeline = graphics_wrapper_->CreateGraphicsPipeline(directionalGPCI);
}

void SLight::DrawShadows() {
	graphics_wrapper_->SetImmediateBlending(BLEND_NONE);
	glm::mat4 pv;
	for (auto &light : spotLights) {
		if (light.castShadow) {
			light.shadowFBO->BindWrite();
			light.shadowFBO->Clear();
			pv = light.calculateMatrix();
			engine.ubo->UpdateUniformBuffer(&pv);
			engine.ubo->Bind();
			engine.ubo2->Bind();

			engine.materialManager.DrawShadowsImmediate();
		}
	}

	for (auto &light : directionalLights) {
		if (light.castShadow) {
			light.shadowFBO->BindWrite();
			light.shadowFBO->Clear();
			pv = light.calculateMatrix();
			engine.ubo->UpdateUniformBuffer(&pv);
			engine.ubo->Bind();
			engine.ubo2->Bind();

			engine.materialManager.DrawShadowsImmediate();
		}
	}
}
