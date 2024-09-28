#include <cstdio>
#include <GL/gl3w.h>

#include <Common/Logging.hpp>
#include <EngineCore/Logger.hpp>
#include <EngineCore/Utils/MemoryAllocator.hpp>

#include "GLCore.hpp"
#include "GLDescriptorSet.hpp"
#include "GLDescriptorSetLayout.hpp"
#include "GLVertexArrayObject.hpp"
#include "GLVertexBuffer.hpp"
#include "GLIndexBuffer.hpp"
#include "GLGraphicsPipeline.hpp"
#include "GLComputePipeline.hpp"
#include "GLRenderTarget.hpp"
#include "GLWindowGraphicsBinding.hpp"
#include "GLFormats.hpp"

#ifdef _WIN32
	#include <GL/wglext.h>
	#include <Common/Window/Win32Window.hpp>
#endif

using namespace Grindstone::GraphicsAPI;
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

bool GLCore::Initialize(Core::CreateInfo& ci) {
	apiType = API::OpenGL;
	debug = ci.debug;
	primaryWindow = ci.window;

	auto wgb = AllocatorCore::Allocate<GLWindowGraphicsBinding>();
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

void GLCore::ResizeViewport(uint32_t w, uint32_t h) {
	glViewport(0, 0, w, h);
}

void GLCore::Clear(ClearMode mask, float clear_color[4], float clear_depth, uint32_t clear_stencil) {
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

void GLCore::AdjustPerspective(float *perspective) {
}

void GLCore::RegisterWindow(Window* window) {
	auto wgb = AllocatorCore::Allocate<GLWindowGraphicsBinding>();
	window->AddBinding(wgb);
	wgb->Initialize(window);
	wgb->ShareLists((GLWindowGraphicsBinding *)primaryWindow->GetWindowGraphicsBinding());
}

//==================================
// Get Text Metainfo
//==================================
const char* GLCore::GetVendorName() {
	return vendorName.c_str();
}

const char* GLCore::GetAdapterName() {
	return adapterName.c_str();
}

const char* GLCore::GetAPIName() {
	return "OpenGL";
}

const char* GLCore::GetDefaultShaderExtension() {
	return ".ogl.spv";
}

const char* GLCore::GetAPIVersion() {
	return apiVersion.c_str();
}

//==================================
// Creators
//==================================
DescriptorSet* GLCore::CreateDescriptorSet(DescriptorSet::CreateInfo& createInfo) {
	return static_cast<DescriptorSet*>(AllocatorCore::Allocate<GLDescriptorSet>(createInfo));
}

DescriptorSetLayout* GLCore::CreateDescriptorSetLayout(DescriptorSetLayout::CreateInfo& createInfo) {
	return static_cast<DescriptorSetLayout*>(AllocatorCore::Allocate<GLDescriptorSetLayout>(createInfo));
}

Framebuffer* GLCore::CreateFramebuffer(Framebuffer::CreateInfo& ci) {
	return static_cast<Framebuffer*>(AllocatorCore::Allocate<GLFramebuffer>(ci));
}

RenderPass* GLCore::CreateRenderPass(RenderPass::CreateInfo& ci) {
	return 0;
}

ComputePipeline* GLCore::CreateComputePipeline(ComputePipeline::CreateInfo& ci) {
	return static_cast<ComputePipeline*>(AllocatorCore::Allocate<GLComputePipeline>(ci));
}

GraphicsPipeline* GLCore::CreateGraphicsPipeline(GraphicsPipeline::CreateInfo& ci) {
	return static_cast<GraphicsPipeline*>(AllocatorCore::Allocate<GLGraphicsPipeline>(ci));
}

VertexArrayObject* GLCore::CreateVertexArrayObject(VertexArrayObject::CreateInfo& ci) {
	return static_cast<VertexArrayObject*>(AllocatorCore::Allocate<GLVertexArrayObject>(ci));
}

CommandBuffer* GLCore::CreateCommandBuffer(CommandBuffer::CreateInfo& ci) {
	return 0;
}

VertexBuffer* GLCore::CreateVertexBuffer(VertexBuffer::CreateInfo& ci) {
	return static_cast<VertexBuffer*>(AllocatorCore::Allocate<GLVertexBuffer>(ci));
}

IndexBuffer* GLCore::CreateIndexBuffer(IndexBuffer::CreateInfo& ci) {
	return static_cast<IndexBuffer*>(AllocatorCore::Allocate<GLIndexBuffer>(ci));
}

UniformBuffer* GLCore::CreateUniformBuffer(UniformBuffer::CreateInfo& ci) {
	return static_cast<UniformBuffer*>(AllocatorCore::Allocate<GLUniformBuffer>(ci));
}

Texture* GLCore::CreateCubemap(Texture::CubemapCreateInfo& ci) {
	return static_cast<Texture*>(AllocatorCore::Allocate<GLTexture>(ci));
}

Texture* GLCore::CreateTexture(Texture::CreateInfo& ci) {
	return static_cast<Texture*>(AllocatorCore::Allocate<GLTexture>(ci));
}

RenderTarget* GLCore::CreateRenderTarget(RenderTarget::CreateInfo* rt, uint32_t rc, bool cube) {
	return static_cast<RenderTarget*>(AllocatorCore::Allocate<GLRenderTarget>(rt, rc, cube));
}

RenderTarget* GLCore::CreateRenderTarget(RenderTarget::CreateInfo& rt) {
	return static_cast<RenderTarget*>(AllocatorCore::Allocate<GLRenderTarget>(rt));
}

DepthTarget* GLCore::CreateDepthTarget(DepthTarget::CreateInfo& rt) {
	return static_cast<DepthTarget*>(AllocatorCore::Allocate<GLDepthTarget>(rt));
}

//==================================
// Booleans
//==================================
const bool GLCore::ShouldUseImmediateMode() {
	return true;
}
const bool GLCore::SupportsCommandBuffers() {
	return false;
}
const bool GLCore::SupportsTesselation() {
	return gl3wIsSupported(4, 0) ? true : false;
}
const bool GLCore::SupportsGeometryShader() {
	return gl3wIsSupported(3, 2) ? true : false;
}
const bool GLCore::SupportsComputeShader() {
	return gl3wIsSupported(4, 3) ? true : false;
}
const bool GLCore::SupportsMultiDrawIndirect() {
	return gl3wIsSupported(4, 3) ? true : false;
}

//==================================
// Deleters
//==================================
void GLCore::DeleteRenderTarget(RenderTarget *ptr) {
	AllocatorCore::Free(static_cast<GLRenderTarget *>(ptr));
}

void GLCore::DeleteDepthTarget(DepthTarget *ptr) {
	AllocatorCore::Free(static_cast<GLDepthTarget *>(ptr));
}

void GLCore::DeleteFramebuffer(Framebuffer *ptr) {
	AllocatorCore::Free(static_cast<GLFramebuffer *>(ptr));
}

void GLCore::DeleteVertexBuffer(VertexBuffer *ptr) {
	AllocatorCore::Free(static_cast<GLVertexBuffer *>(ptr));
}

void GLCore::DeleteIndexBuffer(IndexBuffer *ptr) {
	AllocatorCore::Free(static_cast<GLIndexBuffer *>(ptr));
}

void GLCore::DeleteUniformBuffer(UniformBuffer * ptr) {
	AllocatorCore::Free(static_cast<GLUniformBuffer *>(ptr));
}

void GLCore::DeleteComputePipeline(ComputePipeline* ptr) {
	AllocatorCore::Free(static_cast<GLComputePipeline*>(ptr));
}

void GLCore::DeleteGraphicsPipeline(GraphicsPipeline* ptr) {
	AllocatorCore::Free(static_cast<GLGraphicsPipeline *>(ptr));
}

void GLCore::DeleteRenderPass(RenderPass *ptr) {
	//AllocatorCore::Free(static_cast<GLRenderPass *>(ptr));
}

void GLCore::DeleteTexture(Texture *ptr) {
	AllocatorCore::Free(static_cast<GLTexture *>(ptr));
}

void GLCore::DeleteDescriptorSet(DescriptorSet *ptr) {
	AllocatorCore::Free(static_cast<GLDescriptorSet *>(ptr));
}

void GLCore::DeleteDescriptorSetLayout(DescriptorSetLayout *ptr) {
	AllocatorCore::Free(static_cast<GLDescriptorSetLayout *>(ptr));
}

void GLCore::DeleteCommandBuffer(CommandBuffer * ptr) {
}

void GLCore::DeleteVertexArrayObject(VertexArrayObject * ptr) {
	AllocatorCore::Free(static_cast<GLVertexArrayObject *>(ptr));
}

void GLCore::CopyDepthBufferFromReadToWrite(uint32_t srcWidth, uint32_t srcHeight, uint32_t dstWidth, uint32_t dstHeight) {
	glBlitFramebuffer(0, 0, srcWidth, srcHeight, 0, 0, dstWidth, dstHeight, GL_DEPTH_BUFFER_BIT, GL_NEAREST);
}

void GLCore::WaitUntilIdle() {

}

void GLCore::SetColorMask(ColorMask mask) {
	glColorMask((GLboolean)(mask & ColorMask::Red), (GLboolean)(mask & ColorMask::Green), (GLboolean)(mask & ColorMask::Blue), (GLboolean)(mask & ColorMask::Alpha));
}

void GLCore::BindGraphicsPipeline(GraphicsPipeline* pipeline) {
	GLGraphicsPipeline* p = (GLGraphicsPipeline*)pipeline;
	p->Bind();
}

void GLCore::BindVertexArrayObject(VertexArrayObject *vao) {
	vao->Bind();
}

void GLCore::DrawImmediateIndexed(GeometryType geometryType, bool largeBuffer, int32_t baseVertex, uint32_t indexOffsetPtr, uint32_t indexCount) {
	uint32_t size = largeBuffer ? sizeof(uint32_t) : sizeof(uint16_t);
	uint64_t finalPtrUint = indexOffsetPtr * size;
	void *ptr = reinterpret_cast<void *>(finalPtrUint);
	glDrawElementsBaseVertex(TranslateGeometryTypeToOpenGL(geometryType), indexCount, largeBuffer ? GL_UNSIGNED_INT : GL_UNSIGNED_SHORT, ptr, baseVertex);
}

void GLCore::DrawImmediateVertices(GeometryType geometryType, uint32_t base, uint32_t count) {
	glDrawArrays(TranslateGeometryTypeToOpenGL(geometryType), base, count);
}

void GLCore::EnableDepthWrite(bool isDepthEnabled) {
	glDepthMask(isDepthEnabled ? GL_TRUE : GL_FALSE);
}

void GLCore::BindDefaultFramebufferRead() {
	glBindFramebuffer(GL_READ_FRAMEBUFFER, 0);
}

void GLCore::BindDefaultFramebufferWrite() {
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
}

void GLCore::BindDefaultFramebuffer() {
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void GLCore::SetImmediateBlending(
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
