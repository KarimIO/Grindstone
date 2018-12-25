#include "PostProcessSSAO.hpp"
#include "../Core/Engine.hpp"
#include "../../GraphicsCommon/UniformBuffer.hpp"
#include <GraphicsWrapper.hpp>
#include <glm/glm.hpp>
#include "Core/Utilities.hpp"

PostProcessSSAO::PostProcessSSAO(PostPipeline *pipeline, RenderTargetContainer *source) : BasePostProcess(pipeline), source_(source) {
    GraphicsWrapper *graphics_wrapper = engine.getGraphicsWrapper();
	auto settings = engine.getSettings();

    //=====================
	// SSAO
	//=====================

	UniformBufferBindingCreateInfo ubbci;
	ubbci.binding = 1;
	ubbci.shaderLocation = "SSAOBufferObject";
	ubbci.size = sizeof(SSAOBufferObject);
	ubbci.stages = SHADER_STAGE_FRAGMENT_BIT;
	UniformBufferBinding *ubb = graphics_wrapper->CreateUniformBufferBinding(ubbci);

	UniformBufferCreateInfo ubci;
	ubci.isDynamic = false;
	ubci.size = sizeof(SSAOBufferObject);
	ubci.binding = ubb;
	ssao_ub = graphics_wrapper->CreateUniformBuffer(ubci);

	const int noise_dim = 4;
	const int noise_size = noise_dim * noise_dim * 2;
	unsigned char noise[noise_size];
	for (int i = 0; i < noise_size; i += 2) {
		glm::vec3 pixel = glm::normalize(glm::vec3(
		(2.0f * (float)rand() / (float)RAND_MAX) - 1.0f,
		(2.0f * (float)rand() / (float)RAND_MAX) - 1.0f, 0));
		noise[i] = (pixel.x / 2.0f + 0.5f) * 255;
		noise[i+1] = (pixel.y / 2.0f + 0.5f) * 255;
	}
	
	int kernel_size = 32;
	for (int i = 0; i < 32; ++i) {
		glm::vec3 kernel = glm::vec3(
			(2.0f * (float)rand() / (float)RAND_MAX) - 1.0f,
			(2.0f * (float)rand() / (float)RAND_MAX) - 1.0f,
			(float)rand() / (float)RAND_MAX);

		kernel = glm::normalize(kernel);
		
		float scale = float(i) / float(kernel_size);
		scale = (scale * scale) * (1.0 - 0.1f) + 0.1f;
		kernel *= scale;
		ssao_buffer.kernel[i * 4] = kernel.x;
		ssao_buffer.kernel[i * 4 + 1] = kernel.y;
		ssao_buffer.kernel[i * 4 + 2] = kernel.z;
	}

	ssao_buffer.radius = 0.8f;
    ssao_buffer.bias = 0.025f;

	ssao_ub->UpdateUniformBuffer(&ssao_buffer);

	TextureCreateInfo ssao_noise_ci;
	ssao_noise_ci.data = noise;
	ssao_noise_ci.format = FORMAT_COLOR_R8G8;
	ssao_noise_ci.mipmaps = 0;
	ssao_noise_ci.width = noise_dim;
	ssao_noise_ci.height = noise_dim;
	ssao_noise_ci.ddscube = false;

	Texture *ssao_noise_ = graphics_wrapper->CreateTexture(ssao_noise_ci);

	TextureSubBinding ssao_noise_sub_binding_ = TextureSubBinding("ssao_noise", 4);

	TextureBindingLayoutCreateInfo tblci;
	tblci.bindingLocation = 1;
	tblci.bindings = &ssao_noise_sub_binding_;
	tblci.bindingCount = 1;
	tblci.stages = SHADER_STAGE_FRAGMENT_BIT;
	TextureBindingLayout *ssao_noise_binding_layout = graphics_wrapper->CreateTextureBindingLayout(tblci);

	SingleTextureBind stb;
	stb.texture = ssao_noise_;
	stb.address = 4;

	TextureBindingCreateInfo ci;
	ci.textures = &stb;
	ci.layout = ssao_noise_binding_layout;
	ci.textureCount = 1;
	ssao_noise_binding_ = graphics_wrapper->CreateTextureBinding(ci);

	ShaderStageCreateInfo vi;
	ShaderStageCreateInfo fi;
	std::vector<char> vfile;
	std::vector<char> ffile;

	if (settings->graphics_language_ == GRAPHICS_OPENGL) {
		vi.fileName = "../assets/shaders/lights_deferred/spotVert.glsl";
		fi.fileName = "../assets/shaders/post_processing/ssao.glsl";
	}
	else if (settings->graphics_language_ == GRAPHICS_DIRECTX) {
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
	vi.type = SHADER_VERTEX;

	ffile.clear();
	if (!readFile(fi.fileName, ffile)) {
		throw std::runtime_error("SSAO Fragment Shader missing.\n");
		return;
	}
	fi.content = ffile.data();
	fi.size = (uint32_t)ffile.size();
	fi.type = SHADER_FRAGMENT;

	ShaderStageCreateInfo stages[2] = { vi, fi };

	GraphicsPipelineCreateInfo ssaoGPCI;
	ssaoGPCI.cullMode = CULL_BACK;
	ssaoGPCI.bindings = &engine.getPlaneVBD();
	ssaoGPCI.bindingsCount = 1;
	ssaoGPCI.attributes = &engine.getPlaneVAD();
	ssaoGPCI.attributesCount = 1;
	ssaoGPCI.width = (float)settings->resolution_x_; // DIVIDE BY TWO
	ssaoGPCI.height = (float)settings->resolution_y_;
	ssaoGPCI.scissorW = settings->resolution_x_;
	ssaoGPCI.scissorH = settings->resolution_y_;
	ssaoGPCI.primitiveType = PRIM_TRIANGLES;
	ssaoGPCI.shaderStageCreateInfos = stages;
	ssaoGPCI.shaderStageCreateInfoCount = 2;
	std::vector<TextureBindingLayout *>tbls = {engine.gbuffer_tbl_};

	std::vector<UniformBufferBinding*> ubbs = { engine.deff_ubb_ , ubb };
	ssaoGPCI.textureBindings = tbls.data();
	ssaoGPCI.textureBindingCount = (uint32_t)tbls.size();
	ssaoGPCI.uniformBufferBindings = ubbs.data();
	ssaoGPCI.uniformBufferBindingCount = ubbs.size();
	pipeline_ = graphics_wrapper->CreateGraphicsPipeline(ssaoGPCI);
}

void PostProcessSSAO::Process() {
	GraphicsWrapper *graphics_wrapper = engine.getGraphicsWrapper();

	engine.getGraphicsWrapper()->BindDefaultFramebuffer(false);

    if (source_ != nullptr) {
        source_->framebuffer->BindRead();
		source_->framebuffer->BindTextures(0);
    }
    
    //graphics_wrapper->EnableDepth(false);
    //graphics_wrapper->SetColorMask(COLOR_MASK_ALPHA);
    ssao_ub->Bind();
    pipeline_->Bind();
    graphics_wrapper->BindTextureBinding(ssao_noise_binding_);
    graphics_wrapper->DrawImmediateVertices(0, 6);
    //graphics_wrapper->SetColorMask(COLOR_MASK_RGBA);
    //graphics_wrapper->EnableDepth(true);
}