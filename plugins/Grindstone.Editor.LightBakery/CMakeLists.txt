set(EDITOR_LIGHT_BAKERY_BASE ${PLUGIN_DIR}/Grindstone.Editor.LightBakery)
set(EDITOR_LIGHT_BAKERY_INCLUDES ${EDITOR_LIGHT_BAKERY_BASE}/include)
set(EDITOR_LIGHT_BAKERY_BIN ${EDITOR_LIGHT_BAKERY_BASE}/bin)
set(EDITOR_LIGHT_BAKERY_LIB ${EDITOR_LIGHT_BAKERY_BASE}/lib)
set(SRC ${EDITOR_LIGHT_BAKERY_BASE}/source)
set(INC ${EDITOR_LIGHT_BAKERY_BASE}/include)

set(EDITOR_LIGHT_BAKERY_SOURCES ${SRC}/EntryPoint.cpp)
set(EDITOR_LIGHT_BAKERY_HEADERS )

file(GLOB_RECURSE SOURCE_REFLECTION "${ENGINE_CORE_DIR}/Reflection/*.cpp")
file(GLOB_RECURSE HEADER_REFLECTION "${ENGINE_CORE_DIR}/Reflection/*.hpp")
source_group("Source Files\\Reflection" FILES ${SOURCE_REFLECTION})
source_group("Header Files\\Reflection" FILES ${HEADER_REFLECTION})

add_library(
	PluginEditorLightBakery MODULE
	${EDITOR_LIGHT_BAKERY_SOURCES}
	${EDITOR_LIGHT_BAKERY_HEADERS}
	${SOURCE_REFLECTION}
	${HEADER_REFLECTION}
	${CORE_UTILS}
)

set_property(TARGET PluginEditorLightBakery PROPERTY COMPILE_WARNING_AS_ERROR ON)
target_compile_features(PluginEditorLightBakery PRIVATE cxx_std_20)
set_target_properties(PluginEditorLightBakery PROPERTIES FOLDER Plugins/Editor)

find_path(ENTT_INCLUDE_DIRS "entt/entt.hpp")
target_compile_definitions(PluginEditorLightBakery PUBLIC GLM_ENABLE_EXPERIMENTAL EDITOR_LIGHT_BAKERY_DLL)
target_include_directories(PluginEditorLightBakery PUBLIC ${CODE_DIR} ${ENTT_INCLUDE_DIRS} ${PLUGIN_DIR})

set_target_properties(PluginEditorLightBakery PROPERTIES
	RUNTIME_OUTPUT_DIRECTORY "${EDITOR_LIGHT_BAKERY_BIN}"
	LIBRARY_OUTPUT_DIRECTORY "${EDITOR_LIGHT_BAKERY_LIB}"
	ARCHIVE_OUTPUT_DIRECTORY "${EDITOR_LIGHT_BAKERY_LIB}"
)
target_precompile_headers(PluginEditorLightBakery PUBLIC ${INC}/pch.hpp)

find_package(recastnavigation CONFIG REQUIRED)
target_link_libraries(PluginEditorLightBakery ${CORE_LIBS} Common)

message(STATUS
"PluginEditorLightBakery provides CMake targets:

- EDITOR_LIGHT_BAKERY_BASE => ${EDITOR_LIGHT_BAKERY_BASE}
- EDITOR_LIGHT_BAKERY_INCLUDES => ${EDITOR_LIGHT_BAKERY_INCLUDES}
- EDITOR_LIGHT_BAKERY_BIN => ${EDITOR_LIGHT_BAKERY_BIN}
- EDITOR_LIGHT_BAKERY_LIB => ${EDITOR_LIGHT_BAKERY_LIB}

")
