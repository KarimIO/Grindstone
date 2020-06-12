set(CMAKE_INCLUDE_CURRENT_DIR ON)

set(EDITOR_CORE_HEADERS EditorManager.hpp EditorInstance.hpp)
source_group("Core\\Header Files" FILES ${EDITOR_CORE_HEADERS})
set(EDITOR_OTHER_HEADERS ../Engine/Utilities/Profiling.hpp ../Engine/Utilities/SettingsFile.hpp ../Engine/Utilities/Reflection.hpp ../Engine/Utilities/DLLHandler.hpp ../Engine/Utilities/DLLGraphics.hpp ../Engine/Utilities/DLLAudio.hpp)
source_group("Other\\Header Files" FILES ${EDITOR_OTHER_HEADERS})
set(EDITOR_HEADERS ${EDITOR_OTHER_HEADERS} ${EDITOR_CORE_HEADERS}) 

set(EDITOR_CORE_SOURCES EditorManager.cpp Main.cpp)
source_group("Core\\Source Files" FILES ${EDITOR_CORE_SOURCES})
set(EDITOR_OTHER_SOURCES ../Engine/Utilities/Logger.cpp ../Engine/Utilities/DLLHandler.cpp ../Engine/Utilities/Reflection.cpp ../Engine/Utilities/DLLGraphics.cpp ../Engine/Utilities/DLLAudio.cpp)
source_group("Other\\Source Files" FILES ${EDITOR_OTHER_SOURCES})
set(EDITOR_WINDOW ../WindowModule/Win32Window.cpp ../WindowModule/Win32Window.hpp ../WindowModule/BaseWindow.hpp)
source_group("Windowing" FILES ${EDITOR_WINDOW})
set(EDITOR_SOURCES ${EDITOR_WINDOW} ${EDITOR_CORE_SOURCES} ${EDITOR_OTHER_SOURCES}) 

set(GAME_EXEC_HEADERS ../ExecutableCommon/DLLEngine.hpp ../ExecutableCommon/DLLGame.hpp)
set(GAME_EXEC_SOURCES ../ExecutableCommon/DLLEngine.cpp ../ExecutableCommon/DLLGame.cpp)
source_group("DLL Interface" FILES ${GAME_EXEC_SOURCES} ${GAME_EXEC_HEADERS})

set(INCLUDE_DIRECTORIES ../ ${DEPS_DIR}/spdlog/include/ ../Engine)

set(MONO_PATH CACHE PATH "Mono Path")
set(RAPIDJSON_PATH CACHE PATH "RAPIDJSON Path")

link_directories(${ASSIMP_LIBRARY_DIR} ${MONO_PATH}/lib)
add_executable(Editor ${EDITOR_SOURCES} ${EDITOR_HEADERS} ${GAME_EXEC_SOURCES} ${GAME_EXEC_HEADERS})

target_compile_definitions(Editor PRIVATE _CRT_SECURE_NO_WARNINGS INCLUDE_EDITOR GRIND_PROFILE)

if (MSVC)
    set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} /MTd")
endif()

set_target_properties(Editor PROPERTIES VS_DEBUGGER_WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}/bin")

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)
target_precompile_headers(Editor PRIVATE "pch.hpp")

find_package(OpenGL) 
target_link_libraries(Editor ${OPENGL_gl_LIBRARY} ${assimp_LIBRARIES})
if (WIN32)
    #target_link_libraries(Editor optimized ${bullet_LIB_REL})
	target_link_libraries(Editor ${bullet_LIB_DEB} mono-2.0-sgen.lib)
else ()
    find_package (Threads)
    target_link_libraries(Editor stdc++fs ${CMAKE_THREAD_LIBS_INIT} ${CMAKE_DL_LIBS} BulletDynamics BulletCollision LinearMath monosgen-2.0)
endif()

target_include_directories(Editor PUBLIC ${MONO_PATH}/include/mono-2.0 /usr/include/mono-2.0 ${ASSIMP_INCLUDE_DIR} ${RAPIDJSON_PATH} ${DEPS_DIR} ${RAPIDJSON_INCLUDE_DIRS} ${BULLET_INCLUDE_DIR} ${INCLUDE_DIRECTORIES})