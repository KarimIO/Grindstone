set(GRAPHICS_DIR "${COMMON_DIR}/Graphics")
set(RHI_OPENGL_BASE ${PLUGIN_DIR}/Grindstone.RHI.OpenGL)
set(RHI_OPENGL_INCLUDES ${RHI_OPENGL_BASE}/include)
set(RHI_OPENGL_BIN ${RHI_OPENGL_BASE}/bin)
set(RHI_OPENGL_LIB ${RHI_OPENGL_BASE}/lib)
set(SRC ${RHI_OPENGL_BASE}/source)
set(INC ${RHI_OPENGL_BASE}/include)

set(GL_CORE_SOURCES ${SRC}/GLFormats.cpp ${SRC}/GLCore.cpp ${SRC}/EntryPoint.cpp)
set(GL_CORE_HEADERS ${INC}/GLFormats.hpp ${INC}/GLCore.hpp)
set(GL_OBJ_SOURCES ${SRC}/GLSampler.cpp ${SRC}/GLFramebuffer.cpp ${SRC}/GLComputePipeline.cpp ${SRC}/GLGraphicsPipeline.cpp ${SRC}/GLImage.cpp ${SRC}/GLVertexArrayObject.cpp ${SRC}/GLBuffer.cpp ${SRC}/GLDescriptorSet.cpp ${SRC}/GLDescriptorSetLayout.cpp)
set(GL_OBJ_HEADERS ${INC}/GLSampler.hpp ${INC}/GLFramebuffer.hpp ${INC}/GLComputePipeline.hpp ${INC}/GLGraphicsPipeline.hpp ${INC}/GLImage.hpp ${INC}/GLVertexArrayObject.hpp ${INC}/GLBuffer.hpp ${INC}/GLDescriptorSet.hpp ${INC}/GLDescriptorSetLayout.hpp)
set(GL_WINDOW_SOURCES ${COMMON_DIR}/Window/WindowManager.cpp)
set(GL_WINDOW_HEADERS ${COMMON_DIR}/Window/WindowManager.hpp ${COMMON_DIR}/Window/Window.hpp)
set(GL_DISPLAY_SOURCES ${COMMON_DIR}/Display/DisplayManager.cpp)
set(GL_DISPLAY_HEADERS ${COMMON_DIR}/Display/DisplayManager.hpp ${COMMON_DIR}/Display/Display.hpp)
set(GL_COMMON_HEADERS ${GRAPHICS_DIR}/DescriptorSet.hpp ${GRAPHICS_DIR}/DescriptorSetLayout.hpp ${GRAPHICS_DIR}/Sampler.hpp ${GRAPHICS_DIR}/CommandBuffer.hpp ${GRAPHICS_DIR}/Formats.cpp ${GRAPHICS_DIR}/Formats.hpp ${GRAPHICS_DIR}/VertexArrayObject.hpp ${GRAPHICS_DIR}/Framebuffer.hpp ${GRAPHICS_DIR}/GraphicsPipeline.hpp ${GRAPHICS_DIR}/ComputePipeline.hpp ${GRAPHICS_DIR}/Core.hpp ${GRAPHICS_DIR}/RenderPass.hpp ${GRAPHICS_DIR}/Image.hpp ${GRAPHICS_DIR}/Buffer.hpp ${GRAPHICS_DIR}/WindowGraphicsBinding.hpp)

set(GL_OBJ_SOURCES ${GL_OBJ_SOURCES} ${SRC}/GLWindowGraphicsBinding.cpp)
set(GL_OBJ_HEADERS ${GL_OBJ_HEADERS} ${INC}/GLWindowGraphicsBinding.hpp)

#if (WIN32)
#    set(GL_WINDOW_SOURCES ${GL_WINDOW_SOURCES} ${COMMON_DIR}/Window/Win32Window.cpp)
#    set(GL_WINDOW_HEADERS ${GL_WINDOW_HEADERS} ${COMMON_DIR}/Window/Win32Window.hpp)
#elseif (UNIX)
#    set(GL_WINDOW_SOURCES ${GL_WINDOW_SOURCES} ${COMMON_DIR}/Window/X11Window.cpp)
#    set(GL_WINDOW_HEADERS ${GL_WINDOW_HEADERS} ${COMMON_DIR}/Window/X11Window.hpp)
#endif()

set(GL_WINDOW_SOURCES ${GL_WINDOW_SOURCES} ${COMMON_DIR}/Window/GlfwWindow.cpp)
set(GL_WINDOW_HEADERS ${GL_WINDOW_HEADERS} ${COMMON_DIR}/Window/GlfwWindow.hpp)

set(GL_SOURCES ${GL_CORE_SOURCES} ${GL_OBJ_SOURCES} ${GL_WINDOW_SOURCES} ${GL_DISPLAY_SOURCES})
set(GL_HEADERS ${GL_CORE_HEADERS} ${GL_OBJ_HEADERS} ${GL_WINDOW_HEADERS} ${GL_DISPLAY_HEADERS} ${GL_COMMON_HEADERS})

source_group("Header Files\\Common Objects" FILES ${GL_COMMON_HEADERS})

source_group("Source Files\\Objects" FILES ${GL_OBJ_SOURCES})
source_group("Header Files\\Objects" FILES ${GL_OBJ_HEADERS})

source_group("Source Files\\Window" FILES ${GL_WINDOW_SOURCES})
source_group("Header Files\\Window" FILES ${GL_WINDOW_HEADERS})

source_group("Source Files\\Display" FILES ${GL_DISPLAY_SOURCES})
source_group("Header Files\\Display" FILES ${GL_DISPLAY_HEADERS})

add_library(PluginRhiOpenGL MODULE ${GL_SOURCES} ${GL_HEADERS} ${CORE_UTILS})

set_target_properties(PluginRhiOpenGL PROPERTIES
	RUNTIME_OUTPUT_DIRECTORY "${RHI_OPENGL_BIN}"
	LIBRARY_OUTPUT_DIRECTORY "${RHI_OPENGL_LIB}"
	ARCHIVE_OUTPUT_DIRECTORY "${RHI_OPENGL_LIB}"
)
set_property(TARGET PluginRhiOpenGL PROPERTY COMPILE_WARNING_AS_ERROR ON)
target_compile_features(PluginRhiOpenGL PRIVATE cxx_std_20)
set_target_properties(PluginRhiOpenGL PROPERTIES FOLDER "Plugins/Render Hardware Interface")

target_compile_definitions(PluginRhiOpenGL PUBLIC GRAPHICS_OPENGL GRAPHICS_DLL GLM_ENABLE_EXPERIMENTAL)
target_include_directories(PluginRhiOpenGL PUBLIC ${CODE_DIR} ${PLUGIN_DIR})

target_precompile_headers(PluginRhiOpenGL
    PUBLIC ${INC}/pch.hpp
)

find_package(OpenGL REQUIRED)
find_package(gl3w CONFIG REQUIRED)
find_package(glfw3 CONFIG REQUIRED)

target_link_libraries(PluginRhiOpenGL glfw unofficial::gl3w::gl3w glm::glm fmt::fmt ${CORE_LIBS} ${OPENGL_gl_LIBRARY})

message(STATUS
"PluginRhiOpenGL provides CMake targets:

- RHI_OPENGL_BASE => ${RHI_OPENGL_BASE}
- RHI_OPENGL_INCLUDES => ${RHI_OPENGL_INCLUDES}
- RHI_OPENGL_BIN => ${RHI_OPENGL_BIN}
- RHI_OPENGL_LIB => ${RHI_OPENGL_LIB}


")
