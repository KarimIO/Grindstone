set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(DX_SOURCES ../WindowModule/win32Window.cpp DxBufferCommon.cpp DxCommandBuffer.cpp DxDescriptors.cpp DxFramebuffer.h DxFramebuffer.cpp DxGraphicsPipeline.cpp DxGraphicsWrapper.cpp DxIndexBuffer.cpp DxRenderPass.cpp DxTexture.cpp DxUniformBuffer.cpp DxVertexArrayObject.cpp DxVertexBuffer.cpp DxDevice.cpp)
set(DX_HEADERS DxBufferCommon.h DxCommandBuffer.h DxDescriptors.h DxFramebuffer.h DxFramebuffer.h DxGraphicsPipeline.h DxGraphicsWrapper.h DxIndexBuffer.h DxRenderPass.h DxTexture.h DxUniformBuffer.h DxVertexArrayObject.h DxVertexBuffer.h)
set(DX_COMMON ../GraphicsCommon/CommandBuffer.h ../GraphicsCommon/Descriptors.h ../GraphicsCommon/Formats.h ../GraphicsCommon/Framebuffer.h ../GraphicsCommon/GraphicsPipeline.h ../GraphicsCommon/GraphicsWrapper.h ../GraphicsCommon/IndexBuffer.h ../GraphicsCommon/RenderPass.h ../GraphicsCommon/Texture.h ../GraphicsCommon/UniformBuffer.h ../GraphicsCommon/VertexArrayObject.h ../GraphicsCommon/VertexBuffer.h)

source_group("Common Files" FILES ${DX_COMMON})
source_group("Header Files" FILES ${DX_HEADERS})
source_group("Source Files" FILES ${DX_SOURCES})
add_library(graphicsdx MODULE ${DX_SOURCES} ${DX_HEADERS} ${DX_COMMON})

target_compile_definitions(graphicsdx PRIVATE GRAPHICS_DLL)

target_link_libraries(graphicsdx d3d11 dxgi d3dcompiler)