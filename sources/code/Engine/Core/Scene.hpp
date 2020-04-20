#ifndef _LEVEL_LOADER_H
#define _LEVEL_LOADER_H

#include <vector>
#include <string>

class Space;
class GameObject;

class Scene {
public:
	Scene(Space* space, std::string path);
	Scene(const Scene &s);
	~Scene();
	void clear();
	std::string getName();
	std::string getPath();
	void reload();
private:
	bool loadPrefab(std::string name, GameObject& game_object);
	void loadLevel(std::string path);
private:
	Space* space_;
	std::string path_;
	std::string name_;
};

#endif