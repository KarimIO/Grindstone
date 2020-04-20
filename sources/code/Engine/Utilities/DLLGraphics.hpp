#ifndef _DLL_GRAPHICS_H
#define _DLL_GRAPHICS_H

#include "../Utilities/GraphicsLanguage.hpp"
#include "DLLHandler.hpp"
#include <stb\stb_image.h>

namespace Grindstone {
	namespace GraphicsAPI {
		class GraphicsWrapper;
	}
}

class BaseWindow;

class DLLGraphics : public DLLHandler {
public:
	DLLGraphics();
	BaseWindow* DLLGraphics::createWindow();
	Grindstone::GraphicsAPI::GraphicsWrapper* createGraphicsWrapper();
	void deleteGraphicsWrapper(Grindstone::GraphicsAPI::GraphicsWrapper *wrappper);
	void setup(GraphicsLanguage settings);
	void reload();
	~DLLGraphics();
private:
	BaseWindow* (*fnCreateWindow)();
	void* (*fnCreateGraphicsWrapper)();
	void(*fnDeleteGraphicsWrapper)(void*);
};

#endif