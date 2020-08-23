#include <GL/gl3w.h>
#include "GLCore.hpp"

#include "GLVertexArrayObject.hpp"
#include "GLVertexBuffer.hpp"
#include "GLIndexBuffer.hpp"
#include "GLPipeline.hpp"
#include "GLRenderTarget.hpp"
#include "GLWindowGraphicsBinding.hpp"
#include <iostream>
#include <cstdio>

#ifdef _WIN32
	//#include <windows.h>
	//#include <Windowsx.h>
	#include <GL/wglext.h>
	#include <Common/Window/Win32Window.hpp>
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
		bool GLCore::initialize(Core::CreateInfo& ci) {
			api_type_ = API::OpenGL;
			debug_ = ci.debug;
			primary_window_ = ci.window;

			auto wgb = new GLWindowGraphicsBinding();
			ci.window->addBinding(wgb);
			wgb->initialize(ci.window);

			if (gl3wInit()) {
				printf("Failed to initialize GL3W. Returning...\n");
				return false;
			}

			if (!gl3wIsSupported(3, 1)) {
				printf("OpenGL %i.%i or more required for Grindstone Engine.\n", 3, 1);
				printf("Your Graphics Card only supports version %s. Quitting...\n\n", glGetString(GL_VERSION));
				return false;
			}

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

		void GLCore::clear(ClearMode mask, float clear_color[4], float clear_depth, uint32_t clear_stencil) {
			glClearColor(clear_color[0], clear_color[1], clear_color[2], clear_color[3]);
			glClearDepthf(clear_depth);
			glClearStencil(clear_stencil);

			int m = (((uint8_t)mask & (uint8_t)ClearMode::Color) != 0) ? GL_COLOR_BUFFER_BIT : 0;
			m = m | ((((uint8_t)mask & (uint8_t)ClearMode::Depth) != 0) ? GL_DEPTH_BUFFER_BIT : 0);
			m = m | ((((uint8_t)mask & (uint8_t)ClearMode::Stencil) != 0) ? GL_STENCIL_BUFFER_BIT : 0);
			glClear(m);
		}

		void GLCore::adjustPerspective(float *perspective) {
		}

		void GLCore::registerWindow(Window* window) {
			auto wgb = new GLWindowGraphicsBinding();
			window->addBinding(wgb);
			wgb->initialize(window);
			wgb->shareLists((GLWindowGraphicsBinding *)primary_window_->getWindowGraphicsBinding());
		}

		//==================================
		// Get Text Metainfo
		//==================================
		const char* GLCore::getVendorName() {
			return vendor_name_.c_str();
		}

		const char* GLCore::getAdapterName() {
			return adapter_name_.c_str();
		}

		const char* GLCore::getAPIName() {
			return "OpenGL";
		}

		const char* GLCore::getAPIVersion() {
			return api_version_.c_str();
		}


		//==================================
		// Creators
		//==================================
		TextureBinding* GLCore::createTextureBinding(TextureBinding::CreateInfo& createInfo) {
			return static_cast<TextureBinding*>(new GLTextureBinding(createInfo));
		}

		TextureBindingLayout* GLCore::createTextureBindingLayout(TextureBindingLayout::CreateInfo& createInfo) {
			return static_cast<TextureBindingLayout*>(new GLTextureBindingLayout(createInfo));
		}

		Framebuffer* GLCore::createFramebuffer(Framebuffer::CreateInfo& ci) {
			return static_cast<Framebuffer*>(new GLFramebuffer(ci));
		}

		RenderPass* GLCore::createRenderPass(RenderPass::CreateInfo& ci) {
			return 0;
		}

		Pipeline* GLCore::createPipeline(Pipeline::CreateInfo& ci) {
			return static_cast<Pipeline*>(new GLPipeline(ci));
		}

		VertexArrayObject* GLCore::createVertexArrayObject(VertexArrayObject::CreateInfo& ci) {
			return static_cast<VertexArrayObject*>(new GLVertexArrayObject(ci));
		}

		CommandBuffer* GLCore::createCommandBuffer(CommandBuffer::CreateInfo& ci) {
			return 0;
		}

		VertexBuffer* GLCore::createVertexBuffer(VertexBuffer::CreateInfo& ci) {
			return static_cast<VertexBuffer*>(new GLVertexBuffer(ci));
		}

		IndexBuffer* GLCore::createIndexBuffer(IndexBuffer::CreateInfo& ci) {
			return static_cast<IndexBuffer*>(new GLIndexBuffer(ci));
		}

		UniformBuffer* GLCore::createUniformBuffer(UniformBuffer::CreateInfo& ci) {
			return static_cast<UniformBuffer*>(new GLUniformBuffer(ci));
		}

		UniformBufferBinding* GLCore::createUniformBufferBinding(UniformBufferBinding::CreateInfo& ci) {
			return static_cast<UniformBufferBinding*>(new GLUniformBufferBinding(ci));
		}

		Texture* GLCore::createCubemap(Texture::CubemapCreateInfo& ci) {
			return static_cast<Texture*>(new GLTexture(ci));
		}

		Texture* GLCore::createTexture(Texture::CreateInfo& ci) {
			return static_cast<Texture*>(new GLTexture(ci));
		}

		RenderTarget* GLCore::createRenderTarget(RenderTarget::CreateInfo* rt, uint32_t rc, bool cube) {
			return static_cast<RenderTarget*>(new GLRenderTarget(rt, rc, cube));
		}

		DepthTarget* GLCore::createDepthTarget(DepthTarget::CreateInfo& rt) {
			return static_cast<DepthTarget*>(new GLDepthTarget(rt));
		}

		//==================================
		// Booleans
		//==================================
		const bool GLCore::shouldUseImmediateMode() {
			return true;
		}
		const bool GLCore::supportsCommandBuffers() {
			return false;
		}
		const bool GLCore::supportsTesselation() {
			return gl3wIsSupported(4, 0) ? true : false;
		}
		const bool GLCore::supportsGeometryShader() {
			return gl3wIsSupported(3, 2) ? true : false;
		}
		const bool GLCore::supportsComputeShader() {
			return gl3wIsSupported(4, 3) ? true : false;
		}
		const bool GLCore::supportsMultiDrawIndirect() {
			return gl3wIsSupported(4, 3) ? true : false;
		}

		//==================================
		// Deleters
		//==================================
		void GLCore::deleteRenderTarget(RenderTarget *ptr) {
			delete (GLRenderTarget *)ptr;
		}

		void GLCore::deleteDepthTarget(DepthTarget *ptr) {
			delete (GLDepthTarget *)ptr;
		}

		void GLCore::deleteFramebuffer(Framebuffer *ptr) {
			delete (GLFramebuffer *)ptr;
		}

		void GLCore::deleteVertexBuffer(VertexBuffer *ptr) {
			delete (GLVertexBuffer *)ptr;
		}

		void GLCore::deleteIndexBuffer(IndexBuffer *ptr) {
			delete (GLIndexBuffer *)ptr;
		}

		void GLCore::deleteUniformBuffer(UniformBuffer * ptr) {
			delete (GLUniformBuffer *)ptr;
		}

		void GLCore::deleteUniformBufferBinding(UniformBufferBinding * ptr) {
			delete (GLUniformBufferBinding *)ptr;
		}

		void GLCore::deletePipeline(Pipeline*ptr) {
			delete (GLPipeline *)ptr;
		}

		void GLCore::deleteRenderPass(RenderPass *ptr) {
			//delete (GLRenderPass *)ptr;
		}

		void GLCore::deleteTexture(Texture *ptr) {
			delete (GLTexture *)ptr;
		}

		void GLCore::deleteTextureBinding(TextureBinding *ptr) {
			delete (GLTextureBinding *)ptr;
		}

		void GLCore::deleteTextureBindingLayout(TextureBindingLayout *ptr) {
			delete (GLTextureBindingLayout *)ptr;
		}

		void GLCore::deleteCommandBuffer(CommandBuffer * ptr) {
		}

		void GLCore::deleteVertexArrayObject(VertexArrayObject * ptr) {
			delete (GLVertexArrayObject *)ptr;
		}

		void GLCore::copyToDepthBuffer(DepthTarget *p) {
			glBlitFramebuffer(0, 0, 1920, 1080, 0, 0, 1920, 1080, GL_DEPTH_BUFFER_BIT, GL_NEAREST);
		}

		void GLCore::waitUntilIdle() {

		}

		void GLCore::setColorMask(ColorMask mask) {
			glColorMask((GLboolean)(mask & ColorMask::Red), (GLboolean)(mask & ColorMask::Blue), (GLboolean)(mask & ColorMask::Green), (GLboolean)(mask & ColorMask::Alpha));
		}

		void GLCore::bindTextureBinding(TextureBinding *binding) {
			GLTextureBinding *b = (GLTextureBinding *)binding;
			b->bind();
		}

		void GLCore::bindVertexArrayObject(VertexArrayObject *vao) {
			vao->bind();
		}

		GLenum getGeomType(GeometryType geom_type) {
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

		void GLCore::drawImmediateIndexed(GeometryType geom_type, bool largeBuffer, int32_t baseVertex, uint32_t indexOffsetPtr, uint32_t indexCount) {
			uint32_t size = largeBuffer ? sizeof(uint32_t) : sizeof(uint16_t);
			void *ptr = reinterpret_cast<void *>(indexOffsetPtr * size);
			glDrawElementsBaseVertex(getGeomType(geom_type), indexCount, largeBuffer ? GL_UNSIGNED_INT : GL_UNSIGNED_SHORT, ptr, baseVertex);
		}

		void GLCore::drawImmediateVertices(GeometryType geom_type, uint32_t base, uint32_t count) {
			glDrawArrays(getGeomType(geom_type), base, count);
		}

		void GLCore::enableDepth(bool state) {
			glDepthMask(state);
		}

		void GLCore::bindDefaultFramebuffer(bool depth) {
			glDepthMask(depth ? GL_TRUE : GL_FALSE);
			glBindFramebuffer(GL_FRAMEBUFFER, 0);
		}

		void GLCore::setImmediateBlending(BlendMode mode) {
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

	}
}

/*
extern "C" {
	GRAPHICS_EXPORT Grindstone::GraphicsAPI::GraphicsWrapper* createGraphics() {
		return new Grindstone::GraphicsAPI::GLCore();
	}

	GRAPHICS_EXPORT void deleteGraphics(void* ptr) {
		Grindstone::GraphicsAPI::GLCore* glptr = (Grindstone::GraphicsAPI::GLCore*)ptr;
		delete glptr;
	}
}
*/