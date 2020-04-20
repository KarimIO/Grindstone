#include <GL/gl3w.h>
#include "GLGraphicsWrapper.hpp"

#include "GLVertexArrayObject.hpp"
#include "GLVertexBuffer.hpp"
#include "GLIndexBuffer.hpp"
#include "GLGraphicsPipeline.hpp"
#include "GLRenderTarget.hpp"
#include <iostream>
#include <cstdio>

#ifdef _WIN32
	//#include <windows.h>
	//#include <Windowsx.h>
	#include <GL/wglext.h>
	#include "../WindowModule/Win32Window.hpp"
#endif

#ifdef __linux__
	#include <GL/glx.h>
#endif

void APIENTRY glDebugOutput(GLenum source,
	GLenum type,
	GLuint id,
	GLenum severity,
	GLsizei length,
	const GLchar *message,
	void *userParam) {
	// ignore non-significant error/warning codes
	if (id == 131169 || id == 131185 || id == 131218 || id == 131204) return;

	if (severity == GL_DEBUG_SEVERITY_NOTIFICATION) return;

	std::cout << "--------------- \nDebug message (" << id << "): " << message << "\n";

	switch (source) {
	case GL_DEBUG_SOURCE_API:             std::cout << "Source: API"; break;
	case GL_DEBUG_SOURCE_WINDOW_SYSTEM:   std::cout << "Source: Window System"; break;
	case GL_DEBUG_SOURCE_SHADER_COMPILER: std::cout << "Source: Shader Compiler"; break;
	case GL_DEBUG_SOURCE_THIRD_PARTY:     std::cout << "Source: Third Party"; break;
	case GL_DEBUG_SOURCE_APPLICATION:     std::cout << "Source: Application"; break;
	case GL_DEBUG_SOURCE_OTHER:           std::cout << "Source: Other"; break;
	}
	std::cout << "\n";

	switch (type) {
	case GL_DEBUG_TYPE_ERROR:               std::cout << "Type: Error"; break;
	case GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR: std::cout << "Type: Deprecated Behaviour"; break;
	case GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR:  std::cout << "Type: Undefined Behaviour"; break;
	case GL_DEBUG_TYPE_PORTABILITY:         std::cout << "Type: Portability"; break;
	case GL_DEBUG_TYPE_PERFORMANCE:         std::cout << "Type: Performance"; break;
	case GL_DEBUG_TYPE_MARKER:              std::cout << "Type: Marker"; break;
	case GL_DEBUG_TYPE_PUSH_GROUP:          std::cout << "Type: Push Group"; break;
	case GL_DEBUG_TYPE_POP_GROUP:           std::cout << "Type: Pop Group"; break;
	case GL_DEBUG_TYPE_OTHER:               std::cout << "Type: Other"; break;
	}
	std::cout << "\n";

	switch (severity) {
	case GL_DEBUG_SEVERITY_HIGH:         std::cout << "Severity: high"; break;
	case GL_DEBUG_SEVERITY_MEDIUM:       std::cout << "Severity: medium"; break;
	case GL_DEBUG_SEVERITY_LOW:          std::cout << "Severity: low"; break;
	case GL_DEBUG_SEVERITY_NOTIFICATION: std::cout << "Severity: notification"; break;
	}
		std::cout << "\n\n";
}

namespace Grindstone {
	namespace GraphicsAPI {
		bool GLGraphicsWrapper::initialize(GraphicsWrapperCreateInfo ci) {
			api_type_ = GraphicsAPIType::OpenGL;
			debug_ = ci.debug;
			window_ = ci.window;

#ifdef _WIN32
			static	PIXELFORMATDESCRIPTOR pfd =				// pfd Tells Windows How We Want Things To Be
			{
				sizeof(PIXELFORMATDESCRIPTOR),				// Size Of This Pixel Format Descriptor
				1,											// Version Number
				PFD_DRAW_TO_WINDOW |						// Format Must Support Window
				PFD_SUPPORT_OPENGL |						// Format Must Support OpenGL
				PFD_DOUBLEBUFFER,							// Must Support Double Buffering
				PFD_TYPE_RGBA,								// Request An RGBA Format
				32,											// Select Our Color Depth
				0, 0, 0, 0, 0, 0,							// Color Bits Ignored
				0,											// No Alpha Buffer
				0,											// Shift Bit Ignored
				0,											// No Accumulation Buffer
				0, 0, 0, 0,									// Accumulation Bits Ignored
				24,											// 24Bit Z-Buffer (Depth Buffer)  
				8,											// No Stencil Buffer
				0,											// No Auxiliary Buffer
				PFD_MAIN_PLANE,								// Main Drawing Layer
				0,											// Reserved
				0, 0, 0										// Layer Masks Ignored
			};

			if (!(window_device_context_ = GetDC(((Win32Window *)window_)->getHandle())))
				return false;

			unsigned int PixelFormat;
			if (!(PixelFormat = ChoosePixelFormat(window_device_context_, &pfd)))
				return false;

			if (!SetPixelFormat(window_device_context_, PixelFormat, &pfd))
				return false;

			HGLRC temp = wglCreateContext(window_device_context_);
			if (!temp) {
				return false;
			}

			if (wglMakeCurrent(window_device_context_, temp) == NULL) {
				return false;
			}


			int major = 4;
			int minor = 6;
			int attribs[] =
			{
				WGL_CONTEXT_MAJOR_VERSION_ARB, major,
				WGL_CONTEXT_MINOR_VERSION_ARB, minor,
				WGL_CONTEXT_FLAGS_ARB,
				WGL_CONTEXT_FORWARD_COMPATIBLE_BIT_ARB | WGL_CONTEXT_DEBUG_BIT_ARB,
				WGL_CONTEXT_PROFILE_MASK_ARB, WGL_CONTEXT_CORE_PROFILE_BIT_ARB,
				0
			}; //  | WGL_CONTEXT_DEBUG_BIT_ARB

			PFNWGLCREATECONTEXTATTRIBSARBPROC wglCreateContextAttribsARB = NULL;
			wglCreateContextAttribsARB = (PFNWGLCREATECONTEXTATTRIBSARBPROC)
				wglGetProcAddress("wglCreateContextAttribsARB");
			if (wglCreateContextAttribsARB != NULL)
				window_render_context_ = wglCreateContextAttribsARB(window_device_context_, 0, attribs);

			PFNWGLSWAPINTERVALEXTPROC wglSwapIntervalEXT = NULL;
			wglSwapIntervalEXT = (PFNWGLSWAPINTERVALEXTPROC)
				wglGetProcAddress("wglSwapIntervalEXT");
			if (wglSwapIntervalEXT != NULL)
				wglSwapIntervalEXT(true);


			if (!window_render_context_)
				window_render_context_ = temp;
			else
			{
				wglMakeCurrent(window_device_context_, window_render_context_);
				wglDeleteContext(temp);
			}

			if (gl3wInit()) {
				printf("Failed to initialize GL3W. Returning...\n");
				return false;
			}

			if (!gl3wIsSupported(3, 1)) {
				printf("OpenGL %i.%i or more required for Grindstone Engine.\n", 3, 1);
				printf("Your Graphics Card only supports version %s. Quitting...\n\n", glGetString(GL_VERSION));
				return false;
			}
#endif

			vendor_name_		= (const char *)glGetString(GL_VENDOR);
			adapter_name_		= (const char *)glGetString(GL_RENDERER);
			api_version_		= (const char *)glGetString(GL_VERSION);

			if (debug_) {
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

			return true;
		}

		void GLGraphicsWrapper::clear(ClearMode mask, float clear_color[4], float clear_depth, uint32_t clear_stencil) {
			glClearColor(clear_color[0], clear_color[1], clear_color[2], clear_color[3]);
			glClearDepthf(clear_depth);
			glClearStencil(clear_stencil);

			int m = (((uint8_t)mask & (uint8_t)ClearMode::Color) != 0) ? GL_COLOR_BUFFER_BIT : 0;
			m = m | ((((uint8_t)mask & (uint8_t)ClearMode::Depth) != 0) ? GL_DEPTH_BUFFER_BIT : 0);
			m = m | ((((uint8_t)mask & (uint8_t)ClearMode::Stencil) != 0) ? GL_STENCIL_BUFFER_BIT : 0);
			glClear(m);
		}

		void GLGraphicsWrapper::swapBuffers() {
			SwapBuffers(window_device_context_);
		}

		void GLGraphicsWrapper::adjustPerspective(float *perspective) {
		}

		GLGraphicsWrapper::~GLGraphicsWrapper() {
			wglDeleteContext(window_render_context_);
			ReleaseDC(((Win32Window*)window_)->getHandle(), window_device_context_);
		}

		//==================================
		// Get Text Metainfo
		//==================================
		const char* GLGraphicsWrapper::getVendorName() {
			return vendor_name_.c_str();
		}

		const char* GLGraphicsWrapper::getAdapterName() {
			return adapter_name_.c_str();
		}

		const char* GLGraphicsWrapper::getAPIName() {
			return "OpenGL";
		}

		const char* GLGraphicsWrapper::getAPIVersion() {
			return api_version_.c_str();
		}


		//==================================
		// Creators
		//==================================
		TextureBinding* GLGraphicsWrapper::createTextureBinding(TextureBindingCreateInfo createInfo) {
			return static_cast<TextureBinding*>(new GLTextureBinding(createInfo));
		}

		TextureBindingLayout* GLGraphicsWrapper::createTextureBindingLayout(TextureBindingLayoutCreateInfo createInfo) {
			return static_cast<TextureBindingLayout*>(new GLTextureBindingLayout(createInfo));
		}

		Framebuffer* GLGraphicsWrapper::createFramebuffer(FramebufferCreateInfo ci) {
			return static_cast<Framebuffer*>(new GLFramebuffer(ci));
		}

		RenderPass* GLGraphicsWrapper::createRenderPass(RenderPassCreateInfo ci) {
			return 0;
		}

		GraphicsPipeline* GLGraphicsWrapper::createGraphicsPipeline(GraphicsPipelineCreateInfo ci) {
			return static_cast<GraphicsPipeline*>(new GLGraphicsPipeline(ci));
		}

		VertexArrayObject* GLGraphicsWrapper::createVertexArrayObject(VertexArrayObjectCreateInfo ci) {
			return static_cast<VertexArrayObject*>(new GLVertexArrayObject(ci));
		}

		CommandBuffer* GLGraphicsWrapper::createCommandBuffer(CommandBufferCreateInfo ci) {
			return 0;
		}

		VertexBuffer* GLGraphicsWrapper::createVertexBuffer(VertexBufferCreateInfo ci) {
			return static_cast<VertexBuffer*>(new GLVertexBuffer(ci));
		}

		IndexBuffer* GLGraphicsWrapper::createIndexBuffer(IndexBufferCreateInfo ci) {
			return static_cast<IndexBuffer*>(new GLIndexBuffer(ci));
		}

		UniformBuffer* GLGraphicsWrapper::createUniformBuffer(UniformBufferCreateInfo ci) {
			return static_cast<UniformBuffer*>(new GLUniformBuffer(ci));
		}

		UniformBufferBinding* GLGraphicsWrapper::createUniformBufferBinding(UniformBufferBindingCreateInfo ci) {
			return static_cast<UniformBufferBinding*>(new GLUniformBufferBinding(ci));
		}

		Texture* GLGraphicsWrapper::createCubemap(CubemapCreateInfo ci) {
			return static_cast<Texture*>(new GLTexture(ci));
		}

		Texture* GLGraphicsWrapper::createTexture(TextureCreateInfo ci) {
			return static_cast<Texture*>(new GLTexture(ci));
		}

		RenderTarget* GLGraphicsWrapper::createRenderTarget(RenderTargetCreateInfo* rt, uint32_t rc, bool cube) {
			return static_cast<RenderTarget*>(new GLRenderTarget(rt, rc, cube));
		}

		DepthTarget* GLGraphicsWrapper::createDepthTarget(DepthTargetCreateInfo rt) {
			return static_cast<DepthTarget*>(new GLDepthTarget(rt));
		}

		//==================================
		// Booleans
		//==================================
		inline const bool GLGraphicsWrapper::shouldUseImmediateMode() {
			return true;
		}
		inline const bool GLGraphicsWrapper::supportsCommandBuffers() {
			return false;
		}
		inline const bool GLGraphicsWrapper::supportsTesselation() {
			return gl3wIsSupported(4, 0) ? true : false;
		}
		inline const bool GLGraphicsWrapper::supportsGeometryShader() {
			return gl3wIsSupported(3, 2) ? true : false;
		}
		inline const bool GLGraphicsWrapper::supportsComputeShader() {
			return gl3wIsSupported(4, 3) ? true : false;
		}
		inline const bool GLGraphicsWrapper::supportsMultiDrawIndirect() {
			return gl3wIsSupported(4, 3) ? true : false;
		}

		//==================================
		// Deleters
		//==================================
		void GLGraphicsWrapper::deleteRenderTarget(RenderTarget *ptr) {
			delete (GLRenderTarget *)ptr;
		}

		void GLGraphicsWrapper::deleteDepthTarget(DepthTarget *ptr) {
			delete (GLDepthTarget *)ptr;
		}

		void GLGraphicsWrapper::deleteFramebuffer(Framebuffer *ptr) {
			delete (GLFramebuffer *)ptr;
		}

		void GLGraphicsWrapper::deleteVertexBuffer(VertexBuffer *ptr) {
			delete (GLVertexBuffer *)ptr;
		}

		void GLGraphicsWrapper::deleteIndexBuffer(IndexBuffer *ptr) {
			delete (GLIndexBuffer *)ptr;
		}

		void GLGraphicsWrapper::deleteUniformBuffer(UniformBuffer * ptr) {
			delete (GLUniformBuffer *)ptr;
		}

		void GLGraphicsWrapper::deleteUniformBufferBinding(UniformBufferBinding * ptr) {
			delete (GLUniformBufferBinding *)ptr;
		}

		void GLGraphicsWrapper::deleteGraphicsPipeline(GraphicsPipeline *ptr) {
			delete (GLGraphicsPipeline *)ptr;
		}

		void GLGraphicsWrapper::deleteRenderPass(RenderPass *ptr) {
			//delete (GLRenderPass *)ptr;
		}

		void GLGraphicsWrapper::deleteTexture(Texture *ptr) {
			delete (GLTexture *)ptr;
		}

		void GLGraphicsWrapper::deleteTextureBinding(TextureBinding *ptr) {
			delete (GLTextureBinding *)ptr;
		}

		void GLGraphicsWrapper::deleteTextureBindingLayout(TextureBindingLayout *ptr) {
			delete (GLTextureBindingLayout *)ptr;
		}

		void GLGraphicsWrapper::deleteCommandBuffer(CommandBuffer * ptr) {
		}

		void GLGraphicsWrapper::deleteVertexArrayObject(VertexArrayObject * ptr) {
			delete (GLVertexArrayObject *)ptr;
		}

		void GLGraphicsWrapper::getSwapChainRenderTargets(RenderTarget **& rts, uint32_t & rt_count)
		{
		}

		void GLGraphicsWrapper::setViewport(uint16_t x, uint16_t y, uint16_t w, uint16_t h) {
			glViewport(x, y, w, h);
			glScissor(x, y, w, h);
		}

		void GLGraphicsWrapper::copyToDepthBuffer(DepthTarget *p) {
			glBlitFramebuffer(0, 0, 1920, 1080, 0, 0, 1920, 1080, GL_DEPTH_BUFFER_BIT, GL_NEAREST);
		}

		uint32_t GLGraphicsWrapper::getImageIndex() {
			return 0;
		}

		void GLGraphicsWrapper::waitUntilIdle() {

		}

		void GLGraphicsWrapper::drawCommandBuffers(uint32_t imageIndex, CommandBuffer ** commandBuffers, uint32_t commandBufferCount) {

		}

		void GLGraphicsWrapper::setColorMask(ColorMask mask) {
			glColorMask((GLboolean)(mask & ColorMask::Red), (GLboolean)(mask & ColorMask::Blue), (GLboolean)(mask & ColorMask::Green), (GLboolean)(mask & ColorMask::Alpha));
		}

		void GLGraphicsWrapper::bindTextureBinding(TextureBinding *binding) {
			GLTextureBinding *b = (GLTextureBinding *)binding;
			b->Bind();
		}

		void GLGraphicsWrapper::bindVertexArrayObject(VertexArrayObject *vao) {
			vao->Bind();
		}

		GLenum GetGeomType(GeometryType geom_type) {
			switch (geom_type) {
			case GeometryType::Points:
				return GL_POINTS;
			case GeometryType::Lines:
				return GL_LINES;
			case GeometryType::LineStrips:
				return GL_LINE_STRIP;
			case GeometryType::LineLoops:
				return GL_LINE_LOOP;
			case GeometryType::TriangleStrips:
				return GL_TRIANGLE_STRIP;
			case GeometryType::TriangleFans:
				return GL_TRIANGLE_FAN;
			case GeometryType::Triangles:
				return GL_TRIANGLES;
			case GeometryType::LinesAdjacency:
				return GL_LINES_ADJACENCY;
			case GeometryType::TrianglesAdjacency:
				return GL_TRIANGLES_ADJACENCY;
			case GeometryType::TriangleStripsAdjacency:
				return GL_TRIANGLE_STRIP_ADJACENCY;
			case GeometryType::Patches:
				return GL_PATCHES;
			}

			throw std::runtime_error("Invalid Geometry Type");
		}

		void GLGraphicsWrapper::drawImmediateIndexed(GeometryType geom_type, bool largeBuffer, int32_t baseVertex, uint32_t indexOffsetPtr, uint32_t indexCount) {
			uint32_t size = largeBuffer ? sizeof(uint32_t) : sizeof(uint16_t);
			void *ptr = reinterpret_cast<void *>(indexOffsetPtr * size);
			glDrawElementsBaseVertex(GetGeomType(geom_type), indexCount, largeBuffer ? GL_UNSIGNED_INT : GL_UNSIGNED_SHORT, ptr, baseVertex);
		}

		void GLGraphicsWrapper::drawImmediateVertices(GeometryType geom_type, uint32_t base, uint32_t count) {
			glDrawArrays(GetGeomType(geom_type), base, count);
		}

		void GLGraphicsWrapper::enableDepth(bool state) {
			glDepthMask(state);
		}

		void GLGraphicsWrapper::bindDefaultFramebuffer(bool depth) {
			glDepthMask(depth ? GL_TRUE : GL_FALSE);
			glBindFramebuffer(GL_FRAMEBUFFER, 0);
		}

		void GLGraphicsWrapper::setImmediateBlending(BlendMode mode) {
			switch (mode) {
			case BlendMode::None:
			default:
				glDisable(GL_BLEND);
				break;
			case BlendMode::Additive:
				glEnable(GL_BLEND);
				glBlendEquation(GL_FUNC_ADD);
				glBlendFunc(GL_ONE, GL_ONE);
				break;
			case BlendMode::AdditiveAlpha:
				glEnable(GL_BLEND);
				glBlendEquation(GL_FUNC_ADD);
				glBlendFuncSeparate(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA, GL_ONE, GL_ONE_MINUS_SRC_ALPHA);
				break;
			}
		}

		ColorFormat GLGraphicsWrapper::getDeviceColorFormat() {
			return ColorFormat::R8G8B8A8;
		}

	}
}

extern "C" {
	GRAPHICS_EXPORT Grindstone::GraphicsAPI::GraphicsWrapper* createGraphics() {
		return new Grindstone::GraphicsAPI::GLGraphicsWrapper();
	}

	GRAPHICS_EXPORT void deleteGraphics(void* ptr) {
		Grindstone::GraphicsAPI::GLGraphicsWrapper* glptr = (Grindstone::GraphicsAPI::GLGraphicsWrapper*)ptr;
		delete glptr;
	}
}