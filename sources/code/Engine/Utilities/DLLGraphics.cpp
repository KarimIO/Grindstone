#include "DLLGraphics.hpp"
#include <GraphicsCommon/GraphicsWrapper.hpp>
#include <Engine/Utilities/GraphicsLanguage.hpp>

DLLGraphics::DLLGraphics() {
}

BaseWindow* DLLGraphics::createWindow() {
	return fnCreateWindow();
}

Grindstone::GraphicsAPI::GraphicsWrapper* DLLGraphics::createGraphicsWrapper() {
	return (Grindstone::GraphicsAPI::GraphicsWrapper * )fnCreateGraphicsWrapper();
}

void DLLGraphics::deleteGraphicsWrapper(Grindstone::GraphicsAPI::GraphicsWrapper* wrapper) {
	fnDeleteGraphicsWrapper(wrapper);
}

void DLLGraphics::setup(GraphicsLanguage settings) {
	{
		GRIND_PROFILE_SCOPE("Load DLLGraphics");

		std::string library;
		switch (settings) {
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

		/*fnCreateWindow = (BaseWindow * (*)())getFunction("createWindow");
		if (!fnCreateWindow) {
			throw std::runtime_error("Cannot get createWindow function!\n");
		}*/

		fnCreateGraphicsWrapper = (void* (*)())getFunction("createGraphics");
		if (!fnCreateGraphicsWrapper) {
			throw std::runtime_error("Cannot get createGraphics function!\n");
		}

		fnDeleteGraphicsWrapper = (void(*)(void*))getFunction("deleteGraphics");
		if (!fnDeleteGraphicsWrapper) {
			throw std::runtime_error("Cannot get deleteGraphics function!\n");
		}
	}	
}

void DLLGraphics::reload() {
	/*wrapper_->closing_state_ = Grindstone::GraphicsAPI::GraphicsWrapper::WindowClosingState::Closing;
	// Remove previous instance of Wrapper
	if (wrapper_) {
		pfnDeleteGraphics(wrapper_);
		wrapper_ = nullptr;
	}

	// Close DLL
	close();

	// Setup the new DLL and Wrapper
	setup();*/
}

DLLGraphics::~DLLGraphics() {
}