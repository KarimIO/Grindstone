#ifndef _VULKAN_GRAPHICS_H
#define _VULKAN_GRAPHICS_H

#include <iostream>
#include "../GraphicsCommon/GLDefDLL.h"

#ifdef _WIN32
#include <Windows.h>
#define VK_USE_PLATFORM_WIN32_KHR
#endif

#ifdef __linux__
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/keysymdef.h>
#endif

#include <vulkan/vulkan.h>

#define CLEAR_ALL 0
#define CLEAR_COLOR 1
#define CLEAR_DEPTH 2

enum ShapeType {
	SHAPE_POINTS = 0,
	SHAPE_LINES,
	SHAPE_LINE_LOOP,
	SHAPE_LINE_STRIP,
	SHAPE_TRIANGLES,
	SHAPE_TRIANGLE_STRIP,
	SHAPE_TRIANGLE_FAN,
	SHAPE_QUADS,
	SHAPE_PATCHES
};

class InputInterface;

class GRAPHICS_EXPORT_CLASS GraphicsWrapper {
private:
#ifdef _WIN32
	HWND	window_handle;
	HGLRC	hRC;
	HDC		hDC;
#endif
#ifdef __linux__
	Display* display;
	Window *window;
	Screen* screen;
	int screenID;
#endif
	VkInstance vkInstance;
	VkSurfaceKHR vkSurface;
	bool vkInitializeHandleResult(VkResult result);
	void vkInitialize();
	bool vkCheckValidation();
	void vkCreateValidation();

	VkDebugReportCallbackEXT debugCallbackHandler;
	bool debug = true;
	static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(VkDebugReportFlagsEXT flags, VkDebugReportObjectTypeEXT objType, uint64_t obj, size_t location, int32_t code, const char* layerPrefix, const char* msg, void* userData);
public:
	virtual bool InitializeWindowContext();
	virtual bool InitializeGraphics();
	virtual void DrawVertexArray(uint32_t numIndices);
	virtual void DrawBaseVertex(ShapeType type, const void *baseIndex, uint32_t baseVertex, uint32_t numIndices);
	virtual void SwapBuffer();
	virtual void SetResolution(int x, int y, uint32_t width, uint32_t height);
	virtual void Clear(unsigned int clearTarget);
	virtual void SetTesselation(int verts);
#ifdef _WIN32
	virtual void SetWindowContext(HWND);
#endif
#ifdef __linux__
	virtual void SetWindowContext(Display*, Window *, Screen* screen, int);
#endif
};

extern "C" GRAPHICS_EXPORT GraphicsWrapper* createGraphics();

#endif