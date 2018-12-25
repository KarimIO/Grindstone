#include "PostProcessIBL.hpp"
#include "../Core/Engine.hpp"
#include "Core/Utilities.hpp"
#include <GraphicsWrapper.hpp>
#include "Systems/CubemapSystem.hpp"
#include "PostPipeline.hpp"
#include "Core/Space.hpp"

PostProcessIBL::PostProcessIBL(PostPipeline *pipeline, RenderTargetContainer *target) : BasePostProcess(pipeline), target_(target) {
    auto graphics_wrapper = engine.getGraphicsWrapper();
	auto settings = engine.getSettings();

    ShaderStageCreateInfo vi;
	ShaderStageCreateInfo fi;
	if (settings->graphics_language_ == GRAPHICS_OPENGL) {
		vi.fileName = "../assets/shaders/lights_deferred/spotVert.glsl";
		fi.fileName = "../assets/shaders/lights_deferred/ibl.glsl";
	}
	else if (settings->graphics_language_ == GRAPHICS_DIRECTX) {
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
	
	subbinding_ = TextureSubBinding("environmentMap", 4);
	TextureBindingLayoutCreateInfo tblci;
	tblci.bindingLocation = 4;
	tblci.bindings = &subbinding_;
	tblci.bindingCount = (uint32_t)1;
	tblci.stages = SHADER_STAGE_FRAGMENT_BIT;
	env_map_ = graphics_wrapper->CreateTextureBindingLayout(tblci);
	
	GraphicsPipelineCreateInfo iblGPCI;
	iblGPCI.cullMode = CULL_BACK;
	iblGPCI.bindings = &engine.getPlaneVBD();
	iblGPCI.bindingsCount = 1;
	iblGPCI.attributes = &engine.getPlaneVAD();
	iblGPCI.attributesCount = 1;
	iblGPCI.width = (float)settings->resolution_x_;
	iblGPCI.height = (float)settings->resolution_y_;
	iblGPCI.scissorW = settings->resolution_x_;
	iblGPCI.scissorH = settings->resolution_y_;
	iblGPCI.primitiveType = PRIM_TRIANGLES;
	iblGPCI.shaderStageCreateInfos = stages.data();
	iblGPCI.shaderStageCreateInfoCount = (uint32_t)stages.size();
	std::vector<TextureBindingLayout*> tbls_refl = { engine.gbuffer_tbl_, env_map_ }; // refl_tbl

	iblGPCI.textureBindings = tbls_refl.data();
	iblGPCI.textureBindingCount = tbls_refl.size();
	iblGPCI.uniformBufferBindings = &engine.deff_ubb_;
	iblGPCI.uniformBufferBindingCount = 1;
	gpipeline_ = graphics_wrapper->CreateGraphicsPipeline(iblGPCI);
}

void PostProcessIBL::Process() {
	auto graphics_wrapper = engine.getGraphicsWrapper();
	
	graphics_wrapper->BindVertexArrayObject(engine.getPlaneVAO());

	graphics_wrapper->SetImmediateBlending(BLEND_ADDITIVE);

	//graphics_wrapper->BindDefaultFramebuffer(true);
	//engine.getGraphicsWrapper()->Clear(CLEAR_BOTH);
    target_->framebuffer->BindWrite(false);
    //source_->framebuffer->BindRead();
	//target_->framebuffer->BindTextures(0);
	engine.deff_ubo_handler_->Bind();

	glm::vec3 pos = glm::vec3(0, 0, 0); // engine.deffUBOBuffer.eyePos;
    CubemapComponent *cube = ((CubemapSubSystem *)getPipeline()->getSpace()->getSubsystem(COMPONENT_CUBEMAP))->getClosestCubemap(pos);
    if (cube && cube->cubemap_) {
		graphics_wrapper->BindTextureBinding(cube->cubemap_binding_);
    }

	gpipeline_->Bind();
	graphics_wrapper->DrawImmediateVertices(0, 6);
	graphics_wrapper->SetImmediateBlending(BLEND_NONE);
}