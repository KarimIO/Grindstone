#include <iostream>
#include <imgui.h>
#include <imgui_internal.h>
#include <ImGuizmo.h>
#include <Windows.h>
#include <Winuser.h>

#include <Common/Window/WindowManager.hpp>
#include <Common/Event/WindowEvent.hpp>
#include <EngineCore/EngineCore.hpp>
#include <EngineCore/Utils/MemoryAllocator.hpp>
#include <EngineCore/Events/Dispatcher.hpp>
#include <Editor/EditorManager.hpp>

#include "ViewportPanel.hpp"
#include "ImguiEditor.hpp"
#include "ConsolePanel.hpp"
#include "SystemPanel.hpp"
#include "InspectorPanel.hpp"
#include "AssetBrowserPanel.hpp"
#include "SceneHeirarchyPanel.hpp"
#include "UserSettings/UserSettingsWindow.hpp"
#include "ProjectSettings/ProjectSettingsWindow.hpp"
#include "PluginsWindow.hpp"
#include "StatsPanel.hpp"
#include "BuildPopup.hpp"
#include "ControlBar.hpp"
#include "StatusBar.hpp"
#include "Menubar.hpp"
#include "AssetPicker.hpp"
#include "ImguiInput.hpp"
#include "TracingPanel.hpp"
#include "ImguiRenderer.hpp"

using namespace Grindstone::Editor::ImguiEditor;
using namespace Grindstone::Memory;

static void LinkCallback(ImGui::MarkdownLinkCallbackData data) {
	std::string url(data.link, data.linkLength);
	if (!data.isImage) {
		ShellExecuteA(NULL, "open", url.c_str(), NULL, NULL, SW_SHOWNORMAL);
	}
}

static ImGui::MarkdownImageData ImageCallback(ImGui::MarkdownLinkCallbackData data) {
	// In your application you would load an image based on data_ input. Here we just use the imgui font texture.
#ifdef IMGUI_HAS_TEXTURES // used to detect dynamic font capability
	ImTextureID image = ImGui::GetIO().Fonts->TexRef.GetTexID();
#else
	ImTextureID image = ImGui::GetIO().Fonts->TexID;
#endif
	// > C++14 can use ImGui::MarkdownImageData imageData{ true, false, image, ImVec2( 40.0f, 20.0f ) };
	ImGui::MarkdownImageData imageData;
	imageData.isValid = true;
	imageData.useLinkCallback = false;
	imageData.user_texture_id = image;
	imageData.size = ImVec2(40.0f, 20.0f);

	// For image resize when available size.x > image width, add
	ImVec2 const contentSize = ImGui::GetContentRegionAvail();
	if (imageData.size.x > contentSize.x)
	{
		float const ratio = imageData.size.y / imageData.size.x;
		imageData.size.x = contentSize.x;
		imageData.size.y = contentSize.x * ratio;
	}

	return imageData;
}

ImguiEditor::ImguiEditor(EngineCore* engineCore) : engineCore(engineCore) {
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO();
	io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
	io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;
	io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
	io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;

	imguiIniFile = (engineCore->GetProjectPath() / "imgui.ini").string();
	imguiLogFile = (engineCore->GetProjectPath() / "log/imgui.log").string();
	io.IniFilename = imguiIniFile.c_str();
	io.LogFilename = imguiLogFile.c_str();

	markdownConfig.imageCallback = ImageCallback;
	markdownConfig.linkCallback = LinkCallback;

	SetupFonts();
	SetupStyles();
	SetupColors();
}

void ImguiEditor::CreateWindows() {
	imguiRenderer = ImguiRenderer::Create();

	sceneHeirarchyPanel = AllocatorCore::Allocate<SceneHeirarchyPanel>(engineCore->GetSceneManager(), this);
	inspectorPanel = AllocatorCore::Allocate<InspectorPanel>(engineCore, this);
	assetBrowserPanel = AllocatorCore::Allocate<AssetBrowserPanel>(imguiRenderer, engineCore, this);
	userSettingsWindow = AllocatorCore::Allocate<Settings::UserSettingsWindow>();
	projectSettingsWindow = AllocatorCore::Allocate<Settings::ProjectSettingsWindow>();
	pluginsWindow = AllocatorCore::Allocate<PluginsWindow>();
	viewportPanel = AllocatorCore::Allocate<ViewportPanel>();
	consolePanel = AllocatorCore::Allocate<ConsolePanel>(imguiRenderer);
	statsPanel = AllocatorCore::Allocate<StatsPanel>();
	buildPopup = AllocatorCore::Allocate<BuildPopup>();
	systemPanel = AllocatorCore::Allocate<SystemPanel>(engineCore->GetSystemRegistrar());
	controlBar = AllocatorCore::Allocate<ControlBar>(imguiRenderer);
	menubar = AllocatorCore::Allocate<Menubar>(this);
	assetPicker = AllocatorCore::Allocate<AssetPicker>();
	statusBar = AllocatorCore::Allocate<StatusBar>(imguiRenderer);
	tracingPanel = AllocatorCore::Allocate<TracingPanel>();

	auto eventDispatcher = engineCore->GetEventDispatcher();
	eventDispatcher->AddEventListener(
		Events::EventType::WindowResize,
		std::bind(&ImguiEditor::OnWindowResize, this, std::placeholders::_1)
	);
}

ImguiEditor::~ImguiEditor() {
	AllocatorCore::Free(imguiRenderer);

	if (input) {
		AllocatorCore::Free(input);
	}

	AllocatorCore::Free(sceneHeirarchyPanel);
	AllocatorCore::Free(inspectorPanel);
	AllocatorCore::Free(assetBrowserPanel);
	AllocatorCore::Free(userSettingsWindow);
	AllocatorCore::Free(projectSettingsWindow);
	AllocatorCore::Free(pluginsWindow);
	AllocatorCore::Free(viewportPanel);
	AllocatorCore::Free(consolePanel);
	AllocatorCore::Free(statsPanel);
	AllocatorCore::Free(buildPopup);
	AllocatorCore::Free(systemPanel);
	AllocatorCore::Free(controlBar);
	AllocatorCore::Free(menubar);
	AllocatorCore::Free(assetPicker);
	AllocatorCore::Free(statusBar);
	AllocatorCore::Free(tracingPanel);
}

void ImguiEditor::PerformResize() {
	queueResize = false;
	imguiRenderer->Resize();
}

bool ImguiEditor::OnWindowResize(Events::BaseEvent* ev) {
	if (ev->GetEventType() == Events::EventType::WindowResize) {
		Events::WindowResizeEvent* winResizeEvent = (Events::WindowResizeEvent*)ev;
		queueResize = true;
	}

	return false;
}

void ImguiEditor::SetupFonts() {
	auto& io = ImGui::GetIO();

	std::filesystem::path fontFolder = engineCore->GetEngineBinaryPath().parent_path() / "engineassets/editor/fonts";
	std::filesystem::path openSansBoldPath = fontFolder / "OpenSans-Bold.ttf";
	std::filesystem::path openSansItalicPath = fontFolder / "OpenSans-Italic.ttf";
	std::filesystem::path openSansRegularPath = fontFolder / "OpenSans-Regular.ttf";
	std::string openSansBoldPathString = openSansBoldPath.string();

	float fontSize = 14.0f;
	fonts[static_cast<size_t>(FontType::Regular)] = io.Fonts->AddFontFromFileTTF(openSansRegularPath.string().c_str(), fontSize);
	fonts[static_cast<size_t>(FontType::Bold)] = io.Fonts->AddFontFromFileTTF(openSansBoldPathString.c_str(), fontSize);
	fonts[static_cast<size_t>(FontType::Italic)] = io.Fonts->AddFontFromFileTTF(openSansItalicPath.string().c_str(), fontSize);
	fonts[static_cast<size_t>(FontType::H1)] = io.Fonts->AddFontFromFileTTF(openSansBoldPathString.c_str(), fontSize * 2.0f);
	fonts[static_cast<size_t>(FontType::H2)] = io.Fonts->AddFontFromFileTTF(openSansBoldPathString.c_str(), fontSize * 1.5f);
	fonts[static_cast<size_t>(FontType::H3)] = io.Fonts->AddFontFromFileTTF(openSansBoldPathString.c_str(), fontSize * 1.17f);

	io.FontDefault = fonts[static_cast<size_t>(FontType::Regular)];

	markdownConfig.headingFormats[0] = { fonts[static_cast<size_t>(FontType::H1)], true };
	markdownConfig.headingFormats[1] = { fonts[static_cast<size_t>(FontType::H2)], true };
	markdownConfig.headingFormats[2] = { fonts[static_cast<size_t>(FontType::H3)], true };

}

void ImguiEditor::SetupColors() {
	ImGui::StyleColorsDark();

	auto& colors = ImGui::GetStyle().Colors;

	// Swatches
	ImVec4 bgColor							= ImVec4(0.1f, 0.11f, 0.12f, 1.00f);
	ImVec4 highlightColor0					= ImVec4(0.35f, 0.18f, 0.40f, 1.00f);
	ImVec4 highlightColor1					= ImVec4(0.47f, 0.28f, 0.54f, 1.00f);
	ImVec4 highlightColor2					= ImVec4(0.56f, 0.36f, 0.64f, 1.00f);

	// Backgrounds
	colors[ImGuiCol_WindowBg]				= bgColor;
	colors[ImGuiCol_ChildBg]				= bgColor;
	colors[ImGuiCol_PopupBg]				= bgColor;
	colors[ImGuiCol_MenuBarBg]				= bgColor;

	// Text
	colors[ImGuiCol_Text]					= ImVec4(0.95f, 0.95f, 0.95f, 1.00f);
	colors[ImGuiCol_TextDisabled]			= ImVec4(0.50f, 0.50f, 0.50f, 1.00f);
	colors[ImGuiCol_TextSelectedBg]			= highlightColor0;

	// Border
	colors[ImGuiCol_Border]					= ImVec4(0.2f, 0.24f, 0.26f, 0.50f);
	colors[ImGuiCol_BorderShadow]			= ImVec4(0.00f, 0.00f, 0.00f, 0.00f);

	// Frame
	colors[ImGuiCol_FrameBg]				= ImVec4(0.05f, 0.055f, 0.06f, 1.00f);
	colors[ImGuiCol_FrameBgHovered]			= ImVec4(0.033f, 0.0363f, 0.0396f, 1.00f);
	colors[ImGuiCol_FrameBgActive]			= ImVec4(0.033f, 0.0363f, 0.0396f, 0.45f);

	// Title
	colors[ImGuiCol_TitleBg]				= ImVec4(0.08f, 0.09f, 0.1f, 1.00f);
	colors[ImGuiCol_TitleBgActive]			= colors[ImGuiCol_TitleBg];
	colors[ImGuiCol_TitleBgCollapsed]		= colors[ImGuiCol_TitleBg];

	// Scrollbar
	colors[ImGuiCol_ScrollbarBg]			= ImVec4(0.02f, 0.04f, 0.06f, 0.53f);
	colors[ImGuiCol_ScrollbarGrab]			= ImVec4(0.28f, 0.32f, 0.36f, 1.00f);
	colors[ImGuiCol_ScrollbarGrabHovered]	= ImVec4(0.40f, 0.42f, 0.44f, 1.00f);
	colors[ImGuiCol_ScrollbarGrabActive]	= ImVec4(0.46f, 0.50f, 0.54f, 1.00f);

	// Slider
	colors[ImGuiCol_SliderGrab]				= highlightColor0;
	colors[ImGuiCol_SliderGrabActive]		= highlightColor2;

	// Button
	colors[ImGuiCol_Button]					= highlightColor0;
	colors[ImGuiCol_ButtonHovered]			= highlightColor1;
	colors[ImGuiCol_ButtonActive]			= highlightColor2;

	// Header
	colors[ImGuiCol_Header]					= ImVec4(0.22f, 0.22f, 0.22f, 1.00f);
	colors[ImGuiCol_HeaderHovered]			= ImVec4(0.25f, 0.25f, 0.25f, 1.00f);
	colors[ImGuiCol_HeaderActive]			= ImVec4(0.67f, 0.67f, 0.67f, 0.39f);

	// Separator
	colors[ImGuiCol_Separator]				= colors[ImGuiCol_Border];
	colors[ImGuiCol_SeparatorHovered]		= highlightColor0;
	colors[ImGuiCol_SeparatorActive]		= highlightColor2;

	// Resize Grip
	colors[ImGuiCol_ResizeGrip]				= ImVec4(0.00f, 0.00f, 0.00f, 0.00f);
	colors[ImGuiCol_ResizeGripHovered]		= highlightColor0;
	colors[ImGuiCol_ResizeGripActive]		= highlightColor2;

	// Tabs
	colors[ImGuiCol_Tab]					= ImVec4(0.1f, 0.12f, 0.14f, 1.f);
	colors[ImGuiCol_TabHovered]				= ImVec4(0.14f, 0.168f, 0.196f, 1.f);
	colors[ImGuiCol_TabActive]				= highlightColor0;
	colors[ImGuiCol_TabUnfocused]			= ImVec4(0.08f, 0.09f, 0.1f, 1.f);
	colors[ImGuiCol_TabUnfocusedActive]		= bgColor;

	// Docking
	colors[ImGuiCol_DockingPreview]			= ImVec4(0.26f, 0.59f, 0.98f, 0.70f);
	colors[ImGuiCol_DockingEmptyBg]			= ImVec4(0.20f, 0.20f, 0.20f, 1.00f);

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
	colors[ImGuiCol_TableRowBg]				= bgColor;
	colors[ImGuiCol_TableRowBgAlt]			= ImVec4(0.08f, 0.09f, 0.1f, 1.00f);

	// Misc
	colors[ImGuiCol_DragDropTarget]			= highlightColor0;
	colors[ImGuiCol_CheckMark]				= highlightColor0;
	colors[ImGuiCol_ModalWindowDimBg]		= ImVec4(0.80f, 0.80f, 0.80f, 0.35f);;
}

void ImguiEditor::SetupStyles() {
	ImGuiStyle& style		= ImGui::GetStyle();
	style.FramePadding		= ImVec2(5.0f, 5.0f);
	style.TabRounding		= 4.0f;
	style.GrabRounding		= 4.0f;
	style.FrameRounding		= 4.0f;
}

void ImguiEditor::Update() {
	if (queueResize) {
		PerformResize();
		return;
	}

	if (!imguiRenderer->PreRender()) {
		return;
	}

	viewportPanel->RenderCamera(imguiRenderer->GetCommandBuffer());
	imguiRenderer->PrepareImguiRendering();
	Render();
	imguiRenderer->PostRender();
}

void ImguiEditor::Render() {
	RenderDockspace();
	tracingPanel->Render();
	controlBar->Render();
	sceneHeirarchyPanel->Render();
	viewportPanel->Render();
	consolePanel->Render();
	assetBrowserPanel->Render();
	inspectorPanel->Render();
	systemPanel->Render();
	statsPanel->Render();
	buildPopup->Render();
	userSettingsWindow->Render();
	projectSettingsWindow->Render();
	pluginsWindow->Render();
	statusBar->Render();
	assetPicker->Render();
}

void ImguiEditor::PromptAssetPicker(AssetType assetType, AssetPicker::AssetPickerCallback callback) {
	assetPicker->OpenPrompt(assetType, callback);
}

void ImguiEditor::StartBuild() {
	buildPopup->StartBuild();
}

void ImguiEditor::ImportFile(const char* folderPathToImportTo) {
	auto window = engineCore->windowManager->GetWindowByIndex(0);
	auto filePath = window->OpenFileDialogue();
}

ViewportPanel* Grindstone::Editor::ImguiEditor::ImguiEditor::GetViewportPanel() {
	return viewportPanel;
}

const ImGui::MarkdownConfig& Grindstone::Editor::ImguiEditor::ImguiEditor::GetMarkdownConfig() const {
	return markdownConfig;
}

ImguiRenderer& Grindstone::Editor::ImguiEditor::ImguiEditor::GetImguiRenderer() {
	return *imguiRenderer;
}

ImFont* Grindstone::Editor::ImguiEditor::ImguiEditor::GetFont(FontType type) const {
	size_t index = static_cast<size_t>(type);
	return fonts[index];
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
		ImGui::DockBuilderDockWindow("Tracing", dockIdConsoleArea);
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
