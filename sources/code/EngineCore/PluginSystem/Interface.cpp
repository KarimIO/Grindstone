#include "Interface.hpp"
#include "Manager.hpp"
#include "../EngineCore.hpp"
#include "../Logger.hpp"
#include <Common/Window/Window.hpp>
using namespace Grindstone;

Plugins::Interface::Interface(Manager* manager, ECS::Core* core) 
	: manager(manager), ecsCore(core) {
}

void Plugins::Interface::addWindow(Window* win) {
	manager->engineCore->addWindow(win);
}

EngineCore* Plugins::Interface::getEngineCore() {
	return manager->engineCore;
}

GraphicsAPI::Core* Plugins::Interface::getGraphicsCore() {
	return graphicsCore;
}

ECS::Core* Plugins::Interface::getEcsCore() {
	return ecsCore;
}

void Plugins::Interface::log(const char* msg) {
	GRIND_LOG(msg);
}

bool Plugins::Interface::loadPlugin(const char* name) {
	return manager->load(name);
}

void Plugins::Interface::loadPluginCritical(const char* name) {
	manager->loadCritical(name);
}

void Plugins::Interface::registerGraphicsCore(GraphicsAPI::Core* gw) {
	graphicsCore = gw;
	manager->engineCore->registerGraphicsCore(gw);
}

void Plugins::Interface::registerWindowFactory(Window* (*wf)(Window::CreateInfo&)) {
	windowFactoryFn = wf;
}

void Plugins::Interface::registerDisplayFunctions(
	Display(*getMainDisplayFn)(),
	uint8_t(*countDisplaysFn)(),
	void(*enumerateDisplaysFn)(Display*)
) {
	this->getMainDisplayFn = getMainDisplayFn;
	this->countDisplaysFn = countDisplaysFn;
	this->enumerateDisplaysFn = enumerateDisplaysFn;
}

Window* Plugins::Interface::createWindow(Window::CreateInfo& ci) {
	if (windowFactoryFn) {
		return windowFactoryFn(ci);
	}

	return nullptr;
}

Display Plugins::Interface::getMainDisplay() {
	if (!getMainDisplayFn) {
		return Display();
	}

	return getMainDisplayFn();
}

uint8_t Plugins::Interface::countDisplays() {
	if (!countDisplaysFn) {
		return -1;
	}

	return countDisplaysFn();
}


void Plugins::Interface::enumerateDisplays(Display* displays) {
	if (enumerateDisplaysFn) {
		enumerateDisplaysFn(displays);
	}
}



void Plugins::Interface::registerSystem(const char* name, ECS::SystemFactory factory) {
	ecsCore->registerSystem(name, factory);
}

void Plugins::Interface::registerComponentType(const char* name, ECS::ComponentFactory factory) {
	ecsCore->registerComponentType(name, factory);
}