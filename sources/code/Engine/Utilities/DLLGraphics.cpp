#include "DLLGraphics.hpp"
#include "Core/Engine.hpp"
#include <GraphicsWrapper.hpp>

DLLGraphics::DLLGraphics() {
	setup();
}

GraphicsWrapper *DLLGraphics::getWrapper() {
	return wrapper_;
}

void DLLGraphics::setup() {
	auto settings = engine.getSettings();

	std::string library;
	switch (settings->graphics_language_) {
	default:
		library = "graphicsgl";
		break;
#ifndef __APPLE__
	case GRAPHICS_VULKAN:
		library = "graphicsvk";
		break;
#endif
#ifdef _WIN32
	case GRAPHICS_DIRECTX:
		library = "graphicsdx";
		break;
#endif
#ifdef __APPLE__
	case GRAPHICS_METAL:
		library = "graphicsml";
		break;
#endif
	};

	initialize(library);

	InstanceCreateInfo createInfo;
	createInfo.width = settings->resolution_x_;
	createInfo.height = settings->resolution_y_;
	createInfo.vsync = settings->vsync_;
	createInfo.inputInterface = (InputInterface *)engine.getInputManager();
	createInfo.title = "Grindstone";
#ifdef NDEBUG
	createInfo.debug = false;
#else
	createInfo.debug = true;
#endif

	GraphicsWrapper* (*pfnCreateGraphics)(InstanceCreateInfo) = (GraphicsWrapper* (*)(InstanceCreateInfo))getFunction("createGraphics");
	if (!pfnCreateGraphics) {
		throw std::runtime_error("Cannot get createGraphics function!\n");
	}

	pfnDeleteGraphics = (void(*)(GraphicsWrapper*))getFunction("deleteGraphics");
	if (!pfnDeleteGraphics) {
		throw std::runtime_error("Cannot get deleteGraphics function!\n");
	}

	wrapper_ = (GraphicsWrapper*)pfnCreateGraphics(createInfo);
}

void DLLGraphics::reload() {
	wrapper_->closing_state_ = GraphicsWrapper::WindowClosingState::Closing;
	// Remove previous instance of Wrapper
	if (wrapper_) {
		pfnDeleteGraphics(wrapper_);
		wrapper_ = nullptr;
	}

	// Close DLL
	close();

	// Setup the new DLL and Wrapper
	setup();
}

DLLGraphics::~DLLGraphics() {
	if (wrapper_) {
		pfnDeleteGraphics(wrapper_);
		wrapper_ = nullptr;
	}
}