#ifndef _SPACE_H
#define _SPACE_H

// STD Headers
#include <vector>
#include <string>

// Grindstone Headers
#include <Engine/Core/GameObject.hpp>

#ifdef _MSC_VER
#ifdef GRIND_ENGINE_DLL
#define PP __declspec(dllexport)
#else
#define PP __declspec(dllimport)
#endif
#else
#define PP
#endif

class Scene;

class Space {
public:
	Space(const char *name);
	Space(const Space &s);
	void clear();
	~Space();
public:
	PP virtual GameObjectHandle createObject(const char *name);
	GameObject& getObject(GameObjectHandle handle);
	PP virtual SubSystem *getSubsystem(ComponentType type);
	Scene* getScene(std::string levelname);
	Scene* getScene(unsigned int id);
	std::string getName();
	size_t getNumObjects();
	PP virtual Scene* addScene(const char *levelname);
private:
	SubSystem* addSubsystem(SubSystem* system);
private:
	std::string name_;
	std::vector<Scene*> scenes_;
	std::vector<GameObject> objects_;
	SubSystem *subsystems_[NUM_COMPONENTS];
};

#endif