#include <Common/Window/Window.hpp>
#include <EngineCore/AssetRenderer/AssetRendererManager.hpp>
#include <EngineCore/ECS/SystemRegistrar.hpp>
#include <EngineCore/Logger.hpp>
#include <EngineCore/EngineCore.hpp>

#include "Interface.hpp"
#include "Manager.hpp"
using namespace Grindstone;

Plugins::Interface::Interface(Manager* manager) 
	: manager(manager), engineCore(&EngineCore::GetInstance()) {
}

void Plugins::Interface::EditorRegisterAssetImporter(const char* extension, void(*importer)(const std::filesystem::path&)) {

}

void Plugins::Interface::EditorRegisterAssetTemplate(AssetType assetType, const char* name, const char* extension, const void* const sourcePtr, size_t sourceSize) {

}

void Plugins::Interface::EditorDeregisterAssetImporter(const char* extension) {

}

void Plugins::Interface::EditorDeregisterAssetTemplate(AssetType assetType) {

}

void Plugins::Interface::Print(LogSeverity logSeverity, const char* message) {
	Logger::Print(logSeverity, message);
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

void Plugins::Interface::SetReloadCsharpCallback(std::function<void()> callback) {
	GetEngineCore()->callbackReloadCsharp = callback;
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

void Plugins::Interface::RegisterAssetRenderer(BaseAssetRenderer* assetRenderer) {
	EngineCore::GetInstance().assetRendererManager->AddAssetRenderer(assetRenderer);
}

void Plugins::Interface::RegisterAssetType(AssetType assetType, const char* typeName, AssetImporter* assetImporter) {
	engineCore->assetManager->RegisterAssetType(assetType, typeName, assetImporter);
}
