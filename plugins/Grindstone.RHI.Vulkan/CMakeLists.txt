set(GRAPHICS_DIR "${COMMON_DIR}/Graphics")
set(RHI_VULKAN_BASE ${PLUGIN_DIR}/Grindstone.RHI.Vulkan)
set(RHI_VULKAN_INCLUDES ${RHI_VULKAN_BASE}/include)
set(RHI_VULKAN_BIN ${RHI_VULKAN_BASE}/bin)
set(RHI_VULKAN_LIB ${RHI_VULKAN_BASE}/lib)
set(SRC ${RHI_VULKAN_BASE}/source)
set(INC ${RHI_VULKAN_BASE}/include)

set(Vk_CORE_SOURCES ${SRC}/VulkanCore.cpp ${SRC}/EntryPoint.cpp)
set(Vk_CORE_HEADERS ${INC}/VulkanCore.hpp)
set(Vk_OBJ_SOURCES ${SRC}/VulkanUtils.cpp ${SRC}/VulkanFormat.cpp ${SRC}/VulkanSampler.cpp ${SRC}/VulkanFramebuffer.cpp ${SRC}/VulkanComputePipeline.cpp ${SRC}/VulkanGraphicsPipeline.cpp ${SRC}/VulkanImage.cpp ${SRC}/VulkanRenderPass.cpp ${SRC}/VulkanBuffer.cpp ${SRC}/VulkanCommandBuffer.cpp ${SRC}/VulkanDescriptorSetLayout.cpp ${SRC}/VulkanDescriptorSet.cpp ${SRC}/VulkanVertexArrayObject.cpp)
set(Vk_OBJ_HEADERS ${INC}/VulkanUtils.hpp ${INC}/VulkanFormat.hpp ${INC}/VulkanSampler.hpp ${INC}/VulkanFramebuffer.hpp ${INC}/VulkanComputePipeline.hpp ${INC}/VulkanGraphicsPipeline.hpp ${INC}/VulkanImage.hpp ${INC}/VulkanRenderPass.hpp ${INC}/VulkanBuffer.hpp ${INC}/VulkanCommandBuffer.hpp ${INC}/VulkanDescriptorSetLayout.hpp ${INC}/VulkanDescriptorSet.hpp ${INC}/VulkanVertexArrayObject.hpp)
set(Vk_WINDOW_SOURCES ${COMMON_DIR}/Window/WindowManager.cpp)
set(Vk_WINDOW_HEADERS ${COMMON_DIR}/Window/WindowManager.hpp ${COMMON_DIR}/Window/Window.hpp)
set(Vk_DISPLAY_SOURCES ${COMMON_DIR}/Display/DisplayManager.cpp)
set(Vk_DISPLAY_HEADERS ${COMMON_DIR}/Display/DisplayManager.hpp ${COMMON_DIR}/Display/Display.hpp)
set(Vk_COMMON_HEADERS ${GRAPHICS_DIR}/Sampler.hpp ${GRAPHICS_DIR}/CommandBuffer.hpp ${GRAPHICS_DIR}/Formats.cpp ${GRAPHICS_DIR}/Formats.hpp ${GRAPHICS_DIR}/VertexArrayObject.hpp ${GRAPHICS_DIR}/Framebuffer.hpp ${GRAPHICS_DIR}/GraphicsPipeline.hpp ${GRAPHICS_DIR}/ComputePipeline.hpp ${GRAPHICS_DIR}/Core.hpp ${GRAPHICS_DIR}/RenderPass.hpp ${GRAPHICS_DIR}/Image.hpp ${GRAPHICS_DIR}/Buffer.hpp ${GRAPHICS_DIR}/WindowGraphicsBinding.hpp)

set(Vk_OBJ_SOURCES ${Vk_OBJ_SOURCES} ${SRC}/VulkanWindowGraphicsBinding.cpp)
set(Vk_OBJ_HEADERS ${Vk_OBJ_HEADERS} ${INC}/VulkanWindowGraphicsBinding.hpp)

#if (WIN32)
#    set(Vk_WINDOW_SOURCES ${Vk_WINDOW_SOURCES} ${COMMON_DIR}/Window/Win32Window.cpp)
#    set(Vk_WINDOW_HEADERS ${Vk_WINDOW_HEADERS} ${COMMON_DIR}/Window/Win32Window.hpp)
#elseif (UNIX)
#    set(Vk_WINDOW_SOURCES ${Vk_WINDOW_SOURCES} ${COMMON_DIR}/Window/X11Window.cpp)
#    set(Vk_WINDOW_HEADERS ${Vk_WINDOW_HEADERS} ${COMMON_DIR}/Window/X11Window.hpp)
#endif()

set(Vk_WINDOW_SOURCES ${Vk_WINDOW_SOURCES} ${COMMON_DIR}/Window/GlfwWindow.cpp)
set(Vk_WINDOW_HEADERS ${Vk_WINDOW_HEADERS} ${COMMON_DIR}/Window/GlfwWindow.hpp)

set(Vk_SOURCES ${Vk_CORE_SOURCES} ${Vk_OBJ_SOURCES} ${Vk_WINDOW_SOURCES} ${Vk_DISPLAY_SOURCES})
set(Vk_HEADERS ${Vk_CORE_HEADERS} ${Vk_OBJ_HEADERS} ${Vk_WINDOW_HEADERS} ${Vk_DISPLAY_HEADERS} ${Vk_COMMON_HEADERS})

source_group("Header Files\\Common Objects" FILES ${Vk_COMMON_HEADERS})

source_group("Source Files\\Objects" FILES ${Vk_OBJ_SOURCES})
source_group("Header Files\\Objects" FILES ${Vk_OBJ_HEADERS})

source_group("Source Files\\Window" FILES ${Vk_WINDOW_SOURCES})
source_group("Header Files\\Window" FILES ${Vk_WINDOW_HEADERS})

source_group("Source Files\\Display" FILES ${Vk_DISPLAY_SOURCES})
source_group("Header Files\\Display" FILES ${Vk_DISPLAY_HEADERS})

add_library(PluginRhiVulkan MODULE ${Vk_SOURCES} ${Vk_HEADERS} ${CORE_UTILS})
set_property(TARGET PluginRhiVulkan PROPERTY COMPILE_WARNING_AS_ERROR ON)
set_target_properties(PluginRhiVulkan PROPERTIES FOLDER "Plugins/Render Hardware Interface")

find_path(VULKAN_HEADERS_INCLUDE_DIRS "vk_video/vulkan_video_codec_h264std.h")
find_package(Vulkan REQUIRED)
find_package(VulkanMemoryAllocator CONFIG REQUIRED)

target_include_directories(PluginRhiVulkan PUBLIC ${CODE_DIR} ${DEPS_DIR} ${VULKAN_HEADERS_INCLUDE_DIRS} ${PLUGIN_DIR})

target_compile_definitions(PluginRhiVulkan PRIVATE GRAPHICS_DLL GLM_ENABLE_EXPERIMENTAL)

find_package(glfw3 CONFIG REQUIRED)

target_link_libraries(PluginRhiVulkan ${CORE_LIBS} Vulkan::Vulkan glfw fmt::fmt GPUOpen::VulkanMemoryAllocator)

target_compile_features(PluginRhiVulkan PRIVATE cxx_std_20)

if (WIN32)
    target_compile_definitions(PluginRhiVulkan PRIVATE Vk_USE_PLATFORM_WIN32_KHR)
else()
    SET(Vk_PLATFORM "Vk_USE_PLATFORM_WIN32_KHR" CACHE STRING "Vulkan platform to use")
    SET_PROPERTY(CACHE Vk_PLATFORM PROPERTY STRINGS Vk_USE_PLATFORM_WIN32_KHR Vk_KHR_XCB_SURFACE_EXTENSION_NAME  Vk_USE_PLATFORM_XLIB_KHR Vk_USE_PLATFORM_ANDROID_KHR)

    target_compile_definitions(PluginRhiVulkan PRIVATE ${Vk_PLATFORM})
endif()

set_target_properties(PluginRhiVulkan PROPERTIES
	RUNTIME_OUTPUT_DIRECTORY "${RHI_VULKAN_BIN}"
	LIBRARY_OUTPUT_DIRECTORY "${RHI_VULKAN_LIB}"
	ARCHIVE_OUTPUT_DIRECTORY "${RHI_VULKAN_LIB}"
)
target_compile_definitions(PluginRhiVulkan PRIVATE GRAPHICS_VULKAN GRAPHICS_DLL)

target_precompile_headers(PluginRhiVulkan PUBLIC ${INC}/pch.hpp)

message(STATUS
"Plugin RhiVulkan provides CMake targets:

- RHI_VULKAN_BASE => ${RHI_VULKAN_BASE}
- RHI_VULKAN_INCLUDES => ${RHI_VULKAN_INCLUDES}
- RHI_VULKAN_BIN => ${RHI_VULKAN_BIN}
- RHI_VULKAN_LIB => ${RHI_VULKAN_LIB}


")
