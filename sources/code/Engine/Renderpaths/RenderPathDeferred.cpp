#include "RenderPathDeferred.h"
#include "Core/Engine.h"
#include <stb/stb_image.h>

Texture *LoadCubemap(std::string path, GraphicsWrapper *m_graphics_wrapper_) {

	std::string facePaths[6];
	facePaths[0] = path + "FT.png";
	facePaths[1] = path + "BK.png";
	facePaths[2] = path + "UP.png";
	facePaths[3] = path + "DN.png";
	facePaths[4] = path + "RT.png";
	facePaths[5] = path + "LF.png";


	CubemapCreateInfo createInfo;
	
	int texWidth, texHeight, texChannels;
	for (int i = 0; i < 6; i++) {
		createInfo.data[i] = stbi_load(facePaths[i].c_str(), &texWidth, &texHeight, &texChannels, STBI_default);
		if (!createInfo.data[i]) {
			printf("Texture failed to load!: %s \n", facePaths[i].c_str());
			for (int j = 0; j < i; j++) {
				stbi_image_free(createInfo.data[j]);
			}
			return NULL;
		}
	}

	printf("Cubemap loaded: %s \n", path.c_str());

	ColorFormat format;
	switch (texChannels) {
	case 1:
		format = FORMAT_COLOR_R8;
		break;
	case 2:
		format = FORMAT_COLOR_R8G8;
		break;
	case 3:
		format = FORMAT_COLOR_R8G8B8;
		break;
	default:
	case 4:
		format = FORMAT_COLOR_R8G8B8A8;
		break;
	}

	createInfo.format = format;
	createInfo.mipmaps = 0;
	createInfo.width = texWidth;
	createInfo.height = texHeight;

	Texture *t = m_graphics_wrapper_->CreateCubemap(createInfo);

	for (int i = 0; i < 6; i++) {
		stbi_image_free(createInfo.data[i]);
	}

	return t;
}

RenderPathDeferred::RenderPathDeferred(GraphicsWrapper * graphics_wrapper_) {
	m_graphics_wrapper_ = graphics_wrapper_;

	float planeVerts[4 * 6] = {
		-1.0, -1.0,
		1.0, -1.0,
		-1.0,  1.0,
		1.0,  1.0,
		-1.0,  1.0,
		1.0, -1.0,
	};

	UniformBufferCreateInfo deffubci;
	deffubci.isDynamic = false;
	deffubci.size = sizeof(DefferedUBO);
	deffubci.binding = engine.deffubb;
	deffUBO = graphics_wrapper_->CreateUniformBuffer(deffubci);

	VertexBufferCreateInfo planeVboCI;
	planeVboCI.binding = &engine.planeVBD;
	planeVboCI.bindingCount = 1;
	planeVboCI.attribute = &engine.planeVAD;
	planeVboCI.attributeCount = 1;
	planeVboCI.content = planeVerts;
	planeVboCI.count = 6;
	planeVboCI.size = sizeof(float) * 6 * 2;

	VertexArrayObjectCreateInfo vaci;
	vaci.vertexBuffer = planeVBO;
	vaci.indexBuffer = nullptr;
	planeVAO = graphics_wrapper_->CreateVertexArrayObject(vaci);
	planeVBO = graphics_wrapper_->CreateVertexBuffer(planeVboCI);

	vaci.vertexBuffer = planeVBO;
	vaci.indexBuffer = nullptr;
	planeVAO->BindResources(vaci);
	planeVAO->Unbind();

	m_cubemap = LoadCubemap("../assets/cubemaps/level4", m_graphics_wrapper_);

	std::vector<TextureSubBinding> cubemapBindings;
	cubemapBindings.reserve(1);
	cubemapBindings.emplace_back("environmentMap", 4);

	TextureBindingLayoutCreateInfo tblci;
	tblci.bindingLocation = 2;
	tblci.bindings = cubemapBindings.data();
	tblci.bindingCount = (uint32_t)cubemapBindings.size();
	tblci.stages = SHADER_STAGE_FRAGMENT_BIT;
	TextureBindingLayout *cubemapLayout = graphics_wrapper_->CreateTextureBindingLayout(tblci);

	SingleTextureBind stb;
	stb.texture = m_cubemap;
	stb.address = 4;

	TextureBindingCreateInfo ci;
	ci.textures = &stb;
	ci.textureCount = 1;
	m_cubemapBinding = m_graphics_wrapper_->CreateTextureBinding(ci);

	ShaderStageCreateInfo vi;
	ShaderStageCreateInfo fi;
	if (engine.settings.graphicsLanguage == GRAPHICS_OPENGL) {
		vi.fileName = "../assets/shaders/spotVert.glsl";
		fi.fileName = "../assets/shaders/ibl.glsl";
	}
	else if (engine.settings.graphicsLanguage == GRAPHICS_DIRECTX) {
		vi.fileName = "../assets/shaders/pointVert.fxc";
		fi.fileName = "../assets/shaders/ibl.fxc";
	}
	else {
		vi.fileName = "../assets/shaders/spotVert.spv";
		fi.fileName = "../assets/shaders/ibl.spv";
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

	GraphicsPipelineCreateInfo iblGPCI;
	iblGPCI.cullMode = CULL_BACK;
	iblGPCI.bindings = &engine.planeVBD;
	iblGPCI.bindingsCount = 1;
	iblGPCI.attributes = &engine.planeVAD;
	iblGPCI.attributesCount = 1;
	iblGPCI.width = (float)engine.settings.resolutionX;
	iblGPCI.height = (float)engine.settings.resolutionY;
	iblGPCI.scissorW = engine.settings.resolutionX;
	iblGPCI.scissorH = engine.settings.resolutionY;
	iblGPCI.primitiveType = PRIM_TRIANGLES;
	iblGPCI.shaderStageCreateInfos = stages.data();
	iblGPCI.shaderStageCreateInfoCount = (uint32_t)stages.size();
	std::vector<TextureBindingLayout *> tbls = { engine.tbl, cubemapLayout };

	iblGPCI.textureBindings = tbls.data();
	iblGPCI.textureBindingCount = (uint32_t)tbls.size();
	iblGPCI.uniformBufferBindings = &engine.deffubb;
	iblGPCI.uniformBufferBindingCount = 1;
	m_iblPipeline = graphics_wrapper_->CreateGraphicsPipeline(iblGPCI);
}

void RenderPathDeferred::Draw(Framebuffer *gbuffer) {
	gbuffer->BindTextures();

	CCamera *cam = &engine.cameraSystem.components[0];
	Entity *ent = &engine.entities[cam->entityID];
	glm::vec3 eyePos = engine.transformSystem.components[ent->components[COMPONENT_TRANSFORM]].position;
	deffUBOBuffer.eyePos.x = eyePos.x;
	deffUBOBuffer.eyePos.y = eyePos.y;
	deffUBOBuffer.eyePos.z = eyePos.z;
	deffUBOBuffer.invProj = glm::inverse(cam->GetProjection());
	deffUBOBuffer.view = glm::inverse(cam->GetView());

	deffUBOBuffer.resolution.x = engine.settings.resolutionX;
	deffUBOBuffer.resolution.y = engine.settings.resolutionY;
	deffUBO->UpdateUniformBuffer(&deffUBOBuffer);

	m_graphics_wrapper_->Clear();
	m_graphics_wrapper_->SetImmediateBlending(true);
	deffUBO->Bind();
	engine.graphics_wrapper_->BindVertexArrayObject(planeVAO);
	
	engine.lightSystem.m_pointLightPipeline->Bind();
	for (auto light : engine.lightSystem.pointLights) {
		light.Bind();

		m_graphics_wrapper_->DrawImmediateVertices(0, 6);
	}

	engine.lightSystem.m_spotLightPipeline->Bind();
	for (auto light : engine.lightSystem.spotLights) {
		light.Bind();

		m_graphics_wrapper_->DrawImmediateVertices(0, 6);
	}

	m_iblPipeline->Bind();
	//m_graphics_wrapper_->BindTextureBinding(m_cubemapBinding);
	m_graphics_wrapper_->DrawImmediateVertices(0, 6);

	m_graphics_wrapper_->SetImmediateBlending(false);
}
