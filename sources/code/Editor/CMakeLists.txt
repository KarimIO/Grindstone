file(GLOB_RECURSE SOURCE_MAIN Main.cpp ../Common/Utilities/ModuleLoading.cpp)

set(HEADERS_CONVERTER ModelConverter.hpp MaterialCreator.hpp ImageConverter.hpp EditorManager.hpp UiRenderer.hpp Editors/SceneEditor.hpp Editors/AssetBrowser.hpp IEditor.hpp)
set(SOURCES_CONVERTER ModelConverter.cpp MaterialCreator.cpp ImageConverter.cpp EditorManager.cpp UiRenderer.cpp Editors/SceneEditor.cpp Editors/AssetBrowser.cpp)

find_package(assimp REQUIRED)

add_executable(Editor ${SOURCE_MAIN} ${SOURCES_CONVERTER} ${HEADERS_CONVERTER})

set_property(TARGET Editor PROPERTY VS_DEBUGGER_WORKING_DIRECTORY ${BUILD_DIRECTORY})

target_include_directories(Editor
    PUBLIC ../ ${ASSIMP_INCLUDE_DIR}
)

target_link_libraries(Editor ${assimp_LIBRARIES} ${CMAKE_DL_LIBS})