#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/keysymdef.h>
#include <GL/glx.h>

#include <iostream>
#include <assert.h>
#include <vector>
#include <cstring>

#include "X11Window.hpp"
#include <Common/Input.hpp>
#include <EngineCore/EngineCore.hpp>

using namespace Grindstone;

Events::KeyPressCode TranslateKey(int key);
Events::MouseButtonCode TranslateMouseButton(int key);

static bool IsExtensionSupported(const char* extList, const char* extension) {
	const char* start;
	const char* where, * terminator;

	where = strchr(extension, ' ');
	if (where || *extension == '\0') {
		return false;
	}

	for (start = extList;;) {
		where = strstr(start, extension);

		if (!where) {
			break;
		}

		terminator = where + strlen(extension);

		if (where == start || *(where - 1) == ' ') {
			if (*terminator == ' ' || *terminator == '\0') {
				return true;
			}
		}

		start = terminator;
	}

	return false;
}

bool X11Window::Initialize(CreateInfo& createInfo) {
	shouldClose = false;
	width = createInfo.width;
	height = createInfo.height;
	fullscreenMode = createInfo.fullscreen;
	engineCore = createInfo.engineCore;

	xDisplay = XOpenDisplay(NULL);
	if (xDisplay == NULL) {
		std::cout << "Could not open display\n";
		return false;
	}

	Screen* screen = DefaultScreenOfDisplay(xDisplay);
	int screenID = DefaultScreen(xDisplay);

	GLint glxAttribs[] = {
		GLX_X_RENDERABLE    , True,
		GLX_DRAWABLE_TYPE   , GLX_WINDOW_BIT,
		GLX_RENDER_TYPE     , GLX_RGBA_BIT,
		GLX_X_VISUAL_TYPE   , GLX_TRUE_COLOR,
		GLX_RED_SIZE        , 8,
		GLX_GREEN_SIZE      , 8,
		GLX_BLUE_SIZE       , 8,
		GLX_ALPHA_SIZE      , 8,
		GLX_DEPTH_SIZE      , 24,
		GLX_STENCIL_SIZE    , 8,
		GLX_DOUBLEBUFFER    , True,
		None
	};

	int fbcount;
	GLXFBConfig* fbc = glXChooseFBConfig(xDisplay, screenID, glxAttribs, &fbcount);
	if (fbc == 0) {
		std::cout << "Failed to retrieve framebuffer.\n";
		XCloseDisplay(xDisplay);
		return false;
	}

	int best_fbc = -1, worst_fbc = -1, best_num_samp = -1, worst_num_samp = 999;
	for (int i = 0; i < fbcount; ++i) {
		XVisualInfo* vi = glXGetVisualFromFBConfig(xDisplay, fbc[i]);
		if (vi != 0) {
			int samp_buf, samples;
			glXGetFBConfigAttrib(xDisplay, fbc[i], GLX_SAMPLE_BUFFERS, &samp_buf);
			glXGetFBConfigAttrib(xDisplay, fbc[i], GLX_SAMPLES, &samples);
			//std::cout << "  Matching fbconfig " << i << ", SAMPLE_BUFFERS = " << samp_buf << ", SAMPLES = " << samples << ".\n";

			if (best_fbc < 0 || (samp_buf && samples > best_num_samp)) {
				best_fbc = i;
				best_num_samp = samples;
			}
			if (worst_fbc < 0 || !samp_buf || samples < worst_num_samp)
				worst_fbc = i;
			worst_num_samp = samples;
		}
		XFree(vi);
	}

	//std::cout << "Best visual info index: " << best_fbc << "\n";
	GLXFBConfig bestFbc = fbc[best_fbc];
	XFree(fbc); // Make sure to free this!

	XVisualInfo* visual = glXGetVisualFromFBConfig(xDisplay, bestFbc);

	if (visual == 0) {
		std::cout << "Could not create correct visual window.\n";
		XCloseDisplay(xDisplay);
		return false;
	}

	if (screenID != visual->screen) {
		std::cout << "screenID(" << screenID << ") does not match visual->screen(" << visual->screen << ").\n";
		XCloseDisplay(xDisplay);
		return false;
	}

	// Open the window
	XSetWindowAttributes windowAttribs;
	windowAttribs.border_pixel = BlackPixel(xDisplay, screenID);
	windowAttribs.background_pixel = WhitePixel(xDisplay, screenID);
	windowAttribs.override_redirect = True;
	windowAttribs.colormap = XCreateColormap(xDisplay, RootWindow(xDisplay, screenID), visual->visual, AllocNone);
	windowAttribs.event_mask = ExposureMask | KeyPressMask | KeyReleaseMask | KeymapStateMask | PointerMotionMask | ButtonPressMask | ButtonReleaseMask | FocusChangeMask;

	xWindow = XCreateWindow(
		xDisplay,
		RootWindow(xDisplay, screenID),
		0,
		0,
		width,
		height,
		0,
		visual->depth,
		InputOutput,
		visual->visual,
		CWBackPixel | CWColormap | CWBorderPixel | CWEventMask,
		&windowAttribs
	);

	// Name the window
	XStoreName(xDisplay, xWindow, createInfo.title);

	// Create GLX OpenGL context
	// glXCreateContextAttribsARBProc glXCreateContextAttribsARB = 0;
	// glXCreateContextAttribsARB = (glXCreateContextAttribsARBProc)glXGetProcAddressARB((const GLubyte*)"glXCreateContextAttribsARB");

	int contextAttribs[] = {
		GLX_CONTEXT_MAJOR_VERSION_ARB,	3,
		GLX_CONTEXT_MINOR_VERSION_ARB,	3,
		GLX_CONTEXT_PROFILE_MASK_ARB, GLX_CONTEXT_CORE_PROFILE_BIT_ARB,
		None
	};

	GLXContext context = 0;
	const char* glxExts = glXQueryExtensionsString(xDisplay, screenID);
	// if (!IsExtensionSupported(glxExts, "GLX_ARB_create_context")) {
		// std::cout << "GLX_ARB_create_context not supported\n";
		context = glXCreateNewContext(xDisplay, bestFbc, GLX_RGBA_TYPE, 0, True);
	// }
	// else {
		// context = glXCreateContextAttribsARB(xDisplay, bestFbc, 0, true, contextAttribs);
	// }

	GLint majorGLX, minorGLX = 0;
	glXQueryVersion(xDisplay, &majorGLX, &minorGLX);
	if (majorGLX <= 1 && minorGLX < 2) {
		std::cout << "GLX 1.2 or greater is required.\n";
		XCloseDisplay(xDisplay);
		return false;
	}

	if (context == NULL) {
		std::cout << "Null Context!" << "\n";
		return 0;
	}


	XSync(xDisplay, False);

	// Verifying that context is a direct context
	if (!glXIsDirect(xDisplay, context)) {
		std::cout << "Indirect GLX rendering context obtained\n";
	}

	//if (!glXMakeCurrent(xDisplay, window, context))
	if (!glXMakeContextCurrent(xDisplay, xWindow, xWindow, context))
		std::cout << "GLX Make Current Failed!!" << "\n";

	// Show the window
	// XClearWindow(xDisplay, xWindow);
	// XMapRaised(xDisplay, xWindow);

	return true;
}

void X11Window::Show() {
}

bool X11Window::ShouldClose() {
	return shouldClose;
}

void X11Window::HandleEvents() {
	Input::Interface* input = nullptr;
	if (engineCore) {
		input = engineCore->GetInputManager();
		if (input == nullptr) {
			return;
		}
	}
	else {
		return;
	}

	XWindowAttributes attribs;
	char str[25] = {0};
	KeySym keysym = 0;
	int len = 0;
	XEvent ev;
		while (XPending(xDisplay) > 0) {
		XNextEvent(xDisplay, &ev);
		switch(ev.type) {
			case KeymapNotify:
				XRefreshKeyboardMapping(&ev.xmapping);
				break;
			case KeyPress:
				// len = XLookupString(&ev.xkey, str, 25, &keysym, NULL);
				input->SetKeyPressed(TranslateKey(keysym), true);
				break;
			case KeyRelease:
				// len = XLookupString(&ev.xkey, str, 25, &keysym, NULL);
				input->SetKeyPressed(TranslateKey(keysym), false);
				break;
			case ButtonPress:
				input->SetMouseButton(TranslateMouseButton(ev.xbutton.button - 1), true);
	   			break;
			case ButtonRelease:
				input->SetMouseButton(TranslateMouseButton(ev.xbutton.button - 1), false);
				break;
			case MotionNotify:
				input->SetMousePosition(ev.xmotion.x, ev.xmotion.y);
				break;
		   	case FocusIn:
				input->SetIsFocused(true);
				break;
		   	case FocusOut:
				input->SetIsFocused(false);
				break;
			case DestroyNotify:
				input->ForceQuit();
				break;
			/*case Expose: {
				//XWindowAttributes attribs;
				std::cout << "Expose event fired: " << "\n";
				XGetWindowAttributes(xDisplay, window, &attribs);
				//std::cout << "\tWindow Width: " << attribs.width << ", Height: " << attribs.height << "\n";
				}
				break;*/
		}
	}
}

void X11Window::SetFullscreen(FullscreenMode mode) {
}


void X11Window::GetWindowRect(unsigned int &left, unsigned int &top, unsigned int &right, unsigned int &bottom) {
}

void X11Window::GetWindowSize(unsigned int &width, unsigned int &height) {
}

void X11Window::SetWindowSize(unsigned int width, unsigned int height) {
}

void X11Window::SetMousePos(unsigned int x, unsigned int y) {
	XWarpPointer(xDisplay, None, xWindow, 0, 0, 0, 0, x, y);
	XFlush(xDisplay);
}

void X11Window::GetMousePos(unsigned int& x, unsigned int& y) {
	::Window root, child;
	int originalX, originalY;
	unsigned int mask;
	if (!XQueryPointer(xDisplay, xWindow, &root, &child, &originalX, &originalY, &originalX, &originalY, &mask)) {
		std::cout << "Could not get cursor position\n";
	}

	x = originalX;
	y = originalY;
}

void X11Window::SetWindowPos(unsigned int x, unsigned int y) {
}

void X11Window::GetWindowPos(unsigned int& width, unsigned int& height) {
}

void X11Window::Close() {
}

::Window X11Window::GetHandle() {
	return xWindow;
}

void X11Window::SetWindowFocus() {
	XRaiseWindow(xDisplay, xWindow);
}

bool X11Window::GetWindowFocus() {
	return false;
}

bool X11Window::GetWindowMinimized() {
	return false;
}

void X11Window::SetWindowTitle(const char* title) {
	XStoreName(xDisplay, xWindow, title);
}

void X11Window::SetWindowAlpha(float alpha) {
}

float X11Window::GetWindowDpiScale() {
	return 1.0f;
}

bool X11Window::CopyStringToClipboard(const std::string& stringToCopy) {
	return false;
}

std::string X11Window::OpenFileDialogue(const char* filter) {
	return "";
}

std::string X11Window::SaveFileDialogue(const char* filter) {
	return "";
}

void X11Window::ExplorePath(const char* path) {
}

void X11Window::OpenFileUsingDefaultProgram(const char* path) {
}

using namespace Grindstone::Events;

MouseButtonCode TranslateMouseButton(int btn) {
	switch (btn) {
		default:
		case 0:
			return MouseButtonCode::Left;
		case 1:
			return MouseButtonCode::Middle;
		case 2:
			return MouseButtonCode::Right;
		case 3:
			return MouseButtonCode::Mouse4;
		case 4:
			return MouseButtonCode::Mouse5;
	}
}

KeyPressCode TranslateKey(int key) {
	switch (key) {
		/*case XK_A: case XK_a: return KeyPressCode::A;
		case XK_B: case XK_b: return KeyPressCode::B;
		case XK_C: case XK_c: return KeyPressCode::C;
		case XK_D: case XK_d: return KeyPressCode::D;
		case XK_E: case XK_e: return KeyPressCode::E;
		case XK_F: case XK_f: return KeyPressCode::F;
		case XK_G: case XK_g: return KeyPressCode::G;
		case XK_H: case XK_h: return KeyPressCode::H;
		case XK_I: case XK_i: return KeyPressCode::I;
		case XK_J: case XK_j: return KeyPressCode::J;
		case XK_K: case XK_k: return KeyPressCode::K;
		case XK_L: case XK_l: return KeyPressCode::L;
		case XK_M: case XK_m: return KeyPressCode::M;
		case XK_N: case XK_n: return KeyPressCode::N;
		case XK_O: case XK_o: return KeyPressCode::O;
		case XK_P: case XK_p: return KeyPressCode::P;
		case XK_Q: case XK_q: return KeyPressCode::Q;
		case XK_R: case XK_r: return KeyPressCode::R;
		case XK_S: case XK_s: return KeyPressCode::S;
		case XK_T: case XK_t: return KeyPressCode::T;
		case XK_U: case XK_u: return KeyPressCode::U;
		case XK_V: case XK_v: return KeyPressCode::V;
		case XK_W: case XK_w: return KeyPressCode::W;
		case XK_X: case XK_x: return KeyPressCode::X;
		case XK_Y: case XK_y: return KeyPressCode::Y;
		case XK_Z: case XK_z: return KeyPressCode::Z;
		case XK_Escape: return KeyPressCode::ESCAPE;
		case XK_Tab: return KeyPressCode::TAB;
		case XK_space: return KeyPressCode::SPACE;
		case XK_Control_L: return KeyPressCode::LCONTROL;
		case XK_Control_R: return KeyPressCode::CONTROL;
		case XK_Shift_L: return KeyPressCode::LSHIFT;
		case XK_Shift_R: return KeyPressCode::SHIFT;
		case XK_Alt_L: return KeyPressCode::LALT;
		case XK_Alt_R: return KeyPressCode::ALT;

		case XK_Left:	return KeyPressCode::LEFT;
		case XK_Right:	return KeyPressCode::RIGHT;
		case XK_Up:	return KeyPressCode::UP;
		case XK_Down:	return KeyPressCode::DOWN;

		case XK_KP_0: return KeyPressCode::NUMPAD_0;
		case XK_KP_1: return KeyPressCode::NUMPAD_1;
		case XK_KP_2: return KeyPressCode::NUMPAD_2;
		case XK_KP_3: return KeyPressCode::NUMPAD_3;
		case XK_KP_4: return KeyPressCode::NUMPAD_4;
		case XK_KP_5: return KeyPressCode::NUMPAD_5;
		case XK_KP_6: return KeyPressCode::NUMPAD_6;
		case XK_KP_7: return KeyPressCode::NUMPAD_7;
		case XK_KP_8: return KeyPressCode::NUMPAD_8;
		case XK_KP_9: return KeyPressCode::NUMPAD_9;

		case XK_Num_Lock:	return KeyPressCode::NUMPAD_NUMLOCK;
		case XK_KP_Divide: 	return KeyPressCode::NUMPAD_DIVIDE;
		case XK_KP_Multiply: 	return KeyPressCode::NUMPAD_MULTIPLY;
		case XK_KP_Subtract: 	return KeyPressCode::NUMPAD_SUBTRACT;
		case XK_KP_Add: 	return KeyPressCode::NUMPAD_ADD;
		case XK_KP_Enter: 	return KeyPressCode::NUMPAD_ENTER;
		case XK_KP_Decimal: 	return KeyPressCode::NUMPAD_DOT;

		case XK_F1: return KeyPressCode::F1;
		case XK_F2: return KeyPressCode::F2;
		case XK_F3: return KeyPressCode::F3;
		case XK_F4: return KeyPressCode::F4;
		case XK_F5: return KeyPressCode::F5;
		case XK_F6: return KeyPressCode::F6;
		case XK_F7: return KeyPressCode::F7;
		case XK_F8: return KeyPressCode::F8;
		case XK_F9: return KeyPressCode::F9;
		case XK_F10:return KeyPressCode::F10;
		case XK_F11:return KeyPressCode::F11;
		case XK_F12:return KeyPressCode::F12;
		case XK_F13:return KeyPressCode::F13;
		case XK_F14:return KeyPressCode::F14;
		case XK_F15:return KeyPressCode::F15;
		case XK_F16:return KeyPressCode::F16;
		case XK_F17:return KeyPressCode::F17;
		case XK_F18:return KeyPressCode::F18;
		case XK_F19:return KeyPressCode::F19;
		case XK_F20:return KeyPressCode::F20;
		case XK_F21:return KeyPressCode::F21;
		case XK_F22:return KeyPressCode::F22;
		case XK_F23:return KeyPressCode::F23;
		case XK_F24:return KeyPressCode::F24;
		case XK_F25:return KeyPressCode::F25;

		case XK_0:	return KeyPressCode::0;
		case XK_1:	return KeyPressCode::1;
		case XK_2:	return KeyPressCode::2;
		case XK_3:	return KeyPressCode::3;
		case XK_4:	return KeyPressCode::4;
		case XK_5:	return KeyPressCode::5;
		case XK_6:	return KeyPressCode::6;
		case XK_7:	return KeyPressCode::7;
		case XK_8:	return KeyPressCode::8;
		case XK_9:	return KeyPressCode::9;
		case XK_minus:	return KeyPressCode::DASH;
		case XK_equal:	return KeyPressCode::ADD;

		case XK_Insert:		return KeyPressCode::INSERT;
		case XK_Home:		return KeyPressCode::HOME;
		case XK_Page_Up:	return KeyPressCode::PG_UP;
		case XK_Page_Down:	return KeyPressCode::PG_DOWN;
		case XK_End:		return KeyPressCode::END;
		case XK_KP_Delete:	return KeyPressCode::DELETE;
		case XK_Pause:		return KeyPressCode::PAUSE;
		case XK_Caps_Lock:	return KeyPressCode::CAPSLOCK;
		case XK_Scroll_Lock:	return KeyPressCode::SCROLL_LOCK;

		case XK_comma:		return KeyPressCode::COMMA;
		case XK_period:		return KeyPressCode::PERIOD;
		case XK_slash:		return KeyPressCode::FORWARD_SLASH;
		case XK_backslash:	return KeyPressCode::BACK_SLASH;
		case XK_semicolon:	return KeyPressCode::SEMICOLON;
		case XK_apostrophe:	return KeyPressCode::APOSTROPHE;
		case XK_bracketleft:	return KeyPressCode::LBRACKET;
		case XK_bracketright:	return KeyPressCode::RBRACKET;

		case XK_Return:		return KeyPressCode::ENTER;
		case XK_BackSpace:	return KeyPressCode::BACKSPACE;
		case XK_grave:		return KeyPressCode::TILDE;*/
		default: return KeyPressCode::Last;
	}
}
