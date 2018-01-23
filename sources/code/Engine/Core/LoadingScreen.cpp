#include "LoadingScreen.hpp"
#include "Engine.hpp"
#include <stb/stb_image.h>

LoadingScreen::LoadingScreen(GraphicsWrapper *gw) : graphics_wrapper_(gw) {
	VertexBindingDescription vbd;
	vbd.binding = 0;
	vbd.elementRate = false;
	vbd.stride = sizeof(glm::vec2);

	VertexAttributeDescription vad;
	vad.binding = 0;
	vad.location = 0;
	vad.format = VERTEX_R32_G32;
	vad.size = 2;
	vad.name = "vertexPosition";
	vad.offset = 0;
	vad.usage = ATTRIB_POSITION;

	RenderPassCreateInfo rpci;
	ClearColorValue colorClearValues = { 1.0f, 0.0f, 0.0f, 1.0f };
	ClearDepthStencil dscf;
	dscf.hasDepthStencilAttachment = false;
	dscf.depth = 1.0f;
	dscf.stencil = 0;
	rpci.m_colorClearValues = &colorClearValues;
	rpci.m_colorClearCount = (uint32_t)1;
	rpci.m_depthStencilClearValue = dscf;
	rpci.m_width = engine.settings.resolutionX;
	rpci.m_height = engine.settings.resolutionY;
	rpci.m_depthFormat = FORMAT_DEPTH_NONE;
	render_pass_ = graphics_wrapper_->CreateRenderPass(rpci);

	GraphicsPipelineCreateInfo gpci;
	gpci.scissorW = engine.settings.resolutionX;
	gpci.width = static_cast<float>(engine.settings.resolutionX);
	gpci.scissorH = engine.settings.resolutionY;
	gpci.height = static_cast<float>(engine.settings.resolutionY);
	gpci.renderPass = render_pass_;
	gpci.attributes = &vad;
	gpci.attributesCount = (uint32_t)1;
	gpci.bindings = &vbd;
	gpci.bindingsCount = 1;

	ShaderStageCreateInfo vi;
	ShaderStageCreateInfo fi;
	if (engine.settings.graphicsLanguage == GRAPHICS_OPENGL) {
		vi.fileName = "../assets/shaders/loading_screen/loading_vert.glsl";
		fi.fileName = "../assets/shaders/loading_screen/loading_frag.glsl";
	}
	else if (engine.settings.graphicsLanguage == GRAPHICS_DIRECTX) {
		vi.fileName = "../assets/shaders/loading_screen/loading_vert.fxc";
		fi.fileName = "../assets/shaders/loading_screen/loading_frag.fxc";
	}
	else {
		vi.fileName = "../assets/shaders/loading_screen/loading_vert.spv";
		fi.fileName = "../assets/shaders/loading_screen/loading_frag.spv";
	}
	std::vector<char> vfile;
	if (!readFile(vi.fileName, vfile)) {
		std::cout << "Could not load vertex shader for loading screen!\n";
		return;
	}

	vi.content = vfile.data();
	vi.size = (uint32_t)vfile.size();
	vi.type = SHADER_VERTEX;

	std::vector<char> ffile;
	if (!readFile(fi.fileName, ffile)) {
		std::cout << "Could not load fragment shader for loading screen!\n";
		return;
	}

	fi.content = ffile.data();
	fi.size = (uint32_t)ffile.size();
	fi.type = SHADER_FRAGMENT;

	TextureSubBinding binding = {"logo", 0};

	TextureBindingLayoutCreateInfo tblci;
	tblci.bindingLocation = 0;
	tblci.bindings = &binding;
	tblci.bindingCount = (uint32_t)1;
	tblci.stages = SHADER_STAGE_FRAGMENT_BIT;
	tbl_ = graphics_wrapper_->CreateTextureBindingLayout(tblci);

	std::vector<ShaderStageCreateInfo> stages = { vi, fi };
	gpci.shaderStageCreateInfos = stages.data();
	gpci.shaderStageCreateInfoCount = (uint32_t)stages.size();
	gpci.uniformBufferBindings = nullptr;
	gpci.uniformBufferBindingCount = 0;
	gpci.textureBindings = &tbl_;
	gpci.textureBindingCount = 1;
	gpci.cullMode = CULL_NONE;
	gpci.primitiveType = PRIM_TRIANGLES;
	pipeline_ = graphics_wrapper_->CreateGraphicsPipeline(gpci);

	int texWidth, texHeight, texChannels;
	stbi_uc* pixels = stbi_load("../assets/materials/grindstone.png", &texWidth, &texHeight, &texChannels, STBI_rgb_alpha);
	texChannels = 4;
	if (!pixels) {
		printf("Texture failed to load!: materials/grindstone.png \n");
		return;
	}
	TextureCreateInfo createInfo;
	createInfo.format = FORMAT_COLOR_R8G8B8A8;
	createInfo.mipmaps = 0;
	createInfo.data = pixels;
	createInfo.width = texWidth;
	createInfo.height = texHeight;

	texture_ = graphics_wrapper_->CreateTexture(createInfo);

	stbi_image_free(pixels);

	SingleTextureBind stb;
	stb.texture = texture_;
	stb.address = 0;

	TextureBindingCreateInfo tbci;
	tbci.textures = &stb;
	tbci.textureCount = 1;
	tbci.layout = tbl_;
	tb_ = graphics_wrapper_->CreateTextureBinding(tbci);

	float planeVerts[4 * 6] = {
		-1.0, -1.0,
		1.0, -1.0,
		-1.0,  1.0,
		1.0,  1.0,
		-1.0,  1.0,
		1.0, -1.0,
	};

	VertexBufferCreateInfo planeVboCI;
	planeVboCI.binding = &engine.planeVBD;
	planeVboCI.bindingCount = 1;
	planeVboCI.attribute = &engine.planeVAD;
	planeVboCI.attributeCount = 1;
	planeVboCI.content = planeVerts;
	planeVboCI.count = 6;
	planeVboCI.size = sizeof(float) * 6 * 2;

	VertexArrayObjectCreateInfo vaci;
	vaci.vertexBuffer = vbo_;
	vaci.indexBuffer = nullptr;
	vao_ = graphics_wrapper_->CreateVertexArrayObject(vaci);
	vbo_ = graphics_wrapper_->CreateVertexBuffer(planeVboCI);

	vaci.vertexBuffer = vbo_;
	vaci.indexBuffer = nullptr;
	vao_->BindResources(vaci);
	vao_->Unbind();
	graphics_wrapper_->BindVertexArrayObject(vao_);

	UniformBufferBindingCreateInfo ubbci;
	ubbci.binding = 0;
	ubbci.shaderLocation = "UniformBufferObject";
	ubbci.size = sizeof(loadUBO);
	ubbci.stages = SHADER_STAGE_FRAGMENT_BIT;
	ubb_ = graphics_wrapper_->CreateUniformBufferBinding(ubbci);

	UniformBufferCreateInfo ubci;
	ubci.isDynamic = false;
	ubci.size = sizeof(loadUBO);
	ubci.binding = ubb_;
	ubo_ = graphics_wrapper_->CreateUniformBuffer(ubci);

	loadUBO.aspect = (float)engine.settings.resolutionX / (float)engine.settings.resolutionY;
	Render(0.0f);
}

void LoadingScreen::Render(double dt) {
	loadUBO.time = (float)dt;
	ubo_->UpdateUniformBuffer(&loadUBO);
	ubo_->Bind();

	graphics_wrapper_->BindDefaultFramebuffer(true);
	graphics_wrapper_->Clear();
	pipeline_->Bind();
	graphics_wrapper_->BindTextureBinding(tb_);
	graphics_wrapper_->DrawImmediateVertices(0, 6);
	graphics_wrapper_->SwapBuffer();
}

LoadingScreen::~LoadingScreen() {
	graphics_wrapper_->DeleteTexture(texture_);
	graphics_wrapper_->DeleteGraphicsPipeline(pipeline_);
}
