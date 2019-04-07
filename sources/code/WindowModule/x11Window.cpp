#if defined(__linux__)
#include "../Engine/Core/Input.hpp"
#include <stdio.h>

#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/keysymdef.h>

#include "../GraphicsOpenGL/GLGraphicsWrapper.hpp"

#include <cstring>

static bool isExtensionSupported(const char *extList, const char *extension) {
    const char *start;
    const char *where, *terminator;

    where = strchr(extension, ' ');
    if (where || *extension == '\0') {
        return false;
    }

    for (start=extList;;) {
        where = strstr(start, extension);

        if (!where) {
            break;
        }

        terminator = where + strlen(extension);

        if ( where == start || *(where - 1) == ' ' ) {
            if ( *terminator == ' ' || *terminator == '\0' ) {
                return true;
            }
        }   

        start = terminator;
    }

    return false;
}

void GraphicsWrapper::SetCursorShown(bool) {
	// Create blank cursor
	static char data[1] = {0};
	Cursor cursor;
	Pixmap blank;
	XColor dummy;

	blank = XCreateBitmapFromData(xDisplay, xWindow, data, 1, 1);
	if (blank == None) {
		std::cout << "Can't create blank cursor.\n";
		return;
	}
	cursor = XCreatePixmapCursor(xDisplay, blank, blank,
	&dummy, &dummy, 0, 0);
	XFreePixmap(xDisplay, blank);

	XDefineCursor(xDisplay, xWindow, cursor);
}

void GraphicsWrapper::ResetCursor() {
	SetCursor(width / 2, height / 2);
}

void GraphicsWrapper::SetCursor(int x, int y) {
	XWarpPointer(xDisplay, None, xWindow, 0, 0, 0, 0, x, y);
	XFlush(xDisplay);
}

void GraphicsWrapper::GetCursor(int &x, int &y) {
	Window root, child;
	int rootX, rootY;
	unsigned int mask;
	if (!XQueryPointer(xDisplay,xWindow,&root,&child,&x,&y,&x,&y,&mask))
		std::cout << "Could not get cursor position\n";
}

int TranslateKey(int key) {
	switch (key) {
		case XK_A: case XK_a: return KEY_A;
		case XK_B: case XK_b: return KEY_B;
		case XK_C: case XK_c: return KEY_C;
		case XK_D: case XK_d: return KEY_D;
		case XK_E: case XK_e: return KEY_E;
		case XK_F: case XK_f: return KEY_F;
		case XK_G: case XK_g: return KEY_G;
		case XK_H: case XK_h: return KEY_H;
		case XK_I: case XK_i: return KEY_I;
		case XK_J: case XK_j: return KEY_J;
		case XK_K: case XK_k: return KEY_K;
		case XK_L: case XK_l: return KEY_L;
		case XK_M: case XK_m: return KEY_M;
		case XK_N: case XK_n: return KEY_N;
		case XK_O: case XK_o: return KEY_O;
		case XK_P: case XK_p: return KEY_P;
		case XK_Q: case XK_q: return KEY_Q;
		case XK_R: case XK_r: return KEY_R;
		case XK_S: case XK_s: return KEY_S;
		case XK_T: case XK_t: return KEY_T;
		case XK_U: case XK_u: return KEY_U;
		case XK_V: case XK_v: return KEY_V;
		case XK_W: case XK_w: return KEY_W;
		case XK_X: case XK_x: return KEY_X;
		case XK_Y: case XK_y: return KEY_Y;
		case XK_Z: case XK_z: return KEY_Z;
		case XK_Escape: return KEY_ESCAPE;
		case XK_Tab: return KEY_TAB;
		case XK_space: return KEY_SPACE;
		case XK_Control_L: return KEY_LCONTROL;
		case XK_Control_R: return KEY_CONTROL;
		case XK_Shift_L: return KEY_LSHIFT;
		case XK_Shift_R: return KEY_SHIFT;
		case XK_Alt_L: return KEY_LALT;
		case XK_Alt_R: return KEY_ALT;

		case XK_Left:	return KEY_LEFT;
		case XK_Right:	return KEY_RIGHT;
		case XK_Up:	return KEY_UP;
		case XK_Down:	return KEY_DOWN;

		case XK_KP_0: return KEY_NUMPAD_0;
		case XK_KP_1: return KEY_NUMPAD_1;
		case XK_KP_2: return KEY_NUMPAD_2;
		case XK_KP_3: return KEY_NUMPAD_3;
		case XK_KP_4: return KEY_NUMPAD_4;
		case XK_KP_5: return KEY_NUMPAD_5;
		case XK_KP_6: return KEY_NUMPAD_6;
		case XK_KP_7: return KEY_NUMPAD_7;
		case XK_KP_8: return KEY_NUMPAD_8;
		case XK_KP_9: return KEY_NUMPAD_9;

		case XK_Num_Lock:	return KEY_NUMPAD_NUMLOCK;
		case XK_KP_Divide: 	return KEY_NUMPAD_DIVIDE;
		case XK_KP_Multiply: 	return KEY_NUMPAD_MULTIPLY;
		case XK_KP_Subtract: 	return KEY_NUMPAD_SUBTRACT;
		case XK_KP_Add: 	return KEY_NUMPAD_ADD;
		case XK_KP_Enter: 	return KEY_NUMPAD_ENTER;
		case XK_KP_Decimal: 	return KEY_NUMPAD_DOT;

		case XK_F1: return KEY_F1;
		case XK_F2: return KEY_F2;
		case XK_F3: return KEY_F3;
		case XK_F4: return KEY_F4;
		case XK_F5: return KEY_F5;
		case XK_F6: return KEY_F6;
		case XK_F7: return KEY_F7;
		case XK_F8: return KEY_F8;
		case XK_F9: return KEY_F9;
		case XK_F10:return KEY_F10;
		case XK_F11:return KEY_F11;
		case XK_F12:return KEY_F12;
		case XK_F13:return KEY_F13;
		case XK_F14:return KEY_F14;
		case XK_F15:return KEY_F15;
		case XK_F16:return KEY_F16;
		case XK_F17:return KEY_F17;
		case XK_F18:return KEY_F18;
		case XK_F19:return KEY_F19;
		case XK_F20:return KEY_F20;
		case XK_F21:return KEY_F21;
		case XK_F22:return KEY_F22;
		case XK_F23:return KEY_F23;
		case XK_F24:return KEY_F24;
		case XK_F25:return KEY_F25;

		case XK_0:	return KEY_0;
		case XK_1:	return KEY_1;
		case XK_2:	return KEY_2;
		case XK_3:	return KEY_3;
		case XK_4:	return KEY_4;
		case XK_5:	return KEY_5;
		case XK_6:	return KEY_6;
		case XK_7:	return KEY_7;
		case XK_8:	return KEY_8;
		case XK_9:	return KEY_9;
		case XK_minus:	return KEY_DASH;
		case XK_equal:	return KEY_ADD;

		case XK_Insert:		return KEY_INSERT;
		case XK_Home:		return KEY_HOME;
		case XK_Page_Up:	return KEY_PG_UP;
		case XK_Page_Down:	return KEY_PG_DOWN;
		case XK_End:		return KEY_END;
		case XK_KP_Delete:	return KEY_DELETE;
		case XK_Pause:		return KEY_PAUSE;
		case XK_Caps_Lock:	return KEY_CAPSLOCK;
		case XK_Scroll_Lock:	return KEY_SCROLL_LOCK;

		case XK_comma:		return KEY_COMMA;
		case XK_period:		return KEY_PERIOD;
		case XK_slash:		return KEY_FORWARD_SLASH;
		case XK_backslash:	return KEY_BACK_SLASH;
		case XK_semicolon:	return KEY_SEMICOLON;
		case XK_apostrophe:	return KEY_APOSTROPHE;
		case XK_bracketleft:	return KEY_LBRACKET;
		case XK_bracketright:	return KEY_RBRACKET;

		case XK_Return:		return KEY_ENTER;
		case XK_BackSpace:	return KEY_BACKSPACE;
		case XK_grave:		return KEY_TILDE;
		default: return -1;
	}
}

void GraphicsWrapper::setTitle(const char *str) {
	XStoreName(xDisplay, xWindow, str);
}

void GraphicsWrapper::HandleEvents() {
	if (input == NULL) {
		std::cout << "No Interface!" << std::endl;
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
				len = XLookupString(&ev.xkey, str, 25, &keysym, NULL);
				input->SetKey(TranslateKey(keysym), true);
				break;
			case KeyRelease:
				len = XLookupString(&ev.xkey, str, 25, &keysym, NULL);
				input->SetKey(TranslateKey(keysym), false);
				break;
			case ButtonPress:
				input->SetMouseButton(ev.xbutton.button-1, true);
	   			break;
			case ButtonRelease:
				input->SetMouseButton(ev.xbutton.button-1, false);
				break;
			case MotionNotify:
				input->SetMousePosition(ev.xmotion.x, ev.xmotion.y);
				break;
           	case FocusIn:
				input->SetFocused(true);
				break;
           	case FocusOut:
				input->SetFocused(false);
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

void GraphicsWrapper::setFocus() {
	XRaiseWindow(xDisplay, xWindow);
	//XSetInputFocus(xDisplay, xWindow, RevertToNone, CurrentTime);
}

#endif