file(GLOB_RECURSE SOURCE_ASSET_RENDERER "${ENGINE_CORE_DIR}/AssetRenderer/*.cpp")
file(GLOB_RECURSE HEADER_ASSET_RENDERER "${ENGINE_CORE_DIR}/AssetRenderer/*.hpp")
source_group("AssetRenderer\\Source Files" FILES ${SOURCE_ASSET_RENDERER})
source_group("AssetRenderer\\Header Files" FILES ${HEADER_ASSET_RENDERER})

file(GLOB_RECURSE SOURCE_ASSETS_BASE ${ENGINE_CORE_DIR}/Assets/AssetManager.cpp ${ENGINE_CORE_DIR}/Assets/Assets.cpp ${COMMON_DIR}/ResourcePipeline/AssetType.cpp)
file(GLOB_RECURSE HEADER_ASSETS_BASE ${ENGINE_CORE_DIR}/Assets/AssetManager.hpp ${ENGINE_CORE_DIR}/Assets/AssetImporter.hpp ${ENGINE_CORE_DIR}/Assets/Asset.hpp ${COMMON_DIR}/ResourcePipeline/AssetType.hpp)
source_group("Assets\\Source Files" FILES ${SOURCE_ASSETS_BASE})
source_group("Assets\\Header Files" FILES ${HEADER_ASSETS_BASE})

file(GLOB_RECURSE SOURCE_ASSET_LOADERS ${ENGINE_CORE_DIR}/Assets/Loaders/ArchiveAssetLoader.cpp ${ENGINE_CORE_DIR}/Assets/Loaders/ArchiveDirectoryDeserializer.cpp)
file(GLOB_RECURSE HEADER_ASSET_LOADERS ${ENGINE_CORE_DIR}/Assets/Loaders/ArchiveAssetLoader.hpp ${ENGINE_CORE_DIR}/Assets/Loaders/ArchiveDirectoryDeserializer.hpp ${ENGINE_CORE_DIR}/Assets/Loaders/AssetLoader.hpp)
source_group("Assets\\Loaders\\Source Files" FILES ${SOURCE_ASSET_LOADERS})
source_group("Assets\\Loaders\\Header Files" FILES ${HEADER_ASSET_LOADERS})

file(GLOB_RECURSE SOURCE_ASSETS_MATERIALS "${ENGINE_CORE_DIR}/Assets/Materials/*.cpp")
file(GLOB_RECURSE HEADER_ASSETS_MATERIALS "${ENGINE_CORE_DIR}/Assets/Materials/*.hpp")
source_group("Assets\\Materials\\Source Files" FILES ${SOURCE_ASSETS_MATERIALS})
source_group("Assets\\Materials\\Header Files" FILES ${HEADER_ASSETS_MATERIALS})

file(GLOB_RECURSE SOURCE_ASSETS_SHADERS "${ENGINE_CORE_DIR}/Assets/Shaders/*.cpp")
file(GLOB_RECURSE HEADER_ASSETS_SHADERS "${ENGINE_CORE_DIR}/Assets/Shaders/*.hpp")
source_group("Assets\\Shaders\\Source Files" FILES ${SOURCE_ASSETS_SHADERS})
source_group("Assets\\Shaders\\Header Files" FILES ${HEADER_ASSETS_SHADERS})

file(GLOB_RECURSE SOURCE_ASSETS_TEXTURES "${ENGINE_CORE_DIR}/Assets/Textures/*.cpp")
file(GLOB_RECURSE HEADER_ASSETS_TEXTURES "${ENGINE_CORE_DIR}/Assets/Textures/*.hpp")
source_group("Assets\\Textures\\Source Files" FILES ${SOURCE_ASSETS_TEXTURES})
source_group("Assets\\Textures\\Header Files" FILES ${HEADER_ASSETS_TEXTURES})

file(GLOB_RECURSE SOURCE_AUDIO "${ENGINE_CORE_DIR}/Audio/*.cpp")
file(GLOB_RECURSE HEADER_AUDIO "${ENGINE_CORE_DIR}/Audio/*.hpp")
source_group("Audio\\Source Files" FILES ${SOURCE_AUDIO})
source_group("Audio\\Header Files" FILES ${HEADER_AUDIO})

file(GLOB_RECURSE SOURCE_BUILD_SETTINGS "${ENGINE_CORE_DIR}/BuildSettings/*.cpp")
file(GLOB_RECURSE HEADER_BUILD_SETTINGS "${ENGINE_CORE_DIR}/BuildSettings/*.hpp")
source_group("BuildSettings\\Source Files" FILES ${SOURCE_BUILD_SETTINGS})
source_group("BuildSettings\\Header Files" FILES ${HEADER_BUILD_SETTINGS})

file(GLOB_RECURSE SOURCE_CORE_COMPONENTS "${ENGINE_CORE_DIR}/CoreComponents/*.cpp")
file(GLOB_RECURSE HEADER_CORE_COMPONENTS "${ENGINE_CORE_DIR}/CoreComponents/*.hpp")
source_group("CoreComponents\\Source Files" FILES ${SOURCE_CORE_COMPONENTS})
source_group("CoreComponents\\Header Files" FILES ${HEADER_CORE_COMPONENTS})

file(GLOB_RECURSE SOURCE_CORE_SYSTEMS "${ENGINE_CORE_DIR}/CoreSystems/*.cpp")
file(GLOB_RECURSE HEADER_CORE_SYSTEMS "${ENGINE_CORE_DIR}/CoreSystems/*.hpp")
source_group("CoreSystems\\Source Files" FILES ${SOURCE_CORE_SYSTEMS})
source_group("CoreSystems\\Header Files" FILES ${HEADER_CORE_SYSTEMS})

file(GLOB_RECURSE SOURCE_ECS "${ENGINE_CORE_DIR}/ECS/*.cpp")
file(GLOB_RECURSE HEADER_ECS "${ENGINE_CORE_DIR}/ECS/*.hpp")
source_group("ECS\\Source Files" FILES ${SOURCE_ECS})
source_group("ECS\\Header Files" FILES ${HEADER_ECS})

file(GLOB_RECURSE SOURCE_SCENES "${ENGINE_CORE_DIR}/Scenes/*.cpp")
file(GLOB_RECURSE HEADER_SCENES "${ENGINE_CORE_DIR}/Scenes/*.hpp")
source_group("Scenes\\Source Files" FILES ${SOURCE_SCENES})
source_group("Scenes\\Header Files" FILES ${HEADER_SCENES})

file(GLOB_RECURSE SOURCE_PLUGIN_SYSTEM "${ENGINE_CORE_DIR}/PluginSystem/*.cpp")
file(GLOB_RECURSE HEADER_PLUGIN_SYSTEM "${ENGINE_CORE_DIR}/PluginSystem/*.hpp")
source_group("Plugin System\\Source Files" FILES ${SOURCE_PLUGIN_SYSTEM})
source_group("Plugin System\\Header Files" FILES ${HEADER_PLUGIN_SYSTEM})

file(GLOB_RECURSE SOURCE_EVENTS "${ENGINE_CORE_DIR}/Events/*.cpp")
file(GLOB_RECURSE HEADER_EVENTS "${ENGINE_CORE_DIR}/Events/*.hpp")
source_group("Events\\Source Files" FILES ${SOURCE_EVENTS})
source_group("Events\\Header Files" FILES ${HEADER_EVENTS})

file(GLOB_RECURSE SOURCE_MEMORY ${ENGINE_CORE_DIR}/Utils/MemoryAllocator.cpp ${COMMON_DIR}/Memory/Allocators/DynamicAllocator.cpp ${COMMON_DIR}/Memory/Allocators/LinearAllocator.cpp ${COMMON_DIR}/Memory/Allocators/StackAllocator.cpp)
file(GLOB_RECURSE HEADER_MEMORY ${ENGINE_CORE_DIR}/Utils/MemoryAllocator.hpp ${COMMON_DIR}/Memory/Allocators/DynamicAllocator.hpp ${COMMON_DIR}/Memory/Allocators/LinearAllocator.hpp ${COMMON_DIR}/Memory/Allocators/StackAllocator.hpp ${ENGINE_CORE_DIR}/Memory/Allocators.hpp ${ENGINE_CORE_DIR}/Memory/Memory.hpp)
source_group("Memory\\Source Files" FILES ${SOURCE_MEMORY})
source_group("Memory\\Header Files" FILES ${HEADER_MEMORY})

file(GLOB_RECURSE SOURCE_COMMON_UTILS ${COMMON_DIR}/Utilities/ModuleLoading.cpp ${ENGINE_CORE_DIR}/Utils/Utilities.cpp ${COMMON_DIR}/ResourcePipeline/Uuid.cpp ${COMMON_DIR}/Graphics/Formats.cpp ${ENGINE_CORE_DIR}/Utils/WindowGlue.cpp)
file(GLOB_RECURSE HEADER_COMMON_UTILS ${COMMON_DIR}/Utilities/ModuleLoading.hpp ${ENGINE_CORE_DIR}/Utils/Utilities.hpp ${COMMON_DIR}/ResourcePipeline/Uuid.hpp ${COMMON_DIR}/Buffer.hpp)
source_group("Utilities\\Source Files" FILES ${SOURCE_COMMON_UTILS})
source_group("Utilities\\Header Files" FILES ${HEADER_COMMON_UTILS})

file(GLOB_RECURSE SOURCE_RENDERING "${ENGINE_CORE_DIR}/Rendering/*.cpp")
file(GLOB_RECURSE HEADER_RENDERING "${ENGINE_CORE_DIR}/Rendering/*.hpp")
source_group("Rendering\\Source Files" FILES ${SOURCE_RENDERING})
source_group("Rendering\\Header Files" FILES ${HEADER_RENDERING})

source_group("Reflection\\Source Files" FILES ${SOURCE_REFLECTION})
source_group("Reflection\\Header Files" FILES ${HEADER_REFLECTION})

file(GLOB_RECURSE SOURCE_MAIN EngineCore.cpp EntryPoint.cpp Logger.cpp Profiling.cpp)
file(GLOB_RECURSE HEADER_MAIN EngineCore.hpp pch.hpp Logger.hpp Profiling.hpp ${COMMON_DIR}/Display/Display.hpp)

set(ENGINE_CORE_SOURCES
	${SOURCE_MAIN}
	${SOURCE_ASSET_RENDERER}
	${SOURCE_ASSET_LOADERS}
	${SOURCE_ASSETS_BASE}
	${SOURCE_ASSETS_ANIMATION_CLIP}
	${SOURCE_ASSETS_MATERIALS}
	${SOURCE_ASSETS_SHADERS}
	${SOURCE_ASSETS_TEXTURES}
	${SOURCE_AUDIO}
	${SOURCE_PLUGIN_SYSTEM}
	${SOURCE_ECS}
	${SOURCE_COMMON_UTILS}
	${SOURCE_SCENES}
	${SOURCE_CORE_COMPONENTS}
	${SOURCE_CORE_SYSTEMS}
	${SOURCE_BUILD_SETTINGS}
	${SOURCE_REFLECTION}
	${SOURCE_RENDERING}
	${SOURCE_EVENTS}
	${SOURCE_MEMORY}
)

set(ENGINE_CORE_HEADERS
	${HEADER_MAIN}
	${HEADER_ASSET_RENDERER}
	${HEADER_ASSET_LOADERS}
	${HEADER_ASSETS_BASE}
	${HEADER_ASSETS_ANIMATION_CLIP}
	${HEADER_ASSETS_MATERIALS}
	${HEADER_ASSETS_MESHES_3D}
	${HEADER_ASSETS_RIG}
	${HEADER_ASSETS_SHADERS}
	${HEADER_ASSETS_TEXTURES}
	${HEADER_AUDIO}
	${HEADER_PLUGIN_SYSTEM}
	${HEADER_ECS}
	${HEADER_COMMON_UTILS}
	${HEADER_SCENES}
	${HEADER_CORE_COMPONENTS}
	${HEADER_CORE_SYSTEMS}
	${HEADER_BUILD_SETTINGS}
	${HEADER_REFLECTION}
	${HEADER_RENDERING}
	${HEADER_EVENTS}
	${HEADER_MEMORY}
)

add_library(EngineCore MODULE ${ENGINE_CORE_SOURCES} ${ENGINE_CORE_HEADERS})

target_compile_features(EngineCore PRIVATE cxx_std_17)
target_compile_definitions(EngineCore PUBLIC ENGINE_CORE NOMINMAX GLM_ENABLE_EXPERIMENTAL)

find_package(OpenAL CONFIG REQUIRED)
find_package(fmt CONFIG REQUIRED)

target_include_directories(EngineCore PUBLIC ../)

target_link_libraries(EngineCore
	Rpcrt4.lib
	rapidjson
	fmt::fmt
	OpenAL::OpenAL
)

target_precompile_headers(EngineCore PUBLIC pch.hpp)
