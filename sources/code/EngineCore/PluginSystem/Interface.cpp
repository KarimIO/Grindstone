#include "Interface.hpp"
#include "Interface.hpp"
#include "Interface.hpp"
#include <Common/Window/Window.hpp>
#include <EngineCore/AssetRenderer/AssetRendererManager.hpp>
#include <EngineCore/ECS/SystemRegistrar.hpp>
#include <EngineCore/Logger.hpp>
#include <EngineCore/Utils/MemoryAllocator.hpp>
#include <EngineCore/EngineCore.hpp>
#include <EngineCore/WorldContext/WorldContextManager.hpp>

// TODO: Define this type.
#ifndef GS_RUNTIME
#include <Editor/AssetRegistry.hpp>
#endif

#include "Interface.hpp"
#include "Manager.hpp"
using namespace Grindstone;

Plugins::Interface::Interface(Manager* manager) 
	: manager(manager), engineCore(&EngineCore::GetInstance()) {
}

void Plugins::Interface::SetEditorInterface(BaseEditorInterface* editorInterface) {
	this->editorInterface = editorInterface;
}

Plugins::BaseEditorInterface* Plugins::Interface::GetEditorInterface() const {
	return editorInterface;
}

EngineCore* Plugins::Interface::GetEngineCore() {
	return manager->engineCore;
}

GraphicsAPI::Core* Plugins::Interface::GetGraphicsCore() {
	return graphicsCore;
}

Grindstone::Logger::LoggerState* Plugins::Interface::GetLoggerState() const {
	return Grindstone::Logger::GetLoggerState();
}

Grindstone::Memory::AllocatorCore::AllocatorState* Plugins::Interface::GetAllocatorState() const {
	return Grindstone::Memory::AllocatorCore::GetAllocatorState();
}

void Plugins::Interface::RegisterWorldContextFactory(Grindstone::HashedString contextName, Grindstone::UniquePtr<Grindstone::WorldContext>(*factoryFn)()) {
	Grindstone::EngineCore::GetInstance().GetWorldContextManager()->Register(contextName, factoryFn);
}

void Plugins::Interface::UnregisterWorldContextFactory(Grindstone::HashedString contextName) {
	Grindstone::EngineCore::GetInstance().GetWorldContextManager()->Unregister(contextName);
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

Grindstone::HashedString::HashMap* Grindstone::Plugins::Interface::GetHashedStringMap() const {
	return Grindstone::HashedString::GetHashedStringMap();
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

void Plugins::Interface::RegisterEditorSystem(const char* name, ECS::SystemFactory factory) {
	systemRegistrar->RegisterSystem(name, factory);
}

void Plugins::Interface::UnregisterSystem(const char* name) {
	systemRegistrar->UnregisterSystem(name);
}

void Plugins::Interface::UnregisterEditorSystem(const char* name) {
	systemRegistrar->UnregisterSystem(name);
}

void Plugins::Interface::RegisterAssetRenderer(BaseAssetRenderer* assetRenderer) {
	engineCore->assetRendererManager->AddAssetRenderer(assetRenderer);
}

void Plugins::Interface::UnregisterAssetRenderer(BaseAssetRenderer* assetRenderer) {
	engineCore->assetRendererManager->RemoveAssetRenderer(assetRenderer);
}

void Plugins::Interface::RegisterAssetType(AssetType assetType, const char* typeName, AssetImporter* assetImporter) {
	engineCore->assetManager->RegisterAssetType(assetType, typeName, assetImporter);
}

void Plugins::Interface::UnregisterAssetType(AssetType assetType) {
	engineCore->assetManager->UnregisterAssetType(assetType);
}
