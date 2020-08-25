#include "Interface.hpp"
#include "Manager.hpp"
#include "../EngineCore.hpp"
#include "../Logger.hpp"
#include <Common/Window/Window.hpp>
using namespace Grindstone;

Plugins::Interface::Interface(Manager* manager, ECS::Core* core) : manager_(manager), ecs_core_(core) {}

void Plugins::Interface::addWindow(Window* win) {
	manager_->engine_core_->addWindow(win);
}

EngineCore* Plugins::Interface::getEngineCore() {
	return manager_->engine_core_;
}

GraphicsAPI::Core* Plugins::Interface::getGraphicsCore() {
	return graphics_core_;
}

ECS::Core* Plugins::Interface::getEcsCore() {
	return ecs_core_;
}

void Plugins::Interface::log(const char* msg) {
	GRIND_LOG(msg);
}

bool Plugins::Interface::loadPlugin(const char* name) {
	return manager_->load(name);
}

void Plugins::Interface::loadPluginCritical(const char* name) {
	manager_->loadCritical(name);
}

void Plugins::Interface::registerGraphicsCore(GraphicsAPI::Core* gw) {
	graphics_core_ = gw;
	manager_->engine_core_->registerGraphicsCore(gw);
}

void Plugins::Interface::registerWindowFactory(Window* (*wf)(Window::CreateInfo&)) {
	fn_window_factory_ = wf;
}

void Plugins::Interface::registerDisplayFunctions(Display(*fn_get_main_display)(), uint8_t(*fn_count_displays)(), void(*fn_enumerate_displays)(Display*)) {
	fn_get_main_display_ = fn_get_main_display;
	fn_count_displays_ = fn_count_displays;
	fn_enumerate_displays_ = fn_enumerate_displays;
}

Window* Plugins::Interface::createWindow(Window::CreateInfo& ci) {
	if (fn_window_factory_) {
		return fn_window_factory_(ci);
	}

	return nullptr;
}

Display Plugins::Interface::getMainDisplay()
{
	return fn_get_main_display_();
}

uint8_t Plugins::Interface::countDisplays() {
	if (fn_count_displays_) {
		return fn_count_displays_();
	}

	return -1;
}


void Plugins::Interface::enumerateDisplays(Display* displays) {
	if (fn_enumerate_displays_) {
		fn_enumerate_displays_(displays);
	}
}



void Plugins::Interface::registerSystem(const char* name, ECS::SystemFactory factory) {
	ecs_core_->registerSystem(name, factory);
}

void Plugins::Interface::registerComponentType(const char* name, ECS::ComponentFactory factory) {
	ecs_core_->registerComponentType(name, factory);
}