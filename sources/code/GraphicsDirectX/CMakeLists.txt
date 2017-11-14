set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(DX_SOURCES ../WindowModule/win32Window.cpp DxBufferCommon.cpp DxCommandBuffer.cpp DxDescriptors.cpp DxFramebuffer.hpp DxFramebuffer.cpp DxGraphicsPipeline.cpp DxGraphicsWrapper.cpp DxIndexBuffer.cpp DxRenderPass.cpp DxTexture.cpp DxUniformBuffer.cpp DxVertexArrayObject.cpp DxVertexBuffer.cpp DxDevice.cpp)
set(DX_HEADERS DxBufferCommon.hpp DxCommandBuffer.hpp DxDescriptors.hpp DxFramebuffer.hpp DxFramebuffer.hpp DxGraphicsPipeline.hpp DxGraphicsWrapper.hpp DxIndexBuffer.hpp DxRenderPass.hpp DxTexture.hpp DxUniformBuffer.hpp DxVertexArrayObject.hpp DxVertexBuffer.hpp)
set(DX_COMMON ../GraphicsCommon/CommandBuffer.hpp ../GraphicsCommon/Descriptors.hpp ../GraphicsCommon/Formats.hpp ../GraphicsCommon/Framebuffer.hpp ../GraphicsCommon/GraphicsPipeline.hpp ../GraphicsCommon/GraphicsWrapper.hpp ../GraphicsCommon/IndexBuffer.hpp ../GraphicsCommon/RenderPass.hpp ../GraphicsCommon/Texture.hpp ../GraphicsCommon/UniformBuffer.hpp ../GraphicsCommon/VertexArrayObject.hpp ../GraphicsCommon/VertexBuffer.hpp)

source_group("Common Files" FILES ${DX_COMMON})
source_group("Header Files" FILES ${DX_HEADERS})
source_group("Source Files" FILES ${DX_SOURCES})
add_library(graphicsdx MODULE ${DX_SOURCES} ${DX_HEADERS} ${DX_COMMON})

target_compile_definitions(graphicsdx PRIVATE GRAPHICS_DLL)

target_link_libraries(graphicsdx d3d11 dxgi d3dcompiler)