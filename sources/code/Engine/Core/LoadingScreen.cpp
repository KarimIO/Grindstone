#if 0
#include "LoadingScreen.hpp"
#include "Engine.hpp"
#include <stb/stb_image.h>

LoadingScreen::LoadingScreen(GraphicsWrapper *gw) : graphics_wrapper_(gw) {
	Grindstone::GraphicsAPI::VertexBindingDescription vbd;
	vbd.binding = 0;
	vbd.elementRate = false;
	vbd.stride = sizeof(glm::vec2);

	Grindstone::GraphicsAPI::VertexAttributeDescription vad;
	vad.binding = 0;
	vad.location = 0;
	vad.format = Grindstone::GraphicsAPI::VertexFormat::R32_G32;
	vad.size = 2;
	vad.name = "vertexPosition";
	vad.offset = 0;
	vad.usage = Grindstone::GraphicsAPI::AttributeUsage::Position;

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
	rpci.m_depthFormat = DepthFormat::DNONE;
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

	Grindstone::GraphicsAPI::ShaderStageCreateInfo vi;
	Grindstone::GraphicsAPI::ShaderStageCreateInfo fi;
	if (engine.settings.graphicsLanguage == GraphicsLanguage::OpenGL) {
		vi.fileName = "../assets/shaders/loading_screen/loading_vert.glsl";
		fi.fileName = "../assets/shaders/loading_screen/loading_frag.glsl";
	}
	else if (engine.settings.graphicsLanguage == GraphicsLanguage::DirectX) {
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
	vi.type = Grindstone::GraphicsAPI::ShaderStage::Vertex;

	std::vector<char> ffile;
	if (!readFile(fi.fileName, ffile)) {
		std::cout << "Could not load fragment shader for loading screen!\n";
		return;
	}

	fi.content = ffile.data();
	fi.size = (uint32_t)ffile.size();
	fi.type = Grindstone::GraphicsAPI::ShaderStage::Fragment;

	TextureSubBinding binding = {"logo", 0};

	TextureBindingLayoutCreateInfo tblci;
	tblci.bindingLocation = 0;
	tblci.bindings = &binding;
	tblci.bindingCount = (uint32_t)1;
	tblci.stages = Grindstone::GraphicsAPI::ShaderStageBit::Fragment;
	tbl_ = graphics_wrapper_->CreateTextureBindingLayout(tblci);

	std::vector<Grindstone::GraphicsAPI::ShaderStageCreateInfo> stages = { vi, fi };
	gpci.shaderStageCreateInfos = stages.data();
	gpci.shaderStageCreateInfoCount = (uint32_t)stages.size();
	gpci.uniformBufferBindings = nullptr;
	gpci.uniformBufferBindingCount = 0;
	gpci.textureBindings = &tbl_;
	gpci.textureBindingCount = 1;
	gpci.cullMode = Grindstone::GraphicsAPI::CullMode::None;
	gpci.primitiveType = Grindstone::GraphicsAPI::GeometryType::Triangles;
	pipeline_ = graphics_wrapper_->CreateGraphicsPipeline(gpci);

	int texWidth, texHeight, texChannels;
	stbi_uc* pixels = stbi_load("../assets/materials/grindstone.png", &texWidth, &texHeight, &texChannels, STBI_rgb_alpha);
	texChannels = 4;
	if (!pixels) {
		printf("Texture failed to load!: materials/grindstone.png \n");
		return;
	}
	Grindstone::GraphicsAPI::TextureCreateInfo createInfo;
	createInfo.format = Grindstone::GraphicsAPI::ColorFormat::R8G8B8A8;
	createInfo.mipmaps = 0;
	createInfo.data = pixels;
	createInfo.width = texWidth;
	createInfo.height = texHeight;
	createInfo.ddscube = false;

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

	Grindstone::GraphicsAPI::VertexBufferCreateInfo planeVboCI;
	planeVboCI.binding = &engine.planeVBD;
	planeVboCI.bindingCount = 1;
	planeVboCI.attribute = &engine.planeVAD;
	planeVboCI.attributeCount = 1;
	planeVboCI.content = planeVerts;
	planeVboCI.count = 6;
	planeVboCI.size = sizeof(float) * 6 * 2;

	Grindstone::GraphicsAPI::VertexArrayObjectCreateInfo vaci;
	vaci.vertexBuffer = vbo_;
	vaci.indexBuffer = nullptr;
	vao_ = graphics_wrapper_->CreateVertexArrayObject(vaci);
	vbo_ = graphics_wrapper_->CreateVertexBuffer(planeVboCI);

	vaci.vertexBuffer = vbo_;
	vaci.indexBuffer = nullptr;
	vao_->BindResources(vaci);
	vao_->Unbind();
	graphics_wrapper_->BindVertexArrayObject(vao_);

	Grindstone::GraphicsAPI::UniformBufferBindingCreateInfo ubbci;
	ubbci.binding = 0;
	ubbci.shaderLocation = "UniformBufferObject";
	ubbci.size = sizeof(loadUBO);
	ubbci.stages = Grindstone::GraphicsAPI::ShaderStageBit::Fragment;
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
	graphics_wrapper_->Clear(Grindstone::GraphicsAPI::ClearMode::Both);
	pipeline_->Bind();
	graphics_wrapper_->BindTextureBinding(tb_);
	graphics_wrapper_->DrawImmediateVertices(0, 6);
	graphics_wrapper_->SwapBuffer();
}

LoadingScreen::~LoadingScreen() {
	graphics_wrapper_->DeleteTexture(texture_);
	graphics_wrapper_->DeleteGraphicsPipeline(pipeline_);
}
#endif