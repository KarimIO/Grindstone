#include "DLLGraphics.hpp"
#include "Core/Engine.hpp"
#include <GraphicsWrapper.hpp>

DLLGraphics::DLLGraphics() {
	setup();
}

Grindstone::GraphicsAPI::GraphicsWrapper *DLLGraphics::getWrapper() {
	return wrapper_;
}

void DLLGraphics::setup() {
	Grindstone::GraphicsAPI::GraphicsWrapper* (*pfnCreateGraphics)(Grindstone::GraphicsAPI::InstanceCreateInfo);
	auto settings = engine.getSettings();
	{
		GRIND_PROFILE_SCOPE("Load DLLGraphics");
		auto settings = engine.getSettings();

		std::string library;
		switch (settings->graphics_language_) {
		default:
			library = "graphicsgl";
			break;
#ifndef __APPLE__
		case GraphicsLanguage::Vulkan:
			library = "graphicsvk";
			break;
#endif
#ifdef _WIN32
		case GraphicsLanguage::DirectX:
			library = "graphicsdx";
			break;
#endif
#ifdef __APPLE__
		case GraphicsLanguage::Metal:
			library = "graphicsml";
			break;
#endif
		};

		initialize(library);

		pfnCreateGraphics = (Grindstone::GraphicsAPI::GraphicsWrapper* (*)(Grindstone::GraphicsAPI::InstanceCreateInfo))getFunction("createGraphics");
		if (!pfnCreateGraphics) {
			throw std::runtime_error("Cannot get createGraphics function!\n");
		}

		pfnDeleteGraphics = (void(*)(Grindstone::GraphicsAPI::GraphicsWrapper*))getFunction("deleteGraphics");
		if (!pfnDeleteGraphics) {
			throw std::runtime_error("Cannot get deleteGraphics function!\n");
		}
	}

	GRIND_PROFILE_SCOPE("Create GraphicsWrapper");
	Grindstone::GraphicsAPI::InstanceCreateInfo createInfo;
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

	wrapper_ = (Grindstone::GraphicsAPI::GraphicsWrapper*)pfnCreateGraphics(createInfo);
}

void DLLGraphics::reload() {
	wrapper_->closing_state_ = Grindstone::GraphicsAPI::GraphicsWrapper::WindowClosingState::Closing;
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