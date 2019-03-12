// dear imgui: Platform Binding for Grindstone
// This needs to be used along with a Renderer (e.g. OpenGL3, Vulkan..)
// (Info: Grindstone is a cross-platform general purpose library for handling windows, inputs, OpenGL/Vulkan graphics context creation, etc.)
// (Requires: Grindstone 3.1+)

// Implemented features:
//  [X] Platform: Clipboard support.
//  [X] Platform: Gamepad support. Enable with 'io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad'.
//  [x] Platform: Mouse cursor shape and visibility. Disable with 'io.ConfigFlags |= ImGuiConfigFlags_NoMouseCursorChange'. FIXME: 3 cursors types are missing from Grindstone.
//  [X] Platform: Keyboard arrays indexed using Grindstone_KEY_* codes, e.g. ImGui::IsKeyPressed(Grindstone_KEY_SPACE).

// You can copy and use unmodified imgui_impl_* files in your project. See main.cpp for an example of using this.
// If you are new to dear imgui, read examples/README.txt and read the documentation at the top of imgui.cpp.
// https://github.com/ocornut/imgui

// CHANGELOG
// (minor and older changes stripped away, please see git history for details)
//  2018-11-30: Misc: Setting up io.BackendPlatformName so it can be displayed in the About Window.
//  2018-11-07: Inputs: When installing our Grindstone callbacks, we save user's previously installed ones - if any - and chain call them.
//  2018-08-01: Inputs: Workaround for Emscripten which doesn't seem to handle focus related calls.
//  2018-06-29: Inputs: Added support for the ImGuiMouseCursor_Hand cursor.
//  2018-06-08: Misc: Extracted imgui_impl_Grindstone.cpp/.h away from the old combined Grindstone+OpenGL/Vulkan examples.
//  2018-03-20: Misc: Setup io.BackendFlags ImGuiBackendFlags_HasMouseCursors flag + honor ImGuiConfigFlags_NoMouseCursorChange flag.
//  2018-02-20: Inputs: Added support for mouse cursors (ImGui::GetMouseCursor() value, passed to GrindstoneSetCursor()).
//  2018-02-06: Misc: Removed call to ImGui::Shutdown() which is not available from 1.60 WIP, user needs to call CreateContext/DestroyContext themselves.
//  2018-02-06: Inputs: Added mapping for ImGuiKey_Space.
//  2018-01-25: Inputs: Added gamepad support if ImGuiConfigFlags_NavEnableGamepad is set.
//  2018-01-25: Inputs: Honoring the io.WantSetMousePos by repositioning the mouse (when using navigation and ImGuiConfigFlags_NavMoveMouse is set).
//  2018-01-20: Inputs: Added Horizontal Mouse Wheel support.
//  2018-01-18: Inputs: Added mapping for ImGuiKey_Insert.
//  2017-08-25: Inputs: MousePos set to -FLT_MAX,-FLT_MAX when mouse is unavailable/missing (instead of -1,-1).
//  2016-10-15: Misc: Added a void* user_data parameter to Clipboard function handlers.

#include "imgui/imgui.h"
#include "ImguiInputInterface.hpp"

// Data
enum GrindstoneClientApi
{
    GrindstoneClientApi_Unknown,
    GrindstoneClientApi_OpenGL,
    GrindstoneClientApi_Vulkan
};

static const char* ImGui_ImplGrindstone_GetClipboardText(void* user_data)
{
    return "Clippy"; //GrindstoneGetClipboardString((Grindstonewindow*)user_data);
}

static void ImGui_ImplGrindstone_SetClipboardText(void* user_data, const char* text)
{
    //GrindstoneSetClipboardString((Grindstonewindow*)user_data, text);
}

void ImGui_ImplGrindstone_MouseButtonCallback(Grindstonewindow* window, int button, int action, int mods)
{
    if (g_PrevUserCallbackMousebutton != NULL)
        g_PrevUserCallbackMousebutton(window, button, action, mods);

    if (action == Grindstone_PRESS && button >= 0 && button < IM_ARRAYSIZE(g_MouseJustPressed))
        g_MouseJustPressed[button] = true;
}

void ImGui_ImplGrindstone_ScrollCallback(Grindstonewindow* window, double xoffset, double yoffset)
{
    if (g_PrevUserCallbackScroll != NULL)
        g_PrevUserCallbackScroll(window, xoffset, yoffset);

    ImGuiIO& io = ImGui::GetIO();
    io.MouseWheelH += (float)xoffset;
    io.MouseWheel += (float)yoffset;
}

void ImGui_ImplGrindstone_KeyCallback(Grindstonewindow* window, int key, int scancode, int action, int mods)
{
    if (g_PrevUserCallbackKey != NULL)
        g_PrevUserCallbackKey(window, key, scancode, action, mods);

    ImGuiIO& io = ImGui::GetIO();
    if (action == Grindstone_PRESS)
        io.KeysDown[key] = true;
    if (action == Grindstone_RELEASE)
        io.KeysDown[key] = false;

    // Modifiers are not reliable across systems
    io.KeyCtrl = io.KeysDown[Grindstone_KEY_LEFT_CONTROL] || io.KeysDown[Grindstone_KEY_RIGHT_CONTROL];
    io.KeyShift = io.KeysDown[Grindstone_KEY_LEFT_SHIFT] || io.KeysDown[Grindstone_KEY_RIGHT_SHIFT];
    io.KeyAlt = io.KeysDown[Grindstone_KEY_LEFT_ALT] || io.KeysDown[Grindstone_KEY_RIGHT_ALT];
    io.KeySuper = io.KeysDown[Grindstone_KEY_LEFT_SUPER] || io.KeysDown[Grindstone_KEY_RIGHT_SUPER];
}

void ImGui_ImplGrindstone_CharCallback(Grindstonewindow* window, unsigned int c)
{
    if (g_PrevUserCallbackChar != NULL)
        g_PrevUserCallbackChar(window, c);

    ImGuiIO& io = ImGui::GetIO();
    if (c > 0 && c < 0x10000)
        io.AddInputCharacter((unsigned short)c);
}

static bool ImGui_ImplGrindstone_Init(Grindstonewindow* window, bool install_callbacks, GrindstoneClientApi client_api)
{
    g_Window = window;
    g_Time = 0.0;

    // Setup back-end capabilities flags
    ImGuiIO& io = ImGui::GetIO();
    io.BackendFlags |= ImGuiBackendFlags_HasMouseCursors;         // We can honor GetMouseCursor() values (optional)
    io.BackendFlags |= ImGuiBackendFlags_HasSetMousePos;          // We can honor io.WantSetMousePos requests (optional, rarely used)
    io.BackendPlatformName = "imgui_impl_Grindstone";

    // Keyboard mapping. ImGui will use those indices to peek into the io.KeysDown[] array.
    io.KeyMap[ImGuiKey_Tab] = Grindstone_KEY_TAB;
    io.KeyMap[ImGuiKey_LeftArrow] = Grindstone_KEY_LEFT;
    io.KeyMap[ImGuiKey_RightArrow] = Grindstone_KEY_RIGHT;
    io.KeyMap[ImGuiKey_UpArrow] = Grindstone_KEY_UP;
    io.KeyMap[ImGuiKey_DownArrow] = Grindstone_KEY_DOWN;
    io.KeyMap[ImGuiKey_PageUp] = Grindstone_KEY_PAGE_UP;
    io.KeyMap[ImGuiKey_PageDown] = Grindstone_KEY_PAGE_DOWN;
    io.KeyMap[ImGuiKey_Home] = Grindstone_KEY_HOME;
    io.KeyMap[ImGuiKey_End] = Grindstone_KEY_END;
    io.KeyMap[ImGuiKey_Insert] = Grindstone_KEY_INSERT;
    io.KeyMap[ImGuiKey_Delete] = Grindstone_KEY_DELETE;
    io.KeyMap[ImGuiKey_Backspace] = Grindstone_KEY_BACKSPACE;
    io.KeyMap[ImGuiKey_Space] = Grindstone_KEY_SPACE;
    io.KeyMap[ImGuiKey_Enter] = Grindstone_KEY_ENTER;
    io.KeyMap[ImGuiKey_Escape] = Grindstone_KEY_ESCAPE;
    io.KeyMap[ImGuiKey_A] = Grindstone_KEY_A;
    io.KeyMap[ImGuiKey_C] = Grindstone_KEY_C;
    io.KeyMap[ImGuiKey_V] = Grindstone_KEY_V;
    io.KeyMap[ImGuiKey_X] = Grindstone_KEY_X;
    io.KeyMap[ImGuiKey_Y] = Grindstone_KEY_Y;
    io.KeyMap[ImGuiKey_Z] = Grindstone_KEY_Z;

    io.SetClipboardTextFn = ImGui_ImplGrindstone_SetClipboardText;
    io.GetClipboardTextFn = ImGui_ImplGrindstone_GetClipboardText;
    io.ClipboardUserData = g_Window;
#if defined(_WIN32)
    io.ImeWindowHandle = (void*)GrindstoneGetWin32Window(g_Window);
#endif

    g_MouseCursors[ImGuiMouseCursor_Arrow] = GrindstoneCreateStandardCursor(Grindstone_ARROW_CURSOR);
    g_MouseCursors[ImGuiMouseCursor_TextInput] = GrindstoneCreateStandardCursor(Grindstone_IBEAM_CURSOR);
    g_MouseCursors[ImGuiMouseCursor_ResizeAll] = GrindstoneCreateStandardCursor(Grindstone_ARROW_CURSOR);   // FIXME: Grindstone doesn't have this.
    g_MouseCursors[ImGuiMouseCursor_ResizeNS] = GrindstoneCreateStandardCursor(Grindstone_VRESIZE_CURSOR);
    g_MouseCursors[ImGuiMouseCursor_ResizeEW] = GrindstoneCreateStandardCursor(Grindstone_HRESIZE_CURSOR);
    g_MouseCursors[ImGuiMouseCursor_ResizeNESW] = GrindstoneCreateStandardCursor(Grindstone_ARROW_CURSOR);  // FIXME: Grindstone doesn't have this.
    g_MouseCursors[ImGuiMouseCursor_ResizeNWSE] = GrindstoneCreateStandardCursor(Grindstone_ARROW_CURSOR);  // FIXME: Grindstone doesn't have this.
    g_MouseCursors[ImGuiMouseCursor_Hand] = GrindstoneCreateStandardCursor(Grindstone_HAND_CURSOR);

    // Chain Grindstone callbacks: our callbacks will call the user's previously installed callbacks, if any.
    g_PrevUserCallbackMousebutton = NULL;
    g_PrevUserCallbackScroll = NULL;
    g_PrevUserCallbackKey = NULL;
    g_PrevUserCallbackChar = NULL;
    if (install_callbacks)
    {
        g_PrevUserCallbackMousebutton = GrindstoneSetMouseButtonCallback(window, ImGui_ImplGrindstone_MouseButtonCallback);
        g_PrevUserCallbackScroll = GrindstoneSetScrollCallback(window, ImGui_ImplGrindstone_ScrollCallback);
        g_PrevUserCallbackKey = GrindstoneSetKeyCallback(window, ImGui_ImplGrindstone_KeyCallback);
        g_PrevUserCallbackChar = GrindstoneSetCharCallback(window, ImGui_ImplGrindstone_CharCallback);
    }

    g_ClientApi = client_api;
    return true;
}

bool ImGui_ImplGrindstone_InitForOpenGL(Grindstonewindow* window, bool install_callbacks)
{
    return ImGui_ImplGrindstone_Init(window, install_callbacks, GrindstoneClientApi_OpenGL);
}

bool ImGui_ImplGrindstone_InitForVulkan(Grindstonewindow* window, bool install_callbacks)
{
    return ImGui_ImplGrindstone_Init(window, install_callbacks, GrindstoneClientApi_Vulkan);
}

void ImGui_ImplGrindstone_Shutdown()
{
    for (ImGuiMouseCursor cursor_n = 0; cursor_n < ImGuiMouseCursor_COUNT; cursor_n++)
    {
        GrindstoneDestroyCursor(g_MouseCursors[cursor_n]);
        g_MouseCursors[cursor_n] = NULL;
    }
    g_ClientApi = GrindstoneClientApi_Unknown;
}

static void ImGui_ImplGrindstone_UpdateMousePosAndButtons()
{
    // Update buttons
    ImGuiIO& io = ImGui::GetIO();
    for (int i = 0; i < IM_ARRAYSIZE(io.MouseDown); i++)
    {
        // If a mouse press event came, always pass it as "mouse held this frame", so we don't miss click-release events that are shorter than 1 frame.
        io.MouseDown[i] = g_MouseJustPressed[i] || GrindstoneGetMouseButton(g_Window, i) != 0;
        g_MouseJustPressed[i] = false;
    }

    // Update mouse position
    const ImVec2 mouse_pos_backup = io.MousePos;
    io.MousePos = ImVec2(-FLT_MAX, -FLT_MAX);
#ifdef __EMSCRIPTEN__
    const bool focused = true; // Emscripten
#else
    const bool focused = GrindstoneGetWindowAttrib(g_Window, Grindstone_FOCUSED) != 0;
#endif
    if (focused)
    {
        if (io.WantSetMousePos)
        {
            GrindstoneSetCursorPos(g_Window, (double)mouse_pos_backup.x, (double)mouse_pos_backup.y);
        }
        else
        {
            double mouse_x, mouse_y;
            GrindstoneGetCursorPos(g_Window, &mouse_x, &mouse_y);
            io.MousePos = ImVec2((float)mouse_x, (float)mouse_y);
        }
    }
}

static void ImGui_ImplGrindstone_UpdateMouseCursor()
{
    ImGuiIO& io = ImGui::GetIO();
    if ((io.ConfigFlags & ImGuiConfigFlags_NoMouseCursorChange) || GrindstoneGetInputMode(g_Window, Grindstone_CURSOR) == Grindstone_CURSOR_DISABLED)
        return;

    ImGuiMouseCursor imgui_cursor = ImGui::GetMouseCursor();
    if (imgui_cursor == ImGuiMouseCursor_None || io.MouseDrawCursor)
    {
        // Hide OS mouse cursor if imgui is drawing it or if it wants no cursor
        GrindstoneSetInputMode(g_Window, Grindstone_CURSOR, Grindstone_CURSOR_HIDDEN);
    }
    else
    {
        // Show OS mouse cursor
        // FIXME-PLATFORM: Unfocused windows seems to fail changing the mouse cursor with Grindstone 3.2, but 3.3 works here.
        GrindstoneSetCursor(g_Window, g_MouseCursors[imgui_cursor] ? g_MouseCursors[imgui_cursor] : g_MouseCursors[ImGuiMouseCursor_Arrow]);
        GrindstoneSetInputMode(g_Window, Grindstone_CURSOR, Grindstone_CURSOR_NORMAL);
    }
}

static void ImGui_ImplGrindstone_UpdateGamepads()
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
    MAP_BUTTON(ImGuiNavInput_Activate,   0);     // Cross / A
    MAP_BUTTON(ImGuiNavInput_Cancel,     1);     // Circle / B
    MAP_BUTTON(ImGuiNavInput_Menu,       2);     // Square / X
    MAP_BUTTON(ImGuiNavInput_Input,      3);     // Triangle / Y
    MAP_BUTTON(ImGuiNavInput_DpadLeft,   13);    // D-Pad Left
    MAP_BUTTON(ImGuiNavInput_DpadRight,  11);    // D-Pad Right
    MAP_BUTTON(ImGuiNavInput_DpadUp,     10);    // D-Pad Up
    MAP_BUTTON(ImGuiNavInput_DpadDown,   12);    // D-Pad Down
    MAP_BUTTON(ImGuiNavInput_FocusPrev,  4);     // L1 / LB
    MAP_BUTTON(ImGuiNavInput_FocusNext,  5);     // R1 / RB
    MAP_BUTTON(ImGuiNavInput_TweakSlow,  4);     // L1 / LB
    MAP_BUTTON(ImGuiNavInput_TweakFast,  5);     // R1 / RB
    MAP_ANALOG(ImGuiNavInput_LStickLeft, 0,  -0.3f,  -0.9f);
    MAP_ANALOG(ImGuiNavInput_LStickRight,0,  +0.3f,  +0.9f);
    MAP_ANALOG(ImGuiNavInput_LStickUp,   1,  +0.3f,  +0.9f);
    MAP_ANALOG(ImGuiNavInput_LStickDown, 1,  -0.3f,  -0.9f);
    #undef MAP_BUTTON
    #undef MAP_ANALOG
    if (axes_count > 0 && buttons_count > 0)
        io.BackendFlags |= ImGuiBackendFlags_HasGamepad;
    else
        io.BackendFlags &= ~ImGuiBackendFlags_HasGamepad;
}

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
    io.DisplayFramebufferScale = ImVec2(w > 0 ? ((float)display_w / w) : 0, h > 0 ? ((float)display_h / h) : 0);

    // Setup time step
    double current_time = GrindstoneGetTime();
    io.DeltaTime = g_Time > 0.0 ? (float)(current_time - g_Time) : (float)(1.0f/60.0f);
    g_Time = current_time;

    ImGui_ImplGrindstone_UpdateMousePosAndButtons();
    ImGui_ImplGrindstone_UpdateMouseCursor();

    // Gamepad navigation mapping
    ImGui_ImplGrindstone_UpdateGamepads();
}