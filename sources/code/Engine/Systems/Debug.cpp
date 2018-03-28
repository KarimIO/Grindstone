#include "Debug.hpp"
#include "../Core/Engine.hpp"

const unsigned char DEBUG_MODE_MAX = 7;

Debug::Debug() : framebuffer_(nullptr), debug_mode_(0) {}

void Debug::Initialize(Framebuffer *framebuffer) {
    framebuffer_ = framebuffer;

	engine.inputSystem.AddControl("1", "SwitchDebug", NULL, 1);
	engine.inputSystem.BindAction("SwitchDebug", NULL, this, &Debug::SwitchDebug);

    UniformBufferBindingCreateInfo ubbci;
	ubbci.binding = 2;
	ubbci.shaderLocation = "DebugBufferObject";
	ubbci.size = sizeof(unsigned int);
	ubbci.stages = SHADER_STAGE_FRAGMENT_BIT;
	UniformBufferBinding *ubb = engine.graphics_wrapper_->CreateUniformBufferBinding(ubbci);

	UniformBufferCreateInfo ubci;
	ubci.isDynamic = true;
	ubci.size = sizeof(unsigned int);
	ubci.binding = ubb;
	ubo = engine.graphics_wrapper_->CreateUniformBuffer(ubci);
    ubo->UpdateUniformBuffer(&debug_mode_);

	ShaderStageCreateInfo stages[2];
	if (engine.settings.graphicsLanguage == GRAPHICS_OPENGL) {
		stages[0].fileName = "../assets/shaders/lights_deferred/spotVert.glsl";
		stages[1].fileName = "../assets/shaders/post_processing/debug.glsl";
	}
	else if (engine.settings.graphicsLanguage == GRAPHICS_DIRECTX) {
		stages[0].fileName = "../assets/shaders/lights_deferred/pointVert.fxc";
		stages[1].fileName = "../assets/shaders/post_processing/debug.fxc";
	}
	else {
		stages[0].fileName = "../assets/shaders/lights_deferred/spotVert.spv";
	    stages[1].fileName = "../assets/shaders/post_processing/debug.spv";
	}

	std::vector<char> vfile;
	if (!readFile(stages[0].fileName, vfile)) {
		throw std::runtime_error("Debug Vertex Shader missing.\n");
	}
	stages[0].content = vfile.data();
	stages[0].size = (uint32_t)vfile.size();
	stages[0].type = SHADER_VERTEX;

	std::vector<char> ffile;
	if (!readFile(stages[1].fileName, ffile)) {
		throw std::runtime_error("Debug Fragment Shader missing.\n");
	}
	stages[1].content = ffile.data();
	stages[1].size = (uint32_t)ffile.size();
	stages[1].type = SHADER_FRAGMENT;

	GraphicsPipelineCreateInfo gpci;
	gpci.cullMode = CULL_BACK;
	gpci.bindings = &engine.planeVBD;
	gpci.bindingsCount = 1;
	gpci.attributes = &engine.planeVAD;
	gpci.attributesCount = 1;
	gpci.width = (float)engine.settings.resolutionX;
	gpci.height = (float)engine.settings.resolutionY;
	gpci.scissorW = engine.settings.resolutionX;
	gpci.scissorH = engine.settings.resolutionY;
	gpci.primitiveType = PRIM_TRIANGLES;
	gpci.shaderStageCreateInfos = stages;
	gpci.shaderStageCreateInfoCount = 2;

	gpci.textureBindings = &engine.tbl;
	gpci.textureBindingCount = 1;
	gpci.uniformBufferBindings = &ubb;
	gpci.uniformBufferBindingCount = 1;
	pipeline_ = engine.graphics_wrapper_->CreateGraphicsPipeline(gpci);
}

const unsigned int Debug::GetDebugMode() {
    return debug_mode_;
}

void Debug::SwitchDebug(double p) {
    debug_mode_ = ((debug_mode_ + 1) % DEBUG_MODE_MAX);
    ubo->UpdateUniformBuffer(&debug_mode_);

    std::cout << "Debug Mode: ";
    std::string debug_str;
    switch (debug_mode_) {
    default:
        debug_str = "No Debug";
        break;
    case 1:
        debug_str = "Position";
        break;
    case 2:
        debug_str = "Normal";
        break;
    case 3:
        debug_str = "Albedo";
        break;
    case 4:
        debug_str = "Specular";
        break;
    case 5:
        debug_str = "Roughness";
        break;
    case 6:
        debug_str = "Distance";
        break;
    }
    std::cout << debug_str << std::endl;
}

void Debug::Draw() {
	engine.deffUBO->Bind();
	engine.graphics_wrapper_->BindVertexArrayObject(engine.planeVAO);

    pipeline_->Bind();
	ubo->Bind();

	engine.graphics_wrapper_->SetImmediateBlending(BLEND_NONE);
	engine.graphics_wrapper_->EnableDepth(false);
	engine.graphics_wrapper_->BindDefaultFramebuffer(false);
	engine.graphics_wrapper_->Clear(CLEAR_BOTH);
	framebuffer_->BindRead();
	framebuffer_->BindTextures(0);
	engine.graphics_wrapper_->DrawImmediateVertices(0, 6);
}

Debug::~Debug() {

}