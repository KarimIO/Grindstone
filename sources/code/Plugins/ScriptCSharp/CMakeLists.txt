set(CSHARP_SOURCES CSharpManager.cpp CSharpSystem.cpp ScriptClass.cpp EntryPoint.cpp ../../EngineCore/ECS/Entity.cpp ../../EngineCore/Utils/Utilities.cpp)
set(CSHARP_HEADERS CSharpManager.hpp CSharpSystem.hpp ScriptClass.hpp)

file(GLOB_RECURSE CSHARP_COMPONENTS_SOURCES "Components/*.cpp")
file(GLOB_RECURSE CSHARP_COMPONENTS_HEADER "Components/*.hpp")
source_group("Components\\Source Files" FILES ${CSHARP_COMPONENTS_SOURCES})
source_group("Components\\Header Files" FILES ${CSHARP_COMPONENTS_HEADER})

source_group("Reflection\\Source Files" FILES ${SOURCE_REFLECTION})
source_group("Reflection\\Header Files" FILES ${HEADER_REFLECTION})

add_library(
	PluginScriptCSharp MODULE
	${CSHARP_SOURCES}
	${CSHARP_HEADERS}
	${CSHARP_COMPONENTS_SOURCES}
	${CSHARP_COMPONENTS_HEADER}
	${SOURCE_REFLECTION}
	${HEADER_REFLECTION}
	${CORE_UTILS}
)

target_compile_features(PluginScriptCSharp PRIVATE cxx_std_17)

find_path(ENTT_INCLUDE_DIRS "entt/entt.hpp")

target_compile_definitions(PluginScriptCSharp PUBLIC CSHARP_DLL GLM_ENABLE_EXPERIMENTAL)
target_link_libraries(PluginScriptCSharp PUBLIC ${CORE_LIBS} mono-2.0-sgen.lib)

target_include_directories(PluginScriptCSharp PUBLIC
	${CODE_DIR}
	${DEPS_DIR}/Mono/include/mono-2.0
	${ENTT_INCLUDE_DIRS}
)

target_link_directories(PluginScriptCSharp PUBLIC ${DEPS_DIR}/Mono/lib)
target_precompile_headers(PluginScriptCSharp PUBLIC pch.hpp)
