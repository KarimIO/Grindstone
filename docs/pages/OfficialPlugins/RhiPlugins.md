Render-Hardware Interface Plugins {#RhiPlugins}
============

Render-Hardware Interfaces add wrappers around APIs, which allow for communication with the GPU.

 - @subpage Plugin_Grindstone_RHI_DirectX12
 - @subpage Plugin_Grindstone_RHI_OpenGL
 - @subpage Plugin_Grindstone_RHI_Vulkan

RHIs makes the engine portable and extensible for different platforms and GPU backends. They implement abstract classes including:

 - [Graphics Core](@ref Grindstone::GraphicsAPI::Core)
 - [Buffer](@ref Grindstone::GraphicsAPI::Buffer)
 - [Command Buffer](@ref Grindstone::GraphicsAPI::CommandBuffer)
 - [Descriptor Set](@ref Grindstone::GraphicsAPI::DescriptorSet)
 - [Descriptor Set Layout](@ref Grindstone::GraphicsAPI::DescriptorSetLayout)
 - [Framebuffer](@ref Grindstone::GraphicsAPI::Framebuffer)
 - [Graphics Pipeline](@ref Grindstone::GraphicsAPI::GraphicsPipeline)
 - [Compute Pipeline](@ref Grindstone::GraphicsAPI::ComputePipeline)
 - [Image](@ref Grindstone::GraphicsAPI::Image)
 - [RenderPass](@ref Grindstone::GraphicsAPI::RenderPass)
 - [Sampler](@ref Grindstone::GraphicsAPI::Sampler)
 - [Vertex Array Object](@ref Grindstone::GraphicsAPI::VertexArrayObject)
 - [Window Graphics Binding](@ref Grindstone::GraphicsAPI::WindowGraphicsBinding)
