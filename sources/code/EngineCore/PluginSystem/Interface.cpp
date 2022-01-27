#include "Interface.hpp"
#include "Manager.hpp"
#include "../EngineCore.hpp"
#include "../Logger.hpp"
#include "EngineCore/ECS/SystemRegistrar.hpp"
#include <Common/Window/Window.hpp>
using namespace Grindstone;

Plugins::Interface::Interface(Manager* manager) 
	: manager(manager) {
}

EngineCore* Plugins::Interface::GetEngineCore() {
	return manager->engineCore;
}

GraphicsAPI::Core* Plugins::Interface::GetGraphicsCore() {
	return graphicsCore;
}

bool Plugins::Interface::LoadPlugin(const char* name) {
	return manager->Load(name);
}

void Plugins::Interface::LoadPluginCritical(const char* name) {
	manager->LoadCritical(name);
}

void Plugins::Interface::RegisterGraphicsCore(GraphicsAPI::Core* gw) {
	graphicsCore = gw;
	manager->engineCore->RegisterGraphicsCore(gw);
}

void Plugins::Interface::RegisterWindowManager(WindowManager* windowManager) {
	GetEngineCore()->windowManager = windowManager;
}

void Plugins::Interface::RegisterDisplayManager(DisplayManager* displayManager) {
	GetEngineCore()->displayManager = displayManager;
}

Window* Plugins::Interface::CreateDisplayWindow(Window::CreateInfo& ci) {
	if (windowFactoryFn) {
		return windowFactoryFn(ci);
	}

	return nullptr;
}

Display Plugins::Interface::GetMainDisplay() {
	if (!getMainDisplayFn) {
		return Display();
	}

	return getMainDisplayFn();
}

uint8_t Plugins::Interface::CountDisplays() {
	if (!countDisplaysFn) {
		return -1;
	}

	return countDisplaysFn();
}


void Plugins::Interface::EnumerateDisplays(Display* displays) {
	if (enumerateDisplaysFn) {
		enumerateDisplaysFn(displays);
	}
}

void Plugins::Interface::RegisterSystem(const char* name, ECS::SystemFactory factory) {
	systemRegistrar->RegisterSystem(name, factory);
}
