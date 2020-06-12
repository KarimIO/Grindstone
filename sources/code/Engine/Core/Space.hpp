#ifndef _SPACE_H
#define _SPACE_H

// STD Headers
#include <vector>
#include <string>

// Grindstone Headers
#include <Engine/Core/GameObject.hpp>

#include <Engine/Rendering/GizmoRenderer.hpp>

class Space {
public:
	Space(const char *name);
	Space(const Space &s);
	~Space();
public:
	virtual void clear();
	virtual bool loadFromScene(std::string path);
	virtual bool reloadScene();
	virtual GameObject& createObject(const char *name);
	virtual GameObject &getObject(GameObjectHandle handle);
	virtual SubSystem *getSubsystem(ComponentType type);
	virtual std::string getName();
	virtual std::string getPath();
	virtual size_t getNumObjects();
	virtual Grindstone::GizmoRenderer& getGizmoRenderer();
private:
	SubSystem* addSubsystem(SubSystem* system);
	bool loadPrefab(std::string name, GameObject& game_object);
private:
	std::string name_;
	std::string path_;
	std::vector<GameObject> objects_;
	SubSystem *subsystems_[NUM_COMPONENTS];
	Grindstone::GizmoRenderer gizmo_renderer_;
};

#endif