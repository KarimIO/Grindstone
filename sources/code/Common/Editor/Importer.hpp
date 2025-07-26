#pragma once

#include <filesystem>

#include <Editor/AssetRegistry.hpp>
#include <EngineCore/Assets/AssetManager.hpp>

namespace Grindstone::Editor {
	using ImporterVersion = uint32_t;
	using ImporterFactory = void(*)(Grindstone::Editor::AssetRegistry& assetRegistry, Grindstone::Assets::AssetManager& assetManager, const std::filesystem::path&);

	using ImporterMenuOnStart = void* (*)(const std::filesystem::path&);
	using ImporterMenuOnRender = void (*)(void* payload);
	using ImporterMenuOnCleanup = void (*)(void* payload);

	struct ImporterData {
		Grindstone::Editor::ImporterVersion importerVersion;
		Grindstone::Editor::ImporterFactory factory;
		Grindstone::Editor::ImporterMenuOnStart onMenuStart;
		Grindstone::Editor::ImporterMenuOnRender onMenuRender;
		Grindstone::Editor::ImporterMenuOnCleanup onMenuCleanup;
	};

}
