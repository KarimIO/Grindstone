#include "PostProcessIBL.hpp"
#include "../Core/Engine.hpp"
#include "Core/Utilities.hpp"
#include <GraphicsCommon/GraphicsWrapper.hpp>
#include "Systems/CubemapSystem.hpp"
#include "PostPipeline.hpp"
#include "Core/Space.hpp"

PostProcessIBL::PostProcessIBL(unsigned int w, unsigned int h, PostPipeline *pipeline, RenderTargetContainer *target) : BasePostProcess(pipeline), target_(target), viewport_w_(w), viewport_h_(h) {
	reloadGraphics(w, h);
}

PostProcessIBL::~PostProcessIBL() {
	destroyGraphics();
}

void PostProcessIBL::prepareIBL(unsigned int w, unsigned h) {
	auto graphics_wrapper = engine.getGraphicsWrapper();
	auto settings = engine.getSettings();

    Grindstone::GraphicsAPI::ShaderStageCreateInfo vi;
	Grindstone::GraphicsAPI::ShaderStageCreateInfo fi;
	if (settings->graphics_language_ == GraphicsLanguage::OpenGL) {
		vi.fileName = "../assets/shaders/lights_deferred/spotVert.glsl";
		fi.fileName = "../assets/shaders/lights_deferred/ibl.glsl";
	}
	else if (settings->graphics_language_ == GraphicsLanguage::DirectX) {
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
	vi.type = Grindstone::GraphicsAPI::ShaderStage::Vertex;

	std::vector<char> ffile;
	if (!readFile(fi.fileName, ffile))
		return;
	fi.content = ffile.data();
	fi.size = (uint32_t)ffile.size();
	fi.type = Grindstone::GraphicsAPI::ShaderStage::Fragment;

	std::vector<Grindstone::GraphicsAPI::ShaderStageCreateInfo> stages = { vi, fi };
	
	subbinding_ = Grindstone::GraphicsAPI::TextureSubBinding("environmentMap", 4);
	Grindstone::GraphicsAPI::TextureBindingLayoutCreateInfo tblci;
	tblci.bindingLocation = 4;
	tblci.bindings = &subbinding_;
	tblci.bindingCount = (uint32_t)1;
	tblci.stages = Grindstone::GraphicsAPI::ShaderStageBit::Fragment;
	env_map_ = graphics_wrapper->createTextureBindingLayout(tblci);
	
	auto vertex_layout = engine.getPlaneVertexLayout();

	Grindstone::GraphicsAPI::GraphicsPipelineCreateInfo iblGPCI;
	iblGPCI.cullMode = Grindstone::GraphicsAPI::CullMode::Back;
	iblGPCI.vertex_bindings = &vertex_layout;
	iblGPCI.vertex_bindings_count = 1;
	iblGPCI.width = (float)w;
	iblGPCI.height = (float)h;
	iblGPCI.scissorW = w;
	iblGPCI.scissorH = h;
	iblGPCI.primitiveType = Grindstone::GraphicsAPI::GeometryType::Triangles;
	iblGPCI.shaderStageCreateInfos = stages.data();
	iblGPCI.shaderStageCreateInfoCount = (uint32_t)stages.size();
	std::vector<Grindstone::GraphicsAPI::TextureBindingLayout*> tbls_refl = { engine.getGbufferTBL(), env_map_ }; // refl_tbl

	tbls_refl.push_back(ssao_layout_);

	iblGPCI.textureBindings = tbls_refl.data();
	iblGPCI.textureBindingCount = (uint32_t)tbls_refl.size();
	iblGPCI.uniformBufferBindings = &engine.deff_ubb_;
	iblGPCI.uniformBufferBindingCount = 1;
	gpipeline_ = graphics_wrapper->createGraphicsPipeline(iblGPCI);
}

void PostProcessIBL::prepareSSAO(unsigned int w, unsigned h) {
	Grindstone::GraphicsAPI::GraphicsWrapper *graphics_wrapper = engine.getGraphicsWrapper();
	auto settings = engine.getSettings();

	Grindstone::GraphicsAPI::RenderTargetCreateInfo ssao_buffer_ci(Grindstone::GraphicsAPI::ColorFormat::R8, w, h);
	ssao_buffer_ = graphics_wrapper->createRenderTarget(&ssao_buffer_ci, 1);

	Grindstone::GraphicsAPI::FramebufferCreateInfo hdr_framebuffer_ci;
	hdr_framebuffer_ci.render_target_lists = &ssao_buffer_;
	hdr_framebuffer_ci.num_render_target_lists = 1;
	hdr_framebuffer_ci.depth_target = nullptr; // depth_image_;
	hdr_framebuffer_ci.render_pass = nullptr;
	ssao_fbo_ = graphics_wrapper->createFramebuffer(hdr_framebuffer_ci);

	//=====================
	// SSAO
	//=====================

	Grindstone::GraphicsAPI::UniformBufferBindingCreateInfo ubbci;
	ubbci.binding = 1;
	ubbci.shaderLocation = "SSAOBufferObject";
	ubbci.size = sizeof(SSAOBufferObject);
	ubbci.stages = Grindstone::GraphicsAPI::ShaderStageBit::Fragment;
	Grindstone::GraphicsAPI::UniformBufferBinding *ubb = graphics_wrapper->createUniformBufferBinding(ubbci);

	Grindstone::GraphicsAPI::UniformBufferCreateInfo ubci;
	ubci.isDynamic = false;
	ubci.size = sizeof(SSAOBufferObject);
	ubci.binding = ubb;
	ssao_ub = graphics_wrapper->createUniformBuffer(ubci);

	const int noise_dim = 4;
	const int noise_size = noise_dim * noise_dim * 2;
	unsigned char noise[noise_size];
	for (int i = 0; i < noise_size; i += 2) {
		glm::vec3 pixel = glm::normalize(glm::vec3(
			(2.0f * (float)rand() / (float)RAND_MAX) - 1.0f,
			(2.0f * (float)rand() / (float)RAND_MAX) - 1.0f, 0));
		noise[i] = (unsigned char)((pixel.x / 2.0f + 0.5f) * 255);
		noise[i + 1] = (unsigned char)((pixel.y / 2.0f + 0.5f) * 255);
	}

	int kernel_size = 32;
	for (int i = 0; i < 32; ++i) {
		glm::vec3 kernel = glm::vec3(
			(2.0f * (float)rand() / (float)RAND_MAX) - 1.0f,
			(2.0f * (float)rand() / (float)RAND_MAX) - 1.0f,
			(float)rand() / (float)RAND_MAX);

		kernel = glm::normalize(kernel);

		float scale = float(i) / float(kernel_size);
		scale = (scale * scale) * (1.0f - 0.1f) + 0.1f;
		kernel *= scale;
		ssao_buffer.kernel[i * 4] = kernel.x;
		ssao_buffer.kernel[i * 4 + 1] = kernel.y;
		ssao_buffer.kernel[i * 4 + 2] = kernel.z;
	}

	ssao_buffer.radius = 0.3f;
	ssao_buffer.bias = 0.025f;

	ssao_ub->updateBuffer(&ssao_buffer);

	Grindstone::GraphicsAPI::TextureCreateInfo ssao_noise_ci;
	ssao_noise_ci.data = noise;
	ssao_noise_ci.format = Grindstone::GraphicsAPI::ColorFormat::R8G8;
	ssao_noise_ci.mipmaps = 0;
	ssao_noise_ci.width = noise_dim;
	ssao_noise_ci.height = noise_dim;
	ssao_noise_ci.ddscube = false;

	Grindstone::GraphicsAPI::Texture *ssao_noise_ = graphics_wrapper->createTexture(ssao_noise_ci);

	Grindstone::GraphicsAPI::TextureSubBinding ssao_noise_sub_binding_ = Grindstone::GraphicsAPI::TextureSubBinding("ssao_noise", 4);

	Grindstone::GraphicsAPI::TextureBindingLayoutCreateInfo tblci;
	tblci.bindingLocation = 4;
	tblci.bindings = &ssao_noise_sub_binding_;
	tblci.bindingCount = 1;
	tblci.stages = Grindstone::GraphicsAPI::ShaderStageBit::Fragment;
	Grindstone::GraphicsAPI::TextureBindingLayout *ssao_noise_binding_layout = graphics_wrapper->createTextureBindingLayout(tblci);

	Grindstone::GraphicsAPI::SingleTextureBind stb;
	stb.texture = ssao_noise_;
	stb.address = 4;

	Grindstone::GraphicsAPI::TextureBindingCreateInfo ci;
	ci.textures = &stb;
	ci.layout = ssao_noise_binding_layout;
	ci.textureCount = 1;
	ssao_noise_binding_ = graphics_wrapper->createTextureBinding(ci);

	Grindstone::GraphicsAPI::ShaderStageCreateInfo vi;
	Grindstone::GraphicsAPI::ShaderStageCreateInfo fi;
	std::vector<char> vfile;
	std::vector<char> ffile;

	if (settings->graphics_language_ == GraphicsLanguage::OpenGL) {
		vi.fileName = "../assets/shaders/lights_deferred/spotVert.glsl";
		fi.fileName = "../assets/shaders/post_processing/ssao.glsl";
	}
	else if (settings->graphics_language_ == GraphicsLanguage::DirectX) {
		vi.fileName = "../assets/shaders/lights_deferred/pointVert.fxc";
		fi.fileName = "../assets/shaders/post_processing/ssao.fxc";
	}
	else {
		vi.fileName = "../assets/shaders/lights_deferred/spotVert.spv";
		fi.fileName = "../assets/shaders/post_processing/ssao.spv";
	}

	vfile.clear();
	if (!readFile(vi.fileName, vfile)) {
		throw std::runtime_error("SSAO Vertex Shader missing.\n");
		return;
	}
	vi.content = vfile.data();
	vi.size = (uint32_t)vfile.size();
	vi.type = Grindstone::GraphicsAPI::ShaderStage::Vertex;

	ffile.clear();
	if (!readFile(fi.fileName, ffile)) {
		throw std::runtime_error("SSAO Fragment Shader missing.\n");
		return;
	}
	fi.content = ffile.data();
	fi.size = (uint32_t)ffile.size();
	fi.type = Grindstone::GraphicsAPI::ShaderStage::Fragment;

	Grindstone::GraphicsAPI::ShaderStageCreateInfo stages[2] = { vi, fi };

	auto vertex_layout = engine.getPlaneVertexLayout();

	Grindstone::GraphicsAPI::GraphicsPipelineCreateInfo ssaoGPCI;
	ssaoGPCI.cullMode = Grindstone::GraphicsAPI::CullMode::Back;
	ssaoGPCI.vertex_bindings = &vertex_layout;
	ssaoGPCI.vertex_bindings_count = 1;
	ssaoGPCI.width = (float)w; // DIVIDE BY TWO
	ssaoGPCI.height = (float)h;
	ssaoGPCI.scissorW = w;
	ssaoGPCI.scissorH = h;
	ssaoGPCI.primitiveType = Grindstone::GraphicsAPI::GeometryType::Triangles;
	ssaoGPCI.shaderStageCreateInfos = stages;
	ssaoGPCI.shaderStageCreateInfoCount = 2;
	std::vector<Grindstone::GraphicsAPI::TextureBindingLayout *>tbls = { engine.getGbufferTBL(), ssao_noise_binding_layout };

	std::vector<Grindstone::GraphicsAPI::UniformBufferBinding*> ubbs = { engine.deff_ubb_ , ubb };
	ssaoGPCI.textureBindings = tbls.data();
	ssaoGPCI.textureBindingCount = (uint32_t)tbls.size();
	ssaoGPCI.uniformBufferBindings = ubbs.data();
	ssaoGPCI.uniformBufferBindingCount = (uint32_t)ubbs.size();
	pipeline_ = graphics_wrapper->createGraphicsPipeline(ssaoGPCI);

	// Export SSAO Layout
	ssao_output_ = Grindstone::GraphicsAPI::TextureSubBinding("ssao", 5);

	Grindstone::GraphicsAPI::TextureBindingLayoutCreateInfo stblci;
	stblci.bindingLocation = 5;
	stblci.bindings = &ssao_output_;
	stblci.bindingCount = 1;
	stblci.stages = Grindstone::GraphicsAPI::ShaderStageBit::Fragment;
	ssao_layout_ = graphics_wrapper->createTextureBindingLayout(stblci);
}

void PostProcessIBL::ssao() {
	GRIND_PROFILE_FUNC();
	Grindstone::GraphicsAPI::GraphicsWrapper *graphics_wrapper = engine.getGraphicsWrapper();

	//engine.getGraphicsWrapper()->bindDefaultFramebuffer(true);
	ssao_fbo_->BindWrite(true);
	float col[4] = { 0.0f, 0.0f, 0.0f, 1.0f };
	engine.getGraphicsWrapper()->clear(Grindstone::GraphicsAPI::ClearMode::ColorAndDepth, col, 1.0f, 0);

	/*if (source_ != nullptr) {
		source_->framebuffer->BindRead();
		source_->framebuffer->BindTextures(0);
	}*/

	engine.getGraphicsWrapper()->bindTextureBinding(ssao_noise_binding_);
	
	graphics_wrapper->enableDepth(false);
	//graphics_wrapper->SetColorMask(COLOR_MASK_ALPHA);
	ssao_ub->Bind();
	pipeline_->Bind();
	graphics_wrapper->bindTextureBinding(ssao_noise_binding_);
	graphics_wrapper->drawImmediateVertices(Grindstone::GraphicsAPI::GeometryType::Triangles, 0, 6);
	//graphics_wrapper->SetColorMask(COLOR_MASK_RGBA);
	graphics_wrapper->enableDepth(true);
}

void PostProcessIBL::ibl() {
	GRIND_PROFILE_FUNC();
	auto graphics_wrapper = engine.getGraphicsWrapper();

	graphics_wrapper->bindVertexArrayObject(engine.getPlaneVAO());

	graphics_wrapper->setImmediateBlending(Grindstone::GraphicsAPI::BlendMode::Additive);

	//graphics_wrapper->bindDefaultFramebuffer(true);
	//engine.getGraphicsWrapper()->clear(Grindstone::GraphicsAPI::ClearMode::ColorAndDepth);
	ssao_fbo_->BindRead();
	ssao_fbo_->BindTextures(5);
	target_->framebuffer->BindWrite(false);
	//source_->framebuffer->BindRead();
	//target_->framebuffer->BindTextures(0);
	engine.deff_ubo_handler_->Bind();

	glm::vec3 pos = glm::vec3(0, 0, 0); // engine.deffUBOBuffer.eyePos;
	Space *s = getPipeline()->getSpace();
	CubemapSubSystem *sys = ((CubemapSubSystem *)s->getSubsystem(COMPONENT_CUBEMAP));
	CubemapComponent *cube = sys->getClosestCubemap(pos);
	if (cube && cube->cubemap_) {
		graphics_wrapper->bindTextureBinding(cube->cubemap_binding_);

		gpipeline_->Bind();
		graphics_wrapper->drawImmediateVertices(Grindstone::GraphicsAPI::GeometryType::Triangles, 0, 6);
		graphics_wrapper->setImmediateBlending(Grindstone::GraphicsAPI::BlendMode::None);
	}
}

void PostProcessIBL::Process() {
	GRIND_PROFILE_FUNC();
	if (usesSSAO()) {
		ssao();
	}
	ibl();
}

void PostProcessIBL::recreateFramebuffer(unsigned int w, unsigned int h) {
	auto graphics_wrapper = engine.getGraphicsWrapper();

	// graphics_wrapper->DeleteRenderTarget(ssao_buffer_);
	graphics_wrapper->deleteFramebuffer(ssao_fbo_);

	Grindstone::GraphicsAPI::RenderTargetCreateInfo ssao_buffer_ci(Grindstone::GraphicsAPI::ColorFormat::R8, w, h);
	ssao_buffer_ = graphics_wrapper->createRenderTarget(&ssao_buffer_ci, 1);

	Grindstone::GraphicsAPI::FramebufferCreateInfo hdr_framebuffer_ci;
	hdr_framebuffer_ci.render_target_lists = &ssao_buffer_;
	hdr_framebuffer_ci.num_render_target_lists = 1;
	hdr_framebuffer_ci.depth_target = nullptr; // depth_image_;
	hdr_framebuffer_ci.render_pass = nullptr;
	ssao_fbo_ = graphics_wrapper->createFramebuffer(hdr_framebuffer_ci);
}

bool PostProcessIBL::usesSSAO() {
	return true;
}

void PostProcessIBL::resizeBuffers(unsigned int w, unsigned h) {
	prepareSSAO(w, h);
	prepareIBL(w, h);
}

void PostProcessIBL::reloadGraphics(unsigned int w, unsigned h) {
	resizeBuffers(w, h);
}

void PostProcessIBL::destroyGraphics() {
	auto gw = engine.getGraphicsWrapper();
	if (env_map_) {
		gw->deleteTextureBindingLayout(env_map_);
		env_map_ = nullptr;
	}

	if (gpipeline_) {
		gw->deleteGraphicsPipeline(gpipeline_);
		gpipeline_ = nullptr;
	}
	if (pipeline_) {
		gw->deleteGraphicsPipeline(pipeline_);
		pipeline_ = nullptr;
	}
	if (ssao_noise_binding_) {
		gw->deleteTextureBinding(ssao_noise_binding_);
		ssao_noise_binding_ = nullptr;
	}
	if (ssao_ub) {
		gw->deleteUniformBuffer(ssao_ub);
		ssao_ub = nullptr;
	}
	if (ssao_buffer_) {
		gw->deleteRenderTarget(ssao_buffer_);
		ssao_buffer_ = nullptr;
	}
	if (ssao_fbo_) {
		gw->deleteFramebuffer(ssao_fbo_);
		ssao_fbo_ = nullptr;
	}
	if (ssao_layout_) {
		gw->deleteTextureBindingLayout(ssao_layout_);
		ssao_layout_ = nullptr;
	}
}
