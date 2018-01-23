#include "SLight.hpp"
#include "../Core/Engine.hpp"

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

#if 0
	if (cast) {
		fbo->Initialize(0);
		fbo->AddDepthBuffer(256, 256);
		fbo->Generate();
	}
#endif
}

void CSpotLight::SetShadow(bool state) {
	castShadow = state;
#if 0
	if (state) {
		fbo = pfnCreateFramebuffer();
		fbo->Initialize(0);
		fbo->AddDepthBuffer(128, 128);
		fbo->Generate();
	}
#endif
}

void CSpotLight::Bind() {
	Entity *entity = &engine.entities[entityID];
	unsigned int transID = entity->components_[COMPONENT_TRANSFORM];
	CTransform *trans = &engine.transformSystem.components[transID];
	lightUBOBuffer.position = trans->GetPosition();
	lightUBOBuffer.direction = trans->GetForward();
	lightUBO->UpdateUniformBuffer(&lightUBOBuffer);
	lightUBO->Bind();
}

CDirectionalLight::CDirectionalLight(unsigned int entityID) {
	this->entityID = entityID;
}

CDirectionalLight::CDirectionalLight(unsigned int entID, glm::vec3 color, float strength, bool cast, float radius) {
	lightUBOBuffer.sourceRadius = radius;
	lightUBOBuffer.color = color;
	lightUBOBuffer.power = strength;

	castShadow = cast;

#if 0
	if (cast) {
		fbo->Initialize(0);
		fbo->AddDepthBuffer(128, 128);
		fbo->Generate();
	}
#endif
}

void CDirectionalLight::SetShadow(bool state) {
	castShadow = state;
#if 0
	if (state) {
		fbo = pfnCreateFramebuffer();
		fbo->Initialize(0);
		fbo->AddDepthBuffer(2048, 2048);
		fbo->Generate();
	}
#endif
}

void CDirectionalLight::Bind() {
	Entity *entity = &engine.entities[entityID];
	unsigned int transID = entity->components_[COMPONENT_TRANSFORM];
	CTransform *trans = &engine.transformSystem.components[transID];
	lightUBOBuffer.position = trans->position;
	lightUBO->UpdateUniformBuffer(&lightUBOBuffer);
	lightUBO->Bind();
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

	engine.bindings.reserve(4);
	engine.bindings.emplace_back("gbuffer0", 0); // R G B MatID
	engine.bindings.emplace_back("gbuffer1", 1); // nX nY nZ MatData
	engine.bindings.emplace_back("gbuffer2", 2); // sR sG sB Roughness
	engine.bindings.emplace_back("gbuffer3", 3); // Depth

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

	/*if (engine.settings.graphicsLanguage == GRAPHICS_OPENGL) {
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
	m_spotLightPipeline = graphics_wrapper_->CreateGraphicsPipeline(spotGPCI);*/
}

void SLight::DrawShadows() {
}
