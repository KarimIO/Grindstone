#include "PostProcessIBL.hpp"
#include "../Core/Engine.hpp"
/*
PostProcessIBL::PostProcessIBL(RenderTargetContainer *source, RenderTargetContainer *target) : source_(source), target_(target) {
    engine.graphics_wrapper_ = engine.engine.graphics_wrapper_;
    
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
	TextureBindingLayout *refl_tbl = engine.reflection_cubemap_layout_;
	TextureBindingLayout * tbls_refl[2] = { engine.tbl, refl_tbl };

	iblGPCI.textureBindings = tbls_refl;
	iblGPCI.textureBindingCount = 2;
	iblGPCI.uniformBufferBindings = &engine.deffubb;
	iblGPCI.uniformBufferBindingCount = 1;
	pipeline_ = engine.graphics_wrapper_->CreateGraphicsPipeline(iblGPCI);
}

void PostProcessIBL::Process() {
	engine.graphics_wrapper_->SetImmediateBlending(BLEND_ADDITIVE);

    target_->framebuffer->BindWrite(false);
    source_->framebuffer->BindRead();
    source_->framebuffer->BindTextures(0);

    glm::vec3 pos = engine.deffUBOBuffer.eyePos;
    CubemapComponent *cube = engine.cubemapSystem.GetClosestCubemap(pos);
    pipeline_->Bind();
    if (cube && cube->cubemap) {
        engine.graphics_wrapper_->BindTextureBinding(cube->cubemap_binding);
    }
    engine.graphics_wrapper_->DrawImmediateVertices(0, 6);
	engine.graphics_wrapper_->SetImmediateBlending(BLEND_NONE);
}*/