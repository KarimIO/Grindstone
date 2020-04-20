#if 0
#include "..\..\Engine\AssetManagers\ImguiManager.hpp"
#include "ImguiManager.hpp"

#include <dear_imgui/imgui.h>
#include <dear_imgui/examples/imgui_impl_opengl3.h>

#include <iostream>

#include <WindowModule/Win32Window.hpp>
#include <GL/gl3w.h>

ImguiManager* imgui_mgr;
bool g_WantUpdateMonitors;
BaseWindow* g_window;

static BOOL CALLBACK ImGui_ImplGrindstone_UpdateMonitors_EnumFunc(HMONITOR monitor, HDC, LPRECT, LPARAM)
{
	MONITORINFO info = { 0 };
	info.cbSize = sizeof(MONITORINFO);
	if (!::GetMonitorInfo(monitor, &info))
		return TRUE;
	ImGuiPlatformMonitor imgui_monitor;
	imgui_monitor.MainPos = ImVec2((float)info.rcMonitor.left, (float)info.rcMonitor.top);
	imgui_monitor.MainSize = ImVec2((float)(info.rcMonitor.right - info.rcMonitor.left), (float)(info.rcMonitor.bottom - info.rcMonitor.top));
	imgui_monitor.WorkPos = ImVec2((float)info.rcWork.left, (float)info.rcWork.top);
	imgui_monitor.WorkSize = ImVec2((float)(info.rcWork.right - info.rcWork.left), (float)(info.rcWork.bottom - info.rcWork.top));
	imgui_monitor.DpiScale = 1.0f; // ImGui_ImplGrindstone_GetDpiScaleForMonitor(monitor);
	ImGuiPlatformIO& io = ImGui::GetPlatformIO();
	if (info.dwFlags & MONITORINFOF_PRIMARY)
		io.Monitors.push_front(imgui_monitor);
	else
		io.Monitors.push_back(imgui_monitor);
	return TRUE;
}

static void ImGui_ImplGrindstone_UpdateMonitors()
{
	ImGui::GetPlatformIO().Monitors.resize(0);
	::EnumDisplayMonitors(NULL, NULL, ImGui_ImplGrindstone_UpdateMonitors_EnumFunc, NULL);
	g_WantUpdateMonitors = false;
}

/*
void ImGui_ImplGrindstone_NewFrame()
{
	ImGuiIO& io = ImGui::GetIO();
	IM_ASSERT(io.Fonts->IsBuilt() && "Font atlas not built! It is generally built by the renderer back-end. Missing call to renderer _NewFrame() function? e.g. ImGui_ImplOpenGL3_NewFrame().");

	// Setup display size (every frame to accommodate for window resizing)
	int w, h;
	int display_w, display_h;
	GrindstoneGetWindowSize(g_Window, &w, &h);
	GrindstoneGetFramebufferSize(g_Window, &display_w, &display_h);
	io.DisplaySize = ImVec2((float)w, (float)h);
	if (w > 0 && h > 0)
		io.DisplayFramebufferScale = ImVec2((float)display_w / w, (float)display_h / h);
	if (g_WantUpdateMonitors)
		ImGui_ImplGrindstone_UpdateMonitors();

	// Setup time step
	double current_time = GrindstoneGetTime();
	io.DeltaTime = g_Time > 0.0 ? (float)(current_time - g_Time) : (float)(1.0f / 60.0f);
	g_Time = current_time;

	ImGui_ImplGrindstone_UpdateMousePosAndButtons();
	ImGui_ImplGrindstone_UpdateMouseCursor();

	// Update game controllers (if enabled and available)
	ImGui_ImplGrindstone_UpdateGamepads();
}
*/

//--------------------------------------------------------------------------------------------------------
// MULTI-VIEWPORT / PLATFORM INTERFACE SUPPORT
// This is an _advanced_ and _optional_ feature, allowing the back-end to create and handle multiple viewports simultaneously.
// If you are new to dear imgui or creating a new binding for dear imgui, it is recommended that you completely ignore this section first..
//--------------------------------------------------------------------------------------------------------

struct ImGuiViewportDataGrindstone
{
	BaseWindow* window;
	bool        window_owned;
	int         ignore_window_size_event_frame;

	ImGuiViewportDataGrindstone() { window = NULL; window_owned = false; ignore_window_size_event_frame = -1; }
	~ImGuiViewportDataGrindstone() { IM_ASSERT(window == NULL); }
};

static void ImGui_ImplGrindstone_WindowCloseCallback(BaseWindow* window)
{
	if (ImGuiViewport* viewport = ImGui::FindViewportByPlatformHandle(window))
		viewport->PlatformRequestClose = true;
}

static void ImGui_ImplGrindstone_WindowPosCallback(BaseWindow* window, int, int)
{
	if (ImGuiViewport* viewport = ImGui::FindViewportByPlatformHandle(window))
		viewport->PlatformRequestMove = true;
}

static void ImGui_ImplGrindstone_WindowSizeCallback(BaseWindow* window, int, int)
{
	if (ImGuiViewport* viewport = ImGui::FindViewportByPlatformHandle(window))
	{
		if (ImGuiViewportDataGrindstone* data = (ImGuiViewportDataGrindstone*)viewport->PlatformUserData)
		{
			// Grindstone may dispatch window size event after calling GrindstoneSetWindowSize().
			// However depending on the platform the callback may be invoked at different time: on Windows it
			// appears to be called within the GrindstoneSetWindowSize() call whereas on Linux it is queued and invoked
			// during GrindstonePollEvents().
			// Because the event doesn't always fire on GrindstoneSetWindowSize() we use a frame counter tag to only
			// ignore recent GrindstoneSetWindowSize() calls.
			bool ignore_event = (ImGui::GetFrameCount() <= data->ignore_window_size_event_frame + 1);
			data->ignore_window_size_event_frame = -1;
			if (ignore_event)
				return;
		}
		viewport->PlatformRequestResize = true;
	}
}

static void ImGui_ImplGrindstone_CreateWindow(ImGuiViewport* viewport)
{
	ImGuiViewportDataGrindstone* data = IM_NEW(ImGuiViewportDataGrindstone)();
	viewport->PlatformUserData = data;
	data->window = new Win32Window();
	WindowCreateInfo ci;
	ci.fullscreen = WindowFullscreenMode::Borderless;
	ci.positioning_anchor = WindowPositioningAnchor::TopLeft;
	ci.positioning_unit = WindowPositioningUnit::Pixels;
	ci.offset_left = (double)viewport->Pos.x;
	ci.offset_top = (double)viewport->Pos.y;
	ci.sizing_unit = WindowSizingUnit::Pixels;
	ci.sizing_width = viewport->Size.x;
	ci.sizing_height = viewport->Size.y;
	ci.title = "Untitled";
	ci.input_interface = imgui_mgr;
	data->window->initialize(ci);
	g_window = data->window;

	// Grindstone 3.2 unfortunately always set focus on GrindstoneCreateWindow() if Grindstone_VISIBLE is set, regardless of Grindstone_FOCUSED
	// With Grindstone 3.3, the hint Grindstone_FOCUS_ON_SHOW fixes this problem
	/*GrindstoneWindowHint(Grindstone_VISIBLE, false);
	GrindstoneWindowHint(Grindstone_FOCUSED, false);
#if Grindstone_HAS_FOCUS_ON_SHOW
	GrindstoneWindowHint(Grindstone_FOCUS_ON_SHOW, false);
#endif
	GrindstoneWindowHint(Grindstone_DECORATED, (viewport->Flags & ImGuiViewportFlags_NoDecoration) ? false : true);
#if Grindstone_HAS_WINDOW_TOPMOST
	GrindstoneWindowHint(Grindstone_FLOATING, (viewport->Flags & ImGuiViewportFlags_TopMost) ? true : false);
#endif
	BaseWindow* share_window = (g_ClientApi == GrindstoneClientApi_OpenGL) ? g_Window : NULL;
	data->Window = GrindstoneCreateWindow((int)viewport->Size.x, (int)viewport->Size.y, "No Title Yet", NULL, share_window);
	data->WindowOwned = true;
	viewport->PlatformHandle = (void*)data->Window;
#ifdef _WIN32
	viewport->PlatformHandleRaw = GrindstoneGetWin32Window(data->Window);
#endif
	GrindstoneSetWindowPos(data->Window, (int)viewport->Pos.x, (int)viewport->Pos.y);

	// Install callbacks for secondary viewports
	GrindstoneSetMouseButtonCallback(data->Window, ImGui_ImplGrindstone_MouseButtonCallback);
	GrindstoneSetScrollCallback(data->Window, ImGui_ImplGrindstone_ScrollCallback);
	GrindstoneSetKeyCallback(data->Window, ImGui_ImplGrindstone_KeyCallback);
	GrindstoneSetCharCallback(data->Window, ImGui_ImplGrindstone_CharCallback);
	GrindstoneSetWindowCloseCallback(data->Window, ImGui_ImplGrindstone_WindowCloseCallback);
	GrindstoneSetWindowPosCallback(data->Window, ImGui_ImplGrindstone_WindowPosCallback);
	GrindstoneSetWindowSizeCallback(data->Window, ImGui_ImplGrindstone_WindowSizeCallback);
	if (g_ClientApi == GrindstoneClientApi_OpenGL)
	{
		GrindstoneMakeContextCurrent(data->Window);
		GrindstoneSwapInterval(0);
	}*/
}

static void ImGui_ImplGrindstone_DestroyWindow(ImGuiViewport* viewport)
{
	if (ImGuiViewportDataGrindstone* data = (ImGuiViewportDataGrindstone*)viewport->PlatformUserData)
	{
		if (data->window_owned) {
/*#if Grindstone_HAS_Grindstone_HOVERED && defined(_WIN32)
			HWND hwnd = (HWND)viewport->PlatformHandleRaw;
			::RemovePropA(hwnd, "IMGUI_VIEWPORT");
#endif*/
			data->window->close();
		}
		data->window = NULL;
		IM_DELETE(data);
	}
	viewport->PlatformUserData = viewport->PlatformHandle = NULL;
}

// FIXME-VIEWPORT: Implement same work-around for Linux/OSX in the meanwhile.
#if defined(_WIN32) && Grindstone_HAS_Grindstone_HOVERED
static WNDPROC g_GrindstoneWndProc = NULL;
static LRESULT CALLBACK WndProcNoInputs(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	if (msg == WM_NCHITTEST)
	{
		// Let mouse pass-through the window. This will allow the back-end to set io.MouseHoveredViewport properly (which is OPTIONAL).
		// The ImGuiViewportFlags_NoInputs flag is set while dragging a viewport, as want to detect the window behind the one we are dragging.
		// If you cannot easily access those viewport flags from your windowing/event code: you may manually synchronize its state e.g. in
		// your main loop after calling UpdatePlatformWindows(). Iterate all viewports/platform windows and pass the flag to your windowing system.
		ImGuiViewport* viewport = (ImGuiViewport*)::GetPropA(hWnd, "IMGUI_VIEWPORT");
		if (viewport->Flags & ImGuiViewportFlags_NoInputs)
			return HTTRANSPARENT;
	}
	return ::CallWindowProc(g_GrindstoneWndProc, hWnd, msg, wParam, lParam);
}
#endif

static void ImGui_ImplGrindstone_ShowWindow(ImGuiViewport* viewport) {
	
	ImGuiViewportDataGrindstone* data = (ImGuiViewportDataGrindstone*)viewport->PlatformUserData;
	/*
#if defined(_WIN32)
	// Grindstone hack: Hide icon from task bar
	HWND hwnd = (HWND)viewport->PlatformHandleRaw;
	if (viewport->Flags & ImGuiViewportFlags_NoTaskBarIcon)
	{
		LONG ex_style = ::GetWindowLong(hwnd, GWL_EXSTYLE);
		ex_style &= ~WS_EX_APPWINDOW;
		ex_style |= WS_EX_TOOLWINDOW;
		::SetWindowLong(hwnd, GWL_EXSTYLE, ex_style);
	}

	// Grindstone hack: install hook for WM_NCHITTEST message handler
#if Grindstone_HAS_Grindstone_HOVERED && defined(_WIN32)
	::SetPropA(hwnd, "IMGUI_VIEWPORT", viewport);
	if (g_GrindstoneWndProc == NULL)
		g_GrindstoneWndProc = (WNDPROC)::GetWindowLongPtr(hwnd, GWLP_WNDPROC);
	::SetWindowLongPtr(hwnd, GWLP_WNDPROC, (LONG_PTR)WndProcNoInputs);
#endif

#if !Grindstone_HAS_FOCUS_ON_SHOW
	// Grindstone hack: Grindstone 3.2 has a bug where GrindstoneShowWindow() also activates/focus the window.
	// The fix was pushed to Grindstone repository on 2018/01/09 and should be included in Grindstone 3.3 via a Grindstone_FOCUS_ON_SHOW window attribute.
	// See https://github.com/Grindstone/Grindstone/issues/1189
	// FIXME-VIEWPORT: Implement same work-around for Linux/OSX in the meanwhile.
	if (viewport->Flags & ImGuiViewportFlags_NoFocusOnAppearing)
	{
		::ShowWindow(hwnd, SW_SHOWNA);
		return;
	}
#endif
#endif*/

	data->window->show();
}

static ImVec2 ImGui_ImplGrindstone_GetWindowPos(ImGuiViewport* viewport) {
	ImGuiViewportDataGrindstone* data = (ImGuiViewportDataGrindstone*)viewport->PlatformUserData;
	unsigned int x = 0, y = 0;
	data->window->getWindowPos(x, y);
	return ImVec2((float)x, (float)y);
}

static void ImGui_ImplGrindstone_SetWindowPos(ImGuiViewport* viewport, ImVec2 pos) {
	ImGuiViewportDataGrindstone* data = (ImGuiViewportDataGrindstone*)viewport->PlatformUserData;
	data->window->setWindowPos(pos.x, pos.y);
}

static ImVec2 ImGui_ImplGrindstone_GetWindowSize(ImGuiViewport* viewport)
{
	ImGuiViewportDataGrindstone* data = (ImGuiViewportDataGrindstone*)viewport->PlatformUserData;
	unsigned int x = 0, y = 0;
	data->window->getWindowSize(x, y);
	return ImVec2((float)x, (float)y);
}

static void ImGui_ImplGrindstone_SetWindowSize(ImGuiViewport* viewport, ImVec2 size)
{
	ImGuiViewportDataGrindstone* data = (ImGuiViewportDataGrindstone*)viewport->PlatformUserData;
/*#if __APPLE__
	// Native OS windows are positioned from the bottom-left corner on macOS, whereas on other platforms they are
	// positioned from the upper-left corner. Grindstone makes an effort to convert macOS style coordinates, however it
	// doesn't handle it when changing size. We are manually moving the window in order for changes of size to be based
	// on the upper-left corner.
	int x, y, width, height;
	GrindstoneGetWindowPos(data->Window, &x, &y);
	GrindstoneGetWindowSize(data->Window, &width, &height);
	GrindstoneSetWindowPos(data->Window, x, y - height + size.y);
#endif*/
	data->ignore_window_size_event_frame = ImGui::GetFrameCount();
	data->window->setWindowSize((int)size.x, (int)size.y);
}

static void ImGui_ImplGrindstone_SetWindowTitle(ImGuiViewport* viewport, const char* title) {
	ImGuiViewportDataGrindstone* data = (ImGuiViewportDataGrindstone*)viewport->PlatformUserData;
	data->window->setWindowTitle(title);
}

static void ImGui_ImplGrindstone_SetWindowFocus(ImGuiViewport* viewport) {
	ImGuiViewportDataGrindstone* data = (ImGuiViewportDataGrindstone*)viewport->PlatformUserData;
	data->window->setWindowFocus();
	g_window = data->window;
}

static bool ImGui_ImplGrindstone_GetWindowFocus(ImGuiViewport* viewport) {
	ImGuiViewportDataGrindstone* data = (ImGuiViewportDataGrindstone*)viewport->PlatformUserData;
	return data->window->getWindowFocus();
}

static bool ImGui_ImplGrindstone_GetWindowMinimized(ImGuiViewport* viewport) {
	ImGuiViewportDataGrindstone* data = (ImGuiViewportDataGrindstone*)viewport->PlatformUserData;
	return data->window->getWindowMinimized();
}

static void ImGui_ImplGrindstone_SetWindowAlpha(ImGuiViewport* viewport, float alpha) {
	ImGuiViewportDataGrindstone* data = (ImGuiViewportDataGrindstone*)viewport->PlatformUserData;
	data->window->setWindowAlpha(alpha);
}

static void ImGui_ImplGrindstone_RenderWindow(ImGuiViewport* viewport, void*) {
	ImGuiViewportDataGrindstone* data = (ImGuiViewportDataGrindstone*)viewport->PlatformUserData;
	data->window->setAsCurrentWindow();
}

static void ImGui_ImplGrindstone_SwapBuffers(ImGuiViewport* viewport, void*)
{
	ImGuiViewportDataGrindstone* data = (ImGuiViewportDataGrindstone*)viewport->PlatformUserData;
	data->window->setAsCurrentWindow(); 
	data->window->swapBuffers();
}

#include <Engine/Core/Input.hpp>

void ImguiManager::initialize(BaseWindow* win) {
	GRIND_PROFILE_FUNC();
    IMGUI_CHECKVERSION();

	imgui_mgr = this;

    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;
	io.IniFilename = NULL;
	io.DisplaySize = ImVec2(1024, 740);
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;  // Enable Keyboard Controls
    //io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;   // Enable Gamepad Controls
	io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;           // Enable Docking
	//io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;

    // Setup Dear ImGui style
    ImGui::StyleColorsDark();
	ImGuiStyle& style = ImGui::GetStyle(); 
	if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable) {
		style.WindowRounding = 0.0f;
		style.Colors[ImGuiCol_WindowBg].w = 1.0f;
	}

    //ImguiManager::InitForOpenGL(window, true);

	io.BackendFlags |= ImGuiBackendFlags_HasMouseCursors;         // We can honor GetMouseCursor() values (optional)
	io.BackendFlags |= ImGuiBackendFlags_HasSetMousePos;          // We can honor io.WantSetMousePos requests (optional, rarely used)
	//io.BackendFlags |= ImGuiBackendFlags_PlatformHasViewports;    // We can create multi-viewports on the Platform side (optional)
	io.BackendFlags |= ImGuiBackendFlags_HasMouseHoveredViewport; // We can set io.MouseHoveredViewport correctly (optional, not easy)
	io.BackendPlatformName = "imgui_impl_grindstone";

	// Keyboard mapping. ImGui will use those indices to peek into the io.KeysDown[] array.
	io.KeyMap[ImGuiKey_Tab] = KEY_TAB;
	io.KeyMap[ImGuiKey_LeftArrow] = KEY_LEFT;
	io.KeyMap[ImGuiKey_RightArrow] = KEY_RIGHT;
	io.KeyMap[ImGuiKey_UpArrow] = KEY_UP;
	io.KeyMap[ImGuiKey_DownArrow] = KEY_DOWN;
	io.KeyMap[ImGuiKey_PageUp] = KEY_PG_UP;
	io.KeyMap[ImGuiKey_PageDown] = KEY_PG_DOWN;
	io.KeyMap[ImGuiKey_Home] = KEY_HOME;
	io.KeyMap[ImGuiKey_End] = KEY_END;
	io.KeyMap[ImGuiKey_Insert] = KEY_INSERT;
	io.KeyMap[ImGuiKey_Delete] = KEY_DELETE;
	io.KeyMap[ImGuiKey_Backspace] = KEY_BACKSPACE;
	io.KeyMap[ImGuiKey_Space] = KEY_SPACE;
	io.KeyMap[ImGuiKey_Enter] = KEY_ENTER;
	io.KeyMap[ImGuiKey_Escape] = KEY_ESCAPE;
	io.KeyMap[ImGuiKey_KeyPadEnter] = KEY_NUMPAD_ENTER;
	io.KeyMap[ImGuiKey_A] = KEY_A;
	io.KeyMap[ImGuiKey_C] = KEY_C;
	io.KeyMap[ImGuiKey_V] = KEY_V;
	io.KeyMap[ImGuiKey_X] = KEY_X;
	io.KeyMap[ImGuiKey_Y] = KEY_Y;
	io.KeyMap[ImGuiKey_Z] = KEY_Z;

	//io.SetClipboardTextFn = ImGui_ImplGrindstone_SetClipboardText;
	//io.GetClipboardTextFn = ImGui_ImplGrindstone_GetClipboardText;
	//io.ClipboardUserData = g_Window;

	/*g_MouseCursors[ImGuiMouseCursor_Arrow] = GrindstoneCreateStandardCursor(Grindstone_ARROW_CURSOR);
	g_MouseCursors[ImGuiMouseCursor_TextInput] = GrindstoneCreateStandardCursor(Grindstone_IBEAM_CURSOR);
	g_MouseCursors[ImGuiMouseCursor_ResizeNS] = GrindstoneCreateStandardCursor(Grindstone_VRESIZE_CURSOR);
	g_MouseCursors[ImGuiMouseCursor_ResizeEW] = GrindstoneCreateStandardCursor(Grindstone_HRESIZE_CURSOR);
	g_MouseCursors[ImGuiMouseCursor_Hand] = GrindstoneCreateStandardCursor(Grindstone_HAND_CURSOR);
#if Grindstone_HAS_NEW_CURSORS
	g_MouseCursors[ImGuiMouseCursor_ResizeAll] = GrindstoneCreateStandardCursor(Grindstone_RESIZE_ALL_CURSOR);
	g_MouseCursors[ImGuiMouseCursor_ResizeNESW] = GrindstoneCreateStandardCursor(Grindstone_RESIZE_NESW_CURSOR);
	g_MouseCursors[ImGuiMouseCursor_ResizeNWSE] = GrindstoneCreateStandardCursor(Grindstone_RESIZE_NWSE_CURSOR);
	g_MouseCursors[ImGuiMouseCursor_NotAllowed] = GrindstoneCreateStandardCursor(Grindstone_NOT_ALLOWED_CURSOR);
#else
	g_MouseCursors[ImGuiMouseCursor_ResizeAll] = GrindstoneCreateStandardCursor(Grindstone_ARROW_CURSOR);
	g_MouseCursors[ImGuiMouseCursor_ResizeNESW] = GrindstoneCreateStandardCursor(Grindstone_ARROW_CURSOR);
	g_MouseCursors[ImGuiMouseCursor_ResizeNWSE] = GrindstoneCreateStandardCursor(Grindstone_ARROW_CURSOR);
	g_MouseCursors[ImGuiMouseCursor_NotAllowed] = GrindstoneCreateStandardCursor(Grindstone_ARROW_CURSOR);
#endif*/

	// Chain Grindstone callbacks: our callbacks will call the user's previously installed callbacks, if any.
	/*g_PrevUserCallbackMousebutton = NULL;
	g_PrevUserCallbackScroll = NULL;
	g_PrevUserCallbackKey = NULL;
	g_PrevUserCallbackChar = NULL;*/
	
	//g_InstalledCallbacks = true;
	/*
	g_PrevUserCallbackMousebutton = GrindstoneSetMouseButtonCallback(window, ImGui_ImplGrindstone_MouseButtonCallback);
	g_PrevUserCallbackScroll = GrindstoneSetScrollCallback(window, ImGui_ImplGrindstone_ScrollCallback);
	g_PrevUserCallbackKey = GrindstoneSetKeyCallback(window, ImGui_ImplGrindstone_KeyCallback);
	g_PrevUserCallbackChar = GrindstoneSetCharCallback(window, ImGui_ImplGrindstone_CharCallback);
	*/

	// Our mouse update function expect PlatformHandle to be filled for the main viewport
	/*ImGuiViewport* main_viewport = ImGui::GetMainViewport();
	main_viewport->PlatformHandle = (void*)g_Window;
#ifdef _WIN32
	main_viewport->PlatformHandleRaw = GrindstoneGetWin32Window(g_Window);
#endif*/


	/*window_ = new Win32Window();
	WindowCreateInfo ci;
	ci.fullscreen = WindowFullscreenMode::Borderless;
	ci.positioning_anchor = WindowPositioningAnchor::TopLeft;
	ci.positioning_unit = WindowPositioningUnit::Pixels;
	ci.offset_left = 0.0;
	ci.offset_top = 0.0;
	ci.sizing_unit = WindowSizingUnit::Percentage0to1;
	ci.sizing_width = 1.0;
	ci.sizing_height = 1.0;
	ci.title = "Temp";
	ci.input_interface = this;
	window_->initialize(ci);*/
	window_ = win;

	if (gl3wInit() != 0) {
		fprintf(stderr, "Failed to initialize OpenGL loader!\n");
		return;
	}

	if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable) {
		ImGuiPlatformIO& platform_io = ImGui::GetPlatformIO();
		platform_io.Platform_CreateWindow = ImGui_ImplGrindstone_CreateWindow;
		platform_io.Platform_DestroyWindow = ImGui_ImplGrindstone_DestroyWindow;
		platform_io.Platform_ShowWindow = ImGui_ImplGrindstone_ShowWindow;
		platform_io.Platform_SetWindowPos = ImGui_ImplGrindstone_SetWindowPos;
		platform_io.Platform_GetWindowPos = ImGui_ImplGrindstone_GetWindowPos;
		platform_io.Platform_SetWindowSize = ImGui_ImplGrindstone_SetWindowSize;
		platform_io.Platform_GetWindowSize = ImGui_ImplGrindstone_GetWindowSize;
		platform_io.Platform_SetWindowFocus = ImGui_ImplGrindstone_SetWindowFocus;
		platform_io.Platform_GetWindowFocus = ImGui_ImplGrindstone_GetWindowFocus;
		platform_io.Platform_GetWindowMinimized = ImGui_ImplGrindstone_GetWindowMinimized;
		platform_io.Platform_SetWindowTitle = ImGui_ImplGrindstone_SetWindowTitle;
		platform_io.Platform_RenderWindow = ImGui_ImplGrindstone_RenderWindow;
		platform_io.Platform_SwapBuffers = ImGui_ImplGrindstone_SwapBuffers;
		platform_io.Platform_SetWindowAlpha = ImGui_ImplGrindstone_SetWindowAlpha;
		//platform_io.Platform_UpdateWindow = ImGui_ImplWin32_UpdateWindow;
		//platform_io.Platform_GetWindowDpiScale = ImGui_ImplWin32_GetWindowDpiScale; // FIXME-DPI
		//platform_io.Platform_OnChangedViewport = ImGui_ImplWin32_OnChangedViewport; // FIXME-DPI

#if Grindstone_HAS_VULKAN
		platform_io.Platform_CreateVkSurface = ImGui_ImplGrindstone_CreateVkSurface;
#endif
#if HAS_WIN32_IME
		platform_io.Platform_SetImeInputPos = ImGui_ImplWin32_SetImeInputPos;
#endif

		// Note: monitor callback are broken Grindstone 3.2 and earlier (see github.com/Grindstone/Grindstone/issues/784)
		//ImGui_ImplGrindstone_UpdateMonitors();
		//GrindstoneSetMonitorCallback(ImGui_ImplGrindstone_MonitorCallback);

		// Register main window handle (which is owned by the main application, not by us)

		ImGuiViewport* main_viewport = ImGui::GetMainViewport();
		ImGuiViewportDataGrindstone* data = IM_NEW(ImGuiViewportDataGrindstone)();
		data->window = window_;
		data->window_owned = false;
		main_viewport->PlatformUserData = data;
		main_viewport->PlatformHandle = (void*)window_;
	}

	ImGui_ImplOpenGL3_Init("#version 420");
	window_->show();

	glClearColor(0.3f, 0.6f, 0.9f, 1.0f);
}

void ImguiManager::swapBuffers() {
	ImGui::Render();
	ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
	ImGui::UpdatePlatformWindows();
	ImGui::RenderPlatformWindowsDefault();
	window_->swapBuffers();
}

ImguiManager::~ImguiManager()
{
	ImGui_ImplOpenGL3_Shutdown();
}

BaseWindow* ImguiManager::getWindow() {
	return window_;
}

#define Grindstone_HAS_WINDOW_TOPMOST     (Grindstone_VERSION_MAJOR * 1000 + Grindstone_VERSION_MINOR * 100 >= 3200) // 3.2+ Grindstone_FLOATING
#define Grindstone_HAS_WINDOW_HOVERED     (Grindstone_VERSION_MAJOR * 1000 + Grindstone_VERSION_MINOR * 100 >= 3300) // 3.3+ Grindstone_HOVERED
#define Grindstone_HAS_WINDOW_ALPHA       (Grindstone_VERSION_MAJOR * 1000 + Grindstone_VERSION_MINOR * 100 >= 3300) // 3.3+ GrindstoneSetWindowOpacity
#define Grindstone_HAS_PER_MONITOR_DPI    (Grindstone_VERSION_MAJOR * 1000 + Grindstone_VERSION_MINOR * 100 >= 3300) // 3.3+ GrindstoneGetMonitorContentScale
#define Grindstone_HAS_VULKAN             (Grindstone_VERSION_MAJOR * 1000 + Grindstone_VERSION_MINOR * 100 >= 3200) // 3.2+ GrindstoneCreateWindowSurface

void (*g_PrevUserCallbackMousebutton)(int button, int action, int mods);
void (*g_PrevUserCallbackScroll)(double xoffset, double yoffset);
void (*g_PrevUserCallbackKey)(int key, int scancode, int action, int mods);
void(*g_PrevUserCallbackChar)(unsigned int c);

static double               g_Time = 0.0;
static bool                 g_MouseJustPressed[5] = { false, false, false, false, false };

const char *GetClipboardText(void* user_data) {
	return ""; // GrindstoneGetClipboardString((BaseWindow*)user_data);
}

void SetClipboardText(void* user_data, const char* text) {
	//GrindstoneSetClipboardString((BaseWindow*)user_data, text);
}

void ImguiManager::ResizeEvent(int x, int y) {

}

void ImguiManager::SetMouseButton(int button, bool state) {
	ImGuiIO& io = ImGui::GetIO();

	if (g_PrevUserCallbackMousebutton != NULL)
		g_PrevUserCallbackMousebutton(button, state, 0);

	if (button >= 0 && button < IM_ARRAYSIZE(g_MouseJustPressed)) {
		g_MouseJustPressed[button] = state;
		io.MouseDown[button] = state;
	}
}

void ImguiManager::SetMousePosition(int x, int y) {
	/*ImGuiIO& io = ImGui::GetIO();
	for (int i = 0; i < IM_ARRAYSIZE(io.MouseDown); i++)
	{
		// If a mouse press event came, always pass it as "mouse held this frame", so we don't miss click-release events that are shorter than 1 frame.
		bool mousebtn = engine.getInputManager()->GetMouseButton(i) > 0;
		io.MouseDown[i] = g_MouseJustPressed[i] || mousebtn;
		g_MouseJustPressed[i] = false;
	}

	// Update mouse position
	const ImVec2 mouse_pos_backup = io.MousePos;
	io.MousePos = ImVec2(-FLT_MAX, -FLT_MAX);

	if (true) {
		if (io.WantSetMousePos) {
			//engine.getInputManager()->SetMousePosition(int(mouse_pos_backup.x), int(mouse_pos_backup.y));
		}
		else {
			io.MousePos = ImVec2((float)x, (float)y);
		}
	}*/
}

void ImguiManager::SetFocused(bool) {

}

bool ImguiManager::IsFocused() {
	return true;
}

void ImguiManager::SetKey(int key, bool state) {
	ImGuiIO& io = ImGui::GetIO();
	if (state)
		io.KeysDown[key] = true;
	else
		io.KeysDown[key] = false;

	if (g_PrevUserCallbackChar != NULL)
		g_PrevUserCallbackChar(key);

	if (state)
		io.AddInputCharacter((unsigned short)state);
}

void ImguiManager::Quit() {
}

void ImguiManager::ForceQuit() {
	Quit();
}

/*
void ImguiManager::ScrollCallback(double xoffset, double yoffset) {
	if (g_PrevUserCallbackScroll != NULL)
		g_PrevUserCallbackScroll(xoffset, yoffset);

	ImGuiIO& io = ImGui::GetIO();
	io.MouseWheelH += (float)xoffset;
	io.MouseWheel += (float)yoffset;
}


void ImguiManager::CharCallback(unsigned int c)
{
	if (g_PrevUserCallbackChar != NULL)
		g_PrevUserCallbackChar(c);

	ImGuiIO& io = ImGui::GetIO();
	if (c > 0 && c < 0x10000)
		io.AddInputCharacter((unsigned short)c);
}
*/

void ImguiManager::Shutdown()
{
	/*for (ImGuiMouseCursor cursor_n = 0; cursor_n < ImGuiMouseCursor_COUNT; cursor_n++)
	{
		g_MouseCursors[cursor_n] = NULL;
	}
	g_ClientApi = GrindstoneClientApi_Unknown;*/
}

void ImguiManager::UpdateMousePos() {
	ImGuiIO& io = ImGui::GetIO();

	// Set OS mouse position if requested (rarely used, only when ImGuiConfigFlags_NavEnableSetMousePos is enabled by user)
	// (When multi-viewports are enabled, all imgui positions are same as OS positions)
	/*if (io.WantSetMousePos) {
		window_->setMousePos();
		POINT pos = { (int)io.MousePos.x, (int)io.MousePos.y };
		if ((io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable) == 0)
			::ClientToScreen(g_hWnd, &pos);
		::SetCursorPos(pos.x, pos.y);
	}*/

	unsigned int x, y;
	// Set imgui mouse position
	window_->getMousePos(x, y);

	io.MousePos = ImVec2(x, y);
} 

void ImguiManager::UpdateMouseCursor() {
	/*ImGuiIO& io = ImGui::GetIO();
	if ((io.ConfigFlags & ImGuiConfigFlags_NoMouseCursorChange))
		return;

	ImGuiMouseCursor imgui_cursor = ImGui::GetMouseCursor();*/
}

/*static void ImguiManager::UpdateGamepads()
{
	ImGuiIO& io = ImGui::GetIO();
	memset(io.NavInputs, 0, sizeof(io.NavInputs));
	if ((io.ConfigFlags & ImGuiConfigFlags_NavEnableGamepad) == 0)
		return;

	// Update gamepad inputs
#define MAP_BUTTON(NAV_NO, BUTTON_NO)       { if (buttons_count > BUTTON_NO && buttons[BUTTON_NO] == Grindstone_PRESS) io.NavInputs[NAV_NO] = 1.0f; }
#define MAP_ANALOG(NAV_NO, AXIS_NO, V0, V1) { float v = (axes_count > AXIS_NO) ? axes[AXIS_NO] : V0; v = (v - V0) / (V1 - V0); if (v > 1.0f) v = 1.0f; if (io.NavInputs[NAV_NO] < v) io.NavInputs[NAV_NO] = v; }
	int axes_count = 0, buttons_count = 0;
	const float* axes = GrindstoneGetJoystickAxes(Grindstone_JOYSTICK_1, &axes_count);
	const unsigned char* buttons = GrindstoneGetJoystickButtons(Grindstone_JOYSTICK_1, &buttons_count);
	MAP_BUTTON(ImGuiNavInput_Activate, 0);     // Cross / A
	MAP_BUTTON(ImGuiNavInput_Cancel, 1);     // Circle / B
	MAP_BUTTON(ImGuiNavInput_Menu, 2);     // Square / X
	MAP_BUTTON(ImGuiNavInput_Input, 3);     // Triangle / Y
	MAP_BUTTON(ImGuiNavInput_DpadLeft, 13);    // D-Pad Left
	MAP_BUTTON(ImGuiNavInput_DpadRight, 11);    // D-Pad Right
	MAP_BUTTON(ImGuiNavInput_DpadUp, 10);    // D-Pad Up
	MAP_BUTTON(ImGuiNavInput_DpadDown, 12);    // D-Pad Down
	MAP_BUTTON(ImGuiNavInput_FocusPrev, 4);     // L1 / LB
	MAP_BUTTON(ImGuiNavInput_FocusNext, 5);     // R1 / RB
	MAP_BUTTON(ImGuiNavInput_TweakSlow, 4);     // L1 / LB
	MAP_BUTTON(ImGuiNavInput_TweakFast, 5);     // R1 / RB
	MAP_ANALOG(ImGuiNavInput_LStickLeft, 0, -0.3f, -0.9f);
	MAP_ANALOG(ImGuiNavInput_LStickRight, 0, +0.3f, +0.9f);
	MAP_ANALOG(ImGuiNavInput_LStickUp, 1, +0.3f, +0.9f);
	MAP_ANALOG(ImGuiNavInput_LStickDown, 1, -0.3f, -0.9f);
#undef MAP_BUTTON
#undef MAP_ANALOG
	if (axes_count > 0 && buttons_count > 0)
		io.BackendFlags |= ImGuiBackendFlags_HasGamepad;
	else
		io.BackendFlags &= ~ImGuiBackendFlags_HasGamepad;
}*/

void ImguiManager::newFrame(double time) {
	window_->setAsCurrentWindow();
	glClear(GL_COLOR_BUFFER_BIT);

	window_->handleEvents();

	ImGui_ImplOpenGL3_NewFrame();

	ImGuiIO& io = ImGui::GetIO();
	IM_ASSERT(io.Fonts->IsBuilt() && "Font atlas not built! It is generally built by the renderer back-end. Missing call to renderer _NewFrame() function? e.g. ImGui_ImplOpenGL3_NewFrame().");

	// Setup display size (every frame to accommodate for window resizing)
	unsigned int w, h;
	//int display_w, display_h;
	window_->getWindowSize(w, h);

	//glfwGetFramebufferSize(g_Window, &display_w, &display_h);
	io.DisplaySize = ImVec2((float)w, (float)h);

	//if (g_WantUpdateMonitors)
		ImGui_ImplGrindstone_UpdateMonitors();

	// Setup time step
	io.DeltaTime = g_Time > 0.0 ? (float)(time - g_Time) : (float)(1.0f / 60.0f);
	g_Time = time;

	UpdateMousePos();
	UpdateMouseCursor();

	// Gamepad navigation mapping
	// UpdateGamepads();

	ImGui::NewFrame();
}

#endif