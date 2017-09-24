#include "SLight.h"
#include "../Core/Engine.h"

void SLight::AddPointLight(unsigned int entityID, glm::vec3 lightColor, float intensity, bool castShadow, float lightRadius) {
	engine.entities[entityID].components[COMPONENT_LIGHT_POINT] = (unsigned int)pointLights.size();
	pointLights.push_back(CPointLight(entityID, lightColor, intensity, engine.settings.enableShadows && castShadow, lightRadius));
}

void SLight::AddSpotLight(unsigned int entityID, glm::vec3 lightColor, float intensity, bool castShadow, float lightRadius, float innerSpotAngle, float outerSpotAngle) {
	engine.entities[entityID].components[COMPONENT_LIGHT_SPOT] = (unsigned int)spotLights.size();
	spotLights.push_back(CSpotLight(entityID, lightColor, intensity, engine.settings.enableShadows && castShadow, lightRadius, innerSpotAngle, outerSpotAngle));
}

void SLight::AddDirectionalLight(unsigned int entityID) {
	engine.entities[entityID].components[COMPONENT_LIGHT_DIRECTIONAL] = (unsigned int)directionalLights.size();
	directionalLights.push_back(CDirectionalLight(entityID));
}

void SLight::AddPointLight(unsigned int entityID) {
	engine.entities[entityID].components[COMPONENT_LIGHT_POINT] = (unsigned int)pointLights.size();
	pointLights.push_back(CPointLight(entityID));
}

void SLight::AddSpotLight(unsigned int entityID) {
	engine.entities[entityID].components[COMPONENT_LIGHT_SPOT] = (unsigned int)spotLights.size();
	spotLights.push_back(CSpotLight(entityID));
}

void SLight::AddDirectionalLight(unsigned int entityID, glm::vec3 lightColor, float intensity, bool castShadow, float sunRadius) {
	engine.entities[entityID].components[COMPONENT_LIGHT_DIRECTIONAL] = (unsigned int)directionalLights.size();
	directionalLights.push_back(CDirectionalLight(entityID, lightColor, intensity, engine.settings.enableShadows && castShadow, sunRadius));
}

void SLight::SetPointers(GraphicsWrapper *gw, SModel *gc) {
	graphicsWrapper = gw;
	geometryCache = gc;

	engine.planeVBD.binding = 0;
	engine.planeVBD.elementRate = false;
	engine.planeVBD.stride = sizeof(float) * 2;

	engine.planeVAD.binding = 0;
	engine.planeVAD.location = 0;
	engine.planeVAD.format = VERTEX_R32_G32;
	engine.planeVAD.size = 2;
	engine.planeVAD.name = "vertexPosition";
	engine.planeVAD.offset = 0;
	engine.planeVAD.usage = ATTRIB_POSITION;

	UniformBufferBindingCreateInfo deffubbci;
	deffubbci.binding = 0;
	deffubbci.shaderLocation = "UniformBufferObject";
	deffubbci.size = sizeof(DefferedUBO);
	deffubbci.stages = SHADER_STAGE_FRAGMENT_BIT;
	engine.deffubb = graphicsWrapper->CreateUniformBufferBinding(deffubbci);

	UniformBufferBindingCreateInfo lightubbci;
	lightubbci.binding = 1;
	lightubbci.shaderLocation = "Light";
	lightubbci.size = sizeof(LightPointUBO);
	lightubbci.stages = SHADER_STAGE_FRAGMENT_BIT;
	engine.pointLightUBB = graphicsWrapper->CreateUniformBufferBinding(lightubbci);

	lightubbci.binding = 1;
	lightubbci.shaderLocation = "Light";
	lightubbci.size = sizeof(LightSpotUBO);
	lightubbci.stages = SHADER_STAGE_FRAGMENT_BIT;
	engine.spotLightUBB = graphicsWrapper->CreateUniformBufferBinding(lightubbci);

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
	engine.tbl = graphicsWrapper->CreateTextureBindingLayout(tblci);

	ShaderStageCreateInfo vi;
	ShaderStageCreateInfo fi;
	if (engine.settings.graphicsLanguage == GRAPHICS_OPENGL) {
		vi.fileName = "../assets/shaders/pointVert.glsl";
		fi.fileName = "../assets/shaders/pointFrag.glsl";
	}
	else if (engine.settings.graphicsLanguage == GRAPHICS_DIRECTX) {
		vi.fileName = "../assets/shaders/pointVert.fxc";
		fi.fileName = "../assets/shaders/pointFrag.fxc";
	}
	else {
		vi.fileName = "../assets/shaders/pointVert.spv";
		fi.fileName = "../assets/shaders/pointFrag.spv";
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
	m_pointLightPipeline = graphicsWrapper->CreateGraphicsPipeline(pointGPCI);

	if (engine.settings.graphicsLanguage == GRAPHICS_OPENGL) {
		vi.fileName = "../assets/shaders/pointVert.glsl";
		fi.fileName = "../assets/shaders/spotFrag.glsl";
	}
	else if (engine.settings.graphicsLanguage == GRAPHICS_DIRECTX) {
		vi.fileName = "../assets/shaders/pointVert.fxc";
		fi.fileName = "../assets/shaders/spotFrag.fxc";
	}
	else {
		vi.fileName = "../assets/shaders/pointVert.spv";
		fi.fileName = "../assets/shaders/spotFrag.spv";
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
	m_spotLightPipeline = graphicsWrapper->CreateGraphicsPipeline(spotGPCI);
}

void SLight::DrawShadows() {
}
