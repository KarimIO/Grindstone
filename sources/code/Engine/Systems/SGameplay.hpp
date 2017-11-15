#ifndef _S_GAMEPLAY_H
#define _S_GAMEPLAY_H

#include <set>
#include <vector>

class GBase {
public:
};

class SGBase {
public:
	virtual void Update(double dt) = 0;
};

class CGameplay {
public:
	void AddComponent(GBase *component);
	void RemoveComponent(GBase *component);
private:
	std::vector<GBase *> subcomponents;
};

class SGameplay {
public:
	void AddComponent(CGameplay *component);
	void RemoveComponent(CGameplay *component);

	void AddSystem(SGBase *system);
	void RemoveSystem(SGBase *system);

	void Update(double d);
	~SGameplay();
private:
	std::set<SGBase *> subsystems;
	std::vector<CGameplay *> components;
};

#endif