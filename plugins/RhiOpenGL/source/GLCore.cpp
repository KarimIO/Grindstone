#include <cstdio>
#include <GL/gl3w.h>

#include <Common/Logging.hpp>
#include <EngineCore/Logger.hpp>
#include <EngineCore/Utils/MemoryAllocator.hpp>

#include <RhiOpenGL/include/GLCore.hpp>
#include <RhiOpenGL/include/GLFramebuffer.hpp>
#include <RhiOpenGL/include/GLDescriptorSet.hpp>
#include <RhiOpenGL/include/GLDescriptorSetLayout.hpp>
#include <RhiOpenGL/include/GLVertexArrayObject.hpp>
#include <RhiOpenGL/include/GLBuffer.hpp>
#include <RhiOpenGL/include/GLGraphicsPipeline.hpp>
#include <RhiOpenGL/include/GLComputePipeline.hpp>
#include <RhiOpenGL/include/GLSampler.hpp>
#include <RhiOpenGL/include/GLImage.hpp>
#include <RhiOpenGL/include/GLWindowGraphicsBinding.hpp>
#include <RhiOpenGL/include/GLFormats.hpp>

#ifdef _WIN32
	#include <GL/wglext.h>
	#include <Common/Window/Win32Window.hpp>
#endif

namespace Base = Grindstone::GraphicsAPI;
namespace OpenGL = Grindstone::GraphicsAPI::OpenGL;
using namespace Grindstone::Memory;

static void APIENTRY glDebugOutput(
	GLenum source,
	GLenum type,
	GLuint id,
	GLenum severity,
	GLsizei length,
	const GLchar *message,
	void *userParam
) {
	// ignore non-significant error/warning codes
	if (id == 131169 || id == 131185 || id == 131218 || id == 131204) return;

	const char* sourceStr = nullptr;
	switch (source) {
	case GL_DEBUG_SOURCE_API:             sourceStr = "[SOURCE: API]"; break;
	case GL_DEBUG_SOURCE_WINDOW_SYSTEM:   sourceStr = "[SOURCE: Window System]"; break;
	case GL_DEBUG_SOURCE_SHADER_COMPILER: sourceStr = "[SOURCE: Shader Compiler]"; break;
	case GL_DEBUG_SOURCE_THIRD_PARTY:     sourceStr = "[SOURCE: Third Party]"; break;
	case GL_DEBUG_SOURCE_APPLICATION:     sourceStr = "[SOURCE: Application]"; break;
	default:
	case GL_DEBUG_SOURCE_OTHER:           sourceStr = "[SOURCE: Other]"; break;
	}

	const char* typeStr = nullptr;
	switch (type) {
	case GL_DEBUG_TYPE_ERROR:               typeStr = "[TYPE: Error]"; break;
	case GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR: typeStr = "[TYPE: Deprecated Behaviour]"; break;
	case GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR:  typeStr = "[TYPE: Undefined Behaviour]"; break;
	case GL_DEBUG_TYPE_PORTABILITY:         typeStr = "[TYPE: Portability]"; break;
	case GL_DEBUG_TYPE_PERFORMANCE:         typeStr = "[TYPE: Performance]"; break;
	case GL_DEBUG_TYPE_MARKER:              typeStr = "[TYPE: Marker]"; break;
	case GL_DEBUG_TYPE_PUSH_GROUP:          typeStr = "[TYPE: Push Group]"; break;
	case GL_DEBUG_TYPE_POP_GROUP:           typeStr = "[TYPE: Pop Group]"; break;
	default:
	case GL_DEBUG_TYPE_OTHER:               typeStr = "[TYPE: Other]"; break;
	}

	Grindstone::LogSeverity logSeverity = Grindstone::LogSeverity::Info;
	switch (severity) {
	case GL_DEBUG_SEVERITY_HIGH:         logSeverity = Grindstone::LogSeverity::Error; break;
	case GL_DEBUG_SEVERITY_MEDIUM:       logSeverity = Grindstone::LogSeverity::Warning; break;
	case GL_DEBUG_SEVERITY_LOW:          logSeverity = Grindstone::LogSeverity::Info; break;
	case GL_DEBUG_SEVERITY_NOTIFICATION: logSeverity = Grindstone::LogSeverity::Trace; break;
	}

	std::string formattedMessage = std::string(sourceStr) + typeStr + message;
	GPRINT_TYPED(logSeverity, Grindstone::LogSource::GraphicsAPI, id, formattedMessage.c_str());
}

bool OpenGL::Core::Initialize(const Core::CreateInfo& ci) {
	apiType = Base::API::OpenGL;
	debug = ci.debug;
	primaryWindow = ci.window;

	auto wgb = AllocatorCore::Allocate<OpenGL::WindowGraphicsBinding>();
	ci.window->AddBinding(wgb);
	wgb->Initialize(ci.window);

	if (gl3wInit()) {
		printf("Failed to initialize GL3W. Returning...\n");
		return false;
	}

	if (!gl3wIsSupported(4, 6)) {
		printf("OpenGL %i.%i or more required for Grindstone Engine.\n", 4, 6);
		printf("Your Graphics Card only supports version %s. Quitting...\n\n", glGetString(GL_VERSION));
		return false;
	}

	vendorName		= (const char *)glGetString(GL_VENDOR);
	adapterName		= (const char *)glGetString(GL_RENDERER);
	apiVersion		= (const char *)glGetString(GL_VERSION);

	if (debug) {
		GLint flags;
		glGetIntegerv(GL_CONTEXT_FLAGS, &flags);
		if (flags & GL_CONTEXT_FLAG_DEBUG_BIT) {
			glEnable(GL_DEBUG_OUTPUT);
			glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
			glDebugMessageCallback((GLDEBUGPROC)glDebugOutput, nullptr);
			glDebugMessageControl(GL_DONT_CARE, GL_DONT_CARE, GL_DONT_CARE, 0, nullptr, GL_TRUE);
		}
	}

	glDepthMask(GL_TRUE);
	glClearDepth(1.0f);
	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LEQUAL);

	unsigned int w, h;
	primaryWindow->GetWindowSize(w, h);
	glViewport(0, 0, w, h);

	return true;
}

void OpenGL::Core::ResizeViewport(uint32_t w, uint32_t h) {
	glViewport(0, 0, w, h);
}

void OpenGL::Core::Clear(ClearMode mask, float clear_color[4], float clear_depth, uint32_t clear_stencil) {
	if (clear_color == nullptr) {
		glClearColor(0, 0, 0, 1);
	}
	else {
		glClearColor(clear_color[0], clear_color[1], clear_color[2], clear_color[3]);
	}

	glClearDepthf(clear_depth);
	glClearStencil(clear_stencil);

	int m = (((uint8_t)mask & (uint8_t)ClearMode::Color) != 0) ? GL_COLOR_BUFFER_BIT : 0;
	m = m | ((((uint8_t)mask & (uint8_t)ClearMode::Depth) != 0) ? GL_DEPTH_BUFFER_BIT : 0);
	m = m | ((((uint8_t)mask & (uint8_t)ClearMode::Stencil) != 0) ? GL_STENCIL_BUFFER_BIT : 0);
	glClear(m);
}

void OpenGL::Core::AdjustPerspective(float *perspective) {
}

void OpenGL::Core::RegisterWindow(Window* window) {
	auto wgb = AllocatorCore::Allocate<OpenGL::WindowGraphicsBinding>();
	window->AddBinding(wgb);
	wgb->Initialize(window);
	wgb->ShareLists(static_cast<OpenGL::WindowGraphicsBinding*>(primaryWindow->GetWindowGraphicsBinding()));
}

//==================================
// Get Text Metainfo
//==================================
const char* OpenGL::Core::GetVendorName() const {
	return vendorName.c_str();
}

const char* OpenGL::Core::GetAdapterName() const {
	return adapterName.c_str();
}

const char* OpenGL::Core::GetAPIName() const {
	return "OpenGL";
}

const char* OpenGL::Core::GetDefaultShaderExtension() const {
	return ".ogl.spv";
}

const char* OpenGL::Core::GetAPIVersion() const {
	return apiVersion.c_str();
}

//==================================
// Creators
//==================================
Base::DescriptorSet* OpenGL::Core::CreateDescriptorSet(const Base::DescriptorSet::CreateInfo& createInfo) {
	return static_cast<DescriptorSet*>(AllocatorCore::Allocate<OpenGL::DescriptorSet>(createInfo));
}

Base::DescriptorSetLayout* OpenGL::Core::CreateDescriptorSetLayout(const Base::DescriptorSetLayout::CreateInfo& createInfo) {
	return static_cast<DescriptorSetLayout*>(AllocatorCore::Allocate<OpenGL::DescriptorSetLayout>(createInfo));
}

Base::Framebuffer* OpenGL::Core::CreateFramebuffer(const Base::Framebuffer::CreateInfo& ci) {
	return static_cast<Framebuffer*>(AllocatorCore::Allocate<OpenGL::Framebuffer>(ci));
}

Base::RenderPass* OpenGL::Core::CreateRenderPass(const Base::RenderPass::CreateInfo& ci) {
	return 0;
}

Base::ComputePipeline* OpenGL::Core::CreateComputePipeline(const Base::ComputePipeline::CreateInfo& ci) {
	return static_cast<ComputePipeline*>(AllocatorCore::Allocate<OpenGL::ComputePipeline>(ci));
}

Base::GraphicsPipeline* OpenGL::Core::CreateGraphicsPipeline(const Base::GraphicsPipeline::CreateInfo& ci) {
	return static_cast<GraphicsPipeline*>(AllocatorCore::Allocate<OpenGL::GraphicsPipeline>(ci));
}

Base::VertexArrayObject* OpenGL::Core::CreateVertexArrayObject(const Base::VertexArrayObject::CreateInfo& ci) {
	return static_cast<VertexArrayObject*>(AllocatorCore::Allocate<OpenGL::VertexArrayObject>(ci));
}

Base::CommandBuffer* OpenGL::Core::CreateCommandBuffer(const Base::CommandBuffer::CreateInfo& ci) {
	return 0;
}

Base::Buffer* OpenGL::Core::CreateBuffer(const Base::Buffer::CreateInfo& ci) {
	return static_cast<Buffer*>(AllocatorCore::Allocate<OpenGL::Buffer>(ci));
}

Base::Image* OpenGL::Core::CreateImage(const Base::Image::CreateInfo& ci) {
	return static_cast<Image*>(AllocatorCore::Allocate<OpenGL::Image>(ci));
}

Base::Sampler* OpenGL::Core::CreateSampler(const Base::Sampler::CreateInfo& rt) {
	return static_cast<Sampler*>(AllocatorCore::Allocate<OpenGL::Sampler>(rt));
}

Base::GraphicsPipeline* OpenGL::Core::GetOrCreateGraphicsPipelineFromCache(const GraphicsPipeline::PipelineData& pipelineData, const VertexInputLayout* vertexInputLayout) {
	size_t hash = std::hash<GraphicsPipeline::PipelineData>{}(pipelineData);
	auto iterator = graphicsPipelineCache.find(hash);
	if (iterator != graphicsPipelineCache.end()) {
		return iterator->second;
	}

	Grindstone::GraphicsAPI::GraphicsPipeline::CreateInfo createInfo{};
	createInfo.pipelineData = pipelineData;
	Grindstone::GraphicsAPI::GraphicsPipeline* newPipeline = CreateGraphicsPipeline(createInfo);

	graphicsPipelineCache[hash] = newPipeline;
	return newPipeline;
}

//==================================
// Booleans
//==================================
bool OpenGL::Core::ShouldUseImmediateMode() const {
	return true;
}

bool OpenGL::Core::SupportsCommandBuffers() const {
	return false;
}

bool OpenGL::Core::SupportsTesselation() const {
	return gl3wIsSupported(4, 0) ? true : false;
}

bool OpenGL::Core::SupportsGeometryShader() const {
	return gl3wIsSupported(3, 2) ? true : false;
}

bool OpenGL::Core::SupportsComputeShader() const {
	return gl3wIsSupported(4, 3) ? true : false;
}

bool OpenGL::Core::SupportsMultiDrawIndirect() const {
	return gl3wIsSupported(4, 3) ? true : false;
}

//==================================
// Deleters
//==================================
void OpenGL::Core::DeleteFramebuffer(Base::Framebuffer *ptr) {
	AllocatorCore::Free(static_cast<OpenGL::Framebuffer*>(ptr));
}

void OpenGL::Core::DeleteBuffer(Base::Buffer *ptr) {
	AllocatorCore::Free(static_cast<OpenGL::Buffer*>(ptr));
}

void OpenGL::Core::DeleteComputePipeline(Base::ComputePipeline* ptr) {
	AllocatorCore::Free(static_cast<OpenGL::ComputePipeline*>(ptr));
}

void OpenGL::Core::DeleteGraphicsPipeline(Base::GraphicsPipeline* ptr) {
	AllocatorCore::Free(static_cast<OpenGL::GraphicsPipeline*>(ptr));
}

void OpenGL::Core::DeleteRenderPass(Base::RenderPass *ptr) {
	//AllocatorCore::Free(static_cast<OpenGL::RenderPass*>(ptr));
}

void OpenGL::Core::DeleteSampler(Base::Sampler* ptr) {
	AllocatorCore::Free(static_cast<OpenGL::Sampler*>(ptr));
}

void OpenGL::Core::DeleteImage(Base::Image* ptr) {
	AllocatorCore::Free(static_cast<OpenGL::Image*>(ptr));
}

void OpenGL::Core::DeleteDescriptorSet(Base::DescriptorSet* ptr) {
	AllocatorCore::Free(static_cast<OpenGL::DescriptorSet*>(ptr));
}

void OpenGL::Core::DeleteDescriptorSetLayout(Base::DescriptorSetLayout* ptr) {
	AllocatorCore::Free(static_cast<OpenGL::DescriptorSetLayout*>(ptr));
}

void OpenGL::Core::DeleteCommandBuffer(Base::CommandBuffer* ptr) {
}

void OpenGL::Core::DeleteVertexArrayObject(Base::VertexArrayObject* ptr) {
	AllocatorCore::Free(static_cast<OpenGL::VertexArrayObject*>(ptr));
}

void OpenGL::Core::CopyDepthBufferFromReadToWrite(uint32_t srcWidth, uint32_t srcHeight, uint32_t dstWidth, uint32_t dstHeight) {
	glBlitFramebuffer(0, 0, srcWidth, srcHeight, 0, 0, dstWidth, dstHeight, GL_DEPTH_BUFFER_BIT, GL_NEAREST);
}

void OpenGL::Core::WaitUntilIdle() {

}

void OpenGL::Core::SetColorMask(ColorMask mask) {
	glColorMask((GLboolean)(mask & ColorMask::Red), (GLboolean)(mask & ColorMask::Green), (GLboolean)(mask & ColorMask::Blue), (GLboolean)(mask & ColorMask::Alpha));
}

void OpenGL::Core::BindGraphicsPipeline(Base::GraphicsPipeline* pipeline) {
	OpenGL::GraphicsPipeline* p = static_cast<OpenGL::GraphicsPipeline*>(pipeline);
	p->Bind();
}

void OpenGL::Core::BindVertexArrayObject(Base::VertexArrayObject *vao) {
	vao->Bind();
}

void OpenGL::Core::DrawImmediateIndexed(GeometryType geometryType, bool largeBuffer, int32_t baseVertex, uint32_t indexOffsetPtr, uint32_t indexCount) {
	uint64_t size = largeBuffer ? sizeof(uint32_t) : sizeof(uint16_t);
	uint64_t finalPtrUint = static_cast<uint64_t>(indexOffsetPtr) * size;
	void *ptr = reinterpret_cast<void*>(finalPtrUint);
	glDrawElementsBaseVertex(TranslateGeometryTypeToOpenGL(geometryType), indexCount, largeBuffer ? GL_UNSIGNED_INT : GL_UNSIGNED_SHORT, ptr, baseVertex);
}

void OpenGL::Core::DrawImmediateVertices(GeometryType geometryType, uint32_t base, uint32_t count) {
	glDrawArrays(TranslateGeometryTypeToOpenGL(geometryType), base, count);
}

void OpenGL::Core::EnableDepthWrite(bool isDepthEnabled) {
	glDepthMask(isDepthEnabled ? GL_TRUE : GL_FALSE);
}

void OpenGL::Core::BindDefaultFramebufferRead() {
	glBindFramebuffer(GL_READ_FRAMEBUFFER, 0);
}

void OpenGL::Core::BindDefaultFramebufferWrite() {
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
}

void OpenGL::Core::BindDefaultFramebuffer() {
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void OpenGL::Core::SetImmediateBlending(
	BlendOperation colorOp, BlendFactor colorSrc, BlendFactor colorDst,
	BlendOperation alphaOp, BlendFactor alphaSrc, BlendFactor alphaDst
) {
	if (colorOp == BlendOperation::None && alphaOp == BlendOperation::None) {
		glDisable(GL_BLEND);
		return;
	}

	glEnable(GL_BLEND);
	glBlendEquationSeparate(GL_FUNC_ADD, GL_FUNC_ADD);
	glBlendFuncSeparate(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA, GL_ONE, GL_ONE_MINUS_SRC_ALPHA);
}
