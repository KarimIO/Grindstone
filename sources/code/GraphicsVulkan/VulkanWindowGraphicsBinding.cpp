#pragma once

#include "../WindowModule/BaseWindow.hpp"

namespace Grindstone {
	namespace GraphicsAPI {
		void VulkanWindowGraphicsBinding::initialize(BaseWindow *window) {
#ifdef VK_USE_PLATFORM_WIN32_KHR
			VkWin32SurfaceCreateInfoKHR surfaceCreateInfo;
			surfaceCreateInfo.sType = VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR;
			surfaceCreateInfo.hinstance = GetModuleHandle(NULL);
			surfaceCreateInfo.hwnd = ((Win32Window *)window)->getHandle();
			surfaceCreateInfo.pNext = VK_NULL_HANDLE;
			surfaceCreateInfo.flags = 0;

			if (vkCreateWin32SurfaceKHR(instance_, &surfaceCreateInfo, nullptr, &surface_) != VK_SUCCESS)
				return;
#elif defined(VK_USE_PLATFORM_XLIB_KHR)
			VkXlibSurfaceCreateInfoKHR surfaceCreateInfo;
			surfaceCreateInfo.sType = VK_STRUCTURE_TYPE_XLIB_SURFACE_CREATE_INFO_KHR;
			surfaceCreateInfo.hinstance = GetModuleHandle(NULL);
			surfaceCreateInfo.dpy = xDisplay;
			surfaceCreateInfo.window = xWindow;
			surfaceCreateInfo.pNext = VK_NULL_HANDLE;
			surfaceCreateInfo.flags = 0;

			if (vkCreateXlibSurfaceKHR(instance_, &surfaceCreateInfo, nullptr, &surface_) != VK_SUCCESS)
				return;
#endif
		}

		void :: {
			wglMakeCurrent( hDC, hRC );
		}
	};
};