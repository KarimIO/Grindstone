#include <GL/gl3w.h>
#include "GLGraphicsWrapper.hpp"

#include "GLVertexArrayObject.hpp"
#include "GLVertexBuffer.hpp"
#include "GLIndexBuffer.hpp"
#include "GLGraphicsPipeline.hpp"
#include "GLRenderTarget.hpp"
#include <iostream>
#include <cstdio>

#ifdef __linux__
	#include <GL/glx.h>
#endif

void APIENTRY glDebugOutput(GLenum source,
	GLenum type,
	GLuint id,
	GLenum severity,
	GLsizei length,
	const GLchar *message,
	void *userParam)
{
	// ignore non-significant error/warning codes
	if (id == 131169 || id == 131185 || id == 131218 || id == 131204) return;

	std::cout << "--------------- \nDebug message (" << id << "): " << message << "\n";

	switch (source)
	{
		case GL_DEBUG_SOURCE_API:             std::cout << "Source: API"; break;
		case GL_DEBUG_SOURCE_WINDOW_SYSTEM:   std::cout << "Source: Window System"; break;
		case GL_DEBUG_SOURCE_SHADER_COMPILER: std::cout << "Source: Shader Compiler"; break;
		case GL_DEBUG_SOURCE_THIRD_PARTY:     std::cout << "Source: Third Party"; break;
		case GL_DEBUG_SOURCE_APPLICATION:     std::cout << "Source: Application"; break;
		case GL_DEBUG_SOURCE_OTHER:           std::cout << "Source: Other"; break;
	} std::cout << "\n";

	switch (type)
	{
		case GL_DEBUG_TYPE_ERROR:               std::cout << "Type: Error"; break;
		case GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR: std::cout << "Type: Deprecated Behaviour"; break;
		case GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR:  std::cout << "Type: Undefined Behaviour"; break;
		case GL_DEBUG_TYPE_PORTABILITY:         std::cout << "Type: Portability"; break;
		case GL_DEBUG_TYPE_PERFORMANCE:         std::cout << "Type: Performance"; break;
		case GL_DEBUG_TYPE_MARKER:              std::cout << "Type: Marker"; break;
		case GL_DEBUG_TYPE_PUSH_GROUP:          std::cout << "Type: Push Group"; break;
		case GL_DEBUG_TYPE_POP_GROUP:           std::cout << "Type: Pop Group"; break;
		case GL_DEBUG_TYPE_OTHER:               std::cout << "Type: Other"; break;
	} std::cout << "\n";

	switch (severity)
	{
		case GL_DEBUG_SEVERITY_HIGH:         std::cout << "Severity: high"; break;
		case GL_DEBUG_SEVERITY_MEDIUM:       std::cout << "Severity: medium"; break;
		case GL_DEBUG_SEVERITY_LOW:          std::cout << "Severity: low"; break;
		case GL_DEBUG_SEVERITY_NOTIFICATION: std::cout << "Severity: notification"; break;
	} std::cout << "\n\n";
}

GLGraphicsWrapper::GLGraphicsWrapper(InstanceCreateInfo createInfo) {
	debug = createInfo.debug;
	vsync = createInfo.vsync;
	width = createInfo.width;
	height = createInfo.height;
	title = createInfo.title;
	input = createInfo.inputInterface;

	std::cout << "About to initialize context...\n";
	if (!InitializeWindowContext())
		return;

	if (gl3wInit()) {
		printf("Failed to initialize GL3W. Returning...\n");
		return;
	}

	if (!gl3wIsSupported(3, 1)) {
		printf("OpenGL %i.%i or more required for Grindstone Engine.\n", 3, 1);
		printf("Your Graphics Card only supports version %s. Quitting...\n\n", glGetString(GL_VERSION));
		return;
	}

	printf("OpenGL %s initialized using GLSL %s.\n\n", glGetString(GL_VERSION), glGetString(GL_SHADING_LANGUAGE_VERSION));

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
	printf("OpenGL Initialized!\n===================================\n");
}

void GLGraphicsWrapper::Clear() {
	glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);
}

void GLGraphicsWrapper::SwapBuffer() {
#if defined(GLFW_WINDOW)
	glfwSwapBuffers(window);
#elif defined(_WIN32)
	SwapBuffers(hDC);
#else
	glXSwapBuffers(xDisplay, xWindow);
#endif
}

void GLGraphicsWrapper::CreateDefaultStructures() {

}

void GLGraphicsWrapper::Cleanup() {
#if defined(GLFW_WINDOW)
	glfwTerminate();
#elif defined(_WIN32)
	ReleaseDC(window_handle, hDC);
	wglDeleteContext(hRC);
#else
	CleanX11();
#endif
}

void GLGraphicsWrapper::DeleteFramebuffer(Framebuffer *ptr) {
	delete (GLFramebuffer *)ptr;
}

void GLGraphicsWrapper::DeleteVertexBuffer(VertexBuffer *ptr) {
	delete (GLVertexBuffer *)ptr;
}

void GLGraphicsWrapper::DeleteIndexBuffer(IndexBuffer *ptr) {
	delete (GLIndexBuffer *)ptr;
}

void GLGraphicsWrapper::DeleteUniformBuffer(UniformBuffer * ptr) {
	delete (GLUniformBuffer *)ptr;
}

void GLGraphicsWrapper::DeleteUniformBufferBinding(UniformBufferBinding * ptr) {
	delete (GLUniformBufferBinding *)ptr;
}

void GLGraphicsWrapper::DeleteGraphicsPipeline(GraphicsPipeline *ptr) {
	delete (GLGraphicsPipeline *)ptr;
}

void GLGraphicsWrapper::DeleteRenderPass(RenderPass *ptr) {
	//delete (GLRenderPass *)ptr;
}

void GLGraphicsWrapper::DeleteTexture(Texture *ptr) {
	delete (GLTexture *)ptr;
}

TextureBinding * GLGraphicsWrapper::CreateTextureBinding(TextureBindingCreateInfo createInfo) {
	return static_cast<TextureBinding *>(new GLTextureBinding(createInfo));
}

TextureBindingLayout * GLGraphicsWrapper::CreateTextureBindingLayout(TextureBindingLayoutCreateInfo createInfo) {
	return static_cast<TextureBindingLayout *>(new GLTextureBindingLayout(createInfo));
}

void GLGraphicsWrapper::DeleteCommandBuffer(CommandBuffer * ptr) {
}

void GLGraphicsWrapper::DeleteVertexArrayObject(VertexArrayObject * ptr) {
	delete (GLVertexArrayObject *)ptr;
}


Framebuffer * GLGraphicsWrapper::CreateFramebuffer(FramebufferCreateInfo ci) {
	return static_cast<Framebuffer *>(new GLFramebuffer(ci));
}

RenderPass *GLGraphicsWrapper::CreateRenderPass(RenderPassCreateInfo ci) {
	return 0;
}

GraphicsPipeline *GLGraphicsWrapper::CreateGraphicsPipeline(GraphicsPipelineCreateInfo ci) {
	return static_cast<GraphicsPipeline *>(new GLGraphicsPipeline(ci));
}

void GLGraphicsWrapper::CreateDefaultFramebuffers(DefaultFramebufferCreateInfo ci, Framebuffer **&framebuffers, uint32_t &framebufferCount) {
	framebuffers = nullptr;
	framebufferCount = 0;
}

VertexArrayObject *GLGraphicsWrapper::CreateVertexArrayObject(VertexArrayObjectCreateInfo ci) {
	return static_cast<VertexArrayObject *>(new GLVertexArrayObject(ci));
}

CommandBuffer *GLGraphicsWrapper::CreateCommandBuffer(CommandBufferCreateInfo ci) {
	return 0;
}

VertexBuffer *GLGraphicsWrapper::CreateVertexBuffer(VertexBufferCreateInfo ci) {
	return static_cast<VertexBuffer *>(new GLVertexBuffer(ci));
}

IndexBuffer *GLGraphicsWrapper::CreateIndexBuffer(IndexBufferCreateInfo ci) {
	return static_cast<IndexBuffer *>(new GLIndexBuffer(ci));
}

UniformBuffer *GLGraphicsWrapper::CreateUniformBuffer(UniformBufferCreateInfo ci) {
	return static_cast<UniformBuffer *>(new GLUniformBuffer(ci));
}

UniformBufferBinding *GLGraphicsWrapper::CreateUniformBufferBinding(UniformBufferBindingCreateInfo ci) {
	return static_cast<UniformBufferBinding *>(new GLUniformBufferBinding(ci));
}

Texture * GLGraphicsWrapper::CreateCubemap(CubemapCreateInfo ci) {
	return static_cast<Texture *>(new GLTexture(ci));
}

Texture *GLGraphicsWrapper::CreateTexture(TextureCreateInfo ci) {
	return static_cast<Texture *>(new GLTexture(ci));
}

RenderTarget *GLGraphicsWrapper::CreateRenderTarget(RenderTargetCreateInfo *rt, uint32_t rc) {
	return static_cast<RenderTarget *>(new GLRenderTarget(rt, rc));
}

DepthTarget * GLGraphicsWrapper::CreateDepthTarget(DepthTargetCreateInfo * rt, uint32_t rc) {
	return static_cast<DepthTarget *>(new GLDepthTarget(rt, rc));
}


uint32_t GLGraphicsWrapper::GetImageIndex() {
	return 0;
}

bool GLGraphicsWrapper::SupportsCommandBuffers() {
	return false;
}

bool GLGraphicsWrapper::SupportsTesselation() {
	return gl3wIsSupported(4, 0) ? true : false;
}

bool GLGraphicsWrapper::SupportsGeometryShader() {
	return gl3wIsSupported(3, 2) ? true : false;
}

bool GLGraphicsWrapper::SupportsComputeShader() {
	return gl3wIsSupported(4, 3) ? true : false;
}

bool GLGraphicsWrapper::SupportsMultiDrawIndirect() {
	return gl3wIsSupported(4, 3) ? true : false;
}

void GLGraphicsWrapper::WaitUntilIdle() {

}

void GLGraphicsWrapper::DrawCommandBuffers(uint32_t imageIndex, CommandBuffer ** commandBuffers, uint32_t commandBufferCount) {

}

void GLGraphicsWrapper::BindTextureBinding(TextureBinding *binding) {
	GLTextureBinding *b = (GLTextureBinding *)binding;
	b->Bind();
}

void GLGraphicsWrapper::BindVertexArrayObject(VertexArrayObject *vao) {
	vao->Bind();
}

void GLGraphicsWrapper::DrawImmediateIndexed(bool largeBuffer, int32_t baseVertex, uint32_t indexOffsetPtr, uint32_t indexCount) {
	uint32_t size = largeBuffer ? sizeof(uint32_t) : sizeof(uint16_t);
	void *ptr = reinterpret_cast<void *>(indexOffsetPtr * size);
	glDrawElementsBaseVertex(GL_TRIANGLES, indexCount, largeBuffer ? GL_UNSIGNED_INT : GL_UNSIGNED_SHORT, ptr, baseVertex);
}

void GLGraphicsWrapper::DrawImmediateVertices(uint32_t base, uint32_t count) {
	glDrawArrays(GL_TRIANGLE_STRIP, base, count);
}

void GLGraphicsWrapper::EnableDepth(bool state) {
	if (state) {
		glEnable(GL_DEPTH_TEST);
	}
	else {
		glDisable(GL_DEPTH_TEST);
	}
}

void GLGraphicsWrapper::SetImmediateBlending(BlendMode mode) {
	switch (mode) {
	case BLEND_NONE:
	default:
		glDisable(GL_BLEND);
		break;
	case BLEND_ADDITIVE:
		glEnable(GL_BLEND);
		glBlendEquation(GL_FUNC_ADD);
		glBlendFunc(GL_ONE, GL_ONE);
		break;
	case BLEND_ADD_ALPHA:
		glEnable(GL_BLEND);
		glBlendEquation(GL_FUNC_ADD);
		glBlendFuncSeparate(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA, GL_ONE, GL_ONE_MINUS_SRC_ALPHA);
		break;
	}
}

void GLGraphicsWrapper::BindDefaultFramebuffer()
{
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

ImageFormat GLGraphicsWrapper::GetDeviceColorFormat() {
	return FORMAT_COLOR_R8G8B8A8;
}

GRAPHICS_EXPORT GraphicsWrapper* createGraphics(InstanceCreateInfo createInfo) {
	return new GLGraphicsWrapper(createInfo);
}

GRAPHICS_EXPORT void deleteGraphics(void * ptr) {
	free(ptr);
}