set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(GL_CORE_SOURCES ../../deps/GL/gl3w.cpp GLGraphicsWrapper.cpp ../WindowModule/glfwWindow.cpp)
set(GL_CORE_HEADERS ../../deps/GL/gl3w.h glcorearb.hpp GLGraphicsWrapper.hpp)
set(GL_OBJ_SOURCES GLDepthTarget.cpp GLRenderTarget.cpp GLFramebuffer.cpp GLGraphicsPipeline.cpp GLTexture.cpp GLVertexArrayObject.cpp GLVertexBuffer.cpp GLIndexBuffer.cpp GLUniformBuffer.cpp)
set(GL_OBJ_HEADERS GLDepthTarget.hpp GLRenderTarget.hpp GLFramebuffer.hpp GLGraphicsPipeline.hpp GLTexture.hpp GLVertexArrayObject.hpp GLVertexBuffer.hpp GLIndexBuffer.hpp GLUniformBuffer.hpp)
set(GL_COMMON_HEADERS ../GraphicsCommon/DepthTarget.hpp ../GraphicsCommon/RenderTarget.hpp ../GraphicsCommon/CommandBuffer.hpp ../GraphicsCommon/Formats.hpp ../GraphicsCommon/Formats.cpp ../GraphicsCommon/VertexArrayObject.hpp ../GraphicsCommon/Framebuffer.hpp ../GraphicsCommon/GraphicsPipeline.hpp ../GraphicsCommon/GraphicsWrapper.hpp ../GraphicsCommon/IndexBuffer.hpp ../GraphicsCommon/RenderPass.hpp ../GraphicsCommon/Texture.hpp ../GraphicsCommon/UniformBuffer.hpp ../GraphicsCommon/VertexBuffer.hpp)

if (WIN32)
    set(GL_SOURCES_WINDOW ../WindowModule/win32Window.cpp)
else(WIN32)
    set(GL_SOURCES_WINDOW ../WindowModule/x11Window.cpp)
endif(WIN32)

set(GL_SOURCES ${GL_CORE_SOURCES} ${GL_OBJ_SOURCES} ${GL_SOURCES_WINDOW})
set(GL_HEADERS ${GL_CORE_HEADERS} ${GL_OBJ_HEADERS} ${GL_COMMON_HEADERS})

source_group("Header Files\\Common Objects" FILES ${GL_COMMON_HEADERS})

source_group("Source Files\\Objects" FILES ${ENGINE_CORE_SOURCES})
source_group("Header Files\\Objects" FILES ${ENGINE_CORE_SOURCES})

add_library(graphicsgl MODULE ${GL_SOURCES} ${GL_HEADERS})

target_include_directories(graphicsgl PUBLIC ${GLM_DIR} ${DEPS_DIR})

target_compile_definitions(graphicsgl PRIVATE GRAPHICS_DLL)

find_package(OpenGL) 
target_link_libraries(graphicsgl ${OPENGL_gl_LIBRARY})

if (APPLE)
    target_link_directories(graphicsgl ${GLFW_DIR}/lib)
    target_link_libraries(graphicsgl glfw3)
    target_include_directories(graphicsgl PUBLIC ${GLFW_DIR}/include)
endif(APPLE)