#include "RenderPathDeferred.hpp"
#include "Core/Engine.hpp"
#include <stb/stb_image.h>

Texture *LoadCubemap(std::string path, GraphicsWrapper *m_graphics_wrapper_) {

	std::string facePaths[6];
	facePaths[0] = path + "ft.tga";
	facePaths[1] = path + "bk.tga";
	facePaths[2] = path + "up.tga";
	facePaths[3] = path + "dn.tga";
	facePaths[4] = path + "rt.tga";
	facePaths[5] = path + "lf.tga";


	CubemapCreateInfo createInfo;
	
	int texWidth, texHeight, texChannels;
	for (int i = 0; i < 6; i++) {
		createInfo.data[i] = stbi_load(facePaths[i].c_str(), &texWidth, &texHeight, &texChannels, 4);
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
	switch (4) {
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

RenderPathDeferred::RenderPathDeferred(GraphicsWrapper * graphics_wrapper_, VertexArrayObject *plane_vao) {
	m_graphics_wrapper_ = graphics_wrapper_;

	plane_vao_ = plane_vao;

	m_cubemap = LoadCubemap("../assets/cubemaps/glacier_", m_graphics_wrapper_);

	std::vector<TextureSubBinding> cubemapBindings;
	cubemapBindings.reserve(1);
	cubemapBindings.emplace_back("environmentMap", 4);

	TextureBindingLayoutCreateInfo tblci;
	tblci.bindingLocation = 1;
	tblci.bindings = cubemapBindings.data();
	tblci.bindingCount = (uint32_t)cubemapBindings.size();
	tblci.stages = SHADER_STAGE_FRAGMENT_BIT;
	TextureBindingLayout *cubemapLayout = graphics_wrapper_->CreateTextureBindingLayout(tblci);

	SingleTextureBind stb;
	stb.texture = m_cubemap;
	stb.address = 4;

	TextureBindingCreateInfo ci;
	ci.textures = &stb;
	ci.layout = cubemapLayout;
	ci.textureCount = 1;
	m_cubemapBinding = m_graphics_wrapper_->CreateTextureBinding(ci);

	ShaderStageCreateInfo vi;
	ShaderStageCreateInfo fi;
	if (engine.settings.graphicsLanguage == GRAPHICS_OPENGL) {
		vi.fileName = "../assets/shaders/lights_deferred/spotVert.glsl";
		fi.fileName = "../assets/shaders/lights_deferred/ibl.glsl";
	}
	else if (engine.settings.graphicsLanguage == GRAPHICS_DIRECTX) {
		vi.fileName = "../assets/shaders/lights_deferred/pointVert.fxc";
		fi.fileName = "../assets/shaders/lights_deferred/ibl.fxc";
	}
	else {
		vi.fileName = "../assets/shaders/lights_deferred/spotVert.spv";
		fi.fileName = "../assets/shaders/lights_deferred/ibl.spv";
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

	//=====================
	// SSAO
	//=====================

	UniformBufferBindingCreateInfo ubbci;
	ubbci.binding = 0;
	ubbci.shaderLocation = "SSAOBufferObject";
	ubbci.size = sizeof(SSAOBufferObject);
	ubbci.stages = SHADER_STAGE_FRAGMENT_BIT;
	UniformBufferBinding *ubb = graphics_wrapper_->CreateUniformBufferBinding(ubbci);

	UniformBufferCreateInfo ubci;
	ubci.isDynamic = false;
	ubci.size = sizeof(SSAOBufferObject);
	ubci.binding = ubb;
	ssao_ub = graphics_wrapper_->CreateUniformBuffer(ubci);

	int noise_dim = 4;
	int noise_size = noise_dim * noise_dim * 2;
	unsigned char noise[noise_size];
	for (int i = 0; i < noise_size; i += 2) {
		glm::vec3 pixel = glm::normalize(glm::vec3(rand(), rand(), 0));
		noise[i] = pixel.x;
		noise[i] = pixel.y;
	}
	
	int kernel_size = 32;
	for (int i = 0; i < kernel_size; ++i) {
		glm::vec3 kernel = glm::vec3(
			(2.0f * (float)rand() / (float)RAND_MAX) - 1.0f,
			(2.0f * (float)rand() / (float)RAND_MAX) - 1.0f,
			(float)rand() / (float)RAND_MAX);

		kernel = glm::normalize(kernel);
		
		float scale = float(i) / float(kernel_size);
		scale = (scale * scale) * (1.0 - 0.1f) + 0.1f;
		kernel *= scale;
		ssao_buffer.kernel[i] = kernel;
	}

	ssao_buffer.radius = 1.2f;
    ssao_buffer.bias = 0.025f;

	ssao_ub->UpdateUniformBuffer(&ssao_buffer);

	TextureCreateInfo ssao_noise_ci;
	ssao_noise_ci.data = noise;
	ssao_noise_ci.format = FORMAT_COLOR_R8G8;
	ssao_noise_ci.mipmaps = 0;
	ssao_noise_ci.width = noise_dim;
	ssao_noise_ci.height = noise_dim;
	ssao_noise_ci.ddscube = false;

	Texture *ssao_noise_ = m_graphics_wrapper_->CreateTexture(ssao_noise_ci);

	TextureSubBinding ssao_noise_sub_binding_ = TextureSubBinding("ssao_noise", 4);

	tblci.bindingLocation = 1;
	tblci.bindings = &ssao_noise_sub_binding_;
	tblci.bindingCount = 1;
	tblci.stages = SHADER_STAGE_FRAGMENT_BIT;
	TextureBindingLayout *ssao_noise_binding_layout = graphics_wrapper_->CreateTextureBindingLayout(tblci);

	stb.texture = ssao_noise_;
	stb.address = 4;

	ci.textures = &stb;
	ci.layout = ssao_noise_binding_layout;
	ci.textureCount = 1;
	ssao_noise_binding_ = m_graphics_wrapper_->CreateTextureBinding(ci);

	if (engine.settings.graphicsLanguage == GRAPHICS_OPENGL) {
		vi.fileName = "../assets/shaders/lights_deferred/spotVert.glsl";
		fi.fileName = "../assets/shaders/post_processing/ssao.glsl";
	}
	else if (engine.settings.graphicsLanguage == GRAPHICS_DIRECTX) {
		vi.fileName = "../assets/shaders/lights_deferred/pointVert.fxc";
		fi.fileName = "../assets/shaders/post_processing/ssao.fxc";
	}
	else {
		vi.fileName = "../assets/shaders/lights_deferred/spotVert.spv";
		fi.fileName = "../assets/shaders/post_processing/ssao.spv";
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

	stages = { vi, fi };

	GraphicsPipelineCreateInfo ssaoGPCI;
	ssaoGPCI.cullMode = CULL_BACK;
	ssaoGPCI.bindings = &engine.planeVBD;
	ssaoGPCI.bindingsCount = 1;
	ssaoGPCI.attributes = &engine.planeVAD;
	ssaoGPCI.attributesCount = 1;
	ssaoGPCI.width = (float)engine.settings.resolutionX; /// DIVIDE BY TWO
	ssaoGPCI.height = (float)engine.settings.resolutionY;
	ssaoGPCI.scissorW = engine.settings.resolutionX;
	ssaoGPCI.scissorH = engine.settings.resolutionY;
	ssaoGPCI.primitiveType = PRIM_TRIANGLES;
	ssaoGPCI.shaderStageCreateInfos = stages.data();
	ssaoGPCI.shaderStageCreateInfoCount = (uint32_t)stages.size();
	tbls = { engine.tbl, ssao_noise_binding_layout };

	ssaoGPCI.textureBindings = tbls.data();
	ssaoGPCI.textureBindingCount = (uint32_t)tbls.size();
	ssaoGPCI.uniformBufferBindings = &engine.deffubb;
	ssaoGPCI.uniformBufferBindingCount = 1;
	ssao_pipeline_ = graphics_wrapper_->CreateGraphicsPipeline(ssaoGPCI);
	/*
	//=====================
	// SSAO Blur
	//=====================
	std::vector<TextureSubBinding> cubemapBindings;
	cubemapBindings.reserve(1);
	cubemapBindings.emplace_back("environmentMap", 4);

	TextureBindingLayoutCreateInfo tblci;
	tblci.bindingLocation = 1;
	tblci.bindings = cubemapBindings.data();
	tblci.bindingCount = (uint32_t)cubemapBindings.size();
	tblci.stages = SHADER_STAGE_FRAGMENT_BIT;
	TextureBindingLayout *cubemapLayout = graphics_wrapper_->CreateTextureBindingLayout(tblci);

	SingleTextureBind stb;
	stb.texture = m_cubemap;
	stb.address = 4;

	TextureBindingCreateInfo ci;
	ci.textures = &stb;
	ci.layout = cubemapLayout;
	ci.textureCount = 1;
	m_cubemapBinding = m_graphics_wrapper_->CreateTextureBinding(ci);

	ShaderStageCreateInfo vi;
	ShaderStageCreateInfo fi;
	if (engine.settings.graphicsLanguage == GRAPHICS_OPENGL) {
		vi.fileName = "../assets/shaders/lights_deferred/spotVert.glsl";
		fi.fileName = "../assets/shaders/lights_deferred/ibl.glsl";
	}
	else if (engine.settings.graphicsLanguage == GRAPHICS_DIRECTX) {
		vi.fileName = "../assets/shaders/lights_deferred/pointVert.fxc";
		fi.fileName = "../assets/shaders/lights_deferred/ibl.fxc";
	}
	else {
		vi.fileName = "../assets/shaders/lights_deferred/spotVert.spv";
		fi.fileName = "../assets/shaders/lights_deferred/ibl.spv";
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
	m_iblPipeline = graphics_wrapper_->CreateGraphicsPipeline(iblGPCI);*/
}

void RenderPathDeferred::Draw(Framebuffer *gbuffer) {
	engine.deffUBO->Bind();
	engine.graphics_wrapper_->BindVertexArrayObject(plane_vao_);
	
	if (engine.settings.use_ssao) {
		gbuffer->BindRead();
		gbuffer->BindTextures(0);
		m_graphics_wrapper_->EnableDepth(false);
		m_graphics_wrapper_->SetColorMask(COLOR_MASK_ALPHA);
		ssao_pipeline_->Bind();
		m_graphics_wrapper_->BindTextureBinding(ssao_noise_binding_);
		m_graphics_wrapper_->DrawImmediateVertices(0, 6);
		m_graphics_wrapper_->SetColorMask(COLOR_MASK_RGBA);
		m_graphics_wrapper_->EnableDepth(true);
	}
	
	m_graphics_wrapper_->SetImmediateBlending(BLEND_ADDITIVE);
	m_graphics_wrapper_->BindDefaultFramebuffer(false);
	m_graphics_wrapper_->Clear();
	gbuffer->BindRead();
	gbuffer->BindTextures(0);
	engine.lightSystem.m_pointLightPipeline->Bind();
	for (auto &light : engine.lightSystem.pointLights) {
		light.Bind();

		m_graphics_wrapper_->DrawImmediateVertices(0, 6);
	}

	engine.lightSystem.m_spotLightPipeline->Bind();
	for (auto &light : engine.lightSystem.spotLights) {
		light.Bind();

		m_graphics_wrapper_->DrawImmediateVertices(0, 6);
	}

	engine.lightSystem.m_directionalLightPipeline->Bind();
	for (auto &light : engine.lightSystem.directionalLights) {
		light.Bind();

		m_graphics_wrapper_->DrawImmediateVertices(0, 6);
	}

	if (m_cubemap) {
		m_iblPipeline->Bind();
		m_graphics_wrapper_->BindTextureBinding(m_cubemapBinding);
		m_graphics_wrapper_->DrawImmediateVertices(0, 6);
	}
}
