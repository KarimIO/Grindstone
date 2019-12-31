set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(Vk_CORE_SOURCES vkGraphicsWrapper.cpp VkGraphicsWindows.cpp VkGraphicsX11.cpp ../WindowModule/glfwWindow.cpp)
set(Vk_CORE_HEADERS vkGraphicsWrapper.hpp)
set(Vk_OBJ_SOURCES VkDepthTarget.cpp VkRenderTarget.cpp VkFramebuffer.cpp VkGraphicsPipeline.cpp VkTexture.cpp VkRenderPass.cpp VkVertexBuffer.cpp VkIndexBuffer.cpp VkUniformBuffer.cpp)
set(Vk_OBJ_HEADERS VkDepthTarget.hpp VkRenderTarget.hpp VkFramebuffer.hpp VkGraphicsPipeline.hpp VkTexture.hpp VkRenderPass.hpp VkVertexBuffer.hpp VkIndexBuffer.hpp VkUniformBuffer.hpp)
set(Vk_COMMON_HEADERS ../GraphicsCommon/DepthTarget.hpp ../GraphicsCommon/RenderTarget.hpp ../GraphicsCommon/CommandBuffer.hpp ../GraphicsCommon/Formats.hpp ../GraphicsCommon/VertexArrayObject.hpp ../GraphicsCommon/Framebuffer.hpp ../GraphicsCommon/GraphicsPipeline.hpp ../GraphicsCommon/GraphicsWrapper.hpp ../GraphicsCommon/IndexBuffer.hpp ../GraphicsCommon/RenderPass.hpp ../GraphicsCommon/Texture.hpp ../GraphicsCommon/UniformBuffer.hpp ../GraphicsCommon/VertexBuffer.hpp)

if (WIN32)
    set(Vk_SOURCES_WINDOW ../WindowModule/win32Window.cpp)
else(WIN32)
    set(Vk_SOURCES_WINDOW ../WindowModule/x11Window.cpp)
endif(WIN32)

set(Vk_SOURCES ${Vk_CORE_SOURCES} ${Vk_OBJ_SOURCES} ${Vk_SOURCES_WINDOW})
set(Vk_HEADERS ${Vk_CORE_HEADERS} ${Vk_OBJ_HEADERS} ${Vk_COMMON_HEADERS})

source_group("Header Files\\Common Objects" FILES ${Vk_COMMON_HEADERS})

source_group("Source Files\\Objects" FILES ${ENGINE_CORE_SOURCES})
source_group("Header Files\\Objects" FILES ${ENGINE_CORE_SOURCES})

add_library(graphicsvk MODULE ${Vk_SOURCES} ${Vk_HEADERS})

target_include_directories(graphicsvk PUBLIC ${VkM_DIR} ${DEPS_DIR} ${Vulkan_INCLUDE_DIR})

target_compile_definitions(graphicsvk PRIVATE GRAPHICS_DLL)

target_link_libraries(graphicsvk ${Vulkan_LIBRARIES})

if (WIN32)
    target_compile_definitions(graphicsvk PRIVATE Vk_USE_PLATFORM_WIN32_KHR)
else()
    SET(Vk_PLATFORM "Vk_USE_PLATFORM_WIN32_KHR" CACHE STRING "Vulkan platform to use")
    SET_PROPERTY(CACHE Vk_PLATFORM PROPERTY STRINGS Vk_USE_PLATFORM_WIN32_KHR Vk_KHR_XCB_SURFACE_EXTENSION_NAME  Vk_USE_PLATFORM_XLIB_KHR Vk_USE_PLATFORM_ANDROID_KHR) 

    target_compile_definitions(graphicsvk PRIVATE ${Vk_PLATFORM})
endif()

target_compile_definitions(graphicsvk PRIVATE GRAPHICS_DLL)

target_link_libraries(graphicsvk vulkan-1)