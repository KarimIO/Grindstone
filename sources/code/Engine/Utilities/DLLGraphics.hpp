#ifndef _DLL_GRAPHICS_H
#define _DLL_GRAPHICS_H

#include "DLLHandler.hpp"
#include "GraphicsLanguage.hpp"

namespace Grindstone {
	namespace GraphicsAPI {
		class GraphicsWrapper;
	}
}

class DLLGraphics : public DLLHandler {
public:
	DLLGraphics();
	Grindstone::GraphicsAPI::GraphicsWrapper *getWrapper();
	void setup();
	void reload();
	~DLLGraphics();
private:
	Grindstone::GraphicsAPI::GraphicsWrapper *wrapper_;
	void(*pfnDeleteGraphics)(Grindstone::GraphicsAPI::GraphicsWrapper*);
};

#endif