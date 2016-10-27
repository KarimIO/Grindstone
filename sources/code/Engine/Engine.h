#ifndef _ENGINE_H

class Engine {
public:
#ifdef UseGameInstance
	static Engine *GetInstance();
#else
	static Engine &GetInstance();
#endif
	Engine();
	~Engine();
};

#endif