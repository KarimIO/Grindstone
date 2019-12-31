#include "../GraphicsCommon/GraphicsWrapper.hpp"

#if defined(GLFW_WINDOW)
#include "../Engine/Core/Input.hpp"
#include <stdio.h>

#include <GLFW/glfw3.h>

#include <cstring>

InputInterface *g_input;


namespace Grindstone {
	namespace GraphicsAPI {
		void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods) {
			g_input->SetMouseButton(key, action == GLFW_PRESS);
		}

		void mouse_button_callback(GLFWwindow* window, int button, int action, int mods) {
			g_input->SetMouseButton(button, action == GLFW_PRESS);
		}

		void cursor_enter_callback(GLFWwindow* window, int entered) {
			if (entered) {
			}
			else {
			}
		}

		void window_focus_callback(GLFWwindow* window, int focused) {
			g_input->SetFocused(focused);
		}

		void window_size_callback(GLFWwindow* window, int width, int height) {
			g_input->ResizeEvent(width, height);
		}


		void GraphicsWrapper::SetCursorShown(bool show) {
			glfwSetInputMode(window, GLFW_CURSOR, show ? GLFW_CURSOR_NORMAL : GLFW_CURSOR_HIDDEN);
		}

		void GraphicsWrapper::ResetCursor() {
			SetCursor(width / 2, height / 2);
		}

		void GraphicsWrapper::SetCursor(int x, int y) {
			glfwSetCursorPos(window, x, y);
		}

		void GraphicsWrapper::GetCursor(int &x, int &y) {
			double xpos, ypos;
			glfwGetCursorPos(window, &xpos, &ypos);
			x = xpos;
			y = ypos;
		}

		int TranslateKey(int key) {
			switch (key) {
			case GLFW_KEY_SPACE: return KEY_SPACE;
			case GLFW_KEY_APOSTROPHE: return KEY_APOSTROPHE;
			case GLFW_KEY_COMMA: return KEY_COMMA;
			case GLFW_KEY_MINUS: return KEY_DASH;
			case GLFW_KEY_PERIOD: return KEY_PERIOD;
			case GLFW_KEY_SLASH: return KEY_FORWARD_SLASH;

			case GLFW_KEY_0: return KEY_0;
			case GLFW_KEY_1: return KEY_1;
			case GLFW_KEY_2: return KEY_2;
			case GLFW_KEY_3: return KEY_3;
			case GLFW_KEY_4: return KEY_4;
			case GLFW_KEY_5: return KEY_5;
			case GLFW_KEY_6: return KEY_6;
			case GLFW_KEY_7: return KEY_7;
			case GLFW_KEY_8: return KEY_8;
			case GLFW_KEY_9: return KEY_9;

			case GLFW_KEY_SEMICOLON: return KEY_SEMICOLON;
			case GLFW_KEY_EQUAL: return KEY_ADD;

			case GLFW_KEY_A: return KEY_A;
			case GLFW_KEY_B: return KEY_B;
			case GLFW_KEY_C: return KEY_C;
			case GLFW_KEY_D: return KEY_D;
			case GLFW_KEY_E: return KEY_E;
			case GLFW_KEY_F: return KEY_F;
			case GLFW_KEY_G: return KEY_G;
			case GLFW_KEY_H: return KEY_H;
			case GLFW_KEY_I: return KEY_I;
			case GLFW_KEY_J: return KEY_J;
			case GLFW_KEY_K: return KEY_K;
			case GLFW_KEY_L: return KEY_L;
			case GLFW_KEY_M: return KEY_M;
			case GLFW_KEY_N: return KEY_N;
			case GLFW_KEY_O: return KEY_O;
			case GLFW_KEY_P: return KEY_P;
			case GLFW_KEY_Q: return KEY_Q;
			case GLFW_KEY_R: return KEY_R;
			case GLFW_KEY_S: return KEY_S;
			case GLFW_KEY_T: return KEY_T;
			case GLFW_KEY_U: return KEY_U;
			case GLFW_KEY_V: return KEY_V;
			case GLFW_KEY_W: return KEY_W;
			case GLFW_KEY_X: return KEY_X;
			case GLFW_KEY_Y: return KEY_Y;
			case GLFW_KEY_Z: return KEY_Z;

			case GLFW_KEY_LEFT_BRACKET: return KEY_LBRACKET;
			case GLFW_KEY_BACKSLASH: return KEY_BACK_SLASH;
			case GLFW_KEY_RIGHT_BRACKET: return KEY_RBRACKET;
			case GLFW_KEY_GRAVE_ACCENT: return KEY_TILDE;
			case GLFW_KEY_WORLD_1: return -1; /* non-US #1 */
			case GLFW_KEY_WORLD_2: return -1; /* non-US #2 */

			case GLFW_KEY_ESCAPE: return KEY_ESCAPE;
			case GLFW_KEY_ENTER: return KEY_ENTER;
			case GLFW_KEY_TAB: return KEY_TAB;
			case GLFW_KEY_BACKSPACE: return KEY_BACKSPACE;
			case GLFW_KEY_INSERT: return KEY_INSERT;
			case GLFW_KEY_DELETE: return KEY_DELETE;
			case GLFW_KEY_RIGHT: return KEY_RIGHT;
			case GLFW_KEY_LEFT: return KEY_LEFT;
			case GLFW_KEY_DOWN: return KEY_DOWN;
			case GLFW_KEY_UP: return KEY_UP;
			case GLFW_KEY_PAGE_UP: return KEY_PG_UP;
			case GLFW_KEY_PAGE_DOWN: return KEY_PG_DOWN;

			case GLFW_KEY_HOME: return KEY_HOME;
			case GLFW_KEY_END: return KEY_END;
			case GLFW_KEY_CAPS_LOCK: return KEY_CAPSLOCK;

			case GLFW_KEY_SCROLL_LOCK: return KEY_SCROLL_LOCK;
			case GLFW_KEY_NUM_LOCK: return KEY_NUMPAD_NUMLOCK;
			case GLFW_KEY_PRINT_SCREEN: return -1;
			case GLFW_KEY_PAUSE: return KEY_PAUSE;

			case GLFW_KEY_F1: return KEY_F1;
			case GLFW_KEY_F2: return KEY_F2;
			case GLFW_KEY_F3: return KEY_F3;
			case GLFW_KEY_F4: return KEY_F4;
			case GLFW_KEY_F5: return KEY_F5;
			case GLFW_KEY_F6: return KEY_F6;
			case GLFW_KEY_F7: return KEY_F7;
			case GLFW_KEY_F8: return KEY_F8;
			case GLFW_KEY_F9: return KEY_F9;
			case GLFW_KEY_F10: return KEY_F10;
			case GLFW_KEY_F11: return KEY_F11;
			case GLFW_KEY_F12: return KEY_F12;
			case GLFW_KEY_F13: return KEY_F13;
			case GLFW_KEY_F14: return KEY_F14;
			case GLFW_KEY_F15: return KEY_F15;
			case GLFW_KEY_F16: return KEY_F16;
			case GLFW_KEY_F17: return KEY_F17;
			case GLFW_KEY_F18: return KEY_F18;
			case GLFW_KEY_F19: return KEY_F19;
			case GLFW_KEY_F20: return KEY_F20;
			case GLFW_KEY_F21: return KEY_F21;
			case GLFW_KEY_F22: return KEY_F22;
			case GLFW_KEY_F23: return KEY_F23;
			case GLFW_KEY_F24: return KEY_F24;
			case GLFW_KEY_F25: return KEY_F25;

			case GLFW_KEY_KP_0: return KEY_NUMPAD_0;
			case GLFW_KEY_KP_1: return KEY_NUMPAD_1;
			case GLFW_KEY_KP_2: return KEY_NUMPAD_2;
			case GLFW_KEY_KP_3: return KEY_NUMPAD_3;
			case GLFW_KEY_KP_4: return KEY_NUMPAD_4;
			case GLFW_KEY_KP_5: return KEY_NUMPAD_5;
			case GLFW_KEY_KP_6: return KEY_NUMPAD_6;
			case GLFW_KEY_KP_7: return KEY_NUMPAD_7;
			case GLFW_KEY_KP_8: return KEY_NUMPAD_8;
			case GLFW_KEY_KP_9: return KEY_NUMPAD_9;

			case GLFW_KEY_KP_DECIMAL: return KEY_NUMPAD_DOT;
			case GLFW_KEY_KP_DIVIDE: return KEY_NUMPAD_DIVIDE;
			case GLFW_KEY_KP_MULTIPLY: return KEY_NUMPAD_MULTIPLY;
			case GLFW_KEY_KP_SUBTRACT: return KEY_NUMPAD_SUBTRACT;
			case GLFW_KEY_KP_ADD: return KEY_NUMPAD_ADD;
			case GLFW_KEY_KP_ENTER: return KEY_NUMPAD_ENTER;
			case GLFW_KEY_KP_EQUAL: return -1;

			case GLFW_KEY_LEFT_SHIFT: return KEY_LSHIFT;
			case GLFW_KEY_LEFT_CONTROL: return KEY_LCONTROL;
			case GLFW_KEY_LEFT_ALT: return KEY_LALT;
			case GLFW_KEY_LEFT_SUPER: return -1;
			case GLFW_KEY_RIGHT_SHIFT: return KEY_SHIFT;
			case GLFW_KEY_RIGHT_CONTROL: return KEY_CONTROL;
			case GLFW_KEY_RIGHT_ALT: return KEY_ALT;
			case GLFW_KEY_RIGHT_SUPER: return -1;
			case GLFW_KEY_MENU: return -1;
			}
			return 0;
		}

		bool GraphicsWrapper::InitializeWindowContext() {
			/* Initialize the library */
			if (!glfwInit())
				return -1;

			/* Create a windowed mode window and its OpenGL context */
			window = glfwCreateWindow(width, height, title, NULL, NULL);
			if (!window) {
				glfwTerminate();
				return -1;
			}

			/* Make the window's context current */
			glfwMakeContextCurrent(window);

			glfwSetKeyCallback(window, key_callback);
			glfwSetWindowFocusCallback(window, window_focus_callback);
			glfwSetCursorEnterCallback(window, cursor_enter_callback);
			glfwSetMouseButtonCallback(window, mouse_button_callback);
			glfwSetWindowSizeCallback(window, window_size_callback);

			g_input = input;
		}

		void GraphicsWrapper::HandleEvents() {
			if (input == NULL) {
				std::cout << "No Interface!" << std::endl;
				return;
			}

			glfwPollEvents();
		}
	}
}

#endif