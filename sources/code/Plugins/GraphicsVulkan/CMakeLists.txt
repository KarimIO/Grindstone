set(GRAPHICS_DIR "${COMMON_DIR}/Graphics")

set(Vk_CORE_SOURCES ${GRAPHICS_DIR}/Formats.cpp VulkanCore.cpp EntryPoint.cpp)
set(Vk_CORE_HEADERS VulkanCore.hpp)
set(Vk_OBJ_SOURCES VulkanUtils.cpp VulkanFormat.cpp VulkanDepthTarget.cpp VulkanRenderTarget.cpp VulkanFramebuffer.cpp VulkanPipeline.cpp VulkanTexture.cpp VulkanRenderPass.cpp VulkanVertexBuffer.cpp VulkanIndexBuffer.cpp VulkanUniformBuffer.cpp VulkanCommandBuffer.cpp)
set(Vk_OBJ_HEADERS VulkanUtils.hpp VulkanFormat.hpp VulkanDepthTarget.hpp VulkanRenderTarget.hpp VulkanFramebuffer.hpp VulkanPipeline.hpp VulkanTexture.hpp VulkanRenderPass.hpp VulkanVertexBuffer.hpp VulkanIndexBuffer.hpp VulkanUniformBuffer.hpp VulkanCommandBuffer.hpp)
set(Vk_WINDOW_SOURCES ${COMMON_DIR}/Window/Window.cpp)
set(Vk_WINDOW_HEADERS ${COMMON_DIR}/Window/Window.hpp)
set(Vk_DISPLAY_SOURCES ${COMMON_DIR}/Display/Display.cpp)
set(Vk_DISPLAY_HEADERS ${COMMON_DIR}/Display/Display.hpp)
set(Vk_COMMON_HEADERS ${GRAPHICS_DIR}/DepthTarget.hpp ${GRAPHICS_DIR}/RenderTarget.hpp ${GRAPHICS_DIR}/CommandBuffer.hpp ${GRAPHICS_DIR}/Formats.hpp ${GRAPHICS_DIR}/Formats.cpp ${GRAPHICS_DIR}/VertexArrayObject.hpp ${GRAPHICS_DIR}/Framebuffer.hpp ${GRAPHICS_DIR}/Pipeline.hpp ${GRAPHICS_DIR}/Core.hpp ${GRAPHICS_DIR}/IndexBuffer.hpp ${GRAPHICS_DIR}/RenderPass.hpp ${GRAPHICS_DIR}/Texture.hpp ${GRAPHICS_DIR}/UniformBuffer.hpp ${GRAPHICS_DIR}/VertexBuffer.hpp ${GRAPHICS_DIR}/WindowGraphicsBinding.hpp)

set(Vk_OBJ_SOURCES ${Vk_OBJ_SOURCES} VulkanWindowGraphicsBinding.cpp)
set(Vk_OBJ_HEADERS ${Vk_OBJ_HEADERS} VulkanWindowGraphicsBinding.hpp)

if (WIN32)
    set(Vk_WINDOW_SOURCES ${Vk_WINDOW_SOURCES} ${COMMON_DIR}/Window/Win32Window.cpp)
    set(Vk_WINDOW_HEADERS ${Vk_WINDOW_HEADERS} ${COMMON_DIR}/Window/Win32Window.hpp)
elseif (UNIX)
    set(Vk_WINDOW_SOURCES ${Vk_WINDOW_SOURCES} ${COMMON_DIR}/Window/X11Window.cpp)
    set(Vk_WINDOW_HEADERS ${Vk_WINDOW_HEADERS} ${COMMON_DIR}/Window/X11Window.hpp)
endif()

set(Vk_SOURCES ${Vk_CORE_SOURCES} ${Vk_OBJ_SOURCES} ${Vk_WINDOW_SOURCES} ${Vk_DISPLAY_SOURCES})
set(Vk_HEADERS ${Vk_CORE_HEADERS} ${Vk_OBJ_HEADERS} ${Vk_WINDOW_HEADERS} ${Vk_DISPLAY_HEADERS} ${Vk_COMMON_HEADERS})

source_group("Header Files\\Common Objects" FILES ${Vk_COMMON_HEADERS})

source_group("Source Files\\Objects" FILES ${Vk_OBJ_SOURCES})
source_group("Header Files\\Objects" FILES ${Vk_OBJ_HEADERS})

source_group("Source Files\\Window" FILES ${Vk_WINDOW_SOURCES})
source_group("Header Files\\Window" FILES ${Vk_WINDOW_HEADERS})

source_group("Source Files\\Display" FILES ${Vk_DISPLAY_SOURCES})
source_group("Header Files\\Display" FILES ${Vk_DISPLAY_HEADERS})

add_library(PluginGraphicsVulkan MODULE ${Vk_SOURCES} ${Vk_HEADERS})
    
find_package(Vulkan) 

target_include_directories(PluginGraphicsVulkan PUBLIC ${CODE_DIR} ${DEPS_DIR} ${Vulkan_INCLUDE_DIR})

target_compile_definitions(PluginGraphicsVulkan PRIVATE GRAPHICS_DLL)

target_link_libraries(PluginGraphicsVulkan ${Vulkan_LIBRARIES})

target_compile_features(PluginGraphicsVulkan PRIVATE cxx_std_17)

if (WIN32)
    target_compile_definitions(PluginGraphicsVulkan PRIVATE Vk_USE_PLATFORM_WIN32_KHR)
else()
    SET(Vk_PLATFORM "Vk_USE_PLATFORM_WIN32_KHR" CACHE STRING "Vulkan platform to use")
    SET_PROPERTY(CACHE Vk_PLATFORM PROPERTY STRINGS Vk_USE_PLATFORM_WIN32_KHR Vk_KHR_XCB_SURFACE_EXTENSION_NAME  Vk_USE_PLATFORM_XLIB_KHR Vk_USE_PLATFORM_ANDROID_KHR) 

    target_compile_definitions(PluginGraphicsVulkan PRIVATE ${Vk_PLATFORM})
endif()

target_compile_definitions(PluginGraphicsVulkan PRIVATE GRAPHICS_VULKAN GRAPHICS_DLL)

target_precompile_headers(PluginGraphicsVulkan
    PUBLIC pch.hpp)

##
