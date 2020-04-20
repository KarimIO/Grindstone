#ifndef _DLL_ENGINE_H
#define _DLL_ENGINE_H

#include <Engine/Utilities/DLLHandler.hpp>

class DLLEngine : public DLLHandler {
public:
	void initializeDLL();
	void *(*launchEngine)();
	void (*deleteEngine)(void*);
private:
};

#endif