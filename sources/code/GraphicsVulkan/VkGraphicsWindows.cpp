#ifdef _WIN32
#include "VkGraphicsWrapper.h"

bool GraphicsWrapper::InitializeWindowContext() {
	/*const VkWin32SurfaceCreateInfoKHR surface_create_info{
		VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR,
		NULL,
		0,
		GetModuleHandle(NULL),
		window_handle
	};

	if (vkCreateWin32SurfaceKHR(vkInstance, &surface_create_info, nullptr, &vkSurface) != VK_SUCCESS)
		return false;*/

	return true;
}

void GraphicsWrapper::SetWindowContext(HWND hwnd) {
	window_handle = hwnd;
}


void GraphicsWrapper::SwapBuffer() {
	SwapBuffers(hDC);
}

#endif