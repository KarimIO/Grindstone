set(GRAPHICS_DIR "${COMMON_DIR}/Graphics")

set(GL_CORE_SOURCES ${DEPS_DIR}/GL/gl3w.cpp ${GRAPHICS_DIR}/Formats.cpp GLCore.cpp EntryPoint.cpp)
set(GL_CORE_HEADERS ${DEPS_DIR}/GL/gl3w.h glcorearb.hpp GLCore.hpp)
set(GL_OBJ_SOURCES GLDepthTarget.cpp GLRenderTarget.cpp GLFramebuffer.cpp GLPipeline.cpp GLTexture.cpp GLVertexArrayObject.cpp GLVertexBuffer.cpp GLIndexBuffer.cpp GLUniformBuffer.cpp)
set(GL_OBJ_HEADERS GLDepthTarget.hpp GLRenderTarget.hpp GLFramebuffer.hpp GLPipeline.hpp GLTexture.hpp GLVertexArrayObject.hpp GLVertexBuffer.hpp GLIndexBuffer.hpp GLUniformBuffer.hpp)
set(GL_WINDOW_SOURCES ${COMMON_DIR}/Window/Window.cpp)
set(GL_WINDOW_HEADERS ${COMMON_DIR}/Window/Window.hpp)
set(GL_DISPLAY_SOURCES ${COMMON_DIR}/Display/Display.cpp)
set(GL_DISPLAY_HEADERS ${COMMON_DIR}/Display/Display.hpp)
set(GL_COMMON_HEADERS ${GRAPHICS_DIR}/DepthTarget.hpp ${GRAPHICS_DIR}/RenderTarget.hpp ${GRAPHICS_DIR}/CommandBuffer.hpp ${GRAPHICS_DIR}/Formats.hpp ${GRAPHICS_DIR}/Formats.cpp ${GRAPHICS_DIR}/VertexArrayObject.hpp ${GRAPHICS_DIR}/Framebuffer.hpp ${GRAPHICS_DIR}/Pipeline.hpp ${GRAPHICS_DIR}/Core.hpp ${GRAPHICS_DIR}/IndexBuffer.hpp ${GRAPHICS_DIR}/RenderPass.hpp ${GRAPHICS_DIR}/Texture.hpp ${GRAPHICS_DIR}/UniformBuffer.hpp ${GRAPHICS_DIR}/VertexBuffer.hpp ${GRAPHICS_DIR}/WindowGraphicsBinding.hpp)

set(GL_OBJ_SOURCES ${GL_OBJ_SOURCES} GLWindowGraphicsBinding.cpp)
set(GL_OBJ_HEADERS ${GL_OBJ_HEADERS} GLWindowGraphicsBinding.hpp)

if (WIN32)
    set(GL_WINDOW_SOURCES ${GL_WINDOW_SOURCES} ${COMMON_DIR}/Window/Win32Window.cpp)
    set(GL_WINDOW_HEADERS ${GL_WINDOW_HEADERS} ${COMMON_DIR}/Window/Win32Window.hpp)
elseif (UNIX)
    set(GL_WINDOW_SOURCES ${GL_WINDOW_SOURCES} ${COMMON_DIR}/Window/X11Window.cpp)
    set(GL_WINDOW_HEADERS ${GL_WINDOW_HEADERS} ${COMMON_DIR}/Window/X11Window.hpp)
endif()

set(GL_SOURCES ${GL_CORE_SOURCES} ${GL_OBJ_SOURCES} ${GL_WINDOW_SOURCES} ${GL_DISPLAY_SOURCES})
set(GL_HEADERS ${GL_CORE_HEADERS} ${GL_OBJ_HEADERS} ${GL_WINDOW_HEADERS} ${GL_DISPLAY_HEADERS} ${GL_COMMON_HEADERS})

source_group("Header Files\\Common Objects" FILES ${GL_COMMON_HEADERS})

source_group("Source Files\\Objects" FILES ${GL_OBJ_SOURCES})
source_group("Header Files\\Objects" FILES ${GL_OBJ_HEADERS})

source_group("Source Files\\Window" FILES ${GL_WINDOW_SOURCES})
source_group("Header Files\\Window" FILES ${GL_WINDOW_HEADERS})

source_group("Source Files\\Display" FILES ${GL_DISPLAY_SOURCES})
source_group("Header Files\\Display" FILES ${GL_DISPLAY_HEADERS})

add_library(PluginGraphicsOpenGL MODULE ${GL_SOURCES} ${GL_HEADERS})

target_include_directories(PluginGraphicsOpenGL PUBLIC ${GLM_DIR} ${DEPS_DIR})

target_compile_features(PluginGraphicsOpenGL PRIVATE cxx_std_17)

target_compile_definitions(PluginGraphicsOpenGL PUBLIC GRAPHICS_OPENGL GRAPHICS_DLL)

target_include_directories(PluginGraphicsOpenGL
    PUBLIC ${CODE_DIR} ${DEPS_DIR}
)

target_precompile_headers(PluginGraphicsOpenGL
    PUBLIC pch.hpp)

find_package(OpenGL) 
target_link_libraries(PluginGraphicsOpenGL ${OPENGL_gl_LIBRARY})

if (APPLE)
    target_link_directories(PluginGraphicsOpenGL ${GLFW_DIR}/lib)
    target_link_libraries(PluginGraphicsOpenGL glfw3)
    target_include_directories(PluginGraphicsOpenGL PUBLIC ${GLFW_DIR}/include)
endif(APPLE)