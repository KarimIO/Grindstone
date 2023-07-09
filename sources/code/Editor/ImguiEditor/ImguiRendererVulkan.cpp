#include <iostream>
#include <imgui.h>
#include <imgui_internal.h>
#include <imgui_impl_vulkan.h>
#include <imgui_impl_win32.h>
#include <Windows.h>

#include <Common/Window/WindowManager.hpp>
#include <Editor/EditorManager.hpp>
#include <EngineCore/Assets/AssetManager.hpp>
#include <EngineCore/Assets/Textures/TextureAsset.hpp>
#include <Plugins/GraphicsVulkan/VulkanCore.hpp>
#include <Plugins/GraphicsVulkan/VulkanRenderPass.hpp>
#include <Plugins/GraphicsVulkan/VulkanCommandBuffer.hpp>
#include <Plugins/GraphicsVulkan/VulkanDescriptorSet.hpp>

#include "ImguiRendererVulkan.hpp"

using namespace Grindstone::GraphicsAPI;
using namespace Grindstone::Editor::ImguiEditor;

ImguiRendererVulkan::ImguiRendererVulkan() {
	Grindstone::EngineCore& engineCore = Grindstone::Editor::Manager::GetEngineCore();
	auto graphicsCore = engineCore.GetGraphicsCore();

	HWND win = GetActiveWindow();
	ImGui_ImplWin32_Init(win);

	VkDescriptorPoolSize poolSizes[] = {
		{ VK_DESCRIPTOR_TYPE_SAMPLER, 1000 },
		{ VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1000 },
		{ VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, 1000 },
		{ VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 1000 },
		{ VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER, 1000 },
		{ VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER, 1000 },
		{ VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1000 },
		{ VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1000 },
		{ VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, 1000 },
		{ VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC, 1000 },
		{ VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT, 1000 }
	};

	VkDescriptorPoolCreateInfo poolInfo = {};
	poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
	poolInfo.flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT;
	poolInfo.maxSets = 1000;
	poolInfo.poolSizeCount = static_cast<uint32_t>(std::size(poolSizes));
	poolInfo.pPoolSizes = poolSizes;

	auto vulkanCore = static_cast<Grindstone::GraphicsAPI::VulkanCore*>(graphicsCore);

	if (vkCreateDescriptorPool(vulkanCore->GetDevice(), &poolInfo, nullptr, &imguiPool) != VK_SUCCESS) {
		throw std::runtime_error("failed to allocate imgui descriptor pool!");
	}

	ImGui_ImplVulkan_InitInfo imguiInitInfo{};
	imguiInitInfo.Instance = vulkanCore->GetInstance();
	imguiInitInfo.PhysicalDevice = vulkanCore->GetPhysicalDevice();
	imguiInitInfo.Device = vulkanCore->GetDevice();
	imguiInitInfo.Queue = vulkanCore->graphicsQueue;
	imguiInitInfo.DescriptorPool = imguiPool;
	imguiInitInfo.MinImageCount = 3;
	imguiInitInfo.ImageCount = 3;
	imguiInitInfo.MSAASamples = VK_SAMPLE_COUNT_1_BIT;

	auto window = engineCore.windowManager->GetWindowByIndex(0)->GetWindowGraphicsBinding();
	auto vulkanRenderPass = static_cast<Grindstone::GraphicsAPI::VulkanRenderPass*>(window->GetRenderPass());
	ImGui_ImplVulkan_Init(&imguiInitInfo, vulkanRenderPass->GetRenderPassHandle());

	Grindstone::GraphicsAPI::CommandBuffer::CreateInfo commandBufferCreateInfo{};

	commandBuffers.resize(window->GetMaxFramesInFlight());
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
	auto window = engineCore.windowManager->GetWindowByIndex(0)->GetWindowGraphicsBinding();
	return commandBuffers[window->GetCurrentImageIndex()];
}

void ImguiRendererVulkan::PreRender() {
	Grindstone::EngineCore& engineCore = Grindstone::Editor::Manager::GetEngineCore();
	auto window = engineCore.windowManager->GetWindowByIndex(0)->GetWindowGraphicsBinding();
	window->AcquireNextImage();
	auto currentCommandBuffer = commandBuffers[window->GetCurrentImageIndex()];

	currentCommandBuffer->BeginCommandBuffer();
}

void ImguiRendererVulkan::PrepareImguiRendering() {
	Grindstone::EngineCore& engineCore = Grindstone::Editor::Manager::GetEngineCore();
	auto window = engineCore.windowManager->GetWindowByIndex(0)->GetWindowGraphicsBinding();
	auto renderPass = window->GetRenderPass();
	auto framebuffer = window->GetCurrentFramebuffer();
	auto currentCommandBuffer = commandBuffers[window->GetCurrentImageIndex()];

	GraphicsAPI::ClearColorValue clearColor = { 0.0f, 0.0f, 0.0f, 0.f };
	GraphicsAPI::ClearDepthStencil clearDepthStencil;
	clearDepthStencil.depth = 1.0f;
	clearDepthStencil.stencil = 0;
	clearDepthStencil.hasDepthStencilAttachment = true;

	currentCommandBuffer->BindRenderPass(renderPass, framebuffer, 800, 600, &clearColor, 1, clearDepthStencil);

	ImGui_ImplVulkan_NewFrame();
	ImGui_ImplWin32_NewFrame();
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
	window->PresentSwapchain();
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

	GraphicsAPI::DescriptorSet::CreateInfo descriptorSetCreateInfo{};
	descriptorSetCreateInfo.debugName = path.filename().string().c_str();
	descriptorSetCreateInfo.bindings = &binding;
	descriptorSetCreateInfo.bindingCount = 1;
	descriptorSetCreateInfo.layout = layout;
	auto dset = Editor::Manager::GetEngineCore().GetGraphicsCore()->CreateDescriptorSet(descriptorSetCreateInfo);

	auto descriptor = static_cast<VulkanDescriptorSet*>(dset)->GetDescriptorSet();

	return (ImTextureID)(uint64_t)descriptor;
}
