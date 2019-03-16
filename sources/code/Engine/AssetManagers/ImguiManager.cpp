#include "ImguiManager.hpp"
#include "../Core/Engine.hpp"

#include "imgui/imgui.h"
#include "imgui/imgui_impl_opengl3.h"

#include "../Core/Input.hpp"

#include <iostream>

ImguiManager::ImguiManager() {
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;
	io.DisplaySize = ImVec2((float)engine.getSettings()->resolution_x_, (float)engine.getSettings()->resolution_y_);
    //io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;  // Enable Keyboard Controls
    //io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;   // Enable Gamepad Controls
	io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;       // Enable Keyboard Controls
	//io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls
	io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;           // Enable Docking
	io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;

    // Setup Dear ImGui style
    ImGui::StyleColorsClassic();
	ImGuiStyle& style = ImGui::GetStyle(); 
	/*ImGui::GetStyle().WindowRounding = 0.0f;// <- Set this on init or use ImGui::PushStyleVar()
	ImGui::GetStyle().ChildRounding = 0.0f;
	ImGui::GetStyle().FrameRounding = 0.0f;
	ImGui::GetStyle().GrabRounding = 0.0f;
	ImGui::GetStyle().PopupRounding = 0.0f;
	ImGui::GetStyle().ScrollbarRounding = 0.0f;*/
	if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
	{
		style.WindowRounding = 0.0f;
		style.Colors[ImGuiCol_WindowBg].w = 1.0f;
	}

    //ImguiManager::InitForOpenGL(window, true);
	ImGui_ImplOpenGL3_Init("#version 150");

	Init(false);
}

ImguiManager::~ImguiManager()
{
}

#define GLFW_HAS_WINDOW_TOPMOST     (GLFW_VERSION_MAJOR * 1000 + GLFW_VERSION_MINOR * 100 >= 3200) // 3.2+ GLFW_FLOATING
#define GLFW_HAS_WINDOW_HOVERED     (GLFW_VERSION_MAJOR * 1000 + GLFW_VERSION_MINOR * 100 >= 3300) // 3.3+ GLFW_HOVERED
#define GLFW_HAS_WINDOW_ALPHA       (GLFW_VERSION_MAJOR * 1000 + GLFW_VERSION_MINOR * 100 >= 3300) // 3.3+ glfwSetWindowOpacity
#define GLFW_HAS_PER_MONITOR_DPI    (GLFW_VERSION_MAJOR * 1000 + GLFW_VERSION_MINOR * 100 >= 3300) // 3.3+ glfwGetMonitorContentScale
#define GLFW_HAS_VULKAN             (GLFW_VERSION_MAJOR * 1000 + GLFW_VERSION_MINOR * 100 >= 3200) // 3.2+ glfwCreateWindowSurface

void (*g_PrevUserCallbackMousebutton)(int button, int action, int mods);
void (*g_PrevUserCallbackScroll)(double xoffset, double yoffset);
void (*g_PrevUserCallbackKey)(int key, int scancode, int action, int mods);
void(*g_PrevUserCallbackChar)(unsigned int c);

static double               g_Time = 0.0;
static bool                 g_MouseJustPressed[5] = { false, false, false, false, false };

const char *GetClipboardText(void* user_data) {
	return ""; // glfwGetClipboardString((GLFWwindow*)user_data);
}

void SetClipboardText(void* user_data, const char* text) {
	//glfwSetClipboardString((GLFWwindow*)user_data, text);
}

void ImguiManager::MouseButtonCallback(int button, int action, int mods) {
	if (g_PrevUserCallbackMousebutton != NULL)
		g_PrevUserCallbackMousebutton(button, action, mods);

	if (action >= 1 && button >= 0 && button < IM_ARRAYSIZE(g_MouseJustPressed))
		g_MouseJustPressed[button] = true;
}

void ImguiManager::ScrollCallback(double xoffset, double yoffset) {
	if (g_PrevUserCallbackScroll != NULL)
		g_PrevUserCallbackScroll(xoffset, yoffset);

	ImGuiIO& io = ImGui::GetIO();
	io.MouseWheelH += (float)xoffset;
	io.MouseWheel += (float)yoffset;
}

void ImguiManager::KeyCallback(int key, int action) {
	ImGuiIO& io = ImGui::GetIO();
	if (action >= 1)
		io.KeysDown[key] = true;
	if (action == 0)
		io.KeysDown[key] = false;

	// Modifiers are not reliable across systems
	io.KeyCtrl = io.KeysDown[KEY_LCONTROL] || io.KeysDown[KEY_CONTROL];
	io.KeyShift = io.KeysDown[KEY_LSHIFT] || io.KeysDown[KEY_SHIFT];
	io.KeyAlt = io.KeysDown[KEY_LALT] || io.KeysDown[KEY_ALT];
	io.KeySuper = io.KeysDown[KEY_WINDOW]; // || io.KeysDown[KEY_WINDOW];

	unsigned char keycode = 0;
	if (key >= KEY_A && key <= KEY_Z) {
		keycode = key - KEY_A + (io.KeyShift ? 'A' : 'a');
	}
	else if (key >= KEY_0 && key <= KEY_9) {
		keycode = key - KEY_0 + '0';
	}
	else if (key == KEY_SPACE) keycode = ' ';
	
	if (keycode > 0)
		CharCallback(keycode);
}

void ImguiManager::CharCallback(unsigned int c)
{
	if (g_PrevUserCallbackChar != NULL)
		g_PrevUserCallbackChar(c);

	ImGuiIO& io = ImGui::GetIO();
	if (c > 0 && c < 0x10000)
		io.AddInputCharacter((unsigned short)c);
}

void ImguiManager::Init(bool install_callbacks) {
	g_Time = 0.0;

	// Setup back-end capabilities flags
	ImGuiIO& io = ImGui::GetIO();
	io.BackendFlags |= ImGuiBackendFlags_HasMouseCursors;         // We can honor GetMouseCursor() values (optional)
	io.BackendFlags |= ImGuiBackendFlags_HasSetMousePos;          // We can honor io.WantSetMousePos requests (optional, rarely used)
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
	io.KeyMap[ImGuiKey_A] = KEY_A;
	io.KeyMap[ImGuiKey_C] = KEY_C;
	io.KeyMap[ImGuiKey_V] = KEY_V;
	io.KeyMap[ImGuiKey_X] = KEY_X;
	io.KeyMap[ImGuiKey_Y] = KEY_Y;
	io.KeyMap[ImGuiKey_Z] = KEY_Z;

	io.SetClipboardTextFn = SetClipboardText;
	io.GetClipboardTextFn = GetClipboardText;
	io.ClipboardUserData = nullptr; // g_Window;
/*#if defined(_WIN32)
	io.ImeWindowHandle = nullptr; // (void*)glfwGetWin32Window(g_Window);
#endif*/

	/*g_MouseCursors[ImGuiMouseCursor_Arrow] = glfwCreateStandardCursor(GLFW_ARROW_CURSOR);
	g_MouseCursors[ImGuiMouseCursor_TextInput] = glfwCreateStandardCursor(GLFW_IBEAM_CURSOR);
	g_MouseCursors[ImGuiMouseCursor_ResizeAll] = glfwCreateStandardCursor(GLFW_ARROW_CURSOR);   // FIXME: GLFW doesn't have this.
	g_MouseCursors[ImGuiMouseCursor_ResizeNS] = glfwCreateStandardCursor(GLFW_VRESIZE_CURSOR);
	g_MouseCursors[ImGuiMouseCursor_ResizeEW] = glfwCreateStandardCursor(GLFW_HRESIZE_CURSOR);
	g_MouseCursors[ImGuiMouseCursor_ResizeNESW] = glfwCreateStandardCursor(GLFW_ARROW_CURSOR);  // FIXME: GLFW doesn't have this.
	g_MouseCursors[ImGuiMouseCursor_ResizeNWSE] = glfwCreateStandardCursor(GLFW_ARROW_CURSOR);  // FIXME: GLFW doesn't have this.
	g_MouseCursors[ImGuiMouseCursor_Hand] = glfwCreateStandardCursor(GLFW_HAND_CURSOR);*/

	// Chain GLFW callbacks: our callbacks will call the user's previously installed callbacks, if any.
	g_PrevUserCallbackMousebutton = NULL;
	g_PrevUserCallbackScroll = NULL;
	g_PrevUserCallbackKey = NULL;
	g_PrevUserCallbackChar = NULL;

	/*TODO: Fix this
	if (install_callbacks) {
		g_PrevUserCallbackMousebutton = glfwSetMouseButtonCallback(window, ImguiManager::MouseButtonCallback);
		g_PrevUserCallbackScroll = glfwSetScrollCallback(window, ImguiManager::ScrollCallback);
		g_PrevUserCallbackKey = glfwSetKeyCallback(window, ImguiManager::KeyCallback);
		g_PrevUserCallbackChar = glfwSetCharCallback(window, ImguiManager::CharCallback);
	}*/

	// g_ClientApi = client_api;
}

void ImguiManager::Shutdown()
{
	/*for (ImGuiMouseCursor cursor_n = 0; cursor_n < ImGuiMouseCursor_COUNT; cursor_n++)
	{
		g_MouseCursors[cursor_n] = NULL;
	}
	g_ClientApi = GlfwClientApi_Unknown;*/
}

void ImguiManager::UpdateMousePosAndButtons() {
	// Update buttons
	ImGuiIO& io = ImGui::GetIO();
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

	if (engine.getInputManager()->IsFocused()) {
		if (io.WantSetMousePos) {
			engine.getInputManager()->SetMousePosition(mouse_pos_backup.x, mouse_pos_backup.y);
		}
		else {
			int mouse_x, mouse_y;
			engine.getInputManager()->GetMousePosition(mouse_x, mouse_y);
			io.MousePos = ImVec2((float)mouse_x, (float)mouse_y);
		}
	}
} 

void ImguiManager::UpdateMouseCursor() {
	ImGuiIO& io = ImGui::GetIO();
	if ((io.ConfigFlags & ImGuiConfigFlags_NoMouseCursorChange))
		return;

	ImGuiMouseCursor imgui_cursor = ImGui::GetMouseCursor();
}

/*static void ImguiManager::UpdateGamepads()
{
	ImGuiIO& io = ImGui::GetIO();
	memset(io.NavInputs, 0, sizeof(io.NavInputs));
	if ((io.ConfigFlags & ImGuiConfigFlags_NavEnableGamepad) == 0)
		return;

	// Update gamepad inputs
#define MAP_BUTTON(NAV_NO, BUTTON_NO)       { if (buttons_count > BUTTON_NO && buttons[BUTTON_NO] == GLFW_PRESS) io.NavInputs[NAV_NO] = 1.0f; }
#define MAP_ANALOG(NAV_NO, AXIS_NO, V0, V1) { float v = (axes_count > AXIS_NO) ? axes[AXIS_NO] : V0; v = (v - V0) / (V1 - V0); if (v > 1.0f) v = 1.0f; if (io.NavInputs[NAV_NO] < v) io.NavInputs[NAV_NO] = v; }
	int axes_count = 0, buttons_count = 0;
	const float* axes = glfwGetJoystickAxes(GLFW_JOYSTICK_1, &axes_count);
	const unsigned char* buttons = glfwGetJoystickButtons(GLFW_JOYSTICK_1, &buttons_count);
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

void ImguiManager::NewFrame()
{
	ImGuiIO& io = ImGui::GetIO();
	IM_ASSERT(io.Fonts->IsBuilt() && "Font atlas not built! It is generally built by the renderer back-end. Missing call to renderer _NewFrame() function? e.g. ImGui_ImplOpenGL3_NewFrame().");

	// Setup display size (every frame to accommodate for window resizing)
	int w, h;
	int display_w, display_h;
	w = display_w = engine.getSettings()->resolution_x_;
	h = display_h = engine.getSettings()->resolution_y_;
	//glfwGetWindowSize(g_Window, &w, &h);
	//glfwGetFramebufferSize(g_Window, &display_w, &display_h);
	io.DisplaySize = ImVec2((float)w, (float)h);
	io.DisplayFramebufferScale = ImVec2(w > 0 ? ((float)display_w / w) : 0, h > 0 ? ((float)display_h / h) : 0);
	
	// Setup time step
	double current_time = engine.getTimeCurrent();
	io.DeltaTime = g_Time > 0.0 ? (float)(current_time - g_Time) : (float)(1.0f / 60.0f);
	g_Time = current_time;

	UpdateMousePosAndButtons();
	UpdateMouseCursor();

	// Gamepad navigation mapping
	// UpdateGamepads();
}