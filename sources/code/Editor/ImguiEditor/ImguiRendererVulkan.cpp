#include <iostream>
#include <imgui.h>
#include <imgui_internal.h>
#include <imgui_impl_vulkan.h>
#include <imgui_impl_glfw.h>
#include <GLFW/glfw3.h>

#include <Common/Window/WindowManager.hpp>
#include <Common/Window/GlfwWindow.hpp>
#include <Editor/EditorManager.hpp>
#include <Common/Logger.hpp>
#include <EngineCore/Assets/AssetManager.hpp>
#include <EngineCore/Assets/Textures/TextureAsset.hpp>
#include <Grindstone.RHI.Vulkan/include/VulkanCore.hpp>
#include <Grindstone.RHI.Vulkan/include/VulkanRenderPass.hpp>
#include <Grindstone.RHI.Vulkan/include/VulkanCommandBuffer.hpp>
#include <Grindstone.RHI.Vulkan/include/VulkanDescriptorSet.hpp>
#include <Grindstone.RHI.Vulkan/include/VulkanWindowGraphicsBinding.hpp>

#include "ImguiRendererVulkan.hpp"

using namespace Grindstone::GraphicsAPI;
using namespace Grindstone::Editor::ImguiEditor;

static ImGui_ImplVulkanH_Window g_MainWindowData;

void ImguiRendererVulkan::SetupVulkanWindow(
	Grindstone::GraphicsAPI::Vulkan::Core* graphicsCore,
	Grindstone::GraphicsAPI::WindowGraphicsBinding* wgb,
	int width,
	int height
) {
	ImGui_ImplVulkanH_Window* wd = &g_MainWindowData;
	Grindstone::GraphicsAPI::Vulkan::WindowGraphicsBinding* vwgb = static_cast<Grindstone::GraphicsAPI::Vulkan::WindowGraphicsBinding*>(wgb);
	VkSurfaceKHR surface = vwgb->GetSurface();
	wd->Surface = surface;

	VkPhysicalDevice physicalDevice = graphicsCore->GetPhysicalDevice();
	uint32_t graphicsFamily = graphicsCore->GetGraphicsFamily();

	// Check for WSI support
	VkBool32 res;
	vkGetPhysicalDeviceSurfaceSupportKHR(physicalDevice, graphicsFamily, wd->Surface, &res);
	if (res != VK_TRUE)
	{
		fprintf(stderr, "Error no WSI support on physical device 0\n");
		exit(-1);
	}

	// Select Surface Format
	const VkFormat requestSurfaceImageFormat[] = { VK_FORMAT_B8G8R8A8_UNORM, VK_FORMAT_R8G8B8A8_UNORM, VK_FORMAT_B8G8R8_UNORM, VK_FORMAT_R8G8B8_UNORM };
	const VkColorSpaceKHR requestSurfaceColorSpace = VK_COLORSPACE_SRGB_NONLINEAR_KHR;
	wd->SurfaceFormat = ImGui_ImplVulkanH_SelectSurfaceFormat(physicalDevice, wd->Surface, requestSurfaceImageFormat, (size_t)IM_ARRAYSIZE(requestSurfaceImageFormat), requestSurfaceColorSpace);

	// Select Present Mode
#ifdef IMGUI_UNLIMITED_FRAME_RATE
	VkPresentModeKHR presentModes[] = { VK_PRESENT_MODE_MAILBOX_KHR, VK_PRESENT_MODE_IMMEDIATE_KHR, VK_PRESENT_MODE_FIFO_KHR };
#else
	VkPresentModeKHR presentModes[] = { VK_PRESENT_MODE_FIFO_KHR };
#endif
	wd->PresentMode = ImGui_ImplVulkanH_SelectPresentMode(physicalDevice, wd->Surface, &presentModes[0], IM_ARRAYSIZE(presentModes));
	//printf("[vulkan] Selected PresentMode = %d\n", wd->PresentMode);

	VkInstance instance = graphicsCore->GetInstance();
	VkDevice device = graphicsCore->GetDevice();

	// Create SwapChain, RenderPass, Framebuffer, etc.
	int minImageCount = wgb->GetMaxFramesInFlight();
	IM_ASSERT(minImageCount >= 2);
	wd->Swapchain = vwgb->GetSwapchain();
	CreateOrResizeWindow(graphicsCore, vwgb, width, height);
}

ImguiRendererVulkan::ImguiRendererVulkan() {
	Grindstone::EngineCore& engineCore = Grindstone::Editor::Manager::GetEngineCore();
	auto graphicsCore = engineCore.GetGraphicsCore();

	Grindstone::Window* window = engineCore.windowManager->GetWindowByIndex(0);

	unsigned int width, height;
	window->GetWindowSize(width, height);

	auto wgb = window->GetWindowGraphicsBinding();

	ImGui_ImplVulkanH_Window* wd = &g_MainWindowData;
	SetupVulkanWindow(static_cast<GraphicsAPI::Vulkan::Core*>(graphicsCore), wgb, width, height);

	VkDescriptorPoolSize poolSizes[] = {
		{ VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1 },
		{ VK_DESCRIPTOR_TYPE_SAMPLER, 1 },
		{ VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, 64 }
	};

	VkDescriptorPoolCreateInfo poolInfo = {};
	poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
	poolInfo.flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT;
	poolInfo.maxSets = 64;
	poolInfo.poolSizeCount = static_cast<uint32_t>(std::size(poolSizes));
	poolInfo.pPoolSizes = poolSizes;

	auto vulkanCore = static_cast<Grindstone::GraphicsAPI::Vulkan::Core*>(graphicsCore);

	if (vkCreateDescriptorPool(vulkanCore->GetDevice(), &poolInfo, nullptr, &imguiPool) != VK_SUCCESS) {
		throw std::runtime_error("failed to allocate imgui descriptor pool!");
	}

	vulkanCore->NameObject(VK_OBJECT_TYPE_DESCRIPTOR_POOL, imguiPool, "IMGUI Descriptor Pool");

	ImGui_ImplGlfw_InitForVulkan(static_cast<Grindstone::GlfwWindow*>(window)->GetHandle(), true);
	auto vulkanRenderPass = static_cast<Grindstone::GraphicsAPI::Vulkan::RenderPass*>(wgb->GetRenderPass());

	VkFormat colorFormat = static_cast<Grindstone::GraphicsAPI::Vulkan::WindowGraphicsBinding*>(wgb)->GetSwapchainVulkanFormat();

	ImGui_ImplVulkan_InitInfo imguiInitInfo{
		.Instance = vulkanCore->GetInstance(),
		.PhysicalDevice = vulkanCore->GetPhysicalDevice(),
		.Device = vulkanCore->GetDevice(),
		.Queue = vulkanCore->graphicsQueue,
		.DescriptorPool = imguiPool,
		.MinImageCount = 3,
		.ImageCount = 3,
		.PipelineInfoMain = ImGui_ImplVulkan_PipelineInfo{
			.PipelineRenderingCreateInfo = VkPipelineRenderingCreateInfoKHR{
				.sType = VkStructureType::VK_STRUCTURE_TYPE_PIPELINE_RENDERING_CREATE_INFO,
				.colorAttachmentCount = 1,
				.pColorAttachmentFormats = &colorFormat,
				.depthAttachmentFormat = VkFormat::VK_FORMAT_UNDEFINED,
				.stencilAttachmentFormat = VkFormat::VK_FORMAT_UNDEFINED
			}
		},
		.PipelineInfoForViewports = {

		},
		.UseDynamicRendering = true
	};

	ImGui_ImplVulkan_Init(&imguiInitInfo);

	Grindstone::GraphicsAPI::CommandBuffer::CreateInfo commandBufferCreateInfo{};

	commandBuffers.resize(wgb->GetMaxFramesInFlight());
	for (size_t i = 0; i < commandBuffers.size(); ++i) {
		std::string debugName = "Editor Command Buffer " + std::to_string(i);
		commandBufferCreateInfo.debugName = debugName.c_str();
		commandBuffers[i] = graphicsCore->CreateCommandBuffer(commandBufferCreateInfo);
	}

	VkCommandBuffer commandBuffer = vulkanCore->BeginSingleTimeCommands("Imgui::CreateFontsTexture");
	vulkanCore->EndSingleTimeCommands(commandBuffer);

	GraphicsAPI::DescriptorSetLayout::Binding layoutBinding{};
	layoutBinding.bindingId = 0;
	layoutBinding.type = BindingType::CombinedImageSampler;
	layoutBinding.count = 1;
	layoutBinding.stages = ShaderStageBit::Fragment;

	GraphicsAPI::DescriptorSetLayout::CreateInfo descriptorSetLayoutCreateInfo{};
	descriptorSetLayoutCreateInfo.debugName = "Imgui Texture Layout";
	descriptorSetLayoutCreateInfo.bindingCount = 1;
	descriptorSetLayoutCreateInfo.bindings = &layoutBinding;
	textureDescriptorLayout = Editor::Manager::GetEngineCore().GetGraphicsCore()->GetOrCreateDescriptorSetLayoutFromCache(descriptorSetLayoutCreateInfo);

}

ImguiRendererVulkan::~ImguiRendererVulkan() {
	Grindstone::EngineCore& engineCore = Grindstone::Editor::Manager::GetEngineCore();
	GraphicsAPI::Vulkan::Core* graphicsCore = static_cast<Vulkan::Core*>(engineCore.GetGraphicsCore());

	graphicsCore->DeleteDescriptorSetLayout(textureDescriptorLayout);

	VkDevice device = graphicsCore->GetDevice();
	vkDestroyDescriptorPool(device, imguiPool, nullptr);

	for (GraphicsAPI::CommandBuffer* cb : commandBuffers) {
		graphicsCore->DeleteCommandBuffer(cb);
	}
}

Grindstone::GraphicsAPI::CommandBuffer* ImguiRendererVulkan::GetCommandBuffer() {
	Grindstone::EngineCore& engineCore = Grindstone::Editor::Manager::GetEngineCore();
	Grindstone::GraphicsAPI::WindowGraphicsBinding* window = engineCore.windowManager->GetWindowByIndex(0)->GetWindowGraphicsBinding();
	return commandBuffers[window->GetCurrentImageIndex()];
}

bool ImguiRendererVulkan::PreRender() {
	Grindstone::EngineCore& engineCore = Grindstone::Editor::Manager::GetEngineCore();
	Grindstone::GraphicsAPI::WindowGraphicsBinding* window = engineCore.windowManager->GetWindowByIndex(0)->GetWindowGraphicsBinding();
	
	if (!window->AcquireNextImage()) {
		WaitForResizeAndRecreateSwapchain();
		return false;
	}

	Grindstone::GraphicsAPI::CommandBuffer* currentCommandBuffer = commandBuffers[window->GetCurrentImageIndex()];

	currentCommandBuffer->BeginCommandBuffer();
	return true;
}

void ImguiRendererVulkan::WaitForResizeAndRecreateSwapchain() {
	Grindstone::EngineCore& engineCore = Grindstone::Editor::Manager::GetEngineCore();
	auto vulkanCore = static_cast<Grindstone::GraphicsAPI::Vulkan::Core*>(engineCore.GetGraphicsCore());
	auto window = static_cast<GlfwWindow*>(engineCore.windowManager->GetWindowByIndex(0));
	GLFWwindow* winHandle = window->GetHandle();
	int width = 0, height = 0;
	glfwGetFramebufferSize(winHandle, &width, &height);
	while (width == 0 || height == 0) {
		glfwGetFramebufferSize(winHandle, &width, &height);
		glfwWaitEvents();
	}

	CreateOrResizeWindow(vulkanCore, window->GetWindowGraphicsBinding(), width, height);
}

void ImguiRendererVulkan::PrepareImguiRendering() {
	Grindstone::EngineCore& engineCore = Grindstone::Editor::Manager::GetEngineCore();
	GraphicsAPI::WindowGraphicsBinding* window = engineCore.windowManager->GetWindowByIndex(0)->GetWindowGraphicsBinding();
	GraphicsAPI::RenderPass* renderPass = window->GetRenderPass();
	GraphicsAPI::Framebuffer* framebuffer = window->GetCurrentFramebuffer();
	GraphicsAPI::CommandBuffer* currentCommandBuffer = commandBuffers[window->GetCurrentImageIndex()];

	GraphicsAPI::ClearColor clearColor = { 0.0f, 0.0f, 0.0f, 0.0f };

	Grindstone::Math::IntRect2D viewport(0, 0, framebuffer->GetWidth(), framebuffer->GetHeight());

	Grindstone::GraphicsAPI::Image* image = static_cast<GraphicsAPI::Vulkan::WindowGraphicsBinding*>(window)->GetCurrentSwapchainImage();

	GraphicsAPI::ImageBarrier outputImageBarrier{
		.image = image,
		.srcStageMask = GraphicsAPI::PipelineStageBit::TopOfPipe,
		.dstStageMask = GraphicsAPI::PipelineStageBit::ColorAttachmentOutput,
		.oldLayout = Grindstone::GraphicsAPI::ImageLayout::Undefined,
		.newLayout = Grindstone::GraphicsAPI::ImageLayout::ColorAttachment,
		.srcAccess = Grindstone::GraphicsAPI::AccessFlags::None,
		.dstAccess = Grindstone::GraphicsAPI::AccessFlags::ColorAttachmentWrite,
		.imageAspect = GraphicsAPI::ImageAspectBits::Color,
		.baseMipLevel = 0,
		.levelCount = 1,
		.baseArrayLayer = 0,
		.layerCount = 1
	};

	currentCommandBuffer->PipelineBarrier(
		nullptr, 0,
		&outputImageBarrier, 1u
	);

	Grindstone::GraphicsAPI::RenderAttachment colorAttachment{
		.image = image,
		.imageLayout = Grindstone::GraphicsAPI::ImageLayout::ColorAttachment,
		.clearValue = clearColor,
	};

	currentCommandBuffer->BeginRendering(
		"ImGui Pass",
		viewport,
		&colorAttachment,
		1u
	);

	ImGui_ImplVulkan_NewFrame();
	ImGui_ImplGlfw_NewFrame();
	ImGui::NewFrame();
}

void ImguiRendererVulkan::PostRender() {
	Grindstone::EngineCore& engineCore = Grindstone::Editor::Manager::GetEngineCore();
	auto window = engineCore.windowManager->GetWindowByIndex(0)->GetWindowGraphicsBinding();
	auto currentCommandBuffer = commandBuffers[window->GetCurrentImageIndex()];
	ImGui::Render();


	ImDrawData* imDrawData = ImGui::GetDrawData();
	if (imDrawData->Textures != nullptr) {
		for (ImTextureData* tex : *imDrawData->Textures) {
			if (tex->Status != ImTextureStatus_OK) {
				ImGui_ImplVulkan_UpdateTexture(tex);
			}
		}
	}

	auto vkCB = static_cast<GraphicsAPI::Vulkan::CommandBuffer*>(currentCommandBuffer);
	ImGui_ImplVulkan_RenderDrawData(ImGui::GetDrawData(), vkCB->GetCommandBuffer());

	ImGui::UpdatePlatformWindows();
	ImGui::RenderPlatformWindowsDefault();
	currentCommandBuffer->EndRendering();

	uint32_t swapchainImageIndex = window->GetCurrentSwapchainIndex();
	Grindstone::GraphicsAPI::Image* image = static_cast<GraphicsAPI::Vulkan::WindowGraphicsBinding*>(window)->GetCurrentSwapchainImage();

	GraphicsAPI::ImageBarrier presentImageBarrier{
		.image = image,
		.srcStageMask = GraphicsAPI::PipelineStageBit::ColorAttachmentOutput,
		.dstStageMask = GraphicsAPI::PipelineStageBit::BottomOfPipe,
		.oldLayout = Grindstone::GraphicsAPI::ImageLayout::ColorAttachment,
		.newLayout = Grindstone::GraphicsAPI::ImageLayout::Present,
		.srcAccess = Grindstone::GraphicsAPI::AccessFlags::ColorAttachmentWrite,
		.dstAccess = Grindstone::GraphicsAPI::AccessFlags::None,
		.imageAspect = GraphicsAPI::ImageAspectBits::Color,
		.baseMipLevel = 0,
		.levelCount = 1,
		.baseArrayLayer = 0,
		.layerCount = 1
	};

	currentCommandBuffer->PipelineBarrier(
		nullptr, 0,
		&presentImageBarrier, 1u
	);


	currentCommandBuffer->EndCommandBuffer();
	window->SubmitCommandBufferForCurrentFrame(currentCommandBuffer);
	if (!window->PresentSwapchain() || shouldRebuildSwapchain) {
		WaitForResizeAndRecreateSwapchain();
		shouldRebuildSwapchain = false;
	}
}

void ImguiRendererVulkan::Resize() {
	shouldRebuildSwapchain = true;
}

ImTextureID ImguiRendererVulkan::CreateTexture(std::filesystem::path path) {
	EngineCore& engineCore = Editor::Manager::GetEngineCore();
	auto assetManager = engineCore.assetManager;
	std::string assetAddress = "@EDITOR_ICONS/" + path.string();
	Grindstone::Uuid uuid = assetManager->GetUuidByAddress(Grindstone::AssetType::Texture, assetAddress);
	if (!uuid.IsValid()) {
		GPRINT_ERROR(LogSource::Editor, "Could not find texture uuid for {}.", assetAddress);
		return 0;
	}

	Grindstone::TextureAsset* textureAsset = static_cast<TextureAsset*>(assetManager->GetAndIncrementAssetCount(Grindstone::AssetType::Texture, uuid));
	if (textureAsset == nullptr) {
		return 0;
	}

	auto vkImage = static_cast<Grindstone::GraphicsAPI::Vulkan::Image*>(textureAsset->image);

	VkDescriptorSet descriptorSet = ImGui_ImplVulkan_AddTexture(vkImage->GetImageView(), VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
	return (ImTextureID)descriptorSet;
}

ImTextureID ImguiRendererVulkan::GetThumbnailAtlas() {
	Editor::Manager& editorManager = Editor::Manager::GetInstance();
	Grindstone::Editor::ThumbnailManager& thumbnailManager = editorManager.GetThumbnailManager();
	Grindstone::GraphicsAPI::Image* image = thumbnailManager.GetThumbnailImage();
	auto vkImage = static_cast<Grindstone::GraphicsAPI::Vulkan::Image*>(image);
	VkDescriptorSet descriptorSet = ImGui_ImplVulkan_AddTexture(vkImage->GetImageView(), VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
	
	return (ImTextureID)descriptorSet;
}

void ImguiRendererVulkan::CreateOrResizeWindow(
	Grindstone::GraphicsAPI::Vulkan::Core* graphicsCore,
	Grindstone::GraphicsAPI::WindowGraphicsBinding* wgb,
	int width, int height
) {
	VkInstance instance = graphicsCore->GetInstance();
	VkPhysicalDevice physicalDevice = graphicsCore->GetPhysicalDevice();
	VkDevice device = graphicsCore->GetDevice();
	uint32_t graphicsFamily = graphicsCore->GetGraphicsFamily();

	ImGui_ImplVulkanH_CreateOrResizeWindow(instance, physicalDevice, device, &g_MainWindowData, graphicsFamily, nullptr, width, height, wgb->GetMaxFramesInFlight(), VkImageUsageFlagBits::VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT);

	// Send new imgui window data to window
	std::vector<Vulkan::ImageSetNative> imguiImageSets;
	imguiImageSets.resize(g_MainWindowData.ImageCount);
	for (uint32_t i = 0; i < g_MainWindowData.ImageCount; ++i) {
		auto& imageSet = imguiImageSets[i];
		ImGui_ImplVulkanH_Frame& imguiFrame = g_MainWindowData.Frames[i];

		imageSet.image = imguiFrame.Backbuffer;
		imageSet.imageView = imguiFrame.BackbufferView;
		imageSet.framebuffer = imguiFrame.Framebuffer;
	}

	Vulkan::WindowBindingDataNative windowBindingData{};
	windowBindingData.swapChain = g_MainWindowData.Swapchain;
	windowBindingData.renderPass = g_MainWindowData.RenderPass;
	windowBindingData.width = static_cast<uint32_t>(g_MainWindowData.Width);
	windowBindingData.height = static_cast<uint32_t>(g_MainWindowData.Height);
	windowBindingData.surfaceFormat = g_MainWindowData.SurfaceFormat;
	windowBindingData.imageSetCount = g_MainWindowData.ImageCount;
	windowBindingData.imageSets = imguiImageSets.data();

	static_cast<Vulkan::WindowGraphicsBinding*>(wgb)->SubmitWindowObjects(windowBindingData);
}
