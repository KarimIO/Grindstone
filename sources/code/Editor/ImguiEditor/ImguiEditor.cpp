#include <iostream>
#include <imgui.h>
#include <imgui_internal.h>
#if EDITOR_USE_OPENGL
#include <imgui_impl_opengl3.h>
#include "GL/gl3w.h"
#else
#include <imgui_impl_vulkan.h>
#include <Plugins/GraphicsVulkan/VulkanCore.hpp>
#include <Plugins/GraphicsVulkan/VulkanRenderPass.hpp>
#include <Plugins/GraphicsVulkan/VulkanCommandBuffer.hpp>
#endif
#include <imgui_impl_win32.h>
#include <ImGuizmo.h>
#include <Windows.h>
#include <Winuser.h>

#include "Common/Window/WindowManager.hpp"
#include "EngineCore/EngineCore.hpp"
#include "Editor/EditorManager.hpp"
#include "Modals/ModelConverterModal.hpp"
#include "Modals/ImageConverterModal.hpp"
#include "ViewportPanel.hpp"
#include "ImguiEditor.hpp"
#include "ConsolePanel.hpp"
#include "SystemPanel.hpp"
#include "InspectorPanel.hpp"
#include "AssetBrowserPanel.hpp"
#include "SceneHeirarchyPanel.hpp"
#include "UserSettings/UserSettingsWindow.hpp"
#include "ProjectSettings/ProjectSettingsWindow.hpp"
#include "StatsPanel.hpp"
#include "BuildPopup.hpp"
#include "ControlBar.hpp"
#include "StatusBar.hpp"
#include "Menubar.hpp"
#include "ImguiInput.hpp"
using namespace Grindstone::Editor::ImguiEditor;

ImguiEditor::ImguiEditor(EngineCore* engineCore) : engineCore(engineCore), graphicsCore(engineCore->GetGraphicsCore()) {
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO(); (void)io;
	io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
	// io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;

	// SetupFonts();
	SetupStyles();
	SetupColors();

	HWND win = GetActiveWindow();
	ImGui_ImplWin32_Init(win);

#if EDITOR_USE_OPENGL
	if (gl3wInit()) {
		Editor::Manager::Print(LogSeverity::Error, "Failed to initialize OpenGL");
		return;
	}
	if (!gl3wIsSupported(3, 2)) {
		Editor::Manager::Print(LogSeverity::Error, "OpenGL 3.2 not supported\n");
		return;
	}

	ImGui_ImplOpenGL3_Init("#version 150");
#else
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

	auto vulkanCore = static_cast<GraphicsAPI::VulkanCore*>(graphicsCore);

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

	auto window = engineCore->windowManager->GetWindowByIndex(0)->GetWindowGraphicsBinding();
	auto vulkanRenderPass = static_cast<GraphicsAPI::VulkanRenderPass*>(window->GetRenderPass());
	ImGui_ImplVulkan_Init(&imguiInitInfo, vulkanRenderPass->GetRenderPassHandle());

	GraphicsAPI::CommandBuffer::CreateInfo commandBufferCreateInfo{};

	commandBuffers.resize(window->GetMaxFramesInFlight());
	for (size_t i = 0; i < commandBuffers.size(); ++i) {
		commandBuffers[i] = graphicsCore->CreateCommandBuffer(commandBufferCreateInfo);
	}

	VkCommandBuffer commandBuffer = vulkanCore->BeginSingleTimeCommands();
	ImGui_ImplVulkan_CreateFontsTexture(commandBuffer);
	vulkanCore->EndSingleTimeCommands(commandBuffer);


	ImGui_ImplVulkan_DestroyFontUploadObjects();

#endif

	input = new ImguiInput(io, engineCore);

	sceneHeirarchyPanel = new SceneHeirarchyPanel(engineCore->GetSceneManager(), this);
	modelConverterModal = new ModelConverterModal();
	imageConverterModal = new ImageConverterModal();
	inspectorPanel = new InspectorPanel(engineCore);
	assetBrowserPanel = nullptr; //new AssetBrowserPanel(engineCore, this);
	userSettingsWindow = new Settings::UserSettingsWindow();
	projectSettingsWindow = new Settings::ProjectSettingsWindow();
	viewportPanel = nullptr; //new ViewportPanel();
	consolePanel = nullptr; //new ConsolePanel();
	statsPanel = new StatsPanel();
	buildPopup = new BuildPopup();
	systemPanel = new SystemPanel(engineCore->GetSystemRegistrar());
	controlBar = nullptr; //new ControlBar();
	menubar = new Menubar(this);
	statusBar = new StatusBar();
}

ImguiEditor::~ImguiEditor() {
	delete sceneHeirarchyPanel;
	delete modelConverterModal;
	delete imageConverterModal;
	delete inspectorPanel;
	delete assetBrowserPanel;
	delete userSettingsWindow;
	delete projectSettingsWindow;
	delete viewportPanel;
	delete consolePanel;
	delete statsPanel;
	delete buildPopup;
	delete systemPanel;
	delete controlBar;
	delete menubar;
	delete statusBar;
}

void ImguiEditor::SetupFonts() {
	auto& io = ImGui::GetIO();

	std::filesystem::path fontFolder = engineCore->GetEngineBinaryPath().parent_path() / "engineassets/editor/fonts";
	std::filesystem::path robotoBoldPath = fontFolder / "OpenSans-Bold.ttf";
	std::filesystem::path robotoRegularPath = fontFolder / "OpenSans-Regular.ttf";

	float fontSize = 14.0f;
	io.Fonts->AddFontFromFileTTF(robotoBoldPath.string().c_str(), fontSize);
	io.FontDefault = io.Fonts->AddFontFromFileTTF(robotoRegularPath.string().c_str(), fontSize);
}

void ImguiEditor::SetupColors() {
	ImGui::StyleColorsDark();

	auto& colors = ImGui::GetStyle().Colors;

	// Swatches
	ImVec4 bgColor = ImVec4(0.1f, 0.11f, 0.12f, 1.00f);
	ImVec4 highlightColor0 = ImVec4(0.35f, 0.18f, 0.40f, 1.00f);
	ImVec4 highlightColor1 = ImVec4(0.47f, 0.28f, 0.54f, 1.00f);
	ImVec4 highlightColor2 = ImVec4(0.56f, 0.36f, 0.64f, 1.00f);

	// Backgrounds
	colors[ImGuiCol_WindowBg]			= bgColor;
	colors[ImGuiCol_ChildBg]			= bgColor;
	colors[ImGuiCol_PopupBg]			= bgColor;
	colors[ImGuiCol_MenuBarBg]			= bgColor;

	// Text
	colors[ImGuiCol_Text]				= ImVec4(0.95f, 0.95f, 0.95f, 1.00f);
	colors[ImGuiCol_TextDisabled]		= ImVec4(0.50f, 0.50f, 0.50f, 1.00f);
	colors[ImGuiCol_TextSelectedBg]		= highlightColor0;

	// Border
	colors[ImGuiCol_Border]				= ImVec4(0.2f, 0.24f, 0.26f, 0.50f);
	colors[ImGuiCol_BorderShadow]		= ImVec4(0.00f, 0.00f, 0.00f, 0.00f);

	// Frame
	colors[ImGuiCol_FrameBg]			= ImVec4(0.05f, 0.055f, 0.06f, 1.00f);
	colors[ImGuiCol_FrameBgHovered]		= ImVec4(0.033f, 0.0363f, 0.0396f, 1.00f);
	colors[ImGuiCol_FrameBgActive]		= ImVec4(0.033f, 0.0363f, 0.0396f, 0.45f);

	// Title
	colors[ImGuiCol_TitleBg]			= ImVec4(0.08f, 0.09f, 0.1f, 1.00f);
	colors[ImGuiCol_TitleBgActive]		= colors[ImGuiCol_TitleBg];
	colors[ImGuiCol_TitleBgCollapsed]	= colors[ImGuiCol_TitleBg];

	// Scrollbar
	colors[ImGuiCol_ScrollbarBg]			= ImVec4(0.02f, 0.04f, 0.06f, 0.53f);
	colors[ImGuiCol_ScrollbarGrab]			= ImVec4(0.28f, 0.32f, 0.36f, 1.00f);
	colors[ImGuiCol_ScrollbarGrabHovered]	= ImVec4(0.40f, 0.42f, 0.44f, 1.00f);
	colors[ImGuiCol_ScrollbarGrabActive]	= ImVec4(0.46f, 0.50f, 0.54f, 1.00f);

	// Slider
	colors[ImGuiCol_SliderGrab]			= highlightColor0;
	colors[ImGuiCol_SliderGrabActive]	= highlightColor2;

	// Button
	colors[ImGuiCol_Button]				= highlightColor0;
	colors[ImGuiCol_ButtonHovered]		= highlightColor1;
	colors[ImGuiCol_ButtonActive]		= highlightColor2;

	// Header
	colors[ImGuiCol_Header]				= ImVec4(0.22f, 0.22f, 0.22f, 1.00f);
	colors[ImGuiCol_HeaderHovered]		= ImVec4(0.25f, 0.25f, 0.25f, 1.00f);
	colors[ImGuiCol_HeaderActive]		= ImVec4(0.67f, 0.67f, 0.67f, 0.39f);

	// Separator
	colors[ImGuiCol_Separator]			= colors[ImGuiCol_Border];
	colors[ImGuiCol_SeparatorHovered]	= highlightColor0;
	colors[ImGuiCol_SeparatorActive]	= highlightColor2;

	// Resize Grip
	colors[ImGuiCol_ResizeGrip]			= ImVec4(0.00f, 0.00f, 0.00f, 0.00f);
	colors[ImGuiCol_ResizeGripHovered]	= highlightColor0;
	colors[ImGuiCol_ResizeGripActive]	= highlightColor2;

	// Tabs
	colors[ImGuiCol_Tab]				= ImVec4(0.1f, 0.12f, 0.14f, 1.f);
	colors[ImGuiCol_TabHovered]			= ImVec4(0.14f, 0.168f, 0.196f, 1.f);
	colors[ImGuiCol_TabActive]			= highlightColor0;
	colors[ImGuiCol_TabUnfocused]		= ImVec4(0.08f, 0.09f, 0.1f, 1.f);
	colors[ImGuiCol_TabUnfocusedActive] = bgColor;

	// Docking
	colors[ImGuiCol_DockingPreview]		= ImVec4(0.26f, 0.59f, 0.98f, 0.70f);
	colors[ImGuiCol_DockingEmptyBg]		= ImVec4(0.20f, 0.20f, 0.20f, 1.00f);

	// Plots
	colors[ImGuiCol_PlotLines]				= ImVec4(0.61f, 0.61f, 0.61f, 1.00f);
	colors[ImGuiCol_PlotLinesHovered]		= ImVec4(1.00f, 0.43f, 0.35f, 1.00f);
	colors[ImGuiCol_PlotHistogram]			= ImVec4(0.90f, 0.70f, 0.00f, 1.00f);
	colors[ImGuiCol_PlotHistogramHovered]	= ImVec4(1.00f, 0.60f, 0.00f, 1.00f);

	// Nav
	colors[ImGuiCol_NavHighlight]			= ImVec4(0.26f, 0.59f, 0.98f, 1.00f);
	colors[ImGuiCol_NavWindowingHighlight]	= ImVec4(1.00f, 1.00f, 1.00f, 0.70f);
	colors[ImGuiCol_NavWindowingDimBg]		= ImVec4(0.80f, 0.80f, 0.80f, 0.20f);

	// Table
	colors[ImGuiCol_TableRowBg] = bgColor;
	colors[ImGuiCol_TableRowBgAlt] = ImVec4(0.08f, 0.09f, 0.1f, 1.00f);

	// Misc
	colors[ImGuiCol_DragDropTarget]		= highlightColor0;
	colors[ImGuiCol_CheckMark]			= highlightColor0;
	colors[ImGuiCol_ModalWindowDimBg]	= ImVec4(0.80f, 0.80f, 0.80f, 0.35f);;
}

void ImguiEditor::SetupStyles() {
	auto& style = ImGui::GetStyle();
	style.FramePadding = ImVec2(5.0f, 5.0f);
	style.TabRounding = 4.0f;
	style.GrabRounding = 4.0f;
	style.FrameRounding = 4.0f;
}

void ImguiEditor::Update() {
	auto window = engineCore->windowManager->GetWindowByIndex(0)->GetWindowGraphicsBinding();
	window->AcquireNextImage();
	auto renderPass = window->GetRenderPass();
	auto framebuffer = window->GetCurrentFramebuffer();
	auto currentCommandBuffer = commandBuffers[window->GetCurrentImageIndex()];

	GraphicsAPI::ClearColorValue clearColor = { 0.0f, 0.0f, 0.0f, 0.f };
	GraphicsAPI::ClearDepthStencil clearDepthStencil;
	clearDepthStencil.depth = 1.0f;
	clearDepthStencil.stencil = 0;
	clearDepthStencil.hasDepthStencilAttachment = true;

	currentCommandBuffer->BeginCommandBuffer();
	currentCommandBuffer->BindRenderPass(renderPass, framebuffer, 800, 600, &clearColor, 1, clearDepthStencil);

#if EDITOR_USE_OPENGL
	ImGui_ImplOpenGL3_NewFrame();
#else
	ImGui_ImplVulkan_NewFrame();
#endif
	ImGui_ImplWin32_NewFrame();
	ImGui::NewFrame();
	ImGuizmo::BeginFrame();

#if EDITOR_USE_OPENGL
	glViewport(0, 0, 800, 600);
	glClear(GL_COLOR_BUFFER_BIT);
#endif

	Render();

	ImGui::Render();
#if EDITOR_USE_OPENGL
	ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
#else
	auto vkCB = static_cast<GraphicsAPI::VulkanCommandBuffer*>(currentCommandBuffer);
	ImGui_ImplVulkan_RenderDrawData(ImGui::GetDrawData(), vkCB->GetCommandBuffer());
#endif
	ImGui::UpdatePlatformWindows();
	ImGui::RenderPlatformWindowsDefault();
	currentCommandBuffer->UnbindRenderPass();
	currentCommandBuffer->EndCommandBuffer();
	window->SubmitCommandBuffer(currentCommandBuffer);
	window->PresentSwapchain();
}

void ImguiEditor::Render() {
	RenderDockspace();
	// controlBar->Render();
	modelConverterModal->Render();
	imageConverterModal->Render();
	sceneHeirarchyPanel->Render();
	// viewportPanel->Render();
	// consolePanel->Render();
	// assetBrowserPanel->Render();
	systemPanel->Render();
	statsPanel->Render();
	inspectorPanel->Render();
	buildPopup->Render();
	userSettingsWindow->Render();
	projectSettingsWindow->Render();
	statusBar->Render();
}

void ImguiEditor::ShowModelModal() {
	modelConverterModal->Show();
}

void ImguiEditor::ShowImageModal() {
	imageConverterModal->Show();
}

void ImguiEditor::StartBuild() {
	buildPopup->StartBuild();
}

void ImguiEditor::ImportFile(const char* folderPathToImportTo) {
	auto window = engineCore->windowManager->GetWindowByIndex(0);
	auto filePath = window->OpenFileDialogue();
}

void ImguiEditor::RenderDockspace() {
	static ImGuiDockNodeFlags optFlags = ImGuiDockNodeFlags_None;

	// We are using the ImGuiWindowFlags_NoDocking flag to make the parent window not dockable into,
	// - because it would be confusing to have two docking targets within each others.
	ImGuiViewport* viewport = ImGui::GetMainViewport();
	ImGui::SetNextWindowPos(viewport->Pos);
	float statusBarHeight = ImGui::GetFrameHeight();
	ImGui::SetNextWindowSize(ImVec2(viewport->Size.x, viewport->Size.y - statusBarHeight));
	ImGui::SetNextWindowViewport(viewport->ID);
	ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
	ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
	ImGuiWindowFlags windowFlags = ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse |
		ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_MenuBar |
		ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoNavFocus;

	// When using ImGuiDockNodeFlags_PassthruDockspace, DockSpace() will render our background and
	// - handle the pass-thru hole, so we ask Begin() to not render a background.
	if (optFlags & ImGuiDockNodeFlags_PassthruCentralNode)
		windowFlags |= ImGuiWindowFlags_NoBackground;

	ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
	ImGui::Begin("Editor Dockspace", nullptr, windowFlags);
	ImGui::PopStyleVar();
	ImGui::PopStyleVar(2);

	ImGuiID dockspaceId = ImGui::GetID("Editor Dockspace");
	ImGui::DockSpace(dockspaceId, ImVec2(0.0f, 0.0f), optFlags);

	menubar->Render();

	ImGui::End();

	static bool sFirstFrame = true;
	if (sFirstFrame) {
		sFirstFrame = false;

		ImGui::DockBuilderRemoveNode(dockspaceId);
		ImGui::DockBuilderAddNode(dockspaceId, ImGuiDockNodeFlags_None);

		ImGuiID dockIdControlbarArea;
		ImGuiID dockIdAboveConsoleArea;
		ImGuiID dockIdViewportArea;
		ImGuiID dockIdConsoleArea;
		ImGuiID dockIdSceneHeirarchyArea;
		ImGuiID dockIdInspectorArea;
		ImGuiID dockIdViewportAndSceneHeirarchy;
		ImGuiID dockIdMain;

		ImGui::DockBuilderSplitNode(dockspaceId, ImGuiDir_Up, 0.1f, &dockIdControlbarArea, &dockIdMain);
		ImGui::DockBuilderSplitNode(dockIdMain, ImGuiDir_Down, 0.3f, &dockIdConsoleArea, &dockIdAboveConsoleArea);
		ImGui::DockBuilderSplitNode(dockIdAboveConsoleArea, ImGuiDir_Right, 0.3f, &dockIdInspectorArea, &dockIdViewportAndSceneHeirarchy);
		ImGui::DockBuilderSplitNode(dockIdViewportAndSceneHeirarchy, ImGuiDir_Left, 0.3f, &dockIdSceneHeirarchyArea, &dockIdViewportArea);

		ImGui::DockBuilderDockWindow("ControlBar", dockIdControlbarArea);
		ImGui::DockBuilderDockWindow("Viewport", dockIdViewportArea);
		ImGui::DockBuilderDockWindow("Console", dockIdConsoleArea);
		ImGui::DockBuilderDockWindow("Asset Browser", dockIdConsoleArea);
		ImGui::DockBuilderDockWindow("Scene Heirarchy", dockIdSceneHeirarchyArea);
		ImGui::DockBuilderDockWindow("Inspector", dockIdInspectorArea);
		ImGui::DockBuilderDockWindow("Stats", dockIdInspectorArea);
		ImGui::DockBuilderDockWindow("Systems", dockIdInspectorArea);

		ImGuiDockNode* node = ImGui::DockBuilderGetNode(dockIdControlbarArea);
		node->LocalFlags |= ImGuiDockNodeFlags_NoTabBar | ImGuiDockNodeFlags_NoResize;
		ImGui::DockBuilderSetNodeSize(dockIdControlbarArea, ImVec2(1.0f, 4.0f));

		ImGui::DockBuilderFinish(dockspaceId);
	}
}
