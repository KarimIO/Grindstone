#include "Engine.h"

#ifdef UseClassInstance
	Engine *Engine::_instance=0;
#endif

Engine::Engine() {
	// window->Initialize();
	// graphics->Initialize();
	
}

#ifdef UseClassInstance
Engine *Engine::GetInstance() {
	if (!_instance)
		_instance = new Engine();
	return _instance;
}
#else
Engine &Engine::GetInstance() {
	static Engine newEngine;
	return newEngine;
}
#endif


Engine::~Engine() {

}
