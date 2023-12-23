#include <iostream>
#include <imgui.h>
#include <imgui_internal.h>
#include <imgui_impl_vulkan.h>
#include <imgui_impl_glfw.h>
#include <GLFW/glfw3.h>

#include <Common/Window/WindowManager.hpp>
#include <Common/Window/GlfwWindow.hpp>
#include <Editor/EditorManager.hpp>
#include <EngineCore/Assets/AssetManager.hpp>
#include <EngineCore/Assets/Textures/TextureAsset.hpp>
#include <Plugins/GraphicsVulkan/VulkanCore.hpp>
#include <Plugins/GraphicsVulkan/VulkanRenderPass.hpp>
#include <Plugins/GraphicsVulkan/VulkanCommandBuffer.hpp>
#include <Plugins/GraphicsVulkan/VulkanDescriptorSet.hpp>
#include <Plugins/GraphicsVulkan/VulkanWindowGraphicsBinding.hpp>

#include "ImguiRendererVulkan.hpp"

using namespace Grindstone::GraphicsAPI;
using namespace Grindstone::Editor::ImguiEditor;

static ImGui_ImplVulkanH_Window g_MainWindowData;

void ImguiRendererVulkan::SetupVulkanWindow(
	Grindstone::GraphicsAPI::VulkanCore* graphicsCore,
	Grindstone::GraphicsAPI::WindowGraphicsBinding* wgb,
	int width,
	int height
) {
	ImGui_ImplVulkanH_Window* wd = &g_MainWindowData;
	Grindstone::GraphicsAPI::VulkanWindowGraphicsBinding* vwgb = static_cast<Grindstone::GraphicsAPI::VulkanWindowGraphicsBinding*>(wgb);
	VkSurfaceKHR surface = vwgb->GetSurface();
	wd->Surface = surface;

	auto physicalDevice = graphicsCore->GetPhysicalDevice();
	auto graphicsFamily = graphicsCore->GetGraphicsFamily();

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
	SetupVulkanWindow(static_cast<GraphicsAPI::VulkanCore*>(graphicsCore), wgb, width, height);

	VkDescriptorPoolSize poolSizes[] = {
		{ VK_DESCRIPTOR_TYPE_SAMPLER, 1 }
	};

	VkDescriptorPoolCreateInfo poolInfo = {};
	poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
	poolInfo.flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT;
	poolInfo.maxSets = 1;
	poolInfo.poolSizeCount = static_cast<uint32_t>(std::size(poolSizes));
	poolInfo.pPoolSizes = poolSizes;

	auto vulkanCore = static_cast<Grindstone::GraphicsAPI::VulkanCore*>(graphicsCore);

	if (vkCreateDescriptorPool(vulkanCore->GetDevice(), &poolInfo, nullptr, &imguiPool) != VK_SUCCESS) {
		throw std::runtime_error("failed to allocate imgui descriptor pool!");
	}

	ImGui_ImplGlfw_InitForVulkan(static_cast<Grindstone::GlfwWindow*>(window)->GetHandle(), true);

	ImGui_ImplVulkan_InitInfo imguiInitInfo{};
	imguiInitInfo.Instance = vulkanCore->GetInstance();
	imguiInitInfo.PhysicalDevice = vulkanCore->GetPhysicalDevice();
	imguiInitInfo.Device = vulkanCore->GetDevice();
	imguiInitInfo.Queue = vulkanCore->graphicsQueue;
	imguiInitInfo.DescriptorPool = imguiPool;
	imguiInitInfo.MinImageCount = 3;
	imguiInitInfo.ImageCount = 3;
	imguiInitInfo.MSAASamples = VK_SAMPLE_COUNT_1_BIT;

	auto vulkanRenderPass = static_cast<Grindstone::GraphicsAPI::VulkanRenderPass*>(wgb->GetRenderPass());
	ImGui_ImplVulkan_Init(&imguiInitInfo, vulkanRenderPass->GetRenderPassHandle());

	Grindstone::GraphicsAPI::CommandBuffer::CreateInfo commandBufferCreateInfo{};

	commandBuffers.resize(wgb->GetMaxFramesInFlight());
	for (size_t i = 0; i < commandBuffers.size(); ++i) {
		commandBuffers[i] = graphicsCore->CreateCommandBuffer(commandBufferCreateInfo);
	}

	VkCommandBuffer commandBuffer = vulkanCore->BeginSingleTimeCommands();
	ImGui_ImplVulkan_CreateFontsTexture(commandBuffer);
	vulkanCore->EndSingleTimeCommands(commandBuffer);

	ImGui_ImplVulkan_DestroyFontUploadObjects();
}

ImguiRendererVulkan::~ImguiRendererVulkan() {}

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
	auto vulkanCore = static_cast<Grindstone::GraphicsAPI::VulkanCore*>(engineCore.GetGraphicsCore());
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
	auto window = engineCore.windowManager->GetWindowByIndex(0)->GetWindowGraphicsBinding();
	auto renderPass = window->GetRenderPass();
	auto framebuffer = window->GetCurrentFramebuffer();
	auto currentCommandBuffer = commandBuffers[window->GetCurrentImageIndex()];

	GraphicsAPI::ClearColorValue clearColor = { 0.0f, 0.0f, 0.0f, 1.0f };
	GraphicsAPI::ClearDepthStencil clearDepthStencil;
	clearDepthStencil.depth = 1.0f;
	clearDepthStencil.stencil = 0;
	clearDepthStencil.hasDepthStencilAttachment = true;

	currentCommandBuffer->BindRenderPass(renderPass, framebuffer, renderPass->GetWidth(), renderPass->GetHeight(), &clearColor, 1, clearDepthStencil);

	ImGui_ImplVulkan_NewFrame();
	ImGui_ImplGlfw_NewFrame();
	ImGui::NewFrame();
}

void ImguiRendererVulkan::PostRender() {
	Grindstone::EngineCore& engineCore = Grindstone::Editor::Manager::GetEngineCore();
	auto window = engineCore.windowManager->GetWindowByIndex(0)->GetWindowGraphicsBinding();
	auto currentCommandBuffer = commandBuffers[window->GetCurrentImageIndex()];
	ImGui::Render();

	auto vkCB = static_cast<GraphicsAPI::VulkanCommandBuffer*>(currentCommandBuffer);
	ImGui_ImplVulkan_RenderDrawData(ImGui::GetDrawData(), vkCB->GetCommandBuffer());

	ImGui::UpdatePlatformWindows();
	ImGui::RenderPlatformWindowsDefault();
	currentCommandBuffer->UnbindRenderPass();
	currentCommandBuffer->EndCommandBuffer();
	window->SubmitCommandBuffer(currentCommandBuffer);
	if (!window->PresentSwapchain() || shouldRebuildSwapchain) {
		WaitForResizeAndRecreateSwapchain();
		shouldRebuildSwapchain = false;
	}
}

void ImguiRendererVulkan::Resize() {
	shouldRebuildSwapchain = true;
}

ImTextureID ImguiRendererVulkan::CreateTexture(std::filesystem::path path) {
	std::filesystem::path fullPath = "../engineassets/editor/" / path;
	auto assetManager = Editor::Manager::GetEngineCore().assetManager;
	auto textureAsset = static_cast<TextureAsset*>(assetManager->GetAsset(Grindstone::AssetType::Texture, fullPath.string().c_str()));

	if (textureAsset == nullptr) {
		return 0;
	}

	GraphicsAPI::DescriptorSetLayout::Binding layoutBinding{};
	layoutBinding.bindingId = 0;
	layoutBinding.type = BindingType::Texture;
	layoutBinding.count = 1;
	layoutBinding.stages = ShaderStageBit::Fragment;

	GraphicsAPI::DescriptorSetLayout::CreateInfo descriptorSetLayoutCreateInfo{};
	descriptorSetLayoutCreateInfo.debugName = "Layout";
	descriptorSetLayoutCreateInfo.bindingCount = 1;
	descriptorSetLayoutCreateInfo.bindings = &layoutBinding;
	auto layout = Editor::Manager::GetEngineCore().GetGraphicsCore()->CreateDescriptorSetLayout(descriptorSetLayoutCreateInfo);

	GraphicsAPI::DescriptorSet::Binding binding{};
	binding.bindingIndex = 0;
	binding.bindingType = BindingType::Texture;
	binding.count = 1;
	binding.itemPtr = textureAsset->texture;

	auto pathAsStr = path.filename().string();
	GraphicsAPI::DescriptorSet::CreateInfo descriptorSetCreateInfo{};
	descriptorSetCreateInfo.debugName = pathAsStr.c_str();
	descriptorSetCreateInfo.bindings = &binding;
	descriptorSetCreateInfo.bindingCount = 1;
	descriptorSetCreateInfo.layout = layout;
	auto dset = Editor::Manager::GetEngineCore().GetGraphicsCore()->CreateDescriptorSet(descriptorSetCreateInfo);

	auto descriptor = static_cast<VulkanDescriptorSet*>(dset)->GetDescriptorSet();

	return (ImTextureID)(uint64_t)descriptor;
}

void ImguiRendererVulkan::CreateOrResizeWindow(
	Grindstone::GraphicsAPI::VulkanCore* graphicsCore,
	Grindstone::GraphicsAPI::WindowGraphicsBinding* wgb,
	int width, int height
) {
	auto instance = graphicsCore->GetInstance();
	auto physicalDevice = graphicsCore->GetPhysicalDevice();
	auto device = graphicsCore->GetDevice();
	auto graphicsFamily = graphicsCore->GetGraphicsFamily();

	ImGui_ImplVulkanH_CreateOrResizeWindow(instance, physicalDevice, device, &g_MainWindowData, graphicsFamily, nullptr, width, height, wgb->GetMaxFramesInFlight());

	// Send new imgui window data to window
	std::vector<VulkanImageSetNative> imguiImageSets;
	imguiImageSets.resize(g_MainWindowData.ImageCount);
	for (uint32_t i = 0; i < g_MainWindowData.ImageCount; ++i) {
		auto& imageSet = imguiImageSets[i];
		ImGui_ImplVulkanH_Frame& imguiFrame = g_MainWindowData.Frames[i];

		imageSet.image = imguiFrame.Backbuffer;
		imageSet.imageView = imguiFrame.BackbufferView;
		imageSet.framebuffer = imguiFrame.Framebuffer;
	}

	VulkanWindowBindingDataNative windowBindingData{};
	windowBindingData.swapChain = g_MainWindowData.Swapchain;
	windowBindingData.renderPass = g_MainWindowData.RenderPass;
	windowBindingData.width = static_cast<uint32_t>(g_MainWindowData.Width);
	windowBindingData.height = static_cast<uint32_t>(g_MainWindowData.Height);
	windowBindingData.surfaceFormat = g_MainWindowData.SurfaceFormat;
	windowBindingData.imageSetCount = g_MainWindowData.ImageCount;
	windowBindingData.imageSets = imguiImageSets.data();

	static_cast<VulkanWindowGraphicsBinding*>(wgb)->SubmitWindowObjects(windowBindingData);
}
