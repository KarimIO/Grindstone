#include "VkGraphicsWrapper.h"
#include "../GraphicsCommon/Framebuffer.h"
#include "../GraphicsCommon/VertexArrayObject.h"
#include "../GraphicsCommon/Shader.h"
#include "../GraphicsCommon/Texture.h"
#include <vector>

const int validation_layer_count = 1;
const char* validation_layer_names[]{
	"VK_LAYER_LUNARG_standard_validation"
};

VkResult CreateDebugReportCallbackEXT(VkInstance instance, const VkDebugReportCallbackCreateInfoEXT* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkDebugReportCallbackEXT* pCallback) {
	auto func = (PFN_vkCreateDebugReportCallbackEXT)vkGetInstanceProcAddr(instance, "vkCreateDebugReportCallbackEXT");
	if (func != nullptr) {
		return func(instance, pCreateInfo, pAllocator, pCallback);
	}
	else {
		return VK_ERROR_EXTENSION_NOT_PRESENT;
	}
}

void DestroyDebugReportCallbackEXT(VkInstance instance, VkDebugReportCallbackEXT callback, const VkAllocationCallbacks* pAllocator) {
	auto func = (PFN_vkDestroyDebugReportCallbackEXT)vkGetInstanceProcAddr(instance, "vkDestroyDebugReportCallbackEXT");
	if (func != nullptr) {
		func(instance, callback, pAllocator);
	}
}

const std::vector<const char*> deviceExtensions = {
	VK_KHR_SWAPCHAIN_EXTENSION_NAME
};

GRAPHICS_EXPORT GraphicsWrapper* createGraphics() {
	std::cout << "Creating the graphics (dll-side)\n";
	return new GraphicsWrapper;
}

bool GraphicsWrapper::vkInitializeHandleResult(VkResult result) {
	const char *message;
	switch (result) {
	case VK_SUCCESS:
		message = "Vulkan has successfully initialized.\n";
		return true;
		break;
	case VK_ERROR_OUT_OF_HOST_MEMORY:
		message = "Error: Vulkan cannot allocate enough memory to initialize.\n";
		return false;
		break;
	case VK_ERROR_OUT_OF_DEVICE_MEMORY:
		message = "Error: Vulkan cannot allocate enough memory to initialize.\n";
		return false;
		break;
	case VK_ERROR_LAYER_NOT_PRESENT:
		message = "Error: Vulkan cannot load one of its layers.\n";
		return false;
		break;
	case VK_ERROR_EXTENSION_NOT_PRESENT:
		message = "Error: Vulkan cannot load one or more of its extensions.\n";
		return false;
		break;
	case VK_ERROR_INCOMPATIBLE_DRIVER:
		message = "Error: Vulkan cannot find a compatible driver.\n";
		return false;
		break;
	default:
	case VK_ERROR_INITIALIZATION_FAILED:
		message = "Error: Vulkan failed to load for unspecified reasons.\n";
		return false;
		break;
	}

	fprintf(stderr, "%s\n", message);
}

bool GraphicsWrapper::vkCheckValidation() {
	uint32_t layerCount;
	vkEnumerateInstanceLayerProperties(&layerCount, NULL);
	VkLayerProperties *layers = new VkLayerProperties[layerCount];
	vkEnumerateInstanceLayerProperties(&layerCount, layers);

	std::string layerName = "VK_LAYER_LUNARG_standard_validation";
	for (uint32_t i = 0; i < layerCount; i++) {
		if (layerName == layers[i].layerName)
			return true;
	}

	fprintf(stderr, "VULKAN: Failed to set up validation layers, continuing anyways.\n");
	return false;
}

void GraphicsWrapper::vkCreateValidation() {
	if (!debug) return;

	VkDebugReportCallbackCreateInfoEXT createInfo = {};
	createInfo.sType = VK_STRUCTURE_TYPE_DEBUG_REPORT_CALLBACK_CREATE_INFO_EXT;
	createInfo.flags = VK_DEBUG_REPORT_ERROR_BIT_EXT | VK_DEBUG_REPORT_WARNING_BIT_EXT;
	createInfo.pfnCallback = debugCallback;

	if (CreateDebugReportCallbackEXT(vkInstance, &createInfo, nullptr, &debugCallbackHandler) != VK_SUCCESS) {
		throw std::runtime_error("VULKAN: Failed to set up debug callback!");
	}
}

VKAPI_ATTR VkBool32 VKAPI_CALL GraphicsWrapper::debugCallback(VkDebugReportFlagsEXT flags, VkDebugReportObjectTypeEXT objType, uint64_t obj, size_t location, int32_t code, const char * layerPrefix, const char * msg, void * userData) {
	std::cerr << "VULKAN VALIDATION: " << msg << std::endl;

	return VK_FALSE;
}

void GraphicsWrapper::vkInitialize() {

	// Vulkan Application Info
	VkApplicationInfo vkAppInfo{
		VK_STRUCTURE_TYPE_APPLICATION_INFO,	// Type of Struct - Application Info
		nullptr,							// For extensions - 
		"Grind App",						// Application Name
		VK_MAKE_VERSION(1, 0, 0),			// Application Version
		"The Grind Engine",					// Engine Name
		VK_MAKE_VERSION(1, 0, 0),			// Engine Version
		VK_API_VERSION_1_0					// API version
	};

	const int enabled_extension_count = 3;
	const char* enabled_extensions[]{
		VK_KHR_SURFACE_EXTENSION_NAME,        // without surface extension we actually can't show anything on screen so.. it's pretty necessary
		VK_KHR_WIN32_SURFACE_EXTENSION_NAME,  // simply surface extension for windows
		VK_EXT_DEBUG_REPORT_EXTENSION_NAME,   // validation & debug extension.. remove it for release..
	};

	// Vulkan Instance Info / Parameters
	VkInstanceCreateInfo vkInstInfo;
	vkInstInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;	// Type of Struct - Instance
	vkInstInfo.pNext = NULL;									// Next Instance
	vkInstInfo.flags = 0;										// Flags
	vkInstInfo.pApplicationInfo = &vkAppInfo;					// Application Info
	vkInstInfo.enabledExtensionCount = enabled_extension_count;	// Number of Extensions
	vkInstInfo.ppEnabledExtensionNames = enabled_extensions;	// Value of Extensions
	if (debug) {
		vkInstInfo.enabledLayerCount = validation_layer_count;		// Number of Layers
		vkInstInfo.ppEnabledLayerNames = validation_layer_names;	// Value of Layers
	}
	else vkInstInfo.enabledLayerCount = 0;

	if (!vkInitializeHandleResult(vkCreateInstance(&vkInstInfo, NULL, &vkInstance)))
		throw "VULKAN: Cannot Initialize.";
}

GRAPHICS_EXPORT VertexArrayObject* createVAO() {
	return nullptr;
}

GRAPHICS_EXPORT VertexBufferObject* createVBO() {
	return nullptr;
}

GRAPHICS_EXPORT ShaderProgram* createShader() {
	return nullptr;
}

GRAPHICS_EXPORT Texture* createTexture() {
	return nullptr;
}

GRAPHICS_EXPORT Framebuffer* createFramebuffer() {
	return nullptr;
}

bool GraphicsWrapper::InitializeGraphics() {
	if (debug)
		debug = vkCheckValidation(); // Remove debug if validation doesn't exist.

	vkInitialize();
	vkCreateValidation();

	return true;
}

void GraphicsWrapper::DrawVertexArray(uint32_t numVertices) {
}

void GraphicsWrapper::DrawBaseVertex(ShapeType type, const void *baseIndex, uint32_t baseVertex, uint32_t numIndices) {	
}

void GraphicsWrapper::Clear(unsigned int clearTarget) {
}

void GraphicsWrapper::SetResolution(int x, int y, uint32_t w, uint32_t h) {
}

void GraphicsWrapper::SetTesselation(int verts) {
}