set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(VK_SOURCES VkBufferCommon.cpp VkCommandBuffer.cpp VkFormats.cpp VkFramebuffer.hpp VkFramebuffer.cpp VkGraphicsPipeline.cpp VkGraphicsWrapper.cpp VkIndexBuffer.cpp VkRenderPass.cpp VkTexture.cpp VkUniformBuffer.cpp VkVertexBuffer.cpp VkDevice.cpp)
set(VK_HEADERS VkBufferCommon.hpp VkCommandBuffer.hpp VkFormats.hpp VkFramebuffer.hpp VkFramebuffer.hpp VkGraphicsPipeline.hpp VkGraphicsWrapper.hpp VkIndexBuffer.hpp VkRenderPass.hpp VkTexture.hpp VkUniformBuffer.hpp VkVertexBuffer.hpp)
set(VK_COMMON ../GraphicsCommon/CommandBuffer.hpp ../GraphicsCommon/Descriptors.hpp ../GraphicsCommon/Formats.hpp ../GraphicsCommon/Framebuffer.hpp ../GraphicsCommon/GraphicsPipeline.hpp ../GraphicsCommon/GraphicsWrapper.hpp ../GraphicsCommon/IndexBuffer.hpp ../GraphicsCommon/RenderPass.hpp ../GraphicsCommon/Texture.hpp ../GraphicsCommon/UniformBuffer.hpp ../GraphicsCommon/VertexBuffer.hpp)

SET(VULKAN_DIR "D:/Libraries/Vulkan" CACHE PATH "Location of Vulkan Library")
include_directories(${VULKAN_DIR}/Include)
link_directories(${VULKAN_DIR}/Lib)
source_group("Common Files" FILES ${VK_COMMON})
source_group("Header Files" FILES ${VK_HEADERS})
source_group("Source Files" FILES ${VK_SOURCES})
add_library(graphicsvk MODULE ${VK_SOURCES} ${VK_HEADERS} ${VK_COMMON})

SET(VULKAN_DIR "D:/Libraries/Vulkan" CACHE PATH "Location of Vulkan Library")

if (WIN32)
    target_compile_definitions(graphicsvk PRIVATE VK_USE_PLATFORM_WIN32_KHR)
else()
    SET(VK_PLATFORM "VK_USE_PLATFORM_WIN32_KHR" CACHE STRING "Vulkan platform to use")
    SET_PROPERTY(CACHE VK_PLATFORM PROPERTY STRINGS VK_USE_PLATFORM_WIN32_KHR VK_KHR_XCB_SURFACE_EXTENSION_NAME  VK_USE_PLATFORM_XLIB_KHR VK_USE_PLATFORM_ANDROID_KHR) 

    target_compile_definitions(graphicsvk PRIVATE ${VK_PLATFORM})
endif()

target_compile_definitions(graphicsvk PRIVATE GRAPHICS_DLL)

target_link_libraries(graphicsvk vulkan-1)