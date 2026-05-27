file(GLOB_RECURSE SOURCE_ASSET_RENDERER "${ENGINE_CORE_DIR}/AssetRenderer/*.cpp")
file(GLOB_RECURSE HEADER_ASSET_RENDERER "${ENGINE_CORE_DIR}/AssetRenderer/*.hpp")
source_group("Source Files\\AssetRenderer" FILES ${SOURCE_ASSET_RENDERER})
source_group("Header Files\\AssetRenderer" FILES ${HEADER_ASSET_RENDERER})

file(GLOB_RECURSE SOURCE_ASSETS_BASE ${ENGINE_CORE_DIR}/Assets/AssetManager.cpp ${ENGINE_CORE_DIR}/Assets/Assets.cpp ${COMMON_DIR}/ResourcePipeline/AssetType.cpp)
file(GLOB_RECURSE HEADER_ASSETS_BASE ${ENGINE_CORE_DIR}/Assets/AssetManager.hpp ${ENGINE_CORE_DIR}/Assets/AssetImporter.hpp ${ENGINE_CORE_DIR}/Assets/Asset.hpp ${ENGINE_CORE_DIR}/Assets/AssetReference.hpp ${ENGINE_CORE_DIR}/Assets/CountedAssetReference.hpp ${COMMON_DIR}/ResourcePipeline/AssetType.hpp)
source_group("Source Files\\Assets" FILES ${SOURCE_ASSETS_BASE})
source_group("Header Files\\Assets" FILES ${HEADER_ASSETS_BASE})

file(GLOB_RECURSE SOURCE_ASSET_LOADERS ${ENGINE_CORE_DIR}/Assets/Loaders/ArchiveAssetLoader.cpp ${ENGINE_CORE_DIR}/Assets/Loaders/ArchiveDirectoryDeserializer.cpp)
file(GLOB_RECURSE HEADER_ASSET_LOADERS ${ENGINE_CORE_DIR}/Assets/Loaders/ArchiveAssetLoader.hpp ${ENGINE_CORE_DIR}/Assets/Loaders/ArchiveDirectoryDeserializer.hpp ${ENGINE_CORE_DIR}/Assets/Loaders/AssetLoader.hpp)
source_group("Source Files\\Assets\\Loaders" FILES ${SOURCE_ASSET_LOADERS})
source_group("Header Files\\Assets\\Loaders" FILES ${HEADER_ASSET_LOADERS})

file(GLOB_RECURSE SOURCE_ASSETS_MATERIALS "${ENGINE_CORE_DIR}/Assets/Materials/*.cpp")
file(GLOB_RECURSE HEADER_ASSETS_MATERIALS "${ENGINE_CORE_DIR}/Assets/Materials/*.hpp")
source_group("Source Files\\Assets\\Materials" FILES ${SOURCE_ASSETS_MATERIALS})
source_group("Header Files\\Assets\\Materials" FILES ${HEADER_ASSETS_MATERIALS})

file(GLOB_RECURSE SOURCE_ASSETS_PIPELINE_SET "${ENGINE_CORE_DIR}/Assets/PipelineSet/*.cpp")
file(GLOB_RECURSE HEADER_ASSETS_PIPELINE_SET "${ENGINE_CORE_DIR}/Assets/PipelineSet/*.hpp")
source_group("Source Files\\Assets\\Pipeline Set" FILES ${SOURCE_ASSETS_PIPELINE_SET})
source_group("Header Files\\Assets\\Pipeline Set" FILES ${HEADER_ASSETS_PIPELINE_SET})

file(GLOB_RECURSE SOURCE_ASSETS_TEXTURES "${ENGINE_CORE_DIR}/Assets/Textures/*.cpp")
file(GLOB_RECURSE HEADER_ASSETS_TEXTURES "${ENGINE_CORE_DIR}/Assets/Textures/*.hpp")
source_group("Source Files\\Assets\\Textures" FILES ${SOURCE_ASSETS_TEXTURES})
source_group("Header Files\\Assets\\Textures" FILES ${HEADER_ASSETS_TEXTURES})

file(GLOB_RECURSE SOURCE_AUDIO "${ENGINE_CORE_DIR}/Audio/*.cpp")
file(GLOB_RECURSE HEADER_AUDIO "${ENGINE_CORE_DIR}/Audio/*.hpp")
source_group("Source Files\\Audio" FILES ${SOURCE_AUDIO})
source_group("Header Files\\Audio" FILES ${HEADER_AUDIO})

file(GLOB_RECURSE SOURCE_BUILD_SETTINGS "${ENGINE_CORE_DIR}/BuildSettings/*.cpp")
file(GLOB_RECURSE HEADER_BUILD_SETTINGS "${ENGINE_CORE_DIR}/BuildSettings/*.hpp")
source_group("Source Files\\BuildSettings" FILES ${SOURCE_BUILD_SETTINGS})
source_group("Header Files\\BuildSettings" FILES ${HEADER_BUILD_SETTINGS})

file(GLOB_RECURSE SOURCE_CORE_COMPONENTS "${ENGINE_CORE_DIR}/CoreComponents/*.cpp")
file(GLOB_RECURSE HEADER_CORE_COMPONENTS "${ENGINE_CORE_DIR}/CoreComponents/*.hpp")
source_group("Source Files\\CoreComponents" FILES ${SOURCE_CORE_COMPONENTS})
source_group("Header Files\\CoreComponents" FILES ${HEADER_CORE_COMPONENTS})

file(GLOB_RECURSE SOURCE_CORE_SYSTEMS "${ENGINE_CORE_DIR}/CoreSystems/*.cpp")
file(GLOB_RECURSE HEADER_CORE_SYSTEMS "${ENGINE_CORE_DIR}/CoreSystems/*.hpp")
source_group("Source Files\\CoreSystems" FILES ${SOURCE_CORE_SYSTEMS})
source_group("Header Files\\CoreSystems" FILES ${HEADER_CORE_SYSTEMS})

file(GLOB_RECURSE SOURCE_ECS "${ENGINE_CORE_DIR}/ECS/*.cpp")
file(GLOB_RECURSE HEADER_ECS "${ENGINE_CORE_DIR}/ECS/*.hpp")
source_group("Source Files\\ECS" FILES ${SOURCE_ECS})
source_group("Header Files\\ECS" FILES ${HEADER_ECS})

file(GLOB_RECURSE SOURCE_SCENES "${ENGINE_CORE_DIR}/Scenes/*.cpp")
file(GLOB_RECURSE HEADER_SCENES "${ENGINE_CORE_DIR}/Scenes/*.hpp")
source_group("Source Files\\Scenes" FILES ${SOURCE_SCENES})
source_group("Header Files\\Scenes" FILES ${HEADER_SCENES})

file(GLOB_RECURSE SOURCE_PLUGIN_SYSTEM "${ENGINE_CORE_DIR}/PluginSystem/*.cpp")
file(GLOB_RECURSE HEADER_PLUGIN_SYSTEM "${ENGINE_CORE_DIR}/PluginSystem/*.hpp")
source_group("Source Files\\Plugin System" FILES ${SOURCE_PLUGIN_SYSTEM})
source_group("Header Files\\Plugin System" FILES ${HEADER_PLUGIN_SYSTEM})

file(GLOB_RECURSE SOURCE_EVENTS "${ENGINE_CORE_DIR}/Events/*.cpp")
file(GLOB_RECURSE HEADER_EVENTS "${ENGINE_CORE_DIR}/Events/*.hpp")
source_group("Source Files\\Events" FILES ${SOURCE_EVENTS})
source_group("Header Files\\Events" FILES ${HEADER_EVENTS})

file(GLOB_RECURSE SOURCE_MEMORY ${COMMON_DIR}/Memory/Allocators/LinearAllocator.cpp ${COMMON_DIR}/Memory/Allocators/StackAllocator.cpp)
file(GLOB_RECURSE HEADER_MEMORY ${COMMON_DIR}/Memory/Allocators/LinearAllocator.hpp ${COMMON_DIR}/Memory/Allocators/StackAllocator.hpp ${ENGINE_CORE_DIR}/Memory/Allocators.hpp ${ENGINE_CORE_DIR}/Memory/Memory.hpp)
source_group("Source Files\\Memory" FILES ${SOURCE_MEMORY})
source_group("Header Files\\Memory" FILES ${HEADER_MEMORY})

file(GLOB_RECURSE SOURCE_COMMON_UTILS ${CORE_UTILS} ${COMMON_DIR}/Utilities/ModuleLoading.cpp ${ENGINE_CORE_DIR}/Utils/Utilities.cpp ${COMMON_DIR}/ResourcePipeline/Uuid.cpp ${ENGINE_CORE_DIR}/Utils/WindowGlue.cpp ${COMMON_DIR}/HashedString.cpp)
file(GLOB_RECURSE HEADER_COMMON_UTILS ${COMMON_DIR}/Utilities/ModuleLoading.hpp ${ENGINE_CORE_DIR}/Utils/Utilities.hpp ${COMMON_DIR}/ResourcePipeline/Uuid.hpp ${COMMON_DIR}/Buffer.hpp ${COMMON_DIR}/HashedString.hpp)
source_group("Source Files\\Utilities" FILES ${SOURCE_COMMON_UTILS})
source_group("Header Files\\Utilities" FILES ${HEADER_COMMON_UTILS})

file(GLOB_RECURSE SOURCE_RENDERING "${ENGINE_CORE_DIR}/Rendering/*.cpp")
file(GLOB_RECURSE HEADER_RENDERING "${ENGINE_CORE_DIR}/Rendering/*.hpp")
source_group("Source Files\\Rendering" FILES ${SOURCE_RENDERING})
source_group("Header Files\\Rendering" FILES ${HEADER_RENDERING})

file(GLOB_RECURSE SOURCE_WORLDCONTEXT "${ENGINE_CORE_DIR}/WorldContext/*.cpp")
file(GLOB_RECURSE HEADER_WORLDCONTEXT "${ENGINE_CORE_DIR}/WorldContext/*.hpp")
source_group("Source Files\\WorldContext" FILES ${SOURCE_WORLDCONTEXT})
source_group("Header Files\\WorldContext" FILES ${HEADER_WORLDCONTEXT})

source_group("Source Files\\Reflection" FILES ${SOURCE_REFLECTION})
source_group("Header Files\\Reflection" FILES ${HEADER_REFLECTION})

file(GLOB_RECURSE SOURCE_MAIN EngineCore.cpp Logger.cpp Profiling.cpp EntryPoint.cpp)
file(GLOB_RECURSE HEADER_MAIN EngineCore.hpp Logger.hpp Profiling.hpp pch.hpp ${COMMON_DIR}/Display/Display.hpp)

set(ENGINE_CORE_SOURCES
	${SOURCE_MAIN}
	${SOURCE_ASSET_RENDERER}
	${SOURCE_ASSET_LOADERS}
	${SOURCE_ASSETS_BASE}
	${SOURCE_ASSETS_ANIMATION_CLIP}
	${SOURCE_ASSETS_MATERIALS}
	${SOURCE_ASSETS_PIPELINE_SET}
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
	${SOURCE_WORLDCONTEXT}
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
	${HEADER_ASSETS_PIPELINE_SET}
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
	${HEADER_WORLDCONTEXT}
)

add_library(EngineCore MODULE ${ENGINE_CORE_SOURCES} ${ENGINE_CORE_HEADERS})

set_target_properties(EngineCore PROPERTIES
	RUNTIME_OUTPUT_DIRECTORY "${BUILD_DIRECTORY}"
	LIBRARY_OUTPUT_DIRECTORY "${BUILD_DIRECTORY}"
	ARCHIVE_OUTPUT_DIRECTORY "${BUILD_DIRECTORY}"
)

foreach( OUTPUTCONFIG ${CMAKE_CONFIGURATION_TYPES} )
	string( TOUPPER ${OUTPUTCONFIG} OUTPUTCONFIGUPPER )
	set_target_properties(EngineCore PROPERTIES
		RUNTIME_OUTPUT_DIRECTORY_${OUTPUTCONFIGUPPER} "${BUILD_DIRECTORY}/${OUTPUTCONFIG}"
		LIBRARY_OUTPUT_DIRECTORY_${OUTPUTCONFIGUPPER} "${BUILD_DIRECTORY}/${OUTPUTCONFIG}"
		ARCHIVE_OUTPUT_DIRECTORY_${OUTPUTCONFIGUPPER} "${BUILD_DIRECTORY}/${OUTPUTCONFIG}"
	)
endforeach( OUTPUTCONFIG CMAKE_CONFIGURATION_TYPES )

set_property(TARGET EngineCore PROPERTY COMPILE_WARNING_AS_ERROR ON)
target_compile_features(EngineCore PRIVATE cxx_std_20)
target_compile_definitions(EngineCore PUBLIC ENGINE_CORE NOMINMAX GLM_ENABLE_EXPERIMENTAL)

find_package(fmt CONFIG REQUIRED)

target_include_directories(EngineCore PUBLIC ../)

target_link_libraries(EngineCore
	${CORE_LIBS}
	Common
)

target_precompile_headers(EngineCore PUBLIC pch.hpp)
