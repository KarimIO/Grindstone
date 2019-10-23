#ifndef _DLL_GRAPHICS_H
#define _DLL_GRAPHICS_H

#include "DLLHandler.hpp"
#include "GraphicsLanguage.hpp"

class GraphicsWrapper;

class DLLGraphics : public DLLHandler {
public:
	DLLGraphics();
	GraphicsWrapper *getWrapper();
	void setup();
	void reload();
	~DLLGraphics();
private:
	GraphicsWrapper *wrapper_;
	void(*pfnDeleteGraphics)(GraphicsWrapper*);
};

#endif